
#ifndef smidlib_mtrk_h
#define smidlib_mtrk_h

extern void MTRKCC_Init(void);
extern void MTRKCC_Free(void);

extern void MTRKCC_Proc(u32 trk,u8 data0,u8 data1);
extern void MTRK_SetProgram(u32 trk,u32 v);
extern void MTRK_ChangePitchBend(u32 trk,s32 p);
extern void MTRK_NoteOn_LoadProgram(u32 trk,u32 Note,u32 Vel);
extern void MTRK_NoteOn(u32 trk,u32 GT,u32 Note,u32 Vel);
extern void MTRK_NoteOff(u32 trk,u32 Note,u32 Vel);

extern void MTRK_SetExMap(u32 trk,u32 mode);

#endif

