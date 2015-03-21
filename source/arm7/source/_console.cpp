/*---------------------------------------------------------------------------------
 $Id: console.c,v 1.4 2005/07/14 08:00:57 wntrmute Exp $

 console code -- provides basic print functionality

 Copyright (C) 2005
 Michael Noland (joat)
 Jason Rogers (dovoto)
 Dave Murphy (WinterMute)

 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any
 damages arising from the use of this software.

 Permission is granted to anyone to use this software for any
 purpose, including commercial applications, and to alter it and
 redistribute it freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you
 must not claim that you wrote the original software. If you use
 this software in a product, an acknowledgment in the product
 documentation would be appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and
 must not be misrepresented as being the original software.
 3. This notice may not be removed or altered from any source
 distribution.

 $Log: console.c,v $
 Revision 1.4  2005/07/14 08:00:57  wntrmute
 resynchronise with ndslib
 

 ---------------------------------------------------------------------------------*/

#include <nds.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "_console.h"

#include "../../ipc6.h"

#ifdef EnableConsoleToARM9

//  PrintServer
typedef union
{
    u16     s;
    char    c[2];
}TPrintWChar;

#define PRINTSRV_BUFFERSIZE  1024
#define PRINTSRV_WCHARSIZE   (PRINTSRV_BUFFERSIZE/sizeof(TPrintWChar))

typedef volatile struct
{
    u32     in;
    u32     out;
    TPrintWChar buffer[PRINTSRV_WCHARSIZE];
} TPrintServerBuffer;

static void PutStringPrnSrv(const char *str)
{
	if(IPC6->ARM7_DebugLogBuf==NULL || !IPC6->ARM7_DebugLogFlag) return;
	
    TPrintServerBuffer *p;
    register TPrintWChar word;

    u32     in, in_tmp, out;
    u32     isOdd;

    p = (TPrintServerBuffer *)IPC6->ARM7_DebugLogBuf;
    in = p->in;
    out = p->out;
    isOdd = ((u32)str) & 1;

    while (1)
    {
        in_tmp = in + 1;
        if (in_tmp >= PRINTSRV_WCHARSIZE)
            in_tmp = 0;
        if (out == in_tmp)
            break;                     // Buffer full, then exit

        if (isOdd)
        {
            p->buffer[in].s = word.s = (u16)((*(u16 *)(str - 1)) & 0xff00);
            str++;
            isOdd = 0UL;
            if (!word.c[1])
                break;
            in = in_tmp;
        }
        else
        {
            p->buffer[in].s = word.s = *(u16 *)str;
            str += 2;
            if (!word.c[0])
                break;
            in = in_tmp;
            if (!word.c[1])
                break;
        }
    }
    p->in = in;
}

void _consolePrint(const char* s)
{
	if(IPC6->ARM7_DebugLogBuf==NULL || !IPC6->ARM7_DebugLogFlag) return;
	
    if(strlen(s)==0) return;
    PutStringPrnSrv("ARM7: ");
    PutStringPrnSrv(s);
}

void _consolePrintf(const char* format, ...)
{
	if(IPC6->ARM7_DebugLogBuf==NULL || !IPC6->ARM7_DebugLogFlag) return;
	
    char strbuf[256];

    va_list args;

    va_start( args, format );
    vsnprintf( strbuf, 255, format, args );
    _consolePrint(strbuf);
}

#endif
