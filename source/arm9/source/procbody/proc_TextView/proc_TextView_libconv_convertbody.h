

static __attribute__ ((noinline)) void SetDivCharMap(bool *pDivCharMap)
{
    for(u32 idx=0;idx<128;idx++){
        bool f=true;
        if(('A'<=idx)&&(idx<='Z')) f=false;
        if(('a'<=idx)&&(idx<='z')) f=false;
        if(('0'<=idx)&&(idx<='9')) f=false;
        if('('<=idx) f=false;
        if(')'<=idx) f=false;
        if('\''<=idx) f=false;
        pDivCharMap[idx]=f;
    }
}

static __attribute__ ((noinline)) bool ConvertBody_PhaseReadBuf_ins_CheckInterruptKey(void)
{
    u32 keys=(~REG_KEYINPUT)&0x3ff;
    if((keys&KEY_B)!=0){
        RequestInterruptBreak=true;
        return(true);
    }
  
    return(false);
}

static __attribute__ ((noinline)) void ConvertBody_Phase2_ins_msgredraw(FAT_FILE *pfh)
{
    if(!RequestPretreatment && !RequestAllConvert) return;
    DATA_IN_IWRAM_TextView static u32 skip=0;
  
    if(skip<64){
        skip++;
        return;
    }
  
    skip=0;
  
    char str[64];
  
    u32 pos=FAT2_ftell(pfh),max=FAT2_GetFileSize(pfh);
    snprintf(str,64,"%d lines. %d%%",TextLinesCount,pos*100/max);
  
    CallBack_MWin_ProgressDraw(Lang_GetUTF8("TV_PRG_ConvertToUnicode"),str,pos,max);
}

DATA_IN_IWRAM_TextView static UnicodeChar *pLineBuf;
DATA_IN_IWRAM_TextView static u32 LineBufLength,LineBufWidth;
DATA_IN_IWRAM_TextView static u32 LineBufDivLength,LineBufDivWidth;

DATA_IN_IWRAM_TextView static UnicodeChar *pWriteBuf;
DATA_IN_IWRAM_TextView static u32 WriteBufOffset;

DATA_IN_IWRAM_TextView static TTextLine *pTextLines;
DATA_IN_IWRAM_TextView static u32 WriteLineOffset;

DATA_IN_IWRAM_TextView static u32 IgnoreCount;

DATA_IN_IWRAM_TextView static u32 FileOffset;

#define ConvertBody_Phase1 \
    \
    u32 TopOffset=EncodeConvertTopOffset; \
    \
    u32 StartLineCount=TextLinesCount; \
    u8 ReadBuf[DefaultReadBufSize]; \
    u32 ReadBufOffset=0,ReadBufSize=0; \
    \
    const u8 *pWidths=pExtFont->Widths; \
    \
    u32 IgnoreReturnCodeChar; \
    u32 IgnoreReturnCodeSize; \
    \
    u32 StartLineOffset=FileOffset; \
    \
    u32 PreLines=0; \
    \
    switch(ReturnCode){ \
        case ERC_Unknown: IgnoreReturnCodeChar=0; IgnoreReturnCodeSize=0; break; \
        case ERC_CR:   IgnoreReturnCodeChar=CharCR; IgnoreReturnCodeSize=1-1; break; \
        case ERC_LF:   IgnoreReturnCodeChar=CharLF; IgnoreReturnCodeSize=1-1; break; \
        case ERC_CRLF: IgnoreReturnCodeChar=CharCR; IgnoreReturnCodeSize=2-1; break; \
        case ERC_LFCR: IgnoreReturnCodeChar=CharLF; IgnoreReturnCodeSize=2-1; break; \
        default: IgnoreReturnCodeChar=0; IgnoreReturnCodeSize=0; break; \
    } \
    \
    bool DivCharMap[128]; \
    SetDivCharMap(DivCharMap); \
    FAT2_fseek(pfh,EncodeConvertFileOffset,SEEK_SET); \



#define ConvertBody_PhaseReadBuf \
    if((ReadBufSize-ReadBufOffset)<=8){ \
        if(ReadBufOffset!=ReadBufSize){ \
            MemCopy8CPU(&ReadBuf[ReadBufOffset],&ReadBuf[0],ReadBufSize-ReadBufOffset); \
        } \
        ReadBufSize-=ReadBufOffset; \
        ReadBufOffset=0; \
        ReadBufSize+=FAT2_fread(&ReadBuf[ReadBufSize],1,DefaultReadBufSize-ReadBufSize,pfh); \
        EncodeConvertFileOffset=FAT2_ftell(pfh); \
        if(RequestPretreatment && ConvertBody_PhaseReadBuf_ins_CheckInterruptKey()==true) {EncodeConvertAllEnd=true; break;} \
    } \
    if(ReadBufSize<=ReadBufOffset) {EncodeConvertAllEnd=true; break;} \
    if(!RequestAllConvert && RequestPretreatment && RequestBKMResumeFileOffset==0 && PreLines>100 && (TextLinesCount>=5000)) { \
        if(((WriteLineOffset*sizeof(TTextLine))%512)==0){ \
            DFS_WriteSectors((u8*)pTextLines,(WriteLineOffset*sizeof(TTextLine))/512,&DFS_LinesIndex_WriteFile); \
            WriteLineOffset=0; \
            EncodeConvertFileOffset-=ReadBufSize-ReadBufOffset; \
            break; \
        } \
    } \
    if(!RequestAllConvert && !RequestPretreatment && TextLinesCount-StartLineCount>=50) { \
        EncodeConvertFileOffset-=ReadBufSize-ReadBufOffset; \
        break; \
    } \
    if(DLLSound_isOpened()==true){ \
        DLLSound_UpdateLoop(false); \
    } \

#define ConvertBody_Phase2 \
    u32 reqmode=0; /* 0=pass, 1=return, 2=div */ \
    u32 CharWidth=0; \
    \
    if(IgnoreCount!=0){ \
        IgnoreCount--; \
        widx=0; \
    }else{ \
        if(widx<0x20){ \
            if(widx==IgnoreReturnCodeChar){ \
                reqmode=1; /* req return */ \
                IgnoreCount=IgnoreReturnCodeSize; \
            } \
            widx=0; \
        }else{ \
            CharWidth=pWidths[widx]; \
            if(CharWidth==0) widx=0; \
            if(LineWidth<(LineBufWidth+CharWidth)){ \
                if(LineBufDivLength==0){ \
                    reqmode=1; /* req return */ \
                }else{ \
                    reqmode=2; /* req div */ \
                } \
            } \
        } \
    } \
    \
    if(reqmode!=0){ \
        u32 Length; \
        if(reqmode==1){ /* req return */ \
            Length=LineBufLength; \
        }else{ /* req div */ \
            Length=LineBufDivLength; \
        } \
        for(u32 idx=0;idx<Length;idx++){ \
            pWriteBuf[WriteBufOffset++]=pLineBuf[idx]; \
        } \
        pWriteBuf[WriteBufOffset++]=CharLineEnd; \
        pTextLines[WriteLineOffset].TopOffset=TopOffset; \
        pTextLines[WriteLineOffset].FileOffset=StartLineOffset; \
        StartLineOffset=FileOffset; \
        TextLinesCount++; \
        WriteLineOffset++; \
        if(RequestBKMResumeFileOffset!=0) { \
            if(FileOffset>=RequestBKMResumeFileOffset){ \
                Bookmark_SetResumeLineNum(1+TextLinesCount,StartLineOffset); \
                RequestBKMResumeFileOffset=0; \
            } \
        }else{ \
            PreLines++; \
        } \
        if(DefaultLinesBufSize==WriteLineOffset){ \
            DFS_WriteSectors((u8*)pTextLines,(DefaultLinesBufSize*sizeof(TTextLine))/512,&DFS_LinesIndex_WriteFile); \
            WriteLineOffset=0; \
        } \
        if(TextLinesCount==TextLinesMaxCount){ \
            _consolePrintf("Text line buffer overflow. Max lines count is %d.\n",TextLinesMaxCount); \
            EncodeConvertAllEnd=true; \
            break; \
        } \
        TopOffset+=Length+1; \
        if(reqmode==1){ /* req return */ \
            LineBufLength=0; \
            LineBufWidth=0; \
        }else{ /* req div */ \
            if(LineBufLength!=LineBufDivLength){ \
                MemCopy16CPU(&pLineBuf[LineBufDivLength],&pLineBuf[0],(LineBufLength-LineBufDivLength)*2); \
            } \
            LineBufLength-=LineBufDivLength; \
            LineBufWidth-=LineBufDivWidth; \
        } \
        LineBufDivLength=0; \
        LineBufDivWidth=0; \
    } \
    \
    if(widx!=0){ \
        pLineBuf[LineBufLength++]=widx; \
        LineBufWidth+=CharWidth; \
        if((128<=widx)||(DivCharMap[widx]==true)){ \
            LineBufDivLength=LineBufLength; \
            LineBufDivWidth=LineBufWidth; \
        } \
    } \
    \
    if(DefaultWriteBufSize<=WriteBufOffset){ \
        DFS_WriteSectors((u8*)pWriteBuf,(DefaultWriteBufSize*2)/512,&DFS_WriteFile); \
        WriteBufOffset-=DefaultWriteBufSize; \
        if(WriteBufOffset!=0) MemCopy16CPU(&pWriteBuf[DefaultWriteBufSize],&pWriteBuf[0],WriteBufOffset*2); \
        ConvertBody_Phase2_ins_msgredraw(pfh); \
    } \



#define ConvertBody_Phase3 \
    EncodeConvertTopOffset=TopOffset; \
    \
    ScrollBar.MaxPos=ScrollBar.LineHeight*TextLinesCount; \
    \
    if(EncodeConvertAllEnd==true) { \
        if(LineBufLength!=0){ \
            for(u32 idx=0;idx<LineBufLength;idx++){ \
                pWriteBuf[WriteBufOffset++]=pLineBuf[idx]; \
            } \
            pWriteBuf[WriteBufOffset++]=CharLineEnd; \
            if(TextLinesCount==TextLinesMaxCount){ \
                _consolePrintf("Text line buffer overflow. Max lines count is %d.\n",TextLinesMaxCount); \
            }else{ \
                pTextLines[WriteLineOffset].TopOffset=TopOffset; \
                pTextLines[WriteLineOffset].FileOffset=StartLineOffset; \
                StartLineOffset=FileOffset; \
                TextLinesCount++; \
                WriteLineOffset++; \
                if(DefaultLinesBufSize==WriteLineOffset){ \
                    DFS_WriteSectors((u8*)pTextLines,(DefaultLinesBufSize*sizeof(TTextLine))/512,&DFS_LinesIndex_WriteFile); \
                    WriteLineOffset=0; \
                } \
            }\
        } \
        \
        u32 secw=((WriteBufOffset*2)+511)/512; \
        if(secw!=0) DFS_WriteSectors((u8*)pWriteBuf,secw,&DFS_WriteFile); \
        \
        u32 secl=((WriteLineOffset*sizeof(TTextLine))+511)/512; \
        if(secl!=0) DFS_WriteSectors((u8*)pTextLines,secl,&DFS_LinesIndex_WriteFile); \
        \
        TotalCharsCount=TopOffset; \
        \
        libconv_SelectEncode_NULL(); \
        \
        _consolePrintf("Total lines count is %d.\n",TextLinesCount); \
        \
        if(pTextLines!=NULL){ \
            safefree(&MM_Process,pTextLines); pTextLines=NULL; \
        } \
        if(pLineBuf!=NULL){ \
            safefree(&MM_Process,pLineBuf); pLineBuf=NULL; \
        } \
        \
        if(pWriteBuf!=NULL){ \
            safefree(&MM_Process,pWriteBuf); pWriteBuf=NULL; \
        } \
        \
        ScrollBar.MaxPos=ScrollBar.LineHeight*TextLinesCount; \
    } \


