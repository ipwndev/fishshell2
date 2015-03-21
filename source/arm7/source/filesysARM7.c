#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "filesysARM7.h"

#include "_console.h"

#include "../../ipc6.h"

int FileSys_GetFileSize(void) {
	return (IPC6->IR_filesize);
}

int FileSys_MP2_fread(void *buf, int size) {
	while (IPC6->IR!=IR_NULL)
		swiDelay(1);

	while (1) {
		if (IPC6->IR_EOF==true) {
			size=IPC6->IR_readbufsize;
			//      _consolePrintf("MP3FS: End of file. rest:%dbyte.\n",size);
		} else {
			//if (IPC6->IR_readbufsize<size) {
			//	//        _consolePrintf("MP3FS: File system error. IR_readbufsize=%d, size=%d\n",IPC6->IR_readbufsize,size);
			//	continue;
			//}
			if(IPC6->IR_readbufsize<size)size=IPC6->IR_readbufsize;
		}
		break;
	}

	if (size==0)
		return (0);

	DMA2_SRC = (u32)IPC6->IR_readbuf;
	DMA2_DEST = (u32)buf;
	DMA2_CR=(DMA_16_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(size/2);

	IPC6->IR_readsize=size;
	IPC6->IR=IR_MP2_fread;

	REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;

	return (size);
}

