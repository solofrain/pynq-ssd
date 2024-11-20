#include <tsDefs.h>
#include <cadef.h>
#include <ezca.h>

long data[4,384,64];


int main(){

/*char response[16];*/
float cnt_time = 0.5;
int i,j,res,done;
long a[384];
res=0;

/*ezcaDebugOn();*/
  for(i=0;i<10;i++){
  	printf("%i\n",i);
	ezcaPut("det1.TP",ezcaFloat,1,&cnt_time);
	ezcaPut("det1.CNT",ezcaString,1,"Count");
	ezcaSetMonitor("det1.CNT",ezcaString);
	while(ezcaNewMonitorValue("det1.CNT",ezcaString)==0){}		 
	ezcaClearMonitor("det1.CNT",ezcaString);
	printf("leaving Count(%g)\n",cnt_time);

  	res=ezcaGet("det1.S2",ezcaLong,384,a);
	printf("e\n");
	if(res!=0) printf("Get error %i",res);
	for(j=0;j<96;j++){
		data[0,j,i]=a[j];
		}
	}
  for(i=0;i<10;i++){
    for(j=0;j<96;j++){
	printf("%i ",data[0,j,i]);
	}
	printf("\n"); 
  }
  printf("\n"); 
}
