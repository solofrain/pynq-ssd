/*
 * http://elinux.org/Interfacing_with_I2C_Devices
 * https://www.kernel.org/doc/Documentation/i2c/dev-interface
 * http://blog.chrysocome.net/2013/03/programming-i2c.html
 *
 */


#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#define DACTF 0.001220703   // 5v / 2^12 
#define ADCTF 0.001  // 4.096v / 2^12

#define HV_I_GAIN 9.6  //   2.5v / 24uA
#define HV_I_OFFSET 0.293   // 0.293v  

int dacaddr = 0b01001010;        // The I2C address of the DAC 
int adcaddr = 0b00001000;        // The I2C address of the first ADC
 


/************************************************************
* Read LTC2309 ADC
*
* To Start a conversion, write control word to select with channel
* then small pause for conversion time, then read back value
************************************************************/
float ReadADCVoltage(int dev, int channel)
{
   char buf[3] = {0};
   int bytesWritten, bytesRead;
   int voltagei, cntrlword;
   float voltage;

   switch (channel) {
      case 0 : cntrlword = 0x88; break;
      case 2 : cntrlword = 0x98; break;
      case 4 : cntrlword = 0xA8; break;
      case 6 : cntrlword = 0xB8; break;
      case 1 : cntrlword = 0xC8; break;
      case 3 : cntrlword = 0xD8; break;
      case 5 : cntrlword = 0xE8; break;
      case 7 : cntrlword = 0xF8; break;
   }


    //set i2c to ADC address for voltage, current readbacks
    if (ioctl(dev,I2C_SLAVE,adcaddr) < 0) {
        printf("Failed to acquire bus access to ADC and/or talk to slave.\n");
        exit(1);
    }

   buf[0] = cntrlword;
   bytesWritten = write(dev,buf,1);
   usleep(10000);
   bytesRead = read(dev,buf,2);
   //printf("ADC Read : BytesRead:%d     Byte1Val: %x   Byte2Val: %x\n",bytesRead,buf[0],buf[1]);
   voltagei = (((buf[0] << 8) | buf[1]) >> 4);
   voltage = (float)voltagei * ADCTF; 
    
   //printf("Voltage Readback: %f\n",voltage);

   return (float) voltage;
}




/************************************************************
* Enable Internal Reference on DAC7678 
*
************************************************************/
void EnableIntRef(int dev)
{
    char buf[3] = {0};
    int bytesWritten;

    buf[0] = 0x80;  //Command Access Byte
    buf[1] = 0x00;
    buf[2] = 0x10;
    if (bytesWritten=write(dev,buf,3) != 3)
       printf("Error Writing DAC to Set Int Ref...   Bytes Written: %d\n",bytesWritten);
    //else 
      // printf("Internal Reference Enabled...\n");
}



/************************************************************
* Set DAC Voltage on DAC7678 
*
************************************************************/
void SetDacVoltage(int dev, int channel, float voltage)
{
    char buf[3] = {0};
    int bytesWritten;
    short int dacWord; 

    //set i2c to DAC address
    if (ioctl(dev,I2C_SLAVE,dacaddr) < 0) {
       printf("Failed to acquire bus access to DAC and/or talk to slave.\n");
       exit(1);
    }

    // Enable Internal Reference
    EnableIntRef(dev);

    if (voltage > 200) {
       printf("HV Setting too high... Exiting\n");
       exit(1);
       }
       
    voltage = voltage / 100; // (1v => 100volts)
    dacWord = (int)(voltage / DACTF);
    if (dacWord > 4095)  dacWord = 4095; 
    if (dacWord < 0)     dacWord = 0;
    //printf("Set Voltage: %f   V\n",voltage);
    //printf("DAC TF: %f\n",DACTF);
    //printf("DAC Word: %d   (0x%x)\n",dacWord,dacWord);
  
    buf[0] = 0x30 + channel; //Command Access Byte
    buf[1] = (char)((dacWord & 0x0FF0) >> 4);
    buf[2] = (char)((dacWord & 0x000F) << 4);
    //printf("MSB: %x    LSB: %x\n",((dacBits & 0xFF00) >> 8),(dacBits & 0x00FF));
    //printf("MSB: %x    LSB: %x\n",buf[1],buf[2]);
    bytesWritten = write(dev,buf,3);
    //printf("DAC Written...  Bytes Written : %d\n",bytesWritten);
}



/************************************************************
* Read DAC Voltage on DAC7678 
*
************************************************************/
float ReadDacVoltage(int dev, int channel)
{
   char buf[3] = {0};
   int bytesWritten, bytesRead;
   int voltage;

   buf[0] = 0x10 + channel;  //Command Access Byte
   bytesWritten = write(dev,buf,1);
   bytesRead = read(dev,buf,2);
   //printf("DAC Read : BytesRead:%d     Byte1Val: %x   Byte2Val: %x\n",bytesRead,buf[0],buf[1]);   
   voltage = (((buf[0] << 8) | buf[1]) >> 4);
   //printf("Voltage Readback: %f\n",voltage*DACTF); 

   return (float) voltage;
}



 
int main(int argc, char *argv[])
{
    int dev;
    char filename[40], buf[10];
    int i;
    float hv_setpt, rdback; 
    int volt_diff, hv_start, boardNum, channelNum, step_size;
    float hv_v;
    float hv_i, diff;
    unsigned int new_setpt;


    if ((argc < 2) || (argc > 3)) {
       printf("Usage: %s [set read]  voltage\n", argv[0]);
       exit(1);
       } 



    if (strcmp(argv[1], "set") == 0) {
       if (argc != 3) {
          printf("Usage: %s set voltage\n", argv[0]);
          exit(1);
          }
       hv_setpt = atof(argv[2]);
       //printf("Setting HV:  %d   volts\n",voltage);
       }
    else if (strcmp(argv[1], "read") == 0) {
       if (argc != 2) {
          printf("Usage: %s read \n", argv[0]);
          exit(1);
          }
       //printf("Reading HV:\n");
       }
    else {
       printf("Usage: %s [set read] voltage\n",argv[0]);
       exit(1);
       } 

    sprintf(filename,"/dev/i2c-1");
    if ((dev = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        exit(1);
    }

    if (strcmp(argv[1], "set") == 0) {
        //Ramp the HV
        //readback the HV voltage
        hv_v = (ReadADCVoltage(dev,4) * 100.0 );
        //readback the HV current
        hv_i = ReadADCVoltage(dev,5);
        //Gain,offset cal
        hv_i = (hv_i - HV_I_OFFSET) * HV_I_GAIN;
        printf("Voltage: %3.1f V\t Current: %f uA\n",hv_v,hv_i);

        volt_diff = (int)(hv_setpt - hv_v);
        new_setpt = hv_setpt;     
        step_size = 2; 
        printf("Set_Point: %f\tVoltage Diff: %d\n",hv_setpt, volt_diff);
        while (fabs((float)hv_setpt - hv_v) > 0.5) { 
           volt_diff = (int)(hv_setpt - hv_v);
           if (abs(volt_diff) < 10) 
	       step_size = 1;
           if (volt_diff < 0) {
               new_setpt = hv_v-step_size; 
               SetDacVoltage(dev,5,new_setpt);  //channel 5 is HV
           }          
           else  {
               new_setpt = hv_v+step_size;        
               SetDacVoltage(dev,5,new_setpt);  //channel 5 is HV
           } 
           sleep(1);
           //readback the HV voltage
           hv_v = (ReadADCVoltage(dev,4) * 100.0 );
           //readback the HV current
           hv_i = ReadADCVoltage(dev,5);
           //Gain,offset cal
           hv_i = (hv_i - HV_I_OFFSET) * HV_I_GAIN;
           printf("User Setpt: %3.0f V\tNew Setpt: %d V\t\tVoltage Rdback: %3.1f V\t Current Rdback: %f uA\n",hv_setpt, new_setpt, hv_v,hv_i);
           //diff = fabs((float)hv_setpt - hv_v);
           //printf("diff: %f\n",  diff);
         }

        //rdback = ReadDacVoltage(dev,5);
        //printf("\n\n");
    }

    usleep(100000);
    //readback the HV voltage
    hv_v = (ReadADCVoltage(dev,4) * 100.0 );
    //readback the HV current
    hv_i = ReadADCVoltage(dev,5);
    //Gain,offset cal
    hv_i = (hv_i - HV_I_OFFSET) * HV_I_GAIN;
    printf("Voltage: %3.1f V\t Current: %f uA\n",hv_v,hv_i);





return 0;
}

