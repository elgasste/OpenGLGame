#include "assets_file.h"
#include "game.h"
#include "platform.h"

#define ERROR_RETURN_FALSE() \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_GDFERR_FILECORRUPT, filePath ); \
   Platform_Log( errorMsg ); \
   AssetsFile_ClearData( assetsFile ); \
   return False;

internal Bool_t AssetsFile_LoadFile( AssetsFileData_t* assetsFile, const char* filePath );
internal void AssetsFile_ClearData( AssetsFileData_t* assetsFile );
internal Bool_t AssetsFile_ReadChunk( AssetsFileChunk_t* chunk,
                                      FileData_t* fileData,
                                      uint32_t chunkOffset );
internal Bool_t AssetsFile_ReadFileData( GameData_t* gameData, AssetsFileData_t* fileData );
internal void AssetsFile_ClearChunkIDOffsets( AssetsFileChunkIDOffsetArray_t* offsets, uint32_t numOffsets );
internal Bool_t AssetsFile_ReadBitmapsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk );
internal Bool_t AssetsFile_ReadFontsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk );
internal Bool_t AssetsFile_ReadSpriteBasesChunk( GameData_t* gameData, AssetsFileChunk_t* chunk );
internal Bool_t AssetsFile_ReadTileSetsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk );
internal Bool_t AssetsFile_ReadTileMapsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk );

Bool_t AssetsFile_Load( GameData_t* gameData )
{
   AssetsFileData_t dataFile = { 0 };
   Bool_t success;
   char appDirectory[STRING_SIZE_DEFAULT];
   char dataFilePath[STRING_SIZE_DEFAULT];

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( dataFilePath, STRING_SIZE_DEFAULT, "%s%s", appDirectory, ASSETS_FILENAME );

   if ( !AssetsFile_LoadFile( &dataFile, dataFilePath ) )
   {
      return False;
   }

   success = AssetsFile_ReadFileData( gameData, &dataFile );
   AssetsFile_ClearData( &dataFile );
   return success;
}

internal Bool_t AssetsFile_LoadFile( AssetsFileData_t* assetsFile, const char* filePath )
{
   uint32_t bytesRead, i, chunkOffset;
   FileData_t fileData;
   uint32_t* filePos32;
   AssetsFileChunk_t* chunk;
   char errorMsg[STRING_SIZE_DEFAULT];

   assetsFile->chunks = 0;
   assetsFile->numChunks = 0;

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
   assetsFile->numChunks = filePos32[0];
   filePos32++;
   bytesRead = 4;

   // make sure there's enough room to read all the chunk offsets
   if ( fileData.fileSize < ( bytesRead + ( assetsFile->numChunks * 4 ) ) )
   {
      ERROR_RETURN_FALSE();
   }

   assetsFile->chunks = (AssetsFileChunk_t*)Platform_MAlloc( sizeof( AssetsFileChunk_t ) * assetsFile->numChunks );
   chunk = assetsFile->chunks;

   for ( i = 0; i < assetsFile->numChunks; i++ )
   {
      chunk->entries = 0;
      chunk->numEntries = 0;
      chunk++;
   }

   chunk = assetsFile->chunks;

   for ( i = 0; i < assetsFile->numChunks; i++ )
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

internal void AssetsFile_ClearData( AssetsFileData_t* assetsFile )
{
   uint32_t i, j;
   AssetsFileChunk_t* chunk = assetsFile->chunks;
   AssetsFileChunkEntry_t* entry;

   if ( chunk )
   {
      for ( i = 0; i < assetsFile->numChunks; i++ )
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

         Platform_Free( chunk->entries, sizeof( AssetsFileChunkEntry_t ) * chunk->numEntries );
         chunk++;
      }

      Platform_Free( assetsFile->chunks, sizeof( AssetsFileChunk_t ) * assetsFile->numChunks );
      assetsFile->numChunks = 0;
      assetsFile->chunks = 0;
   }
}

internal Bool_t AssetsFile_ReadChunk( AssetsFileChunk_t* chunk,
                                      FileData_t* fileData,
                                      uint32_t chunkOffset )
{
   uint32_t i, j, bytesRead;
   uint8_t* filePos = (uint8_t*)( fileData->contents ) + chunkOffset;
   AssetsFileChunkEntry_t* entry;

   chunk->ID = ( (uint32_t*)filePos )[0];
   chunk->numEntries = ( (uint32_t*)filePos )[1];
   filePos += 8;
   bytesRead = 8;

   // make sure there's enough room to read at least the ID and size of each entry
   if ( fileData->fileSize < ( chunkOffset + bytesRead + ( chunk->numEntries * 8 ) ) )
   {
      return False;
   }

   chunk->entries = (AssetsFileChunkEntry_t*)Platform_MAlloc( sizeof( AssetsFileChunkEntry_t ) * chunk->numEntries );
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

internal Bool_t AssetsFile_ReadFileData( GameData_t* gameData, AssetsFileData_t* dataFile )
{
   uint32_t i, j, numOffsets, chunkID;
   AssetsFileChunk_t* chunk;
   AssetsFileChunkIDOffsetArray_t chunkIDOffsets[AssetsFileChunkID_Count];
   char msg[STRING_SIZE_DEFAULT];
   uint32_t chunkIDOrder[] = {
      (uint32_t)AssetsFileChunkID_Fonts,
      (uint32_t)AssetsFileChunkID_Bitmaps,
      (uint32_t)AssetsFileChunkID_TileSets,
      (uint32_t)AssetsFileChunkID_SpriteBases,
      (uint32_t)AssetsFileChunkID_TileMaps
   };
   Bool_t ( *chunkLoaders[] )( GameData_t*, AssetsFileChunk_t* ) = {
      AssetsFile_ReadFontsChunk,
      AssetsFile_ReadBitmapsChunk,
      AssetsFile_ReadTileSetsChunk,
      AssetsFile_ReadSpriteBasesChunk,
      AssetsFile_ReadTileMapsChunk
   };

   for ( i = 0; i < (uint32_t)AssetsFileChunkID_Count; i++ )
   {
      chunkIDOffsets[i].offsets = 0;
      chunkIDOffsets[i].numOffsets = 0;
   }

   chunk = dataFile->chunks;

   for ( i = 0; i < dataFile->numChunks; i++ )
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
         AssetsFile_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)AssetsFileChunkID_Count );
         return False;
      }

      for ( j = 0; j < chunkIDOffsets[chunkID].numOffsets; j++ )
      {
         chunk = dataFile->chunks + chunkIDOffsets[chunkID].offsets[j];
         if ( !chunkLoaders[i]( gameData, chunk ) )
         {
            AssetsFile_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)AssetsFileChunkID_Count );
            return False;
         }
      }
   }

   AssetsFile_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)AssetsFileChunkID_Count );
   return True;
}

internal void AssetsFile_ClearChunkIDOffsets( AssetsFileChunkIDOffsetArray_t* offsets, uint32_t numOffsets )
{
   uint32_t i;

   for ( i = 0; i < numOffsets; i++ )
   {
      Platform_Free( offsets[i].offsets, 4 * offsets[i].numOffsets );
   }
}

internal Bool_t AssetsFile_ReadBitmapsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk )
{
   uint32_t i;
   ImageID_t imageID;
   AssetsFileChunkEntry_t* entry = chunk->entries;
   Image_t* image;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      imageID = (ImageID_t)entry->ID;

      if ( imageID < ImageID_Count )
      {
         image = &( gameData->renderData.images[entry->ID] );
         Image_ClearData( image );

         if ( !Image_LoadFromBitmapMemory( image, entry->memory, entry->size, entry->ID ) )
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

internal Bool_t AssetsFile_ReadFontsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk )
{
   uint32_t i;
   FontID_t fontID;
   AssetsFileChunkEntry_t* entry = chunk->entries;
   Font_t* font;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      fontID = (FontID_t)entry->ID;

      if ( fontID < FontID_Count )
      {
         font = &( gameData->renderData.fonts[entry->ID] );
         Font_ClearData( font );

         if ( !Font_LoadFromMemory( font, entry->memory, entry->size, entry->ID ) )
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

internal Bool_t AssetsFile_ReadSpriteBasesChunk( GameData_t* gameData, AssetsFileChunk_t* chunk )
{
   uint32_t i, imageID;
   SpriteBaseID_t baseID;
   AssetsFileChunkEntry_t* entry = chunk->entries;
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
                                          baseID,
                                          &( gameData->renderData.images[imageID] ),
                                          (ImageID_t)imageID,
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

internal Bool_t AssetsFile_ReadTileSetsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk )
{
   uint32_t i, imageID;
   TileSetID_t tileSetID;
   AssetsFileChunkEntry_t* entry = chunk->entries;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      tileSetID = (TileSetID_t)entry->ID;

      if ( entry->size < 4 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_TILESETCORRUPT, entry->ID );
         Platform_Log( msg );
         return False;
      }

      imageID = ( (uint32_t*)( entry->memory ) )[0];

      if ( imageID >= ImageID_Count )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_TILESETIMAGENOTFOUND, imageID );
         Platform_Log( msg );
         return False;
      }

      if ( tileSetID < TileSetID_Count )
      {
         if ( entry->size != 12 )
         {
            snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_TILESETCORRUPT, imageID );
            Platform_Log( msg );
            return False;
         }
         else if ( !TileSet_Init( &( gameData->renderData.tileSets[entry->ID] ),
                                  &( gameData->renderData.images[imageID] ),
                                  (ImageID_t)imageID,
                                  ( (uint32_t*)( entry->memory ) )[1], ( (uint32_t*)( entry->memory ) )[2] ) )
         {
            return False;
         }
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNTILESETID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_ReadTileMapsChunk( GameData_t* gameData, AssetsFileChunk_t* chunk )
{
   uint32_t i, j, tileSetID;
   TileMapID_t tileMapID;
   AssetsFileChunkEntry_t* entry = chunk->entries;
   TileMap_t* tileMap;
   uint32_t* memPos32;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      tileMapID = (TileMapID_t)entry->ID;

      if ( entry->size < 4 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_TILEMAPCORRUPT, entry->ID );
         Platform_Log( msg );
         return False;
      }

      memPos32 = (uint32_t*)( entry->memory );
      tileSetID = memPos32[0];
      memPos32++;

      if ( tileSetID >= TileSetID_Count )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_TILEMAPTILESETNOTFOUND, tileSetID );
         Platform_Log( msg );
         return False;
      }

      if ( tileMapID < TileMapID_Count )
      {
         if ( entry->size < 12 )
         {
            snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_TILEMAPCORRUPT, tileMapID );
            Platform_Log( msg );
            return False;
         }

         tileMap = &( gameData->tileMaps[entry->ID] );

         TileMap_Init( tileMap, &( gameData->renderData.tileSets[(uint32_t)tileSetID] ),
                       memPos32[0], memPos32[1] );
         memPos32 += 2;

         if ( entry->size != ( 12 + ( 4 * tileMap->dimensions.x * tileMap->dimensions.y ) ) )
         {
            snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_TILEMAPCORRUPT, entry->ID );
            Platform_Log( msg );
            return False;
         }

         for ( j = 0; j < ( tileMap->dimensions.x * tileMap->dimensions.y ); j++ )
         {
            tileMap->tileIndexes[j] = memPos32[j];
         }
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNTILEMAPID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}
