#if !defined( MENU_H )
#define MENU_H

#include "common.h"
#include "vector.h"

typedef struct Font_t Font_t;
typedef struct Clock_t Clock_t;

typedef struct MenuRenderData_t
{
   Font_t* font;
   uint32_t caratCodepoint;
   Vector2f_t position;
   float textHeight;
   float lineGap;
   float caratOffset;
   uint32_t textColor;
   uint32_t caratColor;
}
MenuRenderData_t;

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
   MenuRenderData_t renderData;
   Bool_t caratFadingOut;
   float caratElapsedSeconds;
   float caratFadeSeconds;
}
Menu_t;

void Menu_ClearItems( Menu_t* menu );
void Menu_Reset( Menu_t* menu );
void Menu_IncrementSelectedItem( Menu_t* menu );
void Menu_DecrementSelectedItem( Menu_t* menu );
void Menu_Tick( Menu_t* menu, Clock_t* clock );

#endif
