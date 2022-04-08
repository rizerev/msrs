//
// Created by martin on 12/17/19.
//
#include <stdexcept>
#include <sstream>
namespace lnx {
#include <unistd.h>
#include <fcntl.h>
}

#include "msr_cpu_file.h"

MsrCpuFile::MsrCpuFile(fs::path &path, cpuid_t cpuid) : cpuid_{cpuid}, path_{path} {
  fd_ = lnx::open(path.c_str(), O_RDONLY);
  if (fd_ == -1) {
    throw std::runtime_error("Could not open " + path.string());
  }
}

MsrCpuFile::~MsrCpuFile() {
  lnx::close(fd_);
}

msr_value_t MsrCpuFile::sampleOnce(msr_address_t msr) {
  msr_value_t value;
  if (lnx::pread(fd_, &value, sizeof(value), msr) == -1) {
    std::stringstream ss;
    ss << "Could not read MSR " << std::hex << msr << " from " << path_.string();
    throw std::runtime_error(ss.str());
  }
  return value;
}

cpuid_t MsrCpuFile::getCpuId() const {
  return cpuid_;
}

std::ostream &operator<<(std::ostream &out, const MsrCpuFile &mcf) {
  return out << mcf.getCpuId();
}

std::ostream &operator<<(std::ostream &out, const std::unique_ptr<MsrCpuFile> &mcf) {
  return out << *mcf;
}