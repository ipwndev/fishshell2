
DATA_IN_AfterSystem static u16 *pCP1252Tbl,*pCP437Tbl,*pCP850Tbl;

static void CP2Unicode_Init(void)
{
  pCP1252Tbl=NULL;
  pCP437Tbl=NULL;
  pCP850Tbl=NULL;
}

static void CP2Unicode_Free(void)
{
  if(pCP1252Tbl!=NULL){
    safefree(&MM_Temp,pCP1252Tbl); pCP1252Tbl=NULL;
  }
  if(pCP437Tbl!=NULL){
    safefree(&MM_Temp,pCP437Tbl); pCP437Tbl=NULL;
  }
  if(pCP850Tbl!=NULL){
    safefree(&MM_Temp,pCP850Tbl); pCP850Tbl=NULL;
  }
  
  CP2Unicode_Init();
}

static void CP2Unicode_Load(void)
{
  {
    FAT_FILE *pf=Shell_FAT_fopen_VariableCodepageToUnicodeTable(1252);
    if(pf==NULL) StopFatalError(11401,"Not found CP1252 to Unicode convert table file.\n");
    pCP1252Tbl=(u16*)safemalloc_chkmem(&MM_Temp,256*2);
    FAT2_fread_fast(pCP1252Tbl,2,256,pf);
    FAT2_fclose(pf);
  }
  {
    FAT_FILE *pf=Shell_FAT_fopen_VariableCodepageToUnicodeTable(437);
    if(pf==NULL) StopFatalError(11402,"Not found CP437 to Unicode convert table file.\n");
    pCP437Tbl=(u16*)safemalloc_chkmem(&MM_Temp,256*2);
    FAT2_fread_fast(pCP437Tbl,2,256,pf);
    FAT2_fclose(pf);
  }
  {
    FAT_FILE *pf=Shell_FAT_fopen_VariableCodepageToUnicodeTable(850);
    if(pf==NULL) StopFatalError(11403,"Not found CP850 to Unicode convert table file.\n");
    pCP850Tbl=(u16*)safemalloc_chkmem(&MM_Temp,256*2);
    FAT2_fread_fast(pCP850Tbl,2,256,pf);
    FAT2_fclose(pf);
  }
}

static void CP12522Unicode_Convert(const char *pStrL,UnicodeChar *pStrW)
{
  while(*pStrL!=0){
    *pStrW++=pCP1252Tbl[*pStrL++];
  }
  *pStrW=0;
}

static void CP4372Unicode_Convert(const char *pStrL,UnicodeChar *pStrW)
{
  while(*pStrL!=0){
    *pStrW++=pCP437Tbl[*pStrL++];
  }
  *pStrW=0;
}

static void CP8502Unicode_Convert(const char *pStrL,UnicodeChar *pStrW)
{
  while(*pStrL!=0){
    *pStrW++=pCP850Tbl[*pStrL++];
  }
  *pStrW=0;
}

