#pragma once

#include <linux/time.h>

void mfs_timet_to_timespec(uint64_t t, struct timespec64* ts);