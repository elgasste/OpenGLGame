#include "game_data_file.h"
#include "platform.h"

#define ERROR_RETURN_FALSE() \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_GDFERR_FILECORRUPT, filePath ); \
   Platform_Log( errorMsg ); \
   GameDataFile_ClearData( gameDataFile ); \
   return False;

internal void GameDataFile_ClearChunkEntries( GameDataFile_t* gameDataFile, GameDataFileChunkType_t type );
internal Bool_t GameDataFile_ReadChunk( GameDataFile_t* gameDataFile,
                                        FileData_t* fileData,
                                        GameDataFileChunkType_t type,
                                        uint32_t chunkOffset );

Bool_t GameDataFile_Load( GameDataFile_t* gameDataFile, const char* filePath )
{
   uint32_t numChunkTypeOffsetPairs, bytesRead, i, chunkType, chunkOffset;
   FileData_t fileData;
   uint32_t* filePos32;
   char errorMsg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < (uint32_t)GameDataFileChunkType_Count; i++ )
   {
      gameDataFile->chunks[i].numEntries = 0;
      gameDataFile->chunks[i].entries = 0;
   }

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      return False;
   }
   else if ( fileData.fileSize < 4 ) // 4 bytes for the number of chunk type/offset pairs
   {
      ERROR_RETURN_FALSE();
   }

   // first 4 bytes are the number of chunk type and offset pairs in the header
   filePos32 = (uint32_t*)fileData.contents;
   numChunkTypeOffsetPairs = filePos32[0];
   filePos32++;
   bytesRead = 4;

   if ( fileData.fileSize < ( bytesRead + ( numChunkTypeOffsetPairs * 8 ) ) )
   {
      ERROR_RETURN_FALSE();
   }

   for ( i = 0; i < numChunkTypeOffsetPairs; i++ )
   {
      chunkType = filePos32[0];
      chunkOffset = filePos32[1];
      filePos32 += 2;
      bytesRead += 8;

      if ( chunkType >= (uint32_t)GameDataFileChunkType_Count )
      {
         ERROR_RETURN_FALSE();
      }
      else if ( gameDataFile->chunks[chunkType].entries )
      {
         // TODO: maybe allow adding entries to any existing ones? this would mean
         // allowing multiple chunks of the same type, for now let's only allow one.
         GameDataFile_ClearChunkEntries( gameDataFile, chunkType );
      }

      // make sure there's enough room to read at least the entry count
      if ( fileData.fileSize < ( chunkOffset + 4 ) )
      {
         ERROR_RETURN_FALSE();
      }

      if ( !GameDataFile_ReadChunk( gameDataFile, &fileData, (GameDataFileChunkType_t)chunkType, chunkOffset ) )
      {
         ERROR_RETURN_FALSE();
      }
   }

   return True;
}

void GameDataFile_ClearData( GameDataFile_t* gameDataFile )
{
   uint32_t i;

   for ( i = 0; i < (uint32_t)GameDataFileChunkType_Count; i++ )
   {
      GameDataFile_ClearChunkEntries( gameDataFile, (GameDataFileChunkType_t)i );
   }
}

internal void GameDataFile_ClearChunkEntries( GameDataFile_t* gameDataFile, GameDataFileChunkType_t type )
{
   uint32_t i;
   GameDataFileChunkEntry_t* entry;
   GameDataFileChunk_t* chunk = &( gameDataFile->chunks[(uint32_t)type] );

   if ( chunk->entries )
   {
      for ( i = 0; i < chunk->numEntries; i++ )
      {
         entry = &( chunk->entries[i] );

         if ( entry )
         {
            Platform_MemFree( entry->memory, (uint64_t)( entry->size ) );
            Platform_MemFree( entry, sizeof( GameDataFileChunkEntry_t ) );
         }
      }

      chunk->numEntries = 0;
      chunk->entries = 0;
   }
}

internal Bool_t GameDataFile_ReadChunk( GameDataFile_t* gameDataFile,
                                        FileData_t* fileData,
                                        GameDataFileChunkType_t type,
                                        uint32_t chunkOffset )
{
   uint32_t i, j, numEntries, bytesRead;
   uint8_t* filePos = (uint8_t*)( fileData->contents ) + chunkOffset;
   GameDataFileChunkEntry_t* entry;

   numEntries = ( (uint32_t*)filePos )[0];
   filePos += 4;
   bytesRead = 4;

   // make sure there's enough room to read at least the ID and size of each entry
   if ( fileData->fileSize < ( chunkOffset + bytesRead + ( numEntries * 8 ) ) )
   {
      return False;
   }

   gameDataFile->chunks[(uint32_t)type].numEntries = numEntries;
   gameDataFile->chunks[(uint32_t)type].entries =
      (GameDataFileChunkEntry_t*)Platform_MemAlloc( sizeof( GameDataFileChunkEntry_t ) * numEntries );
   entry = gameDataFile->chunks[(uint32_t)type].entries;

   for ( i = 0; i < numEntries; i++ )
   {
      entry->ID = 0;
      entry->size = 0;
      entry->memory = 0;
      entry++;
   }

   entry = gameDataFile->chunks[(uint32_t)type].entries;

   for ( i = 0; i < numEntries; i++ )
   {
      entry->ID = ( ( uint32_t*)filePos )[0];
      entry->size = ( ( uint32_t*)filePos )[1];
      filePos += 8;
      bytesRead += 8;

      if ( fileData->fileSize < ( chunkOffset + bytesRead + entry->size ) )
      {
         return False;
      }

      entry->memory = (uint8_t*)Platform_MemAlloc( (uint64_t)( entry->size ) );

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
