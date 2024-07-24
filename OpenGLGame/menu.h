#if !defined( MENU_H )
#define MENU_H

#include "common.h"

typedef struct
{
   char text[STRING_SIZE_DEFAULT];
}
MenuItem_t;

typedef struct
{
   MenuItem_t* items;
   uint32_t numItems;
   uint32_t selectedItem;
}
Menu_t;

void Menu_ClearItems( Menu_t* menu );

#endif
