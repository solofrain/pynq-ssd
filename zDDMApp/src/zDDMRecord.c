/************************************************************************
 * Record support for BNL germanium multi-element germanium detectors    *
 *                                                                       *
 * D. P Siddons, Jan. 2016.                                              *
 *                                                                       *
 * Provides controls for Hermes and Scepter ASICs used in                *
 * 64- and 384-element strip detectors based on Maia ASIC configuration. *
 *                                                                       *
 *************************************************************************/

#define VERSION 1.0

#include        <epicsVersion.h>

#if EPICS_VERSION < 3 || (EPICS_VERSION==3 && EPICS_REVISION < 14)
#define NOT_YET_OSI
#endif

#if defined(NOT_YET_OSI) || defined(VXWORKSTARGET)
#include  <vxWorks.h>
#ifndef __vxworks
#define __vxworks 1
#endif
#include  <types.h>
#include  <stdio.h>
#include  <unistd.h>
#include  <stdioLib.h>
#include  <lstLib.h>
#include  <string.h>
#include  <wdLib.h>
#else
#include    <epicsTimer.h>

extern epicsTimerQueueId  zDDMWdTimerQ;

#endif

#include  <stddef.h>
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <errno.h>
#include  <unistd.h>
#include  <linux/i2c-dev.h>
#include  <sys/ioctl.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <fcntl.h>
#include  <alarm.h>
#include  <callback.h>
#include  <dbDefs.h>
#include  <dbAccess.h>
#include    <dbScan.h>
#include    <dbEvent.h>
#include  <dbFldTypes.h>
#include  <errMdef.h>
#include  <recSup.h>
#include    <recGbl.h>
#include  <devSup.h>
#include  <special.h>
#include  <iocsh.h>
#include  <epicsExport.h>
#define GEN_SIZE_OFFSET
#include  "zDDMRecord.h"
#undef GEN_SIZE_OFFSET
#include  "devzDDM.h"
#include    "pl_reg.h"
#include    "log.h"

#define zDDM_STATE_IDLE 0
#define zDDM_STATE_WAITING 1
#define zDDM_STATE_COUNTING 2

const char* zDDM_STATE[] = { "zDDM_STATE_IDLE",
                             "zDDM_STATE_WAITING",
                             "zDDM_STATE_COUNTING"
                           };

#define USER_STATE_IDLE 0
#define USER_STATE_WAITING 1
#define USER_STATE_REQSTART 2
#define USER_STATE_COUNTING 3

const char* USER_STATE[] = { "USER_STATE_IDLE",
                             "USER_STATE_WAITING",
                             "USER_STATE_REQSTART",
                             "USER_STATE_COUNTING"
                           };

#ifdef NODEBUG
#define Debug(l,FMT,V) ;
#else
#define Debug(l,FMT,V) {  if(l <= zDDMRecordDebug) \
    { printf("%s(%d):",__FILE__,__LINE__); \
        printf(FMT,V); } }
#endif

int I2Cdev; /* The i2c device file name */

volatile int zDDMRecordDebug = 0;
epicsExportAddress(int,zDDMRecordDebug);
volatile int zDDM_wait_time = 3;
epicsExportAddress(int,zDDM_wait_time);

#define MIN(a,b) (a)<(b)?(a):(b)
#define MAX(a,b) (a)>(b)?(a):(b)

volatile int zDDM_NCHAN;
volatile int zDDM_NCHIPS;

//volatile unsigned long mca[MAX_CHANNELS][4096];
//volatile unsigned long tdc[MAX_CHANNELS][1024];
//volatile unsigned long spect[4096];

extern unsigned int *fpgabase;  /* mmap'd fpga registers */

extern det_state zDDM_state;
extern int stuffit();
extern int stuff_DAC();


/* Create RSET - Record Support Entry Table*/
#define report NULL
#define initialize NULL
static long init_record();
static long process();
static long special();
#define get_value NULL
static long cvt_dbaddr();
static long get_array_info();
static long put_array_info();
#define get_units NULL
static long get_precision();
#define get_enum_str NULL
#define get_enum_strs NULL
#define put_enum_str NULL
#define get_graphic_double NULL
#define get_control_double NULL
#define get_alarm_double NULL

rset zDDMRSET = {
    RSETNUMBER,
    report,
    initialize,
    init_record,
    process,
    special,
    get_value,
    cvt_dbaddr,
    get_array_info,
    put_array_info,
    get_units,
    get_precision,
    get_enum_str,
    get_enum_strs,
    put_enum_str,
    get_graphic_double,
    get_control_double,
    get_alarm_double
};
epicsExportAddress(rset,zDDMRSET);

/*struct rpvtStruct {
  epicsMutexId updateMutex;
  CALLBACK *pcallbacks;
  };*/

//extern rpvtStruct;

static void do_alarm();
static void monitor();
//static void updateCounts(zDDMRecord *pscal);

static void deviceCallbackFunc(CALLBACK *pcb)
{
    func_in;

    zDDMRecord *pscal;

    callbackGetUser(pscal, pcb);
    Debug(5,"scaler deviceCallbackFunc: entry for '%s'\n", pscal->name);
    dbScanLock((struct dbCommon *)pscal);
    process((struct dbCommon *)pscal);
    dbScanUnlock((struct dbCommon *)pscal);

    func_out;
}


static void delayCallbackFunc(CALLBACK *pcb)
{
    func_in;

    zDDMRecord *pscal;

    /*
     * User asked us to start counting after a delay that has now expired.
     * If user has not rescinded that request in the meantime, tell
     * process() to start counting.
     */
    callbackGetUser(pscal, pcb);
    Debug(5, "scaler delayCallbackFunc: entry for '%s'\n", pscal->name);
    if (pscal->us == USER_STATE_WAITING && pscal->cnt) {
        pscal->us = USER_STATE_REQSTART;
        (void)scanOnce((void *)pscal);
    }

    func_out;
}

static void autoCallbackFunc(CALLBACK *pcb)
{
    func_in;

    zDDMRecord *pscal;

    callbackGetUser(pscal, pcb);
    Debug(5, "scaler autoCallbackFunc: entry for '%s'\n", pscal->name);
    (void)scanOnce((void *)pscal);
    func_out;
}


static long cvt_dbaddr( struct dbAddr* paddr)
{
    int index;
    index=dbGetFieldIndex(paddr);
    zDDMRecord *pzDDM=(zDDMRecord *)paddr->precord;
    switch(index){
        case zDDMRecordVAL:{
                       paddr->pfield = (void *)(pzDDM->bptr);
                       paddr->no_elements = 3;
                       paddr->field_type = DBF_LONG;
                       paddr->field_size = sizeof(long);
                       paddr->dbr_field_type = DBF_LONG;
                       break;
                   }
        case zDDMRecordS1:{
                      paddr->pfield = (void *)(pzDDM->ps1);
                      paddr->no_elements = pzDDM->nelm;
                      paddr->field_type = DBF_LONG;
                      paddr->field_size = sizeof(long);
                      paddr->dbr_field_type = DBF_LONG;
                      break;
                  }
        case zDDMRecordS2:{
                      paddr->pfield = (void *)(pzDDM->ps2);
                      paddr->no_elements = pzDDM->nelm;
                      paddr->field_type = DBF_LONG;
                      paddr->field_size = sizeof(long);
                      paddr->dbr_field_type = DBF_LONG;
                      break;
                  }
        case zDDMRecordS3:{
                      paddr->pfield = (void *)(pzDDM->ps3);
                      paddr->no_elements = pzDDM->nelm;
                      paddr->field_type = DBF_LONG;
                      paddr->field_size = sizeof(long);
                      paddr->dbr_field_type = DBF_LONG;
                      break;
                  }
        case zDDMRecordTR1:{
                       paddr->pfield = (void *)(pzDDM->ptr1);
                       paddr->no_elements = pzDDM->nelm;
                       paddr->field_type = DBF_CHAR;
                       paddr->field_size = sizeof(char);
                       paddr->dbr_field_type = DBF_CHAR;
                       paddr->special = SPC_MOD;
                       break;
                   }
        case zDDMRecordTR2:{
                       paddr->pfield = (void *)(pzDDM->ptr2);
                       paddr->no_elements = pzDDM->nelm;
                       paddr->field_type = DBF_CHAR;
                       paddr->field_size = sizeof(char);
                       paddr->dbr_field_type = DBF_CHAR;
                       paddr->special = SPC_MOD;
                       break;
                   }
        case zDDMRecordTR3:{
                       paddr->pfield = (void *)(pzDDM->ptr3);
                       paddr->no_elements = pzDDM->nelm;
                       paddr->field_type = DBF_CHAR;
                       paddr->field_size = sizeof(char);
                       paddr->dbr_field_type = DBF_CHAR;
                       paddr->special = SPC_MOD;
                       break;
                   }
        case zDDMRecordTR4:{
                       paddr->pfield = (void *)(pzDDM->ptr4);
                       paddr->no_elements = pzDDM->nelm;
                       paddr->field_type = DBF_CHAR;
                       paddr->field_size = sizeof(char);
                       paddr->dbr_field_type = DBF_CHAR;
                       paddr->special = SPC_MOD;
                       break;
                   }
        case zDDMRecordVL0A:{
                      paddr->pfield = (void *)(pzDDM->pvl0a);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVL0B:{
                      paddr->pfield = (void *)(pzDDM->pvl0b);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVL1A:{
                      paddr->pfield = (void *)(pzDDM->pvl1a);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVL1B:{
                      paddr->pfield = (void *)(pzDDM->pvl1b);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVH1A:{
                      paddr->pfield = (void *)(pzDDM->pvh1a);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVH1B:{
                      paddr->pfield = (void *)(pzDDM->pvh1b);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVL2A:{
                      paddr->pfield = (void *)(pzDDM->pvl2a);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVL2B:{
                      paddr->pfield = (void *)(pzDDM->pvl2b);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVH2A:{
                      paddr->pfield = (void *)(pzDDM->pvh2a);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordVH2B:{
                      paddr->pfield = (void *)(pzDDM->pvh2b);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_FLOAT;
                      paddr->field_size = sizeof(float);
                      paddr->dbr_field_type = DBF_FLOAT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordIVL0:{
                      paddr->pfield = (void *)(pzDDM->pivl0);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_SHORT;
                      paddr->field_size = sizeof(short);
                      paddr->dbr_field_type = DBF_SHORT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordIVL1:{
                      paddr->pfield = (void *)(pzDDM->pivl1);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_SHORT;
                      paddr->field_size = sizeof(short);
                      paddr->dbr_field_type = DBF_SHORT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordIVH1:{
                      paddr->pfield = (void *)(pzDDM->pivh1);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_SHORT;
                      paddr->field_size = sizeof(short);
                      paddr->dbr_field_type = DBF_SHORT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordIVL2:{
                      paddr->pfield = (void *)(pzDDM->pivl2);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_SHORT;
                      paddr->field_size = sizeof(short);
                      paddr->dbr_field_type = DBF_SHORT;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordIVH2:{
                      paddr->pfield = (void *)(pzDDM->pivh2);
                      paddr->no_elements = pzDDM->nelm/32;
                      paddr->field_type = DBF_SHORT;
                      paddr->field_size = sizeof(short);
                      paddr->dbr_field_type = DBF_SHORT;
                      paddr->special = SPC_MOD;
                      break;
                  }

        case zDDMRecordCHEN:{
                      paddr->pfield = (void *)(pzDDM->pchen);
                      paddr->no_elements = pzDDM->nelm;
                      paddr->field_type = DBF_CHAR;
                      paddr->field_size = sizeof(char);
                      paddr->dbr_field_type = DBF_CHAR;
                      paddr->special = SPC_MOD;
                      break;
                  }
        case zDDMRecordTSEN:{
                      paddr->pfield = (void *)(pzDDM->ptsen);
                      paddr->no_elements = pzDDM->nelm;
                      paddr->field_type = DBF_CHAR;
                      paddr->field_size = sizeof(char);
                      paddr->dbr_field_type = DBF_CHAR;
                      paddr->special = SPC_MOD;
                      break;
                  }
    }
    return(0);
}

static long get_array_info(paddr, no_elements, offset)
    struct dbAddr *paddr;
    long *no_elements;
    long *offset;
{
    //   int index;
    //   index=dbGetFieldIndex(paddr);

    zDDMRecord *pzDDM=(zDDMRecord *)paddr->precord;
    //   switch(index){
    //    case zDDMRecordCHEN:{
    //      *no_elements =  pzDDM->nelm;
    //      *offset = 0;
    //      break;
    //      }
    //    case zDDMRecordTSEN:{
    //      *no_elements =  pzDDM->nelm;
    //      *offset = 0;
    //      break;
    //      }
    //    case zDDMRecordMASK:{
    *no_elements =  pzDDM->nelm;
    *offset = 0;    
    //      break;
    //      }


    //     }
    return(0);
}

static long put_array_info(paddr,nNew)
    struct dbAddr *paddr;
    long nNew;
{
    //zDDMRecord *pzDDM=(zDDMRecord *)paddr->precord;
    //
    //   pzDDM->nelm /*nord*/ = nNew;
    /*if (pzDDM->nord > pzDDM->nelm) pzDDM->nord = pzDDM->nelm;*/
    return(0);
}





static long init_record(pscal,pass)
    zDDMRecord *pscal;
    int pass;
{
    func_in;
    //  extern volatile unsigned int *fpgabase;
    long status;
    int i, nchips;
    long *val;
    int *s1, *s2, *s3; 
    float *vl0a, *vl1a, *vh1a, *vl2a, *vh2a,  *vl0b, *vl1b, *vh1b, *vl2b, *vh2b;
    /*float vl0, vl1, vh1, vl2, vh2;*/
    short *ivl0, *ivl1, *ivh1, *ivl2, *ivh2;
    char *tr1, *tr2, *tr3, *tr4, *chen, *tsen;
    //char filename[40];
    det_DSET *pdset = (det_DSET *)(pscal->dset);
    CALLBACK *pcallbacks, *pdelayCallback, *pautoCallback, *pdeviceCallback;
    struct rpvtStruct *prpvt;
    printf("zDDM init_record: pass = %d\n", pass);
    pscal->nelm=zDDM_NCHAN;
    pscal->nchips=zDDM_NCHIPS;
    nchips=zDDM_NCHIPS;        
	trace2("pass %d, .PR1 = %ld", pass, (unsigned long)pscal->pr1);
    Debug(5, "scaler init_record: pass = %d\n", pass);
    Debug(5, "init_record: .PR1 = %ld\n", (unsigned long)pscal->pr1);

    if (pass == 0) {

        /* set up i2c device */
        /*                sprintf(filename,"/dev/i2c-1");
                  if ((I2Cdev = open(filename,O_RDWR)) < 0) {
                  printf("Failed to open the bus.");
                  return(-1);
                  }
                  else printf("I2C device file opened OK\n");
         */ 
        if(pscal->nelm<32) pscal->nelm=32; /* Minimum system is 1 chip, 32 channels */
        if(nchips<=0) nchips=1;
        pscal->bptr =  (char *)calloc(3,sizeof(long));
        pscal->ptr1 =  (char *)calloc(pscal->nelm,sizeof(char));
        pscal->ptr2 =  (char *)calloc(pscal->nelm,sizeof(char));
        pscal->ptr3 =  (char *)calloc(pscal->nelm,sizeof(char));
        pscal->ptr4 =  (char *)calloc(pscal->nelm,sizeof(char));
        pscal->pvl0a =  (float *)calloc(nchips,sizeof(float));
        pscal->pvl1a =  (float *)calloc(nchips,sizeof(float));
        pscal->pvh1a =  (float *)calloc(nchips,sizeof(float));
        pscal->pvl2a =  (float *)calloc(nchips,sizeof(float));
        pscal->pvh2a =  (float *)calloc(nchips,sizeof(float));
        pscal->pvl0b =  (float *)calloc(nchips,sizeof(float));
        pscal->pvl1b =  (float *)calloc(nchips,sizeof(float));
        pscal->pvh1b =  (float *)calloc(nchips,sizeof(float));
        pscal->pvl2b =  (float *)calloc(nchips,sizeof(float));
        pscal->pvh2b =  (float *)calloc(nchips,sizeof(float));
        pscal->pivl0 = (short *) calloc(nchips, sizeof(short));
        pscal->pivl1 = (short *) calloc(nchips, sizeof(short));
        pscal->pivh1 = (short *) calloc(nchips, sizeof(short));
        pscal->pivl2 = (short *) calloc(nchips, sizeof(short));
        pscal->pivh2 = (short *) calloc(nchips, sizeof(short));
        pscal->pchen = (char *)calloc(pscal->nelm,sizeof(char));
        pscal->ptsen = (char *)calloc(pscal->nelm,sizeof(char));
        pscal->ps1 =   (int *) calloc(pscal->nelm,sizeof(int));
        pscal->ps2 =   (int *) calloc(pscal->nelm,sizeof(int));
        pscal->ps3 =   (int *) calloc(pscal->nelm,sizeof(int));
        /*if(pscal->nelm==1) {
          pscal->nord = 1;
          } else {
          pscal->nord = 0;
          } */
        pscal->vers = VERSION;
        pscal->rpvt = (void *)calloc(1, sizeof(struct rpvtStruct));
        /* Gotta have a .val field.  Make its value reproducible. */
        val = pscal->bptr;
        val[0] = 0;
        val[1] = 0;
        val[2] = 0;

        tr1=pscal->ptr1;
        tr2=pscal->ptr2;
        tr3=pscal->ptr3;
        tr4=pscal->ptr4;
        pscal->vl0=0; /* float vl0 value */
        pscal->vl1=0; /* float vl1 value */
        pscal->vh1=0; /* float vh1 value */
        pscal->vl2=0; /* float vl2 value */
        pscal->vh2=0; /* float vh2 value */

        vl0a=pscal->pvl0a; /* arrays [nchips] of slopes */
        vl1a=pscal->pvl1a;
        vh1a=pscal->pvh1a;
        vl2a=pscal->pvl2a;
        vh2a=pscal->pvh2a;

        vl0b=pscal->pvl0b; /* arrays [nchips] of offsets */
        vl1b=pscal->pvl1b;
        vh1b=pscal->pvh1b;
        vl2b=pscal->pvl2b;
        vh2b=pscal->pvh2b;

        ivl0=pscal->pivl0; /* arrays [nchips] of integer DAC values */
        ivl1=pscal->pivl1;
        ivh1=pscal->pivh1;
        ivl2=pscal->pivl2;
        ivh2=pscal->pivh2;

        chen=pscal->pchen; 
        tsen=pscal->ptsen; 
        s1=pscal->ps1;
        s2=pscal->ps2;
        s3=pscal->ps3;

        for(i=0;i<pscal->nelm;i++){
            tr1[i] = 32;
            tr2[i] = 32;
            tr3[i] = 32;
            tr4[i] = 32;
            chen[i] = 0; 
            tsen[i] = 0; 
            s1[i] = 0;
            s2[i] = 0;
            s3[i] = 0;
        }

        for(i=0;i<nchips;i++){
            vl0a[i]=341.33;
            vl1a[i]=341.33;
            vh1a[i]=341.33;
            vl2a[i]=341.33;
            vh2a[i]=341.33;
            vl0b[i]=0.0;
            vl1b[i]=0.0;
            vh1b[i]=0.0;
            vl2b[i]=0.0;
            vh2b[i]=0.0;
            ivl0[i]=0;
            ivl1[i]=0;
            ivh1[i]=0;
            ivl2[i]=0;
            ivh2[i]=0;
        }
        stuffit(pscal);
        stuff_DAC(pscal);

		trace("exit for pass 0");
        func_out;
        return (0);


    
    }

    prpvt = (struct rpvtStruct *)pscal->rpvt;


    /*** setup callback stuff (note: array of 4 callback structures) ***/
    pcallbacks = (CALLBACK *)(calloc(3,sizeof(CALLBACK)));
    if(pcallbacks==NULL){
        printf("Error! null calloc\n");
        exit(-1);
    }
    /*        prpvt->pcallbacks = (void *)pcallbacks; */
    prpvt->pcallbacks = pcallbacks;

    /* first callback to implement delay */
    pdelayCallback = (CALLBACK *)&(pcallbacks[0]);
    callbackSetCallback(delayCallbackFunc, pdelayCallback);
    callbackSetPriority(pscal->prio, pdelayCallback);
    callbackSetUser((void *)pscal, pdelayCallback);

    /* second callback to implement auto-count */
    pautoCallback = (CALLBACK *)&(pcallbacks[1]);
    callbackSetCallback(autoCallbackFunc, pautoCallback);
    callbackSetPriority(pscal->prio, pautoCallback);
    callbackSetUser((void *)pscal, pautoCallback);

    /* third callback for device support */
    pdeviceCallback = (CALLBACK *)&(pcallbacks[2]);
    callbackSetCallback(deviceCallbackFunc, pdeviceCallback);
    callbackSetPriority(pscal->prio, pdeviceCallback);
    callbackSetUser((void *)pscal, pdeviceCallback);


    /* Check that we have everything we need. */
    if (!(pdset = (det_DSET *)(pscal->dset)))
    {
        recGblRecordError(S_dev_noDSET,(void *)pscal, "zDDM: init_record");
		trace("exit for something missing");
        func_out;
        return(S_dev_noDSET);
    }

    Debug(2, "init_record: calling dset->init_record %d\n", 0);
    if (pdset->init_record)
    {
	    trace("calling dset->init_record");
        status=(*pdset->init_record)(pscal, pdeviceCallback);
        Debug(3, "init_record: dset->init_record returns %li\n", status);
        if (status) {
            pscal->card = -1;
			trace1("dset->init_record() failed with %li", status);
            func_out;
            return (status);
        }
        pscal->card = pscal->inp.value.vmeio.card;
        db_post_events(pscal,&(pscal->card),DBE_VALUE);
    }
    /* default clock freq */
    if (pscal->freq == 0.0) {
        pscal->freq = 4e6;
        db_post_events(pscal,&(pscal->freq),DBE_VALUE);
    }
	trace1("pscal->freq = %f", pscal->freq);

    /* default count time */
    if ((pscal->tp == 0.0) && (pscal->pr1 == 0)) {
        pscal->tp = 1.0;
        db_post_events(pscal,&(pscal->pr1),DBE_VALUE);
    }

    /* convert between time and clock ticks */
    if (pscal->tp) {
        /* convert time to clock ticks */
        pscal->pr1 = (unsigned long) (pscal->tp * pscal->freq);
        db_post_events(pscal,&(pscal->pr1),DBE_VALUE);
        Debug(3, "init_record: .TP != 0, so .PR1 set to %ld\n", (unsigned long)pscal->pr1);
    } else if (pscal->pr1 && pscal->freq) {
        /* convert clock ticks to time */
        pscal->tp = (double)(pscal->pr1 / pscal->freq);
        db_post_events(pscal,&(pscal->tp),DBE_VALUE);
        Debug(3, "init_record: .PR1/.FREQ != 0, so .TP set to %f\n", pscal->tp);
    }
    db_post_events(pscal,&(pscal->bptr),DBE_VALUE);
    db_post_events(pscal,&(pscal->ptr1),DBE_VALUE);
    db_post_events(pscal,&(pscal->ptr2),DBE_VALUE);
    db_post_events(pscal,&(pscal->ptr3),DBE_VALUE);
    db_post_events(pscal,&(pscal->ptr4),DBE_VALUE);
    db_post_events(pscal,&(pscal->vl0),DBE_VALUE);
    db_post_events(pscal,&(pscal->vl1),DBE_VALUE);
    db_post_events(pscal,&(pscal->vh1),DBE_VALUE);
    db_post_events(pscal,&(pscal->vl2),DBE_VALUE);
    db_post_events(pscal,&(pscal->vh2),DBE_VALUE);
    db_post_events(pscal,&(pscal->pchen),DBE_VALUE);
    db_post_events(pscal,&(pscal->ptsen),DBE_VALUE);
    db_post_events(pscal,&(pscal->ps1),DBE_VALUE);
    db_post_events(pscal,&(pscal->ps2),DBE_VALUE);
    db_post_events(pscal,&(pscal->ps3),DBE_VALUE);

    func_out;
    return(0);
}


static long process( zDDMRecord* pscal)
{
    func_in;

    int status, prev_scaler_state, /*save_pr1, old_pr1,*/ handled;
    //        double old_freq;
    int justFinishedUserCount=0, justStartedUserCount=0, putNotifyOperation=0;
    //        unsigned long *ppreset = (unsigned long *)&(pscal->pr1);
    struct rpvtStruct *prpvt = (struct rpvtStruct *)pscal->rpvt;
    CALLBACK *pcallbacks = prpvt->pcallbacks;
    det_DSET *pdset = (det_DSET *)(pscal->dset);
    //CALLBACK *pdelayCallback = (CALLBACK *)&(pcallbacks[0]); 
    CALLBACK *pautoCallback = (CALLBACK *)&(pcallbacks[1]);
    //CALLBACK *pdeviceCallback = (CALLBACK *)&(pcallbacks[2]); 

    if(zDDMRecordDebug>2)
    {
        printf("At process entry[%d]:\n",__LINE__);
        printf("User state = %d\n",pscal->us);
        printf("Scaler state = %d\n",pscal->ss);
    }

    Debug(5, "process: entry %d\n",0);
    //        epicsMutexLock(prpvt->updateMutex);
    Debug(5, "Mutex locked %d\n",0);
    pscal->pact = TRUE;
    pscal->udf = FALSE;
    prev_scaler_state = pscal->ss;

    //--------------------------------------------------
    // 1.
    /* If we're being called as a result of a done-counting interrupt, */
    /* (*pdset->done)(pscal) will return TRUE */
    if ((*pdset->done)(pscal))
    {
        trace( "1. (*pdset->done)(pscal)\n" );
        pscal->ss = zDDM_STATE_IDLE;

        Debug(5, "done=1; zDDM_STATE_IDLE %i\n",pscal->ss);

        /* Auto-count cycle is not allowed to reset .CNT field. */
        if (pscal->us == USER_STATE_COUNTING)
		{
            Debug(5, "Setting CNT to 0 %d\n",0);
            pscal->cnt = 0;
            db_post_events(pscal,&(pscal->cnt),DBE_VALUE);
            pscal->us = USER_STATE_IDLE;

            Debug(5, "USER_STATE_IDLE %d\n",0);

            justFinishedUserCount = 1;
            if (pscal->ppn) putNotifyOperation = 1;
        }
    }

    //--------------------------------------------------
	// 2.

    if (pscal->cnt != pscal->pcnt)
    {
        trace( "2. pscal->cnt != pscal->pcnt\n" );
        handled = 0;
        if (pscal->cnt && ( (pscal->us == USER_STATE_REQSTART) ||
                            (pscal->us == USER_STATE_WAITING )   ) ) 
        {
            trace("2.1 pscal-> us == USER_STATE_REQSTART or USER_STATE_WAITING");
            /*** if we're already counting (auto-count), stop ***/
            if (pscal->ss == zDDM_STATE_COUNTING)
            {
                trace("2.1.1 pscal->ss == zDDM_STATE_COUNTING. Killing count");
                Debug(5, "Killing count %d\n",0);
                (*pdset->arm)(pscal, 0);
                pscal->ss = zDDM_STATE_IDLE;
            }

            if (pscal->us == USER_STATE_REQSTART)
            {
                trace("2.1.2 pscal->us==USER_STATE_REQSTART. Start counting");
                Debug(5, "process: USER_STATE_REQSTART. Start counting %d\n",0);
                (*pdset->reset)(pscal);
                pscal->pr1 = (unsigned long)(pscal->tp * pscal->freq);
                (*pdset->write_preset)(pscal, pscal->pr1);
                Debug(5, "process: Arming scaler %d\n",0);
                (*pdset->arm)(pscal, 1);
                pscal->ss = zDDM_STATE_COUNTING;
                pscal->us = USER_STATE_COUNTING;
                handled = 1;
                justStartedUserCount = 1; 
                Debug(5, "process: handled=%d\n",handled);
                Debug(5, "process: justStartedUserCount=%d\n",justStartedUserCount);
				trace2("pscal->tp = %f, pscal->freq=%f", pscal->tp, pscal->freq);
            }
        }
        else
        {
            trace("2.2 pscal->ss != USER_STATE_REQSTART or USER_STATE_WAITING");
            if (!pscal->cnt)
            {
                trace( "!pscal->cnt\n" );
                if (pscal->ss != zDDM_STATE_IDLE)
                    (*pdset->arm)(pscal, 0);
                pscal->ss = zDDM_STATE_IDLE;
                pscal->us = USER_STATE_IDLE;
                justFinishedUserCount = 1;
                handled = 1;
                Debug(5, "process: cnt=%d\n",pscal->cnt);
                Debug(5, "process: User state=%d\n",pscal->us);
                Debug(5, "process: Scaler state=%d\n",pscal->ss);
            }
            else
                trace("pscal->cnt");
        }
        
        if (handled)
        {
            trace ( "handled" );
            pscal->pcnt = pscal->cnt;
            Debug(5, "process:handled: pcnt=%d -> cnt\n",pscal->pcnt);
        }
    }

    //--------------------------------------------------
    // 3.
    /* read and display scalers */
    /*        updateCounts(pscal); */

    if (justStartedUserCount || justFinishedUserCount)
    {
	    trace("3. justStartedUserCount || justFinishedUserCount");
        /* fire .cout link to trigger anything that should coincide with scaler integration */
        status = dbPutLink( &pscal->cout, DBR_SHORT, &pscal->cnt, 1 );
        if (!RTN_SUCCESS(status))
        {
            Debug(5, "scaler:process: ERROR %d PUTTING TO COUT LINK.\n", status);
        }
        
        if (justFinishedUserCount)
        {
            /* fire .coutp link to trigger anything that should coincide with scaler integration */
            status = dbPutLink(&pscal->coutp, DBR_SHORT, &pscal->cnt, 1);
            if (!RTN_SUCCESS(status))
            {
                Debug(5, "scaler:process: ERROR %d PUTTING TO COUTP LINK.\n", status);
            }
        }
    }

    //--------------------------------------------------
    // 4.
    /* done counting? */
    if (pscal->ss == zDDM_STATE_IDLE)
    {
        trace("4. pscal->ss == zDDM_STATE_IDLE");
        Debug(5, "Done counting? %d\n",0 );
        recGblGetTimeStamp( pscal );
        do_alarm(pscal);
        monitor(pscal);
        if ((pscal->pcnt==0) && (pscal->us == USER_STATE_IDLE))
        {
            trace("4.1 pscal->pcnt==0 && USER_STATE_IDLE");
            if (prev_scaler_state == zDDM_STATE_COUNTING)
            {
                trace("4.1.1 zDDM_STATE_COUNTING");
                zDDM_state.newdata = 1; /* tell device support that we acquired new data */
                (*pdset->read)(pscal); /* read data */
                db_post_events(pscal,pscal->bptr,DBE_VALUE);
                db_post_events(pscal,pscal->ps1,DBE_VALUE);
                db_post_events(pscal,pscal->ps2,DBE_VALUE);
                db_post_events(pscal,pscal->ps3,DBE_VALUE);
            }
            else
                trace("4.1.2 Not zDDM_STATE_COUNTING");
            recGblFwdLink(pscal);
        }
        else
            trace("4.2 pscal->pcnt!=0 or not USER_STATE_IDLE");
    }
    else
    {
        trace("4.x pscal->ss != zDDM_STATE_IDLE");
    }

    //--------------------------------------------------
    // 5.
    /* Are we in auto-count mode and not already counting? */
    if ( pscal->us == USER_STATE_IDLE &&
         pscal->cont                  &&
         pscal->ss != zDDM_STATE_COUNTING )
    {
        trace("5. (pscal->us==USER_STATE_IDLE) && (pscal->cont) && (pscal->ss != zDDM_STATE_COUNTING)");

        double dly_sec=pscal->dly1;  /* seconds to delay */

        if (justFinishedUserCount) dly_sec = MAX(pscal->dly1, zDDM_wait_time);
        if (putNotifyOperation) dly_sec = MAX(pscal->dly1, zDDM_wait_time);
        /* if (we-have-to-wait && we-haven't-already-waited) { */
        if (dly_sec > 0 && pscal->ss != zDDM_STATE_WAITING)
        {
            trace("5.1 scheduling autocount restart");
            Debug(5, "process: scheduling autocount restart: %d\n",0);
            /*
             * Schedule a callback, and make a note that counting should start
             * the next time we process (if pscal->ss is still zDDM_STATE_WAITING
             * at that time).
             */
            pscal->ss = zDDM_STATE_WAITING;  /* tell ourselves to start next time */
            callbackRequestDelayed(pautoCallback, dly_sec);
        }
        else
        {
            trace("5.2 restarting autocount");
            Debug(5, "process: restarting autocount %d\n",0);
            /* Either the delay time is zero, or pscal->ss = zDDM_STATE_WAITING
             * (we've already waited), so start auto-count counting.
             * Different rules apply for auto-count counting:
             * If (.TP1 >= .001 s) we count .TP1 seconds, regardless of any
             * presets the user may have set.
             */
            (*pdset->reset)(pscal);
            if (pscal->tp1 >= 1.e-3)
            {
                (*pdset->write_preset)(pscal, (unsigned long)(pscal->tp1*pscal->freq));
            }
            (*pdset->arm)(pscal, 1);
            pscal->ss = zDDM_STATE_COUNTING;
            Debug(5, "zDDM_STATE_COUNTING: %d\n", 0);

            /* schedule first update callback *//* Hermes can't do updates */
            /*                        if (pscal->rat1 > .1) {
                          callbackRequestDelayed(pupdateCallback, 1.0/pscal->rat1);
                          }*/
        }
    }

    pscal->pact = FALSE;
    pscal->runno=fpgabase[FRAME_NO];
    db_post_events(pscal,&(pscal->runno),DBE_VALUE);

    Debug(5, "process:Mutex unlocked\nScaler state=%d\n",pscal->ss);
    Debug(5, "process:Mutex unlocked\nUser state=%d\n",pscal->us);
    //        epicsMutexUnlock(prpvt->updateMutex);
    func_out;
    return(0);
}

/*static void updateCounts(zDDMRecord *pscal)
  {
  Debug(5, "updateCounts: exit %d\n",0);
  } */


static long special( struct dbAddr* paddr, int after )
{

    func_in;

    zDDMRecord *pscal = (zDDMRecord *)(paddr->precord);
    //  det_DSET *pdset = (det_DSET *)(pscal->dset);
    //  int special_type = paddr->special;
    int status, i=0;
    int nchips=zDDM_NCHIPS;
    int modified, DAC_modified;
    /*unsigned short *pdir, *pgate; */
    //  unsigned long *ppreset;
    struct rpvtStruct *prpvt = (struct rpvtStruct *)pscal->rpvt;  
    CALLBACK *pcallbacks = prpvt->pcallbacks;
    CALLBACK *pdelayCallback = (CALLBACK *)&(pcallbacks[0]);
    int fieldIndex = dbGetFieldIndex(paddr);
    double dly=0.0;
    extern unsigned int *fpgabase;
    float *vl0a, *vl0b, *vl1a, *vl1b, *vh1a, *vh1b;
    float *vl2a, *vl2b, *vh2a, *vh2b;
    short *ivl0, *ivl1, *ivh1, *ivl2, *ivh2;

    vl0a=pscal->pvl0a; /* arrays [nchips] of slopes */
    vl1a=pscal->pvl1a;
    vh1a=pscal->pvh1a;
    vl2a=pscal->pvl2a;
    vh2a=pscal->pvh2a;

    vl0b=pscal->pvl0b; /* arrays [nchips] of offsets */
    vl1b=pscal->pvl1b;
    vh1b=pscal->pvh1b;
    vl2b=pscal->pvl2b;
    vh2b=pscal->pvh2b;

    ivl0=pscal->pivl0; /* arrays [nchips] of integer DAC values */
    ivl1=pscal->pivl1;
    ivh1=pscal->pivh1;
    ivl2=pscal->pivl2;
    ivh2=pscal->pivh2;

    Debug(5, "special: entry; after=%d\n", after);
    if (!after)
    {
        func_out;
        return (0);
    }

    modified=0;
    DAC_modified=0;

    switch (fieldIndex) {
        case zDDMRecordCNT:
            /* Ignore redundant (pscal->cnt == 1) commands */
            Debug(5, "special: CNT; User State =%d\n", pscal->us);
            printf( "special: CNT; User State =%d\n", pscal->us );
            if (pscal->cnt && (pscal->us != USER_STATE_IDLE))
            {
                trace ( "pscal->cnt && (pscal->us != USER_STATE_IDLE)" );
                func_out;
                return(0);
            }

            /* fire .coutp link to trigger anything that should coincide with scaler integration */
            status = dbPutLink(&pscal->coutp, DBR_SHORT, &pscal->cnt, 1);
            if (!RTN_SUCCESS(status))
            {
                Debug(5, "scaler:special: ERROR %d PUTTING TO COUTP LINK.\n", status); 
                printf( "scaler:special: ERROR %d PUTTING TO COUTP LINK.\n", status ); 
            }

            /* Scan record if it's not Passive.  (If it's Passive, it'll get */
            /* scanned automatically, since .cnt is a Process-Passive field.) */
            /* Arrange to process after user-specified delay time */
            dly = pscal->dly;   /* seconds to delay */
            if (dly<0.0) dly = 0.0;
            if (dly == 0.0 || pscal->cnt == 0)
            {
                /*** handle request now ***/
                if (pscal->cnt)
                {
                    pscal->us = USER_STATE_REQSTART;
                    Debug(5, "special: start counting. User State= %d\n",pscal->us);
                    printf( "special: start counting. User State= %d\n",pscal->us);
                }
                else
                {
                    /* abort any counting or request to start counting */
                    switch (pscal->us)
                    {
                        case USER_STATE_WAITING:
                            /* We may have a watchdog timer going.  Cancel it. */
                            if (pdelayCallback->timer) epicsTimerCancel(pdelayCallback->timer);
                            pscal->us = USER_STATE_IDLE;
                            break;
                        case USER_STATE_REQSTART:
                            pscal->us = USER_STATE_IDLE;
                            break;
                        default:
                            break;
                    }
                }
                if (pscal->scan)
                {
                    trace( "scanOnce" );
                    scanOnce((void *)pscal);
                }
                else
                {
                    trace( "No scan" );
                }
            }
            else
            {
                /* schedule callback to handle request */
                trace( "schedule callback\n");
                pscal->us = USER_STATE_WAITING;
                callbackRequestDelayed(pdelayCallback, pscal->dly);
            }
            break;

        case zDDMRecordCONT:
            /* Scan record if it's not Passive.  (If it's Passive, it'll get */
            /* scanned automatically, since .cont is a Process-Passive field.) */
            if (pscal->scan)
            {
                trace( "scanOnce" );
                scanOnce((void *)pscal);
            }
            break;

        case zDDMRecordTP:
            /* convert time to clock ticks */
            pscal->pr1 = (unsigned long) (pscal->tp * pscal->freq);
			trace1("pr1 = %d", pscal->pr1);
            db_post_events(pscal,&(pscal->pr1),DBE_VALUE);
            break;

        case zDDMRecordPR1:
            /* convert clock ticks to time */
            pscal->tp = (double)(pscal->pr1 / pscal->freq);
            db_post_events(pscal,&(pscal->tp),DBE_VALUE);
            break;

        case zDDMRecordRUNNO:
            /* Get frame number */
            fpgabase[FRAME_NO]=pscal->runno;
            Debug(2, "special: RUNNO %i\n", pscal->runno);
            db_post_events(pscal,&(pscal->runno),DBE_VALUE);
            break;

        case zDDMRecordAOEN: /* set channel number which has analog output enabled */
            Debug(2, "special: AOEN %i\n", pscal->aoen);
            /* bits set by device support. Tell record to write hardware */
            modified=1;
            db_post_events(pscal,&(pscal->aoen),DBE_VALUE);
            break;

        case zDDMRecordLOEN: /* set channel which has leakage monitor out enabled */
            Debug(2, "special: LOEN\n %i", pscal->loen);
            /* write hardware */
            modified=1; 
            db_post_events(pscal,&(pscal->loen),DBE_VALUE);
            break;

        case zDDMRecordEBLK: /* Enable on-chip bias current generator */
            Debug(2, "special: EBLK\n %i", pscal->eblk);
            /* write hardware */
            modified=1; 
            db_post_events(pscal,&(pscal->eblk),DBE_VALUE);
            break;
        case zDDMRecordGLDACEN: /*enable on-chip DACs*/
            Debug(2, "special: GLDACEN\n %i", pscal->gldacen);
            /* write hardware */
            modified=1; 
            db_post_events(pscal,&(pscal->gldacen),DBE_VALUE);
        case zDDMRecordMDAC: /* Choose which chip's DACs to monitor */
            Debug(2, "special: MDAC\n %i", pscal->mdac);
            /* write hardware */
            modified=1;
            db_post_events(pscal,&(pscal->mdac),DBE_VALUE);
            break;
        case zDDMRecordSDA: /* Route on-chip DAC to analog out pin */
            Debug(2, "special: SDA\n %i", pscal->sda);
            /* write hardware */
            modified=1; 
            db_post_events(pscal,&(pscal->sda),DBE_VALUE);
            break;

        case zDDMRecordSHPT: /* set shaping time */
            Debug(2, "special: SHPT %i\n", pscal->shpt);
            modified=1;
            db_post_events(pscal,&(pscal->shpt),DBE_VALUE);
            break;

        case zDDMRecordGAIN: /* set gain Hi/Lo */
            modified=1;
            Debug(2, "special: GAIN %i\n", pscal->gain);
            db_post_events(pscal,&(pscal->gain),DBE_VALUE);
            break;

        case zDDMRecordCHEN: /* load 'channel-disabled' array */ 
            modified=1;
            db_post_events(pscal,&(pscal->pchen),DBE_VALUE);/* post change */
            Debug(2, "special: CHEN %i\n", 0);
            break;

        case zDDMRecordTSEN: /* load 'test pulse input enabled' array */
            modified=1;
            Debug(2, "special: TSEN %i\n", 0);
            db_post_events(pscal,&(pscal->ptsen),DBE_VALUE);
            break;

        case zDDMRecordTR1:  /* array of trim DAC values */
            modified=1;
            Debug(2, "special: TR1 %i\n", 0);
            db_post_events(pscal,&(pscal->ptr1),DBE_VALUE);
            break;

        case zDDMRecordTR2:  /* array of trim DAC values */
            modified=1;
            Debug(2, "special: TR2 %i\n", 0);
            db_post_events(pscal,&(pscal->ptr2),DBE_VALUE);
            break;

        case zDDMRecordTR3:  /* array of trim DAC values */
            modified=1;
            Debug(2, "special: TR3 %i\n", 0);
            db_post_events(pscal,&(pscal->ptr3),DBE_VALUE);
            break;

        case zDDMRecordTR4:  /* array of trim DAC values */
            modified=1;
            Debug(2, "special: TR4 %i\n", 0);
            db_post_events(pscal,&(pscal->ptr4),DBE_VALUE);
            break;

        case zDDMRecordVL0:  /* VL0 threshold values */
            for(i=0;i<nchips;i++){
                ivl0[i]=(short)(pscal->vl0*vl0a[i]+vl0b[i]);
            }
            DAC_modified=1;
            Debug(2, "special: VL0 %g\n", pscal->vl0);
            db_post_events(pscal,&(pscal->vl0),DBE_VALUE);
            break;

        case zDDMRecordVL1:  /* array of threshold DAC values */
            for(i=0;i<nchips;i++){
                ivl1[i]=(short)(pscal->vl1*vl1a[i]+vl1b[i]);
            }
            DAC_modified=1;
            Debug(2, "special: VL1 %g\n", pscal->vl1);
            db_post_events(pscal,&(pscal->vl1),DBE_VALUE);
            break;

        case zDDMRecordVH1:  /* array of threshold DAC values */
            for(i=0;i<nchips;i++){
                ivh1[i]=(short)(pscal->vh1*vh1a[i]+vh1b[i]);
            }
            DAC_modified=1;
            Debug(2, "special: VH1 %g\n", pscal->vh1);
            db_post_events(pscal,&(pscal->vh1),DBE_VALUE);
            break;

        case zDDMRecordVL2:  /* array of threshold DAC values */
            for(i=0;i<nchips;i++){
                ivl2[i]=(short)(pscal->vl2*vl2a[i]+vl2b[i]);
            }
            DAC_modified=1;
            Debug(2, "special: VL2 %g\n", pscal->vl2);
            db_post_events(pscal,&(pscal->vl2),DBE_VALUE);
            break;

        case zDDMRecordVH2:  /* array of threshold DAC values */
            for(i=0;i<nchips;i++){
                ivh2[i]=(short)(pscal->vh2*vh2a[i]+vh2b[i]);
            }
            DAC_modified=1;
            Debug(2, "special: VH2 %g\n", pscal->vh2);
            db_post_events(pscal,&(pscal->vh2),DBE_VALUE);
            break;
        case zDDMRecordG1:   /* set internal or external gate drive */
            modified=1;
            Debug(2, "special: G1 %i\n", 0);
            db_post_events(pscal,&(pscal->g1),DBE_VALUE);
            break;


        default:
            Debug(2, "special: Bad field index: %i\n", fieldIndex);
            break;
    } /* switch (fieldIndex) */
    
    if(modified)
    {
        Debug(2, "special: Hermes controls modified: modifed=%i\n", modified);
        stuffit(pscal);
    }
    
    if(DAC_modified){
        Debug(2, "special: DAC controls modified: modifed=%i\n", DAC_modified);
        stuff_DAC(pscal);
    }
    
    func_out;
    return(0);
}

static long get_precision(paddr, precision)
    struct dbAddr *paddr;
long          *precision;
{
    zDDMRecord *pscal = (zDDMRecord *) paddr->precord;
    int fieldIndex = dbGetFieldIndex(paddr);

    *precision = pscal->prec;
    if (fieldIndex == zDDMRecordVERS) {
        *precision = 2;
    } else if (fieldIndex >= zDDMRecordVAL) {
        *precision = pscal->prec;
    } else {
        recGblGetPrec(paddr, precision);  /* Field is in dbCommon */
    }
    return (0);
}


static void do_alarm(pscal)
    zDDMRecord *pscal;
{
    if(pscal->udf == TRUE ){
        recGblSetSevr(pscal,UDF_ALARM,INVALID_ALARM);
        return;
    }
    return;
}

static void monitor(pscal)
    zDDMRecord *pscal;
{
    unsigned short monitor_mask;

    monitor_mask = recGblResetAlarms(pscal);

    monitor_mask|=(DBE_VALUE|DBE_LOG);

    /* check all value fields for changes */
    return;
}


