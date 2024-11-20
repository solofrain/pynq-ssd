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

#define ADCTF 0.001   // 4.096v / 2^12 


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
   int voltage, cntrlword;


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

   buf[0] = cntrlword; 
   bytesWritten = write(dev,buf,1);
   printf("Bytes written: %i\n",bytesWritten);
   usleep(10000);
   bytesRead = read(dev,buf,2);
   printf("Bytes read: %i\n",bytesRead);
   printf("DAC Read : BytesRead:%d     Byte1Val: %x   Byte2Val: %x\n",bytesRead,buf[0],buf[1]);   
   voltage = (((buf[0] << 8) | buf[1]) >> 4);
   printf("Voltage Readback: %f\n",voltage*ADCTF); 

   return (float) voltage;
}

 
int main(int argc, char *argv[])
{
    int dev;
    char filename[40], buf[10];
    int addr; 
    int addr1 = 0b00001000;        // 1st ADC i2c address 
    int addr2 = 0b00001010;        // 2nd ADC i2c address
    int i;
    float rdback, voltage;
    int boardNum, channelNum;

    if (argc != 2) {
      printf("Usage: %s channel(0:7) \n", argv[0]);
      printf("Ch0 : Temperature 1\nCh1 : Temperature 4\nCh2 : Temperature 3\nCh3 : Temperature 1\n");
      printf("Ch4 : Bias Voltage\nCh5 : Bias Current\nCh6 : Peltier1 Current\nCh7 : Peltier2 Current\n");
      printf("Ch8 : Detector Leakage\nCh9 : Peltier Voltage\n");
      exit(1);
    }

    channelNum = atoi(argv[1]);

    printf("Reading ADC:  ch: %d \n",channelNum);

    sprintf(filename,"/dev/i2c-0");
    if ((dev = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        exit(1);
    }

    if (channelNum < 8) 
       addr = addr1;
    else  if(channelNum<16){
       addr = addr2;
       channelNum = channelNum - 8;
       }
    else {
         printf("Bad channel number\n");
         exit(1);
    }

    printf("Address=%x\n",addr);

    if (ioctl(dev,I2C_SLAVE,addr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    // Enable Internal Reference
    //EnableIntRef(dev);

    //SetDacVoltage(dev,channelNum,voltage);
    rdback = ReadADCVoltage(dev,channelNum);
    printf("\n\n");

return 0;
}
