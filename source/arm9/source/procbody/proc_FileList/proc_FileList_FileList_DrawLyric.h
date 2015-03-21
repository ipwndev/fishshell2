static void FileList_DrawLyric(CglCanvas *pDstBM, bool DrawFrame) {

	u32 cursec, playsec;
	bool ShowTime=DLLSound_GetTimeSec(&cursec, &playsec);

	if (PlugLRC_isOpened()==false || ShowTime==false)
		return;

	u32 LinesCount=3;

	const u32 h=glCanvasTextHeight+2;
	u32 x=4, y=ScreenHeight-((h*LinesCount)+8);
	u32 w=ScreenWidth-2*4;
	
	if (DrawFrame==true) {
		const u32 DstBMWidth=ScreenWidth; // pDstBM->GetWidth();

		u16 *pb=pDstBM->GetVRAMBuf();
		u32 size;

		size=y*DstBMWidth;
		pb+=size;

		size=2*DstBMWidth;
		pb=FileList_DrawID3Tag_asm_Fill50per(pb, size);

		size=(2+(h*LinesCount)+0)*DstBMWidth;
		pb=FileList_DrawID3Tag_asm_Fill25per(pb, size);

		//

		size=2*DstBMWidth;
		pb=FileList_DrawID3Tag_asm_Fill25per(pb, size);

		size=2*DstBMWidth;
		pb=FileList_DrawID3Tag_asm_Fill50per(pb, size);
	}

	y+=5;

	//#define UseShadow

#ifdef UseShadow
	u16 collow=RGB15(0,0,0)|BIT15;
#endif
	u16 colhigh=ColorTable.FileList.ID3TagText;
	u16 colnormal=RGB15(16,16,16)|BIT15;
	
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

