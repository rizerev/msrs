#ifndef MSR_TYPES_H
#define MSR_TYPES_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

static const uint8_t MSR_TYPE_READABLE = 1;
static const uint8_t MSR_TYPE_WRITEABLE = 2;

typedef uint32_t msr_address_t;

struct __attribute__((__packed__)) typed_msr {
    msr_address_t address_;
    uint8_t type_;
};

#endif // MSR_TYPES_H
