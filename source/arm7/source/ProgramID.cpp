
#include <nds.h>

#include "maindef.h"

const char ROMTITLE[]="FishShell";
const char ROMVERSION[]="for iSmart Premium";
const char ROMDATE[]=""__DATE__" "__TIME__" GMT+08:00";
const char ROMDATESHORT[]=""__DATE__" "__TIME__"";
const char ROMDATEVERYSHORT[]=""__DATE__"";
const char ROMENV[]="ARM RVCT3.1 [Build 1021]";
const char ROMWEB[]="http://www.fishome.org";

const char OverlayHeader_ID[]="OVRC FishShell OverlayCode7 "__DATE__" "__TIME__" GMT+08:00";
__attribute__ ((section ("OverlayHeader"))) const char OverlayHeader_ID_CanNotAccess[]="OVRC FishShell OverlayCode7 "__DATE__" "__TIME__" GMT+08:00" "\0\0\0\0";
