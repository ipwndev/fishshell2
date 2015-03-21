
#ifndef particle_h
#define particle_h

void Particle_Init(void);
void Particle_Free(void);
void Particle_Update(u32 VsyncCount,CglCanvas *pcan);

void Particle_SetMouseDown(s32 x,s32 y);
void Particle_SetMouseMove(s32 x,s32 y);
void Particle_SetMouseUp(void);

#endif

