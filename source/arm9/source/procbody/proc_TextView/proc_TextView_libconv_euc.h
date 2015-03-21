
static u32 libconv_EUC_CheckErrorCharsCount(u8 *pbuf,u32 bufsize,CglFont *pFont)
{
    TEUC2Unicode *ps2u=&EUC2Unicode;
  
    //bool inHalfKana=false;
    u32 errcnt=0;
  
    u8 *pbufterm=&pbuf[bufsize];
  
    while(pbuf<pbufterm){
        u32 widx;
    
        {
            u32 c0=pbuf[0];
            u32 c1=pbuf[1];
            if(ps2u->panktbl[c0]==true){
                if((0xa0<=c0)&&(c0<0xe0)){
                    c0=0xff60+(c0-0xa0);
                    //inHalfKana=true;
                }
                widx=c0;
                pbuf+=1;
            }else{
                u32 euc=(c0<<8)|c1;
                widx=ps2u->ps2utbl[euc];
                pbuf+=2;
            }
        }
    
        if((0x20<=widx)&&(pFont->isExists(widx)==false)) errcnt++;
    }
  
    //if((errcnt==0)&&(inHalfKana==true)) errcnt=1;
  
    return(errcnt);
}

static void libconv_EUC_DetectReturnCode(u8 *pbuf,u32 bufsize)
{
    TEUC2Unicode *ps2u=&EUC2Unicode;
  
    u8 *pbufterm=&pbuf[bufsize];
  
    while(pbuf<pbufterm){
        u32 c0=pbuf[0];
        u32 c1=pbuf[1];
    
        if(ps2u->panktbl[c0]==true){
            u32 ch=c0;
      
            if(ch==CharCR){
                u32 ch=c1;
                ReturnCode=ERC_CR;
                if(ch==CharLF) ReturnCode=ERC_CRLF;
                return;
            }
      
            if(ch==CharLF){
                u32 ch=c1;
                ReturnCode=ERC_LF;
                if(ch==CharCR) ReturnCode=ERC_LFCR;
                return;
            }
      
            pbuf+=1;
        }else{
            pbuf+=2;
        }
    
    }
}

static void libconv_EUC_Convert(FAT_FILE *pfh,u32 bufsize,CglFont *pFont)
{
    TEUC2Unicode *ps2u=&EUC2Unicode;
  
    ConvertBody_Phase1;
    
    while(1){
        ConvertBody_PhaseReadBuf;
    
        u32 widx;
    
        {
            u32 c0=ReadBuf[ReadBufOffset+0];
            u32 c1=ReadBuf[ReadBufOffset+1];
      
            if(ps2u->panktbl[c0]==true){
                if((0xa0<=c0)&&(c0<0xe0)) c0=0xff60+(c0-0xa0);
                widx=c0;
                ReadBufOffset+=1;
                FileOffset+=1;
            }else{
                u32 euc=(c0<<8)|c1;
                widx=ps2u->ps2utbl[euc];
                ReadBufOffset+=2;
                FileOffset+=2;
            }
        }
    
        ConvertBody_Phase2;
    }
  
    ConvertBody_Phase3;
}

static void libconv_SelectEncode_EUC(void)
{
    pEncodeID="EUC/S-JIS";
    libconv_CheckErrorCharsCount=libconv_EUC_CheckErrorCharsCount;
    libconv_DetectReturnCode=libconv_EUC_DetectReturnCode;
    libconv_Convert=libconv_EUC_Convert;
}

