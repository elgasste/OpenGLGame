#include "menu.h"
#include "clock.h"
#include "platform.h"
#include "dialog_border.h"
#include "blit.h"
#include "text_map.h"

void Menu_ClearItems( Menu_t* menu )
{
   Platform_Free( menu->items, sizeof( MenuItem_t ) * menu->numItems );
   menu->items = 0;
}

void Menu_Reset( Menu_t* menu )
{
   menu->selectedItem = 0;
   menu->showCarat = True;
   menu->elapsedSeconds = 0.0f;
}

void Menu_IncrementSelectedItem( Menu_t* menu )
{
   menu->selectedItem++;

   if ( menu->selectedItem == menu->numItems )
   {
      menu->selectedItem = 0;
   }

   menu->showCarat = True;
   menu->elapsedSeconds = 0.0f;
}

void Menu_DecrementSelectedItem( Menu_t* menu )
{
   menu->selectedItem = ( menu->selectedItem == 0 ) ? menu->numItems - 1 : menu->selectedItem - 1;
   menu->showCarat = True;
   menu->elapsedSeconds = 0.0f;
}

void Menu_Tick( Menu_t* menu, Clock_t* clock )
{
   if ( menu->caratBlinkSeconds <= 0.0f )
   {
      return;
   }

   menu->elapsedSeconds += clock->frameDeltaSeconds;

   while ( menu->elapsedSeconds > menu->caratBlinkSeconds )
   {
      TOGGLE_BOOL( menu->showCarat );
      menu->elapsedSeconds -= menu->caratBlinkSeconds;
   }
}

void Menu_Render( Menu_t* menu, float scale )
{
   uint32_t x, y, indexBase, charIndex;
   char c;
   Bool_t drawText, drawCarat;
   TextMap_t* textMap = menu->textMap;
   float fx = menu->position.x;
   float fy = menu->position.y + ( textMap->charSize.y * ( menu->border.numChars.y - 1 ) );

   DialogBorder_Render( &( menu->border ), menu->position.x, menu->position.y, scale );

   for ( y = 1 + menu->border.padding; y < menu->border.numChars.y - menu->border.padding - 1; y++ )
   {
      indexBase = y - menu->border.padding - 1;
      uint32_t currentIndex = indexBase / ( menu->lineGap + 1 );
      drawText = currentIndex < menu->numItems && ( indexBase % ( menu->lineGap + 1 ) ) == 0 ? True: False;
      drawCarat = drawText && ( currentIndex == menu->selectedItem ) ? True : False;

      for ( x = 0; x <= menu->caratOffset; x++ )
      {
         if ( x == 0 && drawCarat && menu->showCarat )
         {
            Blit_TextChar( menu->caratChar, fx + ( ( menu->border.padding + 1 + x ) * textMap->charSize.x ), fy - ( y * textMap->charSize.y ), scale, textMap );
         }
         else
         {
            Blit_TextChar( ' ', fx + ( ( menu->border.padding + 1 + x ) * textMap->charSize.x ), fy - ( y * textMap->charSize.y ), scale, textMap );
         }
      }

      if ( drawText )
      {
         for ( charIndex = 0, x = 2 + menu->border.padding + menu->caratOffset; x < menu->border.numChars.x - menu->border.padding - 1; x++, charIndex++ )
         {
            c = charIndex < (uint32_t)strlen( menu->items[currentIndex].text ) ? menu->items[currentIndex].text[charIndex] : ' ';
            Blit_TextChar( c, fx + ( x * textMap->charSize.x ), fy - ( y * textMap->charSize.y ), scale, textMap );
         }
      }
      else
      {
         for ( x = 2 + menu->border.padding + menu->caratOffset; x < menu->border.numChars.x - menu->border.padding - 1; x++ )
         {
            Blit_TextChar( ' ', fx + ( x * textMap->charSize.x ), fy - ( y * textMap->charSize.y ), scale, textMap );
         }
      }
   }
}
