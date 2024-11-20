/** @file zDDMRecord.h
 * @brief Declarations for the @ref zDDMRecord "zDDM" record type.
 *
 * This header was generated from zDDMRecord.dbd
 */

#ifndef INC_zDDMRecord_H
#define INC_zDDMRecord_H

#include "epicsTypes.h"
#include "link.h"
#include "epicsMutex.h"
#include "ellLib.h"
#include "devSup.h"
#include "epicsTime.h"

#ifndef zDDMCNT_NUM_CHOICES
/** @brief Enumerated type from menu zDDMCNT */
typedef enum {
    zDDMCNT_Done                    /**< @brief State string "Done" */,
    zDDMCNT_Count                   /**< @brief State string "Count" */
} zDDMCNT;
/** @brief Number of states defined for menu zDDMCNT */
#define zDDMCNT_NUM_CHOICES 2
#endif

#ifndef GlbDACen_NUM_CHOICES
/** @brief Enumerated type from menu GlbDACen */
typedef enum {
    GlbDACen_Y                      /**< @brief State string "Yes" */,
    GlbDACen_N                      /**< @brief State string "No" */
} GlbDACen;
/** @brief Number of states defined for menu GlbDACen */
#define GlbDACen_NUM_CHOICES 2
#endif

#ifndef Gain_NUM_CHOICES
/** @brief Enumerated type from menu Gain */
typedef enum {
    gain_H                          /**< @brief State string "H" */,
    gain_L                          /**< @brief State string "L" */
} Gain;
/** @brief Number of states defined for menu Gain */
#define Gain_NUM_CHOICES 2
#endif

#ifndef shapeT_NUM_CHOICES
/** @brief Enumerated type from menu shapeT */
typedef enum {
    shapeT_1                        /**< @brief State string "4.0us" */,
    shapeT_2                        /**< @brief State string "2.0us" */,
    shapeT_3                        /**< @brief State string "1.0us" */,
    shapeT_4                        /**< @brief State string "0.5us" */
} shapeT;
/** @brief Number of states defined for menu shapeT */
#define shapeT_NUM_CHOICES 4
#endif

#ifndef zDDMGate_NUM_CHOICES
/** @brief Enumerated type from menu zDDMGate */
typedef enum {
    zDDMGate_N                      /**< @brief State string "N" */,
    zDDMGate_Y                      /**< @brief State string "Y" */
} zDDMGate;
/** @brief Number of states defined for menu zDDMGate */
#define zDDMGate_NUM_CHOICES 2
#endif

#ifndef DACen_NUM_CHOICES
/** @brief Enumerated type from menu DACen */
typedef enum {
    DACen_1                         /**< @brief State string "None" */,
    DACen_2                         /**< @brief State string "VL0" */,
    DACen_3                         /**< @brief State string "VL1" */,
    DACen_4                         /**< @brief State string "VH1" */,
    DACen_5                         /**< @brief State string "VL2" */,
    DACen_6                         /**< @brief State string "VH2" */
} DACen;
/** @brief Number of states defined for menu DACen */
#define DACen_NUM_CHOICES 6
#endif

#ifndef zDDMCONT_NUM_CHOICES
/** @brief Enumerated type from menu zDDMCONT */
typedef enum {
    zDDMCONT_OneShot                /**< @brief State string "OneShot" */,
    zDDMCONT_AutoCount              /**< @brief State string "AutoCount" */
} zDDMCONT;
/** @brief Number of states defined for menu zDDMCONT */
#define zDDMCONT_NUM_CHOICES 2
#endif

/** @brief Declaration of zDDM record type. */
typedef struct zDDMRecord {
    char                name[61];   /**< @brief Record Name */
    char                desc[41];   /**< @brief Descriptor */
    char                asg[29];    /**< @brief Access Security Group */
    epicsEnum16         scan;       /**< @brief Scan Mechanism */
    epicsEnum16         pini;       /**< @brief Process at iocInit */
    epicsInt16          phas;       /**< @brief Scan Phase */
    char                evnt[40];   /**< @brief Event Name */
    epicsInt16          tse;        /**< @brief Time Stamp Event */
    DBLINK              tsel;       /**< @brief Time Stamp Link */
    epicsEnum16         dtyp;       /**< @brief Device Type */
    epicsInt16          disv;       /**< @brief Disable Value */
    epicsInt16          disa;       /**< @brief Disable */
    DBLINK              sdis;       /**< @brief Scanning Disable */
    epicsMutexId        mlok;       /**< @brief Monitor lock */
    ELLLIST             mlis;       /**< @brief Monitor List */
    ELLLIST             bklnk;      /**< @brief Backwards link tracking */
    epicsUInt8          disp;       /**< @brief Disable putField */
    epicsUInt8          proc;       /**< @brief Force Processing */
    epicsEnum16         stat;       /**< @brief Alarm Status */
    epicsEnum16         sevr;       /**< @brief Alarm Severity */
    char                amsg[40];   /**< @brief Alarm Message */
    epicsEnum16         nsta;       /**< @brief New Alarm Status */
    epicsEnum16         nsev;       /**< @brief New Alarm Severity */
    char                namsg[40];  /**< @brief New Alarm Message */
    epicsEnum16         acks;       /**< @brief Alarm Ack Severity */
    epicsEnum16         ackt;       /**< @brief Alarm Ack Transient */
    epicsEnum16         diss;       /**< @brief Disable Alarm Sevrty */
    epicsUInt8          lcnt;       /**< @brief Lock Count */
    epicsUInt8          pact;       /**< @brief Record active */
    epicsUInt8          putf;       /**< @brief dbPutField process */
    epicsUInt8          rpro;       /**< @brief Reprocess  */
    struct asgMember    *asp;       /**< @brief Access Security Pvt */
    struct processNotify *ppn;      /**< @brief pprocessNotify */
    struct processNotifyRecord *ppnr; /**< @brief pprocessNotifyRecord */
    struct scan_element *spvt;      /**< @brief Scan Private */
    struct typed_rset   *rset;      /**< @brief Address of RSET */
    unambiguous_dset    *dset;      /**< @brief DSET address */
    void                *dpvt;      /**< @brief Device Private */
    struct dbRecordType *rdes;      /**< @brief Address of dbRecordType */
    struct lockRecord   *lset;      /**< @brief Lock Set */
    epicsEnum16         prio;       /**< @brief Scheduling Priority */
    epicsUInt8          tpro;       /**< @brief Trace Processing */
    epicsUInt8          bkpt;       /**< @brief Break Point */
    epicsUInt8          udf;        /**< @brief Undefined */
    epicsEnum16         udfs;       /**< @brief Undefined Alarm Sevrty */
    epicsTimeStamp      time;       /**< @brief Time */
    epicsUInt64         utag;       /**< @brief Time Tag */
    DBLINK              flnk;       /**< @brief Forward Process Link */
    epicsFloat32        vers;       /**< @brief Code Version */
    void *           val;           /**< @brief Value */
    void *           bptr;          /**< @brief Buffer Pointer */
    epicsUInt32         nval;       /**< @brief Number of values */
    epicsUInt32         nelm;       /**< @brief Number of Elements */
    epicsInt16          nchips;     /**< @brief No. of chips */
    epicsFloat64        freq;       /**< @brief Time base freq */
    epicsEnum16         cnt;        /**< @brief Count */
    epicsEnum16         pcnt;       /**< @brief Prev Count */
    epicsInt16          ss;         /**< @brief zDDM state */
    epicsInt16          us;         /**< @brief User state */
    epicsEnum16         cont;       /**< @brief OneShot/AutoCount */
    DBLINK              inp;        /**< @brief Input Specification */
    epicsInt16          card;       /**< @brief Card Number */
    epicsFloat32        dly;        /**< @brief Delay */
    epicsFloat32        dly1;       /**< @brief Auto-mode Delay */
    epicsInt16          nch;        /**< @brief Number of Channels */
    epicsEnum16         g1;         /**< @brief Gate Control */
    epicsFloat64        tp;         /**< @brief Time Preset */
    epicsFloat64        tp1;        /**< @brief Auto Time Preset */
    epicsInt32          pr1;        /**< @brief Preset 1 */
    epicsFloat64        t;          /**< @brief Timer */
    epicsEnum16         sda;        /**< @brief DAC Monitor */
    epicsEnum16         shpt;       /**< @brief Shaping time */
    epicsEnum16         gain;       /**< @brief Gain */
    epicsEnum16         gldacen;    /**< @brief Global DAC en. */
    epicsInt16          mdac;       /**< @brief Which chip's DACs to watch? */
    epicsInt16          aoen;       /**< @brief Analog out chan */
    epicsInt16          loen;       /**< @brief Leakage out chan */
    epicsInt16          eblk;       /**< @brief Enable input bias current */
    void *           chen;          /**< @brief Value */
    void *           pchen;         /**< @brief Buffer Pointer */
    void *           tsen;          /**< @brief Value */
    void *           ptsen;         /**< @brief Buffer Pointer */
    void *           tr1;           /**< @brief Value */
    void *           ptr1;          /**< @brief Buffer Pointer */
    void *           tr2;           /**< @brief Value */
    void *           ptr2;          /**< @brief Buffer Pointer */
    void *           tr3;           /**< @brief Value */
    void *           ptr3;          /**< @brief Buffer Pointer */
    void *           tr4;           /**< @brief Value */
    void *           ptr4;          /**< @brief Buffer Pointer */
    epicsFloat32        vl0;        /**< @brief Value */
    void *           vl0a;          /**< @brief Value */
    void *           pvl0a;         /**< @brief Buffer Pointer */
    void *           vl0b;          /**< @brief Value */
    void *           pvl0b;         /**< @brief Buffer Pointer */
    epicsFloat32        vl1;        /**< @brief Value */
    void *           vl1a;          /**< @brief Value */
    void *           pvl1a;         /**< @brief Buffer Pointer */
    void *           vl1b;          /**< @brief Value */
    void *           pvl1b;         /**< @brief Buffer Pointer */
    epicsFloat32        vh1;        /**< @brief Value */
    void *           vh1a;          /**< @brief Value */
    void *           pvh1a;         /**< @brief Buffer Pointer */
    void *           vh1b;          /**< @brief Value */
    void *           pvh1b;         /**< @brief Buffer Pointer */
    epicsFloat32        vl2;        /**< @brief Value */
    void *           vl2a;          /**< @brief Value */
    void *           pvl2a;         /**< @brief Buffer Pointer */
    void *           vl2b;          /**< @brief Value */
    void *           pvl2b;         /**< @brief Buffer Pointer */
    epicsFloat32        vh2;        /**< @brief Value */
    void *           vh2a;          /**< @brief Value */
    void *           pvh2a;         /**< @brief Buffer Pointer */
    void *           vh2b;          /**< @brief Value */
    void *           pvh2b;         /**< @brief Buffer Pointer */
    void *           ivl0;          /**< @brief Value */
    void *           pivl0;         /**< @brief Buffer Pointer */
    void *           ivl1;          /**< @brief Value */
    void *           pivl1;         /**< @brief Buffer Pointer */
    void *           ivh1;          /**< @brief Value */
    void *           pivh1;         /**< @brief Buffer Pointer */
    void *           ivl2;          /**< @brief Value */
    void *           pivl2;         /**< @brief Buffer Pointer */
    void *           ivh2;          /**< @brief Value */
    void *           pivh2;         /**< @brief Buffer Pointer */
    void *           s1;            /**< @brief Value */
    void *           ps1;           /**< @brief Buffer Pointer */
    void *           s2;            /**< @brief Value */
    void *           ps2;           /**< @brief Buffer Pointer */
    void *           s3;            /**< @brief Value */
    void *           ps3;           /**< @brief Buffer Pointer */
    char                nm1[16];    /**< @brief Scaler A name */
    char                nm2[16];    /**< @brief Scaler B name */
    char                nm3[16];    /**< @brief Scaler C name */
    char                egu[16];    /**< @brief Units Name */
    epicsUInt32         runno;      /**< @brief Run Number */
    struct rpvtStruct *rpvt;        /**< @brief Record Private */
    epicsInt16          prec;       /**< @brief Display Precision */
    DBLINK              cout;       /**< @brief CNT Output */
    DBLINK              coutp;      /**< @brief CNT Output Prompt */
} zDDMRecord;

typedef enum {
	zDDMRecordNAME = 0,
	zDDMRecordDESC = 1,
	zDDMRecordASG = 2,
	zDDMRecordSCAN = 3,
	zDDMRecordPINI = 4,
	zDDMRecordPHAS = 5,
	zDDMRecordEVNT = 6,
	zDDMRecordTSE = 7,
	zDDMRecordTSEL = 8,
	zDDMRecordDTYP = 9,
	zDDMRecordDISV = 10,
	zDDMRecordDISA = 11,
	zDDMRecordSDIS = 12,
	zDDMRecordMLOK = 13,
	zDDMRecordMLIS = 14,
	zDDMRecordBKLNK = 15,
	zDDMRecordDISP = 16,
	zDDMRecordPROC = 17,
	zDDMRecordSTAT = 18,
	zDDMRecordSEVR = 19,
	zDDMRecordAMSG = 20,
	zDDMRecordNSTA = 21,
	zDDMRecordNSEV = 22,
	zDDMRecordNAMSG = 23,
	zDDMRecordACKS = 24,
	zDDMRecordACKT = 25,
	zDDMRecordDISS = 26,
	zDDMRecordLCNT = 27,
	zDDMRecordPACT = 28,
	zDDMRecordPUTF = 29,
	zDDMRecordRPRO = 30,
	zDDMRecordASP = 31,
	zDDMRecordPPN = 32,
	zDDMRecordPPNR = 33,
	zDDMRecordSPVT = 34,
	zDDMRecordRSET = 35,
	zDDMRecordDSET = 36,
	zDDMRecordDPVT = 37,
	zDDMRecordRDES = 38,
	zDDMRecordLSET = 39,
	zDDMRecordPRIO = 40,
	zDDMRecordTPRO = 41,
	zDDMRecordBKPT = 42,
	zDDMRecordUDF = 43,
	zDDMRecordUDFS = 44,
	zDDMRecordTIME = 45,
	zDDMRecordUTAG = 46,
	zDDMRecordFLNK = 47,
	zDDMRecordVERS = 48,
	zDDMRecordVAL = 49,
	zDDMRecordBPTR = 50,
	zDDMRecordNVAL = 51,
	zDDMRecordNELM = 52,
	zDDMRecordNCHIPS = 53,
	zDDMRecordFREQ = 54,
	zDDMRecordCNT = 55,
	zDDMRecordPCNT = 56,
	zDDMRecordSS = 57,
	zDDMRecordUS = 58,
	zDDMRecordCONT = 59,
	zDDMRecordINP = 60,
	zDDMRecordCARD = 61,
	zDDMRecordDLY = 62,
	zDDMRecordDLY1 = 63,
	zDDMRecordNCH = 64,
	zDDMRecordG1 = 65,
	zDDMRecordTP = 66,
	zDDMRecordTP1 = 67,
	zDDMRecordPR1 = 68,
	zDDMRecordT = 69,
	zDDMRecordSDA = 70,
	zDDMRecordSHPT = 71,
	zDDMRecordGAIN = 72,
	zDDMRecordGLDACEN = 73,
	zDDMRecordMDAC = 74,
	zDDMRecordAOEN = 75,
	zDDMRecordLOEN = 76,
	zDDMRecordEBLK = 77,
	zDDMRecordCHEN = 78,
	zDDMRecordPCHEN = 79,
	zDDMRecordTSEN = 80,
	zDDMRecordPTSEN = 81,
	zDDMRecordTR1 = 82,
	zDDMRecordPTR1 = 83,
	zDDMRecordTR2 = 84,
	zDDMRecordPTR2 = 85,
	zDDMRecordTR3 = 86,
	zDDMRecordPTR3 = 87,
	zDDMRecordTR4 = 88,
	zDDMRecordPTR4 = 89,
	zDDMRecordVL0 = 90,
	zDDMRecordVL0A = 91,
	zDDMRecordPVL0A = 92,
	zDDMRecordVL0B = 93,
	zDDMRecordPVL0B = 94,
	zDDMRecordVL1 = 95,
	zDDMRecordVL1A = 96,
	zDDMRecordPVL1A = 97,
	zDDMRecordVL1B = 98,
	zDDMRecordPVL1B = 99,
	zDDMRecordVH1 = 100,
	zDDMRecordVH1A = 101,
	zDDMRecordPVH1A = 102,
	zDDMRecordVH1B = 103,
	zDDMRecordPVH1B = 104,
	zDDMRecordVL2 = 105,
	zDDMRecordVL2A = 106,
	zDDMRecordPVL2A = 107,
	zDDMRecordVL2B = 108,
	zDDMRecordPVL2B = 109,
	zDDMRecordVH2 = 110,
	zDDMRecordVH2A = 111,
	zDDMRecordPVH2A = 112,
	zDDMRecordVH2B = 113,
	zDDMRecordPVH2B = 114,
	zDDMRecordIVL0 = 115,
	zDDMRecordPIVL0 = 116,
	zDDMRecordIVL1 = 117,
	zDDMRecordPIVL1 = 118,
	zDDMRecordIVH1 = 119,
	zDDMRecordPIVH1 = 120,
	zDDMRecordIVL2 = 121,
	zDDMRecordPIVL2 = 122,
	zDDMRecordIVH2 = 123,
	zDDMRecordPIVH2 = 124,
	zDDMRecordS1 = 125,
	zDDMRecordPS1 = 126,
	zDDMRecordS2 = 127,
	zDDMRecordPS2 = 128,
	zDDMRecordS3 = 129,
	zDDMRecordPS3 = 130,
	zDDMRecordNM1 = 131,
	zDDMRecordNM2 = 132,
	zDDMRecordNM3 = 133,
	zDDMRecordEGU = 134,
	zDDMRecordRUNNO = 135,
	zDDMRecordRPVT = 136,
	zDDMRecordPREC = 137,
	zDDMRecordCOUT = 138,
	zDDMRecordCOUTP = 139
} zDDMFieldIndex;

#ifdef GEN_SIZE_OFFSET

#include <epicsExport.h>
#include <cantProceed.h>
#ifdef __cplusplus
extern "C" {
#endif
static int zDDMRecordSizeOffset(dbRecordType *prt)
{
    zDDMRecord *prec = 0;

    if (prt->no_fields != 140) {
        cantProceed("IOC build or installation error:\n"
            "    The zDDMRecord defined in the DBD file has %d fields,\n"
            "    but the record support code was built with 140.\n",
            prt->no_fields);
    }
    prt->papFldDes[zDDMRecordNAME]->size = sizeof(prec->name);
    prt->papFldDes[zDDMRecordNAME]->offset = (unsigned short)offsetof(zDDMRecord, name);
    prt->papFldDes[zDDMRecordDESC]->size = sizeof(prec->desc);
    prt->papFldDes[zDDMRecordDESC]->offset = (unsigned short)offsetof(zDDMRecord, desc);
    prt->papFldDes[zDDMRecordASG]->size = sizeof(prec->asg);
    prt->papFldDes[zDDMRecordASG]->offset = (unsigned short)offsetof(zDDMRecord, asg);
    prt->papFldDes[zDDMRecordSCAN]->size = sizeof(prec->scan);
    prt->papFldDes[zDDMRecordSCAN]->offset = (unsigned short)offsetof(zDDMRecord, scan);
    prt->papFldDes[zDDMRecordPINI]->size = sizeof(prec->pini);
    prt->papFldDes[zDDMRecordPINI]->offset = (unsigned short)offsetof(zDDMRecord, pini);
    prt->papFldDes[zDDMRecordPHAS]->size = sizeof(prec->phas);
    prt->papFldDes[zDDMRecordPHAS]->offset = (unsigned short)offsetof(zDDMRecord, phas);
    prt->papFldDes[zDDMRecordEVNT]->size = sizeof(prec->evnt);
    prt->papFldDes[zDDMRecordEVNT]->offset = (unsigned short)offsetof(zDDMRecord, evnt);
    prt->papFldDes[zDDMRecordTSE]->size = sizeof(prec->tse);
    prt->papFldDes[zDDMRecordTSE]->offset = (unsigned short)offsetof(zDDMRecord, tse);
    prt->papFldDes[zDDMRecordTSEL]->size = sizeof(prec->tsel);
    prt->papFldDes[zDDMRecordTSEL]->offset = (unsigned short)offsetof(zDDMRecord, tsel);
    prt->papFldDes[zDDMRecordDTYP]->size = sizeof(prec->dtyp);
    prt->papFldDes[zDDMRecordDTYP]->offset = (unsigned short)offsetof(zDDMRecord, dtyp);
    prt->papFldDes[zDDMRecordDISV]->size = sizeof(prec->disv);
    prt->papFldDes[zDDMRecordDISV]->offset = (unsigned short)offsetof(zDDMRecord, disv);
    prt->papFldDes[zDDMRecordDISA]->size = sizeof(prec->disa);
    prt->papFldDes[zDDMRecordDISA]->offset = (unsigned short)offsetof(zDDMRecord, disa);
    prt->papFldDes[zDDMRecordSDIS]->size = sizeof(prec->sdis);
    prt->papFldDes[zDDMRecordSDIS]->offset = (unsigned short)offsetof(zDDMRecord, sdis);
    prt->papFldDes[zDDMRecordMLOK]->size = sizeof(prec->mlok);
    prt->papFldDes[zDDMRecordMLOK]->offset = (unsigned short)offsetof(zDDMRecord, mlok);
    prt->papFldDes[zDDMRecordMLIS]->size = sizeof(prec->mlis);
    prt->papFldDes[zDDMRecordMLIS]->offset = (unsigned short)offsetof(zDDMRecord, mlis);
    prt->papFldDes[zDDMRecordBKLNK]->size = sizeof(prec->bklnk);
    prt->papFldDes[zDDMRecordBKLNK]->offset = (unsigned short)offsetof(zDDMRecord, bklnk);
    prt->papFldDes[zDDMRecordDISP]->size = sizeof(prec->disp);
    prt->papFldDes[zDDMRecordDISP]->offset = (unsigned short)offsetof(zDDMRecord, disp);
    prt->papFldDes[zDDMRecordPROC]->size = sizeof(prec->proc);
    prt->papFldDes[zDDMRecordPROC]->offset = (unsigned short)offsetof(zDDMRecord, proc);
    prt->papFldDes[zDDMRecordSTAT]->size = sizeof(prec->stat);
    prt->papFldDes[zDDMRecordSTAT]->offset = (unsigned short)offsetof(zDDMRecord, stat);
    prt->papFldDes[zDDMRecordSEVR]->size = sizeof(prec->sevr);
    prt->papFldDes[zDDMRecordSEVR]->offset = (unsigned short)offsetof(zDDMRecord, sevr);
    prt->papFldDes[zDDMRecordAMSG]->size = sizeof(prec->amsg);
    prt->papFldDes[zDDMRecordAMSG]->offset = (unsigned short)offsetof(zDDMRecord, amsg);
    prt->papFldDes[zDDMRecordNSTA]->size = sizeof(prec->nsta);
    prt->papFldDes[zDDMRecordNSTA]->offset = (unsigned short)offsetof(zDDMRecord, nsta);
    prt->papFldDes[zDDMRecordNSEV]->size = sizeof(prec->nsev);
    prt->papFldDes[zDDMRecordNSEV]->offset = (unsigned short)offsetof(zDDMRecord, nsev);
    prt->papFldDes[zDDMRecordNAMSG]->size = sizeof(prec->namsg);
    prt->papFldDes[zDDMRecordNAMSG]->offset = (unsigned short)offsetof(zDDMRecord, namsg);
    prt->papFldDes[zDDMRecordACKS]->size = sizeof(prec->acks);
    prt->papFldDes[zDDMRecordACKS]->offset = (unsigned short)offsetof(zDDMRecord, acks);
    prt->papFldDes[zDDMRecordACKT]->size = sizeof(prec->ackt);
    prt->papFldDes[zDDMRecordACKT]->offset = (unsigned short)offsetof(zDDMRecord, ackt);
    prt->papFldDes[zDDMRecordDISS]->size = sizeof(prec->diss);
    prt->papFldDes[zDDMRecordDISS]->offset = (unsigned short)offsetof(zDDMRecord, diss);
    prt->papFldDes[zDDMRecordLCNT]->size = sizeof(prec->lcnt);
    prt->papFldDes[zDDMRecordLCNT]->offset = (unsigned short)offsetof(zDDMRecord, lcnt);
    prt->papFldDes[zDDMRecordPACT]->size = sizeof(prec->pact);
    prt->papFldDes[zDDMRecordPACT]->offset = (unsigned short)offsetof(zDDMRecord, pact);
    prt->papFldDes[zDDMRecordPUTF]->size = sizeof(prec->putf);
    prt->papFldDes[zDDMRecordPUTF]->offset = (unsigned short)offsetof(zDDMRecord, putf);
    prt->papFldDes[zDDMRecordRPRO]->size = sizeof(prec->rpro);
    prt->papFldDes[zDDMRecordRPRO]->offset = (unsigned short)offsetof(zDDMRecord, rpro);
    prt->papFldDes[zDDMRecordASP]->size = sizeof(prec->asp);
    prt->papFldDes[zDDMRecordASP]->offset = (unsigned short)offsetof(zDDMRecord, asp);
    prt->papFldDes[zDDMRecordPPN]->size = sizeof(prec->ppn);
    prt->papFldDes[zDDMRecordPPN]->offset = (unsigned short)offsetof(zDDMRecord, ppn);
    prt->papFldDes[zDDMRecordPPNR]->size = sizeof(prec->ppnr);
    prt->papFldDes[zDDMRecordPPNR]->offset = (unsigned short)offsetof(zDDMRecord, ppnr);
    prt->papFldDes[zDDMRecordSPVT]->size = sizeof(prec->spvt);
    prt->papFldDes[zDDMRecordSPVT]->offset = (unsigned short)offsetof(zDDMRecord, spvt);
    prt->papFldDes[zDDMRecordRSET]->size = sizeof(prec->rset);
    prt->papFldDes[zDDMRecordRSET]->offset = (unsigned short)offsetof(zDDMRecord, rset);
    prt->papFldDes[zDDMRecordDSET]->size = sizeof(prec->dset);
    prt->papFldDes[zDDMRecordDSET]->offset = (unsigned short)offsetof(zDDMRecord, dset);
    prt->papFldDes[zDDMRecordDPVT]->size = sizeof(prec->dpvt);
    prt->papFldDes[zDDMRecordDPVT]->offset = (unsigned short)offsetof(zDDMRecord, dpvt);
    prt->papFldDes[zDDMRecordRDES]->size = sizeof(prec->rdes);
    prt->papFldDes[zDDMRecordRDES]->offset = (unsigned short)offsetof(zDDMRecord, rdes);
    prt->papFldDes[zDDMRecordLSET]->size = sizeof(prec->lset);
    prt->papFldDes[zDDMRecordLSET]->offset = (unsigned short)offsetof(zDDMRecord, lset);
    prt->papFldDes[zDDMRecordPRIO]->size = sizeof(prec->prio);
    prt->papFldDes[zDDMRecordPRIO]->offset = (unsigned short)offsetof(zDDMRecord, prio);
    prt->papFldDes[zDDMRecordTPRO]->size = sizeof(prec->tpro);
    prt->papFldDes[zDDMRecordTPRO]->offset = (unsigned short)offsetof(zDDMRecord, tpro);
    prt->papFldDes[zDDMRecordBKPT]->size = sizeof(prec->bkpt);
    prt->papFldDes[zDDMRecordBKPT]->offset = (unsigned short)offsetof(zDDMRecord, bkpt);
    prt->papFldDes[zDDMRecordUDF]->size = sizeof(prec->udf);
    prt->papFldDes[zDDMRecordUDF]->offset = (unsigned short)offsetof(zDDMRecord, udf);
    prt->papFldDes[zDDMRecordUDFS]->size = sizeof(prec->udfs);
    prt->papFldDes[zDDMRecordUDFS]->offset = (unsigned short)offsetof(zDDMRecord, udfs);
    prt->papFldDes[zDDMRecordTIME]->size = sizeof(prec->time);
    prt->papFldDes[zDDMRecordTIME]->offset = (unsigned short)offsetof(zDDMRecord, time);
    prt->papFldDes[zDDMRecordUTAG]->size = sizeof(prec->utag);
    prt->papFldDes[zDDMRecordUTAG]->offset = (unsigned short)offsetof(zDDMRecord, utag);
    prt->papFldDes[zDDMRecordFLNK]->size = sizeof(prec->flnk);
    prt->papFldDes[zDDMRecordFLNK]->offset = (unsigned short)offsetof(zDDMRecord, flnk);
    prt->papFldDes[zDDMRecordVERS]->size = sizeof(prec->vers);
    prt->papFldDes[zDDMRecordVERS]->offset = (unsigned short)offsetof(zDDMRecord, vers);
    prt->papFldDes[zDDMRecordVAL]->size = sizeof(prec->val);
    prt->papFldDes[zDDMRecordVAL]->offset = (unsigned short)offsetof(zDDMRecord, val);
    prt->papFldDes[zDDMRecordBPTR]->size = sizeof(prec->bptr);
    prt->papFldDes[zDDMRecordBPTR]->offset = (unsigned short)offsetof(zDDMRecord, bptr);
    prt->papFldDes[zDDMRecordNVAL]->size = sizeof(prec->nval);
    prt->papFldDes[zDDMRecordNVAL]->offset = (unsigned short)offsetof(zDDMRecord, nval);
    prt->papFldDes[zDDMRecordNELM]->size = sizeof(prec->nelm);
    prt->papFldDes[zDDMRecordNELM]->offset = (unsigned short)offsetof(zDDMRecord, nelm);
    prt->papFldDes[zDDMRecordNCHIPS]->size = sizeof(prec->nchips);
    prt->papFldDes[zDDMRecordNCHIPS]->offset = (unsigned short)offsetof(zDDMRecord, nchips);
    prt->papFldDes[zDDMRecordFREQ]->size = sizeof(prec->freq);
    prt->papFldDes[zDDMRecordFREQ]->offset = (unsigned short)offsetof(zDDMRecord, freq);
    prt->papFldDes[zDDMRecordCNT]->size = sizeof(prec->cnt);
    prt->papFldDes[zDDMRecordCNT]->offset = (unsigned short)offsetof(zDDMRecord, cnt);
    prt->papFldDes[zDDMRecordPCNT]->size = sizeof(prec->pcnt);
    prt->papFldDes[zDDMRecordPCNT]->offset = (unsigned short)offsetof(zDDMRecord, pcnt);
    prt->papFldDes[zDDMRecordSS]->size = sizeof(prec->ss);
    prt->papFldDes[zDDMRecordSS]->offset = (unsigned short)offsetof(zDDMRecord, ss);
    prt->papFldDes[zDDMRecordUS]->size = sizeof(prec->us);
    prt->papFldDes[zDDMRecordUS]->offset = (unsigned short)offsetof(zDDMRecord, us);
    prt->papFldDes[zDDMRecordCONT]->size = sizeof(prec->cont);
    prt->papFldDes[zDDMRecordCONT]->offset = (unsigned short)offsetof(zDDMRecord, cont);
    prt->papFldDes[zDDMRecordINP]->size = sizeof(prec->inp);
    prt->papFldDes[zDDMRecordINP]->offset = (unsigned short)offsetof(zDDMRecord, inp);
    prt->papFldDes[zDDMRecordCARD]->size = sizeof(prec->card);
    prt->papFldDes[zDDMRecordCARD]->offset = (unsigned short)offsetof(zDDMRecord, card);
    prt->papFldDes[zDDMRecordDLY]->size = sizeof(prec->dly);
    prt->papFldDes[zDDMRecordDLY]->offset = (unsigned short)offsetof(zDDMRecord, dly);
    prt->papFldDes[zDDMRecordDLY1]->size = sizeof(prec->dly1);
    prt->papFldDes[zDDMRecordDLY1]->offset = (unsigned short)offsetof(zDDMRecord, dly1);
    prt->papFldDes[zDDMRecordNCH]->size = sizeof(prec->nch);
    prt->papFldDes[zDDMRecordNCH]->offset = (unsigned short)offsetof(zDDMRecord, nch);
    prt->papFldDes[zDDMRecordG1]->size = sizeof(prec->g1);
    prt->papFldDes[zDDMRecordG1]->offset = (unsigned short)offsetof(zDDMRecord, g1);
    prt->papFldDes[zDDMRecordTP]->size = sizeof(prec->tp);
    prt->papFldDes[zDDMRecordTP]->offset = (unsigned short)offsetof(zDDMRecord, tp);
    prt->papFldDes[zDDMRecordTP1]->size = sizeof(prec->tp1);
    prt->papFldDes[zDDMRecordTP1]->offset = (unsigned short)offsetof(zDDMRecord, tp1);
    prt->papFldDes[zDDMRecordPR1]->size = sizeof(prec->pr1);
    prt->papFldDes[zDDMRecordPR1]->offset = (unsigned short)offsetof(zDDMRecord, pr1);
    prt->papFldDes[zDDMRecordT]->size = sizeof(prec->t);
    prt->papFldDes[zDDMRecordT]->offset = (unsigned short)offsetof(zDDMRecord, t);
    prt->papFldDes[zDDMRecordSDA]->size = sizeof(prec->sda);
    prt->papFldDes[zDDMRecordSDA]->offset = (unsigned short)offsetof(zDDMRecord, sda);
    prt->papFldDes[zDDMRecordSHPT]->size = sizeof(prec->shpt);
    prt->papFldDes[zDDMRecordSHPT]->offset = (unsigned short)offsetof(zDDMRecord, shpt);
    prt->papFldDes[zDDMRecordGAIN]->size = sizeof(prec->gain);
    prt->papFldDes[zDDMRecordGAIN]->offset = (unsigned short)offsetof(zDDMRecord, gain);
    prt->papFldDes[zDDMRecordGLDACEN]->size = sizeof(prec->gldacen);
    prt->papFldDes[zDDMRecordGLDACEN]->offset = (unsigned short)offsetof(zDDMRecord, gldacen);
    prt->papFldDes[zDDMRecordMDAC]->size = sizeof(prec->mdac);
    prt->papFldDes[zDDMRecordMDAC]->offset = (unsigned short)offsetof(zDDMRecord, mdac);
    prt->papFldDes[zDDMRecordAOEN]->size = sizeof(prec->aoen);
    prt->papFldDes[zDDMRecordAOEN]->offset = (unsigned short)offsetof(zDDMRecord, aoen);
    prt->papFldDes[zDDMRecordLOEN]->size = sizeof(prec->loen);
    prt->papFldDes[zDDMRecordLOEN]->offset = (unsigned short)offsetof(zDDMRecord, loen);
    prt->papFldDes[zDDMRecordEBLK]->size = sizeof(prec->eblk);
    prt->papFldDes[zDDMRecordEBLK]->offset = (unsigned short)offsetof(zDDMRecord, eblk);
    prt->papFldDes[zDDMRecordCHEN]->size = sizeof(prec->chen);
    prt->papFldDes[zDDMRecordCHEN]->offset = (unsigned short)offsetof(zDDMRecord, chen);
    prt->papFldDes[zDDMRecordPCHEN]->size = sizeof(prec->pchen);
    prt->papFldDes[zDDMRecordPCHEN]->offset = (unsigned short)offsetof(zDDMRecord, pchen);
    prt->papFldDes[zDDMRecordTSEN]->size = sizeof(prec->tsen);
    prt->papFldDes[zDDMRecordTSEN]->offset = (unsigned short)offsetof(zDDMRecord, tsen);
    prt->papFldDes[zDDMRecordPTSEN]->size = sizeof(prec->ptsen);
    prt->papFldDes[zDDMRecordPTSEN]->offset = (unsigned short)offsetof(zDDMRecord, ptsen);
    prt->papFldDes[zDDMRecordTR1]->size = sizeof(prec->tr1);
    prt->papFldDes[zDDMRecordTR1]->offset = (unsigned short)offsetof(zDDMRecord, tr1);
    prt->papFldDes[zDDMRecordPTR1]->size = sizeof(prec->ptr1);
    prt->papFldDes[zDDMRecordPTR1]->offset = (unsigned short)offsetof(zDDMRecord, ptr1);
    prt->papFldDes[zDDMRecordTR2]->size = sizeof(prec->tr2);
    prt->papFldDes[zDDMRecordTR2]->offset = (unsigned short)offsetof(zDDMRecord, tr2);
    prt->papFldDes[zDDMRecordPTR2]->size = sizeof(prec->ptr2);
    prt->papFldDes[zDDMRecordPTR2]->offset = (unsigned short)offsetof(zDDMRecord, ptr2);
    prt->papFldDes[zDDMRecordTR3]->size = sizeof(prec->tr3);
    prt->papFldDes[zDDMRecordTR3]->offset = (unsigned short)offsetof(zDDMRecord, tr3);
    prt->papFldDes[zDDMRecordPTR3]->size = sizeof(prec->ptr3);
    prt->papFldDes[zDDMRecordPTR3]->offset = (unsigned short)offsetof(zDDMRecord, ptr3);
    prt->papFldDes[zDDMRecordTR4]->size = sizeof(prec->tr4);
    prt->papFldDes[zDDMRecordTR4]->offset = (unsigned short)offsetof(zDDMRecord, tr4);
    prt->papFldDes[zDDMRecordPTR4]->size = sizeof(prec->ptr4);
    prt->papFldDes[zDDMRecordPTR4]->offset = (unsigned short)offsetof(zDDMRecord, ptr4);
    prt->papFldDes[zDDMRecordVL0]->size = sizeof(prec->vl0);
    prt->papFldDes[zDDMRecordVL0]->offset = (unsigned short)offsetof(zDDMRecord, vl0);
    prt->papFldDes[zDDMRecordVL0A]->size = sizeof(prec->vl0a);
    prt->papFldDes[zDDMRecordVL0A]->offset = (unsigned short)offsetof(zDDMRecord, vl0a);
    prt->papFldDes[zDDMRecordPVL0A]->size = sizeof(prec->pvl0a);
    prt->papFldDes[zDDMRecordPVL0A]->offset = (unsigned short)offsetof(zDDMRecord, pvl0a);
    prt->papFldDes[zDDMRecordVL0B]->size = sizeof(prec->vl0b);
    prt->papFldDes[zDDMRecordVL0B]->offset = (unsigned short)offsetof(zDDMRecord, vl0b);
    prt->papFldDes[zDDMRecordPVL0B]->size = sizeof(prec->pvl0b);
    prt->papFldDes[zDDMRecordPVL0B]->offset = (unsigned short)offsetof(zDDMRecord, pvl0b);
    prt->papFldDes[zDDMRecordVL1]->size = sizeof(prec->vl1);
    prt->papFldDes[zDDMRecordVL1]->offset = (unsigned short)offsetof(zDDMRecord, vl1);
    prt->papFldDes[zDDMRecordVL1A]->size = sizeof(prec->vl1a);
    prt->papFldDes[zDDMRecordVL1A]->offset = (unsigned short)offsetof(zDDMRecord, vl1a);
    prt->papFldDes[zDDMRecordPVL1A]->size = sizeof(prec->pvl1a);
    prt->papFldDes[zDDMRecordPVL1A]->offset = (unsigned short)offsetof(zDDMRecord, pvl1a);
    prt->papFldDes[zDDMRecordVL1B]->size = sizeof(prec->vl1b);
    prt->papFldDes[zDDMRecordVL1B]->offset = (unsigned short)offsetof(zDDMRecord, vl1b);
    prt->papFldDes[zDDMRecordPVL1B]->size = sizeof(prec->pvl1b);
    prt->papFldDes[zDDMRecordPVL1B]->offset = (unsigned short)offsetof(zDDMRecord, pvl1b);
    prt->papFldDes[zDDMRecordVH1]->size = sizeof(prec->vh1);
    prt->papFldDes[zDDMRecordVH1]->offset = (unsigned short)offsetof(zDDMRecord, vh1);
    prt->papFldDes[zDDMRecordVH1A]->size = sizeof(prec->vh1a);
    prt->papFldDes[zDDMRecordVH1A]->offset = (unsigned short)offsetof(zDDMRecord, vh1a);
    prt->papFldDes[zDDMRecordPVH1A]->size = sizeof(prec->pvh1a);
    prt->papFldDes[zDDMRecordPVH1A]->offset = (unsigned short)offsetof(zDDMRecord, pvh1a);
    prt->papFldDes[zDDMRecordVH1B]->size = sizeof(prec->vh1b);
    prt->papFldDes[zDDMRecordVH1B]->offset = (unsigned short)offsetof(zDDMRecord, vh1b);
    prt->papFldDes[zDDMRecordPVH1B]->size = sizeof(prec->pvh1b);
    prt->papFldDes[zDDMRecordPVH1B]->offset = (unsigned short)offsetof(zDDMRecord, pvh1b);
    prt->papFldDes[zDDMRecordVL2]->size = sizeof(prec->vl2);
    prt->papFldDes[zDDMRecordVL2]->offset = (unsigned short)offsetof(zDDMRecord, vl2);
    prt->papFldDes[zDDMRecordVL2A]->size = sizeof(prec->vl2a);
    prt->papFldDes[zDDMRecordVL2A]->offset = (unsigned short)offsetof(zDDMRecord, vl2a);
    prt->papFldDes[zDDMRecordPVL2A]->size = sizeof(prec->pvl2a);
    prt->papFldDes[zDDMRecordPVL2A]->offset = (unsigned short)offsetof(zDDMRecord, pvl2a);
    prt->papFldDes[zDDMRecordVL2B]->size = sizeof(prec->vl2b);
    prt->papFldDes[zDDMRecordVL2B]->offset = (unsigned short)offsetof(zDDMRecord, vl2b);
    prt->papFldDes[zDDMRecordPVL2B]->size = sizeof(prec->pvl2b);
    prt->papFldDes[zDDMRecordPVL2B]->offset = (unsigned short)offsetof(zDDMRecord, pvl2b);
    prt->papFldDes[zDDMRecordVH2]->size = sizeof(prec->vh2);
    prt->papFldDes[zDDMRecordVH2]->offset = (unsigned short)offsetof(zDDMRecord, vh2);
    prt->papFldDes[zDDMRecordVH2A]->size = sizeof(prec->vh2a);
    prt->papFldDes[zDDMRecordVH2A]->offset = (unsigned short)offsetof(zDDMRecord, vh2a);
    prt->papFldDes[zDDMRecordPVH2A]->size = sizeof(prec->pvh2a);
    prt->papFldDes[zDDMRecordPVH2A]->offset = (unsigned short)offsetof(zDDMRecord, pvh2a);
    prt->papFldDes[zDDMRecordVH2B]->size = sizeof(prec->vh2b);
    prt->papFldDes[zDDMRecordVH2B]->offset = (unsigned short)offsetof(zDDMRecord, vh2b);
    prt->papFldDes[zDDMRecordPVH2B]->size = sizeof(prec->pvh2b);
    prt->papFldDes[zDDMRecordPVH2B]->offset = (unsigned short)offsetof(zDDMRecord, pvh2b);
    prt->papFldDes[zDDMRecordIVL0]->size = sizeof(prec->ivl0);
    prt->papFldDes[zDDMRecordIVL0]->offset = (unsigned short)offsetof(zDDMRecord, ivl0);
    prt->papFldDes[zDDMRecordPIVL0]->size = sizeof(prec->pivl0);
    prt->papFldDes[zDDMRecordPIVL0]->offset = (unsigned short)offsetof(zDDMRecord, pivl0);
    prt->papFldDes[zDDMRecordIVL1]->size = sizeof(prec->ivl1);
    prt->papFldDes[zDDMRecordIVL1]->offset = (unsigned short)offsetof(zDDMRecord, ivl1);
    prt->papFldDes[zDDMRecordPIVL1]->size = sizeof(prec->pivl1);
    prt->papFldDes[zDDMRecordPIVL1]->offset = (unsigned short)offsetof(zDDMRecord, pivl1);
    prt->papFldDes[zDDMRecordIVH1]->size = sizeof(prec->ivh1);
    prt->papFldDes[zDDMRecordIVH1]->offset = (unsigned short)offsetof(zDDMRecord, ivh1);
    prt->papFldDes[zDDMRecordPIVH1]->size = sizeof(prec->pivh1);
    prt->papFldDes[zDDMRecordPIVH1]->offset = (unsigned short)offsetof(zDDMRecord, pivh1);
    prt->papFldDes[zDDMRecordIVL2]->size = sizeof(prec->ivl2);
    prt->papFldDes[zDDMRecordIVL2]->offset = (unsigned short)offsetof(zDDMRecord, ivl2);
    prt->papFldDes[zDDMRecordPIVL2]->size = sizeof(prec->pivl2);
    prt->papFldDes[zDDMRecordPIVL2]->offset = (unsigned short)offsetof(zDDMRecord, pivl2);
    prt->papFldDes[zDDMRecordIVH2]->size = sizeof(prec->ivh2);
    prt->papFldDes[zDDMRecordIVH2]->offset = (unsigned short)offsetof(zDDMRecord, ivh2);
    prt->papFldDes[zDDMRecordPIVH2]->size = sizeof(prec->pivh2);
    prt->papFldDes[zDDMRecordPIVH2]->offset = (unsigned short)offsetof(zDDMRecord, pivh2);
    prt->papFldDes[zDDMRecordS1]->size = sizeof(prec->s1);
    prt->papFldDes[zDDMRecordS1]->offset = (unsigned short)offsetof(zDDMRecord, s1);
    prt->papFldDes[zDDMRecordPS1]->size = sizeof(prec->ps1);
    prt->papFldDes[zDDMRecordPS1]->offset = (unsigned short)offsetof(zDDMRecord, ps1);
    prt->papFldDes[zDDMRecordS2]->size = sizeof(prec->s2);
    prt->papFldDes[zDDMRecordS2]->offset = (unsigned short)offsetof(zDDMRecord, s2);
    prt->papFldDes[zDDMRecordPS2]->size = sizeof(prec->ps2);
    prt->papFldDes[zDDMRecordPS2]->offset = (unsigned short)offsetof(zDDMRecord, ps2);
    prt->papFldDes[zDDMRecordS3]->size = sizeof(prec->s3);
    prt->papFldDes[zDDMRecordS3]->offset = (unsigned short)offsetof(zDDMRecord, s3);
    prt->papFldDes[zDDMRecordPS3]->size = sizeof(prec->ps3);
    prt->papFldDes[zDDMRecordPS3]->offset = (unsigned short)offsetof(zDDMRecord, ps3);
    prt->papFldDes[zDDMRecordNM1]->size = sizeof(prec->nm1);
    prt->papFldDes[zDDMRecordNM1]->offset = (unsigned short)offsetof(zDDMRecord, nm1);
    prt->papFldDes[zDDMRecordNM2]->size = sizeof(prec->nm2);
    prt->papFldDes[zDDMRecordNM2]->offset = (unsigned short)offsetof(zDDMRecord, nm2);
    prt->papFldDes[zDDMRecordNM3]->size = sizeof(prec->nm3);
    prt->papFldDes[zDDMRecordNM3]->offset = (unsigned short)offsetof(zDDMRecord, nm3);
    prt->papFldDes[zDDMRecordEGU]->size = sizeof(prec->egu);
    prt->papFldDes[zDDMRecordEGU]->offset = (unsigned short)offsetof(zDDMRecord, egu);
    prt->papFldDes[zDDMRecordRUNNO]->size = sizeof(prec->runno);
    prt->papFldDes[zDDMRecordRUNNO]->offset = (unsigned short)offsetof(zDDMRecord, runno);
    prt->papFldDes[zDDMRecordRPVT]->size = sizeof(prec->rpvt);
    prt->papFldDes[zDDMRecordRPVT]->offset = (unsigned short)offsetof(zDDMRecord, rpvt);
    prt->papFldDes[zDDMRecordPREC]->size = sizeof(prec->prec);
    prt->papFldDes[zDDMRecordPREC]->offset = (unsigned short)offsetof(zDDMRecord, prec);
    prt->papFldDes[zDDMRecordCOUT]->size = sizeof(prec->cout);
    prt->papFldDes[zDDMRecordCOUT]->offset = (unsigned short)offsetof(zDDMRecord, cout);
    prt->papFldDes[zDDMRecordCOUTP]->size = sizeof(prec->coutp);
    prt->papFldDes[zDDMRecordCOUTP]->offset = (unsigned short)offsetof(zDDMRecord, coutp);
    prt->rec_size = sizeof(*prec);
    return 0;
}
epicsExportRegistrar(zDDMRecordSizeOffset);

#ifdef __cplusplus
}
#endif
#endif /* GEN_SIZE_OFFSET */

#endif /* INC_zDDMRecord_H */
