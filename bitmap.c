#include <stdio.h>
#include <stdlib.h>


// Read BMP File
int readBMP(FILE     *file
           ,char     *bitmap
           ,unsigned *height
           ,unsigned *width
           ,char     *format)
{
  // Read 14Byte BMP Header
  char     ID[2];   // File ID
  unsigned size;    // Total filesize
  unsigned offset;  // Size of the DBI Header

  fread(ID, 1, 2, file);
  fread(&size, 4, 1, file);
  fseek(file, 4, SEEK_CUR); // Move pointer over unused data (32Bit)
  fread(&offset, 4, 1, file);

  // Check if the file is a BMP
  if(ID[0] != 'B' || ID[1] != 'M')
  {
    printf("Error: Wrong file format!");
    return 1;
  }

  //printf("Offset: %u\n", offset);
  
  // Read DBI Header
  fseek(file, 4, SEEK_CUR);
  fread(width, 4, 1, file);
  fread(height, 4, 1, file);
  fseek(file, 2, SEEK_CUR);
  fread(format, 1, 1, file);

  // Move file pointer to Bitmap Data
  fseek(file, offset, SEEK_SET);

  // Convert format from Bits to Bytes
  unsigned formatByte = *format >> 3;

  // Calculate byte padding
  unsigned pad = ((4 - (((*width * formatByte) << 30) >> 30)) << 30) >> 30;

  // Read Bitmap Data
  unsigned rowBytes = (*width * formatByte);
  //printf("rowBytes: %u\n", rowBytes);
  for(unsigned i = 0; i < *height; i++)
  {
    fread(bitmap + (i * rowBytes), 1, rowBytes, file);
    fseek(file, pad, SEEK_CUR);
  }

  return 0;
}
  
// Write BMP File
int writeBMP(FILE     *file
            ,char     *bitmap
            ,unsigned height
            ,unsigned width
            ,char     format)
{
  // Convert format from Bits to Bytes
  unsigned formatByte = format >> 3;

  // Calculate byte padding
  unsigned pad = ((4 - (((width * formatByte) << 30) >> 30)) << 30) >> 30;

  // Calculate Bitmap Size and File Size
  unsigned bitmapSize = height * (width * formatByte + pad);
  unsigned fileSize   = bitmapSize + 54;

  // Fetch panel number (16bit) and format (16bit) into a 32bit unsigned
  unsigned short aux[2] = {1,format};
  unsigned panelAndFormat = (*(unsigned int *)&aux[0]);

  // Define header
  unsigned header[13] = {fileSize       // 2h  Total file size
                        ,0              // 6h  unused
                        ,54             // Ah  Bitmap data offset
                        ,40             // Eh  DIB header size
                        ,width          // 12h Width in pixels
                        ,height         // 16h Height in pixels
                        ,panelAndFormat // 1Ah Panel number (1) and format
                        ,0              // 1Eh Compression: None
                        ,bitmapSize     // 22h Raw bitmap data size
                        ,2835           // 26h Pixels/meter, horizontal
                        ,2835           // 2Ah Pixels/meter, vertical
                        ,0              // 2Eh Number of colors in the pal. 
                        ,0};            // 32h Important colors: None

  // Write header
  fprintf(file, "BM");
  fwrite(header, 4, 13, file);

  // Write bitmap array
  unsigned rowBytes = (width * formatByte);
  char padByte = 0;
  for(unsigned i = 0; i < height; i++)
  {
    fwrite(bitmap + (i * rowBytes), 1, rowBytes, file);
    fwrite(&padByte, 1, pad, file);
  }

  return 0;
}
