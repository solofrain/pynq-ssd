#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

#define NUM_HERMES 12
#define NUM_SCEPTER 12

#define TOKENREG 4 
#define SPIREG 5 
#define HERMESCALREG 6
#define SCEPTERENBREG 7



struct hermes_cnfg {
    int sda;             // internal DAC enable
    int sdaoan;          // 10-bit DAC monitor to OAN
    int eblk;            // internal bias leakage enable
    int peaktime;        // peaking time (00=1us,01=.5us,10=4us,11=2us)
    int gain;            // gain (0=1500mV/fC, 1=750mV/fc)
    int ch_ech[32];      // channel enable
    int ch_ecal[32];     // calibration input enable
    int ch_elk[32];      // leakage current monitor enable
    int ch_ean[32];      // analog output enable
    int ch_vl1[32];      // threshold trim (6 bits)
    int ch_vh1[32];      // threshold trim (6 bits)
    int ch_vl2[32];      // threshold trim (6 bits)
    int ch_vh2[32];      // threshold trim (6 bits)

}; 


struct scepter_cnfg {
    int tcm;	         // comparator mult-fire suppress (00=0s,01=100ns,10=1us,11=2us)
    int filtena;	 // 1=enable additional filtering
    int genspibit;	 // 7 bits general purpose spi pins 
    int tos;		 // timeout slope (000-111=9.3,4.6,2.3,1.2,0.6,0.45,0.3,0.15 us)
    int trke;		 // 1= enable enhanced simultaneous event mode
    int trk;		 // 1= enable simultaneous event mode
    int tds;		 // Time detect TAC slope select (000-111=19.4,9.8,4.9,2.5,1.25,0.83,0.63,0.31 us)
    int tdm;		 // TAC mode (00=ToO,01=risetime,10=falltime,11=ToT)
    int tria;            // 1=PDE/PDF driven on event,0=always driven (if 1,tri1 must be 0)
    int tri2;            // Valid tristate control (1=tristate,0=driven)
    int tri1;		 // 1=PDE/PDF tristate on RR, 0=always driven
    int aux;             // Baseline/lock output port, 1=PDOUT, 0=AAOUT
    int sth;             // disables internal threshold DAC for external bypass
    int thpd;		 // PD reference trim. 0000=-150mV,1111=0mv, step=-10mV
    int lock;		 // Baseline/lock mode select, 1=lock mode
    int vd;	         // 10-bit threshold DAC (2V full scale, 1.95mV step, 0mV baseline
    int ch_mask[32];     // channel mask
    int ch_thtrim[32];  //threshold trim
};


volatile unsigned int *fpgabase;  //mmap'd fpga registers


/*****************************************
* Enable Scepter  (SCEN)
*     bit 0  = scen 
******************************************/
void enable_scepter()
{
   fpgabase[SCEPTERENBREG] = 1;
}



/*****************************************
* Set Token  (TDA)
*  Token Register
*     bit 0 = tda
*     bit 1 = clock
******************************************/
void set_token()
{
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
   int i, tda;

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


}



/*****************************************
* Send out an SPI bit
* SPI Register 
*   bit 0 = data
*   bit 1 = clk
******************************************/
void send_spi_bit(int val)
{

   int sda;
 
   sda = val & 0x1;
   //printf("%d ",(val & 0x1));
  
   //set sclk low  
   fpgabase[SPIREG] = 0;
   //set data with clock low
   fpgabase[SPIREG] = sda;
   //set clk high 
   fpgabase[SPIREG] =  0x2 | sda;
   //set clk low
   fpgabase[SPIREG] =  sda;
   //set data low
   fpgabase[SPIREG] = 0;

}


/******************************************
* Program DAC 
*    Addr 0 => ChanA : E4 testpoint
*    Addr 1 => ChanB : E5 testpoint
*    Addr 2 => ChanC : E6 testpoint
*    Addr 3 => ChanD : E7 testpoint
*    Addr 4 => ChanE : Offset-PD
*    Addr 5 => ChanF : Guard-Bias
*    Addr 6 => ChanG : Offset-TD
*    Addr 7 => ChanH : Cal-DAC
* 
*  SDI stream is 24 bits (4 bit command, 4 bit address, 16 bit data)
*  Command use 0011, Addr and Data is MSB first
*
******************************************/
int prog_dac(int addr, int value)
{

   int i;

   //send command bits 
   for (i=3;i>=0;i--)  send_spi_bit(0x3 >> i);
   //send address bits
   for (i=3;i>=0;i--)  send_spi_bit(addr >> i);
   //send data bits 
   for (i=15;i>=0;i--)  send_spi_bit(value >> i);

}


/******************************************
* Program Analog Mux (ADG1438) 
*   Two parts wired in a chain, enable only 1 channel
*   Output goes to diag_out lemo connector
*   Value Input: 
*    0  => +3v ref 
*    1  => +4v (voltage divided by 2)
*    2 => +5v (voltage divided by 2)
*    3 => -5v (voltage divided by 2)
*    4 => AC coupled HV
*    5 => N.C. (testpoint E3)
*    6 => -6v (voltage divided by 3)
*    7  => +6v (voltage divided by 3)
*    8  => VDD-HA-AB
*    9  => VDD-HA-CD
*    10  => VDD-HP
*    11  => VDD-SA-AB
*    12  => VDD-SA-CD
*    13  => VDD-SD
*    14  => VSS
*    15  => VSSD

*  SDI Stream is 16 bits
*
******************************************/
int prog_anaswitch(int value)
{

   int i, swval;

   
   switch(value) {
      case 0 : swval = 1; break;
      case 1 : swval = 2; break;
      case 2 : swval = 4; break;
      case 3 : swval = 8; break;
      case 4 : swval = 16; break;
      case 5 : swval = 32; break;
      case 6 : swval = 64; break;
      case 7 : swval = 128; break;
      case 8 : swval = 256; break;
      case 9 : swval = 512; break;
      case 10 : swval = 1024; break;
      case 11 : swval = 2048; break;
      case 12 : swval = 4096; break;
      case 13 : swval = 8192; break;
      case 14 : swval = 16384; break;
      case 15 : swval = 32768; break;
      default : swval = 0;
    }
   //swval = pow(2,value); 
   //send command bits 
   for (i=15;i>=0;i--)  send_spi_bit(swval >> i);

}


/******************************************
* Load Config Bits into Hermes ASIC
*
* 904 serial bits.  First bit is D5/VH2 of Ch31
* 
******************************************/
int load_hermes(struct hermes_cnfg *hermes)
{

    int i,j,k; 

   for (k=0;k<100;k++); 
   // send channel bits (896 bits total)
   //printf("Channel Bits...\n"); 
   for (i=31;i>=0;i--) {
       for (j=5;j>=0;j--)  send_spi_bit(hermes->ch_vh2[i] >> j);
       for (j=5;j>=0;j--)  send_spi_bit(hermes->ch_vl2[i] >> j);
       for (j=5;j>=0;j--)  send_spi_bit(hermes->ch_vh1[i] >> j);
       for (j=5;j>=0;j--)  send_spi_bit(hermes->ch_vl1[i] >> j);
       send_spi_bit(hermes->ch_ean[i]);
       send_spi_bit(hermes->ch_elk[i]);
       send_spi_bit(hermes->ch_ecal[i]);
       send_spi_bit(hermes->ch_ech[i]);       
       for (k=0;k<100;k++);  
   //   printf("\n");
   }
   //printf("Global Bits...\n");
   send_spi_bit(hermes->gain);
   for (j=1;j>=0;j--) send_spi_bit(hermes->peaktime >> j);
   send_spi_bit(hermes->eblk);
   for (j=2;j>=0;j--) send_spi_bit(hermes->sdaoan >> j);
   send_spi_bit(hermes->sda);   
   //printf("\n");
   for (k=0;k<100;k++);

}



/*******************************************
* Print Hermes Configuration 
* values from hermes_cnfg structure
*
*******************************************/
void dump_hermescnfg(struct hermes_cnfg *hermes)
{
   int i;

   printf("Hermes Global Bits\n");
   printf("SDA=%d\t SDAOEN=%d\t EBLK=%d\t PeakTime=%d\t Gain=%d\n",
             hermes->sda, hermes->sdaoan, hermes->eblk, hermes->peaktime, hermes->gain);

   printf("Hermes Channel Bits\n");
   for (i=0;i<32;i++) {
      printf("Ch# %d\t ECH=%d\t ECAL=%d\t ELK=%d\t EAN=%d\t VL1=%d\t VH1=%d\t VL2=%d\t VH2=%d\n",
             i, hermes->ch_ech[i], hermes->ch_ecal[i], hermes->ch_elk[i], hermes->ch_ean[i],
                hermes->ch_vl1[i], hermes->ch_vh1[i], hermes->ch_vl2[i], hermes->ch_vh2[i]);   
   }
   printf("\n");
}


/*******************************************
* Read Hermes Configuration File and store
* values into hermes_cnfg structure
*
*
*******************************************/
int read_hermescnfgfile(struct hermes_cnfg *hermes)
{
    FILE *f;
    char line[100];
    char c[2];
    int chnum, ech, ecal, elk, ean, vl1, vh1, vl2, vh2; 
    int sda, sdaoan, eblk, peaktime, gain;


    f = fopen("hermes1.txt","r");
    if (f == NULL) {
       printf("File not Found\n");
       exit(1);
    }

   while (fgets(line, 80, f) != NULL) {
     //printf("%s",line);
     if (line[0] == 'G') { //global bits
       //printf("%s",line);
       sscanf(line,"%s %d %d %d %d %d",c, &sda, &sdaoan, &eblk, &peaktime, &gain);
       hermes->sda = sda;
       hermes->sdaoan = sdaoan;
       hermes->eblk = eblk;
       hermes->peaktime = peaktime;
       hermes->gain = gain;
     }


     if (line[0] == 'C') { //channel bits
       //printf("%s",line);
       sscanf(line, "%s %d %d %d %d %d %d %d %d %d", 
                    c,&chnum,&ech,&ecal,&elk,&ean,&vl1,&vh1,&vl2,&vh2); 
       //printf("Read Channel %d\n",chnum); 
       hermes->ch_ech[chnum] = ech;
       hermes->ch_ecal[chnum] = ecal; 
       hermes->ch_elk[chnum] = elk;
       hermes->ch_ean[chnum] = ean;
       hermes->ch_vl1[chnum] = vl1;
       hermes->ch_vh1[chnum] = vh1;
       hermes->ch_vl2[chnum] = vl2;
       hermes->ch_vh2[chnum] = vh2;


      }
   }

   close(f);
}

/*******************************************
* Print Scepter Configuration 
* values from scepter_cnfg structure
*
*******************************************/
void dump_sceptercnfg(struct scepter_cnfg *scepter)
{
   int i;

   printf("Scepter Global Bits\n");
   printf("TCM=%d\t FILTENB=%d\t GENSPIBITS=%d\t TOS=%d\t TRKE=%d\t TRK=%d\t TDS=%d\t TDM=%d\n",
             scepter->tcm, scepter->filtena, scepter->genspibit, scepter->tos, scepter->trke, 
             scepter->trk, scepter->tds, scepter->tdm);
   printf("TRIA=%d\t TRI2=%d\t TRI1=%d\t AUX=%d\t STH=%d\t TPHD=%d\t LOCK=%d\t VD=%d\n",
             scepter->tria, scepter->tri2, scepter->tri1, scepter->aux, scepter->sth, scepter->thpd, 
             scepter->lock, scepter->vd);


   printf("Scepter Channel Bits\n");
   for (i=0;i<32;i++) {
      printf("Ch# %d\t MASK=%d\t THRESHTRIM=%d\n",
             i, scepter->ch_mask[i], scepter->ch_thtrim[i]);
   }
   printf("\n");
}







/*******************************************
* Read Scepter Configuration File and store
* values into scepter_cnfg structure
*
*
*******************************************/
int read_sceptercnfgfile(struct scepter_cnfg *scepter)
{
    FILE *f;
    char line[100];
    char c[2];
    int chnum;
    int tcm, filtena, genspibit, tos, trke, trk, tds, tdm, tria, tri2, tri1;
    int aux, sth, thpd, lock, vd, chmask, thtrim;

    f = fopen("scepter1.txt","r");
    if (f == NULL) {
       printf("File not Found\n");
       exit(1);
    }

   while (fgets(line, 80, f) != NULL) {
     //printf("%s",line);
     if (line[0] == 'G') { //global bits
       //printf("%s",line);
       sscanf(line,"%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		    c, &tcm, &filtena, &genspibit, &tos, &trke, &trk, &tds, 
                    &tdm, &tria, &tri2, &tri1, &aux, &sth, &thpd, &lock, &vd);
       scepter->tcm = tcm;
       scepter->filtena = filtena;
       scepter->genspibit = genspibit;
       scepter->tos = tos;
       scepter->trke = trke;
       scepter->trk = trk;
       scepter->tds = tds;
       scepter->tdm = tdm;
       scepter->tria = tria;
       scepter->tri2 = tri2;
       scepter->tri1 = tri1;
       scepter->aux = aux;
       scepter->sth = sth;
       scepter->thpd = thpd;
       scepter->lock = lock;
       scepter->vd = vd;
     }


     if (line[0] == 'C') { //channel bits
       //printf("%s",line);
       sscanf(line, "%s %d %d %d", 
                    c,&chnum,&chmask,&thtrim); 
       //printf("Read Channel %d\n",chnum); 
       scepter->ch_mask[chnum] = chmask;
       scepter->ch_thtrim[chnum] = thtrim;

      }
   }

   close(f);
}





/*******************************************
* mmap fpga register space
* returns pointer fpgabase
*
********************************************/
void mmap_fpga()
{
   int fd;


   fd = open("/dev/mem",O_RDWR|O_SYNC);
   if (fd < 0) {
      printf("Can't open /dev/mem\n");
      exit(1);
   }

   fpgabase = (unsigned int *) mmap(0,255,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x43C00000);

   if (fpgabase == NULL) {
      printf("Can't map FPGA space\n");
      exit(1);
   }

}



/*******************************************
* Main 
* 
*
*
*******************************************/
main()
{
   struct hermes_cnfg hermes1;
   struct scepter_cnfg scepter1;
   int i, fd;


   mmap_fpga();
 
   printf("Reading Hermes Config File\n");
   read_hermescnfgfile(&hermes1);
   dump_hermescnfg(&hermes1);


   printf("Reading Scepter Config File\n");
   read_sceptercnfgfile(&scepter1);
   dump_sceptercnfg(&scepter1);
   

   printf("Loading Hermes SPI\n");

   
   //Enable Scepter ASICS
   enable_scepter();


   //Program SPI stream
   set_token();
   
   for (i=0;i<NUM_HERMES;i++) {
      //load_hermes(&hermes1);
      clock_token();
      clear_token();  //only necessary after 1st chip load
   }

   for (i=0;i<NUM_SCEPTER;i++) {
      //load_scepter(&scepter1);
      clock_token();
   }      

   // load 8 channel DAC
   prog_dac(6,0x5AAA);  //testpoint E3
   //prog_dac(1,20000);  //testpoint E4

   clock_token();

   //program analog switch
   prog_anaswitch(0);
   clock_token();

   for (i=0;i<20;i++)
      clock_token();
}
