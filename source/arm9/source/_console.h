
#ifndef _console_h
#define _console_h

#define ConsoleMaxLen (126)

//#define EnableConsoleToARM9

#ifdef EnableConsoleToARM9
extern void _consoleInitPrintServer(bool flag);
extern void _consoleFreePrintServer(void);
extern void _consolePrintServer(void);
#else
static inline void _consoleInitPrintServer(bool flag) {
}
static inline void _consoleFreePrintServer(void) {
}
static inline void _consolePrintServer(void) {
}
#endif

extern void _consoleInitDefault(u16* map, u16* charBase);
extern void _consoleClear(void);
extern void _consolePrintSet(int x, int y);
extern int _consoleGetPrintSetY(void);

extern void _consolePrint(const char* s);
extern void _consolePrintf(const char* format, ...);

extern void _consoleInitLogFile(void);
extern void _consoleSetLogFile(void *_pf);
extern void _consoleClearLogFile(void);
extern bool _consoleGetLogFile(void);
extern void _consoleSetLogOutFlag(bool f);

extern void _consoleLogPause(void);
extern void _consoleLogResume(void);

extern bool VerboseDebugLog;
#endif
