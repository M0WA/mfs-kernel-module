#pragma once

#include "dir.h"
#include "file.h"

enum mfs_record_type {
    MFS_FILE_RECORD = 1,
    MFS_DIR_RECORD  = 2,
};

struct mfs_record {
    enum mfs_record_type type;
    union {
        struct mfs_dir_record dir;
        struct mfs_file_record file;
    };
};