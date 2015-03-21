
#define Trigger_ReleaseTimeout (16) // クリック間隔（ダブルクリック）許容時間
#define Trigger_PressTimeout (12) // 長押し判定時間

// ダブルクリックがシングル二回と誤認識してしまうなら、ReleaseTimeoutを増やす。
// クリックが長押しと誤認識してしまうなら、PressTimeoutを増やす。

CODE_IN_AfterSystem static __attribute__ ((noinline)) void Proc_Trigger(u32 Init,u32 VsyncCount)
{
	DATA_IN_AfterSystem static ETriggerType TriggerType;
  if(Init==true){
    TriggerType=ETT_AButton;
  }
  
  DATA_IN_AfterSystem static bool Process;
  DATA_IN_AfterSystem static bool WaitLongEnd,ProcessingLong,ProcessingSingleLong,ProcessingDoubleLong;
  if(Init==true){
    Process=false;
    WaitLongEnd=false;
    ProcessingLong=false;
    ProcessingSingleLong=false;
    ProcessingDoubleLong=false;
  }
  
  DATA_IN_AfterSystem static u32 ClickCount;
  DATA_IN_AfterSystem static u32 LeaveTime;
  DATA_IN_AfterSystem static bool LastPress;
  if(Init==true){
    ClickCount=0;
    LeaveTime=0;
    LastPress=true;
  }
  
  if(Init==true) return;
  
  bool hppress=false;
  
  {
     u32 KEYS_Cur=(~REG_KEYINPUT)&0x3ff;
    if((KEYS_Cur&(KEY_A|KEY_L|KEY_R))!=0){
      hppress=true;
      if((KEYS_Cur&KEY_A)!=0) TriggerType=ETT_AButton;
      if((KEYS_Cur&KEY_L)!=0) TriggerType=ETT_LButton;
      if((KEYS_Cur&KEY_R)!=0) TriggerType=ETT_RButton;
    }
  }
   
  {
	if((IPC6->XYButtons&KEY_TOUCH)!=0){
		hppress=true;
		TriggerType=ETT_Touch;
	}
  }
  
  if(false){ // for debug.
    if((IPC6->XYButtons&IPC_X)!=0){
      hppress=true;
      TriggerType=ETT_PhoneSwitch;
    }
  }
  
  if(isExistsROMEO2==true){
    if(IPC6->Romeo2_HPSwitch_Pressing==true){
      hppress=true;
      TriggerType=ETT_PhoneSwitch;
    }
  }
  
  if(hppress==true){
    if(KEYS_HPSwitch_Pressed==false){
      if(CallBack.Trigger_Down!=NULL) CallBack.Trigger_Down(TriggerType);
    }
    KEYS_HPSwitch_Pressed=true;
    }else{
    if(KEYS_HPSwitch_Pressed==true){
      if(CallBack.Trigger_Up!=NULL) CallBack.Trigger_Up(TriggerType);
    }
    KEYS_HPSwitch_Pressed=false;
  }
  
  const u32 ReleaseTimeout=Trigger_ReleaseTimeout;
  const u32 PressTimeout=Trigger_PressTimeout;
  
  if((WaitLongEnd==true)&&(hppress==false)){
    WaitLongEnd=false;
    if(ProcessingLong==true){
      ProcessingLong=false;
      if(CallBack.Trigger_LongEnd!=NULL) CallBack.Trigger_LongEnd(TriggerType);
    }
    if(ProcessingSingleLong==true){
      ProcessingSingleLong=false;
      if(CallBack.Trigger_SingleLongEnd!=NULL) CallBack.Trigger_SingleLongEnd(TriggerType);
    }
    if(ProcessingDoubleLong==true){
      ProcessingDoubleLong=false;
      if(CallBack.Trigger_DoubleLongEnd!=NULL) CallBack.Trigger_DoubleLongEnd(TriggerType);
    }
    if(CallBack.Trigger_ProcEnd!=NULL) CallBack.Trigger_ProcEnd(TriggerType);
    return;
  }
  
  if((Process==false)&&(WaitLongEnd==false)&&(hppress==true)){
    if(CallBack.Trigger_ProcStart!=NULL) CallBack.Trigger_ProcStart(TriggerType);
    Process=true;
    ClickCount=0;
    LeaveTime=0;
    LastPress=hppress;
  }
  
  for(u32 idx=0;idx<VsyncCount;idx++){
    if(Process==true){
      LeaveTime++;
      if(LastPress!=hppress){
        if(LastPress==false){
          // Release -> Press
          }else{
          // Press -> Release
          ClickCount++;
          if(ClickCount==3){ // Triple click.
            Process=false;
            if(CallBack.Trigger_TripleClick!=NULL) CallBack.Trigger_TripleClick(TriggerType);
            if(CallBack.Trigger_ProcEnd!=NULL) CallBack.Trigger_ProcEnd(TriggerType);
            break;
          }
        }
        LastPress=hppress;
        LeaveTime=0;
      }
      if(LastPress==false){
        if(ReleaseTimeout<=LeaveTime){ // Release time out.
          // Single/double click.
          Process=false;
          switch(ClickCount){
            case 1: {
              if(CallBack.Trigger_SingleClick!=NULL) CallBack.Trigger_SingleClick(TriggerType);
            } break;
            case 2: {
              if(CallBack.Trigger_DoubleClick!=NULL) CallBack.Trigger_DoubleClick(TriggerType);
            } break;
          }
          if(CallBack.Trigger_ProcEnd!=NULL) CallBack.Trigger_ProcEnd(TriggerType);
          break;
        }
        }else{
        if(PressTimeout<=LeaveTime){ // Press time out.
          // Long or single/double click and long.
          Process=false;
          WaitLongEnd=true;
          switch(ClickCount){
            case 0: {
              ProcessingLong=true;
              if(CallBack.Trigger_LongStart!=NULL) CallBack.Trigger_LongStart(TriggerType);
            } break;
            case 1: {
              ProcessingSingleLong=true;
              if(CallBack.Trigger_SingleLongStart!=NULL) CallBack.Trigger_SingleLongStart(TriggerType);
            } break;
            case 2: {
              ProcessingDoubleLong=true;
              if(CallBack.Trigger_DoubleLongStart!=NULL) CallBack.Trigger_DoubleLongStart(TriggerType);
            } break;
          }
          break;
        }
      }
    }
  }
  
}
