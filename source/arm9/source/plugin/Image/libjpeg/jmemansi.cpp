/*
 * jmemansi.c
 *
 * Copyright (C) 1992-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides a simple generic implementation of the system-
 * dependent portion of the JPEG memory manager.  This implementation
 * assumes that you have the ANSI-standard library routine tmpfile().
 * Also, the problem of determining the amount of memory available
 * is shoved onto the user.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"		/* import the system-dependent declarations */

#ifndef HAVE_STDLIB_H		/* <stdlib.h> should declare malloc(),free() */
extern void * malloc JPP((size_t size));
extern void free JPP((void *ptr));
#endif

#ifndef SEEK_SET		/* pre-ANSI systems may not define this; */
#define SEEK_SET  0		/* if not, assume 0 is correct */
#endif

/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */

GLOBAL(void *)
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
  void *p=safemalloc(&MM_DLLImage,sizeofobject+(512*1024));
  if(p!=NULL) p=saferealloc(&MM_DLLImage,p,sizeofobject);
  return (void FAR *) p;
}

GLOBAL(void)
jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{//_consolePrintf("AllocS(%d),",sizeofobject);
  safefree(&MM_DLLImage,object);
}


/*
 * "Large" objects are treated the same as "small" ones.
 * NB: although we include FAR keywords in the routine declarations,
 * this file won't actually work in 80x86 small/medium model; at least,
 * you probably won't be able to process useful-size images in only 64KB.
 */

GLOBAL(void FAR *)
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{//_consolePrintf("AllocL(%d),",sizeofobject);
  void *p=safemalloc(&MM_DLLImage,sizeofobject+(512*1024));
  if(p!=NULL) p=saferealloc(&MM_DLLImage,p,sizeofobject);
  return (void FAR *) p;
}

GLOBAL(void)
jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
  safefree(&MM_DLLImage,object);
}


/*
 * This routine computes the total memory space available for allocation.
 * It's impossible to do this in a portable way; our current solution is
 * to make the user tell us (with a default value set at compile time).
 * If you can actually get the available space, it's a good idea to subtract
 * a slop factor of 5% or so.
 */

#ifndef DEFAULT_MAX_MEM		/* so can override from makefile */
#define DEFAULT_MAX_MEM		1000000L /* default: one megabyte */
#endif

GLOBAL(long)
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
		    long max_bytes_needed, long already_allocated)
{
  return cinfo->mem->max_memory_to_use - already_allocated;
}


/*
 * Backing store (temporary file) management.
 * Backing store objects are only used when the value returned by
 * jpeg_mem_available is less than the total space needed.  You can dispense
 * with these routines if you have plenty of virtual memory; see jmemnobs.c.
 */

extern int DFS_PrgJpeg_fopen(u32 RequestSize);
extern void DFS_PrgJpeg_fclose(u32 FileHandle);
extern void DFS_PrgJpeg_fread(u32 FileHandle,u32 pos,u8 *pbuf,u32 size);
extern void DFS_PrgJpeg_fwrite(u32 FileHandle,u32 pos,u8 *pbuf,u32 size);

METHODDEF(void)
read_backing_store (j_common_ptr cinfo, backing_store_ptr info,
		    void FAR * buffer_address,
		    long file_offset, long byte_count)
{
  _consolePrint("R");
  DFS_PrgJpeg_fread(info->temp_file,file_offset,(u8*)buffer_address,byte_count);
}


METHODDEF(void)
write_backing_store (j_common_ptr cinfo, backing_store_ptr info,
		     void FAR * buffer_address,
		     long file_offset, long byte_count)
{
  _consolePrint("W");
  DFS_PrgJpeg_fwrite(info->temp_file,file_offset,(u8*)buffer_address,byte_count);
}


METHODDEF(void)
close_backing_store (j_common_ptr cinfo, backing_store_ptr info)
{
  _consolePrintf("jpeg_close_backing_store: fh=%d\n",info->temp_file);
  DFS_PrgJpeg_fclose(info->temp_file);
  /* Since this implementation uses tmpfile() to create the file,
   * no explicit file deletion is needed.
   */
}


/*
 * Initial opening of a backing-store object.
 *
 * This version uses tmpfile(), which constructs a suitable file name
 * behind the scenes.  We don't have to use info->temp_name[] at all;
 * indeed, we can't even find out the actual name of the temp file.
 */

GLOBAL(void)
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
			 long total_bytes_needed)
{
//  info->temp_file = NULL; return;
  
  info->temp_file=DFS_PrgJpeg_fopen(total_bytes_needed);
  info->read_backing_store = read_backing_store;
  info->write_backing_store = write_backing_store;
  info->close_backing_store = close_backing_store;
  
  _consolePrintf("jpeg_open_backing_store: fh=%d, %dbyte.\n",info->temp_file,total_bytes_needed);
}


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.
 */

GLOBAL(long)
jpeg_mem_init (j_common_ptr cinfo)
{
  u32 maxsize=GetMaxMemoryBlockSize();
  _consolePrintf("jpeg_mem_init: maxsize= %dbyte.\n",maxsize);
  maxsize-=512*1024;
  return maxsize;	/* default for max_memory_to_use */
}

GLOBAL(void)
jpeg_mem_term (j_common_ptr cinfo)
{
  /* no work */
}

