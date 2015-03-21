
#ifndef NDSFiles_h
#define NDSFiles_h

#include "NDSROMIcon.h"

enum ENDSFile_FileType {ENFFT_UnknownFile,ENFFT_UpFolder,ENFFT_Folder,ENFFT_Sound,ENFFT_PlayList,ENFFT_Image,ENFFT_Text,ENFFT_Binary,ENFFT_Video,ENFFT_NDSROM,ENFFT_GBAROM,ENFFT_Skin};

typedef struct {
  ENDSFile_FileType FileType;
  char *pFilenameAlias;
  u32 Ext32;
  u32 FileSize;
  UnicodeChar *pFilenameUnicode;
  UnicodeChar *pFilenameUnicode_DoubleLine0,*pFilenameUnicode_DoubleLine1;
  UnicodeChar *pJpnTitle ;
  UnicodeChar *pEngTitle ;
  char *pFileInfo;
  bool UseDoubleLine;
  bool isNDSCommercialROM;
  u32 FileInfoWidth;
  TNDSROMIcon *pNDSROMIcon;
  CglTGF *pIcon;
} TNDSFile;

extern bool ChangedCurrentPath;

extern bool NDSIconInfoLoaded;

extern void NDSFiles_Free(void);
extern void NDSFiles_RefreshCurrentFolder(void);
extern void NDSFiles_LoadNDSIcon(TNDSFile *pNDSFile);
extern void NDSFiles_LoadFileInfo(TNDSFile *pNDSFile);

extern u32 NDSFiles_GetFilesCount(void);
extern TNDSFile* NDSFiles_GetFileBody(u32 idx);

#endif

