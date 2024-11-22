/*******************************************************************************
  devzDDM.c

  Device support for the BNL Multi-element germanium detector based on Maia 
  electronics
  D. P. Siddons
 ********************************************************************************
 1.0  1/15/2016  dps     first ZynQ implementation
 *******************************************************************************/

#include    <epicsVersion.h>

#if EPICS_VERSION < 3 || (EPICS_VERSION==3 && EPICS_REVISION < 14)
#define NOT_YET_OSI
#endif

#ifdef HAS_IOOPS_H
#include    <basicIoOps.h>
#endif

/*typedef unsigned int uint32;
  typedef unsigned short uint16;
 */
#include    <stdio.h>
#include    <stdlib.h>
#include    <errno.h>
#include    <errlog.h>
#include    <string.h>

#include    <unistd.h>
#include    <fcntl.h>
#include    <sys/mman.h>
#include    <epicsTimer.h>
#include    <epicsThread.h>
#include    <epicsExport.h>
#include    <registryFunction.h>
#include    <iocsh.h>

epicsTimerQueueId       zDDMWdTimerQ=0;


#include    <string.h>
#include    <math.h>

#include    <alarm.h>
/* #include <dbRecType.h>  tmm: EPICS 3.13 */
#include    <dbDefs.h>
#include    <dbAccess.h>
#include    <dbCommon.h>
#include    <dbScan.h>
#include    <recGbl.h>
#include    <recSup.h>
#include    <devSup.h>
/*#include  <drvSup.h> */
#include    <dbScan.h>
#include    <special.h>
#include    <callback.h>
#include    <epicsInterrupt.h>
#include    <iocsh.h>
#include    <epicsExport.h>
#include    <cantProceed.h>
#include    <recSup.h>
#include    <devSup.h>
#include     <stdint.h>
#include    <stdarg.h>
#include    <string.h>
#include    <sys/time.h>
#include    <time.h>
#include    <assert.h>
#include    <signal.h>
#include    <uuid/uuid.h>

#include    "zDDMRecord.h"
#include    "devzDDM.h"
/*#include  "zhelpers.h"*/
#include    "pl_lib.h"
#include    "pl_reg.h"
#include    "log.h"

#define FAST_LOCK        epicsMutexId
#define FASTLOCKINIT(PFAST_LOCK) (*(PFAST_LOCK) = epicsMutexCreate())
#define FASTLOCK(PFAST_LOCK)     epicsMutexLock(*(PFAST_LOCK));
#define TRYLOCK(PFAST_LOCK)      epicsMutexTryLock(*(PFAST_LOCK));
#define FASTUNLOCK(PFAST_LOCK)   epicsMutexUnlock(*(PFAST_LOCK));

#define ERROR -1
#define OK     0

/*** Debug support ***/
#define PRIVATE_FUNCTIONS 0 /* normal:1, debug:0 */
#if PRIVATE_FUNCTIONS
#define STATIC static
#else
#define STATIC
#endif
#ifdef NODEBUG
#define Debug0(l,f) ;
#define Debug(l,f,v) ;
#else
#define Debug0(l,FMT) {  if(l <= devzDDMdebug)                    \
                         {                                        \
                             printf("%s(%d):",__FILE__,__LINE__); \
                             printf(FMT);                         \
                             }                                    \
                         }
#define Debug(l,FMT,V) {  if(l <= devzDDMdebug)                    \
                          {                                        \
                              printf("%s(%d):",__FILE__,__LINE__); \
                              printf(FMT,V); }                     \
                          }
#endif


#define zDDM_DELTA 2

#define BIT_SET(reg,bit) reg = reg | (1 << bit)
#define BIT_CLR(reg,bit) reg = reg & ~(1 << bit)
#define BIT_TST(reg,bit) reg & (1 << bit)

extern int zDDM_NCHAN;
extern int zDDM_NCHIPS;



volatile unsigned int *fpgabase;  /* mmap'd fpga registers */
int fd, fe; /* file descriptor for memory map device and IRQ service*/

/*extern*/ epicsMutexId SPI_lock;

volatile int devzDDMdebug=0;
epicsExportAddress(int, devzDDMdebug);

volatile int devSPIdebug=0;
epicsExportAddress(int, devSPIdebug);

/*static long vsc_num_cards = 1;*/
STATIC long zDDM_report(int level);
STATIC long zDDM_init(int after);
STATIC long zDDM_init_record(struct zDDMRecord *psr, CALLBACK *pcallback);
//STATIC long zDDM_get_ioint_info(int cmd, struct dbCommon *p1, IOSCANPVT *p2);
#define zDDM_get_ioint_info NULL
STATIC long zDDM_reset(void);
//#define zDDM_reset NULL
STATIC long zDDM_read(void *pscal);
STATIC long zDDM_write_preset(zDDMRecord *psr, int val);
STATIC long zDDM_arm(void *pscal, int val);
STATIC long zDDM_done(zDDMRecord *pscal);
int wrap(void *);

det_DSET devzDDM = {
    9, 
    zDDM_report,
    zDDM_init,
    zDDM_init_record,
    zDDM_get_ioint_info,
    zDDM_reset,
    zDDM_read,
    zDDM_write_preset,
    zDDM_arm,
    zDDM_done
};
epicsExportAddress(dset, devzDDM);


STATIC det_state zDDM_state;
STATIC int channels[MAX_CHANNELS];
unsigned char glb;
unsigned int  dacs[5];


/**************************************************
 * zDDM_report()
 ***************************************************/
STATIC long zDDM_report(int level)
{
    if (zDDM_state.card_exists)
    {
        printf( "NSLS Multi-element Silicon Detector #%d\n: No. elements= %d\n",
                0,
                zDDM_state.num_channels
              );
    }
    return (0);
}

/**************************************************
 * zDDM_shutdown()
 ***************************************************/
STATIC int zDDM_shutdown()
{
    // if (zDDM_reset()<0) return(ERROR);
    return(0);
}


/*============================================
 * ASIC I/O operations.
 *   - SPI
 *   - Token
 *   - SDAC
 *============================================/

/*****************************************
 * Set SDI 
 *  SPI Register
 *     bit 0 = sdi
 *     bit 1 = sck 
 ******************************************/
void set_sdi()
{
    fpgabase[SPIREG] = 1;
}

/*****************************************
 * Clear SDI 
 *  SPI Register
 *     bit 0 = sdi
 *     bit 1 = sck
 ******************************************/
void clear_sdi()
{
    fpgabase[SPIREG] = 0;
}



/*****************************************
 * Set Token  (TDA)
 *  Token Register
 *     bit 0 = tda
 *     bit 1 = clock
 ******************************************/
void set_token()
{
    Debug0(3,"Set TOKENREG\n");
    fpgabase[TOKENREG] = 1;
}

/*****************************************
 * Clear Token  (TDA)
 *  Token Register
 *     bit 0 = tda
 *     bit 1 = clock
 ******************************************/
void clear_token()
{
    Debug0(3,"Clear TOKENREG\n");
    fpgabase[TOKENREG] = 0;
}

/*****************************************
 * Clock Token  (TDA)
 *  Token Register
 *     bit 0 = tda
 *     bit 1 = clock
 ******************************************/
void clock_token()
{
    func_in;
    int i, tda;

    Debug0(3,"Clock TOKENREG\n");

    //read current value of token
    tda = fpgabase[TOKENREG] & 0x1;
    
    //set tck high 
    fpgabase[TOKENREG] = 0x2 | tda;
    
    //small delay, to debug with scope
    for (i=0; i<100; i++);
    
    //set tck low
    fpgabase[TOKENREG] = 0x0 | tda; 
    
    //small delay, to debug with scope
    for (i=0; i<100; i++);
    func_out;
}

/*****************************************
 * Send out an SPI bit
 * SPI Register 
 *   bit 0 = data out
 *   bit 1 = clk
 *
 * Assumes clock is high on entry and exit
 * Puts data on data out pin and toggles clock
 ******************************************/
void send_spi_bit(int val)
{
    int sda;

    sda = val & 0x1;
    //printf("%d ",(val & 0x1));

    //set data with clock high
    fpgabase[SPIREG] = sda | 0x2;

    //set clk low 
    fpgabase[SPIREG] =  (~0x2) & sda;

    //set clk high
    fpgabase[SPIREG] =  sda | 0x2;
}

/*****************************************
 * Read in an SPI bit
 * SPI Register 
 *   bit 0 = data out
 *   bit 1 = clk
 * SPIOUT reg. bit 0 = data in
 *
 * Assumes clock is high on entry and exit
 * Sets clock to 1 and reads data on data in
 * Resets clock to 0 and returns.
 ******************************************/
int get_spi_bit()
{
    int bit;
    
    //set sclk low 
    fpgabase[SPIREG] = 0;
    
    //get data with clock low
    bit = (fpgabase[SPIOUT] & 0x1);
    
    //set clk high 
    fpgabase[SPIREG] =  0x2;
    
    return(bit);
}


void write_spi(int value, int nbits)
{
    int i;
    for( i=1; i<=nbits; i++)
    {
        send_spi_bit(value & (0x1>>i));
    }
}

int read_spi(int nbits)
{
    int i, bit;
    int value=0;
    for( i=1; i<=nbits; i++ )
    {
        bit   = get_spi_bit();
        value = value | (bit<<i);
    }
    return(value);
}

void token_step(void)
{

    func_in;
    int i, tda;

    Debug0(3,"Token step\n");
    
    //read current value of token
    tda = fpgabase[TOKENREG] & 0x1;
    
    //set tck high 
    fpgabase[TOKENREG] = 0x2 | tda;
    
    //small delay, to debug with scope
    for (i=0;i<100;i++);
    
    //set tck low
    fpgabase[TOKENREG] = 0x0 | tda; 
    
    //small delay, to debug with scope
    for (i=0;i<100;i++);
    func_out;
}


void setread(void)
{
    fpgabase[BITS] = fpgabase[BITS]&(~0x2);
}

void setwrite(void)
{
    fpgabase[BITS] = fpgabase[BITS]|0x2;
}


void setdevice(void)
{
    fpgabase[TOKENREG] = fpgabase[TOKENREG]|0x1;
}

void cleardevice(void)
{
    fpgabase[TOKENREG] = fpgabase[TOKENREG]&(~0x1);
}


int get_SDAC()
{
    return((fpgabase[BITS]&0x4)>>2);
}

void set_SDAC(int i)
{
    int sdac;

    sdac = fpgabase[BITS];

    if(i)
        fpgabase[BITS] = sdac | 0x4;
    else
        fpgabase[BITS] = sdac & (~0x4);
}

void enable_ASIC_spi(void)
{
}

void disable_ASIC_spi(void)
{
}


/*============================================/


/* stuff structs into chp */
int stuffit( void *pscal )
{
    func_in;
    zDDMRecord *pdet = (zDDMRecord *)pscal;
    int i, j, index, chip, SDAC;
    unsigned char tmp8;

    wrap( pdet ); /* set up all bits and regs in channels[] array */
    FASTLOCK( &(SPI_lock) );

    SDAC=get_SDAC();
    set_SDAC(0);
    Debug(3,"stuffit_entry: SDAC=%i\n",SDAC);
    setwrite();
    set_token(); /* set chip-select for first chip */
    index=0;
    for ( chip=0; chip<zDDM_NCHIPS; chip++ )
    {
        for(i=31;i>=0;i--)
        {
            /*send channel bits*/
            for ( j=27; j>=0; j-- )
                send_spi_bit(channels[index] >> (j+4));
            index++;
        }
        /* send chip global bits */
        tmp8 = glb;
        for ( j=7; j>=0; j-- )
            send_spi_bit( tmp8 >> j );

        /* move to next chip */
        token_step();
        if( chip==0 )
        {
            clear_token();
        }
    }
    token_step(); /* move token out of chips */
    set_SDAC(SDAC);

    /* a few more for good luck */
    for( i=0; i<5; i++)
    {
        token_step();
    }

    FASTUNLOCK( &(SPI_lock) );
    func_out;
    return(0);
}

int stuff_DAC(void *pscal)
{
    func_in;

    int chip, SDAC;
    //int NCHIPS=zDDM_NCHIPS;
    zDDMRecord *pdet = (zDDMRecord *)pscal;
    short *ivl0, *ivl1, *ivh1, *ivl2, *ivh2; 

    ivl0 = pdet->pivl0;
    ivl1 = pdet->pivl1;
    ivh1 = pdet->pivh1;
    ivl2 = pdet->pivl2;
    ivh2 = pdet->pivh2;


    FASTLOCK( &(SPI_lock) );
    enable_ASIC_spi();
    SDAC=get_SDAC();
    /*printf("stuff_DAC: SDAC=%i\n",SDAC);*/
    set_SDAC(1);
    setwrite();
    setdevice(); /* set chip-select for first chip */
    for ( chip=0; chip<zDDM_NCHIPS; chip++ )
    {

        write_spi(ivl0[chip],10);
        write_spi(ivl1[chip],10);
        write_spi(ivh1[chip],10);
        write_spi(ivl2[chip],10);
        write_spi(ivh2[chip],10);

        if (devSPIdebug >= 3)
            errlogPrintf( "DACs: %i %i %i %i %i\n",
                          ivl0[chip],
                          ivl1[chip],
                          ivh1[chip],
                          ivl2[chip],
                          ivh2[chip]);

        token_step();
        if( chip == 0 )
        {
            cleardevice();
        }
    }
    token_step();/* move token out of chips */  
    set_SDAC(SDAC);
    disable_ASIC_spi();
    FASTUNLOCK( &(SPI_lock) );
    func_out;
    return(0);
}

/* Service routine called when
 * frame timer times out.  */
void frame_done(int signum)
{
    func_in;
    CALLBACK *pcallbacks;

    zDDMRecord *psr = zDDM_state.psr;
    devPVT *pPvt = (devPVT *)psr->dpvt;
    struct rpvtStruct *rpvt = (struct rpvtStruct *)psr->rpvt;
    pcallbacks = rpvt->pcallbacks;

    pPvt->done=1;
    Debug(2, "Counting exit: done = %d\n\r", pPvt->done); 
    Debug(2, "Frame no. %d\n\r", psr->runno); 
    callbackRequest( &pcallbacks[2] );
}


STATIC long timeout( struct zDDMRecord *psr )
{
    int trig=0;
    int prevtrig=0;

    Debug0(3,"Thread running\n");
    while(1)
    {
        trig = fpgabase[TRIG];
        //      Debug(3,"Trig = %d\n\r",trig);
		//trace2("prevtrig = %d, trig = %d", prevtrig, trig);
        if( (prevtrig==1) && (trig==0) )
        {
            trace("frame done");
            Debug0(3,"Frame done!\n");
            frame_done(1);
            /*       prevtrig=trig;*/
            epicsThreadSleep(0.001);
        }
        else
        {
            /*trace3("frame trig = %d, frame length = %d, frame count = %d",
			       fpgabase[TRIG],
			       fpgabase[PR1],
				   fpgabase[CNTR]
				  );*/
        }
        prevtrig = trig;
        //epicsThreadSleep(1);
    }     
}



/***************************************************
 * initialize all software and hardware
 * zDDM_init()
 ****************************************************/

STATIC long zDDM_init(int after)
{
    func_in;
    Debug(2,"zDDM_init(): entry, after = %d\n\r", after);
    if (after) return(0);

    zDDM_state.card_exists = 1;
    printf("Hardware init.\n");
    Debug(3,"zDDM_init: Total cards = %d\n\n\r",1);

    if (zDDM_shutdown() < 0)
        errlogPrintf ( "zDDM_init: zDDM_shutdown() failed\n\r" );

    zDDMWdTimerQ = epicsTimerQueueAllocate(
            1, /* ok to share queue */
            epicsThreadPriorityLow);

    Debug0(3,"zDDMWdTimerQ created\n\r");

    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (fd < 0)
    {
        printf( "Can't open /dev/mem\n" );
        exit(1);
    }

    fpgabase = (unsigned int *) mmap( 0,
                                      255,
                                      PROT_READ|PROT_WRITE,MAP_SHARED,
                                      fd,
                                      0x43C00000
                                    );

    if ( fpgabase == NULL )
    {
        printf("Can't map FPGA space\n");
        exit(1);
    }

    Debug(3,"zDDM_init: zDDM %i initialized\n\r",0);
    func_out;
    return(0);
}

/***************************************************
 * zDDM_init_record()
 ****************************************************/
STATIC long zDDM_init_record(struct zDDMRecord *psr, CALLBACK *pcallback)
{
    func_in;
    int args, channels;
    char thread_name[32];

    //  struct callback *pcallbacks;

    Debug(5,"zDDM_init_record: card %d\n\r", 0);

    /* inp must be an VME_IO */
    switch (psr->inp.type)
    {
        case (VME_IO) : break;
        default:
            recGblRecordError( S_dev_badBus,
                               (void *)psr,
                               "devzDDM (init_record) Illegal OUT Bus Type" );
		trace("devzDDM (init_record) Illegal OUT Bus Type");
		func_out;
        return( S_dev_badBus );
    }

    Debug(5,"VME zDDM: card %d\n", 0);
    if (!zDDM_state.card_exists)
    {
        recGblRecordError( S_dev_badCard,
                           (void *)psr,
                           "devzDDM (init_record) card does not exist!" );
        trace("devzDDM (init_record) card does not exist!" );
        func_out;
        return( S_dev_badCard );
    }

    if (zDDM_state.card_in_use)
    {
        recGblRecordError( S_dev_badSignal,
                           (void *)psr,
                           "devzDDM (init_record) card already in use!" );
        trace("devzDDM (init_record) card already in use!" );
        func_out;
        return( S_dev_badSignal );
    }

    zDDM_state.card_in_use = 1;
    args = sscanf(psr->inp.value.vmeio.parm, "%d", &channels);
    printf( "args=%i\n", args );
    printf( "devzDDM: channels=%d  zDDM_NCHAN=%d\n",
            channels,
            zDDM_NCHAN);

    if(channels != zDDM_NCHAN)
    {
        recGblRecordError(S_dev_badSignal,(void *)psr,
                "devzDDM: Wrong number of channels!");        
        trace("devzDDM: Wrong number of channels!");        
        func_out;
        return(S_dev_badSignal);
    }
    psr->nch = channels;
    zDDM_state.num_channels = channels;
    zDDM_state.tp           = psr->tp;
    zDDM_state.psr          = psr;
    Debug(2, "scaler_init_record(): entry %d\n\r", 1);
    devPVT *dpvt;
    dpvt = callocMustSucceed(1, sizeof(devPVT), "devzDDM init_record()");
    dpvt->exists    = 1;
    dpvt->done      = 0;
    dpvt->newdat    = 0;
    dpvt->pcallback = pcallback; 
    psr->dpvt       = dpvt;
    dpvt->psr       = psr;


    /* Set up interrupt service routine */
    /*
       pl_open(&fe);
       signal(SIGIO, &frame_done); 
       fcntl(fe, F_SETOWN, getpid());
       int oflags = fcntl(fe, F_GETFL);
       fcntl(fe, F_SETFL, oflags | FASYNC);         
       return(0);
     */

    /* Set up thread to detect frame end */
    sprintf(thread_name, "Count_1" );
    epicsThreadCreate( thread_name,
                       epicsThreadPriorityHigh,
                       epicsThreadGetStackSize( epicsThreadStackMedium ),
                       (EPICSTHREADFUNC)timeout,
                       psr
                     );
    
	func_out;
	return(0);
}



STATIC long zDDM_arm(void *psscal, int val)
{    
    func_in;
    Debug(2, "scaler_arm(): entry, val = %d\n\r", val); 

    fpgabase[TRIG] = val;
    trace1("set [TRIG] to %d", fpgabase[TRIG]);
    func_out;
    return(0);
}

/* Abort counting */
STATIC long zDDM_reset(void)
{
    Debug(2, "scaler_reset(): entry %d\n\r", 1);
    fpgabase[TRIG] = 0;
    return(0);
}

/* Write timer preset into register */
STATIC long zDDM_write_preset( zDDMRecord *psr, int val )
{
    func_in;
    Debug(2, "scaler_write_preset(): entry, after = %d\n\r", 1); 
    zDDMRecord *pdet = (zDDMRecord *)psr;
    pdet->pr1 = val;
    fpgabase[PR1] = pdet->pr1;
    func_out;
    return(0);
}



/*****************************************************
 * VSCSetup()
 * User (startup file) calls this function to configure
 * us for the hardware.
 *****************************************************/
void devzDDMConfig( int num_cards,      /* maximum number of cards in crate */
                    int ndets,          /* Max. no. detectors */
                    unsigned channels ) /* Max. no. channels */
{
    /* For NSLS detector this is null, since this is an embedded system
       and only one configuration is possible. */
    extern int zDDM_NCHAN, zDDM_NCHIPS;

    if(num_cards != 1){
        printf("Configure: Illegal number of cards\n");
    }
    if(ndets != 1){
        printf("Configure: Illegal number of detectors\n");
    }
    if((channels >640)||(channels<32)){
        printf("Configure: Illegal number of channels\n");
    }
    else {
        zDDM_NCHAN  = channels;
        zDDM_NCHIPS = channels/32;
    }
    SPI_lock=epicsMutexCreate();

}

/******************************************************************
 * Routines to assemble SPI strings from arrays, and to talk to 
 * ASIC via SPI port.
 *******************************************************************/

int wrap(void *pscal)
{
    func_in;
    zDDMRecord *pdet = (zDDMRecord *)pscal;
    int i,chip,index;
    int bits;
    char tr1, tr2, tr3, tr4, *chen, *tsen;

    //tr1  = pdet->ptr1;
    //tr2  = pdet->ptr2;
    //tr3  = pdet->ptr3;
    //tr4  = pdet->ptr4;
    tr1  = *(char*)pdet->ptr1;
    tr2  = *(char*)pdet->ptr2;
    tr3  = *(char*)pdet->ptr3;
    tr4  = *(char*)pdet->ptr4;
    chen = pdet->pchen;
    tsen = pdet->ptsen;

    glb = 0;

    if( pdet->gldacen )
        BIT_SET( glb, 0 );

    if( pdet->eblk )
        BIT_SET( glb, 4 );

    switch( pdet->shpt )  /* Fixed menu order DPS 05/13 */
    {
        case 2:    /* 00 =1us */
        {
            break;
        }
        case 3:    /* 01 =0.5us */
        {
            BIT_SET(glb,5);
            break;
        }
        case 0:    /* 10 =4us */
        {
            BIT_SET(glb,6);
            break;
        }
        case 1:    /* 11 =2us */
        {
            BIT_SET(glb,5);
            BIT_SET(glb,6);
            break;
        }
    }

    if( pdet->gain )
        BIT_SET(glb,7);

    for( chip=0; chip<zDDM_NCHIPS; chip++ )
    {
        if( chip == pdet->mdac ) /* only set DAC monitor for chosen chip */
        {

            switch(pdet->sda)
            {
                case 0:
                { 
                    break;
                }
                case 1:
                {
                    BIT_SET( glb, 1 );
                    break;
                }
                case 2:
                {
                    BIT_SET( glb, 2 );
                    break;
                }
                case 3:
                {
                    BIT_SET( glb, 1 );
                    BIT_SET( glb, 2 );
                    break;
                }
                case 4:
                {
                    BIT_SET( glb, 3 );
                    break;
                }
                case 5:
                {
                    BIT_SET( glb, 1 );
                    BIT_SET( glb, 3 );
                    break;
                }
            }

        }
        else
        {
            BIT_CLR( glb, 1 );
            BIT_CLR( glb, 2 );
            BIT_CLR( glb, 3 );
        }


        Debug(30, "Global=%x\n\n",glb);

        for(i=0;i<=31;i++)
        {
            index = i + chip * 32;
            Debug(30,"index=%i\n",index);
            bits = 0;

            if( chen[index] )
                BIT_SET(bits,0);

            if( tsen[index] )
                BIT_SET(bits,1);

            if( pdet->loen == index )
                BIT_SET(bits,2);

            if( pdet->aoen == index )
                BIT_SET(bits,3);

            channels[index] = 0                  |
                              ((tr4 & 0x3f)<<26) |
                              ((tr3 & 0x3f)<<20) |
                              ((tr2 & 0x3f)<<14) |
                              ((tr1 & 0x3f)<<8)  |
                              (bits<<4);
            /*  Debug(30, "Channel=%x\n\n",channels[index]); */
        }
    }

    func_out;

    return(0);
}


/* read all counters from chip j */
int getcounts(void *pscal)
{
    func_in;
    /* get NCHIPS x 32 x 3 24-bit values via SPI 2 */

    zDDMRecord *pdet = (zDDMRecord *)pscal; 
    int i,bit,chip,index,SDAC;
    int *s1, *s2, *s3;

    Debug0(5,"getcounts\n\r");
    s1 = pdet->ps1;
    s2 = pdet->ps2;
    s3 = pdet->ps3;

    FASTLOCK( &(SPI_lock) );
    SDAC = get_SDAC(); /* save state of SDAC */
    set_SDAC(0);
    setread();
    setdevice(); /* point to first chip */ 
    index = 0;
    for ( chip=0; chip<zDDM_NCHIPS; chip++ )
    {
        bit = ( fpgabase[SPIOUT] & 0x1 ) << 24; /* get MSB of first counter in chip */
        for( i=31; i>=0; i-- )
        {
            s1[index] = bit | read_spi(23);
            s2[index] = read_spi(24); 
            s3[index] = read_spi(24);   
            index++;
        }
        /* move to next chip */
        token_step();
        if( chip == 0)
        { 
            cleardevice(); /*remove CS from first chip after moving on */
        } 
    }
    token_step(); /* move token out of chips */
    set_SDAC(SDAC);
    disable_ASIC_spi();
    FASTUNLOCK( &(SPI_lock) );
    func_out;
    return(0);
}



STATIC long zDDM_read(void *pscal)
{
    func_in;
    zDDMRecord *pdet = (zDDMRecord *)pscal;
    int status,i;
    long *val;
    int *s1,*s2,*s3;

    val = pdet->bptr;
    s1  = pdet->ps1;
    s2  = pdet->ps2;
    s3  = pdet->ps3;
    
    status = 0;

    /* need to check if detector needs to be read for fresh data
       if not, simply return old data */
    if( zDDM_state.newdata )
    {
        Debug0(3,"Reading hardware\n\r");
        status = getcounts(pdet);
        zDDM_state.newdata = 0;
        val[0] = 0;
        val[1] = 0;
        val[2] = 0;
        for( i=0; i<zDDM_NCHAN; i++ )
        {
            val[0] += s1[i];
            val[1] += s2[i];
            val[2] += s3[i];
        }
    }
    else
    {
        Debug0(3,"Using old data\n\r");
    }

    Debug(8,"zDDM_read: status %i\n\r", status);
    
    func_out;
    return((long)status);
}


STATIC long zDDM_done(zDDMRecord *psr)
{
    func_in;

    devPVT *pPvt = (devPVT *)psr->dpvt;
    Debug(2, "scaler_done(): entry, pPvt->done = %d\n\r", pPvt->done);
    if ( pPvt->done )
    {
        pPvt->done = 0;
        trace("done");
        func_out;
        return(1);
    }
    else
    {
        func_out;
        return(0);
    }
}




static const iocshArg devzDDMConfigArg0 = {"Card #", iocshArgInt};
static const iocshArg devzDDMConfigArg1 = {"No. detectors", iocshArgInt};
static const iocshArg devzDDMConfigArg2 = {"No. channels", iocshArgInt};

static const iocshArg * const devzDDMConfigArgs[3] = { &devzDDMConfigArg0,
    &devzDDMConfigArg1,
    &devzDDMConfigArg2,
};

static const iocshFuncDef devzDDMConfigFuncDef={"devzDDMConfig",3,devzDDMConfigArgs};
static void devzDDMConfigCallFunc(const iocshArgBuf *args)
{
    devzDDMConfig( (int) args[0].ival,
            (unsigned short) args[1].ival,
            (int) args[2].ival
             );
}

void registerzDDMConfig(void){
    iocshRegister(&devzDDMConfigFuncDef,&devzDDMConfigCallFunc);
}

epicsExportRegistrar(registerzDDMConfig);

void pstate(void){
    printf("In device support:\n");
    printf("\n\ncard_exists %i\n", zDDM_state.card_exists);
    printf("num_channels %i\n", zDDM_state.num_channels);
    printf("card_in_use %i\n", zDDM_state.card_in_use);
    printf("count_in_progress %i\n", zDDM_state.count_in_progress); /* count in progress? */
    printf("ident %i\n", zDDM_state.ident); /* identification info for this card */
    /*  printf("ioscanpvt %x\n", zDDM_state.ioscanpvt); */
    printf("done %i\n", zDDM_state.done);
    /*  printf("pcallback %x\n", zDDM_state.pcallback); */
}


static const iocshFuncDef pstateFuncDef={"pstate",0,NULL};
static void pstateCallFunc(const iocshArgBuf *args)
{
    pstate();
}

void registerpstate(void){
    iocshRegister(&pstateFuncDef,&pstateCallFunc);
}

epicsExportRegistrar(registerpstate);

int peek(int reg){
    printf("Register %i = %i\n",reg, fpgabase[reg]);
    return(0);
}

static const iocshArg peekArg0 = {"Register #", iocshArgInt};
static const iocshArg * const peekArgs[1] = {&peekArg0};

static const iocshFuncDef peekFuncDef={"peek",1,peekArgs};
static void peekCallFunc(const iocshArgBuf *args)
{
    peek((int) args[0].ival);
}

void registerpeek(void){
    iocshRegister(&peekFuncDef,&peekCallFunc);
}

epicsExportRegistrar(registerpeek);


void poke(int reg, int val){
    fpgabase[reg]=val;
    peek(reg);
}



static const iocshArg pokeArg0 = {"Register #", iocshArgInt};
static const iocshArg pokeArg1 = {"Val", iocshArgInt};
static const iocshArg * const pokeArgs[2] = {&pokeArg0,&pokeArg1,};


static const iocshFuncDef pokeFuncDef={"poke",2,pokeArgs};
static void pokeCallFunc(const iocshArgBuf *args)
{
    poke((int) args[0].ival, (int) args[1].ival);
}

void registerpoke(void){
    iocshRegister(&pokeFuncDef,&pokeCallFunc);
}

epicsExportRegistrar(registerpoke);


