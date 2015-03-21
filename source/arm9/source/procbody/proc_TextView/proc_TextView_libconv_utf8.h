
static u32 libconv_UTF8_CheckErrorCharsCount(u8 *pbuf,u32 bufsize,CglFont *pFont)
{
    u32 errcnt=0;
  
    u8 *pbufterm=&pbuf[bufsize];
  
    while(pbuf<pbufterm){
        u32 b0=pbuf[0],b1=pbuf[1],b2=pbuf[2];
        u32 widx;
    
        if(b0<0x80){
            widx=b0;
            pbuf++;
        }else{
            if((b0&0xe0)==0xc0){ // 0b 110. ....
                widx=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
                pbuf+=2;
            }else{
                if((b0&0xf0)==0xe0){ // 0b 1110 ....
                    widx=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
                    pbuf+=3;
                }else{
                    widx=(u32)'?';
                    pbuf+=4;
                }
            }
        }
    
        if((0x20<=widx)&&(pFont->isExists(widx)==false)) errcnt++;
    }
  
    return(errcnt);
}

static void libconv_UTF8_DetectReturnCode(u8 *pbuf,u32 bufsize)
{
    u8 *pbufterm=&pbuf[bufsize];
  
    while(pbuf<pbufterm){
        u32 b0=pbuf[0],b1=pbuf[1];
    
        if(b0<0x80){
            u32 ch=b0;
            if(ch==CharCR){
                u32 ch=b1;
                ReturnCode=ERC_CR;
                if(ch==CharLF) ReturnCode=ERC_CRLF;
                return;
            }
      
            if(ch==CharLF){
                u32 ch=b1;
                ReturnCode=ERC_LF;
                if(ch==CharCR) ReturnCode=ERC_LFCR;
                return;
            }
      
            pbuf++;
        }else{
            if((b0&0xe0)==0xc0){ // 0b 110. ....
                pbuf+=2;
            }else{
                if((b0&0xf0)==0xe0){ // 0b 1110 ....
                    pbuf+=3;
                }else{
                    pbuf+=4;
                }
            }
        }
    }
}

static void libconv_UTF8_Convert(FAT_FILE *pfh,u32 FileSize,CglFont *pFont)
{
    ConvertBody_Phase1;
    
    while(1){
        ConvertBody_PhaseReadBuf;
    
        u32 widx;
    
        {
            u32 b0=ReadBuf[ReadBufOffset+0],b1=ReadBuf[ReadBufOffset+1],b2=ReadBuf[ReadBufOffset+2];
      
            if(b0<0x80){
                widx=b0;
                ReadBufOffset+=1;
                FileOffset+=1;
            }else{
                if((b0&0xe0)==0xc0){ // 0b 110. ....
                    widx=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
                    ReadBufOffset+=2;
                    FileOffset+=2;
                }else{
                    if((b0&0xf0)==0xe0){ // 0b 1110 ....
                        widx=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
                        ReadBufOffset+=3;
                        FileOffset+=3;
                    }else{
                        widx=(u32)'?';
                        ReadBufOffset+=4;
                        FileOffset+=4;
                    }
                }
            }
        }
    
        ConvertBody_Phase2;
    }
  
    ConvertBody_Phase3;
}

static void libconv_SelectEncode_UTF8(void)
{
    pEncodeID="UTF-8";
    libconv_CheckErrorCharsCount=libconv_UTF8_CheckErrorCharsCount;
    libconv_DetectReturnCode=libconv_UTF8_DetectReturnCode;
    libconv_Convert=libconv_UTF8_Convert;
}

