#if !defined( MENU_H )
#define MENU_H

#include "common.h"
#include "font.h"
#include "vector.h"
#include "clock.h"

typedef struct
{
   Font_t* font;
   uint32_t caratCodepoint;
   Vector2f_t position;
   float lineGap;
   float caratOffset;
}
MenuRenderData_t;

typedef struct
{
   MenuItemID_t ID;
   char text[STRING_SIZE_DEFAULT];
}
MenuItem_t;

typedef struct
{
   MenuItem_t* items;
   uint32_t numItems;
   uint32_t selectedItem;
   MenuRenderData_t renderData;
   Bool_t showCarat;
   float caratElapsedSeconds;
   float caratBlinkSeconds;
}
Menu_t;

void Menu_ClearItems( Menu_t* menu );
void Menu_Reset( Menu_t* menu );
void Menu_IncrementSelectedItem( Menu_t* menu );
void Menu_DecrementSelectedItem( Menu_t* menu );
void Menu_Tick( Menu_t* menu, Clock_t* clock );

#endif
