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

      if ( !Sprite_LoadFromBase( &( star->sprite ), starSpriteBase, 0.1f ) )
      {
         return False;
      }

      star->isResting = True;
   }

   Sprite_LoadFromBase( &( player->idleSprites[(uint64_t)PlayerDirection_Left] ),
                        &( gameData->renderData.spriteBases[SpriteBaseID_PlayerIdleLeft] ),
                        0.13f );
   Sprite_LoadFromBase( &( player->idleSprites[(uint64_t)PlayerDirection_Right] ),
                        &( gameData->renderData.spriteBases[SpriteBaseID_PlayerIdleRight] ),
                        0.13f );
   Sprite_LoadFromBase( &( player->runSprites[(uint64_t)PlayerDirection_Left] ),
                        &( gameData->renderData.spriteBases[SpriteBaseID_PlayerMoveLeft] ),
                        0.1f );
   Sprite_LoadFromBase( &( player->runSprites[(uint64_t)PlayerDirection_Right] ),
                        &( gameData->renderData.spriteBases[SpriteBaseID_PlayerMoveRight] ),
                        0.1f );
   Sprite_LoadFromBase( &( player->jumpSprites[(uint64_t)PlayerDirection_Left] ),
                        &( gameData->renderData.spriteBases[SpriteBaseID_PlayerJumpLeft] ),
                        0.0f );
   Sprite_LoadFromBase( &( player->jumpSprites[(uint64_t)PlayerDirection_Right] ),
                        &( gameData->renderData.spriteBases[SpriteBaseID_PlayerJumpRight] ),
                        0.0f );
   Player_Init( player );
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
