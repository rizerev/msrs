#include <linux/types.h>
#include <linux/workqueue.h>

#include "types.h"

#define SCAN_RANGE_SIZE 50000
#define SCAN_WQ_NAME "msr_scan_wq"

struct msr_range {
    msr_address_t start_; //incl.
    msr_address_t end_;  //incl.
};

extern struct msr_file result_file;

void setup_scan(msr_address_t range);
void scan_msrs(struct work_struct *work);
bool scan_msr_range(struct msr_range *range);
