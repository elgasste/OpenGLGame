#if !defined( GAME_DATA_FILE_H )
#define GAME_DATA_FILE_H

#include "common.h"

typedef struct GameData_t GameData_t;

typedef struct AssetsFileOffsetTable_t
{
   uint32_t numOffsets;
   uint32_t* offsets;
}
AssetsFileOffsetTable_t;

typedef struct AssetsFileEntry_t
{
   uint32_t ID;
   uint32_t size;
   uint8_t* memory;
}
AssetsFileEntry_t;

typedef struct AssetsFileLump_t
{
   uint32_t ID;
   uint32_t numEntries;
   AssetsFileEntry_t* entries;
}
AssetsFileLump_t;

typedef struct AssetsFileData_t
{
   uint32_t numLumps;
   AssetsFileLump_t* lumps;
}
AssetsFileData_t;

Bool_t AssetsFile_Load( GameData_t* gameData );

#endif
