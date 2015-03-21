
#ifndef lang_h
#define lang_h

extern void Lang_Load(void);
extern void Lang_Free(void);

extern const char* Lang_GetUTF8_internal(const char *pItemName,const char *pErrorMsg);

#define Lang_GetUTF8(name) Lang_GetUTF8_internal(name,"LangErr:" name)

#endif
