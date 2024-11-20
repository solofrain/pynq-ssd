#include <tsDefs.h>
#include <cadef.h>
#include <ezca.h>

int PutByte(const char *pvname, char *a, int n){
int res;
	res=ezcaPut(pvname,ezcaByte,n,a);
	return res;
	}
	
int PutShort (const char *pvname, short *a,int n){
int res;
	res=ezcaPut(pvname,ezcaShort,n,a);
	return res;
	}

int PutLong (const char *pvname, long *a,int n){
int res;
	res=ezcaPut(pvname,ezcaLong,n,a);
	return res;
	}

int PutFloat(const char *pvname, float *a,int n){
int res;
	res=ezcaPut(pvname,ezcaFloat,n,a);
	return res;
	}

int PutDouble(const char *pvname, double *a, int n){
int res;
	res=ezcaPut(pvname,ezcaDouble,n,a);
	return res;
	}

int GetByte(const char *pvname, char *a, int n){
int res;
	res=ezcaGet(pvname,ezcaByte,n,a);
	return res;
	}

int GetLong(const char *pvname, long *a, int n){
int res;
	res=ezcaGet(pvname,ezcaLong,n,a);
	return res;
	}

int GetFloat(const char *pvname, float *a, int n){
int res;
	res=ezcaGet(pvname,ezcaFloat,n,a);
	return res;
	}

int GetDouble(const char *pvname, double *a, int n){
int res;
	res=ezcaGet(pvname,ezcaDouble,n,a);
	return res;
	}

int main(){

int i,res;
long a[384];

  for(i=0;i<200;i++){
  	printf("%i\n",i);
  	res=GetLong("det1.S1",a,384);
	if(res!=0) printf("Get error %i",res);
	}
  for(i=0;i<384;i++) printf("%i ",a[i]);
  printf("\n"); 
}
