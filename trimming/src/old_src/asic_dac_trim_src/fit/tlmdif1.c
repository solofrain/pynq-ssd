

/*     driver for lmdif1 example. */

#include <stdio.h>
#include <math.h>
#include <minpack.h>

int main()
{
  int m, n, info, lwa, iwa[4], one=1;
  double tol, fnorm, x[4], fvec[64], wa[512];
  extern void fcn(), feval();

  m = 32;
  n = 4;

  /* the following starting values provide a rough fit. */

  x[0] = 450.e0;
  x[1] = 15.e0;
  x[2] = 1.e0;
  x[3] = 0.e0;

  lwa = 512;

  /* set tol to the square root of the machine precision.  unless high
     precision solutions are required, this is the recommended
     setting. */

  tol = sqrt(dpmpar_(&one));

  lmdif1_(&fcn, &m, &n, x, fvec, &tol, &info, iwa, wa, &lwa);

  fnorm = enorm_(&m, fvec);

  printf("      FINAL L2 NORM OF THE RESIDUALS%15.7f\n\n",fnorm);
  printf("      EXIT PARAMETER                %10i\n\n", info);
  printf("      FINAL APPROXIMATE SOLUTION\n\n %15.7f%15.7f%15.7f%15.7f\n",
	 x[0], x[1], x[2], x[3]);
  
  feval(m,n,x,fvec,&info);
  
  return 0;
}

void fcn(int m, int n, double *x, double *fvec, int *iflag)
{
  /* function fcn for lmdif1 example */

  int i;
  double tmp1,tmp2,tmp3;
  double y[32]={901,901,901,901,901,901,901,901,901,901,900,889,870,833,756,690,533,412,231,157,84,34,8,1,0,0,0,0,0,0,0};
  double xd[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
  double ud=1.0;
  
  for (i=0; i<32; i++)
    {
      fvec[i] = y[i]-(x[0]*(1+erf(x[2]*ud*(x[1]-xd[i])))+x[3]);
    }
}

void feval(int m, int n, double *x, double *fvec, int *iflag)
{
  /* function fcn for lmdif1 example */

  int i;
  double tmp1,tmp2,tmp3;
  double y[32]={901,901,901,901,901,901,901,901,901,901,900,889,870,833,756,690,533,412,231,157,84,34,8,1,0,0,0,0,0,0,0};
  double xd[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
  double ud=1.0;
  
  for (i=0; i<32; i++)
    {
	fvec[i] = (x[0]*(1+erf(x[2]*ud*(x[1]-xd[i])))+x[3]);
	printf("%g %g\n",y[i],fvec[i]);
    }
    
}
