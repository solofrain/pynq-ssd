
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float ztemp(void)
{
  int t_raw, t_offset;
  double t_scale;
  FILE* ztemp_raw = fopen("/sys/bus/platform/drivers/xadc/f8007100.adc/iio:device0/in_temp0_raw", "r");
  FILE* ztemp_scale = fopen("/sys/bus/platform/drivers/xadc/f8007100.adc/iio:device0/in_temp0_scale","r");
  FILE* ztemp_offset = fopen("/sys/bus/platform/drivers/xadc/f8007100.adc/iio:device0/in_temp0_offset", "r");

  fscanf(ztemp_raw,"%d",&t_raw);
  fscanf(ztemp_offset,"%d",&t_offset);
  fscanf(ztemp_scale,"%lf",&t_scale);

  fclose(ztemp_raw);
  fclose(ztemp_scale);
  fclose(ztemp_offset);
  return ((((float) t_offset + (float) t_raw) * (float) t_scale)  / 1000.0);
}

int main (void)
{
  printf ("Temp = %.1f\n",ztemp());
}

