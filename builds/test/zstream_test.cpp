#include "zstream.hpp"
#include <bit>
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  std::ostringstream com_data_stream;
  {
    zstream compressor(&com_data_stream);
    compressor << "Hello World!";
    compressor.flush();
  }

  std::string com_data = com_data_stream.str();
  std::cerr << "Compressed Size: " << com_data.size() << std::endl;
  std::istringstream decom_data_stream(com_data);
  std::string decom_data, line;
  {
    zstream decompressor(&decom_data_stream);
    while (std::getline(decompressor, line))
      decom_data += line + '\n';
  }
  std::cout << decom_data << std::endl;
  return 0;
}
