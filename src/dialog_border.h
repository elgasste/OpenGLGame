#if !defined( DIALOG_BORDER_H )
#define DIALOG_BORDER_H

#include "common.h"
#include "vector.h"

typedef struct Image_t Image_t;
typedef struct TextMap_t TextMap_t;

typedef struct DialogBorder_t
{
   Image_t* bordersImage;
   TextMap_t* textMap;
   Vector2ui32_t numChars;
   uint32_t padding;
}
DialogBorder_t;

void DialogBorder_Init( DialogBorder_t* border, Image_t* bordersImage, TextMap_t* textMap, uint32_t numCharsX, uint32_t numCharsY, uint32_t padding );
void DialogBorder_Render( DialogBorder_t* border,  float screenX, float screenY, float scale );

#endif
