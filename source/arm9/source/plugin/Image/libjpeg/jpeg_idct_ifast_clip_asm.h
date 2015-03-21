
#ifndef jpeg_idct_ifast_clip_asm_h
#define jpeg_idct_ifast_clip_asm_h

extern "C" {
void jpeg_idct_ifast_clip_asm (s32 *dstblock,s32 *pquant,s16 *psrcblock,unsigned char **ppLineBuf);
}

#endif

