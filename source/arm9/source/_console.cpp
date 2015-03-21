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
#include "_const.h"
#include "memtool.h"
#include "maindef.h"
#include "disc_io.h"
#include "splash.h"
#include "shell.h"

#include "../../ipc6.h"

/*! \brief Send a message to the no$gba debug window 
\param message The message to send
*/
extern "C" void nocashMessage(const char *message);

void nocashMessageSafe(const char *s){
#ifdef _LIBNDS_MINOR_
	const int LENGTH=112;
	int i=0,c;
	for(;i+LENGTH<strlen(s);i+=LENGTH){
		c=s[i+LENGTH];
		((char*)s)[i+LENGTH]=0;
		nocashMessage(s+i);
		((char*)s)[i+LENGTH]=c;
	}
	nocashMessage(s+i);
#endif
}

bool VerboseDebugLog=true;
/////////////////////////////////////////
//global console variables

#define CONSOLE_USE_COLOR255 16

#define CONSOLE_MAPWIDTH (64)
#define CONSOLE_WIDTH (256/6)
#define CONSOLE_HEIGHT (192/6)
#define TAB_SIZE 3

//map to print to
static u16* fontMap;

//location of cursor
static u8 row, col;

static bool LogOutFlag=true;

static char common_buffer[256];        // thread unsafe, but less use of stack

#include "_console_LogFile.h"

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

///////////////////////////////////////////////////////////
//consoleInit
// param: 
//		font: 16 color font
//		charBase: the location the font data will be loaded to
//		numCharacters: count of characters in the font
//		charStart: The ascii number of the first character in the font set
//					if you have a full set this will be zero
//		map: pointer to the map you will be printing to.
//		pal: specifies the 16 color palette to use, if > 15 it will change all non-zero
//			entries in the font to use palette index 255
/*
static void _consoleInit256(u16* font, u16* charBase, u16 numCharacters, u16* map)
{
	int i;

	row = col = 0;
	
	fontMap = map;

	for (i = 0; i < numCharacters * (8*8*8/16); i++){ // 8x8x8bit (16bitBus)
		charBase[i] = font[i];
	}
}
*/

static void _consoleInit256Packed(u8* packedfont, u16* charBase, u16 numCharacters, u16* map)
{
	int i;

	col=0;
	row=CONSOLE_HEIGHT-1;
	
	fontMap = map;

	for (i = 0; i < numCharacters * (8*8*2/8); i++){ // 8x8x2bit (8bitBus)
	  u8 src=packedfont[i];
	  {
	    u16 data=0;
	    data|=((src>>0) & 3);
	    data|=((src>>2) & 3) << 8;
	    charBase[i*2+0]=data;
	  }
	  {
	    u16 data=0;
	    data|=((src>>4) & 3);
	    data|=((src>>6) & 3) << 8;
	    charBase[i*2+1]=data;
	  }
	}
}

#include "_console_font_fixed6x6_packed_bin.h"

void _consoleInitDefault(u16* map, u16* charBase) {
//	_consoleInit256((u16*)_console_font_fixed6x6_bin, charBase, 256, map);
	_consoleInit256Packed((u8*)_console_font_fixed6x6_packed_bin, charBase, 256, map);
	_consoleClear();
}

void _consolePrintSet(int x, int y) {
	if(y < CONSOLE_HEIGHT)
		row = y;
	else
		row = CONSOLE_HEIGHT - 1;

	if(x < CONSOLE_WIDTH)
		col = x;
	else
		col = CONSOLE_WIDTH - 1;
}

int _consoleGetPrintSetY(void)
{
  return(row);
}

void _consoleClear(void)
{
//	for(int i = 0; i < CONSOLE_HEIGHT * (CONSOLE_MAPWIDTH/2); i++) fontMap[i] = 0;
	
	{
	  u16 *dmadst=&fontMap[0*(CONSOLE_MAPWIDTH/2)];
	  u16 dmasize=CONSOLE_HEIGHT*CONSOLE_MAPWIDTH;
	  DCache_FlushRangeOverrun((void*)dmadst,dmasize);
    static vu32 stNullChar=0;
	  DMA3_SRC = (uint32)&stNullChar;
	  DMA3_DEST = (uint32)dmadst;
	  DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_32_BIT | (dmasize>>2);
	  while(DMA3_CR & DMA_BUSY);
	}
	
	_consolePrintSet(0,0);
}

void _consolePrintChar_ScrollLine(u16 *psrc,u16 *pdst,u32 size)
{
	asm volatile (
		"psrc .req r0 \n\t"
		"pdst .req r1 \n\t"
		"size .req r2 \n\t"
		" \n\t"
		"stmfd sp!, {r4,r5,r6} \n\t"
		" \n\t"
		"copy32bitx4: \n\t"
		"ldmia psrc!,{r3,r4,r5,r6} \n\t"
		"stmia pdst!,{r3,r4,r5,r6} \n\t"
		"subs size,size,#4*4 \n\t"
		"cmp size,#4*4 \n\t"
		"bne copy32bitx4 \n\t"
		" \n\t"
		"mov r3,#0 \n\t"
		"mov r4,#0 \n\t"
		"mov r5,#0 \n\t"
		"mov r6,#0 \n\t"
		" \n\t"
		"stmia pdst!,{r3,r4,r5,r6} \n\t"
		"stmia pdst!,{r3,r4,r5,r6} \n\t"
		"stmia pdst!,{r3,r4,r5,r6} \n\t"
		"stmia pdst!,{r3,r4,r5} \n\t"
		" \n\t"
		"ldmfd sp!, {r4,r5,r6} \n\t"
		"bx lr \n\t"
		:::"memory"
	);
}

static __attribute__ ((always_inline)) void _consolePrintChar(u32 c)
{
	if(col >= CONSOLE_WIDTH) {
		col = 0;
		row++;		
	}
	
	if(row >= CONSOLE_HEIGHT) {
		row--;
		_consolePrintChar_ScrollLine(&fontMap[1*(CONSOLE_MAPWIDTH/2)],&fontMap[0*(CONSOLE_MAPWIDTH/2)],(CONSOLE_HEIGHT-1)*CONSOLE_MAPWIDTH);
	}
	
	switch(c){
	  case 10: case 11: case 12: case 13: {
	    row++;
	    col = 0;
	  } break;
	  case 9: {
	    col += TAB_SIZE;
	  } break;
	  default: {
	    u32 ofs=(col/2) + (row * (CONSOLE_MAPWIDTH/2));
	    u16 data=fontMap[ofs];
	    u16 dst=(u16)c;
	    if((col&1)==0){
	      data=(data&0xff00) | (dst << 0);
	      }else{
	      data=(data&0x00ff) | (dst << 8);
	    }
	    fontMap[ofs]=data;
	    
	    col++;
	  } break;
	}
}

extern "C" void f_consolePrintChar(u32 c) 
{ 
	char s[2];
	s[0]=c;
	s[1]='\0';
	StoreLogSector(s);
	
	if(LogOutFlag==false) return;
	_consolePrintChar(c);
}

void _consolePrint(const char* s)
{
//  DTCM_StackCheck(-1); // DPGモードでは使ってはいけない。
  nocashMessageSafe(s);
	
  StoreLogSector(s);
  
  if(LogOutFlag==false) return;
  
  while(*s!=0){
    char c0=s[0],c1=s[1];
    if(0x80<=c0){
      _consolePrintChar('?');
      if(c1==0) return;
      s+=2;
      }else{
      _consolePrintChar(c0);
      s+=1;
    }
  }
}

void _consolePrintf(const char* format, ...)
{
  va_list args;
  
  va_start( args, format );
  vsnprintf( common_buffer, 255, format, args );
  va_end( args );
  _consolePrint(common_buffer);
}

#ifdef EnableConsoleToARM9

void _consoleInitPrintServer(bool flag)
{
	IPC6->ARM7_DebugLogBuf=NULL;
	IPC6->ARM7_DebugLogFlag=false;
	
	if(!flag) return;
	
    // first, allocate buffer and register it.
	TPrintServerBuffer *p = (TPrintServerBuffer *)safemalloc(&MM_System,sizeof(TPrintServerBuffer));
    if(p!=NULL) {
    	MemSet32CPU(0,(void*)p,sizeof(TPrintServerBuffer));
    	p->in = p->out = 0UL;
    	
    	IPC6->ARM7_DebugLogBuf=(u32*)p;
    	IPC6->ARM7_DebugLogFlag=true;
    }
}

void _consoleFreePrintServer(void)
{
	if(IPC6->ARM7_DebugLogBuf!=NULL) {
		safefree(&MM_System,IPC6->ARM7_DebugLogBuf);
		IPC6->ARM7_DebugLogBuf=NULL;
	}
}

void _consolePrintServer(void)
{
	if(IPC6->ARM7_DebugLogBuf==NULL || !IPC6->ARM7_DebugLogFlag) return;
	
    TPrintServerBuffer *p;
    register TPrintWChar word;

    u32     in, out;
    int     i;

    p = (TPrintServerBuffer *)IPC6->ARM7_DebugLogBuf;

    //---- If print buffer isn't set up, do nothing.
    if (!p)
    {
        return;
    }

    out = p->out;
    in = p->in;

    while (in != out)
    {
        i = 0;

        while (in != out && i < sizeof(common_buffer) - 3)
        {
            word.s = p->buffer[out].s;
            if (word.c[0])
            {
                common_buffer[i++] = word.c[0]; // store via cache

                if (word.c[1])
                {
                    common_buffer[i++] = word.c[1];     // store via cache
                }
            }
            out++;
            if (out >= PRINTSRV_WCHARSIZE)
                out = 0;
        }
        common_buffer[i] = '\0';       // store via cache
        _consolePrint(common_buffer);
    }

    // tell finished
    p->out = out;
}
#endif
// ----------------------------------------------------------------------


