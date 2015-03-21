
#define ExtFontCount (0x10000)

typedef struct {
    u16 Color;
    u32 Height;
    u8 Widths[ExtFontCount];
    u8 *pBody;
    u8 **ppBodyLink;
    bool UseClearTypeFont;
    FAT_FILE *pClearTypeFontFileHandle;
    CCTF *pCTF;
} TExtFont;

DATA_IN_IWRAM_TextView static TExtFont *pExtFont;

static void ExtFont_Init(void)
{
    pExtFont=(TExtFont*)safemalloc_chkmem(&MM_Process,sizeof(TExtFont));
  
    pExtFont->Color=RGB15(0,0,0)|BIT15;
    pExtFont->Height=ProcState.Text.FontSize;
    //MemSet32CPU(0,pExtFont->Widths,0x10000);
    pExtFont->pBody=NULL;
    pExtFont->ppBodyLink=NULL;
    pExtFont->UseClearTypeFont=false;
    pExtFont->pClearTypeFontFileHandle=NULL;
    pExtFont->pCTF=NULL;
  
    switch(ProcState.Text.ClearTypeFont){
        case ETCTF_None: pExtFont->UseClearTypeFont=false; break;
        case ETCTF_Lite: case ETCTF_Normal: case ETCTF_Heavy: pExtFont->UseClearTypeFont=true; break;
    }
  
    if(pExtFont->UseClearTypeFont==false){
        char fn[32];
        snprintf(fn,32,FontWidthFilenameFormat,pExtFont->Height);
    
        FAT_FILE *pf=Shell_FAT_fopen_TextFont(fn);
        if(pf==NULL) StopFatalError(18402,"ExtFont: Not found font width file.\n");
    
        FAT2_fread_fast(pExtFont->Widths,1,ExtFontCount,pf);
        for(int idx=0;idx<ExtFontCount;idx++){
            if(pExtFont->Widths[idx]!=0) pExtFont->Widths[idx]++;
        }
    
        FAT2_fclose(pf);
    
    }else{ // for ClearTypeFont
        char fn[32];
        snprintf(fn,32,FontClearTypeFilenameFormat,pExtFont->Height);
    
        pExtFont->pClearTypeFontFileHandle=Shell_FAT_fopen_TextFont(fn);
        if(pExtFont->pClearTypeFontFileHandle==NULL) StopFatalError(18403,"ExtFont: Not found clear type font file.\n");
    
        CCTF *pCTF=new CCTF(pExtFont->pClearTypeFontFileHandle,pExtFont->Height,ProcState.Text.ClearTypeFont);
        pCTF->GetWidthsList(pExtFont->Widths);
        delete pCTF; pCTF=NULL;
    }
}

static void ExtFont_Free(void)
{
    if(pExtFont!=NULL){  
        if(pExtFont->pBody!=NULL){
            safefree(&MM_Process,pExtFont->pBody); pExtFont->pBody=NULL;
        }
        if(pExtFont->ppBodyLink!=NULL){
            safefree(&MM_Process,pExtFont->ppBodyLink); pExtFont->ppBodyLink=NULL;
        }
        if(pExtFont->pClearTypeFontFileHandle!=NULL){
            FAT2_fclose(pExtFont->pClearTypeFontFileHandle); pExtFont->pClearTypeFontFileHandle=NULL;
        }
        if(pExtFont->pCTF!=NULL){
            delete pExtFont->pCTF; pExtFont->pCTF=NULL;
        }
    
        safefree(&MM_Process,pExtFont); pExtFont=NULL;
    }
}

static void ExtFont_LoadBody(void)
{
    if(pExtFont->UseClearTypeFont==true){
        FAT2_fseek(pExtFont->pClearTypeFontFileHandle,0,SEEK_SET);
        pExtFont->pCTF=new CCTF(pExtFont->pClearTypeFontFileHandle,pExtFont->Height,ProcState.Text.ClearTypeFont);
    
        CCTF *pCTF=pExtFont->pCTF;
        pCTF->SetTargetCanvas(NULL);
    
        return;
    }
  
    char fn[32];
    snprintf(fn,32,FontGlyphFilenameFormat,pExtFont->Height);
  
    FAT_FILE *pf=Shell_FAT_fopen_TextFont(fn);
    if(pf==NULL) StopFatalError(18404,"ExtFont_LoadBody: Not found font glyph file.\n");
  
    pExtFont->ppBodyLink=(u8**)safemalloc_chkmem(&MM_Process,sizeof(u8**)*ExtFontCount);
  
    u32 BodySize=FAT2_GetFileSize(pf)-ExtFontCount;
    pExtFont->pBody=(u8*)safemalloc_chkmem(&MM_Process,BodySize);
  
    u8 *pDiffTbl=(u8*)safemalloc_chkmem(&MM_Temp,ExtFontCount);
  
    FAT2_fread_fast(pDiffTbl,1,ExtFontCount,pf);
  
    FAT2_fread_fast(pExtFont->pBody,1,BodySize,pf);
  
    u32 LastOffset=0;
  
    for(int idx=0;idx<ExtFontCount;idx++){
        if(pDiffTbl[idx]==0){
            pExtFont->ppBodyLink[idx]=NULL;
        }else{
            LastOffset+=pDiffTbl[idx];
            pExtFont->ppBodyLink[idx]=&pExtFont->pBody[LastOffset];
        }
    }
  
    if(pDiffTbl!=NULL){
        safefree(&MM_Temp,pDiffTbl); pDiffTbl=NULL;
    }
  
    FAT2_fclose(pf);
}

void ExtFont_DrawFont1bpp16pix(const u8 *pBulkData,u16 *pbuf,u32 Height,u32 TextColor)
{
	asm volatile (
	"ScreenWidth = 256 \n\t"
	"efdf1b16_pBulkData .req r0 \n\t"
	"efdf1b16_pbuf .req r1 \n\t"
	"efdf1b16_Height .req r2 \n\t"
	"efdf1b16_TextColor .req r3 \n\t"
	"efdf1b16_BitImage .req lr \n\t"
	" \n\t"
	"stmfd sp!,{lr} \n\t"
	" \n\t"
	"efdf1b16_DrawFont1bpp_LoopYStart: \n\t"
	" \n\t"
	"ldrh efdf1b16_BitImage,[efdf1b16_pBulkData],#2 \n\t"
	" \n\t"
	"tst efdf1b16_BitImage,#1<<0 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*0] \n\t"
	"tst efdf1b16_BitImage,#1<<1 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*1] \n\t"
	"tst efdf1b16_BitImage,#1<<2 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*2] \n\t"
	"tst efdf1b16_BitImage,#1<<3 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*3] \n\t"
	"tst efdf1b16_BitImage,#1<<4 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*4] \n\t"
	"tst efdf1b16_BitImage,#1<<5 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*5] \n\t"
	"tst efdf1b16_BitImage,#1<<6 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*6] \n\t"
	"tst efdf1b16_BitImage,#1<<7 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*7] \n\t"
	"tst efdf1b16_BitImage,#1<<8 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*8] \n\t"
	"tst efdf1b16_BitImage,#1<<9 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*9] \n\t"
	"tst efdf1b16_BitImage,#1<<10 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*10] \n\t"
	"tst efdf1b16_BitImage,#1<<11 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*11] \n\t"
	"tst efdf1b16_BitImage,#1<<12 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*12] \n\t"
	"tst efdf1b16_BitImage,#1<<13 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*13] \n\t"
	"tst efdf1b16_BitImage,#1<<14 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*14] \n\t"
	"tst efdf1b16_BitImage,#1<<15 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*15] \n\t"
	" \n\t"
	"add efdf1b16_pbuf,#ScreenWidth*2 \n\t"
	" \n\t"
	"subs efdf1b16_Height,#1 \n\t"
	"bne efdf1b16_DrawFont1bpp_LoopYStart \n\t"
	" \n\t"
	"ldmfd sp!, {pc} \n\t"
	".ltorg"
	:::"memory"
	);
}

void ExtFont_DrawFont1bpp16pix_fast(const u8 *pBulkData,u16 *pbuf,u32 Height,u32 TextColor)
{
	asm volatile (
	"ScreenWidth = 256 \n\t"
	"efdf1b16_pBulkData .req r0 \n\t"
	"efdf1b16_pbuf .req r1 \n\t"
	"efdf1b16_Height .req r2 \n\t"
	"efdf1b16_TextColor .req r3 \n\t"
	"efdf1b16_BitImage .req lr \n\t"
	" \n\t"
	"stmfd sp!,{lr} \n\t"
	" \n\t"
	"efdf1b16_DrawFont1bpp_fast_LoopYStart: \n\t"
	" \n\t"
	"ldr efdf1b16_BitImage,[efdf1b16_pBulkData],#4 \n\t"
	" \n\t"
	"tst efdf1b16_BitImage,#1<<0 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*0] \n\t"
	"tst efdf1b16_BitImage,#1<<1 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*1] \n\t"
	"tst efdf1b16_BitImage,#1<<2 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*2] \n\t"
	"tst efdf1b16_BitImage,#1<<3 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*3] \n\t"
	"tst efdf1b16_BitImage,#1<<4 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*4] \n\t"
	"tst efdf1b16_BitImage,#1<<5 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*5] \n\t"
	"tst efdf1b16_BitImage,#1<<6 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*6] \n\t"
	"tst efdf1b16_BitImage,#1<<7 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*7] \n\t"
	"tst efdf1b16_BitImage,#1<<8 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*8] \n\t"
	"tst efdf1b16_BitImage,#1<<9 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*9] \n\t"
	"tst efdf1b16_BitImage,#1<<10 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*10] \n\t"
	"tst efdf1b16_BitImage,#1<<11 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*11] \n\t"
	"tst efdf1b16_BitImage,#1<<12 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*12] \n\t"
	"tst efdf1b16_BitImage,#1<<13 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*13] \n\t"
	"tst efdf1b16_BitImage,#1<<14 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*14] \n\t"
	"tst efdf1b16_BitImage,#1<<15 \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*15] \n\t"
	" \n\t"
	"add efdf1b16_pbuf,#ScreenWidth*2 \n\t"
	" \n\t"
	"tst efdf1b16_BitImage,#1<<(16+0) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*0] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+1) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*1] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+2) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*2] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+3) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*3] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+4) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*4] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+5) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*5] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+6) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*6] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+7) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*7] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+8) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*8] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+9) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*9] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+10) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*10] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+11) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*11] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+12) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*12] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+13) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*13] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+14) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*14] \n\t"
	"tst efdf1b16_BitImage,#1<<(16+15) \n\t"
	"strneh efdf1b16_TextColor,[efdf1b16_pbuf,#2*15] \n\t"
	" \n\t"
	"add efdf1b16_pbuf,#ScreenWidth*2 \n\t"
	" \n\t"
	"subs efdf1b16_Height,#2 \n\t"
	"bne efdf1b16_DrawFont1bpp_fast_LoopYStart \n\t"
	" \n\t"
	"ldmfd sp!, {pc} \n\t"
	".ltorg"
	:::"memory"
	);
}

static void ExtFont_DrawFont(CglCanvas *pCanvas,const int x,const int y,UnicodeChar uidx,const u32 Height)
{
    const u8 *pBulkData=pExtFont->ppBodyLink[uidx];
    if(pBulkData==NULL) return;
      
    u16 *pbuf=pCanvas->GetScanLine(y);
    pbuf+=x;
      
    ExtFont_DrawFont1bpp16pix_fast(pBulkData,pbuf,Height,pExtFont->Color);
}

static void ExtFont_TextOutW(CglCanvas *pCanvas,const int x,const int y,const UnicodeChar *pstr)
{
    if(pExtFont->UseClearTypeFont==true){
        CCTF *pCTF=pExtFont->pCTF;
        pCTF->SetTargetCanvas(pCanvas);
        pCTF->TextOutW(x,y,pstr);
        return;
    }
  
    const u32 Height=pExtFont->Height;
  
    int dx=x;
    int dy=y;
  
    const u32 CanvasWidth=ScreenWidth; // 横サイズ256ピクセル以外には描画できない。
  
    while(*pstr!=0){
        const u16 widx=*pstr++;
        const u32 w=pExtFont->Widths[widx];
        if(CanvasWidth<(dx+w)) return;
        ExtFont_DrawFont(pCanvas,dx,dy,widx,Height);
        dx+=w;
    }
}

