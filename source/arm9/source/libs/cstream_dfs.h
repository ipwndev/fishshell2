#ifndef CSTREAM_DFS_H_
#define CSTREAM_DFS_H_

#include <stdio.h>
#include <nds.h>

#define SectorSize (512)

class CStreamDFS {
	CStreamDFS(const CStreamDFS&);
	CStreamDFS& operator=(const CStreamDFS&);
public:
	CStreamDFS(TMM *pMM,FILE *fp);
	~CStreamDFS(void);
	
	u32 GetSize(void) const;
	u32 GetOffset(void) const;
	void SetOffset(u32 ofs);
	u32 Read16bit(void *pbuf,u32 size);
	u32 Read32bit(void *pbuf,u32 size);
	
	void SetAttribute(u32 TopOffset,u32 Size);
	
	void SeekSectorCount(u32 Sectors);
	void WriteSectors(void *pbuf,u32 Sectors);
	void ReadSectors(void *pbuf,u32 Sectors);
protected:
	typedef struct {
	  u32 Sector;
	  u32 Count;
	} TBurstList;
	
	TMM *pMM;
	
	u32 TopOffset;
	u32 Size;
	u32 Offset;
	u8 SectorBuffer[SectorSize];
	u32 SectorRemainByte;
	
	TBurstList *pBurstList;
	u32 BurstListIndex;
	u32 BurstListCount;
	u32 BurstListCurSector;
	u32 BurstListRemainSectorCount;
	
	void MoveTop(void);
	void ReadSkip(u32 size);
};

#endif /*CSTREAM_DFS_H_*/
