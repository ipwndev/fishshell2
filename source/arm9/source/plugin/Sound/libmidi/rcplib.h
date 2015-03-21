
#ifndef rcplib_h
#define rcplib_h

extern void rcplibSetParam(u8 *data,u32 SampleRate,u32 GenVolume);
extern bool rcplibStart(void);
extern void rcplibFree(void);
extern int rcplibGetNearClock(void);
extern bool rcplibNextClock(bool ShowEventMessage,bool EnableNote,int DecClock);
extern void rcplibAllSoundOff(void);

extern bool RCP_isAllTrackEOF(void);
extern u32 RCP_GetSamplePerClockFix16(void);

enum ERCPTrackPlayMode {ERCPTPM_Play=0,ERCPTPM_Mute=1,ERCPTPM_Mix=2,ERCPTPM_Rec=4};

enum ERCPExcCmd {EEC_None=0x00,EEC_CHExclusive=0x98,EEC_Exec=0x99,EEC_Commment=0xf6};

typedef struct {
  ERCPExcCmd cmd;
  u8 GT,Vel;
  u32 Count;
  u8 Buf[128];
} TRCP_Track_Exc;

typedef struct {
  u8 *pReturn;
  u32 LoopCount;
} TRCP_Track_Loop;

typedef struct {
  bool EndFlag;
  u32 unuseTrackLen;
  u32 unuseTrackNum;
  bool RythmMode;
  u32 MIDICh;
  s32 KeyBias;
  s32 unuseStBias;
  ERCPTrackPlayMode PlayMode;
//  char Comment[36+1];
  u8 *DataTop,*Data,*DataEnd;
  int WaitClock;
  u8 *pReturnSameMeasure;
  u32 LoopFreeIndex;
  TRCP_Track_Loop *pLoop;
  TRCP_Track_Exc *pExc;
} TRCP_Track;

#define RCPTrackMax (18)

typedef struct {
  u8 *File;
  u32 FilePos;
  
  bool FastNoteOn;
  
  u32 TempoFactor;
  
  u32 SampleRate;
  u32 SamplePerClockFix16;
  
  TRCP_Track RCP_Track[RCPTrackMax];
} TRCP;

typedef struct {
  char Title[64+1];
  char Memo[336+1];
  u32 TimeRes;
  u32 Tempo;
  s32 PlayBias;
  u32 TrackCount;
} TRCP_Chank;

extern const TRCP* RCP_GetStruct_RCP(void);
extern const TRCP_Chank* RCP_GetStruct_RCP_Chank(void);

#endif

