/*
 * filter.h
 *
 * Description:	 TTAv1 filter functions
 * Developed by: Alexander Djourik <ald@true-audio.com>
 *               Pavel Zhilin <pzh@true-audio.com>
 *
 * Copyright (c) 2004 True Audio Software. All rights reserved.
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the True Audio Software nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FILTER_H
#define FILTER_H

///////// Filter Settings //////////
//static int flt_set[3] = {10, 9, 10};

#define fs_shift (10) // for 8bit
#define fs_round (1 << (fs_shift - 1)) // for 8bit

void __attribute__ ((noinline)) hybrid_filter_8bit (fltst *fs, int *in) {
  int *pfs_error = &fs->error;
  int *pA = fs->dl;
  int *pB = fs->qm;
  int *pM = fs->dx;

#define REG_pfs_error "%0"
#define REG_pA "%1"
#define REG_pB "%2"
#define REG_pM "%3"
#define REG_in "%4"

#define REG_sum "r12"

  asm volatile(
    "mov "REG_sum",#(1 << (10 - 1)) \n" // = fs_round
    
    "ldr r4,["REG_pfs_error"] \n"
    "cmps r4,#0 \n"
    
    // --------------------------------
    "ttac8bit_p0: \n"
    
    "bne ttac8bit_p1 \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    "ldmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "smlabb "REG_sum",r4,r8,"REG_sum" \n"
    "smlabb "REG_sum",r5,r9,"REG_sum" \n"
    "smlabb "REG_sum",r6,r10,"REG_sum" \n"
    "smlabb "REG_sum",r7,r11,"REG_sum" \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    "ldmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "smlabb "REG_sum",r4,r8,"REG_sum" \n"
    "smlabb "REG_sum",r5,r9,"REG_sum" \n"
    "smlabb "REG_sum",r6,r10,"REG_sum" \n"
    "smlabb "REG_sum",r7,r11,"REG_sum" \n"
    
    "add "REG_pM",#8*4 \n"
    
    "b ttac8bit_end \n"
    
    // --------------------------------
    "ttac8bit_p1: \n"
    
    "bgt ttac8bit_p2 \n"
    
    "ldmia "REG_pB",{r8,r9,r10,r11} \n"
    "ldmia "REG_pM"!,{r4,r5,r6,r7} \n"
    
    "sub r8,r4 \n"
    "sub r9,r5 \n"
    "sub r10,r6 \n"
    "sub r11,r7 \n"
    
    "stmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    
    "smlabb "REG_sum",r4,r8,"REG_sum" \n"
    "smlabb "REG_sum",r5,r9,"REG_sum" \n"
    "smlabb "REG_sum",r6,r10,"REG_sum" \n"
    "smlabb "REG_sum",r7,r11,"REG_sum" \n"
    
    "ldmia "REG_pB",{r8,r9,r10,r11} \n"
    "ldmia "REG_pM"!,{r4,r5,r6,r7} \n"
    
    "sub r8,r4 \n"
    "sub r9,r5 \n"
    "sub r10,r6 \n"
    "sub r11,r7 \n"
    
    "stmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    
    "smlabb "REG_sum",r4,r8,"REG_sum" \n"
    "smlabb "REG_sum",r5,r9,"REG_sum" \n"
    "smlabb "REG_sum",r6,r10,"REG_sum" \n"
    "smlabb "REG_sum",r7,r11,"REG_sum" \n"
    
    "b ttac8bit_end \n"
    
    // --------------------------------
    "ttac8bit_p2: \n"
    
    "ldmia "REG_pB",{r8,r9,r10,r11} \n"
    "ldmia "REG_pM"!,{r4,r5,r6,r7} \n"
    
    "add r8,r4 \n"
    "add r9,r5 \n"
    "add r10,r6 \n"
    "add r11,r7 \n"
    
    "stmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    
    "smlabb "REG_sum",r4,r8,"REG_sum" \n"
    "smlabb "REG_sum",r5,r9,"REG_sum" \n"
    "smlabb "REG_sum",r6,r10,"REG_sum" \n"
    "smlabb "REG_sum",r7,r11,"REG_sum" \n"
    
    "ldmia "REG_pB",{r8,r9,r10,r11} \n"
    "ldmia "REG_pM"!,{r4,r5,r6,r7} \n"
    
    "add r8,r4 \n"
    "add r9,r5 \n"
    "add r10,r6 \n"
    "add r11,r7 \n"
    
    "stmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    
    "smlabb "REG_sum",r4,r8,"REG_sum" \n"
    "smlabb "REG_sum",r5,r9,"REG_sum" \n"
    "smlabb "REG_sum",r6,r10,"REG_sum" \n"
    "smlabb "REG_sum",r7,r11,"REG_sum" \n"
    
    // --------------------------------
    "ttac8bit_end: \n"
    
    "sub "REG_pA",#4*4 \n"
    "ldmia "REG_pA"!,{r8,r9,r10,r11} \n"
    
    "asr r8,#30 \n orr r8,#1 \n"
    "asr r9,#30 \n orr r9,#1 \n lsl r9,#1 \n"
    "asr r10,#30 \n orr r10,#1 \n lsl r10,#1 \n"
    "asr r11,#30 \n orr r11,#1 \n lsl r11,#2 \n"
    
    "sub "REG_pM",#7*4 \n"
    "ldmia "REG_pM",{r4,r5,r6,r7} \n"
    "sub "REG_pM",#1*4 \n"
    "stmia "REG_pM",{r4,r5,r6,r7,r8,r9,r10,r11} \n"
    
    "ldr r4,["REG_in"] \n"
    "str r4,["REG_pfs_error"] \n"
    
    "add r11,r4,"REG_sum",asr #10 \n" // fs_shift
    "str r11,["REG_in"] \n"
    
    "sub "REG_pA",#(8-1)*4 \n"
    "ldmia "REG_pA",{r4,r5,r6,r7,r8,r9,r10} \n"
    
    "sub r10,r11,r10 \n"
    "sub r9,r10,r9 \n"
    "sub r8,r9,r8 \n"
    
    "sub "REG_pA",#1*4 \n"
    "stmia "REG_pA"!,{r4,r5,r6,r7,r8,r9,r10,r11} \n"
    
    : : "r"(pfs_error),"r"(pA),"r"(pB),"r"(pM),"r"(in)
    : "r4","r5","r6","r7","r8","r9","r10","r11","r12"
  );
  
#undef REG_pfs_error
#undef REG_pA
#undef REG_pB
#undef REG_pM
#undef REG_in

#undef REG_sum

}

#undef fs_shift
#undef fs_round

#define fs_shift (9) // for 16bit
#define fs_round (1 << (fs_shift - 1)) // for 16bit

void __attribute__ ((noinline)) hybrid_filter_16bit (fltst *fs, int *in) {
  int *pfs_error = &fs->error;
  int *pA = fs->dl;
  int *pB = fs->qm;
  int *pM = fs->dx;

#define REG_pfs_error "%0"
#define REG_pA "%1"
#define REG_pB "%2"
#define REG_pM "%3"
#define REG_in "%4"

#define REG_sum "r12"

  asm volatile(
    "mov "REG_sum",#(1 << (9 - 1)) \n" // = fs_round
    
    "ldr r4,["REG_pfs_error"] \n"
    "cmps r4,#0 \n"
    
    // --------------------------------
    "ttac16bit_p0: \n"
    
    "bne ttac16bit_p1 \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    "ldmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "mla "REG_sum",r4,r8,"REG_sum" \n"
    "mla "REG_sum",r5,r9,"REG_sum" \n"
    "mla "REG_sum",r6,r10,"REG_sum" \n"
    "mla "REG_sum",r7,r11,"REG_sum" \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    "ldmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "mla "REG_sum",r4,r8,"REG_sum" \n"
    "mla "REG_sum",r5,r9,"REG_sum" \n"
    "mla "REG_sum",r6,r10,"REG_sum" \n"
    "mla "REG_sum",r7,r11,"REG_sum" \n"
    
    "add "REG_pM",#8*4 \n"
    
    "b ttac16bit_end \n"
    
    // --------------------------------
    "ttac16bit_p1: \n"
    
    "bgt ttac16bit_p2 \n"
    
    "ldmia "REG_pB",{r8,r9,r10,r11} \n"
    "ldmia "REG_pM"!,{r4,r5,r6,r7} \n"
    
    "sub r8,r4 \n"
    "sub r9,r5 \n"
    "sub r10,r6 \n"
    "sub r11,r7 \n"
    
    "stmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    
    "mla "REG_sum",r4,r8,"REG_sum" \n"
    "mla "REG_sum",r5,r9,"REG_sum" \n"
    "mla "REG_sum",r6,r10,"REG_sum" \n"
    "mla "REG_sum",r7,r11,"REG_sum" \n"
    
    "ldmia "REG_pB",{r8,r9,r10,r11} \n"
    "ldmia "REG_pM"!,{r4,r5,r6,r7} \n"
    
    "sub r8,r4 \n"
    "sub r9,r5 \n"
    "sub r10,r6 \n"
    "sub r11,r7 \n"
    
    "stmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    
    "mla "REG_sum",r4,r8,"REG_sum" \n"
    "mla "REG_sum",r5,r9,"REG_sum" \n"
    "mla "REG_sum",r6,r10,"REG_sum" \n"
    "mla "REG_sum",r7,r11,"REG_sum" \n"
    
    "b ttac16bit_end \n"
    
    // --------------------------------
    "ttac16bit_p2: \n"
    
    "ldmia "REG_pB",{r8,r9,r10,r11} \n"
    "ldmia "REG_pM"!,{r4,r5,r6,r7} \n"
    
    "add r8,r4 \n"
    "add r9,r5 \n"
    "add r10,r6 \n"
    "add r11,r7 \n"
    
    "stmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    
    "mla "REG_sum",r4,r8,"REG_sum" \n"
    "mla "REG_sum",r5,r9,"REG_sum" \n"
    "mla "REG_sum",r6,r10,"REG_sum" \n"
    "mla "REG_sum",r7,r11,"REG_sum" \n"
    
    "ldmia "REG_pB",{r8,r9,r10,r11} \n"
    "ldmia "REG_pM"!,{r4,r5,r6,r7} \n"
    
    "add r8,r4 \n"
    "add r9,r5 \n"
    "add r10,r6 \n"
    "add r11,r7 \n"
    
    "stmia "REG_pB"!,{r8,r9,r10,r11} \n"
    
    "ldmia "REG_pA"!,{r4,r5,r6,r7} \n"
    
    "mla "REG_sum",r4,r8,"REG_sum" \n"
    "mla "REG_sum",r5,r9,"REG_sum" \n"
    "mla "REG_sum",r6,r10,"REG_sum" \n"
    "mla "REG_sum",r7,r11,"REG_sum" \n"
    
    // --------------------------------
    "ttac16bit_end: \n"
    
    "sub "REG_pA",#4*4 \n"
    "ldmia "REG_pA"!,{r8,r9,r10,r11} \n"
    
    "asr r8,#30 \n orr r8,#1 \n"
    "asr r9,#30 \n orr r9,#1 \n lsl r9,#1 \n"
    "asr r10,#30 \n orr r10,#1 \n lsl r10,#1 \n"
    "asr r11,#30 \n orr r11,#1 \n lsl r11,#2 \n"
    
    "sub "REG_pM",#7*4 \n"
    "ldmia "REG_pM",{r4,r5,r6,r7} \n"
    "sub "REG_pM",#1*4 \n"
    "stmia "REG_pM",{r4,r5,r6,r7,r8,r9,r10,r11} \n"
    
    "ldr r4,["REG_in"] \n"
    "str r4,["REG_pfs_error"] \n"
    
    "add r11,r4,"REG_sum",asr #9 \n" // fs_shift
    "str r11,["REG_in"] \n"
    
    "sub "REG_pA",#(8-1)*4 \n"
    "ldmia "REG_pA",{r4,r5,r6,r7,r8,r9,r10} \n"
    
    "sub r10,r11,r10 \n"
    "sub r9,r10,r9 \n"
    "sub r8,r9,r8 \n"
    
    "sub "REG_pA",#1*4 \n"
    "stmia "REG_pA"!,{r4,r5,r6,r7,r8,r9,r10,r11} \n"
    
    : : "r"(pfs_error),"r"(pA),"r"(pB),"r"(pM),"r"(in)
    : "r4","r5","r6","r7","r8","r9","r10","r11","r12"
  );
  
#undef REG_pfs_error
#undef REG_pA
#undef REG_pB
#undef REG_pM
#undef REG_in

#undef REG_sum

}

#undef fs_shift
#undef fs_round

static inline void filter_init (fltst *fs) {
  MemSet32CPU(0,fs,sizeof(fltst));
}

#endif  /* FILTER_H */
