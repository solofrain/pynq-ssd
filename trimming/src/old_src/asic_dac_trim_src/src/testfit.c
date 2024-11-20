#include <stdio.h>
#include <math.h>
#include <minpack.h>
#include <tsDefs.h>
#include <cadef.h>
#include <ezca.h>
#include <grace_np.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define DEBUG 1

int NCH=384; /* default value. Real value got from device later. */

int NPOINTS=32;
float STEP=0.01;
int NPLT=640;
long data[4][640][64]; /*2 registers x 2 thresholds, up to 640 channels, up to 64-point scans */
double fits[4][640][4]; /*2 registers x 2 thresholds, up to 640 channels, 4 parameters */
double x[64];
double p0[4];
char v1_l[640], v1_h[640], v2_l[640], v2_h[640], dis[640];
float factor[4][640];
double mx=0.0;
double mn=0.0;
float dishi=3.0;
float dislo=0.6;
float cnt_time=0.50;
float guess=1.3;
double fitdat[64],fitud;
char Fname[40];

int m, n, info, lwa, iwa[4], one=1;
double tol, fnorm, fvec[64], wa[512];

void lmdif1_ ( void (*fcn)(int *m, int *n, double *x, double *fvec, int
                      *iflag),
         int *m, int * n, double *x, double *fvec,
         double *tol, int *info, int *iwa, double *wa, int *lwa);

void FitGraceinit(){
/* set up to plot edge position vs edge width for all channels, different color
 for upper and lower thresholds and sca1 and sca2.
*/

int j;
char str1[256];

  if(GraceIsOpen()){
        GraceClose();
	}
  GraceOpen(32768);
  GracePrintf("page size 500,400");
  GracePrintf("redraw");
  GracePrintf("g0 on");
  GracePrintf("focus g0");
  GracePrintf("xaxis ticklabel char size 0.6");
  GracePrintf("yaxis ticklabel char size 0.6");
  for(j=0;j<4;j++){
       sprintf(str1,"s%i on",j);
       GracePrintf(str1);
       sprintf(str1,"s%i symbol %i",j,j+1);
       GracePrintf(str1);
       sprintf(str1,"s%i line type 0",j);
       GracePrintf(str1);
       }
#ifdef DEBUG
	printf("leaving FitGraceinit()\n");
#endif
}

void ScanGraceinit(){
/* set up to plot data for all channels, different color
 for upper and lower thresholds and sca1 and sca2.
*/

int i,j;
char str1[256];
#ifdef DEBUG
	printf("entering ScanGraceinit()\n");
#endif

  if (GraceIsOpen()) GraceClose();
  GraceOpen(32768);
  GracePrintf("page size 450,350");
  GracePrintf("arrange(2,2,0.1,0.1,0.1)");
  GracePrintf("redraw");
  for(j=0;j<4;j++){
	sprintf(str1,"g%i on",j);
	GracePrintf(str1);
	sprintf(str1,"focus g%i",j);
	GracePrintf(str1);
	GracePrintf("xaxis ticklabel char size 0.6");
	GracePrintf("yaxis ticklabel char size 0.6");
	for(i=0;i<NPLT;i++){
		sprintf(str1,"g%i.s%i symbol %i",j,i,j+1);
		GracePrintf(str1);
		}
	}
#ifdef DEBUG
	printf("leaving ScanGraceinit()\n");
#endif

}


int makex(){
int i;
	for(i=0;i<NPOINTS;i++){
		x[i]=guess+(i-NPOINTS/2)*STEP;
#ifdef DEBUG
		printf("x[%i]=%g\n",i,x[i]);
#endif

		}
#ifdef DEBUG
	printf("leaving makex()\n");
#endif
	
}

void count(){
	ezcaPut("det1.TP",ezcaFloat,1,&cnt_time);
	ezcaPut("det1.CNT",ezcaString,1,"Count");
	ezcaSetMonitor("det1.CNT",ezcaString);
	while(ezcaNewMonitorValue("det1.CNT",ezcaString)==0){}		 
#ifdef DEBUG
	printf("leaving Count(%g)\n",cnt_time);
#endif
	ezcaClearMonitor("det1.CNT",ezcaString);
}


void scan_levels(){
int ud,i,j,res,leng,len2;
long y[640]; /* max no. of channels */
char str1[256];
float x_i;
int datfile;

#ifdef DEBUG
	printf("entering scan_levels()\n");
#endif
    
    makex();
    ScanGraceinit();
    ud=1;
    ezcaPut("peter:ao2",ezcaFloat,1,&dishi);
    ezcaPut("peter:ao4",ezcaFloat,1,&dishi);
#ifdef DEBUG
	printf("put dishi\n");
#endif

    for(i=0;i<NPOINTS;i++){
    	x_i=(float)x[i];
	ezcaPut("peter:ao1",ezcaFloat,1,&x_i);
	ezcaPut("peter:ao3",ezcaFloat,1,&x_i);
#ifdef DEBUG
	printf("about to count %g at %g...\n",cnt_time, x[i]);
#endif
	count();			

#ifdef DEBUG
	printf("getting NCH\n");
#endif

	res=ezcaGet("det1.NCH",ezcaShort,1,&NCH);
	if(res!=0) printf("get NCH: CA error %i",res);

#ifdef DEBUG
	printf("NCH= %i\n",NCH);
#endif

	NPLT=NCH;

#ifdef DEBUG
	printf("getting data\n");
#endif
	res=ezcaGet("det1.S2",ezcaLong,NCH,y);
	if(res!=0) printf("Get error %i",res);

	for(j=0;j<NCH;j++){
		data[0][j][i]=y[j];
/*		printf(" %i",data[0][j][i]);*/
		}
/*	printf("\n");*/
	res=ezcaGet("det1.S3",ezcaLong,NCH,y);
	if(res!=0) printf("Get error %i",res);
	for(j=0;j<NCH;j++){
		data[2][j][i]=y[j];
/*		printf(" %i",data[2][j][i]);*/
		}
/*	printf("\n");*/

#ifdef DEBUG
	printf("Plotting point %i..\n",i);
#endif
	GracePrintf("focus g0");
	for(j=0;j<NPLT;j++){
		sprintf(str1,"g0.s%i point %g, %i",j,x[i],data[0][j][i]);
/*		printf("%s\n",str1);*/
		GracePrintf(str1);
		}
	GracePrintf("autoscale");
	GracePrintf("redraw");

	GracePrintf("focus g2");
	for(j=0;j<NPLT;j++){
		sprintf(str1,"g2.s%i point %g, %i",j,x[i],data[2][j][i]);
/*		printf("%s\n",str1);*/
		GracePrintf(str1);
		}
	GracePrintf("autoscale");
	GracePrintf("redraw");
	}
    GraceFlush();
    ud=-1;
#ifdef DEBUG
	printf("Putting dislo\n");
#endif
    ezcaPut("peter:ao1",ezcaFloat,1,&dislo);
    ezcaPut("peter:ao3",ezcaFloat,1,&dislo);
    for(i=0;i<NPOINTS;i++){
    	x_i=(float)x[i];
	ezcaPut("peter:ao2",ezcaFloat,1,&x_i);
	ezcaPut("peter:ao4",ezcaFloat,1,&x_i);
#ifdef DEBUG
	printf("about to count %g at %g\n",cnt_time, x[i]);
#endif
	count();
#ifdef DEBUG
	printf("getting data\n");
#endif
	res=ezcaGet("det1.S2",ezcaLong,NCH,y);
	if(res!=0) printf("Get error %i",res);
	for(j=0;j<NCH;j++){
		data[1][j][i]=y[j];
		}
	res=ezcaGet("det1.S3",ezcaLong,NCH,y);
	if(res!=0) printf("Get error %i",res);
	for(j=0;j<NCH;j++){
		data[3][j][i]=y[j];
		}
#ifdef DEBUG
	printf("plotting point %i\n",i);
#endif
	GracePrintf("focus g1");
	for(j=0;j<NPLT;j++){
		sprintf(str1,"g1.s%i point %g, %i",j,x[i],data[1][j][i]);
		GracePrintf(str1);
		}
	GracePrintf("autoscale");
	GracePrintf("redraw");

	GracePrintf("focus g3");
	for(j=0;j<NPLT;j++){
		sprintf(str1,"g3.s%i point %g, %i",j,x[i],data[3][j][i]);
		GracePrintf(str1);
		}
	GracePrintf("autoscale");
	GracePrintf("redraw");

	}
	GraceFlush();
/*	for(i=0;i<NPLT;i++){
	  for(j=0;j<NPOINTS;j++){
	  	printf(" %i",data[0][i][j]);
		}
		printf("\n");
	} */
	datfile=open("scan_data.dat",O_WRONLY|O_TRUNC);
	printf("%s\n",strerror(errno));
	if((datfile != 0) | (datfile != -1)){
	  leng=4*NCH*64*sizeof(long);
	  printf("File size: %i\n", leng);
	  len2=write(datfile,data,leng);
	  printf("%s\n",strerror(errno));
	  if(len2 != leng) printf("Write error\n");
	  }
	else{
	  printf("File error!\n");
	  }
	  close(datfile);
}

 
		
int save_fits(void){

FILE *dfile;
int i,j;

	dfile=fopen("scan_fits.dat","w");
	printf("%s\n",strerror(errno));
	if(dfile != NULL){
	   for(j=0;j<NCH;j++){
	     fprintf(dfile,"%i  ",j);
	     for(i=0;i<4;i++){
		fprintf(dfile,"%g %g %g %g  ",fits[i][j][0],fits[i][j][1],fits[i][j][2],fits[i][j][3]); 
		}
	     fprintf(dfile,"\n");
	   }
	}
	else{
	  printf("File error!\n");
	}
	fclose(dfile);
	return(0);
}

int save_dacs(void){

FILE *dfile;
int i,j;

	dfile=fopen("dacs.dat","w");
	printf("save-dacs: %s\n",strerror(errno));
	if(dfile != NULL){
	   for(j=0;j<NCH;j++){
		fprintf(dfile,"%i  %i %i %i %i\n",j,v1_l[j],v1_h[j],v2_l[j],v2_h[j]);
	   }
	}
	else{
	  printf("File error!\n");
	}
	fclose(dfile);
	return(0);
}

void Restore_dacs(void){

FILE *dfile;
int i,j,k;

	dfile=fopen("dacs.dat","r");
	printf("%s\n",strerror(errno));
	if(dfile != NULL){
	   for(j=0;j<NCH;j++){
		fscanf(dfile,"%i  %i %i %i %i\n",&k,&v1_l[j],&v1_h[j],&v2_l[j],&v2_h[j]);
	   }
	}
	else{
	  printf("File error!\n");
	}
	fclose(dfile);
	ezcaPut("det1.TR1",ezcaByte,NCH,v1_l);
	ezcaPut("det1.TR2",ezcaByte,NCH,v1_h);
	ezcaPut("det1.TR3",ezcaByte,NCH,v2_l);
	ezcaPut("det1.TR4",ezcaByte,NCH,v2_h);
}

void fitall(){
double p0[4],p[4],half,width;
int i,j,k,ud,info;
char str1[256];

	FitGraceinit();
	half=0.0;
	GracePrintf("focus g0");
	
	for(j=0;j<4;j++){
		for(k=0;k<NCH;k++){
			mn=10000000;
			mx=0;
			for(i=0;i<NPOINTS;i++){
				fitdat[i]=data[j][k][i];
				if(fitdat[i]>=mx) mx=fitdat[i];
				if(fitdat[i]<=mn) mn=fitdat[i];
				}
			half=(mn+mx)/2.0;
			if((j==0) || (j==2)){
			  fitud=1;
			  for(i=0;i<NPOINTS-1;i++){
                        	if((fitdat[i]>=half) && (fitdat[i+1]<=half)){
                                	/*printf("LoMid = %i\n",i);*/
                                	p0[1]=x[i]; /* center */
                                	p0[2]=60.0; /* width (V) */
					p0[0]=(mx-mn)/2;/* height */
					p0[3]=mn;   /* baseline */
					}
				}
			  }

			if((j==1) || (j==3)){
			  fitud=-1;
                  	  for(i=0;i<NPOINTS-1;i++){
                        	if((fitdat[i]<=half) && (fitdat[i+1]>=half)){
                                	/*printf("LoMid = %i\n",i);*/
                                	p0[1]=x[i]; /* center */
                                	p0[2]=60.0; /* width (V) */
					p0[0]=(mx-mn)/2;/* height */
					p0[3]=mn;   /* baseline */
					}
				}
			  }

			printf("Before: p0=%g p1=%g p2=%g p3=%g\n",p0[0],p0[1],p0[2],p0[3]);
			fit(p0,&info);
			printf("After: p0=%g p1=%g p2=%g p3=%g info=%i\n",p0[0],p0[1],p0[2],p0[3],info);
			for(i=0;i<4;i++) fits[j][k][i]=p0[i];
                        /*printf("Center=%g  width=%g  max=%g  min=%g  info=%i\n",fits[j][k][1],fits[j][k][2],fits[j][k][0]+fits[j][k][3],info);*/
                        sprintf(str1,"g0.s%i point %g, %g",j,fits[j][k][1],fits[j][k][2]);
			GracePrintf(str1);
			}
	GracePrintf("autoscale");
	GracePrintf("redraw");
	}
	if(save_fits()) printf("save fits error\n");
}


void std_dev(void){
double sigma[4],mean[4],sum[4];
int i,j,k,n;

for(i=0;i<4;i++){
	sum[i]=0;
	n=0;
        for(j=0;j<NCH;j++){
	  if(dis[j]==0){
		n+=1;
		sum[i]+=fits[i][j][1];
		}
        }
	mean[i]=sum[i]/n;
  }

for(i=0;i<4;i++){
	sum[i]=0;
	n=0;
        for(j=0;j<NCH;j++){
	  if(dis[j]==0){
		n+=1;
		sum[i]+=(fits[i][j][1]-mean[i])*(fits[i][j][1]-mean[i]);
		}
        }
	sigma[i]=sqrt(sum[i]/(n-1));
	printf("i=%i  Mean=%g  Std.Dev.=%g\n",i,mean[i],sigma[i]);
  }
printf("Disabled channels: %i\n",NCH-n);
}


void fit_plot(double upres,double lowres,double upcen,double lowcen){
double p0[4],p[4],half,width,max;
int i,j,k,ud,info;
char str1[256];

	FitGraceinit(); 
	half=0.0;
	GracePrintf("focus g0");
	for(i=0;i<NCH;i++) dis[i]=0;
	for(j=0;j<4;j++){
		for(k=0;k<NCH;k++){
		  max=fits[j][k][3]+2.0*fits[j][k][0];
		  if ((fits[j][k][1]>=lowcen)&&(fits[j][k][1]<=upcen)&&(fits[j][k][2]>=lowres)&&(fits[j][k][2]<=upres)){
			printf("In: k=%i  Center=%lg  width=%lg  min=%lg  max=%lg\n",k,fits[j][k][1],fits[j][k][2],max);
			sprintf(str1,"g0.s0 point %lg, %lg",fits[j][k][1],fits[j][k][2]);
			GracePrintf(str1);
			}
		  else {
		  	dis[k]=1;
			printf("Out:k=%i Center=%lg  width=%lg  max=%lg  min=%lg\n",k,fits[j][k][1],fits[j][k][2],max);
		  	sprintf(str1,"g0.s1 point %lg, %lg",fits[j][k][1],fits[j][k][2]);
			GracePrintf(str1);
			}
		  }
	GracePrintf("autoscale");
	GracePrintf("redraw");
	}
printf("%lg  %lg %lg %lg\n",upcen, lowcen,upres,lowres);
std_dev();
}


double peval(double *x, double *p, int ud){
int i;
double res;
	res=0;
	for(i=0;i<NPOINTS;i++){
        	res+=p[0]*(1+erf(p[2]*ud*(p[1]-x[i])))+p[3];
	}
	return res;
}


double residuals(double *p, double *y, double *x, int ud){
int i;
double err;
	err=0;
        for(i=0;i<NPOINTS;i++){
		err += y[i]-(p[0]*(1+erf(p[2]*ud*(p[1]-x[i])))+p[3]);
		}
         return err;
	 }

void fcn(int m, int n, double *p, double *fvec, int *iflag)
{
  /* function fcn for lmdif1 example */

  int i;
  double tmp1,tmp2,tmp3;

  for (i=0; i<NPOINTS; i++)
    {
      fvec[i] = fitdat[i]-(p[0]*(1+erf(p[2]*fitud*(p[1]-x[i])))+p[3]);
    }
}

int fit( double *p0, int *info){

  int m, n, lwa, iwa[4], one=1;
  double tol, fnorm, x[4], fvec[64], wa[512];
  extern void fcn();
        /*Fits one set of intensities from threshold scan. ud is 1 for lower threshold scan,
        -1 for upper threshold scan. Returns intensity, centroid and width of best fit to
        erf() function. x is array of DAC values for intensities y_meas.
        p0[0...3] is initial guess for solution.*/
	
	tol = sqrt(dpmpar_(&one));
	m=NPOINTS;
	n=4;
	lwa=512;
	lmdif1_(&fcn, &m, &n, p0, fvec, &tol, info, iwa, wa, &lwa);

	}

int get_factors(void){

FILE *fl;
int i,j;

	fl=fopen(Fname,"r");
	printf("fl=%i    %s\n",fl,strerror(errno));
	if(fl != NULL){
	 for(i=0;i<4;i++){
	   for(j=0;j<NCH;j++){
		fscanf(fl,"%g",&factor[i][j]);
		printf("Factor(%i,%i)=%g\n",i,j,factor[i][j]);
	   }
	  }
	}
	else{
	  printf("File error! Factors set to 870\n"); 
	  for(i=0;i<4;i++){
	   for(j=0;j<NCH;j++){
		factor[i][j]=870.0;
	   }
	  }
	
	}
	if(fl != NULL) fclose(fl);
	return(0);
}


void Correct(){
 
  /*Takes individual offsets, and makes a correction to the on-chip 
  dac to bring all channels as close to the mean as possible.
  DAC has range 0-63.
*/

double av1lo,av1hi,av2lo,av2hi;
int i,n;


get_factors();

av1lo=0;
av1hi=0;
av2lo=0;
av2hi=0;
n=0;
for(i=0;i<NCH;i++){
  if(dis[i]==0){
    n+=1;
/*    av1lo+=fits[0][i][1];
    av1hi+=fits[1][i][1];
    av2lo+=fits[2][i][1];
    av2hi+=fits[3][i][1]; */
    av1lo+=fits[0][i][1];
    av1hi+=fits[1][i][1];
    av1lo+=fits[2][i][1];
    av1hi+=fits[3][i][1];
    
    }
  }
av1lo=av1lo/(double)n/2.0;
av1hi=av1hi/(double)n/2.0;
/*av2lo=av2lo/(double)n;
av2hi=av2hi/(double)n;*/

  for (i=0;i<NCH;i++){
    v1_l[i]=(int)((fits[0][i][1]-av1lo)*factor[0][i])+32;
    if(v1_l[i]<=0) v1_l[i]=0;
    if(v1_l[i]>=63) v1_l[i]=63;
    v1_h[i]=(int)((fits[1][i][1]-av1hi)*factor[1][i])+32;
    if(v1_h[i]<=0) v1_h[i]=0;
    if(v1_h[i]>=63) v1_h[i]=63;
    v2_l[i]=(int)((fits[2][i][1]-av1lo)*factor[2][i])+32;
    if(v2_l[i]<=0) v2_l[i]=0;
    if(v2_l[i]>=63) v2_l[i]=63;
    v2_h[i]=(int)((fits[3][i][1]-av1hi)*factor[3][i])+32;
    if(v2_h[i]<=0) v2_h[i]=0;
    if(v2_h[i]>=63) v2_h[i]=63;
    }
  ezcaPut("det1.TR1",ezcaByte,NCH,v1_l);
  ezcaPut("det1.TR2",ezcaByte,NCH,v1_h);
  ezcaPut("det1.TR3",ezcaByte,NCH,v2_l);
  ezcaPut("det1.TR4",ezcaByte,NCH,v2_h);

if(save_dacs()) printf("save fits error\n");
}

int ResetTrims(int n){
int i;

  for(i=0;i<NCH;i++){
  	v1_l[i]=n;
	v1_h[i]=n;
	v2_l[i]=n;
	v2_h[i]=n;
	}
  ezcaPut("det1.TR1",ezcaByte,NCH,v1_l);
  ezcaPut("det1.TR2",ezcaByte,NCH,v1_h);
  ezcaPut("det1.TR3",ezcaByte,NCH,v2_l);
  ezcaPut("det1.TR4",ezcaByte,NCH,v2_h);
}

int Disable_outliers(void){
	ezcaPut("det1.CHEN",ezcaByte,NCH,dis);
}

int Enable_all(void){
int i;
	for(i=0;i<NCH;i++) dis[i]=0;
	ezcaPut("det1.CHEN",ezcaByte,NCH,dis);
}
