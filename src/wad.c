#include "wad.h"
#include "wad_file.h"
#include "platform.h"

internal void Wad_ReadTextures( Wad_t* wad, WadFile_t* wadFile );

Bool_t Wad_LoadFromFile( Wad_t* wad, const char* filePath )
{
   WadFile_t wadFile;

   if ( !WadFile_Load( &wadFile, filePath ) )
   {
      return False;
   }

   Wad_ReadTextures( wad, &wadFile );

   WadFile_Clear( &wadFile );
   return True;
}

void Wad_Clear( Wad_t* wad )
{
   uint32_t i;
   PixelBuffer_t* textureBuffer = wad->textureBuffers;

   if ( textureBuffer )
   {
      for ( i = 0; i < wad->textureCount; i++ )
      {
         Platform_Free( textureBuffer->memory, textureBuffer->dimensions.x * textureBuffer->dimensions.y );
         textureBuffer++;
      }

      Platform_Free( wad->textureBuffers, sizeof( PixelBuffer_t ) * wad->textureCount );
      wad->textureBuffers = 0;
   }
}

internal void Wad_ReadTextures( Wad_t* wad, WadFile_t* wadFile )
{
   uint32_t i;
   int16_t j;
   WadFileTexture_t* texture;
   WadFileTextureHeader_t* header;
   PixelBuffer_t* textureBuffer;
   WadFileTexturePatchInfo_t* patchInfo;
   WadFilePatch_t* patch;

   wad->textureCount = (uint32_t)( wadFile->textureCount );
   wad->textureBuffers = (PixelBuffer_t*)Platform_CAlloc( wad->textureCount, sizeof( PixelBuffer_t ) );
   texture = wadFile->textureTable;
   textureBuffer = wad->textureBuffers;

   for ( i = 0; i < wad->textureCount; i++ )
   {
      header = &( texture->header );
      
      textureBuffer->dimensions.x = (uint32_t)( header->width );
      textureBuffer->dimensions.y = (uint32_t)( header->height );
      textureBuffer->memory = (uint8_t*)Platform_MAlloc( header->width * header->height );

      patchInfo = texture->patchInfoTable;

      for ( j = 0; j < header->patchInfoCount; j++ )
      {
         patch = &( wadFile->patchTable[patchInfo->patchID] );

         // MUFFINS: I'm not really sure how to read the patch memory here. patches themselves
         // have a name and a size, but patch info just has an ID and origin (upper-left based).
         // is there some other lump that I didn't read? something that has patch dimensions?

         patchInfo++;
      }

      texture++;
      textureBuffer++;
   }
}
