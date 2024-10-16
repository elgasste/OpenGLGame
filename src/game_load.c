#include "game.h"
#include "assets_file.h"

internal void Game_LoadMenus( GameData_t* gameData );

Bool_t Game_LoadData( GameData_t* gameData )
{
   if ( !AssetsFile_Load( gameData ) )
   {
      return False;
   }

   Game_LoadMenus( gameData );

   return True;
}

internal void Game_LoadMenus( GameData_t* gameData )
{
   Menu_t* playingMenu = &( gameData->menus[MenuID_Playing] );

   playingMenu->numItems = 2;
   playingMenu->items = (MenuItem_t*)Platform_MAlloc( sizeof( MenuItem_t ) * playingMenu->numItems );
   playingMenu->items[0].ID = MenuItemID_KeepPlaying;
   snprintf( playingMenu->items[0].text, STRING_SIZE_DEFAULT, STR_MENU_KEEPPLAYING );
   playingMenu->items[1].ID = MenuItemID_Quit;
   snprintf( playingMenu->items[1].text, STRING_SIZE_DEFAULT, STR_MENU_QUIT );
   DialogBorder_Init( &( playingMenu->border ), &( gameData->assets.images[ImageID_MessageBoxBorders] ), &( gameData->assets.textMap ), 20, 20, 3 );
   playingMenu->textMap = &( gameData->assets.textMap );
   playingMenu->caratChar = '>';
   playingMenu->position.x = 100.0f;
   playingMenu->position.y = 100.0f;
   playingMenu->lineGap = 4;
   playingMenu->caratOffset = 1;
   playingMenu->caratBlinkSeconds = 0.25f;
}
