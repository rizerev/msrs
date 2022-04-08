//
// Created by martin on 12/17/19.
//

#ifndef TOOLSET_MSRCPUFILE_H
#define TOOLSET_MSRCPUFILE_H

#include <filesystem>
#include <ostream>

#include "types.h"

namespace fs = std::filesystem;

class MsrCpuFile {
  file_descriptor_t fd_;
  cpuid_t cpuid_;
  fs::path path_;

 public:
  MsrCpuFile(fs::path &path, cpuid_t cpuid);
  MsrCpuFile(const MsrCpuFile &orig) = delete;
  virtual ~MsrCpuFile();

  msr_value_t sampleOnce(msr_address_t msr);

  [[nodiscard]] cpuid_t getCpuId() const;
};

std::ostream &operator<<(std::ostream &out, const MsrCpuFile &mcf);
std::ostream &operator<<(std::ostream &out, const std::unique_ptr<MsrCpuFile> &mcf);

#endif //TOOLSET_MSRCPUFILE_H
