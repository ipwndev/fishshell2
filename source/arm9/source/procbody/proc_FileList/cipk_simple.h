
#ifndef cipk_simple_h
#define cipk_simple_h

#include "unicode.h"
#include "fat2.h"
#include "strtool.h"

#include "cstream_fs.h"

#define IPKID_Size (16)
#define IPKID_Data "PackedImages01\0x00\0x00"
#define IPKStateVersion_Size (4)
#define IPKStateBulk_Size (512-IPKID_Size-IPKStateVersion_Size)

#define IPKMCUWidth (64)
#define IPKMCUHeight (64)
#define IPKMCUSize (IPKMCUWidth*IPKMCUHeight)

enum EFileInfoExt {EFIE_None=0,EFIE_Exif=1};

enum EIPKThumbnailID {EIPKTHID_4864=0,EIPKTHID_6448=1,EIPKTHID_192256=2,EIPKTHID_256192=3,EIPKTHID_SKIP=4};
enum EIPKBodyFormat {EIPKBF_Beta15bit=0,EIPKBF_CustomJpegYUV111=1,EIPKBF_CustomJpegYUV411=2};

typedef struct {
  u32 Width,Height;
  u32 Ratio; // fixed fraction 16.16
  u16 *pBuf;
} TIPKThumbnail;

typedef struct {
  u32 Width,Height;
  u16 MCUXCount;
  u16 dummy;
  u16 MCUYCount;
  EIPKBodyFormat BodyFormat;
  s32 QuantizeTable[64];
} TIPKImageInfo;

class CIPK
{
  CIPK(const CIPK&);
  CIPK& operator=(const CIPK&);
protected:
  CStreamFS *prfs;
  u32 FilesCount;
  u32 *pBodyOffsets;
  u32 OffsetEXT0;
public:
  char *pFileInfoExtExif_Description;
  char *pFileInfoExtExif_DateTime;
  CIPK(FAT_FILE *pf);
  ~CIPK(void);
  
  u32 GetFilesCount(void);
  bool GetCoverImage(TIPKThumbnail *pth);
  u32 GetBodyOffset(u32 FileIndex);
  
  void GetFilename(u32 FileIndex,UnicodeChar *pstr);
  bool GetFileInfoExtExif(u32 FileIndex);
  bool GetThumbnail(u32 FileIndex,EIPKThumbnailID thID,TIPKThumbnail *pth);
  void GetImageInfo(u32 FileIndex,TIPKImageInfo *pimginfo);
};

static bool FileCheck_isIPK(FAT_FILE *pf)
{
  FAT2_fseek(pf,0,SEEK_SET);
  
  char ID[IPKID_Size];
  FAT2_fread(&ID[0],1,IPKID_Size,pf);
  if(isStrEqual(ID,IPKID_Data)==false) return(false);
  
  return(true);
}

#endif

