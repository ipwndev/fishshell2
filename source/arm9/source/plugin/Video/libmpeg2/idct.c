/*
 * idct.c
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 * See http://libmpeg2.sourceforge.net/ for updates.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Modified for use with MPlayer, see libmpeg-0.4.0.diff for the exact changes.
 * detailed CVS changelog at http://www.mplayerhq.hu/cgi-bin/cvsweb.cgi/main/
 * $Id: idct.c,v 1.12 2005/02/19 02:32:12 diego Exp $
 */

#include "config.h"

#include <stdlib.h>
#include "inttypes.h"

#include "mpeg2.h"
#include "attributes.h"
#include "mpeg2_internal.h"

#include "_console.h"
#include "_consoleWriteLog.h"

#include "memtool.h"

#define inline __inline__ __attribute__((always_inline))

#define f16W1 (2841<<16) /* sqrt (2) * cos (1 * pi / 16) */
#define f16W2 (2676<<16) /* sqrt (2) * cos (2 * pi / 16) */
#define f16W3 (2408<<16) /* sqrt (2) * cos (3 * pi / 16) */
#define f16W5 (1609<<16) /* sqrt (2) * cos (5 * pi / 16) */
#define f16W6 (1108<<16) /* sqrt (2) * cos (6 * pi / 16) */
#define f16W7 (565<<16)  /* sqrt (2) * cos (7 * pi / 16) */

#if 0
#define BUTTERFLY(t0,t1,W0,W1,d0,d1)	\
do {					\
    t0 = W0 * d0 + W1 * d1;		\
    t1 = W0 * d1 - W1 * d0;		\
} while (0)
#else
#define BUTTERFLY(t0,t1,W0,W1,d0,d1)	\
do {					\
    int tmp = W0 * (d0 + d1);		\
    t0 = tmp + (W1 - W0) * d1;		\
    t1 = tmp - (W1 + W0) * d0;		\
} while (0)
#endif

#define asmBUTTERFLY(_t0,_t1,_W0,_W1,_d0,_d1,_tmp) \
asm { \
  add _tmp,_d0,_d1; \
  smulwb _tmp,_W0,_tmp; \
  smlawb _t0,_W1-_W0,_d1,_tmp; \
  smlawb _t1,-(_W1+_W0),_d0,_tmp; \
}

#define asmBUTTERFLYbb(_t0,_t1,_W0,_W1,_d0,_d1,_tmp)	\
asm {					\
  smulwb _t0,_W1,_d1; \
  smlawb _t0,_W0,_d0,_t0; \
  smulwb _t1,_W1,_d0; \
  rsb _t1,_t1,0; \
  smlawb _t1,_W0,_d1,_t1; \
}

#define asmBUTTERFLYtb(_t0,_t1,_W0,_W1,_d0,_d1,_tmp)	\
asm {					\
  smulwb _t0,_W1,_d1; \
  smlawt _t0,_W0,_d0,_t0; \
  smulwt _t1,_W1,_d0; \
  rsb _t1,_t1,0; \
  smlawb _t1,_W0,_d1,_t1; \
}

#define asmBUTTERFLYbt(_t0,_t1,_W0,_W1,_d0,_d1,_tmp)	\
asm {					\
  smulwt _t0,_W1,_d1; \
  smlawb _t0,_W0,_d0,_t0; \
  smulwb _t1,_W1,_d0; \
  rsb _t1,_t1,0; \
  smlawt _t1,_W0,_d1,_t1; \
}

#define asmBUTTERFLYtt(_t0,_t1,_W0,_W1,_d0,_d1,_tmp)	\
asm {					\
  smulwt _t0,_W1,_d1; \
  smlawt _t0,_W0,_d0,_t0; \
  smulwt _t1,_W1,_d0; \
  rsb _t1,_t1,0 \
  smlawt _t1,_W0,_d1,_t1; \
}

/* idct main entry point  */
//void (*mpeg2_idct_copy)(int16_t * block, uint8_t * dest, int stride);
//void (*mpeg2_idct_add)(int last, int16_t * block, uint8_t * dest, int stride);

/*
 * In legal streams, the IDCT output should be between -384 and +384.
 * In corrupted streams, it is possible to force the IDCT output to go
 * to +-3826 - this is the worst case for a column IDCT where the
 * column inputs are 16-bit values.
 */
extern uint8_t mpeg2_clip8[256*3];
DATA_IN_MTCM_VAR uint8_t mpeg2_clip8[256*3];

#if 1
static CODE_IN_ITCM_DPG inline void idct_row32_asm (int16_t * const block, int32_t * const ws)
{
  /* shortcut */
  /*
  bool f=true;
  
  {
    int tmp;
#if 1
    if (block[1] | ((int32_t *)block)[1] | ((int32_t *)block)[2] | ((int32_t *)block)[3]) f=false;
#else
    u32 tmp1,tmp2,tmp3,tmp4;
    asm {
      ldr tmp1,[block]
      ldr tmp2,[block,#4]
      ldr tmp3,[block,#8]
      ldr tmp4,[block,#12]
      orr tmp,tmp2,tmp1,lsr #16
      orr tmp,tmp,tmp3
      orrs tmp,tmp,tmp4
      movne f,false
    }
#endif
  }
  
  if(f==true){
#if 1
    int32_t tmp = block[0] >> 1;
    ws[0] = tmp;
    ws[1] = tmp;
    ws[2] = tmp;
    ws[3] = tmp;
    ws[4] = tmp;
    ws[5] = tmp;
    ws[6] = tmp;
    ws[7] = tmp;
#else
    u32 tmp;
    asm {
      ldrsh tmp,[block]
    
      mov tmp,tmp,asr #1
      str tmp,[ws]
      str tmp,[ws,#1*4]
      str tmp,[ws,#2*4]
      str tmp,[ws,#3*4]
      str tmp,[ws,#4*4]
      str tmp,[ws,#5*4]
      str tmp,[ws,#6*4]
      str tmp,[ws,#7*4]
    }
#endif    
    return;
  }
    
#define shift1 (11)
#define shift2 (12)
  
  int d0, d1, d2, d3;
  int a0, a1, a2, a3, b0, b1, b2, b3;
  int t0, t1, t2, t3;
  int tmp;
  
#if 0
  asm {
    ldrsh d0,[block,#0*2]
    ldrsh d1,[block,#1*2]
    ldrsh d2,[block,#2*2]
    ldrsh d3,[block,#3*2]
  }
#else
  d0 = block[0];
  d1 = block[1];
  d2 = block[2];
  d3 = block[3];
#endif
  
#if 0
  asm {
    mov d0,d0,lsl #shift1
    add d0,d0,#2048
    add t0,d0,d2,lsl #shift1
    sub t1,d0,d2,lsl #shift1
  }
#else
  d0 = (d0 << shift1) + 2048;
  t0 = d0 + (d2<<shift1);
  t1 = d0 - (d2<<shift1);
#endif
  
  asmBUTTERFLY (t2, t3, f16W6, f16W2, d3, d1, tmp);

#if 0
  asm {
    mov t0,t0,asr #shift2
    mov t1,t1,asr #shift2
    add a0,t0,t2,asr #shift2
    add a1,t1,t3,asr #shift2
    sub a2,t1,t3,asr #shift2
    sub a3,t0,t2,asr #shift2
  }
#else
  t0>>=shift2;
  t1>>=shift2;
  a0 = t0 + (t2>>shift2);
  a1 = t1 + (t3>>shift2);
  a2 = t1 - (t3>>shift2);
  a3 = t0 - (t2>>shift2);
#endif

#if 0
  asm {
    ldr d0,[block,#4*2]
    ldr d2,[block,#6*2]
  }
#else
  d0 = ((int32_t *)block)block[4];
  d2 = ((int32_t *)block)block[6];
#endif
  
  asmBUTTERFLYtb (t0, t1, f16W7, f16W1, d2, d0, tmp);
  asmBUTTERFLYtb (t2, t3, f16W3, f16W5, d0, d2, tmp);
  
#if 0
  asm {
    add b0,t0,t2
    add b3,t1,t3
    sub t0,t0,t2
    sub t1,t1,t3
  }
#else
  b0 = t0 + t2;
  b3 = t1 + t3;
  t0 -= t2;
  t1 -= t3;
#endif
  
#if 0
  asm {
    mov tmp,#181<<6
    mov t0,t0,asl #2
    add b1,t0,t1,asl #2
    smulwb b1,b1,tmp
    sub b2,t0,t1,asl #2
    smulwb b2,b2,tmp
  }
#else
    b1 = ((t0 + t1) >> 8) * 181;
    b2 = ((t0 - t1) >> 8) * 181;
#endif

  {
    int tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;
#if 0
    asm {
      add tmp0,a0,b0,asr #shift2
      add tmp1,a1,b1,asr #shift2
      add tmp2,a2,b2,asr #shift2
      add tmp3,a3,b3,asr #shift2
      sub tmp4,a3,b3,asr #shift2
      sub tmp5,a2,b2,asr #shift2
      sub tmp6,a1,b1,asr #shift2
      sub tmp7,a0,b0,asr #shift2
    }
#else
    tmp0=a0+(b0>>shift2);
    tmp1=a1+(b1>>shift2);
    tmp2=a2+(b2>>shift2);
    tmp3=a3+(b3>>shift2);
    tmp4=a3-(b3>>shift2);
    tmp5=a2-(b2>>shift2);
    tmp6=a1-(b1>>shift2);
    tmp7=a0-(b0>>shift2);
#endif
#if 0
    asm {
      str tmp0,[ws,#0*4]
      str tmp1,[ws,#1*4]
      str tmp2,[ws,#2*4]
      str tmp3,[ws,#3*4]
      str tmp4,[ws,#4*4]
      str tmp5,[ws,#5*4]
      str tmp6,[ws,#6*4]
      str tmp7,[ws,#7*4]
    }
#else
    ws[0]=tmp0;
    ws[1]=tmp1;
    ws[2]=tmp2;
    ws[3]=tmp3;
    ws[4]=tmp4;
    ws[5]=tmp5;
    ws[6]=tmp6;
    ws[7]=tmp7;
#endif
  }*/
  
#undef shift1
#undef shift2
}
#endif

#if 1
static CODE_IN_ITCM_DPG inline void idct_col32_asm (int32_t * const ws)
{/*
#define shift1 (11)
#define shift2 (17)
  
  int d0, d1, d2, d3;
  int a0, a1, a2, a3, b0, b1, b2, b3;
  int t0, t1, t2, t3;
  int tmp;
  
#if 0
  asm {
    ldr d0,[ws,#8*0*4]
    ldr d1,[ws,#8*1*4]
    ldr d2,[ws,#8*2*4]
    ldr d3,[ws,#8*3*4]
  }
#else
  d0 = ws[8*0];
  d1 = ws[8*1];
  d2 = ws[8*2];
  d3 = ws[8*3];
#endif
  
#if 0
  asm {
    mov d0,d0,lsl #shift1
    add d0,d0,#65536
    add t0,d0,d2,lsl #shift1
    sub t1,d0,d2,lsl #shift1
  }
#else
  d0 = (d0 << shift1) + 65536;
  t0 = d0 + (d2<<shift1);
  t1 = d0 - (d2<<shift1);
#endif
  
  asmBUTTERFLY (t2, t3, f16W6, f16W2, d3, d1, tmp);
  
#if 0
  asm {
    add a0,t0,t2
    add a1,t1,t3
    sub a2,t1,t3
    sub a3,t0,t2
  }
#else
  a0 = t0 + t2;
  a1 = t1 + t3;
  a2 = t1 - t3;
  a3 = t0 - t2;
#endif

#if 0
  asm {
    ldr d0,[ws,#8*4*4]
    ldr d1,[ws,#8*5*4]
    ldr d2,[ws,#8*6*4]
    ldr d3,[ws,#8*7*4]
  }
#else
  d0 = ws[8*4];
  d1 = ws[8*5];
  d2 = ws[8*6];
  d3 = ws[8*7];
#endif
  
  asmBUTTERFLY (t0, t1, f16W7, f16W1, d3, d0, tmp);
  asmBUTTERFLY (t2, t3, f16W3, f16W5, d1, d2, tmp);
  
#if 0
  asm {
    add b0,t0,t2
    add b3,t1,t3
    sub t0,t0,t2
    sub t1,t1,t3
  }
#else
  b0 = t0 + t2;
  b3 = t1 + t3;
  t0 -= t2;
  t1 -= t3;
#endif
  
#if 0
  asm {
    mov tmp,#181<<6
    mov t0,t0,asl #2
    add b1,t0,t1,asl #2
    smulwb b1,b1,tmp
    sub b2,t0,t1,asl #2
    smulwb b2,b2,tmp
  }
#else
    b1 = ((t0 + t1) >> 8) * 181;
    b2 = ((t0 - t1) >> 8) * 181;
#endif

  {
    int tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;
#if 0
    asm {
      add tmp0,a0,b0
      mov tmp0,tmp0,asr #shift2
      add tmp1,a1,b1
      mov tmp1,tmp1,asr #shift2
      add tmp2,a2,b2
      mov tmp2,tmp2,asr #shift2
      add tmp3,a3,b3
      mov tmp3,tmp3,asr #shift2
      sub tmp4,a3,b3
      mov tmp4,tmp4,asr #shift2
      sub tmp5,a2,b2
      mov tmp5,tmp5,asr #shift2
      sub tmp6,a1,b1
      mov tmp6,tmp6,asr #shift2
      sub tmp7,a0,b0
      mov tmp7,tmp7,asr #shift2
    }
#else
    tmp0=(a0+b0)>>shift2;
    tmp1=(a1+b1)>>shift2;
    tmp2=(a2+b2)>>shift2;
    tmp3=(a3+b3)>>shift2;
    tmp4=(a3-b3)>>shift2;
    tmp5=(a2-b2)>>shift2;
    tmp6=(a1-b1)>>shift2;
    tmp7=(a0-b0)>>shift2;
#endif
#if 0
    asm {
      str tmp0,[ws,#0*4*8]
      str tmp1,[ws,#1*4*8]
      str tmp2,[ws,#2*4*8]
      str tmp3,[ws,#3*4*8]
      str tmp4,[ws,#4*4*8]
      str tmp5,[ws,#5*4*8]
      str tmp6,[ws,#6*4*8]
      str tmp7,[ws,#7*4*8]
    }
#else
    ws[0*8]=tmp0;
    ws[1*8]=tmp1;
    ws[2*8]=tmp2;
    ws[3*8]=tmp3;
    ws[4*8]=tmp4;
    ws[5*8]=tmp5;
    ws[6*8]=tmp6;
    ws[7*8]=tmp7;
#endif
  }
  */
#undef shift1
#undef shift2
}
#endif

#if 0
asm CODE_IN_ITCM_DPG void idct_col_fullasm (int16_t * const block)
{
  ; r13=sp r14=lr s15=pc
  PUSH {r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}

block RN r0

cnt RN r14
  mov cnt,#8
  
idct_col_asm_loopstart

    ; stage.2
    
s2d0 RN r9 ;r5
s2d1 RN r10 ;r6
s2d2 RN r11 ;r7
s2d3 RN r12 ;r8

s2t0 RN r1
s2t1 RN r2
s2t2 RN r3
s2t3 RN r4

s0b0 RN s2d0
s0b1 RN s2d1
s0b2 RN s2d2
s0b3 RN s2d3

s2tmp RN r5

s2f16W7 RN r5
s2f16Wp1m7 RN r6
s2f16Wm1m7 RN r7
s2f16W3 RN r5
s2f16Wp5m3 RN r6
s2f16Wm5m3 RN r7
s2f16tmp RN r8

    ldrsh s2d0,[block,#8*4*2]
    ldrsh s2d1,[block,#8*5*2]
    ldrsh s2d2,[block,#8*6*2]
    ldrsh s2d3,[block,#8*7*2]
    
    adr s2f16W7,lbl_f16W7packed
    add s2f16tmp,s2d3,s2d0
    ldmia s2f16W7,{s2f16W7,s2f16Wp1m7,s2f16Wm1m7}
    smulwb s2f16tmp,s2f16W7,s2f16tmp
    smlawb s2t0,s2f16Wp1m7,s2d0,s2f16tmp
    smlawb s2t1,s2f16Wm1m7,s2d3,s2f16tmp
    
    adr s2f16W3,lbl_f16W3packed
    add s2f16tmp,s2d1,s2d2
    ldmia s2f16W3,{s2f16W3,s2f16Wp5m3,s2f16Wm5m3}
    smulwb s2f16tmp,s2f16W3,s2f16tmp
    smlawb s2t2,s2f16Wp5m3,s2d2,s2f16tmp
    smlawb s2t3,s2f16Wm5m3,s2d1,s2f16tmp
    
    add s0b0,s2t0,s2t2
    add s0b3,s2t1,s2t3
    
    sub s2t0,s2t0,s2t2
    sub s2t1,s2t1,s2t3
    
    mov s2tmp,#181
    mov s2t0,s2t0,asr #8
    add s0b1,s2t0,s2t1,asr #8
    smulbb s0b1,s0b1,s2tmp
    sub s0b2,s2t0,s2t1,asr #8
    smulbb s0b2,s0b2,s2tmp
    
;    PUSH {s0b0,s0b1,s0b2,s0b3}

    ; stage.1
    
s1d0 RN r1
s1d1 RN r2
s1d2 RN r3
s1d3 RN r4

s0a0 RN s1d0
s0a1 RN s1d1
s0a2 RN s1d2
s0a3 RN s1d3

s1t0 RN r5
s1t1 RN r6
s1t2 RN s1d0
s1t3 RN s1d2

s1f16W2 RN r7
s1f16W6 RN r8

    ldrsh s1d0,[block,#8*0*2]
    ldrsh s1d1,[block,#8*1*2]
    ldrsh s1d2,[block,#8*2*2]
    ldrsh s1d3,[block,#8*3*2]
    add s1d0,s1d0,#32
    mov s1d0,s1d0,lsl #8
    add s1t0,s1d0,s1d2,lsl #8
    sub s1t1,s1d0,s1d2,lsl #8
    
    ldr s1f16W2,lbl_f16W2
    ldr s1f16W6,lbl_f16W6
    smulwb s1t2,s1f16W2,s1d1
    smulwb s1t3,s1f16W2,s1d3
    smlawb s1t2,s1f16W6,s1d3,s1t2
    rsb s1t3,s1t3,#0
    smlawb s1t3,s1f16W6,s1d1,s1t3
    
    add s0a1,s1t1,s1t3
    sub s0a2,s1t1,s1t3
    sub s0a3,s1t0,s1t2
    add s0a0,s1t0,s1t2
    
;    PUSH {s0a0,s0a1,s0a2,s0a3}
    
s0tmp RN r5

;    POP {s0a0,s0a1,s0a2,s0a3}
;    POP {s0b0,s0b1,s0b2,s0b3}
    
    add s0tmp,s0a0,s0b0
    mov s0tmp,s0tmp,asr #14
    strh s0tmp,[block,#8*0*2]
    add s0tmp,s0a1,s0b1
    mov s0tmp,s0tmp,asr #14
    strh s0tmp,[block,#8*1*2]
    add s0tmp,s0a2,s0b2
    mov s0tmp,s0tmp,asr #14
    strh s0tmp,[block,#8*2*2]
    add s0tmp,s0a3,s0b3
    mov s0tmp,s0tmp,asr #14
    strh s0tmp,[block,#8*3*2]
    sub s0tmp,s0a3,s0b3
    mov s0tmp,s0tmp,asr #14
    strh s0tmp,[block,#8*4*2]
    sub s0tmp,s0a2,s0b2
    mov s0tmp,s0tmp,asr #14
    strh s0tmp,[block,#8*5*2]
    sub s0tmp,s0a1,s0b1
    mov s0tmp,s0tmp,asr #14
    strh s0tmp,[block,#8*6*2]
    sub s0tmp,s0a0,s0b0
    mov s0tmp,s0tmp,asr #14
    strh s0tmp,[block,#8*7*2]
  
  add block,#2
  
  subs cnt,#1
  bne idct_col_asm_loopstart
  
  POP {r4,r5,r6,r7,r8,r9,r10,r11,r12,pc}
  
lbl_f16W1
  DCD 2841<<(16-3)
lbl_f16W2
  DCD 2676<<(16-3)
lbl_f16W3
  DCD 2408<<(16-3)
lbl_f16W5
  DCD 1609<<(16-3)
lbl_f16W6
  DCD 1108<<(16-3)
lbl_f16W7
  DCD 565<<(16-3)

lbl_f16W7packed
  DCD 565<<(16-3)
  DCD (2841<<(16-3))-(565<<(16-3))
  DCD (-2841<<(16-3))-(565<<(16-3))
  
lbl_f16W3packed
  DCD 2408<<(16-3)
  DCD (1609<<(16-3))-(2408<<(16-3))
  DCD (-1609<<(16-3))-(2408<<(16-3))
}
#endif
/*
asm CODE_IN_ITCM_DPG void mpeg2_idct_copy32_CopyAndClearBlock(int const *ws,uint8_t * dest, int stride,int16_t * block)
{
#define block copy32_pixels_clamped_asm_tr0
#define dest copy32_pixels_clamped_asm_tr1
#define stride copy32_pixels_clamped_asm_tr2
#define pclip copy32_pixels_clamped_asm_tr3
#define cnt copy32_pixels_clamped_asm_tr4
#define write0 copy32_pixels_clamped_asm_tr5
#define write1 copy32_pixels_clamped_asm_tr6
#define tmp0 copy32_pixels_clamped_asm_tr7
#define tmp1 copy32_pixels_clamped_asm_tr8
#define tmp2 copy32_pixels_clamped_asm_tr9

ws RN r0
dest RN r1
stride RN r2
block RN r3

  ; r13=sp r14=lr s15=pc
  PUSH {r4,r5,r6,r7,r8,r9,lr}

pclip RN r4

  IMPORT mpeg2_clip8
  ldr pclip,=mpeg2_clip8
  add pclip,pclip,#256

cnt RN r5
  mov cnt,#8

write0 RN r6
write1 RN r7
tmp0 RN r8
tmp1 RN r9
tmp2 RN lr

copy32_CopyAndClearBlock_Start

  MACRO
  copy32_CopyAndClearBlock_Make $write,$tmp0,$tmp1,$tmp2
  ldmia ws!,{$write,$tmp0,$tmp1,$tmp2}
  ldrb $write,[pclip,$write] ; for 0bit
  ldrb $tmp0,[pclip,$tmp0] ; for 8bit
  ldrb $tmp1,[pclip,$tmp1] ; for 16bit
  ldrb $tmp2,[pclip,$tmp2] ; for 24bit
  orr $write,$write,$tmp0,lsl #8 ; for 8bit
  orr $write,$write,$tmp1,lsl #16 ; for 16bit
  orr $write,$write,$tmp2,lsl #24 ; for 24bit
  MEND

  copy32_CopyAndClearBlock_Make write0,tmp0,tmp1,tmp2
  copy32_CopyAndClearBlock_Make write1,tmp0,tmp1,tmp2

  stmia dest,{write0,write1}
  add dest,stride

  subs cnt,#1
  bne copy32_CopyAndClearBlock_Start
  
  mov r4,#0
  mov r5,#0
  mov r6,#0
  mov r7,#0
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  
  POP {r4,r5,r6,r7,r8,r9,pc}

#undef block
#undef dest
#undef stride
#undef pclip
#undef cnt
#undef write0
#undef write1
#undef tmp0
#undef tmp1
#undef tmp2
}

asm CODE_IN_ITCM_DPG void mpeg2_idct_add_AddDC_asm(s32 DC,unsigned char *dest, const int stride)
{
#define DC mpeg2_idct_add_AddDC_asm_tr0
#define dest mpeg2_idct_add_AddDC_asm_tr1
#define stride mpeg2_idct_add_AddDC_asm_tr2
#define cnt mpeg2_idct_add_AddDC_asm_tr3
#define tmp mpeg2_idct_add_AddDC_asm_tr4
#define pclip mpeg2_idct_add_AddDC_asm_tr5
#define read0 mpeg2_idct_add_AddDC_asm_tr6
#define read1 mpeg2_idct_add_AddDC_asm_tr7
#define write0 mpeg2_idct_add_AddDC_asm_tr8
#define write1 mpeg2_idct_add_AddDC_asm_tr9

DC RN r0
dest RN r1
stride RN r2

  PUSH {r4,r5,r6,r7,r8,r9,lr}

cnt RN r3
  mov cnt,#8

tmp RN r4

pclip RN DC

  IMPORT mpeg2_clip8
  ldr tmp,=mpeg2_clip8
  add DC,DC,#256
  add pclip,tmp,DC

read0 RN r5
read1 RN r6
write0 RN r7
write1 RN r8

AddDC_Start
  MACRO
  AddDC_Make $read,$write,$tmp1,$tmp2,$tmp3
  and $write,$read,#0xff // for 0bit
  ldrb $write,[pclip,$write] // for 0bit
  mov $tmp1,$read,lsl #32-8-8 // for 8bit
  ldrb $tmp1,[pclip,$tmp1,lsr #24] // for 8bit
  mov $tmp2,$read,lsl #32-16-8 // for 16bit
  ldrb $tmp2,[pclip,$tmp2,lsr #24] // for 16bit
  ldrb $tmp3,[pclip,$read,lsr #24] // for 24bit
  orr $write,$write,$tmp1,lsl #8 // for 8bit
  orr $write,$write,$tmp2,lsl #16 // for 16bit
  orr $write,$write,$tmp3,lsl #24 // for 24bit
  MEND

  ldmia dest,{read0,read1}
  AddDC_Make read0,write0,tmp,write1,r9
  AddDC_Make read1,write1,tmp,read0,r9
  stmia dest,{write0,write1}
  add dest,stride
  
  subs cnt,#1
  bne AddDC_Start
  
  POP {r4,r5,r6,r7,r8,r9,pc}

#undef DC
#undef dest
#undef stride
#undef cnt
#undef tmp
#undef pclip
#undef read0
#undef read1
#undef write0
#undef write1
}

asm CODE_IN_ITCM_DPG void add32_pixels_clamped_asm(int *ws, unsigned char *dest, int stride, short *block)
{
#define block add32_pixels_clamped_asm_tr0
#define dest add32_pixels_clamped_asm_tr1
#define stride add32_pixels_clamped_asm_tr2
#define pclip add32_pixels_clamped_asm_tr3
#define cnt add32_pixels_clamped_asm_tr4
#define read0 add32_pixels_clamped_asm_tr5
#define read1 add32_pixels_clamped_asm_tr6
#define write0 add32_pixels_clamped_asm_tr7
#define write1 add32_pixels_clamped_asm_tr8
#define blockdata add32_pixels_clamped_asm_tr9
#define tmp add32_pixels_clamped_asm_tr10

  ; r13=sp r14=lr s15=pc
  PUSH {r4,r5,r6,r7,r8,r9,r10,lr}

ws RN r0
dest RN r1
stride RN r2
block RN r3

pclip RN r10

  IMPORT mpeg2_clip8
  ldr pclip,=mpeg2_clip8
  add pclip,pclip,#256

cnt RN r4
  mov cnt,#8

read0 RN r5
read1 RN r6
write0 RN r7
write1 RN r8
blockdata RN r9
tmp RN lr

Add32PixelClamped_Start

  MACRO
  Add32PixelClamped_Make $read,$write,$tmp0,$tmp1
  and $tmp0,$read,#0xff ; for 0bit
  add $tmp0,$tmp0,blockdata ; for 0bit
  ldr blockdata,[ws],#4 ; for 8bit
  ldrb $write,[pclip,$tmp0] ; for 0bit
  mov $tmp0,$read,lsl #32-8-8 ; for 8bit
  add $tmp0,blockdata,$tmp0,lsr #32-8 ; for 8bit
  ldr blockdata,[ws],#4 ; for 16bit
  ldrb $tmp0,[pclip,$tmp0] ; for 8bit
  mov $tmp1,$read,lsl #32-8-16 ; for 16bit
  add $tmp1,blockdata,$tmp1,lsr #32-8 ; for 16bit
  ldr blockdata,[ws],#4 ; for 24bit
  ldrb $tmp1,[pclip,$tmp1] ; for 16bit
  orr $write,$write,$tmp0,lsl #8 ; for 8bit
  add $tmp0,blockdata,$read,lsr #24 ; for 24bit
  ldrb $tmp0,[pclip,$tmp0] ; for 24bit
  orr $write,$write,$tmp1,lsl #16 ; for 16bit
  
  MEND

  MACRO
  Add32PixelClamped_Make2 $read,$write,$tmp0,$tmp1
  orr $write,$write,$tmp0,lsl #24 ; for 24bit
  MEND
  
  ldr blockdata,[ws],#4 ; for 0bit
  ldmia dest,{read0,read1}
  Add32PixelClamped_Make read0,write0,tmp,write1
  ldr blockdata,[ws],#4 ; for next 0bit
  Add32PixelClamped_Make2 read0,write0,tmp,write1

  Add32PixelClamped_Make read1,write1,tmp,read0
  subs cnt,#1
  Add32PixelClamped_Make2 read1,write1,tmp,read0

  stmia dest,{write0,write1}
  add dest,stride

  bne Add32PixelClamped_Start
  
  mov r4,#0
  mov r5,#0
  mov r6,#0
  mov r7,#0
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  stmia block!,{r4,r5,r6,r7} ; as 16byte
  
  POP {r4,r5,r6,r7,r8,r9,r10,pc}

#undef block
#undef dest
#undef stride
#undef pclip
#undef cnt
#undef read0
#undef read1
#undef write0
#undef write1
#undef blockdata
#undef tmp
}*/

CODE_IN_ITCM_DPG void mpeg2_idct_copy(int16_t * block, uint8_t * dest, const int stride)
{/*
  int32_t ws[8*8];
  u32 i;
  for (i = 0; i < 8; i++) idct_row32_asm(block + 8 * i,ws + 8 * i);
  for (i = 0; i < 8; i++) idct_col32_asm(ws + i);
  mpeg2_idct_copy32_CopyAndClearBlock(ws,dest,stride,block);*/
}

CODE_IN_ITCM_DPG void mpeg2_idct_add(const int last, int16_t * block, uint8_t * dest, const int stride)
{/*
 u32 i;
 if (last != 129 || (block[0] & (7 << 4)) == (4 << 4)) {
    int32_t ws[8*8];
    for (i = 0; i < 8; i++) idct_row32_asm(block + 8 * i,ws + 8 * i);
    for (i = 0; i < 8; i++) idct_col32_asm(ws + i);
    add32_pixels_clamped_asm(ws, dest, stride,block); // and clear block.
    } else {
    s32 DC=(block[0] + 64) >> 7;
    block[0] = block[63] = 0;
    mpeg2_idct_add_AddDC_asm(DC,dest,stride);
  }*/
}

void mpeg2_idct_init (uint32_t accel)
{
  s32 i;
  for (i=-256;i<256*2;i++){
    s32 c=i;
    if(c<0) c=0;
    if(255<c) c=255;
    mpeg2_clip8[256+i] = c;
  }
}
