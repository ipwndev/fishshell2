
#ifndef datetime_h
#define datetime_h

#include "fat2.h"

typedef struct {
  int Year,Month,Day;
} TDate;

typedef struct {
  int Hour,Min,Sec;
  const char *pAMPMStr;
  int Hour12h;
} TTime;

typedef struct {
  TDate Date;
  TTime Time;
} TDateTime;

typedef struct {
  s32 Days;
  s32 Secs;
} TDateTimeSub;

#define Calender_LineCount (6)
#define Calender_DaysMapCount (Calender_LineCount*7)

typedef struct {
  u32 Year,Month;
  u32 StartWeek;
  u32 DaysMap[Calender_DaysMapCount];
  u32 DaysofMonth;
} TCalendarData;

extern u32 Calendar_CurrentYear,Calendar_CurrentMonth;

void DateTime_Make12hAMPM(TDateTime *pdt);
void DateTime_ResetNow(void);
TDateTime DateTime_GetNow(void);

u32 Date_GetDaysofMonth(s32 Year,s32 Month);

TCalendarData DateTime_CreateCalendarData(u32 Year,u32 Month);

TDate Date_NextDay(TDate date);

s32 DateTime_Compare(TDateTime *pdt1,TDateTime *pdt2); // (res==-1)=(dt1<dt2) (res==0)=(dt1==dt2) (res==1)=(dt2<dt1)

u32 Date_GetWeekNum(TDate date);
void Date_GetWeekStr(char *pstr,u32 len,const TDate date);

TDateTimeSub DateTime_Sub(TDateTime dt1,TDateTime dt2);

enum EDateFormat {EDF_YMD,EDF_DMY,EDF_MDY};

void Date_SetDateFormat(const EDateFormat df);

void Date_GetDateStrBuf(char *pstr,u32 len,const TDate date);
const char* Date_GetDateStr(const TDate date);
const char* Date_GetDateStr_FAT2_TIME(TFAT2_TIME *pt);

void Date_Set24hFormat(bool f);
void Date_GetTimeStrBuf_12h(char *pstr,u32 len,const TTime time);

void Date_GetTimeStrBuf_12h_HHMM(char *pstr,u32 len,const TTime time);
void Date_GetTimeStrBuf_12h_SS(char *pstr,u32 len,const TTime time);
void Date_GetTimeStrBuf_12h_AP(char *pstr,u32 len,const TTime time);
void Date_GetTimeStrBuf_12h_SSAP(char *pstr,u32 len,const TTime time);

#endif

