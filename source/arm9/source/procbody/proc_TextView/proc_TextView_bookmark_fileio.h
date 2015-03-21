
#define BookmarkVersion (5)

typedef struct {
    u32 LineNum;
    u32 FileOffset;
    TDateTime DateTime;
} TBookmarkItem;

#define BookmarkItemCount (4)

typedef struct {
    u32 Version;
    char Header[32];
    u32 ResumeLineNum;
    u32 ResumeFileOffset;
    u32 CurrentItemIndex;
    TBookmarkItem BookmarkItem[BookmarkItemCount];
    ETextEncode TextEncode;
    u8 dummy[512];
} TBookmark;

extern DISC_INTERFACE* active_interface;

DATA_IN_IWRAM_TextView static u32 BookmarkDataSectorIndex=0;

DATA_IN_IWRAM_TextView static TBookmark *pBookmark;

DATA_IN_IWRAM_TextView static bool Bookmark_Enabled;

static void Bookmark_Clear(void)
{
    MemSet32CPU(0,pBookmark,sizeof(TBookmark));
  
    pBookmark->Version=BookmarkVersion;
    StrCopy("FishShell2 bookmark file.",pBookmark->Header);
}

static void Bookmark_ChangeExtBKM(UnicodeChar *pfnu)
{
    u32 pos=0;
  
    u32 idx=0;
    while(pfnu[idx]!=0){
        if(pfnu[idx]=='.') pos=idx+1;
        idx++;
    }
  
    if(pos==0) StopFatalError(18205,"Bookmark_ChangeExtBKM: Not found extention.\n");
  
    pfnu[pos+0]='b';
    pfnu[pos+1]='k';
    pfnu[pos+2]='m';
    pfnu[pos+3]=0;
}

static bool Bookmark_LoadFromFile(void)
{
    BookmarkDataSectorIndex=0;
    
    bool res=false;
  
    Bookmark_Clear();
  
    UnicodeChar fnu[MaxFilenameLength];
    Unicode_Copy(fnu,RelationalFileNameUnicode);
    Bookmark_ChangeExtBKM(fnu);
  
    if(FileExistsUnicode(RelationalFilePathUnicode,fnu)==true){
        FAT_FILE *pf=FAT2_fopen_AliasForRead(ConvertFull_Unicode2Alias(RelationalFilePathUnicode,fnu));
    
        if(FAT2_GetFileSize(pf)==512){
            if(pf->firstCluster!=0){
                BookmarkDataSectorIndex=FAT2_ClustToSect(pf->firstCluster);
            }
        }
        FAT2_fclose(pf);
        
        _consolePrintf("BookmarkDataSectorIndex=%d.\n",BookmarkDataSectorIndex);
      
        if(BookmarkDataSectorIndex!=0) active_interface->readSectors(BookmarkDataSectorIndex,1,pBookmark);
      
        if(pBookmark->Version!=BookmarkVersion) {
            _consolePrint("Bookmark_LoadFromFile: Illigal version detected. Re-initialize.\n");
            Bookmark_Clear();
        }else{
            res=true;
        }
    }
  
    return(res);
}

static void Bookmark_SaveToFile(void)
{
    UnicodeChar fnu[MaxFilenameLength];
    Unicode_Copy(fnu,RelationalFileNameUnicode);
    Bookmark_ChangeExtBKM(fnu);
  
    if(BookmarkDataSectorIndex!=0){
        REG_IME=0;
        active_interface->writeSectors(BookmarkDataSectorIndex,1,pBookmark);
        REG_IME=1;
        return;
    }
  
    const char *pafn=Shell_CreateNewFileUnicode(RelationalFilePathUnicode,fnu);
    _consolePrintf("Book mark file created. [%s]\n",pafn);
  
    FAT_FILE *pf=FAT2_fopen_AliasForWrite(ConvertFull_Unicode2Alias(RelationalFilePathUnicode,fnu));
    FAT2_fwrite(pBookmark,512,1,pf);
    FAT2_fclose(pf);
    
    pf=FAT2_fopen_AliasForRead(ConvertFull_Unicode2Alias(RelationalFilePathUnicode,fnu));
        
    if(FAT2_GetFileSize(pf)==512){
        if(pf->firstCluster!=0){
            BookmarkDataSectorIndex=FAT2_ClustToSect(pf->firstCluster);
        }
    }
    FAT2_fclose(pf);
            
    _consolePrintf("BookmarkDataSectorIndex=%d.\n",BookmarkDataSectorIndex);
            
}

static void Bookmark_Init(void)
{
    _consolePrint("Bookmark initialize.\n");
  
    Bookmark_Enabled=false;
  
    pBookmark=(TBookmark*)safemalloc_chkmem(&MM_Process,sizeof(TBookmark));
  
    if(Bookmark_LoadFromFile()==true){
        _consolePrint("Bookmark file loaded.\n");
        Bookmark_Enabled=true;
    }
  
    _consolePrint("Bookmark initialized.\n");
}

static void Bookmark_Free(void)
{
    if(pBookmark!=NULL){
        safefree(&MM_Process,pBookmark); pBookmark=NULL;
    }
  
    BookmarkDataSectorIndex=0;
  
    Bookmark_Enabled=false;
}

static void Bookmark_Save(u32 itemidx,u32 linenum,u32 fileofs)
{
    TBookmarkItem *pbmi=&pBookmark->BookmarkItem[itemidx];
  
    DateTime_ResetNow();
    pbmi->DateTime=DateTime_GetNow();
    pbmi->LineNum=linenum;
    pbmi->FileOffset=fileofs;
  
    Bookmark_SaveToFile();
  
    Bookmark_Enabled=true;
}

static TBookmarkItem* Bookmark_Load(u32 itemidx)
{
    return(&pBookmark->BookmarkItem[itemidx]);
}

static void Bookmark_SetTextEncode(ETextEncode TextEncode)
{
    pBookmark->TextEncode=TextEncode;
    Bookmark_SaveToFile();
}

static ETextEncode Bookmark_GetTextEncode(void)
{
    return(pBookmark->TextEncode);
}

static void Bookmark_SetResumeLineNum(u32 linenum,u32 fileofs)
{
    pBookmark->ResumeLineNum=linenum;
    pBookmark->ResumeFileOffset=fileofs;
    if(Bookmark_Enabled==true) Bookmark_SaveToFile();
}

static u32 Bookmark_GetResumeLineNum(void)
{
    return(pBookmark->ResumeLineNum);
}

static u32 Bookmark_GetResumeFileOffset(void)
{
    return(pBookmark->ResumeFileOffset);
}

static u32 Bookmark_GetCurrentItemIndex(void)
{
    return(pBookmark->CurrentItemIndex);
}

static void Bookmark_SetCurrentItemIndex(u32 itemidx)
{
    pBookmark->CurrentItemIndex=itemidx;
    if(Bookmark_Enabled==true) Bookmark_SaveToFile();
}
