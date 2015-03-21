
static char Shell_CreateNewFileUnicode_ins_uppercase(char ch)
{
  if(((u32)'a'<=ch)&&(ch<=(u32)'z')) ch-=0x20;
  return(ch);
}

const char* Shell_CreateNewFileUnicode(const UnicodeChar *pFilePathUnicode,const UnicodeChar *pFileNameUnicode)
{
  const char *ppathalias=ConvertFull_Unicode2Alias(pFilePathUnicode,NULL);
  if(FAT2_chdir_Alias(ppathalias)==false) StopFatalError(18203,"Can not change current path.\n");
  
  u32 extpos=0;
  {
    u32 idx=0;
    while(pFileNameUnicode[idx]!=0){
      if(pFileNameUnicode[idx]==(UnicodeChar)'.') extpos=idx+1;
      idx++;
    }
  }
  
  if(extpos==0) StopFatalError(18201,"Not found extention.\n");
  
  const u32 fnstrmaxlen=5;
  char fnstr[fnstrmaxlen+1];
  {
    u32 aidx=0,uidx=0;
    const UnicodeChar *pufn=pFileNameUnicode;
    while((aidx<fnstrmaxlen)&&(uidx<extpos)){
      UnicodeChar ch=pufn[uidx++];
      if((0x21<=ch)&&(ch<0x80)){ // 0x20 = space
        if((ch=='\"')||(ch=='*')||(ch=='/')||(ch==':')||(ch=='<')||(ch=='>')||(ch=='?')||(ch=='\\')||(ch=='|')||
           (ch=='.')||(ch=='+')||(ch==',')||(ch==';')||(ch=='=')||(ch=='[')||(ch==']')){
          }else{
          fnstr[aidx++]=Shell_CreateNewFileUnicode_ins_uppercase((char)ch);
        }
      }
    }
    fnstr[aidx]=0;
  }
  
  char extstr[4];
  extstr[0]=Shell_CreateNewFileUnicode_ins_uppercase((char)pFileNameUnicode[extpos+0]);
  extstr[1]=Shell_CreateNewFileUnicode_ins_uppercase((char)pFileNameUnicode[extpos+1]);
  extstr[2]=Shell_CreateNewFileUnicode_ins_uppercase((char)pFileNameUnicode[extpos+2]);
  extstr[3]=(char)0;
  
  DATA_IN_AfterSystem static char FilenameAlias[13];
  
  u32 digits=1;
  while(1){
    snprintf(FilenameAlias,13,"%s~%02d.%s",fnstr,digits,extstr);
    FAT_FILE *pf=FAT2_fopen_AliasForRead(FilenameAlias);
    if(pf==NULL) break;
    FAT2_fclose(pf);
    digits++;
    if(digits==100) StopFatalError(18202,"Alias entry overflow.\n");
  }
  
  FAT_FILE *pf=FAT2_fopen_CreateForWrite_on_CurrentFolder(FilenameAlias,pFileNameUnicode);
  for(u32 idx=0;idx<512/4;idx++){
    u32 dummy=0;
    FAT2_fwrite(&dummy,4,1,pf);
  }
  FAT2_fclose(pf);
  
  return(FilenameAlias);
}

