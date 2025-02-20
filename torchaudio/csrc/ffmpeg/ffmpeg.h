// One stop header for all ffmepg needs
#pragma once
#include <torch/torch.h>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libavutil/pixdesc.h>
}

namespace torchaudio {
namespace ffmpeg {

using OptionDict = std::map<std::string, std::string>;

// Replacement of av_err2str, which causes
// `error: taking address of temporary array`
// https://github.com/joncampbell123/composite-video-simulator/issues/5
av_always_inline std::string av_err2string(int errnum) {
  char str[AV_ERROR_MAX_STRING_SIZE];
  return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

// Base structure that handles memory management.
// Resource is freed by the destructor of unique_ptr,
// which will call custom delete mechanism provided via Deleter
// https://stackoverflow.com/a/19054280
//
// The resource allocation will be provided by custom constructors.
template <typename T, typename Deleter>
class Wrapper {
 protected:
  std::unique_ptr<T, Deleter> ptr;

 public:
  Wrapper() = delete;
  Wrapper<T, Deleter>(T* t) : ptr(t){};
  T* operator->() const {
    return ptr.get();
  };
  explicit operator bool() const {
    return (bool)ptr;
  };
  operator T*() const {
    return ptr.get();
  }
};

////////////////////////////////////////////////////////////////////////////////
// AVFormatContext
////////////////////////////////////////////////////////////////////////////////
struct AVFormatContextDeleter {
  void operator()(AVFormatContext* p);
};

struct AVFormatContextPtr
    : public Wrapper<AVFormatContext, AVFormatContextDeleter> {
  explicit AVFormatContextPtr(AVFormatContext* p);
};

// create format context for reading media
AVFormatContextPtr get_input_format_context(
    const std::string& src,
    const c10::optional<std::string>& device,
    const OptionDict& option,
    AVIOContext* io_ctx = nullptr);

////////////////////////////////////////////////////////////////////////////////
// AVIO
////////////////////////////////////////////////////////////////////////////////
struct AVIOContextDeleter {
  void operator()(AVIOContext* p);
};

struct AVIOContextPtr : public Wrapper<AVIOContext, AVIOContextDeleter> {
  explicit AVIOContextPtr(AVIOContext* p);
};

////////////////////////////////////////////////////////////////////////////////
// AVPacket
////////////////////////////////////////////////////////////////////////////////
struct AVPacketDeleter {
  void operator()(AVPacket* p);
};

struct AVPacketPtr : public Wrapper<AVPacket, AVPacketDeleter> {
  AVPacketPtr();
};

////////////////////////////////////////////////////////////////////////////////
// AVPacket - buffer unref
////////////////////////////////////////////////////////////////////////////////
// AVPacket structure employs two-staged memory allocation.
// The first-stage is for allocating AVPacket object itself, and it typically
// happens only once throughout the lifetime of application.
// The second-stage is for allocating the content (media data) each time the
// input file is processed and a chunk of data is read. The memory allocated
// during this time has to be released before the next iteration.
// The first-stage memory management is handled by `AVPacketPtr`.
// `AutoPacketUnref` handles the second-stage memory management.
struct AutoPacketUnref {
  AVPacketPtr& p_;
  AutoPacketUnref(AVPacketPtr& p);
  ~AutoPacketUnref();
  operator AVPacket*() const;
};

////////////////////////////////////////////////////////////////////////////////
// AVFrame
////////////////////////////////////////////////////////////////////////////////
struct AVFrameDeleter {
  void operator()(AVFrame* p);
};

struct AVFramePtr : public Wrapper<AVFrame, AVFrameDeleter> {
  AVFramePtr();
};

////////////////////////////////////////////////////////////////////////////////
// AutoBufferUnrer is responsible for performing unref at the end of lifetime
// of AVBufferRefPtr.
////////////////////////////////////////////////////////////////////////////////
struct AutoBufferUnref {
  void operator()(AVBufferRef* p);
};

struct AVBufferRefPtr : public Wrapper<AVBufferRef, AutoBufferUnref> {
  AVBufferRefPtr();
  void reset(AVBufferRef* p);
};

////////////////////////////////////////////////////////////////////////////////
// AVCodecContext
////////////////////////////////////////////////////////////////////////////////
struct AVCodecContextDeleter {
  void operator()(AVCodecContext* p);
};
struct AVCodecContextPtr
    : public Wrapper<AVCodecContext, AVCodecContextDeleter> {
  explicit AVCodecContextPtr(AVCodecContext* p);
};

// Allocate codec context from either decoder name or ID
AVCodecContextPtr get_decode_context(
    enum AVCodecID codec_id,
    const c10::optional<std::string>& decoder);

// Initialize codec context with the parameters
void init_codec_context(
    AVCodecContext* pCodecContext,
    AVCodecParameters* pParams,
    const c10::optional<std::string>& decoder_name,
    const OptionDict& decoder_option,
    const torch::Device& device,
    AVBufferRefPtr& pHWBufferRef);

////////////////////////////////////////////////////////////////////////////////
// AVFilterGraph
////////////////////////////////////////////////////////////////////////////////
struct AVFilterGraphDeleter {
  void operator()(AVFilterGraph* p);
};
struct AVFilterGraphPtr : public Wrapper<AVFilterGraph, AVFilterGraphDeleter> {
  AVFilterGraphPtr();
  void reset();
};
} // namespace ffmpeg
} // namespace torchaudio
