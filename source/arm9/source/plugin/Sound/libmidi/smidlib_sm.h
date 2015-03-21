
#ifndef smidlib_sm_h
#define smidlib_sm_h

typedef struct {
  u8 ID[4];
  u32 Len;
  u16 Format;
  u16 Track;
  u16 TimeRes;
} TSM_Chank;

typedef struct {
  bool EndFlag;
  u8 ID[4];
  u8 *Data,*DataEnd;
  u8 RunningStatus;
  int WaitClock;
} TSM_Track;

#define SM_TracksCountMax (32)

typedef struct {
  u8 *File;
  u32 FilePos;
  
  bool FastNoteOn;
  
  u32 SampleRate;
  u32 SamplePerClockFix16;
  
  TSM_Chank SM_Chank;
  
  TSM_Track SM_Tracks[SM_TracksCountMax];
} TStdMIDI;

extern char *gME_Text,*gME_Copyright,*gME_Title;

extern TStdMIDI StdMIDI;

extern void SM_Init(void);
extern void SM_Free(void);

extern void SM_ProcMetaEvent(void);
extern bool SM_isAllTrackEOF(void);
extern void SM_LoadStdMIDI(u8 *FilePtr,u32 SampleRate);
extern int SM_GetDeltaTime(TSM_Track *pSM_Track);
extern void SM_ProcStdMIDI(bool ShowMessage,bool EnableNote,TSM_Track *pSM_Track);

extern u32 SM_GetSamplePerClockFix16(void);

#endif

