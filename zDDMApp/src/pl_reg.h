#ifndef _PL_REG_H_
#define _PL_REG_H_

#include <stdint.h>

// Registers
#define VERSIONREG     3
#define TOKENREG       4
#define SPIREG         5
#define HERMESCALREG   6
#define BITS           7
#define SPIOUT         8

#define TRIG          52
#define PR1           53
#define COUNT_MODE    55
#define FRAME_NO      54
#define CNTR          55

void fpga_init(uint32_t phy_addr, uint32_t size);
uint32_t fpga_read(uint32_t addr);
void fpga_write(uint32_t addr, uint32_t val);

#endif
