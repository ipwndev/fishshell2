
static void BMPWriter_Create(FAT_FILE *pf,u32 Width,u32 Height)
{
  u32 linelen=Width*3;
  linelen=(linelen+3)&(~3);
  
  u32 bufsize=14+40;
  u8 *pbuf=(u8*)safemalloc_chkmem(&MM_Temp,bufsize);
  
  u32 ofs=0;
  
#define add8(d) { pbuf[ofs]=d; ofs++; }
#define add16(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); ofs+=2; }
#define add32(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); pbuf[ofs+2]=(u8)((d>>16)&0xff); pbuf[ofs+3]=(u8)((d>>24)&0xff); ofs+=4; }

  // BITMAPFILEHEADER
  
  // bfType 2 byte ファイルタイプ 'BM' - OS/2, Windows Bitmap
  add8((u8)'B');
  add8((u8)'M');
  // bfSize 4 byte ファイルサイズ (byte)
  add32(bufsize);
  // bfReserved1 2 byte 予約領域 常に 0
  add16(0);
  // bfReserved2 2 byte 予約領域 常に 0
  add16(0);
  // bfOffBits 4 byte ファイル先頭から画像データまでのオフセット (byte)
  add32(14+40);
  
  // BITMAPINFOHEADER
  
  // biSize 4 byte 情報ヘッダのサイズ (byte) 40
  add32(40);
  // biWidth 4 byte 画像の幅 (ピクセル)
  add32(Width);
  // biHeight 4 byte 画像の高さ (ピクセル) biHeight の値が正数なら，画像データは下から上へ
  add32(Height);
  // biPlanes 2 byte プレーン数 常に 1
  add16(1);
  // biBitCount 2 byte 1 画素あたりのデータサイズ (bit)
  add16(24);
  // biCopmression 4 byte 圧縮形式 0 - BI_RGB (無圧縮)
  add32(0);
  // biSizeImage 4 byte 画像データ部のサイズ (byte) 96dpi ならば3780
  add32(0);
  // biXPixPerMeter 4 byte 横方向解像度 (1mあたりの画素数) 96dpi ならば3780
  add32(0);
  // biYPixPerMeter 4 byte 縦方向解像度 (1mあたりの画素数) 96dpi ならば3780
  add32(0);
  // biClrUsed 4 byte 格納されているパレット数 (使用色数) 0 の場合もある
  add32(0);
  // biCirImportant 4 byte 重要なパレットのインデックス 0 の場合もある
  add32(0);
  
#undef add8
#undef add16
#undef add32
  
  FAT2_fwrite(pbuf,1,ofs,pf);
  
  if(pbuf!=NULL){
    safefree(&MM_Temp,pbuf); pbuf=NULL;
  }
}

static void BMPWriter_Bitmap1Line(FAT_FILE *pf,u32 Width,u32 *psrcbm)
{
  u32 linelen=Width*3;
  linelen=(linelen+3)&(~3);
  
  u32 bufsize=linelen;
  u8 *pbuf=(u8*)safemalloc_chkmem(&MM_Temp,bufsize);
  
  u32 ofs=0;
  
#define add8(d) { pbuf[ofs]=d; ofs++; }
#define add16(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); ofs+=2; }
#define add32(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); pbuf[ofs+2]=(u8)((d>>16)&0xff); pbuf[ofs+3]=(u8)((d>>24)&0xff); ofs+=4; }

  for(int x=0;x<Width;x++){
    u32 col=*psrcbm++;
    u8 b=(col>>0)&0xff;
    u8 g=(col>>8)&0xff;
    u8 r=(col>>16)&0xff;
    add8(b);
    add8(g);
    add8(r);
  }
  for(u32 x=0;x<(linelen-(Width*3));x++){
    add8(0);
  }
  
#undef add8
#undef add16
#undef add32
  
  FAT2_fwrite(pbuf,1,ofs,pf);
  
  if(pbuf!=NULL){
    safefree(&MM_Temp,pbuf); pbuf=NULL;
  }
}
