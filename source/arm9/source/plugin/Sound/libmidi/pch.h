
#ifndef smidlib_pch_h
#define smidlib_pch_h

#define PCH_ChannelsCount (16)

extern bool MemoryOverflowFlag;

extern void PCH_InitProgramMap(void);
extern void PCH_FreeProgramMap(void);
extern bool PCH_LoadProgram(s32 Note,u32 var,u32 prg,bool DrumMode);

extern bool PCH_Init(u32 _SampleRate,u32 _GenVolume);
extern void PCH_Free(void);

extern void PCH_AllSoundOff(void);
extern void PCH_AllNoteOff(u32 trk);

extern void PCH_NextClock(void);

extern void PCH_ChangeVolume(u32 trk,u32 v);
extern void PCH_ChangeExpression(u32 trk,u32 e);
extern void PCH_ChangePitchBend(u32 trk,s32 Pitch);
extern void PCH_ChangePanpot(u32 trk,u32 p);
extern void PCH_ChangeModLevel(u32 trk,u32 ModLevel);

extern void PCH_NoteOn(u32 trk,u32 GT,s32 Note,s32 Pitch,u32 Vol,u32 Exp,u32 Vel,u32 var,u32 prg,u32 panpot,u32 reverb,bool DrumMode,u32 ModLevel);
extern void PCH_NoteOff(u32 trk,u32 Note,bool DrumMode);
extern void PCH_PedalOn(u32 trk);
extern void PCH_PedalOff(u32 trk);

extern bool PCH_RequestRender(u32 TagChannel);
extern void PCH_RenderStart(u32 SampleCount);
extern void PCH_Render(u32 TagChannel,s32 *buf,u32 SampleCount);
extern void PCH_RenderEnd(void);
extern void PCH_IncrementUnusedClockCount(void);

extern u32 PCH_GetReverb(u32 TagChannel);

extern int PCH_GT_GetNearClock(void);
extern void PCH_GT_DecClock(u32 clk);

extern bool PCH_isDrumMap(u32 TagChannel);

// メモリが足りないときにメモリマネージャのsafemallocから呼ばれる
extern bool memtool_PlugMIDI_RequestFreeMemory(void);

#endif

