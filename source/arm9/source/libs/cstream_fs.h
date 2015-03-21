
#ifndef cstream_fs_h
#define cstream_fs_h

#include <stdlib.h>
#include <nds.h>

#include "cstream.h"
#include "fat2.h"

class CStreamFS: public CStream
{
  FAT_FILE *file;
  CStreamFS(const CStreamFS&);
  CStreamFS& operator=(const CStreamFS&);
public:
  CStreamFS(FAT_FILE *_file);
  virtual ~CStreamFS(void);
  virtual int GetOffset(void) const;
  virtual void SetOffset(int _ofs);
  virtual int GetSize(void) const;
  virtual void OverrideSize(int _size);
  virtual bool eof(void) const;
  virtual u8 Readu8(void);
  virtual u16 Readu16(void);
  virtual u32 Readu32(void);
  virtual int ReadBuffer(void *_dstbuf,const int _readsize);
  // fast request 16bit aligned file position and write buffer
  virtual int ReadBuffer16bit(void *_dstbuf,const int _readsize);
  virtual int ReadBuffer32bit(void *_dstbuf,const int _readsize);
  virtual void ReadSkip(const int _readsize);
};

#endif

