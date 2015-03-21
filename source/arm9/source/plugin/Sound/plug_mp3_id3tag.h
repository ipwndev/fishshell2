
#ifdef _WIN32
#pragma pack(1)
#define __ATTRIBUTE_PACKED__
#else
#define __ATTRIBUTE_PACKED__    __attribute__((packed))
#endif

/*****************************************************************
 *            ID3 reader definitions and prototypes              *
 *****************************************************************/

#define ID3_VERSION 3

/* ID3 common headers set */

#define TIT2    1
#define TPE1    2
#define TALB    3
#define TRCK    4
#define TYER    5
#define TCON    6
#define COMM    7

/* ID3 tag checked flags */

#define ID3_UNSYNCHRONISATION_FLAG      0x80
#define ID3_EXTENDEDHEADER_FLAG         0x40
#define ID3_EXPERIMENTALTAG_FLAG        0x20
#define ID3_FOOTERPRESENT_FLAG          0x10

/* ID3 frame checked flags */

#define FRAME_COMPRESSION_FLAG          0x0008
#define FRAME_ENCRYPTION_FLAG           0x0004
#define FRAME_UNSYNCHRONISATION_FLAG    0x0002

/* ID3 field text encoding */

#define FIELD_TEXT_ANSI                 0x00
#define FIELD_TEXT_UTF_16               0x01
#define FIELD_TEXT_UTF_16BE             0x02
#define FIELD_TEXT_UTF_8                0x03

typedef struct {
    char  id[3];
    char  title[30];
    char  artist[30];
    char  album[30];
    char  year[4];
    char  comment[28];
    char  zero;
    char  track;
    char  genre;
} __ATTRIBUTE_PACKED__ id3v1_tag;

typedef struct {
    char  id[3];
    short version;
    char  flags;
    char  size[4];
} __ATTRIBUTE_PACKED__ id3v2_tag;

typedef struct {
    char  id[4];
    char  size[4];
    short flags;
} __ATTRIBUTE_PACKED__ id3v2_frame;

#ifndef MAXLINE
#define MAX_LINE        1024
#endif

typedef struct {
    bool Enabled;
    char  name[MAX_LINE];
    char  title[MAX_LINE];
    char  artist[MAX_LINE];
    char  album[MAX_LINE];
    char  comment[MAX_LINE];
    char  year[5];
    char  track[3];
    char  genre[256];
    int   encode;
    char  id3has;
} TID3Tag;

static TID3Tag ID3Tag;

static const char* GetGenreStr(u8 Genre)
{
  // NOTE: These genre names should be compliant to ID3v1 and the Winamp extended set. Fixed by chuckstudios
  
#define ID3Tag_GenreCount (148)
static const char ID3Tag_Genre[ID3Tag_GenreCount][24]={
"Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge","Hip-Hop","Jazz","Metal",
"New Age","Oldies","Other","Pop","R&B","Rap","Reggae","Rock","Techno","Industrial",
"Alternative","Ska","Death Metal","Pranks","Soundtrack","Euro-Techno","Ambient","Trip-Hop","Vocal","Jazz+Funk",
"Fusion","Trance","Classical","Instrumental","Acid","House","Game","Sound Clip","Gospel","Noise",
"AlternRock","Bass","Soul","Punk","Space","Meditative","Instrumental Pop","Instrumental Rock","Ethnic","Gothic",
"Darkwave","Techno-Industrial","Electronic","Pop-Folk","Eurodance","Dream","Southern Rock","Comedy","Cult","Gangsta",
"Top 40","Christian Rap","Pop/Funk","Jungle","Native American","Cabaret","New Wave","Psychadelic","Rave","Showtunes",
"Trailer","Lo-Fi","Tribal","Acid Punk","Acid Jazz","Polka","Retro","Musical","Rock & Roll","Hard Rock",
"Folk","Folk-Rock","National Folk","Swing","Fast Fusion","Bebob","Latin","Revival","Celtic","Bluegrass",
"Avantgarde","Gothic Rock","Progressive Rock","Psychedelic Rock","Symphonic Rock","Slow Rock","Big Band","Chorus","Easy Listening","Acoustic",
"Humour","Speech","Chanson","Opera","Chamber Music","Sonata","Symphony","Booty Bass","Primus","Porn Groove",
"Satire","Slow Jam","Club","Tango","Samba","Folklore","Ballad","Power Ballad","Rhythmic Soul","Freestyle",
"Duet","Punk Rock","Drum Solo","A capella","Euro-House","Dance Hall","Goa","Drum & Bass","Club-House","Hardcore",
"Terror","Indie","BritPop","Negerpunk","Polsk Punk","Beat","Christian Gansta Rap","Heavy Metal","Black Metal","Crossover",
"Contemporary Christian","Christian Rock","Merengue","Salsa","Thrash Metal","Anime","JPop","Synthpop",
};

  if(ID3Tag_GenreCount<=Genre) return("Genre not found.");
  return(ID3Tag_Genre[Genre]);
}

/*
 * Description:  ID3 tags manipulation routines
 *               Provides read access to ID3 tags v1.1, v2.3.x, v2.4.x
 *               Supported ID3v2 frames: Title, Artist, Album, Track,
 *               Year, Genre, Comment.
 *
 * Copyright (c) 2004 Alexander Djourik. All rights reserved.
 *
 */

static unsigned int unpack_sint28 (const char *ptr) {
    unsigned int value = 0;

    if (ptr[0] & 0x80) return 0;

    value =  value       | (ptr[0] & 0x7f);
    value = (value << 7) | (ptr[1] & 0x7f);
    value = (value << 7) | (ptr[2] & 0x7f);
    value = (value << 7) | (ptr[3] & 0x7f);

    return value;
}

static unsigned int unpack_sint32 (const char *ptr) {
    unsigned int value = 0;

    if (ptr[0] & 0x80) return 0;

    value = (value << 8) | ptr[0];
    value = (value << 8) | ptr[1];
    value = (value << 8) | ptr[2];
    value = (value << 8) | ptr[3];

    return value;
}

static int get_frame_id (const char *id) {
    if (!memcmp(id, "TIT2", 4)) return TIT2;    // Title
    if (!memcmp(id, "TPE1", 4)) return TPE1;    // Artist
    if (!memcmp(id, "TALB", 4)) return TALB;    // Album
    if (!memcmp(id, "TRCK", 4)) return TRCK;    // Track
    if (!memcmp(id, "TYER", 4)) return TYER;    // Year
    if (!memcmp(id, "TCON", 4)) return TCON;    // Genre
    if (!memcmp(id, "COMM", 4)) return COMM;    // Comment
    return 0;
}

static void ReadID3TAG(void)
{
    id3v1_tag id3v1;
    id3v2_tag id3v2;
    id3v2_frame frame_header;
    int id3v2_size;
    char *buffer = NULL;
    char *ptr;
    
    MemSet32CPU(0,&ID3Tag,sizeof(ID3Tag));
  
    ID3Tag.Enabled=false;
  
    if(FileSize<128) return;
  
    ////////////////////////////////////////
    // ID3v1 support
    FAT2_fseek(FileHandle,-(int) sizeof(id3v1_tag),SEEK_END);
    FAT2_fread(&id3v1, sizeof(id3v1_tag),1,FileHandle);
    FAT2_fseek(FileHandle,0,SEEK_SET);
  
    if (!memcmp (id3v1.id, "TAG", 3)) {
        memcpy(ID3Tag.title, id3v1.title, 30);
        memcpy(ID3Tag.artist, id3v1.artist, 30);
        memcpy(ID3Tag.album, id3v1.album, 30);
        memcpy(ID3Tag.year, id3v1.year, 4);
        memcpy(ID3Tag.comment, id3v1.comment, 28);

        if (id3v1.genre > ID3Tag_GenreCount-1) id3v1.genre = 12;
        sprintf(ID3Tag.track, "%02d", id3v1.track);
        if (id3v1.genre && id3v1.genre != 0xFF)
            sprintf(ID3Tag.genre, "%s", GetGenreStr(id3v1.genre));
        ID3Tag.id3has |= 1;
      
        ID3Tag.Enabled=true;
    }
  
    ////////////////////////////////////////
    // ID3v2 minimal support
    FAT2_fread(&id3v2, sizeof(id3v2_tag),1,FileHandle);
  
    if (memcmp(id3v2.id, "ID3", 3)) {
        FAT2_fseek(FileHandle,0,SEEK_SET);
        return;
    }
  
    if (id3v2.size[0] & 0x80) {
        FAT2_fseek(FileHandle,0,SEEK_SET);
        return;
    }
    id3v2_size = unpack_sint28(id3v2.size);
    
    if ((buffer = (char *) safemalloc(&MM_DLLSound,id3v2_size))==NULL) {
        FAT2_fseek(FileHandle,0,SEEK_SET);
        return;
    }

    if ((id3v2.flags & ID3_UNSYNCHRONISATION_FLAG) || (id3v2.version < 3)) {
        //_consolePrintf("ReadID3TAG:id3v2.version=%d\nid3v2.flags=0x%x\n",id3v2.version,id3v2.flags);
        safefree (&MM_DLLSound,buffer);
        FAT2_fseek(FileHandle,0,SEEK_SET);
        return;
    }
    
    if (!FAT2_fread(buffer, id3v2_size,1,FileHandle)) {
        //_consolePrint("ReadID3TAG:FAT2_fread error\n");
        safefree (&MM_DLLSound,buffer);
        FAT2_fseek(FileHandle,0,SEEK_SET);
        return;
    }
    
    MemSet32CPU(0,&ID3Tag,sizeof(ID3Tag));
    
    ptr = buffer;
    
    // skip extended header if present
    if (id3v2.flags & ID3_EXTENDEDHEADER_FLAG) {
        int offset = unpack_sint32(ptr);
        //_consolePrintf("skip extended header=%d\n",offset);
        ptr += offset + 4;
    }
    
    ID3Tag.encode=-1;
    
    // read id3v2 frames
    while (ptr - buffer < id3v2_size) {
        char *data = NULL;
        int data_size, frame_id;
        int size = 0;

        // get frame header
        memcpy(&frame_header, ptr, sizeof(id3v2_frame));
        ptr += sizeof(id3v2_frame);
        data_size = unpack_sint32(frame_header.size);

        // skip unsupported frames
        if ((frame_id = get_frame_id(frame_header.id))==0 ||
                frame_header.flags & FRAME_COMPRESSION_FLAG ||
                frame_header.flags & FRAME_ENCRYPTION_FLAG ||
                frame_header.flags & FRAME_UNSYNCHRONISATION_FLAG) {
            //_consolePrintf("skip unsupported frames ID = %d,size = %d flag=0x%x, encode=0x%x\n",frame_id,data_size,frame_header.flags,*ptr);
            ptr += data_size;
            continue;
        }
        //_consolePrintf("frames ID = %d,size = %d flag=0x%x, encode=0x%x\n",frame_id,data_size,frame_header.flags,*ptr);
        if(ID3Tag.encode==-1) ID3Tag.encode=*ptr;
        
        data_size--; ptr++;

        if(ID3Tag.encode==FIELD_TEXT_UTF_16 || ID3Tag.encode==FIELD_TEXT_UTF_16BE){
            data_size-=2;
            ptr+=2;
        }
        
        switch (frame_id) {
            case TIT2: {
                data = ID3Tag.title;
                size = sizeof(ID3Tag.title) - 1; 
            }break;
            case TPE1: {
                data = ID3Tag.artist;
                size = sizeof(ID3Tag.artist) - 1; 
            }break;
            case TALB: {
                data = ID3Tag.album;
                size = sizeof(ID3Tag.album) - 1; 
            }break;
            case TRCK: {
                data = ID3Tag.track;
                size = sizeof(ID3Tag.track) - 1; 
            }break;
            case TYER: {
                data = ID3Tag.year;
                size = sizeof(ID3Tag.year) - 1; 
            }break;
            case TCON: { 
                data = ID3Tag.genre;
                size = sizeof(ID3Tag.genre) - 1; 
            }break;
            case COMM:  {
                data = ID3Tag.comment;
                size = sizeof(ID3Tag.comment) - 1;
                data_size -= 3; ptr += 3;
                
                if(ID3Tag.encode==FIELD_TEXT_UTF_16 || ID3Tag.encode==FIELD_TEXT_UTF_16BE){
                    data_size-=4;
                    ptr+=4;
                }
                
                // skip zero short description
                if (*ptr == 0) { data_size--; ptr++; }
            }break;
            default:{
                ptr += data_size;
                continue;
            }
        }

        if (data_size < size) size = data_size;
        memcpy(data, ptr, size); data[size] = '\0';
        ptr += data_size;
    }

    ID3Tag.id3has |= 2;
    
    if (buffer!=NULL) safefree (&MM_DLLSound,buffer);
    FAT2_fseek(FileHandle,0,SEEK_SET);
    
    ID3Tag.Enabled=true; 
}

