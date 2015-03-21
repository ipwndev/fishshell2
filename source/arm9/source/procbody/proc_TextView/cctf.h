
#ifndef cctf_h
#define cctf_h

#include "procstate.h"

// Clear type font lib.

// ダイレクトディスクアクセスがクラス独立ではないので、ctfファイルを2つ以上同時に開かないこと。

#include "unicode.h"

#include "glib.h"

#include "fat2.h"

#define CTF_FontPadding (1)

enum ECTF_DisplayFormat {ECTFDF_RGB,ECTFDF_BGR};

#define CTF_FontCacheMax (1024)

#define CTF_FontDataMaxSizeByte (272) // 最大サイズはフォントファイル作成時にチェックすること

typedef struct {
    u16 uidxs[CTF_FontCacheMax];
    u16 datas[CTF_FontCacheMax][CTF_FontDataMaxSizeByte/2];
} TCTF_FontCache;

class CCTF
{
    CglCanvas *pcan;
  
    ECTF_DisplayFormat DisplayFormat;
  
    u32 iaval[7];
    u32 coltbl9[8*8*8]; // 本当はDTCMに入れたい
  
    u32 *pIndexTable;
    u8 *pWidthsTable;
  
    u32 FontHeight;
  
    TCTF_FontCache FontCache;
    u32 FontCacheNextWriteIndex;
  
    CCTF(const CCTF&);
    CCTF& operator=(const CCTF&);
    void make_coltbl9(void);
    void DrawFont_TextBlack(CglCanvas *pcan,int x,int y,UnicodeChar wch);
    void DrawFont_Fast(CglCanvas *pcan,int x,int y,UnicodeChar wch);
    void DrawFont_Fast2(CglCanvas *pcan,int x,int y,UnicodeChar wch);
    u32 GetCharOffset(UnicodeChar wch) const;
    u32 GetCharDataSize(UnicodeChar wch) const;
    u32 GetCharWidth(UnicodeChar wch) const;
    const u16* GetFontData(UnicodeChar wch);
protected:
public:
    CCTF(FAT_FILE *pFileHandle,u32 _FontHeight,ETextClearTypeFont ClearTypeFont);
    ~CCTF(void);
    void SetTargetCanvas(CglCanvas *_pcan);
    void GetWidthsList(u8 *pWidthsList) const;
    void TextOutA(const int x,const int y,const char *str);
    void TextOutW(const int x,const int y,const UnicodeChar *str);
    void TextOutUTF8(const int x,const int y,const char *str);
    int GetTextWidthA(const char *str) const;
    int GetTextWidthW(const UnicodeChar *str) const;
    int GetTextWidthUTF8(const char *str) const;
};

#endif

