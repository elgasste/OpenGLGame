#if !defined( GAME_DATA_FILE_H )
#define GAME_DATA_FILE_H

#include "common.h"

typedef struct GameData_t GameData_t;

typedef struct AssetsFileChunkIDOffsetArray_t
{
   uint32_t* offsets;
   uint32_t numOffsets;
}
AssetsFileChunkIDOffsetArray_t;

typedef struct AssetsFileChunkEntry_t
{
   uint32_t ID;
   uint32_t size;
   uint8_t* memory;
}
AssetsFileChunkEntry_t;

typedef struct AssetsFileChunk_t
{
   uint32_t ID;
   uint32_t numEntries;
   AssetsFileChunkEntry_t* entries;
}
AssetsFileChunk_t;

typedef struct AssetsFileData_t
{
   uint32_t numChunks;
   AssetsFileChunk_t* chunks;
}
AssetsFileData_t;

Bool_t AssetsFile_Load( GameData_t* gameData );

#endif
