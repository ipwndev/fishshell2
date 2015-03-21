
#ifndef BootROM_h
#define BootROM_h

extern void BootROM_Init(void);

extern bool BootROM_GetExecuteFlag(void);
extern const char* BootROM_GetFullPathAlias(void);
extern const UnicodeChar* BootROM_GetPathUnicode(void);
extern const UnicodeChar* BootROM_GetFilenameUnicode(void);

extern void BootROM_SetInfo_TextEdit(const UnicodeChar *pPathUnicode,const UnicodeChar *pFilenameUnicode,bool RequestBackupSave);
extern void BootROM_SetInfo_NoLaunch(const UnicodeChar *pPathUnicode,const UnicodeChar *pFilenameUnicode,bool RequestBackupSave);
extern void BootROM_SetInfo(const UnicodeChar *pPathUnicode,const UnicodeChar *pFilenameUnicode,bool RequestBackupSave);

extern bool BootROM_isExistsSoftResetToFirmware(void);

extern void BootROM_SoftResetToFirmware(void);

extern bool BootROM_CheckNDSHomeBrew(const char *pFilename);
extern void BootROM_Execute(void);

#endif

