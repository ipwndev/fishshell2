@This code comes from a post by CaitSith2 at gbadev.org - THANKS!!
@
@Code to dump the complete Nintendo DS ARM7 bios, including the
@first 0x1204 bytes residing in the secure area.
@
@The ARM7 bios has read protection where 0x(Word)[FFFF(Half word)[FF(Byte)[FF]]]
@is returned, if any reads are attempted while PC is outside the arm7 bios range.
@
@Additionally, if the PC is outside the 0x0000 - 0x1204 range, that range of the bios
@is completely locked out from reading.

	@AREA     globals,CODE,READONLY
	.section .text
	@ void readBios (u8* dest, u32 src, u32 size)
	.global readBios
	
readBios:
	stmfd sp!, {r4-r7, lr}	@Even though we don't use R7, the code we are jumping to is going trash R7, therefore, we must save it.
	movs	r5, r1		@src
	subs	r1, r2, #1	@size
	movs	r2, r0		@dest
	ldr	r0, callMe	@The code that will be made to read the full bios resides here.
loop:
	movs	r6, #0x12	@We Subtract 12 from the location we wish to read
	subs	r3, r1, r6	@because the code at 0x5EC is LDRB R3, [R3,#0x12]
	adds	r3, r3, r5
	adr	r6, ret
	stmfd	sp!,{r2-r6}		@The line of code at 0x5EE is POP {R2,R4,R6,R7,PC}
	bx	r0
	nop
.align	2
ret:
	strb	r3, [r2, r1]	@Store the read byte contained in r3, to SRAM.
	subs	r1, #1			@Subtract 1
	bpl	loop			@And branch as long as R1 doesn't roll into -1 (0xFFFFFFFF).
	ldmfd	sp!,	{r4-r7}			@Restore the saved registers
	ldmfd	sp!,	{r3}			@and return.
	bx	r3
.align	2
callMe:
	.word	0x5ed
        
	@END

@The exact code that resides at 0x5EC (secure area range) of the arm7 bios.
@ROM:000005EC 9B 7C                       LDRB    R3, [R3,#0x12]
@ROM:000005EE D4 BD                       POP     {R2,R4,R6,R7,PC} 
