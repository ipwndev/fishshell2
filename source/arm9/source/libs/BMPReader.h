
#ifndef BMPReader_h
#define BMPReader_h

extern bool BMPReader_Start(FAT_FILE *_FileHandle);
extern void BMPReader_Free(void);
extern void BMPReader_GetBitmap32(u32 LineY,u32 *pBM);
extern s32 BMPReader_GetWidth(void);
extern s32 BMPReader_GetHeight(void);

#endif

