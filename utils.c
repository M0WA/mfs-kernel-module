#include "utils.h"

void mfs_timet_to_timespec(uint64_t t, struct timespec64* ts) {
    ts->tv_sec = t;
    ts->tv_nsec = 0;
}