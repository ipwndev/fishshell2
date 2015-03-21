
static u32 libconv_UTF16LE_CheckErrorCharsCount(u8 *pbuf,u32 bufsize,CglFont *pFont)
{
    u16 bom=*(u16*)pbuf;
    if(bom==0xfeff) return(0);
  
    u32 errcnt=0;
  
    u8 *pbufterm=&pbuf[bufsize];
  
    while(pbuf<pbufterm){
        u32 b0=*pbuf++;
        u32 b1=*pbuf++;
        u32 widx=(b0<<0)|(b1<<8);
    
        if((0x20<=widx)&&(pFont->isExists(widx)==false)) errcnt++;
    }
  
    return(errcnt);
}

static void libconv_UTF16LE_DetectReturnCode(u8 *pbuf,u32 bufsize)
{
    u8 *pbufterm=&pbuf[bufsize];
  
    while(pbuf<pbufterm){
        u32 b0=*pbuf++;
        u32 b1=*pbuf++;
        u32 ch=(b0<<0)|(b1<<8);
    
        if(ch==CharCR){
            u32 b0=*pbuf++;
            u32 b1=*pbuf++;
            u32 ch=(b0<<0)|(b1<<8);
            ReturnCode=ERC_CR;
            if(ch==CharLF) ReturnCode=ERC_CRLF;
            return;
        }
    
        if(ch==CharLF){
            u32 b0=*pbuf++;
            u32 b1=*pbuf++;
            u32 ch=(b0<<0)|(b1<<8);
            ReturnCode=ERC_LF;
            if(ch==CharCR) ReturnCode=ERC_LFCR;
            return;
        }
    }
}

static void libconv_UTF16LE_Convert(FAT_FILE *pfh,u32 FileSize,CglFont *pFont)
{
    ConvertBody_Phase1;
    
    while(1){
        ConvertBody_PhaseReadBuf;
    
        u32 widx;
    
        {
            u32 b0=ReadBuf[ReadBufOffset++];
            u32 b1=ReadBuf[ReadBufOffset++];
            widx=(b0<<0)|(b1<<8);
            FileOffset+=2;
        }
    
        ConvertBody_Phase2;
    }
  
    ConvertBody_Phase3;
}

static void libconv_SelectEncode_UTF16LE(void)
{
    pEncodeID="UTF-16 (LE)";
    libconv_CheckErrorCharsCount=libconv_UTF16LE_CheckErrorCharsCount;
    libconv_DetectReturnCode=libconv_UTF16LE_DetectReturnCode;
    libconv_Convert=libconv_UTF16LE_Convert;
}

