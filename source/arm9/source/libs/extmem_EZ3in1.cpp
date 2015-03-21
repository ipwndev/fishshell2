
#include <nds.h>
#include <_const.h>

// --- EZ3in1 stuff

DATA_IN_IWRAM_MainPass static void SetRompage(u16 page)
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9880000 = page;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

DATA_IN_IWRAM_MainPass static void		OpenNorWrite()
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9C40000 = 0x1500;
	*(vuint16 *)0x9fc0000 = 0x1500;
}


DATA_IN_IWRAM_MainPass static void		CloseNorWrite()
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9C40000 = 0xd200;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

#define FlashBase		0x08000000
#define NorFlashID (0x227E2218)

DATA_IN_IWRAM_MainPass static uint32 ReadNorFlashID()
{
	vuint16 id1,id2;//,id3,id4;
	*((vuint16 *)(FlashBase+0x555*2)) = 0xAA ;
	*((vuint16 *)(FlashBase+0x2AA*2)) = 0x55 ;
	*((vuint16 *)(FlashBase+0x555*2)) = 0x90 ;

	*((vuint16 *)(FlashBase+0x1555*2)) = 0xAA ;
	*((vuint16 *)(FlashBase+0x12AA*2)) = 0x55 ;
	*((vuint16 *)(FlashBase+0x1555*2)) = 0x90 ;

	id1 = *((vuint16 *)(FlashBase+0x2)) ;
	id2 = *((vuint16 *)(FlashBase+0x2002)) ;
	if( (id1!=0x227E)|| (id2!=0x227E))
		return 0;

	id1 = *((vuint16 *)(FlashBase+0xE*2)) ;
	id2 = *((vuint16 *)(FlashBase+0x100e*2)) ;
	if(id1==0x2218 && id2==0x2218)			//H6H6
		return 0x227E2218;
	return 0;
}

DATA_IN_IWRAM_MainPass static bool ExistsEZ3in1(void)
{
  u32 id;
  
  SetRompage(0);
  OpenNorWrite();
  id=ReadNorFlashID();
  CloseNorWrite();
  
  //if(id!=NorFlashID) return(false);
  
  return(true);
}

DATA_IN_IWRAM_MainPass static inline void SetEZ3in1_MemoryOpen(void)
{
	SetRompage(384);
	OpenNorWrite();
}

DATA_IN_IWRAM_MainPass static inline void SetEZ3in1_MemoryClose(void)
{
	CloseNorWrite();
	SetRompage(0);
}

DATA_IN_IWRAM_MainPass u32 extmem_EZ3in1_Start(void)
{
  //if(ExistsEZ3in1()==false) return(0);
  
  SetEZ3in1_MemoryOpen();
  //return(0x8000000+(1*1024*1024));
  return(0x8000000);
}

