
#ifndef PlaySE_h
#define PlaySE_h

static void PlaySE_Sequence(u32 Count,bool AddLong)
{
  if(ProcState.System.ClickSound==false) return;
  
  while(IPC6->ClickSE.Apply==true);
  
  IPC6->ClickSE.Volume=128;
  IPC6->ClickSE.Count=Count;
  IPC6->ClickSE.AddLong=AddLong;
  IPC6->ClickSE.Apply=true;
}

static void PlaySE_Short(void)
{
  if(ProcState.System.ClickSound==false) return;
  
  while(IPC6->ClickSE.Apply==true);
  
  IPC6->ClickSE.Volume=128;
  IPC6->ClickSE.Count=1;
  IPC6->ClickSE.AddLong=false;
  IPC6->ClickSE.Apply=true;
}

static void PlaySE_Long(void)
{
  if(ProcState.System.ClickSound==false) return;
  
  while(IPC6->ClickSE.Apply==true);
  
  IPC6->ClickSE.Volume=128;
  IPC6->ClickSE.Count=0;
  IPC6->ClickSE.AddLong=true;
  IPC6->ClickSE.Apply=true;
}

#endif

