#ifndef EXSTD_ZSTREAM
#define EXSTD_ZSTREAM

#include <cstddef>
#include <iostream>
#include <istream>
#include <ostream>
#include <streambuf>
#include <sys/types.h>
#include <vector>

#include <zconf.h>
#include <zlib.h>

class zstream_buffer : public std::streambuf {
public:
  explicit zstream_buffer(std::ostream *sink, std::size_t buff_sz = 256)
      : sink_stream(sink), is_compressing(true), buffer(buff_sz) {
    init_z_stream();
  }

  explicit zstream_buffer(std::istream *source, std::size_t buff_sz = 256)
      : source_stream(source), is_compressing(false), buffer(buff_sz) {
    init_z_stream();
  }

  ~zstream_buffer() override {
    if (is_compressing)
      deflateEnd(&z_stream_def);
    else
      inflateEnd(&z_stream_def);
  }

protected:
  int_type underflow() override {
    if (is_compressing)
      return traits_type::eof();

    z_stream_def.avail_in = 0;
    z_stream_def.next_in = reinterpret_cast<Bytef *>(buffer.data());

    source_stream->read(buffer.data(), buffer.size());
    z_stream_def.avail_in = source_stream->gcount();

    if (z_stream_def.avail_in == 0)
      return traits_type::eof();

    z_stream_def.next_in = reinterpret_cast<Bytef *>(buffer.data());

    std::vector<char> out_buffer(buffer.size());
    z_stream_def.avail_out = out_buffer.size();
    z_stream_def.next_out = reinterpret_cast<Bytef *>(out_buffer.data());

    int ret = inflate(&z_stream_def, Z_NO_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
      std::cerr << "Decompression error: " << ret << std::endl;
      return traits_type::eof();
    }

    setg(out_buffer.data(), out_buffer.data(),
         out_buffer.data() + out_buffer.size() - z_stream_def.avail_out);
    return traits_type::to_int_type(*gptr());
  }

  int_type overflow(int_type ch = traits_type::eof()) override {
    if (!is_compressing)
      return traits_type::eof();

    if (ch != traits_type::eof()) {
      *pptr() = ch;
      pbump(1);
    }

    return flush_buffer() ? ch : traits_type::eof();
  }

  int sync() override { return flush_buffer() ? 0 : -1; }

private:
  void init_z_stream() {
    z_stream_def.zalloc = Z_NULL;
    z_stream_def.zfree = Z_NULL;
    z_stream_def.opaque = Z_NULL;
    if (is_compressing) {
      deflateInit(&z_stream_def, Z_BEST_COMPRESSION);
      setp(buffer.data(), buffer.data() + buffer.size() - 1);
    } else {
      inflateInit(&z_stream_def);
      setg(buffer.data(), buffer.data(), buffer.data());
    }
  }

  bool flush_buffer() {
    z_stream_def.avail_in = pptr() - pbase();
    z_stream_def.next_in = reinterpret_cast<Bytef *>(buffer.data());

    std::vector<char> out_buffer(buffer.size());
    z_stream_def.avail_out = out_buffer.size();
    z_stream_def.next_out = reinterpret_cast<Bytef *>(out_buffer.data());

    int ret = deflate(&z_stream_def, Z_SYNC_FLUSH);
    if (ret != Z_OK) {
      std::cerr << "ZSTREAM ERROR: " << ret << std::endl;
      return false;
    }

    if (out_buffer.size() != z_stream_def.avail_out) {
      sink_stream->write(out_buffer.data(),
                         out_buffer.size() - z_stream_def.avail_out);
    }

    setp(buffer.data(), buffer.data() + buffer.size() - 1);
    return true;
  }

  // only one stream can be assigned anyway, so lets save some memory.
  union {
    std::ostream *sink_stream;
    std::istream *source_stream;
  };

  bool is_compressing;
  std::vector<char> buffer;
  z_stream z_stream_def;
};

class zstream : public std::iostream {
public:
  zstream(std::ostream *sink) : std::iostream(&buffer), buffer(sink) {
    init(&buffer);
  }

  zstream(std::istream *source) : std::iostream(&buffer), buffer(source) {
    init(&buffer);
  }

  void flush() { buffer.pubsync(); }

private:
  zstream_buffer buffer;
};

#endif
