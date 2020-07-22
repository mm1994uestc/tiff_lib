#ifndef _TIFF_H_
#define _TIFF_H_
#include <stdio.h>

#define MC_GET_CHAR(__data__) __data__
#define MC_GET_SHORT(__data__) ((__data__ & 0xFF) << 8) | (__data__ >> 8)
#define MC_GET_LONG(__data__) ((__data__ & 0xFF) << 24) | ((__data__ & 0xFF00) << 8) | ((__data__ & 0xFF0000) >> 8) | ((__data__ & 0xFF000000) >> 24)

#define DE_N 11
#define GrayDE_N 14

#define uchar_szie sizeof(unsigned char)
#define short_size sizeof(short)
#define ulong_size sizeof(unsigned long)

typedef struct
{
    short endian; // 字节顺序标志位,值为II或者MM:II表示小字节在前,又称为little-endian,MM表示大字节在前,又称为big-endian
    short magic;  // TIFF的标志位，一般都是42
    unsigned long ifd_offset; // 第一个IFD的偏移量,可以在任意位置,但必须是在一个字的边界,也就是说必须是2的整数倍
}IFH;

typedef struct
{
    short tag;  // 此TAG的唯一标识
    short type; // 数据类型
    unsigned long size; // 数量,通过类型和数量可以确定存储此TAG的数据需要占据的字节数
    unsigned long val_offset;
}DE;

typedef struct
{
    short n; // 表示此IFD包含了多少个DE,假设数目为n
    DE *p; // n个DE
    unsigned long next_ifd_offset; // 下一个IFD的偏移量,如果没有则置为0
}IFD;

char mashine_endian(void);
char Tiff_read(unsigned char *Image,unsigned char *Path); // 以平铺platted方式来排布RGB数据，每一个像素点对应三个连续的数据(RGB)
char Tiff_write(unsigned char const *Image,unsigned int w,unsigned int h,unsigned char channel, unsigned char *Path);
#endif