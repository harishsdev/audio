#include <pybind11/stl.h>
#include <torch/extension.h>
#include <torchaudio/csrc/ffmpeg/pybind/stream_reader.h>

namespace torchaudio {
namespace ffmpeg {
namespace {

PYBIND11_MODULE(_torchaudio_ffmpeg, m) {
  py::class_<StreamReaderFileObj, c10::intrusive_ptr<StreamReaderFileObj>>(
      m, "StreamReaderFileObj")
      .def(py::init<
           py::object,
           const c10::optional<std::string>&,
           const c10::optional<OptionDict>&,
           int64_t>())
      .def("num_src_streams", &StreamReaderFileObj::num_src_streams)
      .def("num_out_streams", &StreamReaderFileObj::num_out_streams)
      .def(
          "find_best_audio_stream",
          &StreamReaderFileObj::find_best_audio_stream)
      .def(
          "find_best_video_stream",
          &StreamReaderFileObj::find_best_video_stream)
      .def("get_metadata", &StreamReaderFileObj::get_metadata)
      .def(
          "get_src_stream_info",
          &StreamReaderFileObj::get_src_stream_info_pybind)
      .def("get_out_stream_info", &StreamReaderFileObj::get_out_stream_info)
      .def("seek", &StreamReaderFileObj::seek)
      .def("add_audio_stream", &StreamReaderFileObj::add_audio_stream)
      .def("add_video_stream", &StreamReaderFileObj::add_video_stream)
      .def("remove_stream", &StreamReaderFileObj::remove_stream)
      .def("process_packet", &StreamReaderFileObj::process_packet)
      .def("process_all_packets", &StreamReaderFileObj::process_all_packets)
      .def("is_buffer_ready", &StreamReaderFileObj::is_buffer_ready)
      .def("pop_chunks", &StreamReaderFileObj::pop_chunks);
}

} // namespace
} // namespace ffmpeg
} // namespace torchaudio
