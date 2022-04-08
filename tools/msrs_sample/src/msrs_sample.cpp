#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>
#include <memory>
#include <map>
#include <chrono>

#include "types.h"
#include "msr_cpu_file.h"

namespace fs = std::filesystem;

static const std::string_view msr_path = "/dev/cpu";
static const std::string_view msr_filename = "msr";

std::vector<std::unique_ptr<MsrCpuFile>> listMsrFiles() {
  std::vector<std::unique_ptr<MsrCpuFile>> msr_files;

  for (auto &d : fs::directory_iterator(msr_path)) {
    auto p = d.path() / msr_filename;
    if (d.is_directory() && fs::exists(p)) {
      std::stringstream ss(d.path().filename());
      cpuid_t cpuid;
      ss >> cpuid;
      msr_files.emplace_back(std::make_unique<MsrCpuFile>(p, cpuid));
    }
  }

  if (msr_files.empty()) {
    throw std::runtime_error("Could not find any msr files!");
  }

  return msr_files;
}

std::vector<msr_value_t> sampleMsr(const std::vector<std::unique_ptr<MsrCpuFile>> &msr_files, msr_address_t msr) {
  std::vector<msr_value_t> values;
  values.reserve(msr_files.size());

  for (auto &file : msr_files) {
    values.push_back(file->sampleOnce(msr));
  }

  return values;
}

std::vector<std::vector<msr_value_t>> sampleAllMsrs(const std::vector<std::unique_ptr<MsrCpuFile>> &msr_files,
                                                    const std::vector<msr_address_t> &msrs) {
  std::vector<std::vector<msr_value_t>> read;
  read.reserve(msrs.size());

  for (auto msr : msrs) {
    read.push_back(sampleMsr(msr_files, msr));
  }

  return read;
}

std::vector<std::vector<std::vector<msr_value_t>>> sample(const std::vector<std::unique_ptr<MsrCpuFile>> &msr_files,
                                                          const std::vector<msr_address_t> &msrs,
                                                          const unsigned int seconds) {
  std::vector<std::vector<std::vector<msr_value_t>>> reads;
  auto start = std::chrono::high_resolution_clock::now();

  while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count()
      < seconds) {
    reads.push_back(sampleAllMsrs(msr_files, msrs));
  }

  return reads;
}

unsigned int parseDurationArgument(const std::string &arg) {
  std::stringstream ss(arg);
  long time;

  if (!(ss >> time)) {
    throw std::runtime_error("Invalid duration argument: \"" + arg + "\"!");
  }

  if (time <= 0 || time > UINT32_MAX) {
    throw std::out_of_range("invalid time value");
  }

  return static_cast<unsigned int>(time);
}

std::vector<msr_address_t> parseMsrArguments(unsigned int argc, char **argv) {
  std::vector<msr_address_t> msrs;
  msrs.reserve(argc);
  bool error = false;
  for (unsigned int x = 0; x < argc; x++) {
    try {
      std::int64_t val = std::stol(*(argv + x), nullptr, 0);
      if (val < 0 || val > UINT32_MAX) {
        throw std::out_of_range("invalid value");
      }
      msrs.push_back(static_cast<unsigned int>(val));
    } catch (std::invalid_argument &) {
      error = true;
    } catch (std::out_of_range &) {
      error = true;
    }
    if (error) {
      throw std::runtime_error("Invalid msr argument: \"" + std::string{*(argv + x)} + "\"!");
    }
  }
  return msrs;
}

void printUsage(const std::string &name) {
  std::cerr << "Usage: " << name << " duration msr [msr]...\n";
  std::cerr << "\tduration\tspecifies how long the scan shall run (seconds, decimal)\n";
  std::cerr
      << "\tmsr\t\thexadecimal, decimal, or octal representation of an msr to scan (e.g. 0xe01, 3585, or 07001)\n";
}

void printResults(const std::vector<std::unique_ptr<MsrCpuFile>> &msr_files,
                  const std::vector<msr_address_t> &msrs,
                  const std::vector<std::vector<std::vector<msr_value_t>>> &results,
                  const long time_in_seconds) {
  const char cpu_separator = ',';
  const char msr_separator = ';';
  const char read_separator = '\n';

  const auto cout_flags = std::cout.flags();
  std::cout << std::hex;

  auto sepPrint = [](const char sep, const auto &vec) {
    if (vec.empty()) {
      return;
    }
    std::cout << vec.front();
    for (auto curr = std::next(std::begin(vec)); curr != std::end(vec); curr = std::next(curr)) {
      std::cout << sep << *curr;
    }
  };

  auto sepVecPrint = [&sepPrint](const char vsep, const char sep, const auto &vec) {
    if (vec.empty()) {
      return;
    }
    sepPrint(sep, vec.front());
    for (auto curr = std::next(std::begin(vec)); curr != std::end(vec); curr = std::next(curr)) {
      std::cout << vsep;
      sepPrint(sep, *curr);
    }
  };

  sepPrint(cpu_separator, msr_files);
  std::cout << read_separator;
  sepPrint(cpu_separator, msrs);
  std::cout << read_separator;
  std::cout << time_in_seconds << read_separator;

  for (const auto &read : results) {
    sepVecPrint(msr_separator, cpu_separator, read);
    std::cout << read_separator;
  }

  std::cout.flags(cout_flags);
}

int main(int argc, char **argv) {
  std::vector<std::unique_ptr<MsrCpuFile>> msr_files;
  std::vector<msr_address_t> msrs;
  unsigned int time;

  if (argc < 3) {
    printUsage(*argv);
    return 1;
  }

  try {
    time = parseDurationArgument(*(argv + 1));
    msrs = parseMsrArguments(static_cast<unsigned int>(argc - 2), argv + 2);
  } catch (std::runtime_error &ex) {
    std::cerr << "ERROR! " << ex.what() << '\n';
    printUsage(*(argv));
    return 1;
  }

  try {
    msr_files = listMsrFiles();
  } catch (std::runtime_error &ex) {
    std::cerr << "ERROR! " << ex.what() << '\n';
    std::cerr << "Is the msrs module loaded and are you root?\n";
    return 1;
  }

  try {
    auto result = sample(msr_files, msrs, time);
    printResults(msr_files, msrs, result, time);
  } catch (std::runtime_error &ex) {
    std::cerr << "ERROR! " << ex.what() << '\n';
    return 1;
  }

  return 0;
}