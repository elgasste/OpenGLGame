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
}

void Menu_IncrementSelectedItem( Menu_t* menu )
{
   menu->selectedItem++;

   if ( menu->selectedItem == menu->numItems )
   {
      menu->selectedItem = 0;
   }
}

void Menu_DecrementSelectedItem( Menu_t* menu )
{
   menu->selectedItem = ( menu->selectedItem == 0 ) ? menu->numItems - 1 : menu->selectedItem - 1;
}
