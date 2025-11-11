#ifndef GLOBALS_H
#define GLOBALS_H

typedef __INT8_TYPE__   i8;
typedef __INT16_TYPE__  i16;
typedef __INT32_TYPE__  i32;
typedef __INT64_TYPE__  i64;
typedef __UINT8_TYPE__  ui8;
typedef __UINT16_TYPE__ ui16;
typedef __UINT32_TYPE__ ui32;
typedef __UINT64_TYPE__ ui64;
typedef struct { 
    int x; int y;
} i2;
typedef struct { 
    int x; int y; int z;
} i3;
typedef struct { 
    int x; int y; int z; int w;
} i4;
typedef struct {
    float x; float y; 
} f2;
typedef struct {
    float x; float y; float z;
} f3;
typedef struct { 
    float x; float y; float z; float w;
} f4;
typedef unsigned char  uchar;
typedef unsigned short ushort;
#ifndef __cplusplus
typedef ui8 bool;
#endif

#define mmin(a,b) ((a)<(b)?(a):(b))
#define mmax(a,b) ((a)>(b)?(a):(b))
#define clamp(a,mi,ma) (mmax(mmin(a,ma),mi))

#endif
