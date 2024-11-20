#include <math.h>
#include <tsDefs.h>
#include <cadef.h>
#include <ezca.h>
#include <grace_np.h>

#undef DEBUG

int NCH=384;
int NPOINTS=32;
float STEP=0.01;
int NPLT=96;
long data[4][384][64]; /*2 registers x 2 thresholds, 384 channels, 32-point scans */
double fits[4][384][4];
double x[64];
double p0[4];
char v1_l[384], v1_h[384], v2_l[384], v2_h[384];
int factor=2000;
double mx=0.0;
double mn=0.0;
float dishi=3.0;
float dislo=0.4;
float cnt_time=1.0;
float guess=1.0;
double fitdat[64],fitud;

int m, n, info, lwa, iwa[4], one=1;
double tol, fnorm, fvec[64], wa[512];

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
       sprintf(str1,"s%i symbol %i",j,j);
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
int ud,i,j,res;
long y[384];
char str1[256];

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
	ezcaPut("peter:ao1",ezcaFloat,1,&x[i]);
	ezcaPut("peter:ao3",ezcaFloat,1,&x[i]);
#ifdef DEBUG
	printf("about to count %g at %g...\n",cnt_time, x[i]);
#endif
	count();			
#ifdef DEBUG
	printf("getting data\n");
#endif
	res=ezcaGet("det1.S2",ezcaLong,384,y);
	if(res!=0) printf("Get error %i",res);

	for(j=0;j<384;j++){
		data[0][j][i]=y[j];
		printf(" %i",data[0][j][i]);
		}
	printf("\n");
	res=ezcaGet("det1.S3",ezcaLong,384,y);
	if(res!=0) printf("Get error %i",res);
	for(j=0;j<384;j++){
		data[2][j][i]=y[j];
		printf(" %i",data[2][j][i]);
		}
	printf("\n");

#ifdef DEBUG
	printf("Plotting point %i..\n",i);
#endif
	GracePrintf("focus g0");
	for(j=0;j<NPLT;j++){
		sprintf(str1,"g0.s%i point %g, %i",j,x[i],data[0][j][i]);
		printf("%s\n",str1);
		GracePrintf(str1);
		}
	GracePrintf("autoscale");
	GracePrintf("redraw");

	GracePrintf("focus g2");
	for(j=0;j<NPLT;j++){
		sprintf(str1,"g2.s%i point %g, %i",j,x[i],data[2][j][i]);
		printf("%s\n",str1);
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
	ezcaPut("peter:ao2",ezcaFloat,1,&x[i]);
	ezcaPut("peter:ao4",ezcaFloat,1,&x[i]);
#ifdef DEBUG
	printf("about to count %g at %g\n",cnt_time, x[i]);
#endif
	count();
#ifdef DEBUG
	printf("getting data\n");
#endif
	res=ezcaGet("det1.S2",ezcaLong,384,y);
	if(res!=0) printf("Get error %i",res);
	for(j=0;j<384;j++){
		data[1][j][i]=y[j];
		}
	res=ezcaGet("det1.S3",ezcaLong,384,y);
	if(res!=0) printf("Get error %i",res);
	for(j=0;j<384;j++){
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
	for(i=0;i<NPLT;i++){
	  for(j=0;j<NPOINTS;j++){
	  	printf(" %i",data[0][i][j]);
		}
		printf("\n");
	}
}

 
		


void fitall(){
float p0[4],p[4],half,width;
int i,j,k,ud;
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
				if(fitdat[i]>mx) mx=fitdat[i];
				if(fitdat[i]<mn) mn=fitdat[i];
				}
			if((j==0) || (j==2)) fitud=1;
			if((j==1) || (j==3)) fitud=-1;
                  	half=(mn+mx)/2.0;
			for(i=0;i<NPOINTS-1;i++){
                        	if((fitdat[i]>=half) && (fitdat[i+1]<=half)){
                                	printf("LoMid = %i\n",i);
                                	p0[1]=x[i];
                                	p0[2]=60.0;
					}
				}
			fit(p0);
			for(i=0;i<4;i++) fits[j][k][i]=p0[i];
			if(fits[j][k][2]!=0){
				width=1/fits[j][k][2];
				}
			else width=0;
                        printf("Center=%g  width=%g  max=%g  min=%g",fits[j][k][1],width,fits[j][k][0]+fits[j][k][3],fits[j][k][3]);
                        sprintf(str1,"g0.s%i point %g, %g",j,fits[j][k][1], width);
			GracePrintf(str1);
			}
	GracePrintf("autoscale");
	GracePrintf("redraw");
	}
}

float peval(float *x, float *p, int ud){
int i;
float res;
	res=0;
	for(i=0;i<NPOINTS;i++){
        	res+=p[0]*(1+erf(p[2]*ud*(p[1]-x[i])))+p[3];
	}
	return res;
}


float residuals(float *p, float *y, float *x, int ud){
int i;
float err;
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

  for (i=0; i<32; i++)
    {
      fvec[i] = fitdat[i]-(p[0]*(1+erf(p[2]*fitud*(p[1]-x[i])))+p[3]);
    }
}

int fit( double *p0){

  int m, n, info, lwa, iwa[4], one=1;
  double tol, fnorm, x[4], fvec[64], wa[512];
        /*Fits one set of intensities from threshold scan. ud is 1 for lower threshold scan,
        -1 for upper threshold scan. Returns intensity, centroid and width of best fit to
        erf() function. x is array of DAC values for intensities y_meas.
        p0[0...3] is initial guess for solution.*/
	
	tol = sqrt(dpmpar_(&one));
	m=NPOINTS;
	n=4;
	lwa=512;
	lmdif1_(&fcn, &m, &n, p0, fvec, &tol, &info, iwa, wa, &lwa);

	}


void Correct(){
 
  /*Takes individual offsets, and makes a correction to the on-chip 
  dac to bring all channels as close to the mean as possible.
  DAC has range 0-63, which is equivalent to 34.5mV
*/
/*
float av1lo,av1hi,av2lo,av2hi;


  av1lo=tmean(fits[0,0:NCH,1])
  av1hi=tmean(fits[1,0:NCH,1])
  av2lo=tmean(fits[2,0:NCH,1])
  av2hi=tmean(fits[3,0:NCH,1])
  
  for i in range(NCH):
    v1_l[i]=int((fits[0][i][1]-av1lo)*factor)+32
    if(v1_l[i]<0):
	v1_l[i]=0
    if(v1_l[i]>=63):
	v1_l[i]=63
    v1_h[i]=int((fits[1][i][1]-av1hi)*factor)+32
    if(v1_h[i]<0):
	v1_h[i]=0
    if(v1_h[i]>=63):
	v1_h[i]=63
    v2_l[i]=int((fits[2][i][1]-av2lo)*factor)+32
    if(v2_l[i]<0):
	v2_l[i]=0
    if(v2_l[i]>=63):
	v2_l[i]=63
    v2_h[i]=int((fits[3][i][1]-av2hi)*factor)+32
    if(v2_h[i]<0):
	v2_h[i]=0
    if(v2_h[i]>=63):
	v2_h[i]=63
    
  caput("det1.TR1",v1_l)
  caput("det1.TR2",v1_h)
  caput("det1.TR3",v2_l)
  caput("det1.TR4",v2_h)
    */
}

int ResetTrims(int n){
int i;

  for(i=0;i<NCH;i++){
  	v1_l[i]=n;
	v1_h[i]=n;
	v2_l[i]=n;
	v2_h[i]=n;
	}
  ezcaPut("det1.TR1",ezcaByte,384,v1_l);
  ezcaPut("det1.TR2",ezcaByte,384,v1_h);
  ezcaPut("det1.TR3",ezcaByte,384,v2_l);
  ezcaPut("det1.TR4",ezcaByte,384,v2_h);
}
