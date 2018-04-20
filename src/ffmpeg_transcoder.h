/*
 * FFmpeg decoder class header for ffmpegfs
 *
 * Copyright (C) 2017-2018 Norbert Schlia (nschlia@oblivion-software.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef FFMPEG_TRANSCODER_H
#define FFMPEG_TRANSCODER_H

#pragma once

#include "ffmpeg_base.h"
#include "id3v1tag.h"
#include "wave.h"

#include <queue>

using namespace std;

class Buffer;

class FFMPEG_Transcoder : public FFMPEG_Base
{
public:
    FFMPEG_Transcoder();
    ~FFMPEG_Transcoder();
    bool is_open() const;
    int open_input_file(const char* filename);
    int open_output_file(Buffer* buffer);
    int process_single_fr(int & status);
    int encode_finish();
    void close();

    time_t mtime() const;
    size_t calculate_size();

    const ID3v1 * id3v1tag() const;

    const char *filename() const;
    const char *destname() const;

    static const string & getDestname(string *destname, const string & filename);

protected:
    bool is_video() const;
    void limitVideoSize(AVCodecContext *output_codec_ctx);
    int add_stream(AVCodecID codec_id);
    int open_output_filestreams(Buffer *buffer);
    void copy_metadata(AVDictionary **metadata_out, const AVDictionary *metadata_in);
    int process_metadata();
    int init_resampler();
    int init_fifo();
    int write_output_file_header();
    int decode_audio_frame(AVPacket *pkt, int *decoded);
    int decode_video_frame(AVPacket *pkt, int *decoded);
    int decode_frame(AVPacket *pkt);
    int init_converted_samples(uint8_t ***converted_input_samples, int frame_size);
    int convert_samples(uint8_t **input_data, const int in_samples, uint8_t **converted_data, int *out_samples);
    int add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size);
    int flush_input_frames(int stream_index);
    int read_decode_convert_and_store(int *finished);
    int init_audio_output_frame(AVFrame **frame, int frame_size);
    AVFrame *alloc_picture(AVPixelFormat pix_fmt, int width, int height);
    void produce_audio_dts(AVPacket * pkt, int64_t *pts);
#if LAVC_NEW_PACKET_INTERFACE
    int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt) const;
#endif
    int encode_audio_frame(AVFrame *frame, int *data_present);
    int encode_video_frame(AVFrame *frame, int *data_present);
    int load_encode_and_write(int frame_size);
    int write_output_file_trailer();

    static int write_packet(void * pOpaque, unsigned char * pBuffer, int nBufSize);
    static int64_t seek(void * pOpaque, int64_t i4Offset, int nWhence);

    bool get_output_sample_rate(AVStream *stream, int max_sample_rate, int *sample_rate) const;
#if !defined(USING_LIBAV) && (LIBAVUTIL_VERSION_MAJOR > 54)
    bool get_output_bit_rate(AVStream *stream, int64_t max_bit_rate, int64_t * bit_rate) const;
#else // USING_LIBAV
    bool get_output_bit_rate(AVStream *stream, int max_bit_rate, int * bit_rate) const;
#endif
    double get_aspect_ratio(int width, int height, const AVRational & sample_aspect_ratio);

    int av_dict_set_with_check(AVDictionary **pm, const char *key, const char *value, int flags, const char *fileName);

private:
    time_t                      m_mtime;
    size_t                      m_nCalculated_size;         // Use this as the size instead of computing it.
    bool                        m_bIsVideo;

    // Audio conversion and buffering
#if LAVR_DEPRECATE
    SwrContext *                m_pAudio_resample_ctx;
#else
    AVAudioResampleContext *    m_pAudio_resample_ctx;
#endif
    AVAudioFifo *               m_pAudioFifo;

    // Video conversion and buffering
    SwsContext *                m_pSws_ctx;
    //    AVFilterContext *           m_pBufferSinkContext;
    //    AVFilterContext *           m_pBufferSourceContext;
    //    AVFilterGraph *             m_pFilterGraph;
    queue<AVFrame*>             m_VideoFifo;
    int64_t                     m_pts;
    int64_t                     m_pos;

    // Input file
    struct _tagINPUTFILE
    {
        AVFormatContext *       m_pFormat_ctx;
        AVCodecContext *        m_pAudio_codec_ctx;
        AVCodecContext *        m_pVideo_codec_ctx;
        AVStream *              m_pAudio_stream;
        AVStream *              m_pVideo_stream;
        int                     m_nAudio_stream_idx;
        int                     m_nVideo_stream_idx;
        string                  m_filename;
    } m_in;

    // Output file
    struct _tagOUTPUTFILE
    {
        OUTPUTTYPE              m_output_type;

        AVFormatContext *       m_pFormat_ctx;
        AVCodecContext *        m_pAudio_codec_ctx;
        AVCodecContext *        m_pVideo_codec_ctx;
        AVStream *              m_pAudio_stream;
        AVStream *              m_pVideo_stream;
        int                     m_nAudio_stream_idx;
        int                     m_nVideo_stream_idx;
        string                  m_filename;

        int64_t                 m_nAudio_pts;           // Global timestamp for the audio frames
        int64_t                 m_video_start_pts;      // Video start PTS

        int64_t                 m_last_mux_dts;         // Last muxed DTS

        ID3v1                   m_id3v1;
    } m_out;

};

#endif // FFMPEG_TRANSCODER_H
