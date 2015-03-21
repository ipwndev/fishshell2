
#ifndef _consoleWriteLog_h
#define _consoleWriteLog_h

//#define WriteLogFILELINE

#ifdef WriteLogFILELINE
#define cwl(); _cwl(__FILE__,__LINE__);
#else
#define cwl(); 
#endif

extern void PrfStart(void);
extern void PrfEnd(int data);
extern u32 PrfEnd_GetCPUCount(void);
extern u32 PrfEnd_Getus(void);

static inline void _cwl(char *file,int line)
{
  char *seek=file;
  
  while(*seek!=0){
    if(*seek=='/') file=seek;
    seek++;
  }
  
  _consolePrintf("%s%d",file,line);
}

#endif
