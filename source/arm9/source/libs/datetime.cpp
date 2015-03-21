
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>

#include "_const.h"
#include "_console.h"
#include "datetime.h"
#include "lang.h"

#include "../../ipc6.h"

DATA_IN_AfterSystem u32 Calendar_CurrentYear,Calendar_CurrentMonth;

DATA_IN_AfterSystem static bool Now_RequestReget;
DATA_IN_AfterSystem static TDateTime Now_Cache;

void DateTime_Make12hAMPM(TDateTime *pdt)
{
  u32 Hour=pdt->Time.Hour;
  const char *pAMPMStr;
  u32 Hour12h;
  
  if(Hour==0){
    pAMPMStr="AM";
    Hour12h=12;
    }else{
    if(Hour==12){
      pAMPMStr="PM";
      Hour12h=12;
      }else{
      if(Hour<12){
        pAMPMStr="AM";
        Hour12h=Hour;
        }else{
        pAMPMStr="PM";
        Hour12h=Hour-12;
      }
    }
  }
  
  pdt->Time.pAMPMStr=pAMPMStr;
  pdt->Time.Hour12h=Hour12h;
}
void DateTime_ResetNow(void)
{
  Now_RequestReget=true;
  IPC6->curtimeFlag=true;
}

TDateTime DateTime_GetNow(void)
{
  if(Now_RequestReget==true){
    Now_RequestReget=false;
    
    while(IPC6->curtimeFlag==true){
//      swiWaitForVBlank();
    }
    
    TDateTime DateTime;
    
    DateTime.Date.Year=2000+IPC6->time.rtc.year;
    DateTime.Date.Month=IPC6->time.rtc.month;
    DateTime.Date.Day=IPC6->time.rtc.day;
    
    DateTime.Time.Hour=IPC6->time.rtc.hours;
    DateTime.Time.Min=IPC6->time.rtc.minutes;
    DateTime.Time.Sec=IPC6->time.rtc.seconds;
    
    DateTime_Make12hAMPM(&DateTime);
    
    Now_Cache=DateTime;
  }
  
  return(Now_Cache);
}

static u32 GetWeekNum(s32 Year,s32 Month,s32 Day)
{
  Month-=2;
  if(Month<=0){
    Year--;
    Month+=12;
  }
  
  s32 Week=(s32)(Year+(Year/4)-(Year/100)+(Year/400)+(2.6*Month-0.2)+Day);
  
  return((u32)(Week%7));
}

u32 Date_GetDaysofMonth(s32 Year,s32 Month)
{
  u32 DaysofMonth[12]={31,28,31,30,31,30,31,31,30,31,30,31};
  
  if(Month!=2) return(DaysofMonth[Month-1]);
  
  bool f=false;
  
  if((Year%4)==0) f=true;
  if((Year%100)==0) f=false;
  if((Year%400)==0) f=true;
  
  if(f==true){
    return(29);
    }else{
    return(28);
  }
}

TCalendarData DateTime_CreateCalendarData(u32 Year,u32 Month)
{
  TCalendarData cd;
  
  cd.Year=Year;
  cd.Month=Month;
  
  cd.DaysofMonth=Date_GetDaysofMonth(Year,Month);
  
  cd.StartWeek=GetWeekNum(Year,Month,1);
  
  for(u32 idx=0;idx<Calender_DaysMapCount;idx++){
    cd.DaysMap[idx]=0;
  }
  
  for(u32 idx=0;idx<cd.DaysofMonth;idx++){
    cd.DaysMap[cd.StartWeek+idx]=1+idx;
  }
  
  return(cd);
}

TDate Date_NextDay(TDate date)
{
  date.Day++;
  if(Date_GetDaysofMonth(date.Year,date.Month)<date.Day){
    date.Day=1;
    date.Month++;
    if(12<date.Month){
      date.Month=1;
      date.Year++;
    }
  }
  return(date);
}

s32 DateTime_Compare(TDateTime *pdt1,TDateTime *pdt2)
{
  // (res==-1)=(dt1<dt2) (res==0)=(dt1==dt2) (res==1)=(dt2<dt1)
  
  u32 day1=(pdt1->Date.Year*0x10000)+(pdt1->Date.Month*0x100)+pdt1->Date.Day;
  u32 day2=(pdt2->Date.Year*0x10000)+(pdt2->Date.Month*0x100)+pdt2->Date.Day;
  
  if(day1<day2) return(-1);
  if(day2<day1) return(1);
  
  u32 sec1=(pdt1->Time.Hour*0x10000)+(pdt1->Time.Min*0x100)+pdt1->Time.Sec;
  u32 sec2=(pdt2->Time.Hour*0x10000)+(pdt2->Time.Min*0x100)+pdt2->Time.Sec;
  
  if(sec1<sec2) return(-1);
  if(sec2<sec1) return(1);
  
  return(0);
}

u32 Date_GetWeekNum(TDate date)
{
  return(GetWeekNum(date.Year,date.Month,date.Day));
}

void Date_GetWeekStr(char *pstr,u32 len,const TDate date)
{
  snprintf(pstr,len,"%d\0",Date_GetWeekNum(date));
}

TDateTimeSub DateTime_Sub(TDateTime dt1,TDateTime dt2)
{
  TDateTimeSub dts={0,0};
  
  u32 dt1daycalc=(dt1.Date.Year*0x100*0x100)|(dt1.Date.Month*0x100)|dt1.Date.Day;
  
  while(1){
    u32 dt2daycalc=(dt2.Date.Year*0x100*0x100)|(dt2.Date.Month*0x100)|dt2.Date.Day;
    if(dt1daycalc<dt2daycalc) break;
    dts.Days++;
    dt2.Date=Date_NextDay(dt2.Date);
  }
  dts.Days--;
  
  s32 secs1=(dt1.Time.Hour*60*60)+(dt1.Time.Min*60)+dt1.Time.Sec;
  s32 secs2=(dt2.Time.Hour*60*60)+(dt2.Time.Min*60)+dt2.Time.Sec;
  
  if(secs1<secs2){
    dts.Days--;
    secs1+=24*60*60;
  }
  
  dts.Secs=secs1-secs2;
  
  if(dts.Days<0){
    dts.Days=0;
    dts.Secs=-1;
  }
  
  return(dts);
}

DATA_IN_AfterSystem static EDateFormat DateFormat=EDF_YMD;

void Date_SetDateFormat(const EDateFormat df)
{
  DateFormat=df;
}

void Date_GetDateStrBuf(char *pstr,u32 len,const TDate date)
{
  switch(DateFormat){
    case EDF_YMD: snprintf(pstr,len,"%d/%d/%d\0",date.Year,date.Month,date.Day); break;
    case EDF_DMY: snprintf(pstr,len,"%d/%d/%d\0",date.Day,date.Month,date.Year); break;
    case EDF_MDY: snprintf(pstr,len,"%d/%d/%d\0",date.Month,date.Day,date.Year); break;
    default: snprintf(pstr,len,"%d/%d/%d\0",date.Year,date.Month,date.Day); break;
  }
}

const char* Date_GetDateStr(const TDate date)
{
	DATA_IN_AfterSystem static char str[64];
  
  Date_GetDateStrBuf(str,64,date);
  
  return(str);
}

const char* Date_GetDateStr_FAT2_TIME(TFAT2_TIME *pt)
{
  TDate date;
  date.Year=pt->Year;
  date.Month=pt->Month;
  date.Day=pt->Day;
  
  DATA_IN_AfterSystem static char str[64];
  
  Date_GetDateStrBuf(str,64,date);
  
  return(str);
}

DATA_IN_AfterSystem static bool Use24hFormat=false;
void Date_Set24hFormat(bool f)
{
  Use24hFormat=f;
}

void Date_GetTimeStrBuf_12h(char *pstr,u32 len,const TTime time)
{
  if(Use24hFormat==true){
    snprintf(pstr,len,"%2d:%02d:%02d\0",time.Hour,time.Min,time.Sec);
    return;
  }
  
  snprintf(pstr,len,"%s%2d:%02d:%02d\0",time.pAMPMStr,time.Hour12h,time.Min,time.Sec);
}

void Date_GetTimeStrBuf_12h_HHMM(char *pstr,u32 len,const TTime time)
{
  if(Use24hFormat==true){
    snprintf(pstr,len,"%d:%02d\0",time.Hour,time.Min);
    return;
  }
  
  snprintf(pstr,len,"%d:%02d\0",time.Hour12h,time.Min);
}

void Date_GetTimeStrBuf_12h_SS(char *pstr,u32 len,const TTime time)
{
  snprintf(pstr,len,"%02d\0",time.Sec);
}

void Date_GetTimeStrBuf_12h_AP(char *pstr,u32 len,const TTime time)
{
  if(Use24hFormat==true){
    snprintf(pstr,len,"\0");
    return;
  }
  
  snprintf(pstr,len,"%s\0",time.pAMPMStr);
}

void Date_GetTimeStrBuf_12h_SSAP(char *pstr,u32 len,const TTime time)
{
  if(Use24hFormat==true){
    snprintf(pstr,len,"   :%02d\0",time.Sec);
    return;
  }
  
  snprintf(pstr,len,"%s :%02d\0",time.pAMPMStr,time.Sec);
}

