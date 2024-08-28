#if !defined(MAIN_H)
#define MAIN_H

#define STARTCODEPOINT     32       // space
#define ENDCODEPOINT       126      // tilde
#define MAX_GLYPHHEIGHTS   10

#define NUM_CHUNKS         4

#include "common.h"
#include "font.h"

typedef struct FileInfo_t
{
   char path[STRING_SIZE_DEFAULT];
   char dir[STRING_SIZE_DEFAULT];
   char name[STRING_SIZE_DEFAULT];
}
FileInfo_t;

typedef struct
{
   char fileName[STRING_SIZE_DEFAULT];
   uint32_t ID;
}
AssetFileToIDMapping_t;

typedef struct FontIDToGlyphHeightsMapping_t
{
   uint32_t ID;
   float glyphHeights[MAX_GLYPHHEIGHTS];
   uint32_t numGlyphHeights;
}
FontIDToGlyphHeightsMapping_t;

typedef struct FontData_t
{
   char fileName[STRING_SIZE_DEFAULT];
   Font_t font;
}
FontData_t;

typedef struct BitmapData_t
{
   char fileName[STRING_SIZE_DEFAULT];
   uint32_t size;
   uint8_t* memory;
}
BitmapData_t;

typedef struct SpriteBaseData_t
{
   uint32_t baseID;
   uint32_t imageID;
   Vector2ui32_t frameDimensions;
}
SpriteBaseData_t;

typedef struct SpriteData_t
{
   uint32_t spriteID;
   uint32_t spriteBaseID;
   Vector4f_t hitBox;
   Vector2f_t hitBoxOffset;
   float frameSeconds;
}
SpriteData_t;

typedef struct GameAssets_t
{
   uint32_t numFonts;
   FontData_t* fontDatas;
   uint32_t numBitmaps;
   BitmapData_t* bitmapDatas;
}
GameAssets_t;

#endif
