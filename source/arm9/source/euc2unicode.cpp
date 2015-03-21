#include <stdio.h>
#include <stdlib.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "fat2.h"
#include "memtool.h"
#include "shell.h"
#include "strtool.h"
#include "unicode.h"

#include "arm9tcm.h"

#include "euc2unicode.h"

DATA_IN_AfterSystem TEUC2Unicode EUC2Unicode;

DATA_IN_IWRAM_MainPass void EUC2Unicode_Init(void) {
	TEUC2Unicode *ps2u=&EUC2Unicode;

	ps2u->pBinary=NULL;
	ps2u->BinarySize=0;
	ps2u->panktbl=NULL;
	ps2u->ps2utbl=NULL;
	ps2u->pu2stbl=NULL;
}

void EUC2Unicode_Free(void) {
	TEUC2Unicode *ps2u=&EUC2Unicode;

	if (ps2u->pBinary!=NULL) {
		safefree(&MM_System,ps2u->pBinary);
		ps2u->pBinary=NULL;
	}

	if (ps2u->pu2stbl!=NULL) {
		safefree(&MM_Temp,ps2u->pu2stbl);
		ps2u->pu2stbl=NULL;
	}

	EUC2Unicode_Init();
}

DATA_IN_IWRAM_MainPass void EUC2Unicode_Load(void) {
	TEUC2Unicode *ps2u=&EUC2Unicode;

	if (!Shell_isEUCmode())
		return;

	if (ps2u->pBinary!=NULL)
		return;

	FAT_FILE *pf=Shell_FAT_fopen_EUCToUnicodeTable();
	if (pf==NULL)
		StopFatalError(10901,"Not found EUC to Unicode convert table file.\n");

	ps2u->BinarySize=FAT2_GetFileSize(pf);
	ps2u->pBinary=safemalloc_chkmem(&MM_System,ps2u->BinarySize);
	FAT2_fread_fast(ps2u->pBinary, 1, ps2u->BinarySize, pf);
	FAT2_fclose(pf);

	ps2u->panktbl=(const u8*)ps2u->pBinary;
	ps2u->ps2utbl=(const u16*)&ps2u->panktbl[256];
}

void EUC2Unicode_Convert(const char *pStrL, UnicodeChar *pStrW, u32 dstlen) {
	TEUC2Unicode *ps2u=&EUC2Unicode;

	if ((ps2u->panktbl==NULL)||(ps2u->ps2utbl==NULL))
		StopFatalError(10903,"EUC2Unicode_Convert: Table not loaded.\n");

	u32 widx=0;

	while (pStrL[0]!=0) {
		if (dstlen!=0) {
			if (dstlen<=widx)
				break;
		}
		u32 c0=pStrL[0];
		u32 c1=pStrL[1];
		if (ps2u->panktbl[c0]==true) {
			if ((0xa0<=c0)&&(c0<0xe0))
				c0=0xff60+(c0-0xa0);
			pStrW[widx++]=c0;
			pStrL+=1;
		} else {
			u32 euc=(c0<<8)|c1;
			pStrW[widx++]=ps2u->ps2utbl[euc];
			pStrL+=2;
		}
	}

	pStrW[widx]=0;
}

void Unicode2EUC_Convert(const UnicodeChar *pStrW, char *pStrL, u32 dstlen) {
	TEUC2Unicode *ps2u=&EUC2Unicode;
	if ((ps2u->panktbl==NULL)||(ps2u->ps2utbl==NULL))
		StopFatalError(10903,"Unicode2EUC_Convert: Table not loaded.\n");

	if(ps2u->pu2stbl==NULL){
		ps2u->pu2stbl=(u16*)safemalloc_chkmem(&MM_Temp,0x10000*2);
		MemSet32CPU(0, ps2u->pu2stbl, 0x10000*2);
		for (u32 idx=0; idx<0x10000; idx++) {
			UnicodeChar wc=0;
			if (idx<0x100) {
				if (ps2u->panktbl[idx&0xff]==true) {
					wc=idx;
					if ((0xa0<=wc)&&(wc<0xe0))
						wc=0xff60+(wc-0xa0);
				}
			} else {
				if (ps2u->panktbl[idx>>8]==false) {
					wc=ps2u->ps2utbl[idx];
				}
			}
			if (wc!=0) {
				if (ps2u->pu2stbl[wc]!=0) {
					//_consolePrintf("Duplicate: %04x,%04x,%04x.\n",idx,wc,ps2u->pu2stbl[wc]);
				} else {
					ps2u->pu2stbl[wc]=idx;
				}
			}
		}
	}

	u32 lidx=0;

	while (*pStrW!=0) {
		if (dstlen!=0) {
			if (dstlen<=lidx)
				break;
		}

		UnicodeChar uc=*pStrW++;
		u16 euc = ps2u->pu2stbl[uc];

		pStrL[lidx++]=(char)(euc & 0xFF);
		if ( (euc & 0xFF00)) {
			lidx--;
			pStrL[lidx++] = (char)( (euc & 0xFF00) >> 8 );
			pStrL[lidx++] = (char)(euc & 0xFF);
		}
	}
	pStrL[lidx]='\0';

	if (ps2u->pu2stbl!=NULL) {
		safefree(&MM_Temp,ps2u->pu2stbl);
		ps2u->pu2stbl=NULL;
	}
}

