#pragma once

#define MFS_MAJOR_VERSION  (uint64_t)1
#define MFS_MINOR_VERSION  (uint64_t)1
#define MFS_VERSION        ((uint64_t)( ( (MFS_MAJOR_VERSION) << 32 ) | (MFS_MINOR_VERSION) ))

#define MFS_GET_MAJOR_VERSION(x) ( ((uint64_t)((x) & 0xffffffff00000000)) >> 32 )
#define MFS_GET_MINOR_VERSION(x) ( ((uint64_t)(x)) & 0x00000000ffffffff )