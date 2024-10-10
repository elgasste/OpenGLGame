#if !defined( WAD_FILE_H )
#define WAD_FILE_H

#include "common.h"

#define WADFILE_PALETTE_COLORCOUNT  256
#define WADFILE_PALETTE_COUNT       14

#pragma pack( push, 1 )

typedef struct WadFileHeader_t
{
   char wadType[4];
   int32_t lumpCount;
   int32_t lumpTableOffset;
}
WadFileHeader_t;

typedef struct WadFileLumpInfo_t
{
   int32_t filePos;
   int32_t size;
   char name[8];
}
WadFileLumpInfo_t;

typedef struct WadFilePaletteColor_t
{
   uint8_t r;
   uint8_t g;
   uint8_t b;
}
WadFilePaletteColor_t;

typedef struct WadFilePalette_t
{
   WadFilePaletteColor_t colorTable[WADFILE_PALETTE_COLORCOUNT];
}
WadFilePalette_t;

typedef struct WadFileTextureHeader_t
{
   char name[8];
   Bool_t masked;
   int16_t width;
   int16_t height;
   int32_t columnDirectory;   // unused
   int16_t patchInfoCount;
}
WadFileTextureHeader_t;

typedef struct WadFileTexturePatchInfo_t
{
   int16_t originX;  // upper-left origin
   int16_t originY;
   int16_t patchID;
   int16_t stepDir;  // unused
   int16_t colorMap; // unused
}
WadFileTexturePatchInfo_t;

#pragma pack( pop )

typedef struct WadFileTexture_t
{
   WadFileTextureHeader_t header;
   WadFileTexturePatchInfo_t* patchInfoTable;
}
WadFileTexture_t;

typedef struct WadFilePatch_t
{
   char name[8];
   int32_t size;
   uint8_t* memory;
}
WadFilePatch_t;

typedef struct WadFileFlat_t
{
   char name[8];
   int32_t size;
   uint8_t* memory;
}
WadFileFlat_t;

typedef struct WadFile_t
{
   WadFileHeader_t header;
   WadFileLumpInfo_t* lumpInfoTable;
   WadFilePalette_t* paletteTable;
   int32_t textureCount;
   WadFileTexture_t* textureTable;
   int32_t patchCount;
   WadFilePatch_t* patchTable;
   int32_t flatCount;
   WadFileFlat_t* flatTable;
}
WadFile_t;

Bool_t WadFile_Load( WadFile_t* wadFile, const char* filePath );
void WadFile_Clear( WadFile_t* wadFile );

#endif
