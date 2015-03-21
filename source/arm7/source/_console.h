#ifndef _console_h
#define _console_h

//#define EnableConsoleToARM9

#ifdef EnableConsoleToARM9

extern void _consolePrint(const char* s);
extern void _consolePrintf(const char* format, ...);

#else

static inline void _consolePrint(const char* s) {
}

static inline void _consolePrintf(const char* format, ...) {
}

#endif

#endif
