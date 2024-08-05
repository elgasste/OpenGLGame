#include <math.h>

#include "bmp.h"
#include "pixel_buffer.h"
#include "platform.h"

#define BMP_HEADER_SIZE                14
#define BMP_BITMAPINFOHEADER_SIZE      40
#define BMP_BITMAPV5HEADER_SIZE        124
#define BMP_HEADERTYPE_BM              0x4D42
#define BMP_BI_NONE                    0
#define BMP_BI_BITFIELDS               3

#define ERROR_RETURN_FALSE( s ) \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, s, imageID ); \
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
internal Bool_t Bmp_ReadHeader( BmpData_t* bmpData, uint8_t* memPos, uint32_t memSize, uint32_t imageID );
internal Bool_t Bmp_ReadDIBHeader( BmpData_t* bmpData, uint8_t* memPos, uint32_t memSize, uint32_t imageID );
internal Bool_t Bmp_ReadPalette( BmpData_t* bmpData, uint8_t* memPos, uint32_t memSize, uint32_t imageID );
internal Bool_t Bmp_VerifyDataSize( BmpData_t* bmpData, uint32_t memSize, uint32_t imageID );
internal Bool_t Bmp_ReadPixelBuffer( BmpData_t* bmpData, uint8_t* memPos, PixelBuffer_t* pixelBuffer, uint32_t imageID );

Bool_t Bmp_LoadFromMemory( uint8_t* memory, uint32_t memSize, PixelBuffer_t* pixelBuffer, uint32_t imageID )
{
   BmpData_t bmpData = { 0 };
   uint8_t* memStartPos;
   uint8_t* memPos = memory;

   pixelBuffer->memory = 0;
   pixelBuffer->dimensions.x = 0;
   pixelBuffer->dimensions.y = 0;

   memStartPos = memPos;

   if ( !Bmp_ReadHeader( &bmpData, memory, memSize, imageID ) ||
        !Bmp_ReadDIBHeader( &bmpData, memPos + BMP_HEADER_SIZE, memSize, imageID ) )
   {
      return False;
   }

   memPos += BMP_HEADER_SIZE + bmpData.dibHeaderSize;

   if ( !Bmp_ReadPalette( &bmpData, memPos, memSize, imageID ) ||
        !Bmp_VerifyDataSize( &bmpData, memSize, imageID ) ||
        !Bmp_ReadPixelBuffer( &bmpData, ( memStartPos + bmpData.imageOffset ), pixelBuffer, imageID ) )
   {
      Bmp_Cleanup( &bmpData, pixelBuffer );
      return False;
   }

   return True;
}

internal void Bmp_Cleanup( BmpData_t* bmpData, PixelBuffer_t* pixelBuffer )
{
   if ( bmpData->paletteColors )
   {
      Platform_Free( bmpData->paletteColors, sizeof( uint32_t ) * bmpData->numPaletteColors );
      bmpData->paletteColors = 0;
   }

   if ( pixelBuffer->memory )
   {
      PixelBuffer_ClearData( pixelBuffer );
   }
}

internal Bool_t Bmp_ReadHeader( BmpData_t* bmpData, uint8_t* memPos, uint32_t memSize, uint32_t imageID )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( memSize < BMP_HEADER_SIZE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
   }

   // first 2 bytes are the type, we currenly only support "BM"
   if ( ( (uint16_t*)memPos )[0] != BMP_HEADERTYPE_BM )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDHEADERTYPE );
   }

   memPos += 2;

   // next 4 bytes are the file size
   if ( ( (uint32_t*)memPos )[0] != memSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_HEADERCORRUPT );
   }

   memPos += 8;

   // the next 4 bytes after the file size are reserved, and the last
   // 4 bytes are the image data offset
   bmpData->imageOffset = ( (uint32_t*)memPos )[0];

   if ( bmpData->imageOffset >= memSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
   }

   return True;
}

internal Bool_t Bmp_ReadDIBHeader( BmpData_t* bmpData, uint8_t* memPos, uint32_t memSize, uint32_t imageID )
{
   uint8_t leftoverBits;
   uint32_t compressionMethod;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( memSize < ( BMP_HEADER_SIZE + 4 ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
   }

   // first 4 bytes are the header size
   bmpData->dibHeaderSize = ( (uint32_t*)memPos )[0];

   if ( ( bmpData->dibHeaderSize != BMP_BITMAPINFOHEADER_SIZE ) &&
        ( bmpData->dibHeaderSize != BMP_BITMAPV5HEADER_SIZE ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDDIBHEADERTYPE );
   }

   if ( memSize < ( BMP_HEADER_SIZE + bmpData->dibHeaderSize ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
   }

   memPos += 4;

   // next 8 bytes are the width and height, respectively (height can be negative)
   bmpData->imageWidth = ( (int32_t*)memPos )[0];
   bmpData->imageHeight = ( (int32_t*)memPos )[1];

   if ( bmpData->imageWidth <= 0 || bmpData->imageHeight == 0 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDIMAGESIZE );
   }

   memPos += 8;

   // next 2 bytes are the number of color planes (must be 1)
   if ( ( (uint16_t*)memPos )[0] != 1 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDCOLORPLANES );
   }

   memPos += 2;

   // next 2 bytes are the number of bits per pixel
   bmpData->bitsPerPixel = ( (uint16_t*) memPos )[0];

   if ( bmpData->bitsPerPixel != 1 &&
        bmpData->bitsPerPixel != 4 &&
        bmpData->bitsPerPixel != 8 &&
        bmpData->bitsPerPixel != 24 &&
        bmpData->bitsPerPixel != 32 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDBPP );
   }
   else if ( ( ( bmpData->bitsPerPixel == 32 ) && ( bmpData->dibHeaderSize != BMP_BITMAPV5HEADER_SIZE ) ) ||
             ( ( bmpData->bitsPerPixel != 32 ) && ( bmpData->dibHeaderSize != BMP_BITMAPINFOHEADER_SIZE ) ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDBPP );
   }

   memPos += 2;

   // next 4 bytes are the compression method, we only support uncompressed or BI_BITFIELDS for now
   compressionMethod = ( (uint32_t*)memPos )[0];

   if ( ( compressionMethod != BMP_BI_NONE ) && ( compressionMethod != BMP_BI_BITFIELDS ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_ISCOMPRESSED );
   }

   memPos += 4;

   // next 4 bytes are the size of the raw image data (should be 0 for BI_RGB bitmaps)
   bmpData->imageBytes = ( (uint32_t*)memPos )[0];

   if ( bmpData->imageBytes > 0 && ( bmpData->imageOffset + bmpData->imageBytes ) > memSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
   }

   bmpData->strideBits = bmpData->imageWidth * bmpData->bitsPerPixel;
   leftoverBits = (uint8_t)( bmpData->strideBits % 32 );
   bmpData->paddingBits = ( leftoverBits == 0 ) ? 0 : 32 - leftoverBits;
   bmpData->scanlineSize = ( bmpData->strideBits + bmpData->paddingBits ) / 8;

   // next 8 bytes after the image size are the horizontal and vertical resolution,
   // which we don't care about
   memPos += 12;

   if ( bmpData->bitsPerPixel < 32 )
   {
      // next 4 bytes after that are the number of palette
      // colors. for BPP values less than 24, this should be 2^BPP.
      bmpData->numPaletteColors = ( (uint32_t*)memPos )[0];

      if ( ( bmpData->bitsPerPixel == 24 && bmpData->numPaletteColors != 0 ) ||
           ( bmpData->numPaletteColors > (uint32_t)pow( 2, (double)( bmpData->bitsPerPixel ) ) ) )
      {
         ERROR_RETURN_FALSE( STR_BMPERR_INVALIDPALETTECOUNT );
      }
   }

   // 32bpp bitmaps should have ARGB pixel formats, so we shouldn't need to
   // extract any more information (like color bitmasks) for now
   return True;
}

internal Bool_t Bmp_ReadPalette( BmpData_t* bmpData, uint8_t* memPos, uint32_t memSize, uint32_t imageID )
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
   else if ( ( BMP_HEADER_SIZE + bmpData->dibHeaderSize + paletteSize ) > memSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
   }

   // colors are 4 bytes in RGBA format
   bmpData->paletteColors = (uint32_t*)Platform_CAlloc( 1, bmpData->numPaletteColors * 4 );

   for ( i = 0; i < bmpData->numPaletteColors; i++ )
   {
      // convert RGBA to ARGB format
      bmpData->paletteColors[i] = 0 |
                                  ( (uint32_t)( memPos[3] ) << 24 ) |
                                  ( (uint32_t)( memPos[2] ) << 16 ) |
                                  ( (uint32_t)( memPos[1] ) << 8 ) |
                                  (uint32_t)( memPos[0] );
      memPos += 4;
   }

   return True;
}

internal Bool_t Bmp_VerifyDataSize( BmpData_t* bmpData, uint32_t memSize, uint32_t imageID )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( ( memSize - bmpData->imageOffset ) != ( bmpData->scanlineSize * (uint32_t)abs( bmpData->imageHeight ) ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
   }

   return True;
}

internal Bool_t Bmp_ReadPixelBuffer( BmpData_t* bmpData, uint8_t* memPos, PixelBuffer_t* pixelBuffer, uint32_t imageID )
{
   uint8_t paddingBytes, i;
   uint16_t paletteIndex;
   uint32_t scanlineIndex, imageHeight, scanlinePixelsUnread, pixelBufferRowIndex, scanlineByteNum, color;
   uint32_t* pixelBuffer32;
   char errorMsg[STRING_SIZE_DEFAULT];

   imageHeight = (uint32_t)abs( bmpData->imageHeight );
   paddingBytes = bmpData->paddingBits / 8;

   PixelBuffer_Init( pixelBuffer, bmpData->imageWidth, imageHeight );

   for ( scanlineIndex = 0; scanlineIndex < imageHeight; scanlineIndex++ )
   {
      pixelBuffer32 = ( bmpData->imageHeight < 0 )
         ? (uint32_t*)( pixelBuffer->memory ) + ( bmpData->imageWidth * ( imageHeight - scanlineIndex - 1 ) )
         : (uint32_t*)( pixelBuffer->memory ) + ( bmpData->imageWidth * scanlineIndex );
      pixelBufferRowIndex = 0;
      scanlinePixelsUnread = bmpData->imageWidth;
      scanlineByteNum = 0;

      while ( scanlineByteNum < bmpData->scanlineSize )
      {
         switch ( bmpData->bitsPerPixel )
         {
            case 1:
               for ( i = 0; i < 8; i++, pixelBufferRowIndex++ )
               {
                  paletteIndex = 1 & ( memPos[0] >> ( 8 - ( i + 1 ) ) );
                  if ( paletteIndex >= bmpData->numPaletteColors )
                  {
                     ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
                  }
                  pixelBuffer32[pixelBufferRowIndex] = bmpData->paletteColors[paletteIndex];
                  scanlinePixelsUnread--;
                  if ( scanlinePixelsUnread == 0 )
                  {
                     pixelBufferRowIndex++;
                     break;
                  }
               }
               scanlineByteNum++;
               memPos++;
               break;
            case 4:
               for ( i = 0; i < 2; i++ )
               {
                  paletteIndex = ( i == 0 ) ? ( memPos[0] >> 4 ) : ( memPos[0] & 0xF );
                  if ( paletteIndex >= bmpData->numPaletteColors )
                  {
                     ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
                  }
                  pixelBuffer32[pixelBufferRowIndex] = bmpData->paletteColors[paletteIndex];
                  pixelBufferRowIndex++;
                  scanlinePixelsUnread--;
                  if ( scanlinePixelsUnread == 0 )
                  {
                     break;
                  }
               }
               scanlineByteNum++;
               memPos++;
               break;
            case 8:
               paletteIndex = memPos[0];
               if ( paletteIndex >= bmpData->numPaletteColors )
               {
                  ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
               }
               pixelBuffer32[pixelBufferRowIndex] = bmpData->paletteColors[paletteIndex];
               scanlinePixelsUnread--;
               scanlineByteNum++;
               pixelBufferRowIndex++;
               memPos++;
               break;
            case 24:
               color = 0xFF000000 | ( (uint32_t)memPos[2] << 16 ) | ( (uint32_t)memPos[1]  << 8 ) | (uint32_t)memPos[0];
               pixelBuffer32[pixelBufferRowIndex] = color;
               scanlinePixelsUnread--;
               memPos += 3;
               scanlineByteNum += 3;
               pixelBufferRowIndex++;
               break;
            case 32:
               color = ( (uint32_t*)memPos )[0];
               pixelBuffer32[pixelBufferRowIndex] = color;
               scanlinePixelsUnread--;
               memPos += 4;
               scanlineByteNum += 4;
               pixelBufferRowIndex++;
               break;
         }

         if ( scanlinePixelsUnread == 0 )
         {
            scanlineByteNum += paddingBytes;

            if ( scanlineByteNum != bmpData->scanlineSize )
            {
               ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
            }
         }
      }

      if ( scanlinePixelsUnread != 0 )
      {
         ERROR_RETURN_FALSE( STR_BMPERR_MEMORYCORRUPT );
      }

      memPos += paddingBytes;
   }

   return True;
}
