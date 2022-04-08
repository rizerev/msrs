#include <linux/slab.h>

#include "msr_file.h"
#include "util.h"

#define BUFFER_INC_SIZE 4096
#define BUFFER_SIZE_TO_BYTES(x) ((x) * sizeof(struct typed_msr))

void msr_file_reset(struct msr_file *file) {
    kfree(file->buffer_);
    file->buffer_ = 0;
    file->next_element_ = 0;
    file->buffer_size_ = 0;
}

static bool msr_file_add_msr_unsafe(struct msr_file *file, struct typed_msr *msr) {
    struct typed_msr *tmp;

    if (file->next_element_ >= (file->buffer_ + file->buffer_size_)) {
        tmp = krealloc(file->buffer_, BUFFER_SIZE_TO_BYTES(file->buffer_size_ + BUFFER_INC_SIZE), GFP_KERNEL);
        if (tmp == 0) {
            printk(OUT_PROMPT "Could not increase msr_file size!\n");
            mutex_unlock(&file->lock_);
            return false;
        }
        file->next_element_ = tmp + (file->next_element_ - file->buffer_);
        file->buffer_ = tmp;
        file->buffer_size_ += BUFFER_INC_SIZE;
    }
    *(file->next_element_) = *msr;
    (file->next_element_)++;

    return true;
}

bool msr_file_add_msrs(struct msr_file *file, struct typed_msr *msrs, int count) {
    int x;

    mutex_lock(&file->lock_);
    for (x = 0; x < count; x++) {
        if (!msr_file_add_msr_unsafe(file, (msrs + x))) {
            mutex_unlock(&file->lock_);
            return false;
        }
    }
    mutex_unlock(&file->lock_);

    return true;
}
