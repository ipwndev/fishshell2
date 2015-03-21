
#include <nds.h>
#include <string.h>

#include "NDSROMIcon.h"
#include "_console.h"
#include "procstate.h"

#include "fat2.h"

static bool NDSROMIcon_Get_ins_CheckChar(char ch)
{
  if((ch<='/')&&(ch!='#')) return(false);
  if((':'<=ch)&&(ch<='@')) return(false);
  if(('['<=ch)&&(ch<='_')) return(false);
  if('{'<=ch) return(false);
  return(true);
}

static bool NDSROMIcon_Get_ins_ExistsTransPixel(TNDSROMIcon *pNDSROMIcon)
{
  u16 *psrc=&pNDSROMIcon->Size32BM[0];
  
  for(u32 y=0;y<NDSROMIcon32Height;y++){
    for(u32 x=0;x<NDSROMIcon32Width;x++){
      u16 c=*psrc++;
      if(c==0) return(true);
    }
  }
  
  return(false);
}

static inline bool NDSROMIcon_Get_ins_isTransPixel(TNDSROMIcon *pNDSROMIcon,u32 x,u32 y)
{
  u16 *psrc=&pNDSROMIcon->Size32BM[NDSROMIcon32Width*y];
  u32 c=psrc[x];
  
  if(c==0) return(true);
  return(false);
}

static void NDSROMIcon_Get_ins_TransPaint(TNDSROMIcon *pNDSROMIcon,u32 x,u32 y)
{
  u16 *psrc=&pNDSROMIcon->Size32BM[(NDSROMIcon32Width*y)+x];
  u32 c=*psrc;
  
  *psrc=0;
  
  if(0<x){
    if(c==psrc[-1]) NDSROMIcon_Get_ins_TransPaint(pNDSROMIcon,x-1,y);
  }
  if(x<(NDSROMIcon32Width-1)){
    if(c==psrc[+1]) NDSROMIcon_Get_ins_TransPaint(pNDSROMIcon,x+1,y);
  }
  if(0<y){
    if(c==psrc[-NDSROMIcon32Width]) NDSROMIcon_Get_ins_TransPaint(pNDSROMIcon,x,y-1);
  }
  if(y<(NDSROMIcon32Height-1)){
    if(c==psrc[NDSROMIcon32Width]) NDSROMIcon_Get_ins_TransPaint(pNDSROMIcon,x,y+1);
  }
}

static void NDSROMIcon_Get_ins_Conv32to16(TNDSROMIcon *pNDSROMIcon)
{
  u16 *psrc0=&pNDSROMIcon->Size32BM[NDSROMIcon32Width*0];
  u16 *psrc1=&pNDSROMIcon->Size32BM[NDSROMIcon32Width*1];
  u16 *pdst=&pNDSROMIcon->Size16BM[0];
  u8 *pdstalpha=&pNDSROMIcon->Size16Alpha[0];
  
  for(u32 y=0;y<NDSROMIcon16Height;y++){
    for(u32 x=0;x<NDSROMIcon16Width;x++){
      u32 tr=0,tg=0,tb=0;
      u32 alpha4=0;
      u16 src;
      
      src=*psrc0++;
      if(src!=0){
        tr+=(src>>0)&0x1f; tg+=(src>>5)&0x1f; tb+=(src>>10)&0x1f;
        alpha4++;
      }
      
      src=*psrc0++;
      if(src!=0){
        tr+=(src>>0)&0x1f; tg+=(src>>5)&0x1f; tb+=(src>>10)&0x1f;
        alpha4++;
      }
      
      src=*psrc1++;
      if(src!=0){
        tr+=(src>>0)&0x1f; tg+=(src>>5)&0x1f; tb+=(src>>10)&0x1f;
        alpha4++;
      }
      
      src=*psrc1++;
      if(src!=0){
        tr+=(src>>0)&0x1f; tg+=(src>>5)&0x1f; tb+=(src>>10)&0x1f;
        alpha4++;
      }
      
      *pdstalpha++=4-alpha4;
      
      if(alpha4==0){
        *pdst++=0;
        }else{
        tr/=alpha4;
        tg/=alpha4;
        tb/=alpha4;
        if(alpha4<4){
          tr=(tr*alpha4)/4;
          tg=(tg*alpha4)/4;
          tb=(tb*alpha4)/4;
        }
        *pdst++=RGB15(tr,tg,tb)|BIT15;
      }
    }
    psrc0+=NDSROMIcon32Width;
    psrc1+=NDSROMIcon32Width;
  }
}

bool NDSROMIcon_Get(const char *pFilenameAlias,TNDSROMIcon *pNDSROMIcon,u16 *pJpnTitle ,u16 *pEngTitle,bool *isCommercialROM)
{
  const u32 BinSize=0x840;
  u8 *pBinData=(u8*)safemalloc_chkmem(&MM_Temp,BinSize);
  
  FAT_FILE *pf=FAT2_fopen_AliasForRead(pFilenameAlias);
  if(pf==NULL){
    if(pBinData!=NULL){
      safefree(&MM_Temp,pBinData); pBinData=NULL;
    }
    return(false);
  }
  if(isCommercialROM){
	  u8 header[192];
	  FAT2_fread(header,1,192,pf);
	  
  	  char ID[5];
  	  ID[0]=header[12+0];
  	  ID[1]=header[12+1];
  	  ID[2]=header[12+2];
  	  ID[3]=header[12+3];
  	  ID[4]=0;
  	        
  	  //_consolePrintf("Detected ROMID: %s\n",ID);
  	        
  	  char ID2[5];
  	  ID2[0]=header[172+0];
  	  ID2[1]=header[172+1];
  	  ID2[2]=header[172+2];
  	  ID2[3]=header[172+3];
  	  ID2[4]=0;
  	          
  	  bool homebrew=false;
  	        
  	  if(strcmp("####",ID)==0) homebrew=true;
  	  if(strcmp("PASS",ID)==0) homebrew=true;
  	  if(strcmp("PASS",ID2)==0) homebrew=true;
  	  if((ID[0]==0x3d)&&(ID[1]==0x84)&&(ID[2]==0x82)&&(ID[3]==0x0a)) homebrew=true;
  	  
  	  if(!homebrew) *isCommercialROM=true;
  }
  
  u32 ID;
  FAT2_fseek(pf,0,SEEK_SET);
  FAT2_fread(&ID,4,1,pf);
  
  pBinData[0]=0xff;
  pBinData[1]=0xff;
  
  if(ID==0x4e4f4349){ // Header "ICON"
    char GameID[4];
    FAT2_fseek(pf,0x04,SEEK_SET);
    FAT2_fread(GameID,1,4,pf);
    for(u32 idx=0;idx<4;idx++){
      if(NDSROMIcon_Get_ins_CheckChar(GameID[idx])==false){
        FAT2_fclose(pf);
        if(pBinData!=NULL){
          safefree(&MM_Temp,pBinData); pBinData=NULL;
        }
        return(false);
      }
    }
    FAT2_fseek(pf,0x08,SEEK_SET);
    FAT2_fread(pBinData,1,BinSize,pf);
    }else{ // normal NDSROM file.
    char GameID[4];
    FAT2_fseek(pf,0x0c,SEEK_SET);
    FAT2_fread(GameID,1,4,pf);
    for(u32 idx=0;idx<4;idx++){
      if(NDSROMIcon_Get_ins_CheckChar(GameID[idx])==false){
        FAT2_fclose(pf);
        if(pBinData!=NULL){
          safefree(&MM_Temp,pBinData); pBinData=NULL;
        }
        return(false);
      }
    }
    u32 binofs;
    FAT2_fseek(pf,0x68,SEEK_SET);
    FAT2_fread(&binofs,1,4,pf);
    FAT2_fseek(pf,binofs,SEEK_SET);
    FAT2_fread(pBinData,1,BinSize,pf);
  }
  
  FAT2_fclose(pf);
  if(((pBinData[0]!=0x01)||(pBinData[1]!=0x00)) &&((pBinData[0]!=0x03))&&((pBinData[0]!=0x02)))
  {
    if(pBinData!=NULL){
      safefree(&MM_Temp,pBinData); pBinData=NULL;
    }
    return(false);
  }
  
  u16 *ppal=(u16*)&pBinData[0x220];
  u32 *pbm4=(u32*)&pBinData[0x020];
  
  u16 Palettes[0x10];
  
  Palettes[0x00]=0;
  for(u32 idx=0x01;idx<0x10;idx++){
    Palettes[idx]=ppal[idx]|BIT15;
  }
  
  for(u32 y=0;y<4;y++){
    for(u32 x=0;x<4;x++){
      u32 dstx=x*8;
      u32 dsty=y*8;
      for(u32 yo=0;yo<8;yo++){
        u16 *pdst=&pNDSROMIcon->Size32BM[dstx+((dsty+yo)*NDSROMIcon32Width)];
        u32 data=*pbm4++;
        for(u32 xo=0;xo<8;xo++){
          pdst[xo]=Palettes[data&0x0f];
          data>>=4;
        }
      }
    }
  }
  
  if(pJpnTitle)	MemCopy8CPU(&pBinData[0x240],pJpnTitle,0x100);
  if(pEngTitle)	MemCopy8CPU(&pBinData[0x340],pEngTitle,0x100);
  
/*
  if(NDSROMIcon_Get_ins_ExistsTransPixel(pNDSROMIcon)==false){
    u32 x,y;
    
    x=0; y=0;
    if(NDSROMIcon_Get_ins_isTransPixel(pNDSROMIcon,x,y)==false){
      NDSROMIcon_Get_ins_TransPaint(pNDSROMIcon,x,y);
    }
    x=NDSROMIcon32Width-1; y=0;
    if(NDSROMIcon_Get_ins_isTransPixel(pNDSROMIcon,x,y)==false){
      NDSROMIcon_Get_ins_TransPaint(pNDSROMIcon,x,y);
    }
    x=0; y=NDSROMIcon32Height-1;
    if(NDSROMIcon_Get_ins_isTransPixel(pNDSROMIcon,x,y)==false){
      NDSROMIcon_Get_ins_TransPaint(pNDSROMIcon,x,y);
    }
    x=NDSROMIcon32Width-1; y=NDSROMIcon32Height-1;
    if(NDSROMIcon_Get_ins_isTransPixel(pNDSROMIcon,x,y)==false){
      NDSROMIcon_Get_ins_TransPaint(pNDSROMIcon,x,y);
    }
  }
*/
  
  if(ProcState.FileList.Mode==EPSFLM_Single) NDSROMIcon_Get_ins_Conv32to16(pNDSROMIcon);
  
  if(pBinData!=NULL){
    safefree(&MM_Temp,pBinData); pBinData=NULL;
  }
  
  return(true);
}

void NDSROMIcon_DrawIcon16(TNDSROMIcon *pNDSROMIcon,CglCanvas *ptagbm,u32 dx,u32 dy)
{
  u16 *psrc=&pNDSROMIcon->Size16BM[0];
  u8 *psrcalpha4=&pNDSROMIcon->Size16Alpha[0];
  u16 *pdst=ptagbm->GetVRAMBuf();
  u32 dstwidth=ptagbm->GetWidth();
  pdst=&pdst[dx+(dstwidth*dy)];
  
  for(u32 y=0;y<NDSROMIcon16Height;y++){
    for(u32 x=0;x<NDSROMIcon16Width;x++){
      u32 ialpha4=*psrcalpha4++;
      if(ialpha4<4){
        if(0<ialpha4){
          u16 dstcol=*pdst;
          u16 srccol=*psrc++;
          u32 r,g,b;
          r=((srccol>>0)&0x1f)+(((dstcol>>0)&0x1f)*ialpha4)/4;
          g=((srccol>>5)&0x1f)+(((dstcol>>5)&0x1f)*ialpha4)/4;
          b=((srccol>>10)&0x1f)+(((dstcol>>10)&0x1f)*ialpha4)/4;
          *pdst++=RGB15(r,g,b)|BIT15;
          }else{
          *pdst++=*psrc++;
        }
        }else{
        psrc++;
        pdst++;
      }
    }
    pdst+=dstwidth-NDSROMIcon16Width;
  }
}

void NDSROMIcon_DrawIcon32(TNDSROMIcon *pNDSROMIcon,CglCanvas *ptagbm,u32 dx,u32 dy)
{
  u16 *psrc=&pNDSROMIcon->Size32BM[0];
  u16 *pdst=ptagbm->GetVRAMBuf();
  u32 dstwidth=ptagbm->GetWidth();
  pdst=&pdst[dx+(dstwidth*dy)];
  
  for(u32 y=0;y<NDSROMIcon32Height;y++){
    for(u32 x=0;x<NDSROMIcon32Width;x++){
      u16 c=*psrc++;
      if(c!=0){
        *pdst++=c;
        }else{
        pdst++;
      }
    }
    pdst+=dstwidth-NDSROMIcon32Width;
  }
}

bool NDSROMIcon_isLoaded(TNDSROMIcon *pNDSROMIcon)
{
  if(pNDSROMIcon==NULL) return(false);
  return(true);
}

