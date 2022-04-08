//
// Created by martin on 12/17/19.
//

#ifndef TOOLSET_CMAKE_BUILD_DEBUG_MSRS_SAMPLE_TYPES_H_
#define TOOLSET_CMAKE_BUILD_DEBUG_MSRS_SAMPLE_TYPES_H_

#include <cstdint>

typedef int file_descriptor_t;

typedef std::uint32_t msr_address_t;
struct __attribute__((__packed__)) TypedMsr {
  msr_address_t address_;
  uint8_t type_;
};

typedef std::uint64_t msr_value_t;
typedef std::uint64_t cpuid_t;

static const uint8_t MSR_TYPE_READABLE = 1;
static const uint8_t MSR_TYPE_WRITEABLE = 2;

static constinit std::string_view MSR_TYPES_STR{"RW"};

#endif //TOOLSET_CMAKE_BUILD_DEBUG_MSRS_SAMPLE_TYPES_H_
