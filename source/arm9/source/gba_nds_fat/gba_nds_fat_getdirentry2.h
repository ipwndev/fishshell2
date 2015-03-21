          bool NTF_lowfn=false,NTF_lowext=false;
          if((dir.reserved&BIT(3))!=0) NTF_lowfn=true;
          if((dir.reserved&BIT(4))!=0) NTF_lowext=true;
          
          if((NTF_lowfn==false)&&(NTF_lowext==false)){
            u32 idx;
            for(idx=0;idx<MAX_FILENAME_LENGTH;idx++){
              char fc=alias[idx];
              lfnNameUnicode[idx]=fc;
              if(fc==0) break;
            }
            }else{
            u32 posperiod=(u32)-1;
            {
              u32 idx;
              for(idx=0;idx<MAX_FILENAME_LENGTH;idx++){
                char fc=alias[idx];
                if(fc=='.') posperiod=idx;
                if(fc==0) break;
              }
            }
            if(posperiod==(u32)-1){
              u32 idx;
              for(idx=0;idx<MAX_FILENAME_LENGTH;idx++){
                char fc=alias[idx];
                if(NTF_lowfn==true){
                  if(('A'<=fc)&&(fc<='Z')) fc+=0x20;
                }
                lfnNameUnicode[idx]=fc;
                if(fc==0) break;
              }
              }else{
              u32 idx;
              for(idx=0;idx<MAX_FILENAME_LENGTH;idx++){
                char fc=alias[idx];
                if(NTF_lowfn==true){
                  if(('A'<=fc)&&(fc<='Z')) fc+=0x20;
                }
                lfnNameUnicode[idx]=fc;
                if(fc=='.') break;
              }
              for(;idx<MAX_FILENAME_LENGTH;idx++){
                char fc=alias[idx];
                if(NTF_lowext==true){
                  if(('A'<=fc)&&(fc<='Z')) fc+=0x20;
                }
                lfnNameUnicode[idx]=fc;
                if(fc==0) break;
              }
            }
          }
