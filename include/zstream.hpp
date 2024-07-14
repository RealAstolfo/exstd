#ifndef EXSTD_ZSTREAM
#define EXSTD_ZSTREAM

/**
 * @file zstream.hpp
 * @brief Header file for zlib-based stream buffer.
 */

#include <cstddef>
#include <cstring>
#include <iostream>
#include <istream>
#include <ostream>
#include <streambuf>
#include <sys/types.h>
#include <vector>

#include <zconf.h>
#include <zlib.h>

/**
 * @brief A stream buffer for zlib compression and decompression.
 */
class zstream_buffer : public std::streambuf {
public:
  /**
   * @brief Constructor for compressing streams.
   * @param sink The output stream to write compressed data to.
   * @param buff_sz The size of the buffer to use.
   */
  explicit zstream_buffer(std::ostream *sink, std::size_t buff_sz = 1024)
      : sink_stream(sink), is_compressing(true), buffer(buff_sz) {
    init_z_stream();
  }

  /**
   * @brief Constructor for decompressing streams.
   * @param source The input stream to read compressed data from.
   * @param buff_sz The size of the buffer to use.
   */
  explicit zstream_buffer(std::istream *source, std::size_t buff_sz = 1024)
      : source_stream(source), is_compressing(false), buffer(buff_sz) {
    init_z_stream();
  }

  /**
   * @brief Destructor. Cleans up the zlib stream.
   */
  ~zstream_buffer() override {
    if (is_compressing)
      deflateEnd(&z_stream_def);
    else
      inflateEnd(&z_stream_def);
  }

protected:
  /**
   * @brief Handles input buffer underflow for decompression.
   * @return The next available character from the decompressed stream.
   */
  int_type underflow() override {
    if (!source_stream || source_stream->eof())
      return traits_type::eof();

    source_stream->read(buffer.data(), buffer.size());
    std::streamsize read_bytes = source_stream->gcount();

    if (read_bytes <= 0)
      return traits_type::eof();

    z_stream_def.avail_in = static_cast<uInt>(read_bytes);
    z_stream_def.next_in = reinterpret_cast<Bytef *>(buffer.data());

    std::vector<char> out_buffer(buffer.size() * 2);
    z_stream_def.avail_out = static_cast<uInt>(out_buffer.size());
    z_stream_def.next_out = reinterpret_cast<Bytef *>(out_buffer.data());

    while (z_stream_def.avail_in > 0) {
      int ret = inflate(&z_stream_def, Z_NO_FLUSH);
      if (ret != Z_OK && ret != Z_STREAM_END) {
        std::cerr << "Decompression failed with error code: " << ret;
        if (ret == Z_DATA_ERROR) {
          std::cerr << " (Z_DATA_ERROR, possibly incorrect data or compression "
                       "method)";
        } else if (ret == Z_MEM_ERROR) {
          std::cerr << " (Z_MEM_ERROR, insufficient memory)";
        }
        std::cerr << std::endl;
        return traits_type::eof();
      }

      if (z_stream_def.avail_out == 0) {
        // Increase buffer size and continue decompression
        std::streamsize current_size = out_buffer.size();
        out_buffer.resize(current_size * 2);
        z_stream_def.avail_out = static_cast<uInt>(current_size);
        z_stream_def.next_out =
            reinterpret_cast<Bytef *>(out_buffer.data() + current_size);
      }
    }

    buffer = std::move(out_buffer);
    setg(buffer.data(), buffer.data(),
         buffer.data() + (buffer.size() - z_stream_def.avail_out));

    return traits_type::to_int_type(*gptr());
  }

  /**
   * @brief Handles output buffer overflow for compression.
   * @param ch The character to write.
   * @return The written character, or EOF on failure.
   */
  int_type overflow(int_type ch = traits_type::eof()) override {
    // Check if the buffer is full
    if (pptr() == epptr()) {
      if (!flush_buffer())
        return traits_type::eof();
    }

    // If ch is not EOF, append it to the buffer
    if (ch != traits_type::eof()) {
      *pptr() = traits_type::to_char_type(ch);
      pbump(1);
    }

    return ch;
  }

  /**
   * @brief Synchronizes the buffer by flushing it.
   * @return 0 on success, -1 on failure.
   */
  int sync() override { return flush_buffer() ? 0 : -1; }

private:
  /**
   * @brief Initializes the zlib stream.
   */
  void init_z_stream() {
    memset(&z_stream_def, 0, sizeof(z_stream));
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

  /**
   * @brief Flushes the buffer, compressing its contents and writing to the
   * output stream.
   * @return True on success, false on failure.
   */
  bool flush_buffer() {
    // Prepare the input for compression
    z_stream_def.avail_in = static_cast<uInt>(pptr() - pbase());
    z_stream_def.next_in = reinterpret_cast<Bytef *>(buffer.data());

    // Dynamically resize the output buffer if necessary
    std::vector<char> out_buffer(buffer.size() * 2); // Increase the buffer size

    do {
      z_stream_def.avail_out = static_cast<uInt>(out_buffer.size());
      z_stream_def.next_out = reinterpret_cast<Bytef *>(out_buffer.data());

      // Compress the data
      int ret = deflate(&z_stream_def, Z_SYNC_FLUSH);
      if (ret != Z_OK) {
        return false; // Handle compression error
      }
    } while (z_stream_def.avail_out ==
             0); // Repeat if output buffer was too small

    // Write compressed data to sink stream
    std::streamsize write_size = out_buffer.size() - z_stream_def.avail_out;
    sink_stream->write(out_buffer.data(), write_size);

    // Reset the buffer pointers
    setp(buffer.data(), buffer.data() + buffer.size() - 1);
    pbump(0);

    return true;
  }

  // only one stream can be assigned anyway, so lets save some memory.
  union {
    std::ostream *sink_stream;
    std::istream *source_stream;
  };

  bool is_compressing; ///< Flag indicating whether the buffer is compressing or
                       ///< decompressing.
  std::vector<char> buffer; ///< The buffer for holding data.
  z_stream z_stream_def;    ///< The zlib stream structure.
};

/**
 * @brief A stream class for handling zlib-compressed streams.
 */
class zstream : public std::iostream {
public:
  /**
   * @brief Constructor for compressing streams.
   * @param sink The output stream to write compressed data to.
   */
  zstream(std::ostream *sink) : std::iostream(&buffer), buffer(sink) {
    init(&buffer);
  }

  /**
   * @brief Constructor for decompressing streams.
   * @param source The input stream to read compressed data from.
   */
  zstream(std::istream *source) : std::iostream(&buffer), buffer(source) {
    init(&buffer);
  }

  /**
   * @brief Destructor. Flushes the buffer.
   */
  ~zstream() { flush(); }

  /**
   * @brief Flushes the buffer.
   */
  void flush() { buffer.pubsync(); }

private:
  zstream_buffer buffer; ///< The zlib stream buffer.
};

#endif // EXSTD_ZSTREAM
