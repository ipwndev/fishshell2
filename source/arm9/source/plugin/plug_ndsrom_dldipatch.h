
#define ModifiedString "Based on Revision 1.4 - Mon Aug 6 19:57:56 2007 UTC (4 months, 3 weeks ago) by wntrmute"

/*
    dlditool - Dynamically Linked Disk Interface patch tool
    Copyright (C) 2006  Michael Chisholm (Chishm)

    Send all queries to chishm@hotmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    * v1.24 - 2007-08-02 - SmileyDude
        * Now using EXIT_SUCCESS and EXIT_FAILURE at the suggestion of MachinShin.
        * Defined EXIT_NO_DLDI_SECTION for when there is no DLDI section in a file.
        * Added cast to strcmp() call to appease the compiler.
        
    * v1.23 - 2007-01-23 - Chishm
        * Fixed bug when DLDI section doesn't exist
        * addr_t is now a signed int

    * v1.22 - 2007-01-12 - WinterMute
        * add search paths for dldi files

    * v1.21 - 2007-01-12 - Chishm
        * Improved error messages

    * v1.20 - 2007-01-11 - Chishm
        * Changed offset calculation method

    * v1.10 - 2007-01-07 - Chishm
        * Removed assumptions about endianess of computer
            * Word size shouldn't matter now either, except that an int type must be at least 32 bits long
        * Added *.sc.nds and *.gba.nds file extensions
        * Improved command line argument parsing

    * v1.01 - 2006-12-30 - Chishm
        * Minor bugfix parsing 3 arguments

    * v1.00 - 2006-12-25 - Chishm
        * Original release
*/
#define VERSION "v1.24"

typedef signed int addr_t;
typedef unsigned char data_t;

#define FEATURE_MEDIUM_CANREAD    0x00000001
#define FEATURE_MEDIUM_CANWRITE    0x00000002
#define FEATURE_SLOT_GBA    0x00000010
#define FEATURE_SLOT_NDS    0x00000020

#define MAGIC_TOKEN 0xBF8DA5ED

#define FIX_ALL    0x01
#define FIX_GLUE    0x02
#define FIX_GOT    0x04
#define FIX_BSS    0x08

#define DLDI_VERSION 1

#define EXIT_NO_DLDI_SECTION    2

enum DldiOffsets {
  DO_magicString = 0x00,      // "\xED\xA5\x8D\xBF Chishm"
  DO_magicToken = 0x00,      // 0xBF8DA5ED
  DO_magicShortString = 0x04,    // " Chishm"
  DO_version = 0x0C,
  DO_driverSize = 0x0D,
  DO_fixSections = 0x0E,
  DO_allocatedSpace = 0x0F,

  DO_friendlyName = 0x10,

  DO_text_start = 0x40,      // Data start
  DO_data_end = 0x44,        // Data end
  DO_glue_start = 0x48,      // Interworking glue start  -- Needs address fixing
  DO_glue_end = 0x4C,        // Interworking glue end
  DO_got_start = 0x50,      // GOT start          -- Needs address fixing
  DO_got_end = 0x54,        // GOT end
  DO_bss_start = 0x58,      // bss start          -- Needs setting to zero
  DO_bss_end = 0x5C,        // bss end

  // IO_INTERFACE data
  DO_ioType = 0x60,
  DO_features = 0x64,
  DO_startup = 0x68,  
  DO_isInserted = 0x6C,  
  DO_readSectors = 0x70,  
  DO_writeSectors = 0x74,
  DO_clearStatus = 0x78,
  DO_shutdown = 0x7C,
  DO_code = 0x80
};

data_t dldiMagicString[] = "01234567890";
//const char dldiFileExtension[] = ".dldi";


static addr_t readAddr (data_t *mem, addr_t offset) {
  return (addr_t)( 
      (mem[offset + 0] << 0) |
      (mem[offset + 1] << 8) |
      (mem[offset + 2] << 16) |
      (mem[offset + 3] << 24)
    );
}

static void writeAddr (data_t *mem, addr_t offset, addr_t value) {
  mem[offset + 0] = (data_t)(value >> 0);
  mem[offset + 1] = (data_t)(value >> 8);
  mem[offset + 2] = (data_t)(value >> 16);
  mem[offset + 3] = (data_t)(value >> 24);
}

static bool stringStartsWith (const char *str, const char *start) {
  return (strstr (str, start) == str);
}

static addr_t quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen) {
  const int* dataChunk = (const int*) data;
  int searchChunk = ((const int*)search)[0];
  addr_t i;
  addr_t dataChunkEnd = (addr_t)(dataLen / sizeof(int));

  for ( i = 0; i < dataChunkEnd; i++) {
    if (dataChunk[i] == searchChunk) {
      if ((i*sizeof(int) + searchLen) > dataLen) {
        return -1;
      }
      if (memcmp (&data[i*sizeof(int)], search, searchLen) == 0) {
        return i*sizeof(int);
      }
    }
  }

  return -1;
}

static bool DLDIPatch(data_t *appFileData,u32 appFileSize,data_t *dldiFileData,u32 dldiFileSize)
{
dldiMagicString[0]=0xED;
dldiMagicString[1]=0xA5;
dldiMagicString[2]=0x8D;
dldiMagicString[3]=0xBF;
dldiMagicString[4]=' ';
dldiMagicString[5]='C';
dldiMagicString[6]='h';
dldiMagicString[7]='i';
dldiMagicString[8]='s';
dldiMagicString[9]='h';
dldiMagicString[10]='m';

  addr_t memOffset;      // Offset of DLDI after the file is loaded into memory
  addr_t patchOffset;      // Position of patch destination in the file
  addr_t relocationOffset;  // Value added to all offsets within the patch to fix it properly
  addr_t ddmemOffset;      // Original offset used in the DLDI file
  addr_t ddmemStart;      // Start of range that offsets can be in the DLDI file
  addr_t ddmemEnd;      // End of range that offsets can be in the DLDI file
  addr_t ddmemSize;      // Size of range that offsets can be in the DLDI file

  addr_t addrIter;

  data_t *pDH;
  data_t *pAH;

//  _consolePrintf("Dynamically Linked Disk Interface patch tool " VERSION " by Michael Chisholm (Chishm)\n\n");
  
//  _consolePrintf(ModifiedString "\n\n");
  
  // Find the DSDI reserved space in the file
  patchOffset = quickFind (appFileData, dldiMagicString, appFileSize, sizeof(dldiMagicString)/sizeof(char));

  if (patchOffset < 0) {
//    _consolePrintf("Does not have a DLDI section\n");
    return(true); // return EXIT_NO_DLDI_SECTION;
  }

  pDH = dldiFileData;
  pAH = &appFileData[patchOffset];

  // Overwrite DLDI ID
  strcpy((char*)&pDH[DO_magicString],(char*)dldiMagicString);
  
  // Make sure the DLDI file is valid and usable
  if (strcmp ((char*)dldiMagicString, (char*)&pDH[DO_magicString]) != 0) {
//    _consolePrintf("Invalid DLDI file\n");
    return(false);
  }
  if (pDH[DO_version] != DLDI_VERSION) {
//    _consolePrintf("Incorrect DLDI file version. Expected %d, found %d.\n", DLDI_VERSION, pDH[DO_version]);
    return(false);
  }
  if (pDH[DO_driverSize] > pAH[DO_allocatedSpace]) {
//    _consolePrintf("Not enough space for patch. Available %d bytes, need %d bytes\n", ( 1 << pAH[DO_allocatedSpace]), ( 1 << pDH[DO_driverSize]) );
    return(false);
  }

  memOffset = readAddr (pAH, DO_text_start);
  if (memOffset == 0) {
      memOffset = readAddr (pAH, DO_startup) - DO_code;
  }
  ddmemOffset = readAddr (pDH, DO_text_start);
  relocationOffset = memOffset - ddmemOffset;

/*
  _consolePrintf("Old driver:          %s\n", &pAH[DO_friendlyName]);
  _consolePrintf("New driver:          %s\n", &pDH[DO_friendlyName]);
  _consolePrintf("\n");
  _consolePrintf("Position in file:    0x%08X\n", patchOffset);
  _consolePrintf("Position in memory:  0x%08X\n", memOffset);
  _consolePrintf("Patch base address:  0x%08X\n", ddmemOffset);
  _consolePrintf("Relocation offset:   0x%08X\n", relocationOffset);
  _consolePrintf("\n");
*/

  ddmemStart = readAddr (pDH, DO_text_start);
  ddmemSize = (1 << pDH[DO_driverSize]);
  ddmemEnd = ddmemStart + ddmemSize;

  // Remember how much space is actually reserved
  pDH[DO_allocatedSpace] = pAH[DO_allocatedSpace];
  // Copy the DLDI patch into the application
  memcpy (pAH, pDH, dldiFileSize);

  // Fix the section pointers in the header
  writeAddr (pAH, DO_text_start, readAddr (pAH, DO_text_start) + relocationOffset);
  writeAddr (pAH, DO_data_end, readAddr (pAH, DO_data_end) + relocationOffset);
  writeAddr (pAH, DO_glue_start, readAddr (pAH, DO_glue_start) + relocationOffset);
  writeAddr (pAH, DO_glue_end, readAddr (pAH, DO_glue_end) + relocationOffset);
  writeAddr (pAH, DO_got_start, readAddr (pAH, DO_got_start) + relocationOffset);
  writeAddr (pAH, DO_got_end, readAddr (pAH, DO_got_end) + relocationOffset);
  writeAddr (pAH, DO_bss_start, readAddr (pAH, DO_bss_start) + relocationOffset);
  writeAddr (pAH, DO_bss_end, readAddr (pAH, DO_bss_end) + relocationOffset);
  // Fix the function pointers in the header
  writeAddr (pAH, DO_startup, readAddr (pAH, DO_startup) + relocationOffset);
  writeAddr (pAH, DO_isInserted, readAddr (pAH, DO_isInserted) + relocationOffset);
  writeAddr (pAH, DO_readSectors, readAddr (pAH, DO_readSectors) + relocationOffset);
  writeAddr (pAH, DO_writeSectors, readAddr (pAH, DO_writeSectors) + relocationOffset);
  writeAddr (pAH, DO_clearStatus, readAddr (pAH, DO_clearStatus) + relocationOffset);
  writeAddr (pAH, DO_shutdown, readAddr (pAH, DO_shutdown) + relocationOffset);

  if (pDH[DO_fixSections] & FIX_ALL) { 
    // Search through and fix pointers within the data section of the file
    for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }

  if (pDH[DO_fixSections] & FIX_GLUE) { 
    // Search through and fix pointers within the glue section of the file
    for (addrIter = (readAddr(pDH, DO_glue_start) - ddmemStart); addrIter < (readAddr(pDH, DO_glue_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }

  if (pDH[DO_fixSections] & FIX_GOT) { 
    // Search through and fix pointers within the Global Offset Table section of the file
    for (addrIter = (readAddr(pDH, DO_got_start) - ddmemStart); addrIter < (readAddr(pDH, DO_got_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }

  if (pDH[DO_fixSections] & FIX_BSS) { 
    // Initialise the BSS to 0
    memset (&pAH[readAddr(pDH, DO_bss_start) - ddmemStart] , 0, readAddr(pDH, DO_bss_end) - readAddr(pDH, DO_bss_start));
  }

  // Write the patch back to the file
/*
  fseek (appFile, patchOffset, SEEK_SET);
  fwrite (pAH, 1, ddmemSize, appFile);
  fclose (appFile);
*/

  _consolePrintf("DLDI patched successfully\n");

  return(true);
}

static bool DLDIPatch_ARM9BIN(data_t *pARM9,u32 ARM9Size,data_t *dldiFileData,u32 dldiFileSize)
{
dldiMagicString[0]=0xED;
dldiMagicString[1]=0xA5;
dldiMagicString[2]=0x8D;
dldiMagicString[3]=0xBF;
dldiMagicString[4]=' ';
dldiMagicString[5]='C';
dldiMagicString[6]='h';
dldiMagicString[7]='i';
dldiMagicString[8]='s';
dldiMagicString[9]='h';
dldiMagicString[10]='m';

  addr_t memOffset;      // Offset of DLDI after the file is loaded into memory
  addr_t patchOffset;      // Position of patch destination in the file
  addr_t relocationOffset;  // Value added to all offsets within the patch to fix it properly
  addr_t ddmemOffset;      // Original offset used in the DLDI file
  addr_t ddmemStart;      // Start of range that offsets can be in the DLDI file
  addr_t ddmemEnd;      // End of range that offsets can be in the DLDI file
  addr_t ddmemSize;      // Size of range that offsets can be in the DLDI file

  addr_t addrIter;

  data_t *pDH;
  data_t *pAH;

  _consolePrintf("Dynamically Linked Disk Interface patch tool " VERSION " by Michael Chisholm (Chishm)\n\n");
  
  _consolePrintf(ModifiedString "\n\n");
  
  // Find the DSDI reserved space in the file
  patchOffset = quickFind (pARM9, dldiMagicString, ARM9Size, sizeof(dldiMagicString)/sizeof(char));

  if (patchOffset < 0) {
    _consolePrintf("Does not have a DLDI section\n");
    return(true); // return EXIT_NO_DLDI_SECTION;
  }

  pDH = dldiFileData;
  pAH = &pARM9[patchOffset];

  // Overwrite DLDI ID
  strcpy((char*)&pDH[DO_magicString],(char*)dldiMagicString);
  
  // Make sure the DLDI file is valid and usable
  if (strcmp ((char*)dldiMagicString, (char*)&pDH[DO_magicString]) != 0) {
    _consolePrintf("Invalid DLDI file\n");
    return(false);
  }
  if (pDH[DO_version] != DLDI_VERSION) {
    _consolePrintf("Incorrect DLDI file version. Expected %d, found %d.\n", DLDI_VERSION, pDH[DO_version]);
    return(false);
  }
  if (pDH[DO_driverSize] > pAH[DO_allocatedSpace]) {
    _consolePrintf("Not enough space for patch. Available %d bytes, need %d bytes\n", ( 1 << pAH[DO_allocatedSpace]), ( 1 << pDH[DO_driverSize]) );
    return(false);
  }

  memOffset = readAddr (pAH, DO_text_start);
  if (memOffset == 0) {
      memOffset = readAddr (pAH, DO_startup) - DO_code;
  }
  ddmemOffset = readAddr (pDH, DO_text_start);
  relocationOffset = memOffset - ddmemOffset;

  _consolePrintf("Old driver:          %s\n", &pAH[DO_friendlyName]);
  _consolePrintf("New driver:          %s\n", &pDH[DO_friendlyName]);
  _consolePrintf("\n");
  _consolePrintf("Position in file:    0x%08X\n", patchOffset);
  _consolePrintf("Position in memory:  0x%08X\n", memOffset);
  _consolePrintf("Patch base address:  0x%08X\n", ddmemOffset);
  _consolePrintf("Relocation offset:   0x%08X\n", relocationOffset);
  _consolePrintf("\n");

  ddmemStart = readAddr (pDH, DO_text_start);
  ddmemSize = (1 << pDH[DO_driverSize]);
  ddmemEnd = ddmemStart + ddmemSize;

  // Remember how much space is actually reserved
  pDH[DO_allocatedSpace] = pAH[DO_allocatedSpace];
  // Copy the DLDI patch into the application
  memcpy (pAH, pDH, dldiFileSize);

  // Fix the section pointers in the header
  writeAddr (pAH, DO_text_start, readAddr (pAH, DO_text_start) + relocationOffset);
  writeAddr (pAH, DO_data_end, readAddr (pAH, DO_data_end) + relocationOffset);
  writeAddr (pAH, DO_glue_start, readAddr (pAH, DO_glue_start) + relocationOffset);
  writeAddr (pAH, DO_glue_end, readAddr (pAH, DO_glue_end) + relocationOffset);
  writeAddr (pAH, DO_got_start, readAddr (pAH, DO_got_start) + relocationOffset);
  writeAddr (pAH, DO_got_end, readAddr (pAH, DO_got_end) + relocationOffset);
  writeAddr (pAH, DO_bss_start, readAddr (pAH, DO_bss_start) + relocationOffset);
  writeAddr (pAH, DO_bss_end, readAddr (pAH, DO_bss_end) + relocationOffset);
  // Fix the function pointers in the header
  writeAddr (pAH, DO_startup, readAddr (pAH, DO_startup) + relocationOffset);
  writeAddr (pAH, DO_isInserted, readAddr (pAH, DO_isInserted) + relocationOffset);
  writeAddr (pAH, DO_readSectors, readAddr (pAH, DO_readSectors) + relocationOffset);
  writeAddr (pAH, DO_writeSectors, readAddr (pAH, DO_writeSectors) + relocationOffset);
  writeAddr (pAH, DO_clearStatus, readAddr (pAH, DO_clearStatus) + relocationOffset);
  writeAddr (pAH, DO_shutdown, readAddr (pAH, DO_shutdown) + relocationOffset);

  if (pDH[DO_fixSections] & FIX_ALL) { 
    // Search through and fix pointers within the data section of the file
    for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }

  if (pDH[DO_fixSections] & FIX_GLUE) { 
    // Search through and fix pointers within the glue section of the file
    for (addrIter = (readAddr(pDH, DO_glue_start) - ddmemStart); addrIter < (readAddr(pDH, DO_glue_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }

  if (pDH[DO_fixSections] & FIX_GOT) { 
    // Search through and fix pointers within the Global Offset Table section of the file
    for (addrIter = (readAddr(pDH, DO_got_start) - ddmemStart); addrIter < (readAddr(pDH, DO_got_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }

  if (pDH[DO_fixSections] & FIX_BSS) { 
    // Initialise the BSS to 0
    memset (&pAH[readAddr(pDH, DO_bss_start) - ddmemStart] , 0, readAddr(pDH, DO_bss_end) - readAddr(pDH, DO_bss_start));
  }

  // Write the patch back to the file
/*
  fseek (appFile, patchOffset, SEEK_SET);
  fwrite (pAH, 1, ddmemSize, appFile);
  fclose (appFile);
*/

  _consolePrintf("Patched successfully\n");

  return(true);
}

