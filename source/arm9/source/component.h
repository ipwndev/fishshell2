
#ifndef component_h
#define component_h

#include <nds.h>

#include "glib.h"

#include "rect.h"

extern void Component_SetFont(CglFont *pFont);

typedef struct {
  void (*CallBack_Click)(void *pComponentLabel);
  CglCanvas *pCanvas;
  TRect Rect;
  bool Visible;
  bool isTitle;
  bool Center;
  u16 TextColor;
  const char *pMsgUTF8;
  const UnicodeChar *pMsgUnicode;
  const char *pInfoUTF8;
  const UnicodeChar *pInfoUnicode;
} TComponentLabel;

extern void ComponentLabel_Init(TComponentLabel *pComponentLabel,CglCanvas *pCanvas);
extern void ComponentLabel_Draw(TComponentLabel *pComponentLabel);
extern s32 ComponentLabel_GetWidth(TComponentLabel *pComponentLabel);
extern s32 ComponentLabel_GetIndexFromPos(TComponentLabel *pComponentLabel,s32 mx,s32 my);
extern bool ComponentLabel_MouseUp(TComponentLabel *pComponentLabel,s32 mx,s32 my);

typedef struct {
  void (*CallBack_Click)(void *pComponentCheck);
  u32 UserData;
  CglCanvas *pCanvas;
  CglTGF *pOnIcon,*pOffIcon;
  bool Checked;
  TRect Rect;
  bool Visible;
  const char *pMsgUTF8;
  const UnicodeChar *pMsgUnicode;
  const char *pInfoUTF8;
  const UnicodeChar *pInfoUnicode;
  u16 TextColor;
} TComponentCheck;

extern void ComponentCheck_Init(TComponentCheck *pComponentCheck,CglCanvas *pCanvas);
extern void ComponentCheck_Draw(TComponentCheck *pComponentCheck);
extern s32 ComponentCheck_GetWidth(TComponentCheck *pComponentCheck);
extern bool ComponentCheck_MouseDown(TComponentCheck *pComponentCheck,s32 mx,s32 my);
extern bool ComponentCheck_MouseUp(TComponentCheck *pComponentCheck,s32 mx,s32 my);

typedef struct {
  CglCanvas *pCanvas;
  CglTGF *pIcon;
  bool DrawFrame;
  bool Pressing;
  TRect Rect;
  const char *pMsgUTF8;
  const UnicodeChar *pMsgUnicode;
  u16 NormalTextColor,PressTextColor;
  bool Visible;
  void (*CallBack_Click)(void *pComponentButton);
} TComponentButton;

extern void ComponentButton_Init(TComponentButton *pComponentButton,CglCanvas *pCanvas);
extern void ComponentButton_Draw(TComponentButton *pComponentButton);
extern s32 ComponentButton_GetIndexFromPos(TComponentButton *pComponentButton,s32 mx,s32 my);
extern bool ComponentButton_MouseUp(TComponentButton *pComponentButton,s32 mx,s32 my);

#endif

