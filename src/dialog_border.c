#include "dialog_border.h"
#include "text_map.h"
#include "blit.h"

void DialogBorder_Init( DialogBorder_t* border, Image_t* bordersImage, TextMap_t* textMap, uint32_t numCharsX, uint32_t numCharsY, uint32_t padding )
{
   border->bordersImage = bordersImage;
   border->textMap = textMap;
   border->numChars.x = numCharsX;
   border->numChars.y = numCharsY;
   border->padding = padding;
}

void DialogBorder_Render( DialogBorder_t* border, float screenX, float screenY, float scale )
{
   uint32_t x, y;
   TextMap_t* textMap = border->textMap;
   uint32_t totalCharsX = border->numChars.x + ( border->padding * 2 ) + 2;
   uint32_t totalCharsY = border->numChars.y + ( border->padding * 2 ) + 2;
   float fx = screenX;
   float fy = screenY + ( textMap->charSize.y * ( totalCharsY - 1 ) );

   // border
   Blit_ImageSection( border->bordersImage, fx, fy, (uint32_t)MenuBorderSide_TopLeft * textMap->charSize.x, 0, textMap->charSize.x, textMap->charSize.y, scale );
   Blit_ImageSection( border->bordersImage, fx + ( ( totalCharsX - 1 ) * textMap->charSize.x ), fy, (uint32_t)MenuBorderSide_TopRight * textMap->charSize.x, 0, textMap->charSize.x, textMap->charSize.y, scale );
   Blit_ImageSection( border->bordersImage, fx, screenY, (uint32_t)MenuBorderSide_BottomLeft * textMap->charSize.x, 0, textMap->charSize.x, textMap->charSize.y, scale );
   Blit_ImageSection( border->bordersImage, fx + ( ( totalCharsX - 1 ) * textMap->charSize.x ), screenY, (uint32_t)MenuBorderSide_BottomRight * textMap->charSize.x, 0, textMap->charSize.x, textMap->charSize.y, scale );
   for ( y = 1; y < totalCharsY - 1; y++ )
   {
      Blit_ImageSection( border->bordersImage, fx, fy - ( textMap->charSize.y * y ), (uint32_t)MenuBorderSide_Left * textMap->charSize.x, 0, textMap->charSize.x, textMap->charSize.y, scale );
      Blit_ImageSection( border->bordersImage, fx + ( ( totalCharsX - 1 ) * textMap->charSize.x ), fy - ( textMap->charSize.y * y ), (uint32_t)MenuBorderSide_Right * textMap->charSize.x, 0, textMap->charSize.x, textMap->charSize.y, scale );
   }
   for ( x = 1; x < totalCharsX - 1; x++ )
   {
      Blit_ImageSection( border->bordersImage, fx + ( textMap->charSize.x * x ), fy, (uint32_t)MenuBorderSide_Top * textMap->charSize.x, 0, textMap->charSize.x, textMap->charSize.y, scale );
      Blit_ImageSection( border->bordersImage, fx + ( textMap->charSize.x * x ), fy - ( textMap->charSize.y * y ), (uint32_t)MenuBorderSide_Bottom * textMap->charSize.x, 0, textMap->charSize.x, textMap->charSize.y, scale );
   }

   // padding
   if ( border->padding > 0 )
   {
      for ( y = 1; y <= border->padding; y++ )
         for ( x = 1; x < totalCharsX - 1; x++ ) Blit_TextChar( ' ', fx + ( textMap->charSize.x * x ), fy - ( textMap->charSize.y * y ), scale, textMap );
      for ( y = totalCharsY - border->padding - 1; y < totalCharsY - 1; y++ )
         for ( x = 1; x < totalCharsX - 1; x++ ) Blit_TextChar( ' ', fx + ( textMap->charSize.x * x ), fy - ( textMap->charSize.y * y ), scale, textMap );
      for ( y = border->padding + 1; y < totalCharsY - border->padding - 1; y++ )
      {
         for ( x = 1; x <= border->padding; x++ ) Blit_TextChar( ' ', fx + ( textMap->charSize.x * x ), fy - ( textMap->charSize.y * y ), scale, textMap );
         for ( x = totalCharsX - border->padding - 1; x < totalCharsX - 1; x++ ) Blit_TextChar( ' ', fx + ( textMap->charSize.x * x ), fy - ( textMap->charSize.y * y ), scale, textMap );
      }
   }
}
