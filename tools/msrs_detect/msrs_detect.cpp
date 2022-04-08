//
// Created by martin on 12/29/19.
//

#include <iostream>
#include <array>
#include <climits>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../../module/msr_scan.h"

int main(int argc, char **argv) {
  unsigned long num_workers;
  int file = open(MSR_DEVICE_PATH, O_RDONLY);

  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " thread_count\n";
    std::cout << "A thread count too high will most likely freeze your machine.\n";
    std::cout << "Recommended thread_count: number of online non isolated cpus - 1\n";
    return 1;
  }

  if (file < 0) {
    std::cout << "ERROR! Could not open device! Is the module loaded?\n";
    return 1;
  }

  num_workers = strtoul(argv[1], nullptr, 0);
  if (errno == ERANGE || num_workers == 0 || num_workers >= UINT_MAX) {
    std::cout << "ERROR! Please specify a valid thread count {x | x in N, x < " << UINT_MAX << "}!\n";
    return 1;
  }

  std::cout << "Starting to scan...\n";
  int err = ioctl(file, MSR_IOCTL_CMD_TRIGGER_SCAN, (int) num_workers);

  if (err) {
    std::cout << "ERROR on scan!\n";
  }
  std::cout << "Done\n";

  close(file);

  return err;
}
