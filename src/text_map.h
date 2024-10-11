#if !defined( TEXT_MAP_H )
#define TEXT_MAP_H

#include "common.h"
#include "vector.h"

typedef struct Image_t Image_t;

typedef struct TextMap_t
{
   Image_t* image;
   Vector2ui32_t charSize;
   char startChar;
   char endChar;
}
TextMap_t;

#endif
