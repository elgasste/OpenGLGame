#include "game_data_file.h"
#include "game.h"
#include "platform.h"

#define ERROR_RETURN_FALSE() \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_GDFERR_FILECORRUPT, filePath ); \
   Platform_Log( errorMsg ); \
   GameDataFile_ClearData( gameDataFile ); \
   return False;

internal Bool_t GameDataFile_LoadFile( GameDataFile_t* gameDataFile, const char* filePath );
internal void GameDataFile_ClearData( GameDataFile_t* gameDataFile );
internal Bool_t GameDataFile_ReadChunk( GameDataFileChunk_t* chunk,
                                        FileData_t* fileData,
                                        uint32_t chunkOffset );
internal Bool_t GameDataFile_ReadFileData( GameData_t* gameData, GameDataFile_t* fileData );
internal void GameDataFile_ClearChunkIDOffsets( ChunkIDOffsetArray_t* offsets, uint32_t numOffsets );
internal Bool_t GameDataFile_ReadBitmapsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );
internal Bool_t GameDataFile_ReadFontsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );
internal Bool_t GameDataFile_ReadSpriteBasesChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );

Bool_t GameDataFile_Load( GameData_t* gameData )
{
   GameDataFile_t dataFile = { 0 };
   Bool_t success;
   char appDirectory[STRING_SIZE_DEFAULT];
   char dataFilePath[STRING_SIZE_DEFAULT];

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( dataFilePath, STRING_SIZE_DEFAULT, "%s%s", appDirectory, GAME_DATA_FILENAME );

   if ( !GameDataFile_LoadFile( &dataFile, dataFilePath ) )
   {
      return False;
   }

   success = GameDataFile_ReadFileData( gameData, &dataFile );
   GameDataFile_ClearData( &dataFile );
   return success;
}

internal Bool_t GameDataFile_LoadFile( GameDataFile_t* gameDataFile, const char* filePath )
{
   uint32_t bytesRead, i, chunkOffset;
   FileData_t fileData;
   uint32_t* filePos32;
   GameDataFileChunk_t* chunk;
   char errorMsg[STRING_SIZE_DEFAULT];

   gameDataFile->chunks = 0;
   gameDataFile->numChunks = 0;

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
   gameDataFile->numChunks = filePos32[0];
   filePos32++;
   bytesRead = 4;

   // make sure there's enough room to read all the chunk offsets
   if ( fileData.fileSize < ( bytesRead + ( gameDataFile->numChunks * 4 ) ) )
   {
      ERROR_RETURN_FALSE();
   }

   gameDataFile->chunks = (GameDataFileChunk_t*)Platform_MAlloc( sizeof( GameDataFileChunk_t ) * gameDataFile->numChunks );
   chunk = gameDataFile->chunks;

   for ( i = 0; i < gameDataFile->numChunks; i++ )
   {
      chunk->entries = 0;
      chunk->numEntries = 0;
      chunk++;
   }

   chunk = gameDataFile->chunks;

   for ( i = 0; i < gameDataFile->numChunks; i++ )
   {
      chunkOffset = filePos32[0];
      filePos32++;
      bytesRead += 4;

      // make sure there's enough room to read at least the chunk ID and entry count
      if ( fileData.fileSize < ( chunkOffset + 8 ) )
      {
         ERROR_RETURN_FALSE();
      }

      if ( !GameDataFile_ReadChunk( chunk, &fileData, chunkOffset ) )
      {
         ERROR_RETURN_FALSE();
      }

      chunk++;
   }

   return True;
}

internal void GameDataFile_ClearData( GameDataFile_t* gameDataFile )
{
   uint32_t i, j;
   GameDataFileChunk_t* chunk = gameDataFile->chunks;
   GameDataFileChunkEntry_t* entry;

   if ( chunk )
   {
      for ( i = 0; i < gameDataFile->numChunks; i++ )
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

         Platform_Free( chunk->entries, sizeof( GameDataFileChunkEntry_t ) * chunk->numEntries );
         chunk++;
      }

      Platform_Free( gameDataFile->chunks, sizeof( GameDataFileChunk_t ) * gameDataFile->numChunks );
      gameDataFile->numChunks = 0;
      gameDataFile->chunks = 0;
   }
}

internal Bool_t GameDataFile_ReadChunk( GameDataFileChunk_t* chunk,
                                        FileData_t* fileData,
                                        uint32_t chunkOffset )
{
   uint32_t i, j, bytesRead;
   uint8_t* filePos = (uint8_t*)( fileData->contents ) + chunkOffset;
   GameDataFileChunkEntry_t* entry;

   chunk->ID = ( (uint32_t*)filePos )[0];
   chunk->numEntries = ( (uint32_t*)filePos )[1];
   filePos += 8;
   bytesRead = 8;

   // make sure there's enough room to read at least the ID and size of each entry
   if ( fileData->fileSize < ( chunkOffset + bytesRead + ( chunk->numEntries * 8 ) ) )
   {
      return False;
   }

   chunk->entries = (GameDataFileChunkEntry_t*)Platform_MAlloc( sizeof( GameDataFileChunkEntry_t ) * chunk->numEntries );
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

internal Bool_t GameDataFile_ReadFileData( GameData_t* gameData, GameDataFile_t* dataFile )
{
   uint32_t i, j, numOffsets, chunkID;
   GameDataFileChunk_t* chunk;
   ChunkIDOffsetArray_t chunkIDOffsets[GameDataFileChunkID_Count];
   char msg[STRING_SIZE_DEFAULT];
   uint32_t chunkIDOrder[] = {
      (uint32_t)GameDataFileChunkID_Fonts,
      (uint32_t)GameDataFileChunkID_Bitmaps,
      (uint32_t)GameDataFileChunkID_SpriteBases
   };
   Bool_t ( *chunkLoaders[] )( GameData_t*, GameDataFileChunk_t* ) = {
      GameDataFile_ReadFontsChunk,
      GameDataFile_ReadBitmapsChunk,
      GameDataFile_ReadSpriteBasesChunk
   };

   for ( i = 0; i < (uint32_t)GameDataFileChunkID_Count; i++ )
   {
      chunkIDOffsets[i].offsets = 0;
      chunkIDOffsets[i].numOffsets = 0;
   }

   chunk = dataFile->chunks;

   for ( i = 0; i < dataFile->numChunks; i++ )
   {
      if ( (GameDataFileChunkID_t)( chunk->ID ) < GameDataFileChunkID_Count )
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

   for ( i = 0; i < (uint32_t)GameDataFileChunkID_Count; i++ )
   {
      chunkID = chunkIDOrder[i];

      if ( chunkIDOffsets[chunkID].numOffsets == 0 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_NOCHUNKSFOUNDFORID, i );
         Platform_Log( msg );
         GameDataFile_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)GameDataFileChunkID_Count );
         return False;
      }

      for ( j = 0; j < chunkIDOffsets[chunkID].numOffsets; j++ )
      {
         chunk = dataFile->chunks + chunkIDOffsets[chunkID].offsets[j];
         if ( !chunkLoaders[i]( gameData, chunk ) )
         {
            GameDataFile_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)GameDataFileChunkID_Count );
            return False;
         }
      }
   }

   GameDataFile_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)GameDataFileChunkID_Count );
   return True;
}

internal void GameDataFile_ClearChunkIDOffsets( ChunkIDOffsetArray_t* offsets, uint32_t numOffsets )
{
   uint32_t i;

   for ( i = 0; i < numOffsets; i++ )
   {
      Platform_Free( offsets[i].offsets, 4 * offsets[i].numOffsets );
   }
}

internal Bool_t GameDataFile_ReadBitmapsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk )
{
   uint32_t i;
   ImageID_t imageID;
   GameDataFileChunkEntry_t* entry = chunk->entries;
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

internal Bool_t GameDataFile_ReadFontsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk )
{
   uint32_t i;
   FontID_t fontID;
   GameDataFileChunkEntry_t* entry = chunk->entries;
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

internal Bool_t GameDataFile_ReadSpriteBasesChunk( GameData_t* gameData, GameDataFileChunk_t* chunk )
{
   uint32_t i, imageID;
   SpriteBaseID_t baseID;
   GameDataFileChunkEntry_t* entry = chunk->entries;
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
