#ifndef BITMAP_H_
#define BITMAP_H_

#include <stdio.h>

// Define Pixel structs
typedef struct 
{
  char B;
  char G;
  char R;
  char A;
} PIXEL32;

typedef struct
{
  char B;
  char G;
  char R;
} PIXEL24;

int readBMP(FILE     *file
           ,char     *bitmap
           ,unsigned *height
           ,unsigned *width
           ,char     *format);

int writeBMP(FILE     *file
            ,char     *bitmap
            ,unsigned  height
            ,unsigned  width
            ,char      format);

#endif
