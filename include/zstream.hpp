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
  zstream_buffer(std::istream &is, std::ostream &os)
      : in_stream(is), out_stream(os) {
    in_buffer.resize(buffer_size);
    char *base = &in_buffer[0];
    setg(base, base, base);

    out_buffer.resize(buffer_size);
    char *outbase = &out_buffer[0];
    setp(outbase, outbase + out_buffer.size() - 1);

    zstream_in.zalloc = Z_NULL;
    zstream_in.zfree = Z_NULL;
    zstream_in.opaque = Z_NULL;
    inflateInit(&zstream_in);

    zstream_out.zalloc = Z_NULL;
    zstream_out.zfree = Z_NULL;
    zstream_out.opaque = Z_NULL;
    deflateInit(&zstream_out, Z_BEST_COMPRESSION);
  }

  ~zstream_buffer() {
    inflateEnd(&zstream_in);
    deflateEnd(&zstream_out);
  }

protected:
  int_type underflow() override {
    if (gptr() < egptr())
      return traits_type::to_int_type(*gptr());

    in_stream.read(&in_buffer[0], in_buffer.size());
    std::streamsize num = in_stream.gcount();
    if (num <= 0)
      return traits_type::eof();

    zstream_in.avail_in = num;
    zstream_in.next_in = reinterpret_cast<Bytef *>(&in_buffer[0]);
    zstream_in.avail_out = buffer_size;
    zstream_in.next_out = reinterpret_cast<Bytef *>(&out_buffer[0]);

    inflate(&zstream_in, Z_NO_FLUSH);
    return traits_type::to_int_type(*gptr());
  }

  int_type overflow(int_type ch) override {
    if (ch != traits_type::eof()) {
      *pptr() = ch;
      pbump(1);
    }

    if (flush_buffer() == traits_type::eof()) {
      return traits_type::eof();
    }
    return ch;
  }

  int sync() override {
    flush_buffer();
    return 0;
  }

private:
  int flush_buffer() {
    zstream_out.avail_in = static_cast<uInt>(pptr() - pbase());
    zstream_out.next_in = reinterpret_cast<Bytef *>(pbase());

    do {
      zstream_out.avail_out = buffer_size;
      zstream_out.next_out = reinterpret_cast<Bytef *>(&out_buffer[0]);
      deflate(&zstream_out, Z_NO_FLUSH);

      int have = buffer_size - zstream_out.avail_out;
      if (have > 0) {
        out_stream.write(&out_buffer[0], have);
      }
    } while (zstream_out.avail_out == 0);
    setp(&out_buffer[0], &out_buffer[0] + buffer_size - 1);
    return 0;
  }

  static const size_t buffer_size = 1024;
  std::vector<char> out_buffer;
  std::vector<char> in_buffer;
  z_stream zstream_in;
  z_stream zstream_out;
  std::istream &in_stream;
  std::ostream &out_stream;
};

class zstream : public std::iostream {
public:
  zstream(std::istream &is, std::ostream &os)
      : std::iostream(&zbuf), zbuf(is, os) {}

private:
  zstream_buffer zbuf;
};

#endif
