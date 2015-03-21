.section .text
	.global __main
@---------------------------------------------------------------------------------
_start:
@---------------------------------------------------------------------------------
	@ about 16*1msec wait.
	mov r0,#0x1000*1
waitloop:
  	subs r0,#1
  	bne waitloop
	ldr	r0, =0x04000208			@ REG_IME = 0@
	mov r1, #0
	strh	r1, [r0]
	
 @ wait for DMA
waitfordma:
  	ldr r0,=0x4000000
  	ldr r1,[r0,#0xb8]
  	tst r1,#0x80000000
  	bne waitfordma
	ldr r1,[r0,#0xc4]
	tst r1,#0x80000000
	bne waitfordma
  	ldr r1,[r0,#0xd0]
  	tst r1,#0x80000000
  	bne waitfordma
  	ldr r1,[r0,#0xdc]
  	tst r1,#0x80000000
  	bne waitfordma
  
  	@ Clear DMA
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
  
	mov	r0, #0x13		@ Switch to SVC Mode
	msr	cpsr_cxsf, r0
	ldr	sp, =0x0380fb00		@ Set SVC stack

	mov	r0, #0x12		@ Switch to IRQ Mode
	msr	cpsr_cxsf, r0
	ldr	sp, =0x0380fa00		@ Set IRQ stack

	mov	r0, #0x1F		@ Switch to System Mode
	msr	cpsr_cxsf, r0
	ldr	sp, =0x0380f900		@ Set user stack (not use this parametor. see init.s)
        
					@ import label to __main
	.global __main
	ldr	pc,=__main                @ branch to C Library entry 

	
