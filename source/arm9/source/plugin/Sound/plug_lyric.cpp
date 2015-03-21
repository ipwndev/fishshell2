
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds.h>
#include "_console.h"
#include "maindef.h"
#include "memtool.h"
#include "strtool.h"
#include "lang.h"

#include "plug_lyric.h"

#include "dllsound.h"

// --------------------------------------------------------------------

struct LyricsLine
{
    struct LyricsLine *prev;
    struct LyricsLine *next;

    char *buffer;
    int line_time;
    int line_number;
};

struct Song
{
    struct LyricsLine *head;

    char *title;
    char *artist;
    char *album;
    char *author;
    int offset;
    int count;
};

static struct Song *
init_song(void) /*init the song struct*/
{
    struct Song *song;

    song = (struct Song *)safemalloc(&MM_DLLSound,sizeof(struct Song));

    if(song==NULL) return(NULL);
    
    song->head  = NULL;
    song->title = NULL;
    song->artist = NULL;
    song->album = NULL;
    song->author= NULL;
    song->offset = 0;
    song->count = 0;
    return (song);
}

static void  /*add a new line to Song lyrics */
addline(struct Song *song, struct LyricsLine *node)
{
    struct LyricsLine *tar = NULL;
    node->prev = NULL;
    node->next = NULL;

    if (song->head) 
    {
        for(tar = song->head; tar->next != NULL; tar = tar->next) {
            if(node->line_time < tar->next->line_time)
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
        song->head = node;
}

static char * /*get the line content*/
get_line_body(char *line)
{
    int a = 0;

    while (line[a] != '\0')
    {
    if (line[a] == ']')
    {
        while (isblank(line[a + 1])) a++;
        if (line[a + 1] != '[')
            return (&line[a + 1]);/*return the content after the ] ,but not ][*/
    }
    a++;
    }
    return NULL;
}

static struct LyricsLine *  /*get the lyrics line from the buffer ,the buffer begin with "[" end with '\0'*/
get_lyrics_line(char *buffer, struct Song *song)
{
    struct LyricsLine *newll = NULL;
    int current_pos = 0;
    float minutes, seconds;

    while(1)
    {
        while (isblank(buffer[current_pos])) current_pos ++;
        
        if (buffer[current_pos] != '[') /* not the begin */
            return NULL;

        newll = (struct LyricsLine *)safecalloc(&MM_DLLSound,1, sizeof(struct LyricsLine));
        if (!newll)
            return (NULL);
    
        sscanf(&buffer[current_pos], "[%f:%f]", &minutes, &seconds);
        newll->line_time = minutes * 60 * 100 + seconds * 100;
        newll->buffer = str_AllocateCopy(&MM_DLLSound,get_line_body(buffer));
        addline(song, newll);
    
        while (buffer[current_pos] != ']') current_pos ++;
        current_pos ++;
    }
}

static struct Song *
read_lyrics_file(FAT_FILE *file)
{
    struct LyricsLine *line;
    struct Song *song;
    char buffer[255];
    char *x,*p;
    int line_n=0;

    if (!file)
    return (NULL);

    song = init_song();

    if(song==NULL) return(NULL);
    
    while (FAT2_fgets(buffer, sizeof(buffer), file))
    {
        if ((x = strchr(buffer, '\r'))!=NULL)/*if the file is windows format */
            *x = '\0';
        if ((x = strchr(buffer, '\n'))!=NULL)
            *x = '\0';

        for(x = buffer; isblank(*x); x++) ;
    
        if ((x[0] == '[') && (isdigit(x[1])))
        {
            get_lyrics_line(x, song);
        }
        else if ((x[0] == '[') && !(isdigit(x[1])))
        {
            if ((x[1] == 't') && (x[2] == 'i')) {       // title
                song->title = str_AllocateCopy(&MM_DLLSound,&x[4]);
                if ((p = strchr(song->title, ']'))!=NULL) *p = '\0';
            }else if ((x[1] == 'a') && (x[2] == 'r')) {        // artist
                song->artist = str_AllocateCopy(&MM_DLLSound,&x[4]);
                if ((p = strchr(song->artist, ']'))!=NULL) *p = '\0';
            }else if ((x[1] == 'a') && (x[2] == 'l')) {   // album
                song->album = str_AllocateCopy(&MM_DLLSound,&x[4]);
                if ((p = strchr(song->album, ']'))!=NULL) *p = '\0';
            }else if ((x[1] == 'b') && (x[2] == 'y')) {   // author
                song->author = str_AllocateCopy(&MM_DLLSound,&x[4]);
                if ((p = strchr(song->author, ']'))!=NULL) *p = '\0';
            }else if ((x[1] == 'o') && (x[2] == 'f') && (x[3] == 'f') && (x[4] == 's') && (x[5] == 'e') && (x[6] == 't')) {   // author
                sscanf(&x[8], "%d]", &song->offset);
            }
        }
    }
    /* set the line unmber needed by list.c*/
    for(line=song->head; line!=NULL; line=line->next)
        line->line_number=line_n++; 

    song->count=line_n;
    return (song);
}

static void save_lyrics_file(struct Song* song,int offset,FAT_FILE *file)
{
    struct LyricsLine *line;

    if(song == NULL) return;
    
    if (!file) return;
    
    if(song->title!=NULL) {
        FAT2_fprintf(file, "[ti:");
        FAT2_fwrite(song->title, strlen(song->title), 1, file);
        FAT2_fprintf(file, "]\r\n");
    }
    if(song->artist!=NULL) {
        FAT2_fprintf(file, "[ar:");
        FAT2_fwrite(song->artist, strlen(song->artist), 1, file);
        FAT2_fprintf(file, "]\r\n");
    }
    if(song->album!=NULL) {
        FAT2_fprintf(file, "[al:");
        FAT2_fwrite(song->album, strlen(song->album), 1, file);
        FAT2_fprintf(file, "]\r\n");
    }
    if(song->author!=NULL) {
        FAT2_fprintf(file, "[by:");
        FAT2_fwrite(song->author, strlen(song->author), 1, file);
        FAT2_fprintf(file, "]\r\n");
    }else{
        FAT2_fprintf(file, "[by:%s]\r\n",ROMTITLE);
    }
    
    FAT2_fprintf(file, "\r\n");
    
    int count=1;
    line = song->head;
    while (line)
    {
        int start_time=line->line_time+offset;
        int min = start_time / 6000;
        int sec = start_time % 6000;
        int per_sec = sec % 100;
        sec = sec / 100;

        FAT2_fprintf(file, "[%d:%02d.%02d]", min, sec, per_sec);
        if(line->buffer!=NULL) FAT2_fwrite(line->buffer, strlen(line->buffer), 1, file);
        FAT2_fprintf(file, "\r\n");
        
        CallBack_MWin_ProgressSetPos(Lang_GetUTF8("Progress_SaveLyric"),count++,song->count);
        
        line = line->next;
    }
}

static void lyrics_cleanup(struct Song* song)
{
    struct LyricsLine *line;

    if(song == NULL)
        return;

    for(line=song->head; line!=NULL; line=song->head)
    {
        song->head = line->next;
        if(line->buffer!=NULL) safefree(&MM_DLLSound,line->buffer);
        safefree(&MM_DLLSound,line);
    }

    if(song->artist!=NULL) safefree(&MM_DLLSound,song->artist);
    if(song->author!=NULL) safefree(&MM_DLLSound,song->author);
    if(song->title!=NULL) safefree(&MM_DLLSound,song->title);
    if(song->album!=NULL) safefree(&MM_DLLSound,song->album);

    safefree(&MM_DLLSound,song);
}

// --------------------------------------------------------------------

static char *lyrics_filename = NULL;
static struct Song *lyrics_song = NULL; /*current song name*/
static struct LyricsLine *lyrics_line = NULL; /*currnet line of lyrics*/
static int lyrics_offset = 0;

bool PlugLRC_Start(const char *pFileName)
{
	if(pFileName==NULL) return(false);
	PlugLRC_Free();
	
	lyrics_filename=str_AllocateCopy(&MM_DLLSound,pFileName);
	
	if(lyrics_filename==NULL) return(false);
	
	FAT_FILE *pfh=FAT2_fopen_AliasForRead(lyrics_filename);
	lyrics_song=read_lyrics_file(pfh);
	FAT2_fclose(pfh);
	
	if(lyrics_song!=NULL) {
	    /*_consolePrintf("title:%s\n",lyrics_song->title);
	    _consolePrintf("artist:%s\n",lyrics_song->artist);
	    _consolePrintf("album:%s\n",lyrics_song->album);
	    _consolePrintf("author:%s\n",lyrics_song->author);
	    _consolePrintf("offset:%d\n",lyrics_song->offset);
	    _consolePrintf("count:%d\n",lyrics_song->count);*/
	    lyrics_offset=lyrics_song->offset;
	    return(true);
	}
	
	return(false);
}

void PlugLRC_Free(void)
{
	if(lyrics_song!=NULL){
	    if(lyrics_offset!=lyrics_song->offset){
	        CallBack_MWin_ProgressShow(Lang_GetUTF8("Progress_SaveLyric"),lyrics_song->count);
	        CallBack_MWin_ProgressSetPos(Lang_GetUTF8("Progress_SaveLyric"),0,lyrics_song->count);
	        FAT_FILE *pfh=FAT2_fopen_AliasForWrite(lyrics_filename);
	        save_lyrics_file(lyrics_song,lyrics_offset,pfh);
	        FAT2_fclose(pfh);
	        CallBack_MWin_ProgressHide();
	    }
	    lyrics_cleanup(lyrics_song);
	}
	
	if(lyrics_filename!=NULL){
	    safefree(&MM_DLLSound,lyrics_filename);
	    lyrics_filename=NULL;
	}
	
	lyrics_song=NULL;
	lyrics_line=NULL;
	lyrics_offset=0;
}

bool PlugLRC_isOpened(void)
{
	return(lyrics_song!=NULL);
}

void PlugLRC_SetOffset(s32 ofs)
{
    lyrics_offset=ofs;
}

s32 PlugLRC_GetOffset(void)
{
	return(lyrics_offset);
}

const char* PlugLRC_GetTitle(void)
{
	if(lyrics_song==NULL) return(NULL);
	
	return(lyrics_song->title);
}

const char* PlugLRC_GetArtist(void)
{
	if(lyrics_song==NULL) return(NULL);
	
	return(lyrics_song->artist);
}

const char* PlugLRC_GetAlbum(void)
{
	if(lyrics_song==NULL) return(NULL);
	
	return(lyrics_song->album);
}

const char* PlugLRC_GetAuthor(void)
{
	if(lyrics_song==NULL) return(NULL);

	return(lyrics_song->author);
}

const char* PlugLRC_GetPrevLyric(void)
{
	if(lyrics_line!=NULL && lyrics_line->prev!=NULL && lyrics_line->prev->buffer!=NULL) return(lyrics_line->prev->buffer);
		
	return(NULL);
}

const char* PlugLRC_GetCurLyric(int start_time)
{
	if(lyrics_song==NULL) return(NULL);
	
	struct LyricsLine *line;

	int time=start_time*100+lyrics_offset;
	
	line = lyrics_song->head;
	lyrics_line = NULL;
	while (line)
	{
	    if (line->line_time > time){
	        if(line->prev==NULL) {
	            lyrics_line=line;
	            return (line->buffer);
	        }
	        lyrics_line=line->prev;
	        return (line->prev->buffer);
	    }
	    line = line->next;
	}
	
	return(NULL);
}


const char* PlugLRC_GetNextLyric(void)
{
	if(lyrics_line!=NULL && lyrics_line->next!=NULL && lyrics_line->next->buffer!=NULL) return(lyrics_line->next->buffer);
	
	return(NULL);
}
