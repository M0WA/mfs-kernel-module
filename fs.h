#pragma once

#define MFS_MAJOR_VERSION  (uint32_t)1
#define MFS_MINOR_VERSION  (uint32_t)1
#define MFS_VERSION        ((uint64_t)( ( (uint64_t)MFS_MAJOR_VERSION >> 32 ) & (uint64_t)MFS_MINOR_VERSION ))

#define MFS_GET_MAJOR_VERSION(x) ((uint32_t)( ((uint64_t)(x)) << 32 ))
#define MFS_GET_MINOR_VERSION(x) ((uint32_t)( ((uint64_t)(x)) & 0x0000ffff ))