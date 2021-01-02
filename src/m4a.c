/* M4A support for SoX
 *
 * Uses mp4v2 for transport
 * fdkaac for M4A AAC decoding
 * encoding TODO
 *
 * Written by AO <ao@m9x.fr>
 *
 */

#include "sox_i.h"
#include <string.h>

#if defined(HAVE_M4A)

#ifdef HAVE_MP4V2_H
#include <mp4v2/mp4v2.h>
#endif

#ifdef HAVE_FDKAAC_H
#include <aacdecoder_lib.h>
#include <aacenc_lib.h>
#endif

#ifndef HAVE_LIBLTDL
  #undef DL_MP4V2
  #undef DL_FDKAAC
#endif

static const char* const mp4v2_library_names[] =
{
#ifdef DL_MP4V2
    "libmp4v2",
    "libmp4v2-0",
    "cygmp4v2-0",
#endif
    NULL
};

#ifdef DL_MP4V2
  #define MP4V2_FUNC LSX_DLENTRY_DYNAMIC
#else
  #define MP4V2_FUNC LSX_DLENTRY_STATIC
#endif



#define MP4V2_FUNC_ENTRIES(f,x) \
  MP4V2_FUNC(f,x, MP4FileHandle, MP4ReadProvider, (const char*, const MP4FileProvider*)) \
  MP4V2_FUNC(f,x, MP4FileHandle, MP4CreateProvider, (const char*, uint32_t, const MP4FileProvider*)) \
  MP4V2_FUNC(f,x, uint32_t, MP4GetNumberOfTracks, (MP4FileHandle, const char*, uint8_t)) \
  MP4V2_FUNC(f,x, uint8_t, MP4GetTrackAudioMpeg4Type, (MP4FileHandle, MP4TrackId)) \
  MP4V2_FUNC(f,x, MP4TrackId, MP4FindTrackId, (MP4FileHandle, uint16_t, const char*, uint8_t)) \
  MP4V2_FUNC(f,x, bool, MP4GetTrackESConfiguration, (MP4FileHandle, MP4TrackId, uint8_t**, uint32_t*)) \
  MP4V2_FUNC(f,x, MP4SampleId, MP4GetTrackNumberOfSamples, (MP4FileHandle, MP4TrackId)) \
  MP4V2_FUNC(f,x, uint32_t, MP4GetTrackMaxSampleSize, (MP4FileHandle, MP4TrackId )) \
  MP4V2_FUNC(f,x, MP4Duration, MP4GetTrackFixedSampleDuration, (MP4FileHandle, MP4TrackId )) \
  MP4V2_FUNC(f,x, MP4SampleId, MP4GetSampleIdFromTime, (MP4FileHandle, MP4TrackId, MP4Timestamp, bool)) \
  MP4V2_FUNC(f,x, MP4Timestamp, MP4GetSampleTime, (MP4FileHandle,MP4TrackId, MP4SampleId )) \
  MP4V2_FUNC(f,x, int, MP4GetTrackAudioChannels, (MP4FileHandle, MP4TrackId )) \
  MP4V2_FUNC(f,x, bool, MP4ReadSample, (MP4FileHandle, MP4TrackId, MP4SampleId, uint8_t**, uint32_t*, MP4Timestamp*, MP4Duration*, MP4Duration*, bool* )) \
  MP4V2_FUNC(f,x, void, MP4Close, (MP4FileHandle, uint32_t ))

static const char* const fdkaac_library_names[] =
{
#ifdef DL_FDKAAC
    "libfdkaac",
    "libfdkaac-0",
    "cygfdkaac-0",
#endif
    NULL
};

#ifdef DL_FDKAAC
  #define FDKAAC_FUNC LSX_DLENTRY_DYNAMIC
#else
  #define FDKAAC_FUNC LSX_DLENTRY_STATIC
#endif

#define FDKAAC_FUNC_ENTRIES(f,x) \
    FDKAAC_FUNC(f,x, HANDLE_AACDECODER, aacDecoder_Open, (TRANSPORT_TYPE, UINT)) \
    FDKAAC_FUNC(f,x, AAC_DECODER_ERROR, aacDecoder_SetParam, ( const HANDLE_AACDECODER, const AACDEC_PARAM, const INT )) \
    FDKAAC_FUNC(f,x, AAC_DECODER_ERROR, aacDecoder_ConfigRaw, ( HANDLE_AACDECODER , UCHAR *[], const UINT[] )) \
    FDKAAC_FUNC(f,x, AAC_DECODER_ERROR, aacDecoder_Fill, ( HANDLE_AACDECODER, UCHAR *[], const UINT[], UINT* )) \
    FDKAAC_FUNC(f,x, AAC_DECODER_ERROR, aacDecoder_DecodeFrame, ( HANDLE_AACDECODER, INT_PCM  *, const INT, const UINT )) \
    FDKAAC_FUNC(f,x, CStreamInfo*, aacDecoder_GetStreamInfo, ( HANDLE_AACDECODER )) \
    FDKAAC_FUNC(f,x, void, aacDecoder_Close, ( HANDLE_AACDECODER ))


void* lsxm4a_open( const char* name, MP4FileMode UNUSED(mode) )
{
    // only read supported at the moment
    //if (mode != FILEMODE_READ)
    //    return NULL;

    // hacky way to retrieve pointer
    char * endptr;
    unsigned long ptr = strtoul( name, &endptr, 16 );
    return (void*) ptr;


}

int lsxm4a_seek( void* handle, int64_t pos )
{
    sox_format_t *ft = (sox_format_t *) handle;
    return lsx_seeki(ft, pos, 0); // absolute position
}

/*
int lsxm4a_getSize( void* handle, int64_t* nout )
{
    sox_format_t *ft = (sox_format_t *) handle;
    *nout = lsx_filelength(ft);

    //avoil fail for creating new file
    if (ft->mode == 'r' && *nout == 0)
    {
        return 1;
    }
    return 0;
}
*/

int64_t lsxm4a_size( void* handle )
{
    sox_format_t *ft = (sox_format_t *) handle;
    return lsx_filelength(ft);
}

int lsxm4a_read( void* handle, void* buffer, int64_t size, int64_t* nin, int64_t UNUSED(maxChunkSize))
{
    sox_format_t *ft = (sox_format_t *) handle;
    *nin = lsx_readbuf(ft, buffer, size);
    if (*nin == 0) return 1;
    return 0;
}

int lsxm4a_write( void* handle, const void* buffer, int64_t size, int64_t* nout, int64_t UNUSED(maxChunkSize) )
{
    sox_format_t *ft = (sox_format_t *) handle;
    *nout = lsx_writebuf(ft, buffer, size);
    if (*nout == 0) return 1;
    return 0;
}

int lsxm4a_close( void* UNUSED(handle) )
{
    // do nothing libsox will handle close file
    return 0;
}

MP4FileProvider mp4_file_provider = { lsxm4a_open, lsxm4a_seek, lsxm4a_read, lsxm4a_write,lsxm4a_close, lsxm4a_size };


#endif

// try this instead:
// 2048 = max frame size
// 8 = max num of channels
#define DECODER_BUFFSIZE      (2048 * sizeof(INT_PCM))
#define DECODER_MAX_CHANNELS  8


/* Private data */
typedef struct m4a_priv_t {
  MP4FileHandle h_mp4file;
  MP4TrackId audio_track_id;
  unsigned long sample_id, num_samples;
  HANDLE_AACDECODER h_aacdecoder;
  HANDLE_AACENCODER h_aacencoder;

  unsigned int samples_in_decoded_frame;

  size_t transport_buffer_size;
  uint8_t* transport_buffer;
  uint8_t* transport_pos_ptr;
  size_t transport_leftover_size;

  size_t decoder_buffer_size;
  uint8_t* decoder_buffer; // TODO : use INT_PCM* here ? avoid casts ?
  uint64_t skip_samples;

  size_t encoder_buffer_size;
  INT_PCM* encoder_buffer;



  LSX_DLENTRIES_TO_PTRS(MP4V2_FUNC_ENTRIES, mp4v2_dl);
  LSX_DLENTRIES_TO_PTRS(FDKAAC_FUNC_ENTRIES, fdkaac_dl);

} priv_t;



#if HAVE_MP4V2_H && HAVE_FDKAAC_H

// read from buffers but do nothing with the bytes.
// p->decode_buffer must be allocated before
static int m4a_readbuffer(priv_t * p)
{
    AAC_DECODER_ERROR aac_err;
    bool mp4_err = false;

    // run until something significant happen
    while(1)
    {
        // decode frame
        aac_err = p->aacDecoder_DecodeFrame( p->h_aacdecoder, (INT_PCM *) p->decoder_buffer, p->decoder_buffer_size / sizeof(INT_PCM), 0 );
        if (aac_err == AAC_DEC_OK)
        {
            // frame decoded ok, then retrieve frame info
            CStreamInfo *stream_info = p->aacDecoder_GetStreamInfo( p->h_aacdecoder );
            p->samples_in_decoded_frame = stream_info->frameSize;

            return SOX_SUCCESS;

        }
        else if (aac_err == AAC_DEC_NOT_ENOUGH_BITS)
        {
            // so try getting transport bytes through MP4ReadSample
            if (!p->transport_leftover_size)
            {
                // make a copy because overwritten
                unsigned int transport_buffer_size = p->transport_buffer_size;
                mp4_err = p->MP4ReadSample(p->h_mp4file, p->audio_track_id, p->sample_id, &p->transport_buffer, &transport_buffer_size, 0, 0, 0, 0);
                if (!mp4_err || p->transport_buffer == NULL)
                {
                    goto fail;
                }

                // set the transport ptr at begining of transport buffer
                p->transport_pos_ptr = p->transport_buffer;
                p->transport_leftover_size = transport_buffer_size;

                // next time read next sample ID
                p->sample_id++;

            }
            else
            {
                p->transport_pos_ptr = p->transport_pos_ptr + p->transport_leftover_size;
            }
            size_t input_size = p->transport_leftover_size;
            aac_err = aacDecoder_Fill( p->h_aacdecoder, &p->transport_pos_ptr, &input_size, &p->transport_leftover_size);
            if (aac_err) goto fail;

            // warn : do not clean the buffer until aacDecoder_DecodeFrame has been called.
            // do nothing more so frame will be decoded et next loop
        }
        else
        {
            goto fail;
        }
    }

fail:
  return SOX_EOF;
}


static int m4a_startread(sox_format_t * ft)
{
  priv_t *p = (priv_t *) ft->priv;

  //sox_bool ignore_length = ft->signal.length == SOX_IGNORE_LENGTH;

  int open_library_result;

  LSX_DLLIBRARY_OPEN(
      p,
      mp4v2_dl,
      MP4V2_FUNC_ENTRIES,
      "MP4V2 library",
      mp4v2_library_names,
      open_library_result);
  if (open_library_result)
    return SOX_EOF;

  LSX_DLLIBRARY_OPEN(
      p,
      fdkaac_dl,
      FDKAAC_FUNC_ENTRIES,
      "FDKAAC library",
      fdkaac_library_names,
      open_library_result);

  if (open_library_result)
    return SOX_EOF;

  uint8_t* config = NULL;

  // a pointer will be passed here. 100bytes should do it.
  char filename[100];
  // hackish way of passing handle pointer.
  sprintf( filename, "%p", ft );
  // open the file as a stream through provider.
  p->h_mp4file = p->MP4ReadProvider( filename, &mp4_file_provider );
  if ( !p->h_mp4file ) goto fail;

  // get track count. does not support multi track m4a
  unsigned int audio_track_count = p->MP4GetNumberOfTracks(p->h_mp4file, MP4_AUDIO_TRACK_TYPE, 0);
  if (audio_track_count != 1)
  {
      lsx_debug("mp4v2 number of tracks != 1");
      goto fail;
  }

  // find audio track id
  p->audio_track_id = p->MP4FindTrackId( p->h_mp4file, 0, MP4_AUDIO_TRACK_TYPE, 0 );

  // check track 1 type
  unsigned int track_type = p->MP4GetTrackAudioMpeg4Type(p->h_mp4file, p->audio_track_id);
  if ((track_type != MP4_MPEG4_AAC_LC_AUDIO_TYPE) &&
      (track_type != MP4_MPEG4_AAC_SSR_AUDIO_TYPE) &&
      (track_type != MP4_MPEG4_AAC_HE_AUDIO_TYPE))
  {
      goto fail;
  }

  // init aac decoder
  p->h_aacdecoder = p->aacDecoder_Open( TT_MP4_RAW, 1 );
  if ( !p->h_aacdecoder ) goto fail;

  // always decode to stereo
  p->aacDecoder_SetParam( p->h_aacdecoder, AAC_PCM_MIN_OUTPUT_CHANNELS, 2 );
  p->aacDecoder_SetParam( p->h_aacdecoder, AAC_PCM_MAX_OUTPUT_CHANNELS, 2 );

  // get config from mp4 file and pass it to decoder
  uint32_t config_size;

  bool mp4_err = false;
  mp4_err = p->MP4GetTrackESConfiguration( p->h_mp4file, p->audio_track_id, &config, &config_size);
  if (!mp4_err) goto fail;

  AAC_DECODER_ERROR aac_err;
  aac_err = p->aacDecoder_ConfigRaw( p->h_aacdecoder, &config, &config_size );
  free(config);
  config = NULL;
  if (aac_err) goto fail;

  p->sample_id = 1;

  p->num_samples = p->MP4GetTrackNumberOfSamples( p->h_mp4file, p->audio_track_id );

  // check if file has fixed sample duration
  // maybe not useful but audio files should not have variable one
  //MP4Duration sample_duration = MP4GetTrackFixedSampleDuration(p->h_mp4file, p->audio_track_id);
  //if (sample_duration == MP4_INVALID_DURATION)
  //{
  //    goto fail;
  //}

  // allocate transport buffer
  p->transport_buffer_size = p->MP4GetTrackMaxSampleSize(p->h_mp4file,  p->audio_track_id);
  p->transport_buffer = lsx_malloc(p->transport_buffer_size);
  p->transport_leftover_size = 0;

  // allocate decoder buffer
  p->decoder_buffer_size = DECODER_BUFFSIZE * DECODER_MAX_CHANNELS;
  p->decoder_buffer = lsx_malloc(p->decoder_buffer_size);

  // initialize samples to be zero
  p->samples_in_decoded_frame = 0;
  p->skip_samples = 0; // used for exact seeking

  // start to read
  int readbuf_err = m4a_readbuffer(p);
  if (readbuf_err == SOX_EOF)
  {
      goto fail;
  }

  //store data for sox
  CStreamInfo *stream_info = p->aacDecoder_GetStreamInfo( p->h_aacdecoder );
  p->samples_in_decoded_frame = stream_info->frameSize;

  ft->signal.length = SOX_UNSPEC;
  ft->signal.precision = SAMPLE_BITS; // aac internals == 16.
  ft->encoding.encoding = SOX_ENCODING_AAC;

  ft->signal.channels = stream_info->numChannels;
  ft->signal.rate = stream_info->sampleRate;

  ft->signal.length = p->num_samples * stream_info->frameSize * stream_info->numChannels;

  return SOX_SUCCESS;

fail:
  if (config) free(config);
  if ( p ) {
      if ( p->h_aacdecoder ) p->aacDecoder_Close( p->h_aacdecoder );
      if ( p->h_mp4file ) p->MP4Close( p->h_mp4file, 0 );
      if ( p->decoder_buffer ) free (p->decoder_buffer);
      if ( p->transport_buffer ) free (p->transport_buffer);
  }
  return SOX_EOF;

}

static size_t m4a_read(sox_format_t * ft, sox_sample_t *output_buffer, size_t requested_length)
{
    priv_t *p = (priv_t *) ft->priv;

    size_t obuf_pos = 0;

    //lsx_debug("m4aread --------------------- requested:%d", requested_length);

    // requested length is total with channels
    requested_length /= ft->signal.channels;

    while (1)
    {
        // check if data can be skipped
        if (p->skip_samples >= p->samples_in_decoded_frame)
        {
            // handle skip of data
            p->skip_samples -= p->samples_in_decoded_frame;
            p->samples_in_decoded_frame = 0;
        }
        else
        {
            // compute how much can be fed to output at this loop
            unsigned int out_samples = min(p->samples_in_decoded_frame - p->skip_samples, requested_length);

            // position of start buffer
            size_t ibuf_pos = p->skip_samples * ft->signal.channels;
            size_t ibuf_pos_end = (p->skip_samples + out_samples) * ft->signal.channels;

            while(ibuf_pos < ibuf_pos_end)
            {
                output_buffer[obuf_pos++] = SOX_SIGNED_16BIT_TO_SAMPLE(((INT_PCM *)p->decoder_buffer)[ibuf_pos++],);
            }

            //lsx_debug("m4aread loop len:%d , skip:%d, out_samples:%d", requested_length, p->skip_samples, out_samples);

            // for next loop how much data to skip
            p->skip_samples += out_samples;
            if (p->skip_samples >= p->samples_in_decoded_frame)
            {
                // will need read
                p->samples_in_decoded_frame = 0;
                p->skip_samples = 0;
                //lsx_debug("m4aread need read");
            }

            if (out_samples >= requested_length)
            {
                //lsx_debug("m4aread --------------return ------- skip:%d obuf_pos:%d", p->skip_samples, obuf_pos);
                // obuf_pos contains channel factor
                return obuf_pos;
            }
            else
            {
                requested_length -= out_samples;
            }
        }


        // start to read
        int readbuf_err = m4a_readbuffer(p);
        if (readbuf_err == SOX_EOF)
        {
            //lsx_debug("m4aread readbuf EOF");
            return obuf_pos;
        }
    }

    return 0;
}

static int m4a_stopread(sox_format_t * ft)
{
  priv_t *p=(priv_t*) ft->priv;

  p->aacDecoder_Close( p->h_aacdecoder );
  p->MP4Close( p->h_mp4file, 0 );

  if (p->decoder_buffer) free (p->decoder_buffer);
  if (p->transport_buffer) free (p->transport_buffer);

  LSX_DLLIBRARY_CLOSE(p, fdkaac_dl);
  LSX_DLLIBRARY_CLOSE(p, mp4v2_dl);
  return SOX_SUCCESS;
}

static int m4a_seek(sox_format_t * ft, uint64_t offset)
{
    //off_t seek_ret, stream_offset = 0;
    priv_t *p=(priv_t*) ft->priv;

    // sox offset contains channels also
    offset /= ft->signal.channels;

    // this is a trick so the aac decoder will output the same bytes for the selected range
    // as trim without seek optimization. for this the seek needs to reaf more frames ahead.
    // the value 2 seems to be the minimum for this to work.
    unsigned int frames_ahead = 2;

    // set sample id of decoder to the desired position
    MP4SampleId sample_id = p->MP4GetSampleIdFromTime(p->h_mp4file,  p->audio_track_id, offset, true);

    // handle case too close from start
    p->sample_id = sample_id - min(frames_ahead, sample_id - 1);

    // but there can be an slight difference with requested offset, so compute it
    MP4Timestamp start_sample_offset = p->MP4GetSampleTime(p->h_mp4file,  p->audio_track_id, p->sample_id);

    p->aacDecoder_SetParam(p->h_aacdecoder, AAC_TPDEC_CLEAR_BUFFER, 1);

    p->skip_samples = offset - start_sample_offset;

    // discard decoded data
    p->samples_in_decoded_frame = 0;
    p->transport_leftover_size = 0;

    return 0;
}

static int m4a_startwrite(sox_format_t * ft)
{
  priv_t *p = (priv_t *) ft->priv;

  int open_library_result;

  LSX_DLLIBRARY_OPEN(
      p,
      mp4v2_dl,
      MP4V2_FUNC_ENTRIES,
      "MP4V2 library",
      mp4v2_library_names,
      open_library_result);
  if (open_library_result)
    return SOX_EOF;

  LSX_DLLIBRARY_OPEN(
      p,
      fdkaac_dl,
      FDKAAC_FUNC_ENTRIES,
      "FDKAAC library",
      fdkaac_library_names,
      open_library_result);

  if (open_library_result)
    return SOX_EOF;

  uint32_t max_compression = 6;
  uint32_t compression_level = max_compression;

  if (ft->encoding.compression != HUGE_VAL) {
    compression_level = (uint32_t)ft->encoding.compression;
    if (compression_level != ft->encoding.compression || compression_level > max_compression) {
      lsx_fail_errno(ft, SOX_EINVAL,
                 "FLAC compression level must be a whole number from 1 to %i",
                 max_compression);
      return SOX_EOF;
    }
  }

  lsx_debug("m4a compression level : %d", compression_level);

  // initialize aac encoder
  // 0 to initialize all features, takes more memory. 2 to initialize for up to 2 channels.
  AACENC_ERROR err = aacEncOpen(&p->h_aacencoder, 0, 2);

  // setup params
  aacEncoder_SetParam(p->h_aacencoder, AACENC_CHANNELMODE, (int)ft->signal.channels == 1 ? MODE_1 : MODE_2);
  aacEncoder_SetParam(p->h_aacencoder, AACENC_SAMPLERATE, (unsigned int)ft->signal.rate);

  aacEncoder_SetParam(p->h_aacencoder, AACENC_AOT, AOT_AAC_LC);
  //ex_aacEncoder_SetParam(p->h_aacencoder, AACENC_BITRATE, ); // bitrate is for constant bitrate

  if (compression_level == 6)
  {
      aacEncoder_SetParam(p->h_aacencoder, AACENC_BITRATE, 256*1024); // bitrate is for constant bitrate
      aacEncoder_SetParam(p->h_aacencoder, AACENC_BITRATEMODE, 0); // BITRATEMODE is for VBR. 5 for VBR5, 5 is max quality, 1 is lowest, 0 disable VBR
      aacEncoder_SetParam(p->h_aacencoder, AACENC_BANDWIDTH, 23000); // bitrate is for constant bitrate
  }
  else
  {
    aacEncoder_SetParam(p->h_aacencoder, AACENC_BITRATEMODE, compression_level); // BITRATEMODE is for VBR. 5 for VBR5, 5 is max quality, 1 is lowest, 0 disable VBR
  }




  aacEncoder_SetParam(p->h_aacencoder, AACENC_TRANSMUX, TT_MP4_RAW);
  aacEncoder_SetParam(p->h_aacencoder, AACENC_AFTERBURNER,1);

  err = aacEncEncode(p->h_aacencoder,NULL, NULL,NULL,NULL);
  if (err != AACENC_OK) {
      lsx_fail_errno(ft,SOX_EOF,"aacEncEncode parameter initialiation error");
      return SOX_EOF;
  }

  AACENC_InfoStruct	 aacInfo;

  aacEncInfo(p->h_aacencoder, &aacInfo);

  /* allocate or grow transport buffer */
  p->transport_buffer_size = aacInfo.maxOutBufBytes; // verify it is correct
  lsx_debug("m4a encode transport_buffer_size %zud", p->transport_buffer_size);
  p->transport_buffer = lsx_malloc(p->transport_buffer_size);

  //ft->signal.length = SOX_UNSPEC;
  ft->signal.precision = SAMPLE_BITS;


  // create the MP4 file

  // a pointer will be passed here. 100bytes should do it.
  char filename[100];
  // hackish way of passing handle pointer.
  sprintf( filename, "%p", ft );
  // open the file as a stream through provider.
  p->h_mp4file = MP4CreateProvider( filename, 0, &mp4_file_provider );
  if ( !p->h_mp4file )
  {
      lsx_debug("fail MP4CreateProvider");
      // TODO : goto fail and clean memory
      return SOX_EOF;
  }

  p->audio_track_id = MP4AddAudioTrack(p->h_mp4file, ft->signal.rate, aacInfo.frameLength, MP4_MPEG4_AUDIO_TYPE);
  MP4SetAudioProfileLevel(p->h_mp4file, 0x0F); //high quality audio profile level 2. TOCHECK
  MP4SetTrackESConfiguration(p->h_mp4file, p->audio_track_id, aacInfo.confBuf, aacInfo.confSize);

  return(SOX_SUCCESS);
}

static size_t m4a_write(sox_format_t * ft, const sox_sample_t *buf, size_t const len)
{
    //lsx_debug("m4a_write %d", len);

    priv_t * p = (priv_t *)ft->priv;
    size_t i;
    //int clips = 0;

    /* allocate or grow buffer */
    if (p->encoder_buffer_size < len) {
      p->encoder_buffer_size = len;
      free(p->encoder_buffer);
      p->encoder_buffer = lsx_malloc(p->encoder_buffer_size*sizeof(INT_PCM));  // encoder buffer is INT_PCM*
    }

    /* convert sox sample to signed 16bit */
    for (i = 0; i < len; ++i) {
      SOX_SAMPLE_LOCALS;
      p->encoder_buffer[i] = SOX_SAMPLE_TO_SIGNED_16BIT(buf[i], ft->clips);
    }

    int samples_left = len;
    //int i_samples = len;

    //size_t nb_audio_channels = 2; // todo : unhardcode to handle mono / stereo


    // run and generate frames until all input buffer exhausted
    while(samples_left >= 0)
    {
        /* setup buffer desc */
        AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
        AACENC_InArgs in_args = { 0 };
        AACENC_OutArgs out_args = { 0 };
        int in_identifier = IN_AUDIO_DATA;
        int in_size, in_elem_size;
        int out_identifier = OUT_BITSTREAM_DATA;
        int out_size, out_elem_size;
        //int read;
        void *in_ptr, *out_ptr;
        //uint8_t outbuf[20480];
        AACENC_ERROR err;

        in_ptr = p->encoder_buffer + (len - (size_t)samples_left); // position buffer
        in_size = samples_left;
        in_elem_size = sizeof(INT_PCM); //16bit

        in_args.numInSamples = samples_left;
        in_buf.numBufs = 1;
        in_buf.bufs = &in_ptr;
        in_buf.bufferIdentifiers = &in_identifier;
        in_buf.bufSizes = &in_size;
        in_buf.bufElSizes = &in_elem_size;

        out_ptr = p->transport_buffer;
        out_size = p->transport_buffer_size;
        out_elem_size = 1;
        out_buf.numBufs = 1;
        out_buf.bufs = &out_ptr;
        out_buf.bufferIdentifiers = &out_identifier;
        out_buf.bufSizes = &out_size;
        out_buf.bufElSizes = &out_elem_size;

        if ((err = aacEncEncode(p->h_aacencoder, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK)
        {
            lsx_debug("aacEncEncode failed %d", err);

            // to check
            if (err == AACENC_ENCODE_EOF)
            {
                lsx_debug("got EOF");
                //    break;
            }
            return 0;
        }

        //lsx_debug("num out bytes %d out in samples %d", out_args.numOutBytes, out_args.numInSamples);

        // update samples_left
        samples_left -= out_args.numInSamples; // * channels ?

        // nothing is being written anynmore as aac stream
        if (out_args.numOutBytes <= 0)
        {
            //lsx_debug("breaking");
            break;
        }
        else {
            // write the bytes in the mp4 stream
            //MP4Duration dur = samples_left  frameSize ? frameSize : samples_left;
            //MP4Duration ofs = encoded_samples  0 ? 0 : delay_samples;
            MP4WriteSample(p->h_mp4file, p->audio_track_id, p->transport_buffer, out_args.numOutBytes, MP4_INVALID_DURATION, 0, true);

        }
    }


    //lsx_debug("samples left %d", samples_left);


    // to confirm,
    // this means all input samples have been pushed into the encoder
    return len;

}

static int m4a_stopwrite(sox_format_t * ft)
{

  // todo : clear transport buffer
  // todo : clear pcm buffer
  priv_t * p = (priv_t *)ft->priv;

  MP4Close(p->h_mp4file, 0);

  if (p->encoder_buffer) free (p->encoder_buffer);
  if (p->transport_buffer) free (p->transport_buffer);

  aacEncClose( &p->h_aacencoder );

  LSX_DLLIBRARY_CLOSE(p, fdkaac_dl);
  LSX_DLLIBRARY_CLOSE(p, mp4v2_dl);
  return SOX_SUCCESS;

}


#endif /*HAVE_M4A*/

LSX_FORMAT_HANDLER(m4a)
{
  static char const * const names[] = {"m4a", "audio/m4a", NULL};
  static unsigned const write_encodings[] = { SOX_ENCODING_AAC, 16, 0, 0};
  static sox_rate_t const write_rates[] = {
      8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 0};
  static sox_format_handler_t const handler = {SOX_LIB_VERSION_CODE,
    "M4A audio", names, 0,
    m4a_startread, m4a_read, m4a_stopread,
    m4a_startwrite, m4a_write, m4a_stopwrite,
    m4a_seek, write_encodings, write_rates, sizeof(priv_t)
  };
  return &handler;
}











