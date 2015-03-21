
/*
 referrence from 2006-01-13 - v2.11
 
  NDS MP
 GBAMP NDS Firmware Hack Version 2.0
 An NDS aware firmware patch for the GBA Movie Player.
 By Michael Chisholm (Chishm)
 
 Large parts are based on MultiNDS loader by Darkain.
 Filesystem code based on gbamp_cf.c by Chishm (me).
 Flashing tool written by DarkFader.
 Chunks of firmware removed with help from Dwedit.

 GBAMP firmware flasher written by DarkFader.
 
 This software is completely free. No warranty is provided.
 If you use it, please give due credit and email me about your
 project at chishm@hotmail.com
*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Loader functions

#if 0
__attribute__ ((section (".rebootloader"))) void reset_MemCopy32CPU(const void *src,void *dst,u32 len)
//asm void reset_MemCopy32CPU(const void *src,void *dst,u32 len)
{/* MEMCHK_COPY(3,src,dst,len); */
	asm volatile (
	"c32psrc .req r0 \n\t"
	"c32pdst .req r1 \n\t"
	"c32size .req r2 \n\t"
	" \n\t"
	"cmp c32size,#0 \n\t"
	"bxeq lr \n\t"
	" \n\t"
	"stmfd sp!,{r4,r5,r6,r7,r8,r9,r10} \n\t"
	" \n\t"
	"cmp c32size,#4*8 \n\t"
	"blo c32set32x1 \n\t"
	" \n\t"
	"c32set32x8: \n\t"
	"ldmia c32psrc!,{r3,r4,r5,r6,r7,r8,r9,r10} \n\t"
	"stmia c32pdst!,{r3,r4,r5,r6,r7,r8,r9,r10} \n\t"
	"subs c32size,c32size,#4*8 \n\t"
	"cmp c32size,#4*8 \n\t"
	"bhs c32set32x8 \n\t"
	" \n\t"
	"cmp c32size,#0 \n\t"
	"beq c32setend \n\t"
	" \n\t"
	"c32set32x1: \n\t"
	"ldr r3,[c32psrc],#4 \n\t"
	"subs c32size,c32size,#4 \n\t"
	"str r3,[c32pdst],#4 \n\t"
	"bne c32set32x1 \n\t"
	" \n\t"
	"c32setend: \n\t"
	"ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10} \n\t"
	"bx lr       \n\t"
	:::"memory"
	);
}
__attribute__ ((section (".rebootloader"))) void reset_MemSet32CPU(u32 v,void *dst,u32 len)
//asm void reset_MemSet32CPU(u32 v,void *dst,u32 len)
{/* MEMCHK_SET(3,v,dst,len); */
	asm volatile (
	"s32data .req r0 \n\t"
	"s32pbuf .req r1 \n\t"
	"s32size .req r2 \n\t"
	" \n\t"
	"cmp s32size,#0 \n\t"
	"bxeq lr \n\t"
	" \n\t"
	"stmfd sp!,{r4,r5,r6,r7,r8,r9} \n\t"
	" \n\t"
	"mov r3,s32data \n\t"
	"mov r4,s32data \n\t"
	"mov r5,s32data \n\t"
	"mov r6,s32data \n\t"
	"mov r7,s32data \n\t"
	"mov r8,s32data \n\t"
	"mov r9,s32data \n\t"
	" \n\t"
	"cmp s32size,#4*8 \n\t"
	"blo s32set32x1 \n\t"
	" \n\t"
	"s32set32x8: \n\t"
	"stmia s32pbuf!,{s32data,r3,r4,r5,r6,r7,r8,r9} \n\t"
	"subs s32size,s32size,#4*8 \n\t"
	"cmp s32size,#4*8 \n\t"
	"bhs s32set32x8 \n\t"
	" \n\t"
	"cmp s32size,#0 \n\t"
	"beq s32setend \n\t"
	" \n\t"
	"s32set32x1: \n\t"
	"str s32data,[s32pbuf],#4 \n\t"
	"subs s32size,s32size,#4 \n\t"
	"bne s32set32x1 \n\t"
	" \n\t"
	"s32setend: \n\t"
	"ldmfd sp!,{r4,r5,r6,r7,r8,r9} \n\t"
	"bx lr       \n\t"
	:::"memory"
	);
}

__attribute__ ((section (".rebootloader"))) void resetMemory2_ARM9_NoBIOS (vu32 *pIPC6RESET,u32 ARM9ExecAddr)
{
	// backup params \n\t"
	asm volatile (
	"mov r10,r0 \n\t"
	"mov r11,r1 \n\t"
#if 0
	"mov r1,#0x10000 \n\t"
	"waitloop: \n\t"
	"subs r1,r1,#1 \n\t"
	"bne waitloop \n\t"
#endif
  
#if 0
	"mov r1, #0 \n\t"
	"outer_loop: \n\t"
	"mov r0, #0 \n\t"
	"inner_loop: \n\t"
	"orr r2, r1, r0 \n\t"
	"mcr p15, 0, r2, c7, c14, 2 \n\t"
	"add r0, r0, #0x20 \n\t"
	"cmp r0, #0x400 \n\t"
	"bne inner_loop \n\t"
	"add r1, r1, #0x40000000 \n\t"
	"cmp r1, #0x0 \n\t"
	"bne outer_loop \n\t"
	" \n\t"
	"mov r1, #0 \n\t"
	"mcr p15, 0, r1, c7, c5, 0 @ Flush ICache \n\t"
	"mcr p15, 0, r1, c7, c6, 0 @ Flush DCache \n\t"
	"mcr p15, 0, r1, c7, c10, 4 @ empty write buffer \n\t"
	" \n\t"
	"mcr p15, 0, r1, c3, c0, 0 @ disable write buffer       (def = 0) \n\t"

#define ITCM_LOAD (1<<19)
#define ITCM_ENABLE (1<<18)
#define DTCM_LOAD (1<<17)
#define DTCM_ENABLE (1<<16)
#define DISABLE_TBIT (1<<15)
#define ROUND_ROBIT (1<<14)
#define ALT_VECTORS (1<<13)
#define ICACHE_ENABLE (1<<12)
#define BIG_ENDIAN (1<<7)
#define DCACHE_ENABLE (1<<2)
#define PROTECT_ENABLE (1<<0)
	
  // disable DTCM and protection unit \n\t"
	"	mrc	p15, 0, r0, c1, c0, 0 \n\t"
	"	ldr r1,= ~(ITCM_ENABLE | DTCM_ENABLE | ICACHE_ENABLE | DCACHE_ENABLE | PROTECT_ENABLE) \n\t"
	"	and r0,r0,r1 \n\t"
	"	ldr r1,=2_01111000 ; set SBO \n\t"
	"	orr	r0,r0,r1 \n\t"
	"	ldr r1,= ITCM_ENABLE | DTCM_ENABLE \n\t"
	"	orr	r0,r0,r1 \n\t"
	"	mcr	p15, 0, r0, c1, c0, 0 \n\t"
	" \n\t"
	"mcr p15, 0, r1, c6, c0, 0 @ disable protection unit 0  (def = 0) \n\t"
	"mcr p15, 0, r1, c6, c1, 0 @ disable protection unit 1  (def = 0) \n\t"
	"mcr p15, 0, r1, c6, c2, 0 @ disable protection unit 2  (def = 0) \n\t"
	"mcr p15, 0, r1, c6, c3, 0 @ disable protection unit 3  (def = 0) \n\t"
	"mcr p15, 0, r1, c6, c4, 0 @ disable protection unit 4  (def = ?) \n\t"
	"mcr p15, 0, r1, c6, c5, 0 @ disable protection unit 5  (def = ?) \n\t"
	"mcr p15, 0, r1, c6, c6, 0 @ disable protection unit 6  (def = ?) \n\t"
	"mcr p15, 0, r1, c6, c7, 0 @ disable protection unit 7  (def = ?) \n\t"
	" \n\t"
	"mov r1, #0x0000000C @ Use mov instead \n\t"
	"mcr p15, 0, r1, c9, c1, 1 @ ITCM base  (def = 0x0000000C) ??? \n\t"
	" \n\t"
	"mov r1, #0x00800000 @ Use mov instead \n\t"
	"add r1, r1, #0x00A \n\t"
	"mcr p15, 0, r1, c9, c1, 0 @DTCM base  (def = 0x0080000A) ??? \n\t"
	"	 \n\t"
	"mov r1, #0 \n\t"
	"mcr p15, 0, r1, c5, c0, 3 @ IAccess \n\t"
	"mcr p15, 0, r1, c5, c0, 2 @ DAccess \n\t"
	" \n\t"
	"mov r1, #0x1F \n\t"
	"msr cpsr_cxsf, r1 \n\t"
	"	 \n\t"
	// check ITCM
	"mov r0, #0x00000000 \n\t"
	"add r1,r0,#32*1024 \n\t"
	"fillITCM_loop: \n\t"
	"mov r2,r0 \n\t"
	"	str r2,[r0],#4 \n\t"
	"	cmp r0,r1 \n\t"
	"	bne fillITCM_loop \n\t"
	"	 \n\t"
	"mov r0, #0x00000000 \n\t"
	"add r1,r0,#32*1024 \n\t"
	"checkITCM_loop: \n\t"
	"	ldr r2,[r0] \n\t"
	"	cmp r0,r2 \n\t"
	"	bne checkITCM_loop \n\t"
	"	add r0,r0,#4 \n\t"
	"	cmp r0,r1 \n\t"
	"	bne checkITCM_loop \n\t"
	"	   \n\t"
	// check DTCM
	"mov r0, #0x00800000 \n\t"
	"add r1,r0,#16*1024 \n\t"
	"fillDTCM_loop: \n\t"
	"mov r2,r0 \n\t"
	"	str r2,[r0],#4 \n\t"
	"	cmp r0,r1 \n\t"
	"	bne fillDTCM_loop \n\t"
	"	 \n\t"
	"mov r0, #0x00800000 \n\t"
	"add r1,r0,#16*1024 \n\t"
	"checkDTCM_loop: \n\t"
	"	ldr r2,[r0] \n\t"
	"	cmp r0,r2 \n\t"
	"	bne checkDTCM_loop \n\t"
	"	add r0,r0,#4 \n\t"
	"	cmp r0,r1 \n\t"
	"	bne checkDTCM_loop \n\t"
	"	   \n\t"
	// clear ITCM
	"mov r0, #0x00000000 \n\t"
	"add r1,r0,#32*1024 \n\t"
	"mov r2,#0 \n\t"
	"clearITCM_loop: \n\t"
	"	str r2,[r0],#4 \n\t"
	"	cmp r0,r1 \n\t"
	"	bne clearITCM_loop \n\t"
	"	   \n\t"
	// clear DTCM
	"mov r0, #0x00800000 \n\t"
	"add r1,r0,#16*1024 \n\t"
	"mov r2,#0 \n\t"
	"clearDTCM_loop: \n\t"
	"	str r2,[r0],#4 \n\t"
	"	cmp r0,r1 \n\t"
	"	bne clearDTCM_loop \n\t"
#endif

#if 0
//---------------------------------------------------------------------------------
// Setup memory regions similar to Release Version
//---------------------------------------------------------------------------------

#define PAGE_4K		(0b01011 << 1)
#define PAGE_8K		(0b01100 << 1)
#define PAGE_16K	(0x0d<<1) // (0b01101 << 1)
#define PAGE_32K	(0x0e<<1) // (0b01110 << 1)
#define PAGE_64K	(0b00111 << 1)
#define PAGE_128K	(0b10000 << 1)
#define PAGE_256K	(0b10001 << 1)
#define PAGE_512K	(0b10010 << 1)
#define PAGE_1M		(0b10011 << 1)
#define PAGE_2M		(0b10100 << 1)
#define PAGE_4M		(0x15<<1) // (0b10101 << 1)
#define PAGE_8M		(0b10110 << 1)
#define PAGE_16M	(0b10111 << 1)
#define PAGE_32M	(0b11000 << 1)
#define PAGE_64M	(0x19<<1) // (0b11001 << 1)
#define PAGE_128M	(0x1a<<1) // (0b11010 << 1)
#define PAGE_256M	(0b11011 << 1)
#define PAGE_512M	(0b11100 << 1)
#define PAGE_1G		(0b11101 << 1)
#define PAGE_2G		(0b11110 << 1)
#define PAGE_4G		(0b11111 << 1)

#define ITCM_LOAD	(1<<19)
#define ITCM_ENABLE	(1<<18)
#define DTCM_LOAD	(1<<17)
#define DTCM_ENABLE	(1<<16)
#define DISABLE_TBIT	(1<<15)
#define ROUND_ROBIN	(1<<14)
#define ALT_VECTORS	(1<<13)
#define ICACHE_ENABLE	(1<<12)
#define BIG_ENDIAN	(1<<7)
#define DCACHE_ENABLE	(1<<2)
#define PROTECT_ENABLE	(1<<0)

#define __itcm_start (0x00000000)
#define __dtcm_start (0x00800000)

	//-------------------------------------------------------------------------
	// Region 0 - IO registers
	//-------------------------------------------------------------------------
	"ldr	r0,=( PAGE_64M | 0x04000000 | 1)	\n\t"
	"mcr	p15, 0, r0, c6, c0, 0 \n\t"

	//-------------------------------------------------------------------------
	// Region 1 - Main Memory
	//-------------------------------------------------------------------------
	"ldr	r0,=( PAGE_4M | 0x02000000 | 1)	 \n\t"
	"mcr	p15, 0, r0, c6, c1, 0 \n\t"

	//-------------------------------------------------------------------------
	// Region 2 - iwram
	//-------------------------------------------------------------------------
	"ldr	r0,=( PAGE_32K | 0x037F8000 | 1)	 \n\t"
	"mcr	p15, 0, r0, c6, c2, 0 \n\t"

	//-------------------------------------------------------------------------
	// Region 3 - DS Accessory (GBA Cart)
	//-------------------------------------------------------------------------
	"ldr	r0,=( PAGE_128M | 0x08000000 | 1)	 \n\t"
	"mcr	p15, 0, r0, c6, c3, 0 \n\t"

	//-------------------------------------------------------------------------
	// Region 4 - DTCM
	//-------------------------------------------------------------------------
	"ldr	r0,=__dtcm_start \n\t"
	"orr	r0,r0,#(PAGE_16K | 1) \n\t"
	"mcr	p15, 0, r0, c6, c4, 0 \n\t"

	//-------------------------------------------------------------------------
	// Region 5 - ITCM
	//-------------------------------------------------------------------------
	"ldr	r0,=__itcm_start \n\t"
	"orr	r0,r0,#(PAGE_32K | 1) \n\t"
	"mcr	p15, 0, r0, c6, c5, 0 \n\t"

	//-------------------------------------------------------------------------
	// Region 6 - System ROM
	//-------------------------------------------------------------------------
	"ldr	r0,=( PAGE_32K | 0xFFFF0000 | 1)	 \n\t"
	"mcr	p15, 0, r0, c6, c6, 0 \n\t"

	//-------------------------------------------------------------------------
	// Region 7 - non cacheable main ram
	//-------------------------------------------------------------------------
	"ldr	r0,=( PAGE_4M  | 0x02400000 | 1)	 \n\t"
	"mcr	p15, 0, r0, c6, c7, 0 \n\t"

	//-------------------------------------------------------------------------
	// Write buffer enable
	//-------------------------------------------------------------------------
	"ldr	r0,=0x6 ; 0b00000110 \n\t"
	"mcr	p15, 0, r0, c3, c0, 0 \n\t"

	//-------------------------------------------------------------------------
	// DCache & ICache enable
	//-------------------------------------------------------------------------
	"ldr	r0,=0x42 ; 0b01000010 \n\t"
	"mcr	p15, 0, r0, c2, c0, 0 \n\t"
	"mcr	p15, 0, r0, c2, c0, 1 \n\t"

	//-------------------------------------------------------------------------
	// IAccess
	//-------------------------------------------------------------------------
	"ldr	r0,=0x36636333 \n\t"
	"mcr	p15, 0, r0, c5, c0, 3 \n\t"

	//-------------------------------------------------------------------------
	// DAccess
	//-------------------------------------------------------------------------
	"ldr	r0,=0x36333333 \n\t"
	"mcr     p15, 0, r0, c5, c0, 2 \n\t"
	
// ------------------------------------------------
#endif
	"mov r0,#5 \n\t"
	"	str r0,[r10] \n\t"
	" \n\t"
	"0: \n\t"
	"ldr r0,[r10] \n\t"
	"cmp r0,#6 \n\t"
	"bne 0b \n\t"
	"	 \n\t"
	"	bx r11 \n\t"
	:::"memory"
	);
}
#endif

//static __attribute__ ((section (".rebootloader"))) __attribute__ ((noinline)) void resetMemory2load_ARM9_NoBIOS(void)
__attribute__ ((noinline)) void resetMemory2load_ARM9_NoBIOS(void)
{
  {
    u32 *ARM9_pCopyFrom=(u32*)IPC6->ARMInfo9.pCopyFrom;
    u32 *ARM9_pCopyTo=(u32*)IPC6->ARMInfo9.pCopyTo;
    u32 ARM9_CopySize=IPC6->ARMInfo9.CopySize;
/*
	  for(u32 idx=0;idx<ARM9_CopySize/4;idx++){
	    *ARM9_pCopyTo++=*ARM9_pCopyFrom++;
	  }
	  if(IPC6->RequestClearMemory==true){
      while(ARM9_pCopyTo!=(u32*)0x023ff000){
	      *ARM9_pCopyTo++=0;
	    }
	  }
*/
    //reset_MemCopy32CPU(ARM9_pCopyFrom,ARM9_pCopyTo,ARM9_CopySize);
    u32 idx=0;
	for(;idx<ARM9_CopySize/4;idx++){
		*ARM9_pCopyTo++=*ARM9_pCopyFrom++;
	}
    ARM9_pCopyTo+=ARM9_CopySize/4;
	//if(IPC6->RequestClearMemory==true){
	//    u32 size=0x023ff000-(u32)ARM9_pCopyTo;
    //  reset_MemSet32CPU(0,ARM9_pCopyTo,size);
    //}
  }

	IPC6->RESET=5;
	while(IPC6->RESET!=6){
		// ARM7Wait: Copy EWRAM to ARM7InternalMemory. and, Reset memory.
		//for(w=0;w<0x100;w++);
	}

  u32 ARM9ExecAddr=IPC6->ARMInfo9.ExecAddr;
	*(u32*)0x02fffe24=ARM9ExecAddr;
	asm("swi 0x00");
}


/*-------------------------------------------------------------------------
resetMemory1_ARM9
Clears the ARM9's DMA channels and resets video memory
Written by Darkain.
Modified by Chishm:
 * Changed MultiNDS specific stuff
--------------------------------------------------------------------------*/
static void vramset(void* dst, const u16 n, int len){ //use align2 macro then divide by 2
	u16* dst16 = (u16*)dst;
	for(;len>0;len--)*dst16++=n;
}

#define reset_MemSet32CPU(v,p,l) vramset(p,(u16)v,(l)>>1)
static void resetMemory1_ARM9 (void) 
{
	for (u32 i=0; i<32*1024; i+=4) {
//    (*(vu32*)(i+0x00000000)) = 0x00000000; // clear ITCM
	}
/*
	for (u32 i=0; i<32*1024; i+=4) {
    (*(vu32*)(i+0x00800000)) = 0x00000000; // clear DTCM
	}
*/

//	(*(vu32*)0x00803FFC) = 0;   //IRQ_HANDLER ARM9 version
//	(*(vu32*)0x00803FF8) = ~0;  //VBLANK_INTR_WAIT_FLAGS ARM9 version

 	register int i;
  
	// clear out ARM9 DMA channels
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

	VRAM_A_CR = VRAM_ENABLE | VRAM_A_MAIN_BG_0x06000000;
	VRAM_B_CR = VRAM_ENABLE | VRAM_B_MAIN_BG_0x06020000;
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_MAIN_BG_0x06040000;
	VRAM_D_CR = VRAM_ENABLE | VRAM_D_MAIN_BG_0x06060000;
  reset_MemSet32CPU(0, (void*)0x06000000,  128*4*1024);
	VRAM_A_CR = 0;
	VRAM_B_CR = 0;
	VRAM_C_CR = 0;
	VRAM_D_CR = 0;
	
	VRAM_E_CR = VRAM_ENABLE | VRAM_E_MAIN_BG;
  reset_MemSet32CPU(0, (void*)0x06000000,  64*1024);
	VRAM_E_CR = 0;
	VRAM_F_CR = VRAM_ENABLE | VRAM_F_MAIN_BG;
  reset_MemSet32CPU(0, (void*)0x06000000,  16*1024);
	VRAM_F_CR = 0;
	VRAM_G_CR = VRAM_ENABLE | VRAM_G_MAIN_BG;
  reset_MemSet32CPU(0, (void*)0x06000000,  16*1024);
	VRAM_G_CR = 0;
	
	VRAM_H_CR = VRAM_ENABLE | VRAM_H_SUB_BG;
  reset_MemSet32CPU(0, (void*)0x06200000,  32*1024);
	VRAM_H_CR = 0;
	VRAM_I_CR = VRAM_ENABLE | VRAM_I_SUB_BG_0x06208000;
  reset_MemSet32CPU(0, (void*)0x06208000,  16*1024);
	VRAM_I_CR = 0;
	
  VRAM_CR = 0x80808080;
  reset_MemSet32CPU(0, BG_PALETTE, 2*1024);
  BG_PALETTE[0] = 0xFFFF;
  reset_MemSet32CPU(0, OAM,     2*1024);
  reset_MemSet32CPU(0, (void*)0x04000000, 0x56+2); //clear main display registers
  reset_MemSet32CPU(0, (void*)0x04001000, 0x56+2); //clear sub  display registers
	
	REG_DISPSTAT=0;
	videoSetMode(0);
	videoSetModeSub(0);
	VRAM_A_CR = 0;
	VRAM_B_CR = 0;
	VRAM_C_CR = 0;
	VRAM_D_CR = 0;
	VRAM_E_CR = 0;
	VRAM_F_CR = 0;
	VRAM_G_CR = 0;
	VRAM_H_CR = 0;
	VRAM_I_CR = 0;
	VRAM_CR   = 0x03000000;
	REG_POWERCNT  = 0x820F;

	//set shared ram to ARM7
	WRAM_CR = 0x03;
}

