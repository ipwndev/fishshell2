
#ifndef arm9tcm_h
#define arm9tcm_h

#define CODE_IN_ITCM_Global __attribute__ ((section (".ITCM_Global")))
#define CODE_IN_ITCM_MemTool __attribute__ ((section (".ITCM_MemTool")))
#define CODE_IN_ITCM_GLIB __attribute__ ((section (".ITCM_GLIB")))

#define CODE_IN_ITCM_DPG __attribute__ ((section (".ITCM_libglobal_dpg")))
#define CODE_IN_ITCM_NDSROM __attribute__ ((section (".ITCM_libglobal_ndsrom")))
#define CODE_IN_ITCM_WIFI __attribute__ ((section (".ITCM_libglobal_wifi")))
#define CODE_IN_ITCM_JPEG __attribute__ ((section (".ITCM_libimg_jpeg")))
#define CODE_IN_ITCM_PNG __attribute__ ((section (".ITCM_libimg_png")))
#define CODE_IN_ITCM_BMP __attribute__ ((section (".ITCM_libimg_bmp")))
#define CODE_IN_ITCM_GIF __attribute__ ((section (".ITCM_libimg_gif")))
#define CODE_IN_ITCM_PSD __attribute__ ((section (".ITCM_libimg_psd")))
#define CODE_IN_ITCM_MP3 __attribute__ ((section (".ITCM_libsnd_mp3")))
#define CODE_IN_ITCM_GME __attribute__ ((section (".ITCM_libsnd_gme")))
#define CODE_IN_ITCM_OGG __attribute__ ((section (".ITCM_libsnd_ogg")))
#define CODE_IN_ITCM_MIDI __attribute__ ((section (".ITCM_libsnd_midi")))
#define CODE_IN_ITCM_WAVE __attribute__ ((section (".ITCM_libsnd_wave")))

//bad thing
#define DATA_IN_ITCM_GME __attribute__ ((section (".ITCM_libsnd_gme_DATA")))


#define DATA_IN_DTCM __attribute__ ((section (".dtcm")))

// ---------------------------------------------------
#define DATA_IN_IWRAM_MainPass __attribute__ ((section (".IWRAM_proc_MainPass")))
#define DATA_IN_IWRAM_ChkDsk __attribute__ ((section (".IWRAM_proc_ChkDsk")))
#define DATA_IN_IWRAM_Setup __attribute__ ((section (".IWRAM_proc_Setup")))
#define DATA_IN_IWRAM_FileList __attribute__ ((section (".IWRAM_proc_FileList")))
#define DATA_IN_IWRAM_SysMenu __attribute__ ((section (".IWRAM_proc_SysMenu")))
#define DATA_IN_IWRAM_DPGCustom __attribute__ ((section (".IWRAM_proc_DPGCustom")))
#define DATA_IN_IWRAM_DPGPlay __attribute__ ((section (".IWRAM_proc_DPGPlay")))
#define DATA_IN_IWRAM_ImageCustom __attribute__ ((section (".IWRAM_proc_ImageCustom")))
#define DATA_IN_IWRAM_ImageView __attribute__ ((section (".IWRAM_proc_ImageView")))
#define DATA_IN_IWRAM_TextCustom __attribute__ ((section (".IWRAM_proc_TextCustom")))
#define DATA_IN_IWRAM_TextMenu __attribute__ ((section (".IWRAM_proc_TextMenu")))
#define DATA_IN_IWRAM_TextView __attribute__ ((section (".IWRAM_proc_TextView")))
#define DATA_IN_IWRAM_BinView __attribute__ ((section (".IWRAM_proc_BinView")))
#define DATA_IN_IWRAM_Launch __attribute__ ((section (".IWRAM_proc_Launch")))
#define DATA_IN_IWRAM_Custom __attribute__ ((section (".IWRAM_proc_Custom")))
#define DATA_IN_IWRAM_BootROM __attribute__ ((section (".IWRAM_proc_BootROM")))
#define DATA_IN_IWRAM_MemoEdit __attribute__ ((section (".IWRAM_proc_MemoEdit")))
#define DATA_IN_IWRAM_MemoList __attribute__ ((section (".IWRAM_proc_MemoList")))
#define DATA_IN_IWRAM_AudioPlay __attribute__ ((section (".IWRAM_proc_AudioPlay")))
#define DATA_IN_IWRAM_AudioCustom __attribute__ ((section (".IWRAM_proc_AudioCustom")))
// ---------------------------------------------------
#define DATA_IN_AfterSystem __attribute__ ((section (".OVR_AfterSystem")))
#define DATA_IN_AfterSystem_CONST __attribute__ ((section (".OVR_AfterSystem_CONST")))
#define CODE_IN_AfterSystem __attribute__ ((section (".OVR_AfterSystem1_TEXT")))
#define DATA_IN_AfterSystem2 __attribute__ ((section (".OVR_AfterSystem2")))
#define CODE_IN_AfterSystem2 __attribute__ ((section (".OVR_AfterSystem2_TEXT")))
// ---------------------------------------------------
#endif

