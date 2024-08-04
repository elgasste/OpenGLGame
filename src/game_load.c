#include "game.h"
#include "assets_file.h"

internal void Game_LoadMenus( GameData_t* gameData );
internal void Game_LoadTilemap( GameData_t* gameData );

Bool_t Game_LoadData( GameData_t* gameData )
{
   if ( !AssetsFile_Load( gameData ) )
   {
      return False;
   }

   Game_LoadTilemap( gameData );
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
   playingMenu->renderData.font = &( gameData->renderData.fonts[FontID_Consolas] );
   playingMenu->renderData.caratCodepoint = (uint32_t)( '>' );
   playingMenu->renderData.position.x = 200.0f;
   playingMenu->renderData.position.y = 500.0f;
   playingMenu->renderData.textHeight = 24.0f;
   playingMenu->renderData.lineGap = playingMenu->renderData.font->curGlyphCollection->lineGap;
   playingMenu->renderData.caratOffset = -30.0f;
   playingMenu->renderData.textColor = 0xFFFF8800;
   playingMenu->renderData.caratColor = 0xFFFF8800;
   playingMenu->caratFadeSeconds = 0.25f;
}

internal void Game_LoadTilemap( GameData_t* gameData )
{
   Tileset_Init( &( gameData->renderData.tileset ), &( gameData->renderData.images[ImageID_Tileset] ), ImageID_Tileset, 32, 32 );
   Tilemap_Init( &( gameData->tilemap ), &( gameData->renderData.tileset ), 10, 20 );
}
