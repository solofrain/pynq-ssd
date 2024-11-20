#include <math.h>
#include <tsDefs.h>
#include <cadef.h>
#include <ezca.h>
#include <grace_np.h>


int NCH=384;
int NPOINTS=32;
int STEP=0.01;
int data[4,384,64]; /*2 registers x 2 thresholds, 384 channels, 32-point scans *//
float fits[4,384,4];
float x[64];
float p0[4];
char v1_l[384], v1_h[384], v2_l[384], v2_h[384];
int factor=2000;
float mx=0.0;
float mn=0.0;
float dishi=3.0;
float dislo=0.4;
float cnt_time=1.0;


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
        for(i=0;i<NCH;i++){
                sprintf(str1,"g%i.s%i symbol %i",j,i,j+1);
                GracePrintf(str1);
                }
        }
}

int makex(float guess){
int i;
        for(i=0;i<NPOINTS;i++){
                x[i]=guess+(i-NPOINTS/2)*STEP;
                }
        }

int count(float t){
char response[6];
        ezcaPut("det1.TP",ezcaFloat,1,&t);
        ezcaPut("det1.CNT",ezcaString,1,"Count");
        strncpy(response,"Count",5);
        while(strncmp(response,"Count",5)==0){
                 ezcaGet("det1.CNT",ezcaString,1,response);
                }
}

int scan_levels(float dlo, float dhi, float midguess){
int ud,i,j;
int y[384];
char str1[256];

    makex(midguess);
    ScanGraceinit();
    ud=1;
    ezcaPut("peter:ao2",ezcaFloat,1,&dhi);
    ezcaPut("peter:ao4",ezcaFloat,1,&dhi);
    for(i=0;i<NPOINTS;i++){
        ezcaPut("peter:ao1",ezcaFloat,1,&x[i]);
        ezcaPut("peter:ao3",ezcaFloat,1,&x[i]);
        count(cnt_time);
        ezcaGet("det1.S2",ezcaLong,384,y);
        for(j=0;j<384;j++){
                data[0,j,i]=y[j];
                }
        ezcaGet("det1.S3",ezcaLong,384,y);
        for(j=0;j<384;j++){
                data[2,j,i]=y[j];
                }
        GracePrintf("focus g0");
        for(j=0;j<NCH;j++){
                sprintf(str1,"g0.s%i point %g, %g",j,x[i],data[0,j,i]);
                GracePrintf(str1);
                }
        GracePrintf("autoscale");
        GracePrintf("redraw");

        GracePrintf("focus g2");
        for(j=0;j<NCH;j++){
                sprintf(str1,"g2.s%i point %g, %g",j,x[i],data[2,j,i]);
                GracePrintf(str1);
                }
        }
    ud=-1;
    ezcaPut("peter:ao1",ezcaFloat,1,&dlo);
    ezcaPut("peter:ao3",ezcaFloat,1,&dlo);
    for(i=0;i<NPOINTS;i++){
        ezcaPut("peter:ao2",ezcaFloat,1,&x[i]);
        ezcaPut("peter:ao4",ezcaFloat,1,&x[i]);
        count(cnt_time);
        ezcaGet("det1.S2",ezcaLong,384,y);
        for(j=0;j<384;j++){
                data[1,j,i]=y[j];
                }
        ezcaGet("det1.S3",ezcaLong,384,y);
        for(j=0;j<384;j++){
                data[3,j,i]=y[j];
                }
        GracePrintf("focus g1");
        for(j=0;j<NCH;j++){
                sprintf(str1,"g1.s%i point %g, %g",j,x[i],data[1,j,i]);
                GracePrintf(str1);
                }
        GracePrintf("autoscale");
        GracePrintf("redraw");

        GracePrintf("focus g3");
        for(j=0;j<NCH;j++){
                sprintf(str1,"g3.s%i point %g, %g",j,x[i],data[3,j,i]);
                GracePrintf(str1);
                }
        GracePrintf("autoscale");
        GracePrintf("redraw");

        }
}

int fitall(){
float dat[64],p0[4],p[4],half,width;
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
                                dat[i]=data[j,k,i];
                                if(dat[i]>mx) mx=dat[i];
                                if(dat[i]<mn) mn=dat[i];
                                }
                        if((j==0) || (j==2)) ud=1;
                        if((j==1) || (j==3)) ud=-1;
                        half=(mn+mx)/2.0;
                        for(i=0;i<NPOINTS-1;i++){
                                if((dat[i]>=half) && (dat[i+1]<=half)){
                                        printf("LoMid = %i\n",i);
                                        p0[1]=x[i];
                                        p0[2]=60.0;
                                        }
                                }
                        fit(x,dat,p0,p,ud);
                        for(i=0;i<4;i++) fits[j,k,i]=p[i];
                        if(fits[j,k,2]!=0){
                                width=1/fits[j,k,2];
                                }
                        else width=0;
                        printf("Center=%g  width=%g  max=%g  min=%g",fits[j,k,1],width,fits[j,k,0]+fits[j,k,3],fits[j,k,3]);
                        sprintf(str1,"g0.s%i point %g, %g",j,fits[j,k,1], width);
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

int fit(x,y_meas,p0,p,ud){

        /*Fits one set of intensities from threshold scan. ud is 1 for lower threshold scan,
        -1 for upper threshold scan. Returns intensity, centroid and width of best fit to
        erf() function. x is array of DAC values for intensities y_meas.
        p0[0...3] is initial guess for solution.

        plsq = leastsq(residuals, p0, args=(y_meas, x, ud));
        print plsq;
        return plsq[0]; */
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
    v1_l[i]=int((fits[0,i,1]-av1lo)*factor)+32
    if(v1_l[i]<0):
        v1_l[i]=0
    if(v1_l[i]>=63):
        v1_l[i]=63
    v1_h[i]=int((fits[1,i,1]-av1hi)*factor)+32
    if(v1_h[i]<0):
        v1_h[i]=0
    if(v1_h[i]>=63):
        v1_h[i]=63
    v2_l[i]=int((fits[2,i,1]-av2lo)*factor)+32
    if(v2_l[i]<0):
        v2_l[i]=0
    if(v2_l[i]>=63):
        v2_l[i]=63
    v2_h[i]=int((fits[3,i,1]-av2hi)*factor)+32
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

