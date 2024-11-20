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

#define DACTF 0.001220703   // 5v / 2^12 


void EnableIntRef(int dev)
{
    char buf[3] = {0};
    int bytesWritten;

    buf[0] = 0x80;  //Command Access Byte
    buf[1] = 0x00;
    buf[2] = 0x10;
    if (bytesWritten=write(dev,buf,3) != 3)
       printf("Error Writing DAC to Set Int Ref...   Bytes Written: %d\n",bytesWritten);
    else 
       printf("Internal Reference Enabled...\n");
}


void SetDacVoltage(int dev, int channel, float voltage)
{
    char buf[3] = {0};
    int bytesWritten;
    short int dacWord; 
    
    dacWord = (int)(voltage / DACTF);
    if (dacWord > 4095)  dacWord = 4095; 
    if (dacWord < 0)     dacWord = 0;
    printf("Set Voltage: %f   V\n",voltage);
    //printf("DAC TF: %f\n",DACTF);
    printf("DAC Word: %d   (0x%x)\n",dacWord,dacWord);
  
    buf[0] = 0x30 + channel; //Command Access Byte
    buf[1] = (char)((dacWord & 0x0FF0) >> 4);
    buf[2] = (char)((dacWord & 0x000F) << 4);
    //printf("MSB: %x    LSB: %x\n",((dacBits & 0xFF00) >> 8),(dacBits & 0x00FF));
    printf("MSB: %x    LSB: %x\n",buf[1],buf[2]);
    bytesWritten = write(dev,buf,3);
    printf("DAC Written...  Bytes Written : %d\n",bytesWritten);
}
 
float ReadDacVoltage(int dev, int channel)
{
   char buf[3] = {0};
   int bytesWritten, bytesRead;
   int voltage;

   buf[0] = 0x10 + channel;  //Command Access Byte
   bytesWritten = write(dev,buf,1);
   bytesRead = read(dev,buf,2);
   printf("DAC Read : BytesRead:%d     Byte1Val: %x   Byte2Val: %x\n",bytesRead,buf[0],buf[1]);   
   voltage = (((buf[0] << 8) | buf[1]) >> 4);
   printf("Voltage Readback: %f\n",voltage*DACTF); 

   return (float) voltage;
}

 
int main(int argc, char *argv[])
{
    int dev;
    char filename[40], buf[10];
    int addr = 0b01001010;        // The I2C address of the ADC
    int i;
    float rdback, voltage;
    int boardNum, channelNum;

    if (argc != 3) {
      printf("Usage: %s channel(0:7) voltage\n", argv[0]);
      exit(1);
    }

    channelNum = atoi(argv[1]);
    voltage = atof(argv[2]);

    printf("Setting DAC:  ch: %d,   voltage: %f\n",channelNum,voltage);

    sprintf(filename,"/dev/i2c-0");
    if ((dev = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        exit(1);
    }

    if (ioctl(dev,I2C_SLAVE,addr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    // Enable Internal Reference
    EnableIntRef(dev);

    SetDacVoltage(dev,channelNum,voltage);
    rdback = ReadDacVoltage(dev,channelNum);
    printf("\n\n");

return 0;
}
