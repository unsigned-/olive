/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2019 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef FFMPEGENCODER_H
#define FFMPEGENCODER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include "codec/encoder.h"

OLIVE_NAMESPACE_ENTER

class FFmpegEncoder : public Encoder
{
  Q_OBJECT
public:
  FFmpegEncoder(const EncodingParams &params);

public slots:
  virtual void WriteAudio(OLIVE_NAMESPACE::AudioRenderingParams pcm_info, const QString& pcm_filename, OLIVE_NAMESPACE::TimeRange range) override;

protected:
  virtual bool OpenInternal() override;
  virtual void WriteInternal(FramePtr frame) override;
  virtual void CloseInternal() override;

private:
  /**
   * @brief Handle an error
   *
   * Immediately closes the Decoder (freeing memory resources) and sends the string provided to the warning stream.
   * As this function closes the Decoder, no further Decoder functions should be performed after this is called
   * (unless the Decoder is opened again first).
   */
  void Error(const QString& s);

  /**
   * @brief Handle an FFmpeg error code
   *
   * Uses the FFmpeg API to retrieve a descriptive string for this error code and sends it to Error(). As such, this
   * function also automatically closes the Decoder.
   *
   * @param error_code
   */
  void FFmpegError(const char *context, int error_code);

  bool WriteAVFrame(AVFrame* frame, AVCodecContext *codec_ctx, AVStream *stream);

  bool InitializeStream(enum AVMediaType type, AVStream** stream, AVCodecContext** codec_ctx, const QString& codec);
  bool InitializeCodecContext(AVStream** stream, AVCodecContext** codec_ctx, AVCodec* codec);
  bool SetupCodecContext(AVStream *stream, AVCodecContext *codec_ctx, AVCodec *codec);

  void FlushEncoders();
  void FlushCodecCtx(AVCodecContext* codec_ctx, AVStream *stream);

  AVFormatContext* fmt_ctx_;

  AVStream* video_stream_;
  AVCodecContext* video_codec_ctx_;
  SwsContext* video_scale_ctx_;
  PixelFormat::Format video_conversion_fmt_;

  AVStream* audio_stream_;
  AVCodecContext* audio_codec_ctx_;
  SwrContext* audio_resample_ctx_;

};

OLIVE_NAMESPACE_EXIT

#endif // FFMPEGENCODER_H
