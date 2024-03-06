#include "zstream.hpp"

#include <iostream>
#include <sstream>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  std::ostringstream com_data_stream;
  {
    zstream compressor(&com_data_stream);
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
    compressor << "Hello World!\n";
  }

  std::string com_data = com_data_stream.str();
  std::cout << "Compressed Str: " << com_data
            << " Length: " << com_data.length() << std::endl;

  std::istringstream decom_data_stream(com_data);
  std::string decom_data, line;
  zstream decompressor(&decom_data_stream);
  while (std::getline(decompressor, line))
    decom_data += line;
  std::cout << "Decompressed Str: " << decom_data
            << " Length: " << decom_data.length() << std::endl;
  return 0;
}
