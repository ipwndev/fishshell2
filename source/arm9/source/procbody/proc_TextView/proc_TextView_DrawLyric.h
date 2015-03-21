static void TextView_DrawLyric(CglCanvas *pDstBM) {

	u32 cursec, playsec;
	bool ShowTime=DLLSound_GetTimeSec(&cursec, &playsec);

	if (PlugLRC_isOpened()==false || ShowTime==false) return;

	u32 LinesCount=4; // for Filename.

	const u32 h=glCanvasTextHeight+2;
	u32 x=4, y=ScreenHeight-((h*LinesCount)+8);
	u32 w=ScreenWidth-2*4;
	
	if (true) {//DrawFrame
		const u32 DstBMWidth=ScreenWidth; // pDstBM->GetWidth();

		u16 *pb=pDstBM->GetVRAMBuf();
		u32 size;

		size=y*DstBMWidth;
		pb+=size;

		size=2*DstBMWidth;
		pb=TextView_DrawID3Tag_asm_Fill50per(pb, size);

		size=(2+(h*LinesCount)+0)*DstBMWidth;
		pb=TextView_DrawID3Tag_asm_Fill25per(pb, size);

		//

		size=2*DstBMWidth;
		pb=TextView_DrawID3Tag_asm_Fill25per(pb, size);

		size=2*DstBMWidth;
		pb=TextView_DrawID3Tag_asm_Fill50per(pb, size);
	}

	y+=4;

	//#define UseShadow

#ifdef UseShadow
	u16 collow=RGB15(0,0,0)|BIT15;
#endif
	u16 colhigh=ColorTable.FileList.ID3TagText;
	u16 colnormal=RGB15(16,16,16)|BIT15;
	
  { // for Filename.
    char idxstr[32];
    snprintf(idxstr,32,"%d/%d",1+PlayList_GetCurrentIndex(),PlayList_GetFilesCount());
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutA(x+1,y+1,idxstr);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutA(x+0,y+0,idxstr);
    u32 w=pDstBM->GetTextWidthA(idxstr)+4;

    const UnicodeChar *pstr=PlayList_GetCurrentFilename();
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutW(x+w+1,y+1,pstr);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutW(x+w+0,y+0,pstr);
    
    char str[128];
    snprintf(str,64,"%d:%02d:%02d",cursec/60/60,(cursec/60)%60,cursec%60);

    u32 tx=ScreenWidth-4-pDstBM->GetTextWidthA(str);
    
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutA(tx+1,y+1,str);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutA(tx+0,y+0,str);
    
    y+=h+1;
  }
  
	UnicodeChar pstr[128];
	UnicodeChar LyricStrLine0[128];
	UnicodeChar LyricStrLine1[128];
	
	if (PlugLRC_GetCurLyric(cursec)!=NULL) {
		if (Shell_isEUCmode()) {
			//_consolePrintf("[Convert] %s\n",PlugLRC_GetCurLyric(cursec));
			EUC2Unicode_Convert(PlugLRC_GetCurLyric(cursec), pstr,0);
		} else {
			StrConvert_Ank2Unicode(PlugLRC_GetCurLyric(cursec), pstr);
		}
		
		Unicode_StrWrap2Line(pstr,LyricStrLine0,LyricStrLine1,pDstBM,w);
		
		if(LyricStrLine1[0]==0) y+=h+1;
			
		u32 ofs=(w-pDstBM->GetTextWidthW(LyricStrLine0))/2;
		
#ifdef UseShadow
		pDstBM->SetFontTextColor(collow);
		pDstBM->TextOutW(x+ofs+1,y+1,LyricStrLine0);
#endif
		pDstBM->SetFontTextColor(colhigh);
		pDstBM->TextOutW(x+ofs+0, y+0, LyricStrLine0);
		
		if(LyricStrLine1[0]!=0){
			y+=h+1;
			ofs=(w-pDstBM->GetTextWidthW(LyricStrLine1))/2;
#ifdef UseShadow
			pDstBM->SetFontTextColor(collow);
			pDstBM->TextOutW(x+ofs+1,y+1,LyricStrLine1);
#endif
			pDstBM->SetFontTextColor(colhigh);
			pDstBM->TextOutW(x+ofs+0, y+0, LyricStrLine1);
		}
	}
	
	if (LyricStrLine1[0]==0 && PlugLRC_GetPrevLyric()!=NULL) {
		y-=(h+1);
		if (Shell_isEUCmode()) {
			//_consolePrintf("[Convert] %s\n",PlugLRC_GetCurLyric(cursec));
			EUC2Unicode_Convert(PlugLRC_GetPrevLyric(), pstr,0);
		} else {
			StrConvert_Ank2Unicode(PlugLRC_GetPrevLyric(), pstr);
		}
			
		Unicode_StrWrap2Line(pstr,LyricStrLine0,LyricStrLine1,pDstBM,w);
			
		if(LyricStrLine1[0]==0) Unicode_Copy(LyricStrLine1,LyricStrLine0);
				
		u32 ofs=(w-pDstBM->GetTextWidthW(LyricStrLine1))/2;
			
	#ifdef UseShadow
		pDstBM->SetFontTextColor(collow);
		pDstBM->TextOutW(x+ofs+1,y+1,LyricStrLine1);
	#endif
		pDstBM->SetFontTextColor(colnormal);
		pDstBM->TextOutW(x+ofs+0, y+0, LyricStrLine1);
	}
	
	y+=h*2+1;
	
	if (PlugLRC_GetNextLyric()!=NULL) {
		if (Shell_isEUCmode()) {
			//_consolePrintf("[Convert] %s\n",PlugLRC_GetCurLyric(cursec));
			EUC2Unicode_Convert(PlugLRC_GetNextLyric(), pstr,0);
		} else {
			StrConvert_Ank2Unicode(PlugLRC_GetNextLyric(), pstr);
		}
			
		Unicode_StrWrap2Line(pstr,LyricStrLine0,LyricStrLine1,pDstBM,w);

		u32 ofs=(w-pDstBM->GetTextWidthW(LyricStrLine0))/2;
			
	#ifdef UseShadow
		pDstBM->SetFontTextColor(collow);
		pDstBM->TextOutW(x+ofs+1,y+1,LyricStrLine0);
	#endif
		pDstBM->SetFontTextColor(colnormal);
		pDstBM->TextOutW(x+ofs+0, y+0, LyricStrLine0);
	}

#undef UseShadow
}

