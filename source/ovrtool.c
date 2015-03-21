
#define VERSION "v1.0"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#ifndef _MSC_VER
#include <unistd.h>
#include <sys/param.h>
#else
#define MAXPATHLEN      1024
#endif

#include <sys/stat.h>

#ifndef bool
 typedef enum {false = 0, true = !0} bool;
#endif

typedef int32_t addr_t;
typedef unsigned char data_t;

#define EXIT_NO_OVR_SECTION	0

const data_t OverlayHeader_ID[]="OVRC FishShell OverlayCode";


void printUsage (char* programName) {
	printf ("Usage:\n");
	printf ("%s -[7/9] <app>\n", programName);
	printf ("   <app>         the application binary to apply the split to\n");
#ifdef _MSC_VER_
	system("pause");
#endif
	return;
}

int quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen) {
	const data_t* dataChunk =  data;
	char searchChunk = search[0];
	int i;
	
	for ( i = 0x500; i < dataLen; i++) {
		if (dataChunk[i] == searchChunk) {
			if ((i + searchLen) > dataLen) {
				return -1;
			}
			if (memcmp (&data[i], search, searchLen) == 0) {
				return i;
			}
		}
	}

	return -1;
}

int main(int argc, char* argv[])
{
	char *appFileName = NULL;

	addr_t OverlayOffset;			// Position of patch destination in the file

	FILE* appFile;
	FILE* overlayFile;

	data_t *appFileData = NULL;
	size_t appFileSize = 0;

	bool arm9bin=false;

	if (argc>2&&argv[1][0] == '-'){
		if(argv[1][1] == '7') arm9bin=false;
		if(argv[1][1] == '9') arm9bin=true;
	} else {
		printUsage (argv[0]);
		return EXIT_FAILURE;
	}

	if (argv[2][0] != 0){
		if (appFileName == NULL) {
			appFileName = argv[2];
		}
	} else {
		printUsage (argv[0]);
		return EXIT_FAILURE;
	}

	if (!(appFile = fopen (appFileName, "rb+"))) {
		printf ("Cannot open \"%s\" - %s\n", appFileName, strerror(errno));
		//system("pause");
		return EXIT_FAILURE;
	}

	// Load the app file and the DLDI patch file
	fseek (appFile, 0, SEEK_END);
	appFileSize = ftell(appFile);
	appFileData = (data_t*) malloc (appFileSize);
	fseek (appFile, 0, SEEK_SET);

	if (!appFileData) {
		fclose (appFile);
		if (appFileData) free (appFileData);
		printf ("Out of memory\n");
		//system("pause");
		return EXIT_FAILURE;
	}

	fread (appFileData, 1, appFileSize, appFile);
	
	fclose (appFile);

	// Find the DSDI reserved space in the file
	OverlayOffset = quickFind (appFileData, OverlayHeader_ID, appFileSize, strlen(OverlayHeader_ID));

	if (OverlayOffset < 0) {
		printf ("%s does not have a Overlay section\n", appFileName);
		//system("pause");
		return EXIT_NO_OVR_SECTION;
	}else{
		if(arm9bin==false) OverlayOffset+=64;
		printf ("Overlay section OverlayOffset=0x%x (%d)\n", OverlayOffset,OverlayOffset);
	}

	if(arm9bin==true) {
	if (!(overlayFile = fopen ("overlay.dll", "wb+"))) {
		printf ("Cannot open \"%s\" - %s\n", "overlay.dll", strerror(errno));
		//system("pause");
		return EXIT_FAILURE;
	}}else{
	if (!(overlayFile = fopen ("arm7overlay.bin", "wb+"))) {
		printf ("Cannot open \"%s\" - %s\n", "arm7overlay.bin", strerror(errno));
		//system("pause");
		return EXIT_FAILURE;
	}
	}
	
	fwrite (&appFileData[OverlayOffset], 1, appFileSize-OverlayOffset, overlayFile);
	fclose (overlayFile);
	
	if (!(appFile = fopen (appFileName, "wb+"))) {
		printf ("Cannot open \"%s\" - %s\n", appFileName, strerror(errno));
		//system("pause");
		return EXIT_FAILURE;
	}
	
	fwrite (&appFileData[0], 1, OverlayOffset-64, appFile);
	fclose (overlayFile);

	free (appFileData);

	printf ("successfully\n");
	
	//system("pause");
	return EXIT_SUCCESS;
}

