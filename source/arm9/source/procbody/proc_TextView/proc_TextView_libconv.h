
#define DefaultReadBufSize (256*2)
#define DefaultWriteBufSize (256*2)
#define DefaultLinesBufSize (1024*4)
// -------------------------------------------------------

typedef struct {
    u32 TopOffset;
    u32 FileOffset;
} TTextLine;

DATA_IN_IWRAM_TextView static u32 ReadSectorIndex;
DATA_IN_IWRAM_TextView static UnicodeChar *pReadBuf; // 512uc 2sectors.

DATA_IN_IWRAM_TextView static u32 LineOfsSectorIndex;
DATA_IN_IWRAM_TextView static TTextLine *pLineOfsBuf; // 512u32 1sectors.
// -------------------------------------------------------

enum EReturnCode {ERC_Unknown,ERC_CR,ERC_LF,ERC_CRLF,ERC_LFCR};

DATA_IN_IWRAM_TextView static EReturnCode ReturnCode;

#define CharCR (0x0d)
#define CharLF (0x0a)
#define CharLineEnd (0x00)

// -------------------------------------------------------

DATA_IN_IWRAM_TextView static const char *pEncodeID;

DATA_IN_IWRAM_TextView static u32 (*libconv_CheckErrorCharsCount)(u8 *pbuf,u32 bufsize,CglFont *pFont);
DATA_IN_IWRAM_TextView static void (*libconv_DetectReturnCode)(u8 *pbuf,u32 bufsize);
DATA_IN_IWRAM_TextView static void (*libconv_Convert)(FAT_FILE *pfh,u32 FileSize,CglFont *pFont);

static void libconv_SelectEncode_NULL(void)
{
    libconv_CheckErrorCharsCount=NULL;
    libconv_DetectReturnCode=NULL;
    libconv_Convert=NULL;
}

#include "proc_TextView_libconv_convertbody.h"

#include "proc_TextView_libconv_ansi.h"
#include "proc_TextView_libconv_euc.h"
#include "proc_TextView_libconv_utf16be.h"
#include "proc_TextView_libconv_utf16le.h"
#include "proc_TextView_libconv_utf8.h"

// -------------------------------------------------------

static void libconv_Init(void)
{
    TextLinesCount=0;
    pTextLines=(TTextLine*)safemalloc_chkmem(&MM_Process,sizeof(TTextLine)*(DefaultLinesBufSize+256));
  
    pLineBuf=(UnicodeChar*)safemalloc_chkmem(&MM_Process,sizeof(UnicodeChar)*128);
  
    pWriteBuf=(UnicodeChar*)safemalloc_chkmem(&MM_Process,sizeof(UnicodeChar)*(DefaultWriteBufSize+256));
      
    TotalCharsCount=0;
  
    ReturnCode=ERC_Unknown;
  
    pEncodeID=NULL;
    libconv_SelectEncode_NULL();
  
    ReadSectorIndex=(u32)-1;
    pReadBuf=NULL;
  
    EncodeConvertAllEnd=false;
    EncodeConvertFileOffset=0;
    EncodeConvertTopOffset=0;
  
    LineBufLength=0;
    LineBufWidth=0;
    LineBufDivLength=0;
    LineBufDivWidth=0;

    WriteBufOffset=0;
    WriteLineOffset=0;
  
    IgnoreCount=0;
    FileOffset=0;
}

static void libconv_EndConvert(void)
{
    ReadSectorIndex=(u32)-1;
    pReadBuf=(UnicodeChar*)safemalloc_chkmem(&MM_Process,512*2); // 2sectors.
    LineOfsSectorIndex=(u32)-1;
    pLineOfsBuf=(TTextLine*)safemalloc_chkmem(&MM_Process,128*4); // 1sectors.
}

static void libconv_Free(void)
{
    TextLinesCount=0;
  
    if(pTextLines!=NULL){
        safefree(&MM_Process,pTextLines); pTextLines=NULL;
    }
  
    if(pReadBuf!=NULL){
        safefree(&MM_Process,pReadBuf); pReadBuf=NULL;
    }
    
    if(pLineOfsBuf!=NULL){
        safefree(&MM_Process,pLineOfsBuf); pLineOfsBuf=NULL;
    }
    
    if(pLineBuf!=NULL){
        safefree(&MM_Process,pLineBuf); pLineBuf=NULL;
    }
    
    if(pWriteBuf!=NULL){
        safefree(&MM_Process,pWriteBuf); pWriteBuf=NULL;
    }
}

static void libconv_AutoSelectEncode(u8 *pbuf,u32 bufsize)
{
    u32 errcnt_ANSI=0x7fffffff;
    u32 errcnt_EUC=0x7fffffff;
    u32 errcnt_UTF16BE=0x7fffffff;
    u32 errcnt_UTF16LE=0x7fffffff;
    u32 errcnt_UTF8=0x7fffffff;
  
    TProcState_Text *ptxt=&ProcState.Text;
  
    if(ptxt->DetectCharCode_ANSI==true){
        libconv_SelectEncode_ANSI();
        errcnt_ANSI=libconv_CheckErrorCharsCount(pbuf,bufsize,pCglFontDefault);
        _consolePrintf("errcnt_ANSI= %d.\n",errcnt_ANSI);
        if(errcnt_ANSI==0) return;
        errcnt_ANSI++;
    }
  
    if(ptxt->DetectCharCode_EUC==true){
        libconv_SelectEncode_EUC();
        errcnt_EUC=libconv_CheckErrorCharsCount(pbuf,bufsize,pCglFontDefault);
        _consolePrintf("errcnt_EUC= %d.\n",errcnt_EUC);
        if(errcnt_EUC==0) return;
    }
  
    if(ptxt->DetectCharCode_UTF16BE==true){
        libconv_SelectEncode_UTF16BE();
        errcnt_UTF16BE=libconv_CheckErrorCharsCount(pbuf,bufsize,pCglFontDefault);
        _consolePrintf("errcnt_UTF16BE= %d.\n",errcnt_UTF16BE);
        if(errcnt_UTF16BE==0) return;
    }
  
    if(ptxt->DetectCharCode_UTF16LE==true){
        libconv_SelectEncode_UTF16LE();
        errcnt_UTF16LE=libconv_CheckErrorCharsCount(pbuf,bufsize,pCglFontDefault);
        _consolePrintf("errcnt_UTF16LE= %d.\n",errcnt_UTF16LE);
        if(errcnt_UTF16LE==0) return;
    }
  
    if(ptxt->DetectCharCode_UTF8==true){
        libconv_SelectEncode_UTF8();
        errcnt_UTF8=libconv_CheckErrorCharsCount(pbuf,bufsize,pCglFontDefault);
        _consolePrintf("errcnt_UTF8= %d.\n",errcnt_UTF8);
        if(errcnt_UTF8==0) return;
    }
  
    libconv_SelectEncode_UTF8(); // default.
  
    u32 errcnt=0x7fffffff;
  
    if(errcnt_ANSI<errcnt){
        errcnt=errcnt_ANSI;
        libconv_SelectEncode_ANSI();
    }
  
    if(errcnt_EUC<errcnt){
        errcnt=errcnt_EUC;
        libconv_SelectEncode_EUC();
    }
  
    if(errcnt_UTF16BE<errcnt){
        errcnt=errcnt_UTF16BE;
        libconv_SelectEncode_UTF16BE();
    }
  
    if(errcnt_UTF16LE<errcnt){
        errcnt=errcnt_UTF16LE;
        libconv_SelectEncode_UTF16LE();
    }
  
    if(errcnt_UTF8<errcnt){
        errcnt=errcnt_UTF8;
        libconv_SelectEncode_UTF8();
    }
}

#undef EnableZeroCheck

// -------------------------------------------------------

static u32 libconv_GetTextLineOffset(u32 lineidx)
{
    if(TextLinesCount<=lineidx) return((u32)-1);
    
    u32 secidx=lineidx*sizeof(TTextLine)/512;
    
    if(LineOfsSectorIndex!=secidx){
        //_consolePrintf("read line=%d  secidx=%d\n",lineidx,secidx);
        LineOfsSectorIndex=secidx;
        DFS_SeekSectorCount(LinesIndex_SectorPosition+secidx,&DFS_ReadFile);
        DFS_ReadSectors((u8*)pLineOfsBuf,1,&DFS_ReadFile);
    }

    return(pLineOfsBuf[lineidx%64].TopOffset);
}

static u32 libconv_GetTextFileOffset(u32 lineidx)
{
    if(TextLinesCount<=lineidx) return((u32)-1);
    
    u32 secidx=lineidx*sizeof(TTextLine)/512;
    
    if(LineOfsSectorIndex!=secidx){
        //_consolePrintf("read line=%d  secidx=%d\n",lineidx,secidx);
        LineOfsSectorIndex=secidx;
        DFS_SeekSectorCount(LinesIndex_SectorPosition+secidx,&DFS_ReadFile);
        DFS_ReadSectors((u8*)pLineOfsBuf,1,&DFS_ReadFile);
    }

    return(pLineOfsBuf[lineidx%64].FileOffset);
}

static bool libconv_GetTextLine(u32 lineidx,UnicodeChar *pstrw)
{
    if(TextLinesCount<=lineidx) return(false);
  
    //u32 ofs=pTextLines[lineidx].TopOffset;
    u32 ofs=libconv_GetTextLineOffset(lineidx);
  
    u32 secidx=ofs/256+1;
    if(ReadSectorIndex!=secidx){
        //_consolePrintf("read line=%d  secidx=%d ofs=%d\n",lineidx,secidx,ofs);
        ReadSectorIndex=secidx;
        DFS_SeekSectorCount(secidx,&DFS_ReadFile);
        DFS_ReadSectors((u8*)pReadBuf,2,&DFS_ReadFile);
    }
  
    ofs&=255;
  
    while(pReadBuf[ofs]!=CharLineEnd){
        *pstrw++=pReadBuf[ofs];
        ofs++;
    }
    *pstrw++=0;
  
    return(true);
}

