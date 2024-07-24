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
