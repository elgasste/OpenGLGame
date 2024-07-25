#include "menu.h"
#include "platform.h"

void Menu_ClearItems( Menu_t* menu )
{
   uint32_t i;
   MenuItem_t* item = menu->items;

   for ( i = 0; i < menu->numItems; i++ )
   {
      Platform_MemFree( item );
      item++;
   }

   menu->items = 0;
}

void Menu_Reset( Menu_t* menu )
{
   menu->selectedItem = 0;
   menu->showCarat = True;
   menu->caratElapsedSeconds = 0.0f;
}

void Menu_IncrementSelectedItem( Menu_t* menu )
{
   menu->selectedItem++;

   if ( menu->selectedItem == menu->numItems )
   {
      menu->selectedItem = 0;
   }

   menu->showCarat = True;
   menu->caratElapsedSeconds = 0.0f;
}

void Menu_DecrementSelectedItem( Menu_t* menu )
{
   menu->selectedItem = ( menu->selectedItem == 0 ) ? menu->numItems - 1 : menu->selectedItem - 1;
   menu->showCarat = True;
   menu->caratElapsedSeconds = 0.0f;
}

void Menu_Tick( Menu_t* menu, Clock_t* clock )
{
   menu->caratElapsedSeconds += clock->frameDeltaSeconds;

   while ( menu->caratElapsedSeconds > menu->caratBlinkSeconds )
   {
      TOGGLE_BOOL( menu->showCarat );
      menu->caratElapsedSeconds -= menu->caratBlinkSeconds;
   }
}
