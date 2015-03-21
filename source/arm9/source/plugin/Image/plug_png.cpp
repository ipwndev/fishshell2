
#include <stdio.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"

#include "memtool.h"

#include "internaldrivers.h"

#include "ErrorDialog.h"

#include "fat2.h"
#include "shell.h"

#include "libpng/png.h"

#ifndef ExceptPng

bool PlugPng_Start(FAT_FILE *_FileHandle)
{
  return(false);
}

void PlugPng_Free(void)
{
}

void PlugPng_GetBitmap24(u32 LineY,u8 *pBM)
{
}

s32 PlugPng_GetWidth(void)
{
  return(0);
}

s32 PlugPng_GetHeight(void)
{
  return(0);
}

int PlugPng_GetInfoIndexCount(void)
{
  return(0);
}

bool PlugPng_GetInfoStrA(int idx,char *str,int len)
{
  return(false);
}

bool PlugPng_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugPng_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else // #ifdef ExceptPng

// ------------------------------------------------------------------------------------


#define PNG_DEBUG 1

#undef png_debug
#undef png_debug1
#undef png_debug2

#ifdef PNG_DEBUG
#define png_debug(l,m) _consolePrint(m)
#define png_debug1(l,m,p1) _consolePrintf(m,p1)
#define png_debug2(l,m,p1,p2) _consolePrintf(m,p1,p2)
#else
#define png_debug(l,m) 
#define png_debug1(l,m,p1) 
#define png_debug2(l,m,p1,p2) 
#endif
  
typedef unsigned long b8;
typedef unsigned long b16;
typedef unsigned long b32;
typedef volatile unsigned long _b8;
typedef volatile unsigned long _b16;
typedef volatile unsigned long _b32;

static FAT_FILE *FileHandle;

FAT_FILE * _fopen(FAT_FILE * _FileHandle)
{
  FileHandle=_FileHandle;
  
  return((FAT_FILE *)FileHandle);
}

void _fclose(FAT_FILE * fp)
{
  FileHandle=0;
}

size_t _fread(void *buf, size_t size, size_t n, FAT_FILE * fp)
{
  return((size_t)FAT2_fread(buf,size,n,fp));
}

size_t _fwrite(const void *buf, size_t size, size_t n, FAT_FILE * fp)
{
  return(0);
}

#if defined(_WIN32_WCE)
#  if _WIN32_WCE < 211
     __error__ (f|w)printf functions are not supported on old WindowsCE.;
#  endif
#  include <windows.h>
#  include <stdlib.h>
#  define READFILE(file, data, length, check) \
     if (ReadFile(file, data, length, &check,NULL)) check = 0
#  define WRITEFILE(file, data, length, check)) \
     if (WriteFile(file, data, length, &check, NULL)) check = 0
#  define FCLOSE(file) CloseHandle(file)
#else
//#  include <stdio.h>
//#  include <stdlib.h>
//#  include <assert.h>
#  define READFILE(file, data, length, check) \
     check=(png_size_t)_fread(data,(png_size_t)1,length,file)
#  define WRITEFILE(file, data, length, check) \
     check=(png_size_t)_fwrite(data,(png_size_t)1, length, file)
#  define FCLOSE(file) _fclose(file)
#endif

/* Makes pngtest verbose so we can find problems (needs to be before png.h) */
#ifndef PNG_DEBUG
#  define PNG_DEBUG 0
#endif

// Turn on CPU timing
//#define PNGTEST_TIMING


#ifdef PNG_NO_FLOATING_POINT_SUPPORTED
#undef PNGTEST_TIMING
#endif

#ifdef PNGTEST_TIMING
static float t_start, t_stop, t_decode, t_encode, t_misc;
#include <time.h>
#endif

/* Define png_jmpbuf() in case we are using a pre-1.0.6 version of libpng */
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) png_ptr->jmpbuf
#endif

#ifdef PNGTEST_TIMING
static float t_start, t_stop, t_decode, t_encode, t_misc;
#if !defined(PNG_tIME_SUPPORTED)
#include <time.h>
#endif
#endif

#if defined(PNG_TIME_RFC1123_SUPPORTED)
static int tIME_chunk_present=0;
static char tIME_string[30] = "no tIME chunk present in file";
#endif

#ifdef __TURBOC__
#include <mem.h>
#endif

/* defined so I can write to a file on gui/windowing platforms */
/*  #define STDERR stderr  */
#define STDERR stdout   /* for DOS */

/* example of using row callbacks to make a simple progress meter */
static int status_pass=1;
static int status_dots_requested=0;
static int status_dots=1;

void
#ifdef PNG_1_0_X
PNGAPI
#endif
read_row_callback(png_structp png_ptr, png_uint_32 row_number, int pass);
void
#ifdef PNG_1_0_X
PNGAPI
#endif
read_row_callback(png_structp png_ptr, png_uint_32 row_number, int pass)
{
    if(png_ptr == NULL || row_number > PNG_UINT_31_MAX) return;
    if(status_pass != pass)
    {
//       fprintf(stdout,"\n Pass %d: ",pass);
       status_pass = pass;
       status_dots = 31;
    }
    status_dots--;
    if(status_dots == 0)
    {
//       fprintf(stdout, "\n         ");
       status_dots=30;
    }
//    fprintf(stdout, "r");
}

void
#ifdef PNG_1_0_X
PNGAPI
#endif
write_row_callback(png_structp png_ptr, png_uint_32 row_number, int pass);
void
#ifdef PNG_1_0_X
PNGAPI
#endif
write_row_callback(png_structp png_ptr, png_uint_32 row_number, int pass)
{
    if(png_ptr == NULL || row_number > PNG_UINT_31_MAX || pass > 7) return;
//    fprintf(stdout, "w");
}


#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED)
/* Example of using user transform callback (we don't transform anything,
   but merely examine the row filters.  We set this to 256 rather than
   5 in case illegal filter values are present.) */
static png_uint_32 filters_used[256];
void
#ifdef PNG_1_0_X
PNGAPI
#endif
count_filters(png_structp png_ptr, png_row_infop row_info, png_bytep data);
void
#ifdef PNG_1_0_X
PNGAPI
#endif
count_filters(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
    if(png_ptr != NULL && row_info != NULL)
      ++filters_used[*(data-1)];
}
#endif

#if defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
/* example of using user transform callback (we don't transform anything,
   but merely count the zero samples) */

static png_uint_32 zero_samples;

void
#ifdef PNG_1_0_X
PNGAPI
#endif
count_zero_samples(png_structp png_ptr, png_row_infop row_info, png_bytep data);
void
#ifdef PNG_1_0_X
PNGAPI
#endif
count_zero_samples(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
   png_bytep dp = data;
   if(png_ptr == NULL)return;

   /* contents of row_info:
    *  png_uint_32 width      width of row
    *  png_uint_32 rowbytes   number of bytes in row
    *  png_byte color_type    color type of pixels
    *  png_byte bit_depth     bit depth of samples
    *  png_byte channels      number of channels (1-4)
    *  png_byte pixel_depth   bits per pixel (depth*channels)
    */


    /* counts the number of zero samples (or zero pixels if color_type is 3 */

    if(row_info->color_type == 0 || row_info->color_type == 3)
    {
       int pos=0;
       png_uint_32 n, nstop;
       for (n=0, nstop=row_info->width; n<nstop; n++)
       {
          if(row_info->bit_depth == 1)
          {
             if(((*dp << pos++ ) & 0x80) == 0) zero_samples++;
             if(pos == 8)
             {
                pos = 0;
                dp++;
             }
          }
          if(row_info->bit_depth == 2)
          {
             if(((*dp << (pos+=2)) & 0xc0) == 0) zero_samples++;
             if(pos == 8)
             {
                pos = 0;
                dp++;
             }
          }
          if(row_info->bit_depth == 4)
          {
             if(((*dp << (pos+=4)) & 0xf0) == 0) zero_samples++;
             if(pos == 8)
             {
                pos = 0;
                dp++;
             }
          }
          if(row_info->bit_depth == 8)
             if(*dp++ == 0) zero_samples++;
          if(row_info->bit_depth == 16)
          {
             if((*dp | *(dp+1)) == 0) zero_samples++;
             dp+=2;
          }
       }
    }
    else /* other color types */
    {
       png_uint_32 n, nstop;
       int channel;
       int color_channels = row_info->channels;
       if(row_info->color_type > 3)color_channels--;

       for (n=0, nstop=row_info->width; n<nstop; n++)
       {
          for (channel = 0; channel < color_channels; channel++)
          {
             if(row_info->bit_depth == 8)
                if(*dp++ == 0) zero_samples++;
             if(row_info->bit_depth == 16)
             {
                if((*dp | *(dp+1)) == 0) zero_samples++;
                dp+=2;
             }
          }
          if(row_info->color_type > 3)
          {
             dp++;
             if(row_info->bit_depth == 16)dp++;
          }
       }
    }
}
#endif /* PNG_WRITE_USER_TRANSFORM_SUPPORTED */

#if defined(PNG_NO_STDIO)
/* START of code to validate stdio-free compilation */
/* These copies of the default read/write functions come from pngrio.c and */
/* pngwio.c.  They allow "don't include stdio" testing of the library. */
/* This is the function that does the actual reading of data.  If you are
   not reading from a standard C stream, you should create a replacement
   read_data function and use it at run time with png_set_read_fn(), rather
   than changing the library. */

#ifndef USE_FAR_KEYWORD
static void
pngtest_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_size_t check;

   /* fread() returns 0 on error, so it is OK to store this in a png_size_t
    * instead of an int, which is what fread() actually returns.
    */
   READFILE((FAT_FILE *)png_ptr->io_ptr, data, length, check);

   if (check != length)
   {
      png_error(png_ptr, "Read Error!");
   }
}
#else
/* this is the model-independent version. Since the standard I/O library
   can't handle far buffers in the medium and small models, we have to copy
   the data.
*/

#define NEAR_BUF_SIZE 1024
#define MIN(a,b) (a <= b ? a : b)

static void
pngtest_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   int check;
   png_byte *n_data;
   FAT_FILE * io_ptr;

   /* Check if data really is near. If so, use usual code. */
   n_data = (png_byte *)CVT_PTR_NOCHECK(data);
   io_ptr = (FAT_FILE *)CVT_PTR(png_ptr->io_ptr);
   if ((png_bytep)n_data == data)
   {
      READFILE(io_ptr, n_data, length, check);
   }
   else
   {
      png_byte buf[NEAR_BUF_SIZE];
      png_size_t read, remaining, err;
      check = 0;
      remaining = length;
      do
      {
         read = MIN(NEAR_BUF_SIZE, remaining);
         READFILE(io_ptr, buf, 1, err);
         png_memcpy(data, buf, read); /* copy far buffer to near buffer */
         if(err != read)
            break;
         else
            check += err;
         data += read;
         remaining -= read;
      }
      while (remaining != 0);
   }
   if (check != length)
   {
      png_error(png_ptr, "read Error");
   }
}
#endif /* USE_FAR_KEYWORD */

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
static void
pngtest_flush(png_structp png_ptr)
{
#if !defined(_WIN32_WCE)
   FAT_FILE * io_ptr;
   io_ptr = (FAT_FILE *)CVT_PTR((png_ptr->io_ptr));
   if (io_ptr != NULL)
      fflush(io_ptr);
#endif
}
#endif


/* This function is called when there is a warning, but the library thinks
 * it can continue anyway.  Replacement functions don't have to do anything
 * here if you don't want to.  In the default configuration, png_ptr is
 * not used, but it is passed in case it may be useful.
 */
static void
pngtest_warning(png_structp png_ptr, png_const_charp message)
{
   PNG_CONST char *name = "UNKNOWN (ERROR!)";
   if (png_ptr != NULL && png_ptr->error_ptr != NULL)
      name = (PNG_CONST char *)png_ptr->error_ptr;
   _consolePrintf("%s: libpng warning: %s\n", name, message);
}

/* This is the default error handling function.  Note that replacements for
 * this function MUST NOT RETURN, or the program will likely crash.  This
 * function is used by default, or if the program supplies NULL for the
 * error function pointer in png_set_error_fn().
 */
static void
pngtest_error(png_structp png_ptr, png_const_charp message)
{
   pngtest_warning(png_ptr, message);
   /* We can return because png_error calls the default handler, which is
    * actually OK in this case. */
}
#endif /* PNG_NO_STDIO */
/* END of code to validate stdio-free compilation */

typedef struct memory_information
{
   png_uint_32               size;
   png_voidp                 pointer;
   struct memory_information FAR *next;
} memory_information;
typedef memory_information FAR *memory_infop;

/* START of code to validate memory allocation and deallocation */
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG

static memory_infop pinformation = NULL;
static int current_allocation = 0;
static int maximum_allocation = 0;
static int total_allocation = 0;
static int num_allocations = 0;

/* Allocate memory.  For reasonable files, size should never exceed
   64K.  However, zlib may allocate more then 64K if you don't tell
   it not to.  See zconf.h and png.h for more information.  zlib does
   need to allocate exactly 64K, so whatever you call here must
   have the ability to do that.

   This piece of code can be compiled to validate max 64K allocations
   by setting MAXSEG_64K in zlib zconf.h *or* PNG_MAX_MALLOC_64K. */
png_voidp png_debug_malloc PNGARG((png_structp png_ptr, png_uint_32 size));
void png_debug_free PNGARG((png_structp png_ptr, png_voidp ptr));

png_voidp
png_debug_malloc(png_structp png_ptr, png_uint_32 size)
{

   /* png_malloc has already tested for NULL; png_create_struct calls
      png_debug_malloc directly, with png_ptr == NULL which is OK */

   if (png_ptr == NULL){
     _consolePrint("error: malloc png_ptr=NULL\n");
     while(1);
     return(NULL);
   }
   if (size==0){
     _consolePrint("error: malloc size=0\n");
     while(1);
     return(NULL);
   }
  
   /* This calls the library allocator twice, once to get the requested
      buffer and once to get a new free list entry. */
   {
      /* Disable malloc_fn and free_fn */
      memory_infop pinfo;
      png_set_mem_fn(png_ptr, NULL, NULL, NULL);
      pinfo = (memory_infop)safemalloc(&MM_DLLImage,(png_uint_32)png_sizeof (*pinfo));
      pinfo->size = size;
      current_allocation += size;
      total_allocation += size;
      num_allocations ++;
      if (current_allocation > maximum_allocation)
         maximum_allocation = current_allocation;
      pinfo->pointer = (png_voidp)safemalloc(&MM_DLLImage,size);
      /* Restore malloc_fn and free_fn */
      png_set_mem_fn(png_ptr, png_voidp_NULL, (png_malloc_ptr)png_debug_malloc,
         (png_free_ptr)png_debug_free);
      if (size != 0 && pinfo->pointer == NULL)
      {
         current_allocation -= size;
         total_allocation -= size;
         png_error(png_ptr,
           "out of memory in pngtest->png_debug_malloc.");
      }
      pinfo->next = pinformation;
      pinformation = pinfo;
      /* Make sure the caller isn't assuming zeroed memory. */
      png_memset(pinfo->pointer, 0xdd, pinfo->size);
      return (png_voidp)(pinfo->pointer);
   }
}

/* Free a pointer.  It is removed from the list at the same time. */
void
png_debug_free(png_structp png_ptr, png_voidp ptr)
{
  if (png_ptr == NULL || ptr == NULL) return;

   /* Unlink the element from the list. */
   {
      memory_infop FAR *ppinfo = &pinformation;
      for (;;)
      {
         memory_infop pinfo = *ppinfo;
         if (pinfo->pointer == ptr)
         {
            *ppinfo = pinfo->next;
            current_allocation -= pinfo->size;
            if (current_allocation < 0)
               _consolePrint("Duplicate free of memory\n");
            /* We must free the list element too, but first kill
               the memory that is to be freed. */
            png_memset(ptr, 0x55, pinfo->size);
            if(pinfo!=NULL) {
                safefree(&MM_DLLImage,pinfo);
                pinfo=NULL;
            }
            break;
         }
         if (pinfo->next == NULL)
         {
            _consolePrintf("Pointer %x not found\n", (unsigned int)ptr);
            break;
         }
         ppinfo = &pinfo->next;
      }
   }

   /* Finally free the data. */
   if(ptr!=NULL) {
       safefree(&MM_DLLImage,ptr);
       ptr=NULL;
   }
}
#endif /* PNG_USER_MEM_SUPPORTED && PNG_DEBUG */
/* END of code to test memory allocation/deallocation */

png_voidp ndspng_malloc(png_structp png_ptr, png_uint_32 size)
{

   /* png_malloc has already tested for NULL; png_create_struct calls
      png_debug_malloc directly, with png_ptr == NULL which is OK */

   if (png_ptr == NULL){
     _consolePrint("error: ndspng_malloc png_ptr=NULL\n");
     while(1);
     return(NULL);
   }
   if (size==0){
     _consolePrint("error: ndspng_malloc size=0\n");
     while(1);
     return(NULL);
   }
  
  png_voidp ret;
  
  ret=(png_voidp)safemalloc(&MM_DLLImage,size);
  
  if(ret==NULL){
    _consolePrint("error: ndspng_malloc out of memory.");
    while(1);
    return(NULL);
  }
  
  return(ret);
}

/* Free a pointer.  It is removed from the list at the same time. */
void ndspng_free(png_structp png_ptr, png_voidp ptr)
{
   if (png_ptr == NULL){
     _consolePrint("error: freeing NULL pointer\n");
     while(1);
     return;
   }
   if (ptr == 0)
   {
     _consolePrint("error: freeing NULL pointer\n");
     while(1);
     return;
   }
   
   if(ptr!=NULL)    
       safefree(&MM_DLLImage,ptr);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   static FAT_FILE * fpin;
   
   png_structp read_ptr;
   png_infop read_info_ptr, end_info_ptr;

  png_bytep row_buf;
  u32 DecodedScanLine;

  
static png_uint_32 biWidth=0,biHeight=0;
static png_uint_32 DecodeBufSize;

static b8 __Start(FAT_FILE * FileHandle)
{
#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
   jmp_buf jmpbuf;
#endif
#endif

   if ((fpin = _fopen(FileHandle)) == NULL)
   {
      _consolePrint("Could not find input buffer.\n");
      return(false);
   }

   png_debug(0, "alloc read structures\n");
/*
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
   read_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
      png_error_ptr_NULL, png_error_ptr_NULL, png_voidp_NULL,
      (png_malloc_ptr)png_debug_malloc, (png_free_ptr)png_debug_free);
#else
   read_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
      png_error_ptr_NULL, png_error_ptr_NULL, png_voidp_NULL,
      (png_malloc_ptr)ndspng_malloc, (png_free_ptr)ndspng_free);
//   read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
//      png_error_ptr_NULL, png_error_ptr_NULL);
#endif
*/
   read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
      png_error_ptr_NULL, png_error_ptr_NULL);
#if defined(PNG_NO_STDIO)
   png_set_error_fn(read_ptr, (png_voidp)"srcbuffer", pngtest_error,
       pngtest_warning);
#endif
   png_debug(0, "alloc read_info,end_info struct\n");
   read_info_ptr = png_create_info_struct(read_ptr);
   end_info_ptr = png_create_info_struct(read_ptr);
   
#ifdef PNG_SETJMP_SUPPORTED
   png_debug(0, "Setting jmpbuf for read struct\n");
#ifdef USE_FAR_KEYWORD
   if (setjmp(jmpbuf))
#else
   if (setjmp(png_jmpbuf(read_ptr)))
#endif
   {
      _consolePrintf("%s -> %s: libpng read error\n", "srcbuf","NULL");
      png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
      FCLOSE(fpin);
      FCLOSE(fpout);
      return(false);
   }
#ifdef USE_FAR_KEYWORD
   png_memcpy(png_jmpbuf(read_ptr),jmpbuf,png_sizeof(jmp_buf));
#endif
#endif

   png_debug(0, "Initializing input streams\n");
#if !defined(PNG_NO_STDIO)
   png_init_io(read_ptr, fpin);
#else
   png_set_read_fn(read_ptr, (png_voidp)fpin, pngtest_read_data);
#endif
   if(status_dots_requested == 1)
   {
      png_set_read_status_fn(read_ptr, read_row_callback);
   }
   else
   {
      png_set_read_status_fn(read_ptr, png_read_status_ptr_NULL);
   }

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED)
   {
     int i;
     for(i=0; i<256; i++)
        filters_used[i]=0;
     png_set_read_user_transform_fn(read_ptr, count_filters);
   }
#endif

   png_debug(0, "Reading info struct\n");
   png_read_info(read_ptr, read_info_ptr);
  
  {
   int bit_depth, color_type;
   int interlace_type, compression_type, filter_type;
   
   if (!png_get_IHDR(read_ptr, read_info_ptr, &biWidth, &biHeight, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_type)){
     return(false);
   }
   
   png_set_expand(read_ptr);
   png_set_gray_to_rgb(read_ptr);
   
   if (bit_depth == 16) png_set_strip_16(read_ptr);
   
//   if (color_type & PNG_COLOR_MASK_ALPHA) png_set_strip_alpha(read_ptr);
   png_set_strip_alpha(read_ptr);
   
   if (bit_depth < 8) png_set_packing(read_ptr);
//   if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA) png_set_bgr(read_ptr);
   
   DecodeBufSize=png_get_rowbytes(read_ptr, read_info_ptr);
   if (color_type == PNG_COLOR_TYPE_PALETTE){
     DecodeBufSize*=3;
     switch(bit_depth){
       case 1: DecodeBufSize*=8; break;
       case 2: DecodeBufSize*=4; break;
       case 4: DecodeBufSize*=2; break;
       case 8: DecodeBufSize*=1; break;
       default: DecodeBufSize*=8; break;
     }
   }
   
   row_buf = (png_bytep)safemalloc(&MM_DLLImage,DecodeBufSize+16); // 16byte dummy
   if(row_buf==NULL){
     _consolePrint("row_buf at NULL\n");
     while(1);
     return(false);
   }
   
   DecodedScanLine=0;
   
   char *ct="";
   
   if(color_type==PNG_COLOR_TYPE_GRAY) ct="Gray";
   if(color_type==PNG_COLOR_TYPE_PALETTE) ct="Pallete";
   if(color_type==PNG_COLOR_TYPE_RGB) ct="RGB";
   if(color_type==PNG_COLOR_TYPE_RGB_ALPHA) ct="RGBAlpha";
   if(color_type==PNG_COLOR_TYPE_GRAY_ALPHA) ct="GrayAlpha";
   
   _consolePrintf("PNGINFO=%dx%d %dbit%s\n", biWidth,biHeight,bit_depth,ct);
  }
  
  return(true);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

static void __Decode(u8 *dstbuf)
{
  DecodedScanLine++;
  
  if(dstbuf==NULL){
    png_bytep p=NULL;
    png_read_rows(read_ptr, (png_bytepp)&p, png_bytepp_NULL, 1);
    return;
  }
  
  png_read_rows(read_ptr, (png_bytepp)&dstbuf, png_bytepp_NULL, 1);
  return;
  
  png_read_rows(read_ptr, (png_bytepp)&row_buf, png_bytepp_NULL, 1);
  
  u8 *srcbuf=row_buf;
  u32 x;
  
  for(x=0;x<biWidth;x++){
    *dstbuf++=*srcbuf++;
    *dstbuf++=*srcbuf++;
    *dstbuf++=*srcbuf++;
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

static void __End(void)
{
   if(row_buf!=NULL)
       safefree(&MM_DLLImage,row_buf);
  
/*
  png_debug(0, "seek lastimage\n");
  MWin_ProgressShow("seek lastimage",biHeight-DecodedScanLine);
  
  u32 y;
  for(y=DecodedScanLine;y<biHeight;y++){
    if((y&3)==0) MWin_ProgressSetPos(y-DecodedScanLine);
    png_bytep p=NULL;
    png_read_rows(read_ptr, (png_bytepp)&p, png_bytepp_NULL, 1);
  }
  MWin_ProgressHide();
  
   png_debug(0, "Reading end_info data\n");

   png_read_end(read_ptr, end_info_ptr);
*/

   png_debug(0, "Destroying data structs\n");
   
   png_debug(1, "destroying read_ptr, read_info_ptr, end_info_ptr\n");
   png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
   png_debug(0, "Destruction complete.\n");

   FCLOSE(fpin);

}

extern void PNGINT_Free(void);

b8 PNGINT_Init(FAT_FILE * FileHandle)
{
   _consolePrintf("Testing libpng version %s\n", PNG_LIBPNG_VER_STRING);
   _consolePrintf("   with zlib   version %s\n", ZLIB_VERSION);
   _consolePrintf("%s",png_get_copyright(NULL));
   // Show the version of libpng used in building the library
   _consolePrintf(" library (%d):%s", png_access_version_number(),
      png_get_header_version(NULL));
   // Show the version of libpng used in building the application
   _consolePrintf(" pngtest (%d):%s", (unsigned long)PNG_LIBPNG_VER,
      PNG_HEADER_VERSION_STRING);
   _consolePrintf(" png_sizeof(png_struct)=%d, png_sizeof(png_info)=%d\n",
                    (long)png_sizeof(png_struct), (long)png_sizeof(png_info));

   // Do some consistency checking on the memory allocation settings, I'm not sure this matters, but it is nice to know, the first of these tests should be impossible because of the way the macros are set in pngconf.h
#if defined(MAXSEG_64K) && !defined(PNG_MAX_MALLOC_64K)
      _consolePrintf(" NOTE: Zlib compiled for max 64k, libpng not\n");
#endif
   // I think the following can happen.
#if !defined(MAXSEG_64K) && defined(PNG_MAX_MALLOC_64K)
      _consolePrintf(" NOTE: libpng compiled for max 64k, zlib not\n");
#endif

   if (strcmp(png_libpng_ver, PNG_LIBPNG_VER_STRING))
   {
      _consolePrintf(
         "Warning: versions are different between png.h and png.c\n");
      _consolePrintf("  png.h version: %s\n", PNG_LIBPNG_VER_STRING);
      _consolePrintf("  png.c version: %s\n\n", png_libpng_ver);
   }
  
  if(__Start(FileHandle)==false){
    return(false);
  }
  
  return(true);
}

void PNGINT_Free(void)
{
  __End();
}


s32 PNGINT_GetWidth(void)
{
  return(biWidth);
}

s32 PNGINT_GetHeight(void)
{
  return(biHeight);
}


void PNGINT_GetNextLine(u8 *buf)
{
  __Decode(buf);
}


/* Generate a compiler error if there is an old png.h in the search path. */
typedef version_1_2_14 your_png_h_is_not_version_1_2_14;

// ------------------------------------------------------------------------------------

bool PlugPng_Start(FAT_FILE *_FileHandle)
{
    if(PNGINT_Init(_FileHandle)==false){
      _consolePrint("PNG LoadError.\n");
      return(false);
    }
    
    return(true);
}

void PlugPng_Free(void)
{
    PNGINT_Free();
}

void PlugPng_GetBitmap24(u32 LineY,u8 *pBM)
{
    if(biHeight<=LineY) return;
    PNGINT_GetNextLine(pBM);
}

s32 PlugPng_GetWidth(void)
{
    return(PNGINT_GetWidth());
}

s32 PlugPng_GetHeight(void)
{
    return(PNGINT_GetHeight()-1);
}

int PlugPng_GetInfoIndexCount(void)
{
    return(0);
}

bool PlugPng_GetInfoStrL(int idx,char *str,int len)
{
    return(false);
}

bool PlugPng_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugPng_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif // #ifdef ExceptPng
