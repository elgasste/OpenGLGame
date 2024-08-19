#include "assets_file.h"
#include "game.h"
#include "platform.h"

#define ERROR_RETURN_FALSE() \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_GDFERR_FILECORRUPT, filePath ); \
   Platform_Log( errorMsg ); \
   AssetsFile_ClearData( assetsFileData ); \
   return False;

internal Bool_t AssetsFile_LoadFile( AssetsFileData_t* assetsFileData, const char* filePath );
internal void AssetsFile_ClearData( AssetsFileData_t* assetsFileData );
internal Bool_t AssetsFile_ReadChunk( AssetsFileChunk_t* chunk, FileData_t* fileData, uint32_t chunkOffset );
internal Bool_t AssetsFile_Interpret( AssetsFileData_t* assetsFileData, GameData_t* gameData );
internal void AssetsFile_ClearOffsetTable( AssetsFileOffsetTable_t* offsetTable, uint32_t numOffsets );
internal Bool_t AssetsFile_InterpretBitmapsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk );
internal Bool_t AssetsFile_InterpretFontsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk );
internal Bool_t AssetsFile_InterpretSpriteBasesChunk( GameData_t* gameData, AssetsFileChunk_t* chunk );

Bool_t AssetsFile_Load( GameData_t* gameData )
{
   AssetsFileData_t assetsFileData = { 0 };
   Bool_t success;
   char appDirectory[STRING_SIZE_DEFAULT];
   char dataFilePath[STRING_SIZE_DEFAULT];

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( dataFilePath, STRING_SIZE_DEFAULT, "%s%s", appDirectory, ASSETS_FILENAME );

   if ( !AssetsFile_LoadFile( &assetsFileData, dataFilePath ) )
   {
      return False;
   }

   success = AssetsFile_Interpret( &assetsFileData, gameData );
   AssetsFile_ClearData( &assetsFileData );
   return success;
}

internal Bool_t AssetsFile_LoadFile( AssetsFileData_t* assetsFileData, const char* filePath )
{
   uint32_t bytesRead, i, chunkOffset;
   FileData_t fileData;
   uint32_t* filePos32;
   AssetsFileChunk_t* chunk;
   char errorMsg[STRING_SIZE_DEFAULT];

   assetsFileData->numChunks = 0;
   assetsFileData->chunks = 0;

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      return False;
   }
   else if ( fileData.fileSize < 4 ) // 4 bytes for the number of chunks
   {
      ERROR_RETURN_FALSE();
   }

   // first 4 bytes are the number of chunks
   filePos32 = (uint32_t*)fileData.contents;
   assetsFileData->numChunks = filePos32[0];
   filePos32++;
   bytesRead = 4;

   // make sure there's enough room to read all the chunk offsets
   if ( fileData.fileSize < ( bytesRead + ( assetsFileData->numChunks * 4 ) ) )
   {
      ERROR_RETURN_FALSE();
   }

   assetsFileData->chunks = (AssetsFileChunk_t*)Platform_MAlloc( sizeof( AssetsFileChunk_t ) * assetsFileData->numChunks );
   chunk = assetsFileData->chunks;

   for ( i = 0; i < assetsFileData->numChunks; i++ )
   {
      chunk->entries = 0;
      chunk->numEntries = 0;
      chunk++;
   }

   chunk = assetsFileData->chunks;

   for ( i = 0; i < assetsFileData->numChunks; i++ )
   {
      chunkOffset = filePos32[0];
      filePos32++;
      bytesRead += 4;

      // make sure there's enough room to read at least the chunk ID and entry count
      if ( fileData.fileSize < ( chunkOffset + 8 ) )
      {
         ERROR_RETURN_FALSE();
      }

      if ( !AssetsFile_ReadChunk( chunk, &fileData, chunkOffset ) )
      {
         ERROR_RETURN_FALSE();
      }

      chunk++;
   }

   return True;
}

internal void AssetsFile_ClearData( AssetsFileData_t* assetsFileData )
{
   uint32_t i, j;
   AssetsFileChunk_t* chunk = assetsFileData->chunks;
   AssetsFileEntry_t* entry;

   if ( chunk )
   {
      for ( i = 0; i < assetsFileData->numChunks; i++ )
      {
         entry = chunk->entries;

         if ( entry )
         {
            for ( j = 0; j < chunk->numEntries; j++ )
            {
               if ( entry->memory )
               {
                  Platform_Free( entry->memory, entry->size );
               }

               entry++;
            }
         }

         Platform_Free( chunk->entries, sizeof( AssetsFileEntry_t ) * chunk->numEntries );
         chunk++;
      }

      Platform_Free( assetsFileData->chunks, sizeof( AssetsFileChunk_t ) * assetsFileData->numChunks );
      assetsFileData->numChunks = 0;
      assetsFileData->chunks = 0;
   }
}

internal Bool_t AssetsFile_ReadChunk( AssetsFileChunk_t* chunk, FileData_t* fileData, uint32_t chunkOffset )
{
   uint32_t i, j, bytesRead;
   uint8_t* filePos = (uint8_t*)( fileData->contents ) + chunkOffset;
   AssetsFileEntry_t* entry;

   chunk->ID = ( (uint32_t*)filePos )[0];
   chunk->numEntries = ( (uint32_t*)filePos )[1];
   filePos += 8;
   bytesRead = 8;

   // make sure there's enough room to read at least the ID and size of each entry
   if ( fileData->fileSize < ( chunkOffset + bytesRead + ( chunk->numEntries * 8 ) ) )
   {
      return False;
   }

   chunk->entries = (AssetsFileEntry_t*)Platform_MAlloc( sizeof( AssetsFileEntry_t ) * chunk->numEntries );
   entry = chunk->entries;

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      entry->ID = 0;
      entry->size = 0;
      entry->memory = 0;
      entry++;
   }

   entry = chunk->entries;

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      entry->ID = ( ( uint32_t*)filePos )[0];
      entry->size = ( ( uint32_t*)filePos )[1];
      filePos += 8;
      bytesRead += 8;

      if ( fileData->fileSize < ( chunkOffset + bytesRead + entry->size ) )
      {
         return False;
      }

      entry->memory = (uint8_t*)Platform_CAlloc( 1, entry->size );

      for ( j = 0; j < entry->size; j++ )
      {
         entry->memory[j] = filePos[j];
      }

      filePos += entry->size;
      bytesRead += entry->size;
      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_Interpret( AssetsFileData_t* assetsFileData, GameData_t* gameData )
{
   uint32_t i, j, numOffsets, chunkID;
   AssetsFileChunk_t* chunk;
   AssetsFileOffsetTable_t chunkIDOffsets[AssetsFileChunkID_Count];
   char msg[STRING_SIZE_DEFAULT];
   uint32_t chunkIDOrder[] = {
      (uint32_t)AssetsFileChunkID_Fonts,
      (uint32_t)AssetsFileChunkID_Bitmaps,
      (uint32_t)AssetsFileChunkID_SpriteBases
   };
   Bool_t ( *chunkLoaders[] )( GameData_t*, AssetsFileChunk_t* ) = {
      AssetsFile_InterpretFontsChunk,
      AssetsFile_InterpretBitmapsChunk,
      AssetsFile_InterpretSpriteBasesChunk
   };

   for ( i = 0; i < (uint32_t)AssetsFileChunkID_Count; i++ )
   {
      chunkIDOffsets[i].numOffsets = 0;
      chunkIDOffsets[i].offsets = 0;
   }

   chunk = assetsFileData->chunks;

   for ( i = 0; i < assetsFileData->numChunks; i++ )
   {
      if ( (AssetsFileChunkID_t)( chunk->ID ) < AssetsFileChunkID_Count )
      {
         numOffsets = chunkIDOffsets[chunk->ID].numOffsets;
         chunkIDOffsets[chunk->ID].offsets = (uint32_t*)Platform_ReAlloc( chunkIDOffsets[chunk->ID].offsets,
                                                                          4 * numOffsets,
                                                                          4 * ( numOffsets + 1 ) );
         chunkIDOffsets[chunk->ID].offsets[numOffsets] = i;
         chunkIDOffsets[chunk->ID].numOffsets++;
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNCHUNKID, chunk->ID );
         Platform_Log( msg );
      }

      chunk++;
   }

   for ( i = 0; i < (uint32_t)AssetsFileChunkID_Count; i++ )
   {
      chunkID = chunkIDOrder[i];

      if ( chunkIDOffsets[chunkID].numOffsets == 0 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_NOCHUNKSFOUNDFORID, i );
         Platform_Log( msg );
         AssetsFile_ClearOffsetTable( chunkIDOffsets, (uint32_t)AssetsFileChunkID_Count );
         return False;
      }

      for ( j = 0; j < chunkIDOffsets[chunkID].numOffsets; j++ )
      {
         chunk = assetsFileData->chunks + chunkIDOffsets[chunkID].offsets[j];
         if ( !chunkLoaders[i]( gameData, chunk ) )
         {
            AssetsFile_ClearOffsetTable( chunkIDOffsets, (uint32_t)AssetsFileChunkID_Count );
            return False;
         }
      }
   }

   AssetsFile_ClearOffsetTable( chunkIDOffsets, (uint32_t)AssetsFileChunkID_Count );
   return True;
}

internal void AssetsFile_ClearOffsetTable( AssetsFileOffsetTable_t* offsetTable, uint32_t numOffsets )
{
   uint32_t i;

   for ( i = 0; i < numOffsets; i++ )
   {
      Platform_Free( offsetTable[i].offsets, 4 * offsetTable[i].numOffsets );
      offsetTable[i].offsets = 0;
      offsetTable[i].numOffsets = 0;
   }
}

internal Bool_t AssetsFile_InterpretBitmapsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk )
{
   uint32_t i;
   ImageID_t imageID;
   AssetsFileEntry_t* entry = chunk->entries;
   Image_t* image;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      imageID = (ImageID_t)entry->ID;

      if ( imageID < ImageID_Count )
      {
         image = &( gameData->renderData.images[entry->ID] );
         Image_ClearData( image );

         if ( !Image_LoadFromBitmapMemory( image, entry->memory, entry->size ) )
         {
            return False;
         }
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNBITMAPENTRYID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_InterpretFontsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk )
{
   uint32_t i;
   FontID_t fontID;
   AssetsFileEntry_t* entry = chunk->entries;
   Font_t* font;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      fontID = (FontID_t)entry->ID;

      if ( fontID < FontID_Count )
      {
         font = &( gameData->renderData.fonts[entry->ID] );
         Font_ClearData( font );

         if ( !Font_LoadFromMemory( font, entry->memory, entry->size ) )
         {
            return False;
         }
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNFONTENTRYID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_InterpretSpriteBasesChunk( GameData_t* gameData, AssetsFileChunk_t* chunk )
{
   uint32_t i, imageID;
   SpriteBaseID_t baseID;
   AssetsFileEntry_t* entry = chunk->entries;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      baseID = (SpriteBaseID_t)entry->ID;

      if ( entry->size < 4 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_SPRITEBASECORRUPT, entry->ID );
         Platform_Log( msg );
         return False;
      }

      imageID = ( (uint32_t*)( entry->memory ) )[0];

      if ( imageID >= ImageID_Count )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_SPRITEBASEIMAGENOTFOUND, imageID );
         Platform_Log( msg );
         return False;
      }

      if ( baseID < SpriteBaseID_Count )
      {
         if ( !Sprite_LoadBaseFromMemory( &( gameData->renderData.spriteBases[entry->ID] ),
                                          &( gameData->renderData.images[imageID] ),
                                          entry->memory,
                                          entry->size ) )
         {
            return False;
         }
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNSPRITEBASEID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}
