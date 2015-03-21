
#define KeyRepeat_DelayCount (40)
#define KeyRepeat_RateCount (10)

DATA_IN_AfterSystem static u32 KeyRepeatLastKey;
DATA_IN_AfterSystem static bool KeyRepeatFlag;
DATA_IN_AfterSystem static u32 KeyRepeatCount;

CODE_IN_AfterSystem static void KeyRepeat_Flash(void)
{
  KeyRepeatLastKey=0;
  KeyRepeatFlag=false;
}

CODE_IN_AfterSystem static u32 KeyRepeat_On(u32 NowKey)
{
  if(NowKey!=KeyRepeatLastKey) KeyRepeatFlag=false;
  KeyRepeatLastKey=NowKey;
  
  if(KeyRepeatFlag==false){ cwl();
    KeyRepeatFlag=true;
    KeyRepeatCount=KeyRepeat_DelayCount;
    }else{ cwl();
    if(KeyRepeatCount==0){ cwl();
      KeyRepeatCount=KeyRepeat_RateCount;
      }else{ cwl();
      NowKey=0;
    }
  }
  
  return(NowKey);
}

CODE_IN_AfterSystem static u32 KeyRepeat_Proc(u32 NowKey,u32 VsyncCount)
{
  if(KeyRepeatFlag==true){ cwl();
    if(KeyRepeatCount<=VsyncCount){ cwl();
      KeyRepeatCount=0;
      }else{ cwl();
      KeyRepeatCount-=VsyncCount;
    }
  }
  
  if(NowKey==0){ cwl();
    KeyRepeat_Flash();
    }else{ cwl();
    NowKey=KeyRepeat_On(NowKey);
  }
  
  return(NowKey);
}

CODE_IN_AfterSystem static void KeyRepeat_Delay(u32 Multiple)
{
  KeyRepeatCount=KeyRepeat_DelayCount*Multiple;
}

