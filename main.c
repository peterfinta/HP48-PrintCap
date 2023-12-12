/* Print capture application for the HP48(G) */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include "bitmap.h"

int main(int argc, char *argv[])
{
  char *port = argv[1];
  if(argc == 1)
  {
    printf("No COM port was sepcified");
    return 1;
  }

  // Open virtrual COM-Port
  HANDLE hPort = CreateFile(port
                           ,GENERIC_READ
                           ,0
                           ,NULL
                           ,OPEN_EXISTING
                           ,0
                           ,NULL);
  
  if(hPort == INVALID_HANDLE_VALUE)
  {
    printf("COM port cannot be opened!\n");
    return 1;
  }

  // Config port settings
  DCB dcb;
  GetCommState(hPort, &dcb);
  dcb.BaudRate = CBR_9600;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  SetCommState(hPort, &dcb);

  // Read data
  char buffer[2048];
  DWORD bytesRead = 0;
  DWORD bytesReadTotal = 0;
  DWORD timer = 1000000;
  DWORD signal = 0;
  GetAsyncKeyState(VK_ESCAPE);
  printf("Listening to %s, press ESC to interrupt\n", port);
  for(; timer > 0; timer--, signal = signal | bytesRead)
  {
    if(!signal || bytesRead) timer = 1000000;
    BOOL res = ReadFile(hPort, buffer + bytesReadTotal, 1, &bytesRead, NULL);
    if(!res)
    {
      printf("Cannot read data from COM port!\n");
      return 1;
    }
    if(GetAsyncKeyState(VK_ESCAPE))
    {
      printf("User interrupt\n"); goto exit;
    }
    if(bytesReadTotal >= 2048)
    {
      printf("Recived data is not a HP48 Graphic 131 x 64\n");
      goto exit;
    }
    bytesReadTotal += bytesRead;
  }
  printf("Bytes read: %d\n", bytesReadTotal);

  // Calculate filler Bytes
  DWORD filler = (bytesReadTotal - 131*8) / 8;

  // Validate data
  if(bytesReadTotal < 1080 && filler <= 6 && filler%2 != 1)
  {
    printf("Recived data is not a HP48 Graphic 131 x 64\n");
    goto exit;
  }

  // Convert raw data into 24Bit bitmap
  char *bitmap = malloc(3*131*64*sizeof(char));
  PIXEL24 *pixelArray = (PIXEL24*)bitmap;

  int i = 0;
  for(int x = 0; x < 131; x++)
    for(int y = 0; y < 64; y++)
    {
      i = x + filler + (y / 8) * (131 + filler);
      if(buffer[i] & (0x0001 << (y%8)))
      {
        pixelArray[x + 131*(63-y)].R = 0;
        pixelArray[x + 131*(63-y)].G = 0;
        pixelArray[x + 131*(63-y)].B = 0;
      }
      else
      {
        pixelArray[x + 131*(63-y)].R = 255;
        pixelArray[x + 131*(63-y)].G = 255;
        pixelArray[x + 131*(63-y)].B = 255;
      }
    }

  FILE *output = fopen("output.bmp", "wb");
  writeBMP(output, bitmap, 64, 131, 24);
  fclose(output);

  exit:
  CloseHandle(hPort);
  free(bitmap);

  return 0;
}
