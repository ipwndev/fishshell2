
static void libconv_ANSI_LoadCodepageToUnicodeTable(u16 *ptbl)
{
    FAT_FILE *pf=Shell_FAT_fopen_CodepageToUnicodeTable();
    if(pf==NULL) StopFatalError(18601,"Not found Codepage to Unicode convert table.\n");
    FAT2_fread_fast(ptbl,2,256,pf);
    FAT2_fclose(pf);
}

static u32 libconv_ANSI_CheckErrorCharsCount(u8 *pbuf,u32 bufsize,CglFont *pFont)
{
    u32 errcnt=0;
  
    u16 CPtoUnicodeTable[256];
    libconv_ANSI_LoadCodepageToUnicodeTable(CPtoUnicodeTable);
    
    u8 *pbufterm=&pbuf[bufsize];
  
    while(pbuf<pbufterm){
        u32 widx=CPtoUnicodeTable[*pbuf++];
    
        if((0x20<=widx)&&(pFont->isExists(widx)==false)) errcnt++;
    }
  
    return(errcnt);
}

static void libconv_ANSI_DetectReturnCode(u8 *pbuf,u32 bufsize)
{
    u8 *pbufterm=&pbuf[bufsize];
  
    u16 CPtoUnicodeTable[256];
    libconv_ANSI_LoadCodepageToUnicodeTable(CPtoUnicodeTable);
    
    while(pbuf<pbufterm){
        u32 ch=CPtoUnicodeTable[*pbuf++];
    
        if(ch==CharCR){
            u32 ch=*pbuf;
            ReturnCode=ERC_CR;
            if(ch==CharLF) ReturnCode=ERC_CRLF;
            return;
        }
    
        if(ch==CharLF){
            u32 ch=*pbuf;
            ReturnCode=ERC_LF;
            if(ch==CharCR) ReturnCode=ERC_LFCR;
            return;
        }
    }
}

static void libconv_ANSI_Convert(FAT_FILE *pfh,u32 FileSize,CglFont *pFont)
{
    ConvertBody_Phase1;
    
    u16 CPtoUnicodeTable[256];
    libconv_ANSI_LoadCodepageToUnicodeTable(CPtoUnicodeTable);
    
    while(1){
        ConvertBody_PhaseReadBuf;
    
        u32 widx;
    
        {
            widx=CPtoUnicodeTable[ReadBuf[ReadBufOffset++]];
            FileOffset++;
        }
    
        ConvertBody_Phase2;
    }
  
    ConvertBody_Phase3;
}

static void libconv_SelectEncode_ANSI(void)
{
    pEncodeID="ANSI 8bit";
    libconv_CheckErrorCharsCount=libconv_ANSI_CheckErrorCharsCount;
    libconv_DetectReturnCode=libconv_ANSI_DetectReturnCode;
    libconv_Convert=libconv_ANSI_Convert;
}

