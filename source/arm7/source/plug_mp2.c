#ifdef USE_MP2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "memtoolARM7.h"
#include "_console.h"
#include "filesys.h"

#include "libmp2/frame.h"
#include "libmp2/synth.h"
#include "libmp2/mad.h"

#include "plug_mp2.h"

#pragma Ospace

// --------------------------------------------------------------------

typedef struct mad_decoder Tmad_decoder;

__attribute__ ((__section__ ("IWRAM_libglobal_mp2"))) static Tmad_decoder *pMadDecoder;

__attribute__ ((__section__ ("IWRAM_libglobal_mp2"))) static bool EndFlag;

#define SamplePerFrame (1152*1)

// --------------------------------------------------------------------

__attribute__ ((__section__ ("IWRAM_libglobal_mp2"))) static s16 *outlbuf,*outrbuf;
__attribute__ ((__section__ ("IWRAM_libglobal_mp2"))) static u32 outofs;
//static s16 outlastl,outlastr;

#define INPUT_BUFFER_SIZE (8*1024)
__attribute__ ((__section__ ("IWRAM_libglobal_mp2"))) static u8 *InputBuffer=NULL;

#pragma Otime
static
enum mad_flow input(void *data,
                    struct mad_stream *stream)
{
  size_t ReadSize,Remaining;
  u8 *ReadStart;
  
  if(stream->next_frame!=NULL)
  {
    Remaining=stream->bufend-stream->next_frame;
    if(Remaining!=0){
      DMA1_SRC = (u32)stream->next_frame;
      DMA1_DEST = (u32)InputBuffer;
      DMA1_CR=(DMA_16_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(Remaining/2);
    }
    ReadStart=InputBuffer+Remaining;
    ReadSize=INPUT_BUFFER_SIZE-Remaining;
    
    }else{
    ReadSize=INPUT_BUFFER_SIZE;
    ReadStart=InputBuffer;
    Remaining=0;
  }
  
  ReadSize=FileSys_MP2_fread(ReadStart,ReadSize);
  
  if(ReadSize==0) return MAD_FLOW_STOP;
  
//  _consolePrintf("r%d->%d\n",Remaining,ReadSize);
  
/*
  while(((DMA1_CR&DMA_ENABLE)!=0)||((DMA2_CR&DMA_ENABLE)!=0)){
  }
*/
  
  mad_stream_buffer(stream,InputBuffer,ReadSize+Remaining);
  
  stream->error=MAD_ERROR_NONE;
  
  return MAD_FLOW_CONTINUE;
}

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}
#pragma Ospace

#pragma Otime
static
enum mad_flow output(void *data,
                     struct mad_header const *header,
                     struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

/*
  if(nsamples!=SamplePerFrame){
    _consolePrintf("nsamples==%d!=%d\n",nsamples,SamplePerFrame);
    return MAD_FLOW_STOP;
  }
*/
  
  if((outlbuf==NULL)||(outrbuf==NULL)){
    outofs+=nsamples;
    return MAD_FLOW_CONTINUE;
  }
  
  if (nchannels == 1) {
    outofs+=nsamples;
    s16 *lbuf=outlbuf,*rbuf=outrbuf;
    while (nsamples--) {
      *lbuf++=*rbuf++=scale(*left_ch++);
    }
    outlbuf=lbuf; outrbuf=rbuf;
    return MAD_FLOW_CONTINUE;
  }

  if (nchannels == 2) {
    outofs+=nsamples;
    s16 *lbuf=outlbuf,*rbuf=outrbuf;
    while (nsamples--) {
      *lbuf++=scale(*left_ch++);
      *rbuf++=scale(*right_ch++);
    }
    outlbuf=lbuf; outrbuf=rbuf;
    return MAD_FLOW_CONTINUE;
  }

  return MAD_FLOW_STOP;
}
#pragma Ospace

static
enum mad_flow error(void *data,
                    struct mad_stream *stream,
                    struct mad_frame *frame)
{
/*
  _consolePrintf("decoding error 0x%04x (%s) at byte offset %u\n",
          stream->error, mad_stream_errorstr(stream),
          FileOffset);
*/

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;//MAD_FLOW_STOP;
}

// --------------------------------------------------------------------

#define madusr_Return_OK (0)
#define madusr_Return_NG (-1)

#define madusr_ExecReturn_Next (0)
#define madusr_ExecReturn_End (-1)
#define madusr_ExecReturn_Fail (-2)

static int madusr_decode_init(struct mad_decoder *decoder)
{
  decoder->sync = NULL;
  
  /* configure input, output, and error functions */
  
  mad_decoder_init(decoder, NULL, input, 0 /* header */, 0 /* filter */, output, error, 0 /* message */);
  
//  decoder->options|=MAD_OPTION_HALFSAMPLERATE;
  
  if (decoder->input_func == 0) return madusr_Return_NG;
  if (decoder->output_func == 0) return madusr_Return_NG;
  if (decoder->error_func==0) return madusr_Return_NG;
  
  struct mad_stream *stream;
  struct mad_frame *frame;
  struct mad_synth *synth;
  
  decoder->sync = (struct mad_decoder_sync*)safemalloc(sizeof(*decoder->sync));
  
  if (decoder->sync == NULL) return madusr_Return_NG;
  
  stream = &decoder->sync->stream;
  frame  = &decoder->sync->frame;
  synth  = &decoder->sync->synth;

  mad_stream_init(stream);
  mad_frame_init(frame);
  mad_synth_init(synth);

  mad_stream_options(stream, decoder->options);
  
  if(decoder->input_func(decoder->cb_data, stream)!=MAD_FLOW_CONTINUE) return madusr_Return_NG;
  
  return madusr_Return_OK;
}

#pragma Otime
static
int madusr_decode_exec(struct mad_decoder *decoder)
{
  enum mad_flow (*error_func)(void *, struct mad_stream *, struct mad_frame *);
  void *error_data;
  
  struct mad_stream *stream;
  struct mad_frame *frame;
  struct mad_synth *synth;

  error_func = decoder->error_func;
  error_data = decoder->cb_data;
  
  stream = &decoder->sync->stream;
  frame  = &decoder->sync->frame;
  synth  = &decoder->sync->synth;

  if(stream->error == MAD_ERROR_BUFLEN){
    return madusr_ExecReturn_Fail;
  }
  
  while(1){
    if (decoder->header_func) {
      if (mad_header_decode(&frame->header, stream) == -1) {
        if (!MAD_RECOVERABLE(stream->error)){
          if(stream->error==MAD_ERROR_BUFLEN){
            if(decoder->input_func(decoder->cb_data, stream)!=MAD_FLOW_CONTINUE) return madusr_ExecReturn_Fail;
            continue;
          }
          return madusr_ExecReturn_Fail;
        }
  
        if(stream->error==MAD_ERROR_BUFLEN){
          if(decoder->input_func(decoder->cb_data, stream)!=MAD_FLOW_CONTINUE) return madusr_ExecReturn_Fail;
          continue;
        }
        
        switch (error_func(error_data, stream, frame)) {
          case MAD_FLOW_STOP:
            return madusr_ExecReturn_End;
          case MAD_FLOW_BREAK:
            return madusr_ExecReturn_Fail;
          case MAD_FLOW_IGNORE:
          case MAD_FLOW_CONTINUE:
          default:
            return madusr_ExecReturn_Next;
        }
      }
  
      switch (decoder->header_func(decoder->cb_data, &frame->header)) {
        case MAD_FLOW_STOP:
          return madusr_ExecReturn_End;
        case MAD_FLOW_BREAK:
          return madusr_ExecReturn_Fail;
        case MAD_FLOW_IGNORE:
          return madusr_ExecReturn_Next;
        case MAD_FLOW_CONTINUE:
          continue;
          //break;
      }
    }
    break;
  }
  
  while(1){
    if (mad_frame_decode(frame, stream) == -1) {
    
      if (!MAD_RECOVERABLE(stream->error)){
        if(stream->error==MAD_ERROR_BUFLEN){
          if(decoder->input_func(decoder->cb_data, stream)!=MAD_FLOW_CONTINUE) return madusr_ExecReturn_Fail;
          continue;
        }
        return madusr_ExecReturn_Fail;
      }
      
      switch (error_func(error_data, stream, frame)) {
        case MAD_FLOW_STOP:
          return madusr_ExecReturn_End;
        case MAD_FLOW_BREAK:
          return madusr_ExecReturn_Fail;
        case MAD_FLOW_IGNORE:
          return madusr_ExecReturn_Next;
        case MAD_FLOW_CONTINUE:
          continue;
        default:
          break;
      }
    }
    break;
  }
  
  if((outlbuf==NULL)||(outrbuf==NULL)){
    outofs+=synth->pcm.length;
    return madusr_ExecReturn_Next;
  }
  
  if (decoder->filter_func) {
    switch (decoder->filter_func(decoder->cb_data, stream, frame)) {
      case MAD_FLOW_STOP:
        return madusr_ExecReturn_End;
      case MAD_FLOW_BREAK:
        return madusr_ExecReturn_Fail;
      case MAD_FLOW_IGNORE:
        return madusr_ExecReturn_Next;
      case MAD_FLOW_CONTINUE:
        break;
    }
  }

  mad_synth_frame(synth, frame);

  if (decoder->output_func) {
    switch (decoder->output_func(decoder->cb_data, &frame->header, &synth->pcm)) {
      case MAD_FLOW_STOP:
        return madusr_ExecReturn_End;
      case MAD_FLOW_BREAK:
        return madusr_ExecReturn_Fail;
      case MAD_FLOW_IGNORE:
      case MAD_FLOW_CONTINUE:
        break;
    }
  }
  
  return madusr_ExecReturn_Next;
}
#pragma Ospace

static
void madusr_decode_free(struct mad_decoder *decoder)
{
  struct mad_stream *stream;
  struct mad_frame *frame;
  //struct mad_synth *synth;
  
  if(decoder->sync!=NULL){
    stream = &decoder->sync->stream;
    frame  = &decoder->sync->frame;
    //synth  = &decoder->sync->synth;
    
    //mad_synth_finish(synth);
    mad_frame_finish(frame);
    mad_stream_finish(stream);
    
    safefree(decoder->sync);
    decoder->sync = NULL;
  }
  
  mad_decoder_finish(decoder);
}

// --------------------------------------------------------------------

void FreeMP2(void);

bool StartMP2(void)
{
  FreeMP2();
  
  pMadDecoder=(Tmad_decoder*)safemalloc(sizeof(Tmad_decoder));
  if(pMadDecoder==NULL){
    _consolePrintf("Tmad_decoder out of memory.\n");
    FreeMP2();
    return(false);
  }
  
  EndFlag=false;
  
  InputBuffer=(u8*)safemalloc(INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD);
  
  if(InputBuffer==NULL){
    _consolePrintf("InputBuffer out of memory.\n");
    FreeMP2();
    return(false);
  }
  
  _consolePrintf("libmad init.\n");
  
  int result;
  
  result=madusr_decode_init(pMadDecoder);
  if(result==madusr_Return_NG){
    _consolePrintf("madusr_decode_init()==madusr_Return_NG.\n");
    FreeMP2();
    return(false);
  }
  
  outlbuf=NULL;
  outrbuf=NULL;
  outofs=0;
  //outlastl=0;
  //outlastr=0;
  
  result=madusr_decode_exec(pMadDecoder);
  if(result!=madusr_ExecReturn_Next){
    _consolePrintf("madusr_decode_exec()==%d!=madusr_ExecReturn_Next.\n",result);
    FreeMP2();
    return(false);
  }
  
 /* 
  struct mad_frame *frame;
  
  frame  = &pMadDecoder->sync->frame;
  

  _consolePrintf("\n");
  
  _consolePrintf("Format=");
  switch(frame->header.layer){
    case MAD_LAYER_I:
      _consolePrintf("Layer I\n");
      break;
    case MAD_LAYER_II:
      _consolePrintf("Layer II\n");
      break;
    case MAD_LAYER_III:
      _consolePrintf("Layer III\n");
      break;
    default:
      _consolePrintf("unknown layer.\n");
      FreeMP2();
      return(false);
      break;
  }

  _consolePrintf("ChannelMode=");
  switch(frame->header.mode){
    case MAD_MODE_SINGLE_CHANNEL:
      _consolePrintf("SingleChannel\n");
      break;
    case MAD_MODE_DUAL_CHANNEL:
      _consolePrintf("DualChannel\n");
      break;
    case MAD_MODE_JOINT_STEREO:
      _consolePrintf("JointStereo\n");
      break;
    case MAD_MODE_STEREO:
      _consolePrintf("Normal LR Stereo\n");
      break;
    default:
      _consolePrintf("unknown ChannelMode.\n");
      FreeMP2();
      return(false);
      break;
  }
  
  _consolePrintf("SampleRate=%dHz\n",frame->header.samplerate);
*/
  
  return(true);
}

#pragma Otime
u32 UpdateMP2(s16 *lbuf,s16 *rbuf)
{
  if((pMadDecoder==NULL)||(EndFlag==true)){
    int idx;
    for(idx=0;idx<SamplePerFrame;idx++){
      *lbuf++=*rbuf++=0;
    }
    return(0);
  }
  
  outlbuf=lbuf;
  outrbuf=rbuf;
  outofs=0;
  
  int result=madusr_ExecReturn_Next;
  
  while((result==madusr_ExecReturn_Next)&&(outofs<SamplePerFrame)){
    result=madusr_decode_exec(pMadDecoder);
  }
  
//  if(outofs<SamplePerFrame) _consolePrintf("Render rest. %d,%d\n",outofs,SamplePerFrame);
  
  if(result!=madusr_ExecReturn_Next){
    EndFlag=true;
    if(result==madusr_ExecReturn_End) _consolePrintf("ExecReturn_End\n");
    if(result==madusr_ExecReturn_Fail) _consolePrintf("ExecReturn_Fail\n");
  }
  
  u32 idx;
  for(idx=outofs;idx<SamplePerFrame;idx++){
    *lbuf++=*rbuf++=0;
  }
  
  return(outofs);
}
#pragma Ospace

void FreeMP2(void)
{
  EndFlag=true;
  
  if(pMadDecoder!=NULL){
    madusr_decode_free(pMadDecoder);
    safefree(pMadDecoder); pMadDecoder=NULL;
  }
  
  if(InputBuffer!=NULL){
    safefree(InputBuffer); InputBuffer=NULL;
  }
}

u32 MP2_GetChannelCount(void)
{
  struct mad_frame *frame;
  
  frame  = &pMadDecoder->sync->frame;
  
  switch(frame->header.mode){
    case MAD_MODE_SINGLE_CHANNEL:
      return(1);
      //break;
    case MAD_MODE_DUAL_CHANNEL:
      return(2);
      //break;
    case MAD_MODE_JOINT_STEREO:
      return(2);
      //break;
    case MAD_MODE_STEREO:
      return(2);
      //break;
    default:
      return(1);
      //break;
  }
}

u32 MP2_GetSampleRate(void)
{
  struct mad_frame *frame;
  
  frame  = &pMadDecoder->sync->frame;
  
  return(frame->header.samplerate);
}

u32 MP2_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

#endif
