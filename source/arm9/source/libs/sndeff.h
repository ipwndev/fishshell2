
#ifndef sndeff_h
#define sndeff_h

extern void Sound_Open(void);
extern void Sound_Close(void);

// èáî‘ïœÇ¶ÇøÇ·É_ÉÅÅB
enum EWAVFN {WAVFN_Opening=0,
			WAVFN_Click,
			WAVFN_MovePage,
			WAVFN_Notify,
			WAVFN_Open,
			WAVFN_LongTap,
			WAVFN_PowerOff,
			WAVFN_MemoPenDown,
			WAVFN_MemoErase,
			WAVFN_MemoUndo,
			WAVFN_Splash
			};

extern void Sound_Start(EWAVFN WAVFN);
extern u32 Sound_GetCurrentPlayTimePerVsync(void);

extern void SoundSE_Start(EWAVFN WAVFN);
extern void SoundSE_StartVol(EWAVFN WAVFN,u32 Volume);
extern void SoundSE_Stop(void);

extern void SoundBGM_Start(EWAVFN WAVFN);
extern void SoundBGM_Stop(void);

extern void SoundSE_IRQVBlankHandler(void);
extern bool SoundSE_MainVBlankHandler(void);

#endif

