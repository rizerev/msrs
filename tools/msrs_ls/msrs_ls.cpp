//
// Created by martin on 12/11/19.
//

#include <iostream>
#include <array>
#include <string_view>

#include <fcntl.h>
#include <unistd.h>

#include "types.h"

int main(int argc, char **argv) {
  bool printType = false;

  if (argc > 1) {
    for (int x = 0; x < argc; x++) {
      std::string_view arg{*(argv + x)};
      if (arg == "-t" || arg == "--print-type") {
        printType = true;
      }
    }
  }

  const int buffer_size = 1024;
  std::array <TypedMsr, buffer_size> buffer{};
  off64_t offset = 0;
  ssize_t res;

  int file = open("/dev/msrs", O_RDONLY);

  if (file < 0) {
    std::cerr << "ERROR! Could not open device! Is the module loaded?\n";
    return 1;
  }

  int count = 0;

  std::cout << std::hex;
  while (true) {
    res = pread(file,
                buffer.data(),
                buffer_size * static_cast<long>(sizeof(TypedMsr)),
                offset * static_cast<long>(sizeof(TypedMsr)));

    if (res <= 0) {
      break;
    }

    res /= sizeof(TypedMsr);
    auto end = buffer.begin() + res;

    for (auto cur = buffer.begin(); cur != end; cur++, count++) {
      std::cout << cur->address_;
      if (printType) {
        std::cout << ',';
        for (size_t idx = 0; idx < sizeof(cur->type_) * 8; idx++) {
          if (cur->type_ & (1 << idx)) {
            try {
              std::cout << MSR_TYPES_STR.at(idx);
            } catch (std::out_of_range &) {
              std::cout << '?';
            }
          }
        }
      }
      std::cout << '\n';
    }

    if (res < buffer_size) {
      break;
    }

    offset += buffer_size;

  }

  std::cout << std::dec;

  return 0;
}