#if !defined( GAME_DATA_FILE_H )
#define GAME_DATA_FILE_H

#include "common.h"

typedef struct GameData_t GameData_t;

typedef struct ChunkIDOffsetArray_t
{
   uint32_t* offsets;
   uint32_t numOffsets;
}
ChunkIDOffsetArray_t;

typedef struct GameDataFileChunkEntry_t
{
   uint32_t ID;
   uint32_t size;
   uint8_t* memory;
}
GameDataFileChunkEntry_t;

typedef struct GameDataFileChunk_t
{
   uint32_t ID;
   uint32_t numEntries;
   GameDataFileChunkEntry_t* entries;
}
GameDataFileChunk_t;

typedef struct GameDataFile_t
{
   uint32_t numChunks;
   GameDataFileChunk_t* chunks;
}
GameDataFile_t;

Bool_t GameDataFile_Load( GameData_t* gameData );

#endif
