#include "menu.h"
#include "clock.h"
#include "platform.h"

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
