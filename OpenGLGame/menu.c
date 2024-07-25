#include "menu.h"
#include "clock.h"
#include "platform.h"

void Menu_ClearItems( Menu_t* menu )
{
   uint32_t i;
   MenuItem_t* item = menu->items;

   for ( i = 0; i < menu->numItems; i++ )
   {
      Platform_MemFree( item, sizeof( MenuItem_t ) );
      item++;
   }

   menu->items = 0;
}

void Menu_Reset( Menu_t* menu )
{
   menu->selectedItem = 0;
   menu->caratFadingOut = True;
   menu->caratElapsedSeconds = 0.0f;
}

void Menu_IncrementSelectedItem( Menu_t* menu )
{
   menu->selectedItem++;

   if ( menu->selectedItem == menu->numItems )
   {
      menu->selectedItem = 0;
   }

   menu->caratFadingOut = True;
   menu->caratElapsedSeconds = 0.0f;
}

void Menu_DecrementSelectedItem( Menu_t* menu )
{
   menu->selectedItem = ( menu->selectedItem == 0 ) ? menu->numItems - 1 : menu->selectedItem - 1;
   menu->caratFadingOut = True;
   menu->caratElapsedSeconds = 0.0f;
}

void Menu_Tick( Menu_t* menu, Clock_t* clock )
{
   uint32_t color = menu->renderData.caratColor;
   uint32_t alpha;

   if ( menu->caratFadeSeconds == 0.0f )
   {
      return;
   }

   menu->caratElapsedSeconds += clock->frameDeltaSeconds;

   while ( menu->caratElapsedSeconds > menu->caratFadeSeconds )
   {
      TOGGLE_BOOL( menu->caratFadingOut );
      menu->caratElapsedSeconds -= menu->caratFadeSeconds;
   }

   alpha = (uint32_t)( 255 * ( menu->caratElapsedSeconds / menu->caratFadeSeconds ) );
   color &= 0x00FFFFFF;
   color |= menu->caratFadingOut ? ( ( 255 - alpha ) << 24 ) : ( alpha << 24 );
   menu->renderData.caratColor = color;
}
