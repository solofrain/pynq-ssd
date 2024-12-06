#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <epicsMutex.h>

#include "pl_reg.h"


#define FASTLOCK(PFAST_LOCK)     epicsMutexLock(*(PFAST_LOCK));
#define FASTUNLOCK(PFAST_LOCK)   epicsMutexUnlock(*(PFAST_LOCK));


volatile unsigned int *fpgabase; 

epicsMutexId fpga_lock;

void fpga_init(uint32_t phy_addr, uint32_t size)
{
    int fd = open("/dev/mem",O_RDWR|O_SYNC);

    if (fd < 0)
    {
        printf( "Can't open /dev/mem\n" );
        exit(1);
    }

    fpgabase = (unsigned int *) mmap( 0,
                                      size,
                                      PROT_READ|PROT_WRITE,MAP_SHARED,
                                      fd,
                                      phy_addr
                                    );
    close(fd);

    if ( fpgabase == NULL )
    {
        printf("Can't map FPGA space\n");
        exit(1);
    }
	fpga_lock = epicsMutexCreate();
}

uint32_t fpga_read(uint32_t addr)
{
    uint32_t val;

    FASTLOCK( &(fpga_lock) );
    val = fpgabase[addr];
    FASTUNLOCK( &(fpga_lock) );

    return val;
}

void fpga_write(uint32_t addr, uint32_t val)
{
    FASTLOCK( &(fpga_lock) );
    fpgabase[addr] = val;
    FASTUNLOCK( &(fpga_lock) );
}


