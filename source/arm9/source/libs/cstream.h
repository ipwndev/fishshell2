
#ifndef cstream_h
#define cstream_h

#include <stdlib.h>
#include <nds.h>

class CStream
{
  const u8 *buf;
  CStream(const CStream&);
  CStream& operator=(const CStream&);
protected:
  int size;
  int ofs;
public:
  CStream(const u8 *_buf,const int _size);
  virtual ~CStream(void);
  virtual int GetOffset(void) const;
  virtual void SetOffset(int _ofs);
  virtual int GetSize(void) const;
  virtual void OverrideSize(int _size);
  virtual bool eof(void) const;
  virtual u8 Readu8(void);
  virtual u16 Readu16(void);
  virtual u32 Readu32(void);
  virtual int ReadBuffer(void *_dstbuf,const int _readsize);
  virtual int ReadBuffer16bit(void *_dstbuf,const int _readsize);
  virtual int ReadBuffer32bit(void *_dstbuf,const int _readsize);
  virtual void ReadSkip(const int _readsize);
};

#endif

