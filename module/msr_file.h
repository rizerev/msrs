#ifndef MSR_FILE_H
#define MSR_FILE_H

#include <linux/mutex.h>

#include "types.h"

#define DEFINE_MSR_FILE(x) struct msr_file (x) = {\
    .buffer_ = 0,                             \
    .next_element_ = 0,                       \
    .buffer_size_ = 0,                        \
    .lock_ = __MUTEX_INITIALIZER(x.lock_)     \
}

struct msr_file {
    struct typed_msr *buffer_;
    struct typed_msr *next_element_;
    size_t buffer_size_;
    struct mutex lock_;
};

void msr_file_reset(struct msr_file *file);

bool msr_file_add_msrs(struct msr_file *file, struct typed_msr *msrs, int count);

#endif //MSR_FILE_H
