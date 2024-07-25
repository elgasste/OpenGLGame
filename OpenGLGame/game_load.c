#include "game.h"

internal Bool_t Game_LoadAssets( GameData_t* gameData );
internal void Game_LoadMenus( GameData_t* gameData );

Bool_t Game_LoadData( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;

   if ( !Game_LoadAssets( gameData ) )
   {
      return False;
   }

   Game_LoadMenus( gameData );

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );

      if ( !Sprite_Init( &( star->sprite ), &( gameData->renderData.images[ImageID_Star] ), 6, 6, 0.1f ) )
      {
         return False;
      }

      star->isResting = True;
   }

   return True;
}

internal Bool_t Game_LoadAssets( GameData_t* gameData )
{
   char appDirectory[STRING_SIZE_DEFAULT];
   char backgroundBmpFilePath[STRING_SIZE_DEFAULT];
   char starBmpFilePath[STRING_SIZE_DEFAULT];
   char consolasFontFilePath[STRING_SIZE_DEFAULT];
   char papyrusFontFilePath[STRING_SIZE_DEFAULT];

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( backgroundBmpFilePath, STRING_SIZE_DEFAULT, "%sassets\\background.bmp", appDirectory );
   snprintf( starBmpFilePath, STRING_SIZE_DEFAULT, "%sassets\\star.bmp", appDirectory );
   snprintf( consolasFontFilePath, STRING_SIZE_DEFAULT, "%sassets\\fonts\\Consolas.gff", appDirectory );
   snprintf( papyrusFontFilePath, STRING_SIZE_DEFAULT, "%sassets\\fonts\\Papyrus.gff", appDirectory );

   if ( !Image_LoadFromBitmapFile( &( gameData->renderData.images[ImageID_Background] ), backgroundBmpFilePath) ||
        !Image_LoadFromBitmapFile( &( gameData->renderData.images[ImageID_Star] ), starBmpFilePath ) ||
        !Font_LoadFromFile( &( gameData->renderData.fonts[FontID_Consolas] ), consolasFontFilePath ) ||
        !Font_LoadFromFile( &( gameData->renderData.fonts[FontID_Papyrus] ), papyrusFontFilePath ) )
   {
      return False;
   }

   return True;
}

internal void Game_LoadMenus( GameData_t* gameData )
{
   Menu_t* playingMenu = &( gameData->menus[MenuID_Playing] );

   playingMenu->numItems = 2;
   playingMenu->items = (MenuItem_t*)Platform_MemAlloc( sizeof( MenuItem_t ) * playingMenu->numItems );
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