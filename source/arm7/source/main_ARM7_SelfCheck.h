#define ARM7_StackLimit_Data (0xa5b6c7d8)
#define ARM7_StackLimit_SVC ((u32*)(0x0380fb00-0x100)) // request 40byte.
#define ARM7_StackLimit_IRQ ((u32*)(0x0380fa00-0x100)) // request 40byte.
#define ARM7_StackLimit_SYS ((u32*)(0x0380f900-0x500)) // request 1024byte.
#define ARM7_StackLimit_Size (32/4) // check range 32byte.

static __attribute__((noinline)) void ARM7_SelfCheck_Init(void)
{
  IPC6->ARM7SelfCheck=E7SC_OK;
  
  u32 *pbuf;
  const u32 data=ARM7_StackLimit_Data;
  
  pbuf=ARM7_StackLimit_SVC;
  for(u32 idx=0;idx<ARM7_StackLimit_Size;idx++){
    pbuf[idx]=data;
  }
  pbuf=ARM7_StackLimit_IRQ;
  for(u32 idx=0;idx<ARM7_StackLimit_Size;idx++){
    pbuf[idx]=data;
  }
  pbuf=ARM7_StackLimit_SYS;
  for(u32 idx=0;idx<ARM7_StackLimit_Size;idx++){
    pbuf[idx]=data;
  }
}

static __attribute__((noinline)) void ARM7_SelfCheck_Check(void)
{
  EARM7SelfCheck E7SC=E7SC_OK;
  
  u32 *pbuf;
  const u32 data=ARM7_StackLimit_Data;
  
  pbuf=ARM7_StackLimit_SVC;
  for(u32 idx=0;idx<ARM7_StackLimit_Size;idx++){
    if(pbuf[idx]!=data) E7SC=E7SC_StackOverflow_SVC;
  }
  pbuf=ARM7_StackLimit_IRQ;
  for(u32 idx=0;idx<ARM7_StackLimit_Size;idx++){
    if(pbuf[idx]!=data) E7SC=E7SC_StackOverflow_IRQ;
  }
  pbuf=ARM7_StackLimit_SYS;
  for(u32 idx=0;idx<ARM7_StackLimit_Size;idx++){
    if(pbuf[idx]!=data) E7SC=E7SC_StackOverflow_SYS;
  }
  
  if(E7SC==E7SC_OK) return;
  
  switch(E7SC){
    case E7SC_OK: break;
    case E7SC_StackOverflow_SVC: _consolePrint("Stack overflow SVC.\n"); break;
    case E7SC_StackOverflow_IRQ: _consolePrint("Stack overflow IRQ.\n"); break;
    case E7SC_StackOverflow_SYS: _consolePrint("Stack overflow SYS.\n"); break;
	default:	break;
  }
  
  IPC6->ARM7SelfCheck=E7SC;
  REG_IME=0;
  while(1);
}

