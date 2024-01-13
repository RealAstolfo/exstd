#include "zstream.hpp"
#include <bit>
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  std::ofstream file_out("file.txt", std::ios::binary | std::ios::out);
  zstream zs_write(std::cin, file_out);

  std::cout << "Enter data to compress: ";
  std::string data;
  std::getline(std::cin, data);

  zs_write << data << std::flush; // force write all
  file_out.close();

  std::ifstream file_in("file.txt", std::ios::binary | std::ios::in);
  zstream zs_read(file_in, std::cout);
  std::string decom_data;
  std::getline(zs_read, decom_data);
  std::cout << decom_data << std::endl;
  file_in.close();

  std::cout << std::flush;
  return 0;
}
