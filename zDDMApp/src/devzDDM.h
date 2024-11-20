/* ----------------Device Support Entry Table for devzDDM----------------- */

#define MAX_CHANNELS 640
#define MAX_NCHIPS 20

typedef struct{
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	reset;
	DEVSUPFUN	read;
	DEVSUPFUN	write_preset;
	DEVSUPFUN	arm;
	DEVSUPFUN	done;
} det_DSET;

/*** det_state ***/

typedef struct{
	int card_exists;
	int num_channels;
	int card_in_use;
	int count_in_progress; /* count in progress? */
	int newdat;
	int tp;
	zDDMRecord *psr;
	unsigned short ident; /* identification info for this card */
	IOSCANPVT ioscanpvt;
	int done; 	/* sequence counter for ISR: 
			0=first entry, set up one-shot for time
			1=timer one-shot done
			2=count sequence finished   */
	int newdata; /* flag to tell hardware new data is available */
	CALLBACK *pcallback;
}det_state;


/*** callback stuff ***/
struct callback {
	CALLBACK	callback;
	struct dbCommon *precord;
        epicsTimerId wd_id;
};

typedef struct zDDMDpvt { /* unique for each record */
  int  reg;   /* index of sub-device (determined by signal #*/
  int  nbit;  /* no of significant bits */
  int  type;  /* Type either 0 or 1 for uni, bi polar */
  unsigned long  value; /* to provide readback for ao records */
}zDDMDpvt;

typedef struct {
        int exists;
        int done;
        int newdat;
        zDDMRecord *psr;
        CALLBACK *pcallback;
} devPVT;

typedef struct rpvtStruct {
        epicsMutexId updateMutex;
        CALLBACK *pcallbacks;
} rpvtStruct;


/*epicsMutexId  SPI_lock;*/       /* SPI is shared resource; lock accesses */

