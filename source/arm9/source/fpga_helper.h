#ifndef _fpga_helper_H_
#define _fpga_helper_H_

#include <nds.h>

// MASS-1 (30yen config)
#define REV2 1

#ifdef REV2

// CPLD / HW assign

#define FPGA_CFG_DATA  ((volatile unsigned char *)0x0a000000)
#define FPGA_CFG_STS   ((volatile unsigned char *)0x0a00C000)
#define FPGA_CFG_PGM   ((volatile unsigned char *)0x0a00E000)

#define GET_FPGA_INIT()  ((*FPGA_CFG_STS & 0x01)!=0)
#define GET_FPGA_DONE()  ((*FPGA_CFG_STS & 0x02)!=0)
#define GET_FPGA_SENSE(A) (((FPGA_CFG_STS[A]>>2)&0x03)==(((A)>>8)&3))

// Error Code
#define FPGA_NO_ERR            0
#define FPGA_ERR_PIN_FEEDBACK -1
#define FPGA_ERR_NOT_DONE_L   -2
#define FPGA_ERR_NOT_INIT_L   -3
#define FPGA_ERR_NOT_INIT_H   -4
#define FPGA_ERR_DATA_CRC     -5
#define FPGA_ERR_NOT_DONE_H   -6

//int ds_fpga_bus_init(void);
//int ds_fpga_configration(const u8 *config_data,int config_data_size);

#endif // #ifdef REV2

extern void FPGA_BusOwnerARM7(void);
extern void FPGA_BusOwnerARM9(void);

extern bool FPGA_isExistCartridge(void);
extern bool FPGA_CheckBitStreamFormat(void *pbuf,u32 size);

extern bool FPGA_Start(void *pbuf,u32 size);
extern void FPGA_End(void);

#endif
