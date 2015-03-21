@//---------------------------------------------------------------------------------
@    AREA     globals,CODE,READONLY
@//---------------------------------------------------------------------------------
.section .text
  .global VRAMWriteCache_Enable
VRAMWriteCache_Enable:
  mov r0,#0x80 @// =0b00000111
  mcr p15, 0, r0, c3, c0, 0
  bx lr

@//---------------------------------------------------------------------------------

  .global VRAMWriteCache_Disable
VRAMWriteCache_Disable:
  mov r0,#0x80 @// =0b00000110
  mcr p15, 0, r0, c3, c0, 0
  bx lr

  @END