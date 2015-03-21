//////////////////////////////////////////////////////////////////////
//
// ipc.h -- Inter-processor communication
//
// version 0.1, February 14, 2005
//
//  Copyright (C) 2005 Michael Noland (joat) and Jason Rogers (dovoto)
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any
//  damages arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any
//  purpose, including commercial applications, and to alter it and
//  redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source
//     distribution.
//
// Changelog:
//   0.1: First version
//
//////////////////////////////////////////////////////////////////////

#ifndef NDS_IPC6_INCLUDE
#define NDS_IPC6_INCLUDE

//////////////////////////////////////////////////////////////////////
#include <time.h>
#include <nds.h>

//////////////////////////////////////////////////////////////////////

typedef enum {SPF_PCMx1=1,SPF_PCMx2,SPF_PCMx4,SPF_MP2,SPF_DWORD=0xffff} EstrpcmFormat;

typedef enum {ESC_NOP=0,ESC_Play,ESC_Stop,ESC_DWORD=0xffff} EstrpcmControl;

typedef enum {LCDPC_NOP=0,LCDPC_OFF_BOTH,LCDPC_ON_BOTTOM,LCDPC_ON_TOP,LCDPC_ON_BOTH,LCDPC_SOFT_POWEROFF,LCDPC_DWORD=0xffff} ELCDPC;

typedef enum {RESET_NULL,RESET_HomeBrew,RESET_Commercial,RESET_DWORD=0xffff} ERESET;

typedef enum {IR_NULL,IR_NextSoundData,IR_Flash,IR_MP2_fread,IR_SyncSamples,IR_DWORD=0xffff} EIPCREQ;

typedef enum {E7SC_OK,E7SC_StackOverflow_SVC,E7SC_StackOverflow_IRQ,E7SC_StackOverflow_SYS,E7SC_DWORD=0xffff} EARM7SelfCheck;

typedef struct {
    bool Apply;
    u32 Volume;
    u32 Count;
    bool AddLong;
} TClickSE;

#define IPC6_StartFlag_Data (0xabbccdde)

typedef struct {
  void *pCopyFrom,*pCopyTo;
  u32 CopySize;
  u32 ExecAddr;
} TARMInfo;

typedef enum {DST_DS=0,DST_DSLite,DST_DSi} EDSType;

//copied from old libnds 1.4.x
//---------------------------------------------------------------------------------
typedef struct sTransferSoundData {
//---------------------------------------------------------------------------------
  const void *data;
  u32 len;
  u32 rate;
  u8 vol;
  u8 pan;
  u8 format;
  u8 PADDING;
} TransferSoundData, * pTransferSoundData;


//---------------------------------------------------------------------------------
typedef struct sTransferSound {
//---------------------------------------------------------------------------------
  TransferSoundData data[16];
  u8 count;
  u8 PADDING[3];
} TransferSound, * pTransferSound;


typedef struct sTransferRegion6 {
    u32 StartFlagARM7,StartFlagARM9;
    
    u32 DSCT_SDHCFlag;
      
    u32 RESET;
    TARMInfo ARMInfo7,ARMInfo9;
    bool RequestClearMemory;
    
    uint32 heartbeat;          // counts frames
    u32 UserLanguage;
    
    bool RequestUpdateIPC;
      
    u32 XYButtons;            // X, Y, /PENIRQ , LID buttons
    bool PanelOpened;
      
    u32 touchXpx,touchYpx;
      
    EDSType DSType;
    u32 Brightness,DefaultBrightness; // 0=darkness 1=dark 2=light 3=lightness

    u32 BirthMonth;
    u32 BirthDay;

    ELCDPC LCDPowerControl;

    u32 RESET_BootAddress;

    bool RequestStopSound;
    bool LoopSound;
    
    // for PCM streaming

    EIPCREQ IR;

    EstrpcmControl strpcmControl;
    u32 strpcmWriteRequest;
    u32 strpcmFreq,strpcmSamples,strpcmChannels;
    u32 strpcmAudioVolume64;
    u32 strpcmVideoVolume64;
    EstrpcmFormat strpcmFormat;
    u32 *strpcmLRBuf;
    
    int IR_filesize;
    int IR_readsize;
    void *IR_readbuf;
    int IR_readbufsize;
    bool IR_EOF;
    bool IR_flash;
    u64 IR_SyncSamples_SendToARM9;

    bool MP2PauseFlag;
    
    bool curtimeFlag;
    union {
        uint8 curtime[8];        // current time response from RTC

        struct {
            vu8 command;
            vu8 year;    //add 2000 to get 4 digit year
            vu8 month;    //1 to 12
            vu8 day;    //1 to (days in month)

            vu8 weekday;  // day of week
            vu8 hours;    //0 to 11 for AM, 52 to 63 for PM
            vu8 minutes;  //0 to 59
            vu8 seconds;  //0 to 59
        } rtc;
    } time;

    EARM7SelfCheck ARM7SelfCheck;

    bool RequestFPGAInit;
    bool Romeo2_HPSwitch_Pressing;

    u32 SoundChannels;

    bool ExternalPowerPresent;

    TClickSE ClickSE;

    bool DPG_ControlPowerLED;
    
    u32 *ARM7_DebugLogBuf;
    bool ARM7_DebugLogFlag;
	//sound readded.
	pTransferSound soundData;
} TransferRegion6, * pTransferRegion6;

// copied from libnds_src 1.5.4
//---------------------------------------------------------------------------------
typedef struct __TransferRegion {
//---------------------------------------------------------------------------------
	vs16 touchX,   touchY;		// TSC X, Y
	vs16 touchXpx, touchYpx;	// TSC X, Y pixel values
	vs16 touchZ1,  touchZ2;		// TSC x-panel measurements
	vu16 buttons;				// X, Y, /PENIRQ buttons
	time_t	unixTime;
	struct __bootstub *bootcode;
} __TransferRegion;

// copied from old libnds_src

static inline
__TransferRegion volatile * getIPC() {
	return (__TransferRegion volatile *)(0x02fFF000);
}

#define IPC getIPC()

#define IPC_PEN_DOWN BIT(6)
#define IPC_X BIT(0)
#define IPC_Y BIT(1)
#define IPC_LID_CLOSED BIT(7)
// end of copy
//////////////////////////////////////////////////////////////////////

#define IPC6 ((TransferRegion6 volatile *)(0x02fFF000+sizeof(__TransferRegion)))

#endif

//////////////////////////////////////////////////////////////////////

