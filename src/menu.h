#if !defined( MENU_H )
#define MENU_H

#include "common.h"
#include "dialog_border.h"

typedef struct Clock_t Clock_t;

typedef struct MenuItem_t
{
   MenuItemID_t ID;
   char text[STRING_SIZE_DEFAULT];
}
MenuItem_t;

typedef struct Menu_t
{
   MenuItem_t* items;
   uint32_t numItems;
   uint32_t selectedItem;

   Vector2f_t position;
   DialogBorder_t border;
   uint32_t lineGap;
   uint32_t caratOffset;
   char caratChar;

   Bool_t showCarat;
   float caratBlinkSeconds;
   float elapsedSeconds;
}
Menu_t;

void Menu_ClearItems( Menu_t* menu );
void Menu_Reset( Menu_t* menu );
void Menu_IncrementSelectedItem( Menu_t* menu );
void Menu_DecrementSelectedItem( Menu_t* menu );
void Menu_Tick( Menu_t* menu, Clock_t* clock );

#endif
