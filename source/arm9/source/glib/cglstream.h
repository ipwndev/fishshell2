
#ifndef cglstream_h
#define cglstream_h

#include <stdlib.h>
#include <nds.h>

class CglStream
{
  const u8 *buf;
  int size;
  int ofs;
  CglStream(const CglStream&);
  CglStream& operator=(const CglStream&);
public:
  CglStream(const u8 *_buf,const int _size);
  ~CglStream(void);
  int GetOffset(void) const;
  void SetOffset(int _ofs);
  int GetSize(void) const;
  bool eof(void) const;
  u8 Readu8(void);
  u16 Readu16(void);
  u32 Readu32(void);
  void ReadBuffer(void *_dstbuf,const int _readsize);
};

#endif

