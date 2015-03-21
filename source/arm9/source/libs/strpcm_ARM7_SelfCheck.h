
static void ARM7_SelfCheck_Check(void)
{
  EARM7SelfCheck E7SC=IPC6->ARM7SelfCheck;
  if(E7SC==E7SC_OK) return;
  
  REG_IME=0;
  
  switch(E7SC){
//    case E7SC_OK: break;
    case E7SC_StackOverflow_SVC: StopFatalError(14501,"ARM7/SVC stack overflow signal received.\n"); break;
    case E7SC_StackOverflow_IRQ: StopFatalError(14502,"ARM7/IRQ stack overflow signal received.\n"); break;
    case E7SC_StackOverflow_SYS: StopFatalError(14503,"ARM7/SYS stack overflow signal received.\n"); break;
    default: StopFatalError(14504,"ARM7/Unknown(0x%x) stack overflow signal received.\n",E7SC); break;
  }
  
  while(1);
}

