
#ifndef cglfont_h
#define cglfont_h

#include <stdlib.h>
#include <nds.h>

#include "glmemtool.h"
#include "cglcanvas.h"
#include "tglunicode.h"

#define glFontHeight (12)

class CglFont
{
  TMM *pMM;
  u16 **DataTable;
  u16 *Data;
  u16 TextColor;
  CglFont(const CglFont&);
  CglFont& operator=(const CglFont&);
  u16* GetBulkData(const TglUnicode uidx) const;
public:
  CglFont(TMM *_pMM,const u8 *_buf,const int _size);
  ~CglFont(void);
  void DrawFont(CglCanvas *pCanvas,const int x,const int y,const TglUnicode uidx) const;
  int GetFontWidth(const TglUnicode uidx) const;
  void SetTextColor(const u16 Color);
  bool isExists(const TglUnicode uidx) const;
};

#endif

