
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds.h>
#include "_console.h"
#include "maindef.h"
#include "memtool.h"
#include "strtool.h"

#include "plug_srttitle.h"

// --------------------------------------------------------------------

struct SRTLine
{
    struct SRTLine *prev;
    struct SRTLine *next;

    char buffer[100];
    int start_time;
    int end_time;
    int number;
};

struct SRTTitle
{
    struct SRTLine *head;

    int offset;
    int count;
};

static void  /*add a new line to Song lyrics */
addline(struct SRTTitle *srt, struct SRTLine *node)
{
    struct SRTLine *tar = NULL;
    node->prev = NULL;
    node->next = NULL;

    if (srt->head) 
    {
        for(tar = srt->head; tar->next != NULL; tar = tar->next) {
            if(node->start_time < tar->next->start_time)
            {
                node->prev = tar;
                node->next = tar->next;
                tar->next = node;
                break;
            }
        }
        if(tar->next == NULL){
            node->prev = tar;
            tar->next = node;
        }
    }
    else /*if the Song is empty*/
        srt->head = node;
}

static struct SRTTitle *
init_srttitle(void) /*init the SRTTitle struct*/
{
    struct SRTTitle *srt;

    srt = (struct SRTTitle *)safemalloc(&MM_DLLDPG,sizeof(struct SRTTitle));

    if(srt==NULL) return(NULL);
   
    srt->head = NULL;
    srt->offset = 0;
    srt->count = 0;
    return (srt);
}

static struct SRTTitle *
read_srttitle_file(FAT_FILE *file)
{
    struct SRTLine *line;
    struct SRTTitle *srt;
    char buffer[255];
    char *x,*Stitle;
    int LineCount = 0;
    bool GotTime = false;
    int s_hour, s_min,s_sec,s_msec;
    int e_hour, e_min,e_sec,e_msec;
    if (!file)
    return (NULL);

    srt = init_srttitle();

    if(srt==NULL) return(NULL);
    
    //提取字幕内容和显示时间

    while (FAT2_fgets(buffer, sizeof(buffer), file))
    {
        if ((x = strchr(buffer, '\r'))!=NULL)/*if the file is windows format */
            *x = '\0';
        if ((x = strchr(buffer, '\n'))!=NULL)
            *x = '\0';
        
        Stitle=buffer;

        if(GotTime == true) { //已经取到时间标签
            if(isNumeric(Stitle) == false) {
                if(strlen(Stitle) != 0) {
                    strcat(line->buffer,Stitle);
                }
            }else{ //进入下一个字幕
                GotTime = false; //已经提取完一个字幕
            }
        }
        
        //判断并提取时间标签
        if ((x = strstr(Stitle, "-->"))!=NULL){
            //_consolePrintf("%d [%d:%d:%d,%d --> %d:%d:%d,%d]\n",LineCount,s_hour, s_min, s_sec, s_msec,e_hour, e_min, e_sec, e_msec);
            sscanf(Stitle, "%2d:%2d:%2d,%3d --> %2d:%2d:%2d,%3d",&s_hour, &s_min, &s_sec, &s_msec,&e_hour, &e_min, &e_sec, &e_msec);
            if(s_hour<0 || s_hour>99) continue;
            if(s_min<0 || s_min>99) continue;
            if(s_sec<0 || s_sec>99) continue;
            if(s_msec<0 || s_msec>999) continue;
            if(e_hour<0 || e_hour>99) continue;
            if(e_min<0 || e_min>99) continue;
            if(e_sec<0 || e_sec>99) continue;
            if(e_msec<0 || e_msec>999) continue;
            line = (struct SRTLine *)safecalloc(&MM_DLLDPG,1, sizeof(struct SRTLine));
            if (line!=NULL){
                line->start_time=s_hour*3600000+s_min*60000+s_sec*1000+s_msec;
                line->end_time=e_hour*3600000+e_min*60000+e_sec*1000+e_msec;
                addline(srt,line);
                GotTime=true;
            }
        }
    }
    /* set the line unmber needed by list.c*/
    for(line=srt->head; line!=NULL; line=line->next)
        line->number=LineCount++; 
    
    srt->count=LineCount;
    return (srt);
}

static void srttitle_cleanup(struct SRTTitle* srt)
{
    struct SRTLine *line;

    if(srt == NULL)
        return;

    for(line=srt->head; line!=NULL; line=srt->head)
    {
        srt->head = line->next;
        safefree(&MM_DLLDPG,line);
    }

    safefree(&MM_DLLDPG,srt);
}

// --------------------------------------------------------------------

static struct SRTTitle *srttitle_srt = NULL; /*current srt name*/
static struct SRTLine *srttitle_line = NULL; /*currnet line of srttitle*/
static int srttitle_offset = 0;

bool PlugSRT_Start(FAT_FILE *FileHandle)
{
	if(FileHandle==NULL) return(false);
	PlugSRT_Free();
	
	srttitle_srt=read_srttitle_file(FileHandle);
	
	if(srttitle_srt!=NULL) {
	    _consolePrintf("count:%d\n",srttitle_srt->count);
	    srttitle_offset=srttitle_srt->offset;
	    return(true);
	}
	
	return(false);
}
void PlugSRT_Free(void)
{
	if(srttitle_srt!=NULL){
	    srttitle_cleanup(srttitle_srt);
	}
	
	srttitle_srt=NULL;
	srttitle_line=NULL;
}

bool PlugSRT_isOpened(void)
{
	return(srttitle_srt!=NULL);
}

void PlugSRT_SetOffset(u32 ofs)
{
    srttitle_offset=ofs;
}

u32 PlugSRT_GetOffset(void)
{
	return(srttitle_offset);
}

const char* PlugSRT_GetCurSRTTitle(int start_time)
{
	if(srttitle_srt==NULL) return(NULL);
	
	struct SRTLine *line;

	int time=start_time*1000+srttitle_offset;
	    
	line = srttitle_srt->head;
	srttitle_line = NULL;
	while (line)
	{
	    if (line->start_time <= time && line->end_time >= time){
	        return (line->buffer);
	    }
	    line = line->next;
	}
	
	return(NULL);
}
