#include "game_data_file.h"
#include "platform.h"

#define ERROR_RETURN_FALSE() \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_GDFERR_FILECORRUPT, filePath ); \
   Platform_Log( errorMsg ); \
   GameDataFile_ClearData( gameDataFile ); \
   return False;

internal Bool_t GameDataFile_ReadChunk( GameDataFileChunk_t* chunk,
                                        FileData_t* fileData,
                                        uint32_t chunkOffset );

Bool_t GameDataFile_Load( GameDataFile_t* gameDataFile, const char* filePath )
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

void GameDataFile_ClearData( GameDataFile_t* gameDataFile )
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
