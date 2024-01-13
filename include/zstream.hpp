#ifndef EXSTD_ZSTREAM
#define EXSTD_ZSTREAM

#include <cstddef>
#include <iostream>
#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <sys/types.h>
#include <vector>

#include <zconf.h>
#include <zlib.h>

class zstream_buffer : public std::streambuf {
public:
  zstream_buffer(std::ostream *sink, std::size_t buff_sz = 256)
      : sink_stream(sink), is_compressing(true), buffer(buff_sz) {
    z_stream_def.zalloc = Z_NULL;
    z_stream_def.zfree = Z_NULL;
    z_stream_def.opaque = Z_NULL;
    deflateInit(&z_stream_def, Z_BEST_COMPRESSION);
    setp(buffer.data(), buffer.data() + buffer.size() - 1);
  }

  zstream_buffer(std::istream *source, std::size_t buff_sz = 256)
      : source_stream(source), is_compressing(false), buffer(buff_sz) {
    z_stream_def.zalloc = Z_NULL;
    z_stream_def.zfree = Z_NULL;
    z_stream_def.opaque = Z_NULL;
    inflateInit(&z_stream_def);
    setg(buffer.data(), buffer.data(), buffer.data());
  }

  ~zstream_buffer() override {
    if (is_compressing)
      deflateEnd(&z_stream_def);
    else
      inflateEnd(&z_stream_def);
  }

protected:
  int_type underflow() override {
    if (!is_compressing) {
      if (source_stream->eof() || source_stream->fail()) {
        std::cerr << "Source stream is not in a good state." << std::endl;
        return traits_type::eof();
      }

      source_stream->read(buffer.data(), buffer.size());
      std::streamsize num = source_stream->gcount();
      if (num <= 0) {
        std::cerr << "No data read from source stream." << std::endl;
        return traits_type::eof();
      }

      z_stream_def.avail_in = num;
      z_stream_def.next_in = reinterpret_cast<Bytef *>(buffer.data());
      z_stream_def.avail_out = buffer.size();
      z_stream_def.next_out = reinterpret_cast<Bytef *>(buffer.data());

      int ret = inflate(&z_stream_def, Z_NO_FLUSH);
      if (ret != Z_OK && ret != Z_STREAM_END) {
        std::cerr << "Decompression error: " << ret << std::endl;
        return traits_type::eof();
      }

      setg(buffer.data(), buffer.data(),
           buffer.data() + (buffer.size() - z_stream_def.avail_out));
      return traits_type::to_int_type(*gptr());
    }

    return traits_type::eof();
  }

  int_type overflow(int_type ch = traits_type::eof()) override {
    if (is_compressing) {
      if (ch != traits_type::eof()) {
        *pptr() = ch;
        pbump(1);
      }

      compress_and_write(false);
      return ch;
    }

    return traits_type::eof();
  }

  int sync() override {
    if (is_compressing)
      compress_and_write(true);
    return 0;
  }

private:
  void compress_and_write(bool finish) {
    char *base = buffer.data();
    z_stream_def.avail_in = static_cast<uInt>(pptr() - base);
    z_stream_def.next_in = reinterpret_cast<Byte *>(base);
    int flush = finish ? Z_FINISH : Z_NO_FLUSH;
    do {
      z_stream_def.avail_out = buffer.size();
      z_stream_def.next_out = reinterpret_cast<Bytef *>(base);
      int ret = deflate(&z_stream_def, flush);
      if (ret == Z_STREAM_ERROR) {
        std::cerr << "ZSTREAM ERROR: " << ret << std::endl;
      }

      std::size_t have = buffer.size() - z_stream_def.avail_out;
      if (have > 0)
        sink_stream->write(base, have);
    } while (z_stream_def.avail_out == 0);

    setp(buffer.data(), buffer.data() + buffer.size() - 1);
  }

  std::ostream *sink_stream;
  std::istream *source_stream;
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
