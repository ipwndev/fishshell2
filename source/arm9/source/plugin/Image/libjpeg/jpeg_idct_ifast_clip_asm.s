@ /*
@  * jidctfst.c
@  *
@  * Copyright (C) 1994-1998, Thomas G. Lane.
@  * This file is part of the Independent JPEG Group's software.
@  * For conditions of distribution and use, see the accompanying README file.
@  *
@  * This file contains a fast, not so accurate integer implementation of the
@  * inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
@  * must also perform dequantization of the input coefficients.
@  *
@  * A 2-D IDCT can be done by 1-D IDCT on each column followed by 1-D IDCT
@  * on each row (or vice versa, but it's more convenient to emit a row at
@  * a time).  Direct algorithms are also available, but they are much more
@  * complex and seem not to be any faster when reduced to code.
@  *
@  * This implementation is based on Arai, Agui, and Nakajima's algorithm for
@  * scaled DCT.  Their original paper (Trans. IEICE E-71(11):1095) is in
@  * Japanese, but the algorithm is described in the Pennebaker & Mitchell
@  * JPEG textbook (see REFERENCES section in file README).  The following code
@  * is based directly on figure 4-8 in P&M.
@  * While an 8-point DCT cannot be done in less than 11 multiplies, it is
@  * possible to arrange the computation so that many of the multiplies are
@  * simple scalings of the final outputs.  These multiplies can then be
@  * folded into the multiplications or divisions by the JPEG quantization
@  * table entries.  The AA&N method leaves only 5 multiplies and 29 adds
@  * to be done in the DCT itself.
@  * The primary disadvantage of this method is that with fixed-point math,
@  * accuracy is lost due to imprecise representation of the scaled
@  * quantization values.  The smaller the quantization table entry, the less
@  * precise the scaled value, so this implementation does worse with high-
@  * quality-setting files than with low-quality ones.
@  */

@ /*
@  * This module is specialized to the case DCTSIZE = 8.
@  */

@ /* Scaling decisions are generally the same as in the LL&M algorithm@
@  * see jidctint.c for more details.  However, we choose to descale
@  * (right shift) multiplication products as soon as they are formed,
@  * rather than carrying additional fractional bits into subsequent additions.
@  * This compromises accuracy slightly, but it lets us save a few shifts.
@  * More importantly, 16-bit arithmetic is then adequate (for 8-bit samples)
@  * everywhere except in the multiplications proper@ this saves a good deal
@  * of work on 16-bit-int machines.
@  *
@  * The dequantized coefficients are not integers because the AA&N scaling
@  * factors have been incorporated.  We represent them scaled up by PASS1_BITS,
@  * so that the first and second IDCT rounds have the same input scaling.
@  * For 8-bit JSAMPLEs, we choose IFAST_SCALE_BITS = PASS1_BITS so as to
@  * avoid a descaling shift@ this compromises accuracy rather drastically
@  * for small quantization table entries, but it saves a lot of shifts.
@  * For 12-bit JSAMPLEs, there's no hope of using 16x16 multiplies anyway,
@  * so we use a much larger scaling factor to preserve accuracy.
@  *
@  * A final compromise is to represent the multiplicative constants to only
@  * 8 fractional bits, rather than 13.  This saves some shifting work on some
@  * machines, and may also reduce the cost of multiplication (since there
@  * are fewer one-bits in the constants).
@  */

@ /*
@  * Perform dequantization and inverse DCT on one block of coefficients.
@  */


.section .ITCM_libimg_jpeg

  .global jpeg_idct_ifast_clip_asm
jpeg_idct_ifast_clip_asm:
    
REG_blockptr .req r14

REG_count_pass1 .req r11
REG_FIX .req r12

REG_slot_pquant .req r10

  .macro   savestack_ppLineBuf reg
  str \reg,[sp,#1*4]
  .endm
  
  .macro   loadstack_ppLineBuf reg
  ldr \reg,[sp,#1*4]
  .endm
    
@ //    bkpt 0 @ // デバッグ用ブレークポイント設定
    
    stmfd sp!, {r4,r5,r6,r7,r8,r9,r10,r11,r12,REG_blockptr}
    
    @ // ---------------------------------------
    @ // quant not trans zigzag
    
    mov r12,#64
    
pass0_loopstart:
    
    ldmia r2!,{r4,r5,r6,r7} @ // load coef_block 16bit
    
    ldmia r1!,{r8,r9,r10,r11} @ // load quantptr 32bit
    smulbb r8,r4,r8
    smultb r9,r4,r9
    smulbb r10,r5,r10
    smultb r11,r5,r11
    stmia r0!,{r8,r9,r10,r11} @ // save workspace
    
    ldmia r1!,{r8,r9,r10,r11} @ // load quantptr 32bit
    smulbb r8,r6,r8
    smultb r9,r6,r9
    smulbb r10,r7,r10
    smultb r11,r7,r11
    stmia r0!,{r8,r9,r10,r11} @ // save workspace
    
    subs r12,#8
    bne pass0_loopstart
    
    sub r0,r0,#64*4
    
    @ // ---------------------------------------
    mov REG_blockptr,r0
    
    sub sp,#4*4
    
    savestack_ppLineBuf r3
    
    @ /* Pass 1: process columns from input, store into work array. */
    
    mov REG_count_pass1,#8
    
pass1_loopstart:
    
      @ // ---------------------------------------
pass1_loaddata:
      
      ldr r0,[REG_blockptr,#0*8*4]
      ldr r1,[REG_blockptr,#2*8*4]
      ldr r2,[REG_blockptr,#4*8*4]
      ldr r3,[REG_blockptr,#6*8*4]
      b pass1_EvenPart
      
      cmp r1,#0
      bne pass1_EvenPart
      cmpeq r2,#0
      cmpeq r3,#0
      bne pass1_EvenPart
      
      ldr r4,[REG_blockptr,#1*8*4]
      ldr r5,[REG_blockptr,#3*8*4]
      ldr r6,[REG_blockptr,#5*8*4]
      ldr r7,[REG_blockptr,#7*8*4]
      
      cmp r4,#0
      cmpeq r5,#0
      cmpeq r6,#0
      cmpeq r7,#0
      bne pass1_EvenPart
      
pass1_loaddata_end:
      
      str r0,[REG_blockptr,#0*8*4]
      str r0,[REG_blockptr,#1*8*4]
      str r0,[REG_blockptr,#2*8*4]
      str r0,[REG_blockptr,#3*8*4]
      str r0,[REG_blockptr,#4*8*4]
      str r0,[REG_blockptr,#5*8*4]
      str r0,[REG_blockptr,#6*8*4]
      str r0,[REG_blockptr,#7*8*4]
      
      add REG_blockptr,#4
      
      subs REG_count_pass1,#1
      
      bne pass1_loopstart
      b pass1_loopend
      
      @ // ---------------------------------------
pass1_EvenPart:
      
      @ // r0=tmp0 r1=tmp1 r2=tmp2 r3=tmp3
      @ // r4=tmp10 r5=tmp11 r6=tmp12 r7=tmp13
      
pass1ev_phase3:
      
      add r4,r0,r2
      sub r5,r0,r2
      
pass1ev_phases53:
      
      add r7,r1,r3
      
pass1ev_phases53_2c4:
      
      @ // FIX_1_414213562
      ldr REG_FIX,=0x016a09
      sub r6,r1,r3
      smulwb r6,REG_FIX,r6
      
pass1ev_phases2:
      
      add r0,r4,r7
      @ // インターロック対策
      sub r6,r7
      sub r3,r4,r7
      add r1,r5,r6
      sub r2,r5,r6
      
pass1_EvenPart_end:
      
      @ // ---------------------------------------
pass1_OddPart:
      
      ldr r4,[REG_blockptr,#1*8*4]
      ldr r5,[REG_blockptr,#3*8*4]
      ldr r6,[REG_blockptr,#5*8*4]
      ldr r7,[REG_blockptr,#7*8*4]
      
      @ // r2,r3=can not used.
      @ // r4=z10 r5=z11 r9=z12 r10=z13
      @ // r4=tmp4 r5=tmp5 r6=tmp6 r7=tmp7
      @ // r8=tmp10
      
pass1od_phase6:
      
      add r10,r6,r5
      sub r8,r6,r5
      add r5,r4,r7
      sub r9,r4,r7
      mov r4,r8
      
pass1od_phases5:
      
      add r7,r5,r10
      
pass1od_phases5_2c4:
      
      @ // r10=z13 changed to r10=tmp11
      
      @ // FIX_1_414213562
      ldr REG_FIX,=0x016a09
      sub r10,r5,r10
      smulwb r10,REG_FIX,r10
      
      @ // r5=z11 changed to r5=z5
      
pass1od_phases5_2c2:
      
      @ // FIX_1_847759065
      ldr REG_FIX,=0x01d906
      add r5,r4,r9
      smulwb r5,REG_FIX,r5
      
pass1od_phases5_2c2pc6:
pass1od_phases5_2c2mc6:
      
      @ // r6=REG_FIX sub
      
      @ // r4=z10 changed to r4=tmp4
      
      @ // FIX_-2_613125930
      @ // FIX_1_082392200
      ldr REG_FIX,=0xfffd630b
      ldr r6,=0x011517
      smulwb r4,REG_FIX,r4
      smulwb r8,r6,r9
      @ // インターロック対策
      add r4,r5
      @ // r4 mixed tmp6==tmp12
      
pass1od_phases2:
      
      sub r6,r4,r7
      @ // インターロック対策
      sub r8,r5
      sub r5,r10,r6
      add r4,r8,r5
      
pass1_OddPart_end:
      
      @ // ---------------------------------------
pass1_savedata:
      
      add r8,r0,r7
      str r8,[REG_blockptr,#0*8*4]
      add r8,r1,r6
      str r8,[REG_blockptr,#1*8*4]
      add r8,r2,r5
      str r8,[REG_blockptr,#2*8*4]
      sub r8,r3,r4
      str r8,[REG_blockptr,#3*8*4]
      add r8,r3,r4
      str r8,[REG_blockptr,#4*8*4]
      sub r8,r2,r5
      str r8,[REG_blockptr,#5*8*4]
      sub r8,r1,r6
      str r8,[REG_blockptr,#6*8*4]
      sub r8,r0,r7
      str r8,[REG_blockptr,#7*8*4]
      
pass1_savedata_end:
      
      @ // ---------------------------------------
      
      add REG_blockptr,#4
      
      subs REG_count_pass1,#1
      
      bne pass1_loopstart
@ //      b pass1_loopend
      
pass1_loopend:
    
    sub REG_blockptr,#8*4
    
    @ /* Pass 2: process rows from work array, store into output array. */
    
@ //    bkpt 0 @ // デバッグ用ブレークポイント設定
    
    @ // pass.2 define macros --------------------------------------------------------------

  .macro   macro_pass2_stgx_loaddata
  ldmia REG_blockptr,{r0,r1,r2,r3,r4,r5,r6,r7}
  .endm

@ // -----------

  .macro   macro_pass2_stgx_EvenPart
  @ /* r0=ws[0], r2=ws[2], r4=ws[4], r6=ws[6] */
  @ /* r0=tmp0, r2=tmp1, r4=tmp2, r6=tmp3 */
  @ /* r8=tmp10, r9=tmp11, r10=tmp12, r11=tmp13 */
  
  add r8,r0,r4
  sub r9,r0,r4
  
  add r11,r2,r6
  
  @ /* FIX_1_414213562 */
  ldr REG_FIX,=0x016a09
  sub r10,r2,r6
  smulwb r10,REG_FIX,r10
  
  add r0,r8,r11
  @ /* インターロック対策 */
  sub r10,r10,r11
  sub r6,r8,r11
  add r2,r9,r10
  sub r4,r9,r10
  .endm

@ // -----------

  .macro   macro_pass2_stgx_OddPart
  @ /* r1=ws[1], r3=ws[3], r5=ws[5], r7=ws[7] */
  @ /* r1=tmp4, r3=tmp5, r5=tmp6, r7=tmp7 */
  @ /* r9=tmp10, r10=tmp11, r11=tmp12 */
  @ /* r1=z11, r3=z12, r5=z13, r5=z5, r8=z10 */
  
  sub r8,r5,r3 @ /* z10 */
  add r5,r5,r3 @ /* z13 */
  sub r3,r1,r7 @ /* z12 */
  add r1,r1,r7 @ /* z11 */
  
  @ /* pass2_stgx_phase5: */
  
  add r7,r1,r5
  
  @ /* pass2_stgx_phase5_2c4: */
  
  @ /* FIX_1_414213562 */
  ldr REG_FIX,=0x016a09
  sub r10,r1,r5
  smulwb r10,REG_FIX,r10
  
  @ /* pass2_stgx_phase5_2c2: */
  
  @ /* FIX_1_847759065 */
  ldr REG_FIX,=0x01d906
  add r5,r8,r3
  smulwb r5,REG_FIX,r5
  
  @ /* pass2_stgx_phase5_2c2mc6: */
  @ /* pass2_stgx_od_phases5_2c2pc6: */
  
  @ /* r1=REG_FIX sub */
  
  @ /* FIX_1_082392200 */
  @ /* FIX_-2_613125930 */
  ldr REG_FIX,=0x011517
  ldr r1,=0xfffd630b
  smulwb r9,REG_FIX,r3
  smulwb r11,r1,r8
  @ /* インターロック対策 */
  sub r9,r9,r5
  @ /* インターロック対策 */
  add r11,r11,r5
  
  @ /* pass2_stgx_phase2: */
  
  sub r5,r11,r7
  sub r3,r10,r5
  add r1,r9,r3
  .endm

@ // -----------

  .macro   macro_pass2_stgx_savedata
  
  @ /* asr tmp,#5 */ /* PASS1_BITS+3 = 5 */
  
  @ /* r0=tmp0, r2=tmp1, r4=tmp2, r6=tmp3 */
  @ /* r1=tmp4, r3=tmp5, r5=tmp6, r7=tmp7 */
  @ /* r10=tmpw0, r7=tmpw7, r8=tmpw1, r5=tmpw6 */
  @ /* r11=tmpw2, r3=tmpw5, r9=tmpw3, r1=tmpw4 */
  
  add r10,r0,r7
  sub r7,r0,r7
  add r8,r2,r5
  sub r5,r2,r5
  add r11,r4,r3
  sub r3,r4,r3
  sub r9,r6,r1
  add r1,r6,r1
  
  @ /* r10=tmpw0->r0, r7=tmpw7->r7, r8=tmpw1->r1, r5=tmpw6->r6 */
  @ /* r11=tmpw2->r2, r3=tmpw5->r5, r9=tmpw3->r3, r1=tmpw4->r4 */
  
  ldr r12,=cliptable_top
  
  asr r0,r10,#5
  ldrb r0,[r12,r0]
  asr r4,r1,#5
  ldrb r4,[r12,r4]
  asr r1,r8,#5
  ldrb r1,[r12,r1]
  asr r2,r11,#5
  ldrb r2,[r12,r2]
  asr r6,r5,#5
  ldrb r6,[r12,r6]
  asr r5,r3,#5
  ldrb r5,[r12,r5]
  asr r3,r9,#5
  ldrb r3,[r12,r3]
  asr r7,r7,#5
  ldrb r7,[r12,r7]
  
  add REG_blockptr,#8*4
  
  loadstack_ppLineBuf r8
  orr r0,r0,r1,lsl #8
  orr r0,r0,r2,lsl #16
  orr r0,r0,r3,lsl #24
  ldr r9,[r8],#4
  orr r4,r4,r5,lsl #8
  orr r4,r4,r6,lsl #16
  orr r4,r4,r7,lsl #24
  savestack_ppLineBuf r8
  
  str r0,[r9,#0*4]
  str r4,[r9,#1*4]
  .endm
  
    @ // pass.2 stage.0 --------------------------------------------------------------
pass2_stg0_loopstart:
    
      @ // ---------------------------------------
pass2_stg0_loaddata:
      macro_pass2_stgx_loaddata
      
      @ // ---------------------------------------
pass2_stg0_EvenPart:
      macro_pass2_stgx_EvenPart
      
      @ // ---------------------------------------
pass2_stg0_OddPart:
      macro_pass2_stgx_OddPart
      
      @ // ---------------------------------------
pass2_stg0_savedata:
      macro_pass2_stgx_savedata
      
pass2_stg0_loopend:
    
    @ // pass.2 stage.1 --------------------------------------------------------------
pass2_stg1_loopstart:
    
      @ // ---------------------------------------
pass2_stg1_loaddata:
      macro_pass2_stgx_loaddata
      
      @ // ---------------------------------------
pass2_stg1_EvenPart:
      macro_pass2_stgx_EvenPart
      
      @ // ---------------------------------------
pass2_stg1_OddPart:
      macro_pass2_stgx_OddPart
      
      @ // ---------------------------------------
pass2_stg1_savedata:
      macro_pass2_stgx_savedata
      
pass2_stg1_loopend:
    
    @ // pass.2 stage.2 --------------------------------------------------------------
pass2_stg2_loopstart:
    
      @ // ---------------------------------------
pass2_stg2_loaddata:
      macro_pass2_stgx_loaddata
      
      @ // ---------------------------------------
pass2_stg2_EvenPart:
      macro_pass2_stgx_EvenPart
      
      @ // ---------------------------------------
pass2_stg2_OddPart:
      macro_pass2_stgx_OddPart
      
      @ // ---------------------------------------
pass2_stg2_savedata:
      macro_pass2_stgx_savedata
      
pass2_stg2_loopend:
    
    @ // pass.2 stage.3 --------------------------------------------------------------
pass2_stg3_loopstart:
    
      @ // ---------------------------------------
pass2_stg3_loaddata:
      macro_pass2_stgx_loaddata
      
      @ // ---------------------------------------
pass2_stg3_EvenPart:
      macro_pass2_stgx_EvenPart
      
      @ // ---------------------------------------
pass2_stg3_OddPart:
      macro_pass2_stgx_OddPart
      
      @ // ---------------------------------------
pass2_stg3_savedata:
      macro_pass2_stgx_savedata
      
pass2_stg3_loopend:
    
    @ // pass.2 stage.4 --------------------------------------------------------------
pass2_stg4_loopstart:
    
      @ // ---------------------------------------
pass2_stg4_loaddata:
      macro_pass2_stgx_loaddata
      
      @ // ---------------------------------------
pass2_stg4_EvenPart:
      macro_pass2_stgx_EvenPart
      
      @ // ---------------------------------------
pass2_stg4_OddPart:
      macro_pass2_stgx_OddPart
      
      @ // ---------------------------------------
pass2_stg4_savedata:
      macro_pass2_stgx_savedata
      
pass2_stg4_loopend:
    
    @ // pass.2 stage.5 --------------------------------------------------------------
pass2_stg5_loopstart:
    
      @ // ---------------------------------------
pass2_stg5_loaddata:
      macro_pass2_stgx_loaddata
      
      @ // ---------------------------------------
pass2_stg5_EvenPart:
      macro_pass2_stgx_EvenPart
      
      @ // ---------------------------------------
pass2_stg5_OddPart:
      macro_pass2_stgx_OddPart
      
      @ // ---------------------------------------
pass2_stg5_savedata:
      macro_pass2_stgx_savedata
      
pass2_stg5_loopend:
    
    @ // pass.2 stage.6 --------------------------------------------------------------
pass2_stg6_loopstart:
    
      @ // ---------------------------------------
pass2_stg6_loaddata:
      macro_pass2_stgx_loaddata
      
      @ // ---------------------------------------
pass2_stg6_EvenPart:
      macro_pass2_stgx_EvenPart
      
      @ // ---------------------------------------
pass2_stg6_OddPart:
      macro_pass2_stgx_OddPart
      
      @ // ---------------------------------------
pass2_stg6_savedata:
      macro_pass2_stgx_savedata
      
pass2_stg6_loopend:
    
    @ // pass.2 stage.7 --------------------------------------------------------------
pass2_stg7_loopstart:
    
      @ // ---------------------------------------
pass2_stg7_loaddata:
      macro_pass2_stgx_loaddata
      
      @ // ---------------------------------------
pass2_stg7_EvenPart:
      macro_pass2_stgx_EvenPart
      
      @ // ---------------------------------------
pass2_stg7_OddPart:
      macro_pass2_stgx_OddPart
      
      @ // ---------------------------------------
pass2_stg7_savedata:
      macro_pass2_stgx_savedata
      
pass2_stg7_loopend:
    
    @ // --------------------------
pass2_stg8_loopstart:
    @ // --------------------------
    
@ //    sub REG_blockptr,#8*8*4
    
    add sp,#4*4
    
@ //    bkpt 0 @ // デバッグ用ブレークポイント設定
    
    ldmfd sp!, {r4,r5,r6,r7,r8,r9,r10,r11,r12,pc}
    
cliptable:
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    .byte 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07
    .byte 0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F
    .byte 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17
    .byte 0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F
    .byte 0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27
    .byte 0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F
    .byte 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37
    .byte 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F
    .byte 0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47
    .byte 0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F
    .byte 0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57
    .byte 0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F
    .byte 0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67
    .byte 0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F
    .byte 0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77
    .byte 0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F
cliptable_top:
    .byte 0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87
    .byte 0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F
    .byte 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97
    .byte 0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F
    .byte 0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7
    .byte 0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF
    .byte 0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7
    .byte 0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF
    .byte 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7
    .byte 0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF
    .byte 0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7
    .byte 0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF
    .byte 0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7
    .byte 0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF
    .byte 0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7
    .byte 0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    .byte 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF

