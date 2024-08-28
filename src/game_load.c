#include "game.h"
#include "assets_file.h"

internal void Game_LoadMenus( GameData_t* gameData );

Bool_t Game_LoadData( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;
   SpriteBase_t* starSpriteBase;
   Player_t* player = &( gameData->player );

   if ( !AssetsFile_Load( gameData ) )
   {
      return False;
   }

   Game_LoadMenus( gameData );

   starSpriteBase = &( gameData->renderData.spriteBases[SpriteBaseID_Star] );

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );
      Sprite_LoadFromBase( &( star->sprite ), starSpriteBase, 0.1f );
      star->isResting = True;
   }

   Player_Init( player, &( gameData->renderData ) );
   player->position.x = 100.0f;
   player->position.y = 0.0f;
   player->maxVelocity.x = 300.0f;
   player->maxVelocity.y = 800.0f;
   player->runAcceleration = 3000.0f;
   player->gravityDeceleration = 3000.0f;
   player->jumpFrameThreshold = 80.0f;
   player->maxJumpExtensionSeconds = 0.2f;
   Player_SetFacingDirection( player, PlayerDirection_Right );

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
