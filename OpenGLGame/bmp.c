#include <math.h>

#include "bmp.h"
#include "platform.h"

#define BMP_HEADER_SIZE                14
#define BMP_BITMAPINFOHEADER_SIZE      40
#define BMP_HEADER_TYPE                0x4D42

#define ERROR_RETURN_FALSE( s ) \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, s, fileData->filePath ); \
   Platform_Log( errorMsg ); \
   return False

typedef struct
{
   uint32_t dibHeaderSize;
   uint32_t imageOffset;
   int32_t imageWidth;
   int32_t imageHeight;
   uint16_t bitsPerPixel;
   uint32_t strideBits;
   uint8_t paddingBits;
   uint32_t scanlineSize;
   uint32_t imageBytes;
   uint32_t numPaletteColors;
   uint32_t* paletteColors;
}
BmpData_t;

internal void Bmp_Cleanup( BmpData_t* bmpData, PixelBuffer_t* pixelBuffer );
internal Bool_t Bmp_ReadHeader( BmpData_t* bmpData, FileData_t* fileData, uint8_t* filePos );
internal Bool_t Bmp_ReadDIBHeader( BmpData_t* bmpData, FileData_t* fileData, uint8_t* filePos );
internal Bool_t Bmp_ReadPalette( BmpData_t* bmpData, FileData_t* fileData, uint8_t* filePos );
internal Bool_t Bmp_VerifyDataSize( BmpData_t* bmpData, FileData_t* fileData );
internal Bool_t Bmp_ReadPixelBuffer( BmpData_t* bmpData, FileData_t* fileData, uint8_t* filePos, PixelBuffer_t* pixelBuffer );

Bool_t Bmp_LoadFromFile( const char* filePath, PixelBuffer_t* pixelBuffer )
{
   FileData_t fileData;
   BmpData_t bmpData = { 0 };
   uint8_t* fileStartPos;
   uint8_t* filePos;

   pixelBuffer->buffer = 0;
   pixelBuffer->width = 0;
   pixelBuffer->height = 0;

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      return False;
   }

   filePos = (uint8_t*)fileData.contents;
   fileStartPos = filePos;

   if ( !Bmp_ReadHeader( &bmpData, &fileData, filePos ) ||
        !Bmp_ReadDIBHeader( &bmpData, &fileData, filePos + BMP_HEADER_SIZE ) )
   {
      Platform_ClearFileData( &fileData );
      return False;
   }

   filePos += BMP_HEADER_SIZE + bmpData.dibHeaderSize;

   if ( !Bmp_ReadPalette( &bmpData, &fileData, filePos ) ||
        !Bmp_VerifyDataSize( &bmpData, &fileData ) ||
        !Bmp_ReadPixelBuffer( &bmpData, &fileData, ( fileStartPos + bmpData.imageOffset ), pixelBuffer ) )
   {
      Bmp_Cleanup( &bmpData, pixelBuffer );
      Platform_ClearFileData( &fileData );
      return False;
   }

   return True;
}

internal void Bmp_Cleanup( BmpData_t* bmpData, PixelBuffer_t* pixelBuffer )
{
   if ( bmpData->paletteColors )
   {
      Platform_MemFree( bmpData->paletteColors );
      bmpData->paletteColors = 0;
   }

   if ( pixelBuffer->buffer )
   {
      Platform_MemFree( pixelBuffer->buffer );
      pixelBuffer->buffer = 0;
      pixelBuffer->width = 0;
      pixelBuffer->height = 0;
   }
}

internal Bool_t Bmp_ReadHeader( BmpData_t* bmpData, FileData_t* fileData, uint8_t* filePos )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( fileData->fileSize < BMP_HEADER_SIZE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   // first 2 bytes are the type, we currenly only support "BM"
   if ( ( (uint16_t*)filePos )[0] != BMP_HEADER_TYPE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDHEADERTYPE );
   }

   filePos += 2;

   // next 4 bytes are the file size
   if ( ( (uint32_t*)filePos )[0] != fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_HEADERCORRUPT );
   }

   filePos += 8;

   // the next 4 bytes after the file size are reserved, and the last
   // 4 bytes are the image data offset
   bmpData->imageOffset = ( (uint32_t*)filePos )[0];

   if ( bmpData->imageOffset >= fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   return True;
}

internal Bool_t Bmp_ReadDIBHeader( BmpData_t* bmpData, FileData_t* fileData, uint8_t* filePos )
{
   uint8_t leftoverBits;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( fileData->fileSize < ( BMP_HEADER_SIZE + 4 ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   // first 4 bytes are the header size
   // TODO: 32-bit headers are coming in at 124 bytes, which is a BITMAPV5HEADER.
   // we'll want to support alpha values though, so figure that one out.
   bmpData->dibHeaderSize = ( (uint32_t*)filePos )[0];

   if ( bmpData->dibHeaderSize != BMP_BITMAPINFOHEADER_SIZE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDDIBHEADERTYPE );
   }

   if ( fileData->fileSize < BMP_HEADER_SIZE + bmpData->dibHeaderSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   filePos += 4;

   // next 8 bytes are the width and height, respectively (height can be negative)
   bmpData->imageWidth = ( (int32_t*)filePos )[0];
   bmpData->imageHeight = ( (int32_t*)filePos )[1];

   if ( bmpData->imageWidth <= 0 || bmpData->imageHeight == 0 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDIMAGESIZE );
   }

   filePos += 8;

   // next 2 bytes are the number of color planes (must be 1)
   if ( ( (uint16_t*)filePos )[0] != 1 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDCOLORPLANES );
   }

   filePos += 2;

   // next 2 bytes are the number of bits per pixel
   bmpData->bitsPerPixel = ( (uint16_t*) filePos )[0];

   // TODO: add support for 32 bpp bitmaps
   if ( bmpData->bitsPerPixel != 1 && bmpData->bitsPerPixel != 4 && bmpData->bitsPerPixel != 8 && bmpData->bitsPerPixel != 24 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDBPP );
   }

   filePos += 2;

   // next 4 bytes are the compression method, we only support uncompressed images for now
   if ( ( (uint32_t*)filePos )[0] != 0 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_ISCOMPRESSED );
   }

   filePos += 4;

   // next 4 bytes are the size of the raw image data (should be 0 for BI_RGB bitmaps)
   bmpData->imageBytes = ( (uint32_t*)filePos )[0];

   if ( bmpData->imageBytes > 0 && ( bmpData->imageOffset + bmpData->imageBytes ) > fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   filePos += 12;

   // next 8 bytes after the image size are the horizontal and vertical resolution,
   // which we don't care about. next 4 bytes after that are the number of palette
   // colors. for BPP values less than 24, this should be 2^BPP.
   bmpData->numPaletteColors = ( (uint32_t*)filePos )[0];

   if ( ( bmpData->bitsPerPixel == 24 && bmpData->numPaletteColors != 0 ) ||
        bmpData->numPaletteColors > (uint32_t)pow( 2, (double)( bmpData->bitsPerPixel ) ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDPALETTECOUNT );
   }

   bmpData->strideBits = bmpData->imageWidth * bmpData->bitsPerPixel;
   leftoverBits = (uint8_t)( bmpData->strideBits % 32 );
   bmpData->paddingBits = ( leftoverBits == 0 ) ? 0 : 32 - leftoverBits;
   bmpData->scanlineSize = ( bmpData->strideBits + bmpData->paddingBits ) / 8;

   return True;
}

internal Bool_t Bmp_ReadPalette( BmpData_t* bmpData, FileData_t* fileData, uint8_t* filePos )
{
   uint32_t i;
   uint32_t paletteSize;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( bmpData->numPaletteColors == 0 )
   {
      return True;
   }

   paletteSize = bmpData->imageOffset - ( BMP_HEADER_SIZE + bmpData->dibHeaderSize );
   
   if ( paletteSize != ( (uint32_t)( bmpData->numPaletteColors * 4 ) ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_PALETTECORRUPT );
   }
   else if ( ( BMP_HEADER_SIZE + bmpData->dibHeaderSize + paletteSize ) > fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   // colors are 4 bytes in RGBA format
   bmpData->paletteColors = (uint32_t*)Platform_MemAlloc( bmpData->numPaletteColors * 4 );

   for ( i = 0; i < bmpData->numPaletteColors; i++ )
   {
      // convert RGBA to our ARGB format
      bmpData->paletteColors[i] = 0 |
                                  ( (uint32_t)( filePos[3] ) << 24 ) |
                                  ( (uint32_t)( filePos[2] ) << 16 ) |
                                  ( (uint32_t)( filePos[1] ) << 8 ) |
                                  (uint32_t)( filePos[0] );
      filePos += 4;
   }

   return True;
}

internal Bool_t Bmp_VerifyDataSize( BmpData_t* bmpData, FileData_t* fileData )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( ( fileData->fileSize - bmpData->imageOffset ) != ( bmpData->scanlineSize * bmpData->imageHeight ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   return True;
}

internal Bool_t Bmp_ReadPixelBuffer( BmpData_t* bmpData, FileData_t* fileData, uint8_t* filePos, PixelBuffer_t* pixelBuffer )
{
   int8_t scanlineIncrement, paddingBytes, i;
   uint16_t paletteIndex;
   uint32_t scanlineNum, scanlineStart, imageHeight, scanlinePixelsUnread, pixelBufferIndex, scanlineByteNum, color;
   uint32_t* pixelBuffer32;
   char errorMsg[STRING_SIZE_DEFAULT];

   scanlineIncrement = ( bmpData->imageHeight < 0 ) ? -1 : 1;
   scanlineStart = ( bmpData->imageHeight < 0 ) ? bmpData->imageHeight - 1 : 0;
   // TODO: test this with a negative image height
   imageHeight = (uint32_t)abs( bmpData->imageHeight );
   paddingBytes = bmpData->paddingBits / 8;

   pixelBuffer->width = bmpData->imageWidth;
   pixelBuffer->height = imageHeight;
   pixelBuffer->buffer = (uint8_t*)Platform_MemAlloc( bmpData->imageWidth * imageHeight * GRAPHICS_BPP );
   pixelBuffer32 = (uint32_t*)( pixelBuffer->buffer );

   scanlineNum = scanlineStart;
   pixelBufferIndex = 0;

   // TODO: is there any real way this could hang?
   while ( 1 )
   {
      scanlinePixelsUnread = bmpData->imageWidth;
      scanlineByteNum = 0;

      while ( scanlineByteNum < bmpData->scanlineSize )
      {

         switch ( bmpData->bitsPerPixel )
         {
            case 1:
               for ( i = 0; i < 8; i++, pixelBufferIndex++ )
               {
                  paletteIndex = 1 & ( filePos[0] >> ( 8 - ( i + 1 ) ) );
                  if ( paletteIndex >= bmpData->numPaletteColors )
                  {
                     ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
                  }
                  pixelBuffer32[pixelBufferIndex] = bmpData->paletteColors[paletteIndex];
                  scanlinePixelsUnread--;
                  if ( scanlinePixelsUnread == 0 )
                  {
                     pixelBufferIndex++;
                     break;
                  }
               }
               scanlineByteNum++;
               filePos++;
               break;
            case 4:
               for ( i = 0; i < 2; i++ )
               {
                  paletteIndex = ( i == 0 ) ? ( filePos[0] >> 4 ) : ( filePos[0] & 0xF );
                  if ( paletteIndex >= bmpData->numPaletteColors )
                  {
                     ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
                  }
                  pixelBuffer32[pixelBufferIndex] = bmpData->paletteColors[paletteIndex];
                  pixelBufferIndex++;
                  scanlinePixelsUnread--;
                  if ( scanlinePixelsUnread == 0 )
                  {
                     break;
                  }
               }
               scanlineByteNum++;
               filePos++;
               break;
            case 8:
               paletteIndex = filePos[0];
               if ( paletteIndex >= bmpData->numPaletteColors )
               {
                  ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
               }
               pixelBuffer32[pixelBufferIndex] = bmpData->paletteColors[paletteIndex];
               scanlinePixelsUnread--;
               scanlineByteNum++;
               pixelBufferIndex++;
               filePos++;
               break;
            case 24:
               color = 0xFF000000 | ( (uint32_t)filePos[2] << 16 ) | ( (uint32_t)filePos[1]  << 8 ) | (uint32_t)filePos[0];
               pixelBuffer32[pixelBufferIndex] = color;
               scanlinePixelsUnread--;
               filePos += 3;
               scanlineByteNum += 3;
               pixelBufferIndex++;
               break;
         }

         if ( scanlinePixelsUnread == 0 )
         {
            scanlineByteNum += paddingBytes;

            if ( scanlineByteNum != bmpData->scanlineSize )
            {
               ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
            }
         }
      }

      if ( scanlinePixelsUnread != 0 )
      {
         ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
      }

      filePos += paddingBytes;
      scanlineNum += scanlineIncrement;

      if ( ( bmpData->imageHeight < 0 && scanlineNum <= 0 ) ||
           ( bmpData->imageHeight > 0 && scanlineNum >= (uint32_t)( bmpData->imageHeight ) ) )
      {
         break;
      }
   }

   return True;
}
