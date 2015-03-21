
        AREA   INIT946TCM, CODE, READONLY   ; name this block of code

__itcm_start equ 0x01ff0000
__dtcm_start equ 0x0b000000

PAGE_4K EQU 2_01011 << 1
PAGE_8K EQU 2_01100 << 1
PAGE_16K EQU 2_01101 << 1
PAGE_32K EQU 2_01110 << 1
PAGE_64K EQU 2_00111 << 1
PAGE_128K EQU 2_10000 << 1
PAGE_256K EQU 2_10001 << 1
PAGE_512K EQU 2_10010 << 1
PAGE_1M EQU 2_10011 << 1
PAGE_2M EQU 2_10100 << 1
PAGE_4M EQU 2_10101 << 1
PAGE_8M EQU 2_10110 << 1
PAGE_16M EQU 2_10111 << 1
PAGE_32M EQU 2_11000 << 1
PAGE_64M EQU 2_11001 << 1
PAGE_128M EQU 2_11010 << 1
PAGE_256M EQU 2_11011 << 1
PAGE_512M EQU 2_11100 << 1
PAGE_1G EQU 2_11101 << 1
PAGE_2G EQU 2_11110 << 1
PAGE_4G EQU 2_11111 << 1

SB1_BITSET EQU 2_01111000

ITCM_LOAD_MODE EQU 1<<19
DTCM_LOAD_MODE EQU 1<<17
ITCM_ENABLE EQU 1<<18
DTCM_ENABLE EQU 1<<16
LD_INTERWORK_DISABLE EQU 1<<15
CACHE_ROUND_ROBIN EQU 1<<14
CACHE_PSEUDO_RANDOM EQU 0
EXCEPT_VEC_UPPER EQU 1<<13
EXCEPT_VEC_LOWER EQU 0
ICACHE_ENABLE EQU 1<<12
DCACHE_ENABLE EQU 1<<2
LITTLE_ENDIAN EQU 0
BIG_ENDIAN EQU 1<<7
PROTECT_UNIT_ENABLE EQU 1<<0

;---------------------------------------------------------------------------------
; _start
;---------------------------------------------------------------------------------
; about 16*1msec wait.
	mov r0,#0x10000*1
waitloop
  	subs r0,#1
  	bne waitloop
	ldr	r0, =0x04000208			; REG_IME = 0;
	mov r1, #0
	strh	r1, [r0]
	
  	; wait for DMA
waitfordma
  	ldr r0,=0x4000000
  	ldr r1,[r0,#0xb8]
  	tsts r1,#0x80000000
  	bne waitfordma
  	ldr r1,[r0,#0xc4]
  	tsts r1,#0x80000000
  	bne waitfordma
  	ldr r1,[r0,#0xd0]
  	tsts r1,#0x80000000
  	bne waitfordma
  	ldr r1,[r0,#0xdc]
  	tsts r1,#0x80000000
  	bne waitfordma
  
  	; Clear DMA
  	ldr r0,=0x40000B0
  	mov r1,#0
  	str r1,[r0],#4
  	str r1,[r0],#4
  	str r1,[r0],#4
  	str r1,[r0],#4
  	str r1,[r0],#4
  	str r1,[r0],#4
 	str r1,[r0],#4
  	str r1,[r0],#4
  	str r1,[r0],#4
  	str r1,[r0],#4
  	str r1,[r0],#4
  	str r1,[r0],#4
  
  	; set sensible stacks to allow bios call

	mov	r0, #0x13		; Switch to SVC Mode
	msr	cpsr_cxsf, r0
	mov	r1,#0x03000000
	sub	r1,r1,#0x1000
	mov	sp,r1
	mov	r0, #0x1F		; Switch to System Mode
	msr	cpsr_cxsf, r0
	sub	r1,r1,#0x100
	mov	sp,r1
	
	IMPORT __init_mpu_setup
	ldr	r3, =__init_mpu_setup
	blx	r3
	
    ;-------------------------------------------------------------------------
    ;initialize stack pointer
    ;-------------------------------------------------------------------------
	mov	r0, #0x13		; Switch to SVC Mode
	msr	cpsr_cxsf, r0
	ldr	sp, =__dtcm_start+0x3fe0		; Set SVC stack

	mov	r0, #0x12		; Switch to IRQ Mode
	msr	cpsr_cxsf, r0
	ldr	sp, =__dtcm_start+0x3f80		; Set IRQ stack

	mov	r0, #0x1F		; Switch to System Mode
	msr	cpsr_cxsf, r0
	ldr	sp, =__dtcm_start+0x3f00		; Set user stack (not use this parametor. see init.s)
  
  	;-------------------------------------------------------------------------
  	;---- clear memory
  	;-------------------------------------------------------------------------
	; DTCM (16KB)
	mov	r0, #0
	ldr	r1, =__dtcm_start
	mov	r2, #0x4000
	bl	ClearMem32CPU

	; BG/OBJ palette (1KB)
	mov	r0, #0
	ldr	r1, =0x05000000
	mov	r2, #0x400
	bl	ClearMem32CPU

	; OAM (1KB)
	mov	r0, #0x0200
	ldr	r1, =0x07000000
	mov	r2, #0x400
	bl	ClearMem32CPU
  
	IMPORT  __main                     ; import label to __main
	LDR pc,=__main                     ; branch to C Library entry 

	;-------------------------------------------------------------------------
	; ClearMem32CPU( register u32 data, register void *destp, register u32 size )
	;-------------------------------------------------------------------------
ClearMem32CPU
	add     r12, r1, r2             ; r12: destEndp = destp + size
ClrLoop
	cmp     r1, r12                 ; while (destp < destEndp)
	stmltia r1!, {r0}               ; *((vu32 *)(destp++)) = data
	blt     ClrLoop
	bx      lr
  	
	END
	
