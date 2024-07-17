#include "game.h"
#include "random.h"

internal Bool_t Game_LoadAssets( GameData_t* gameData );
internal void Game_HandleInput( GameData_t* gameData );
internal void Game_Tick( GameData_t* gameData );
internal void Game_Render( GameData_t* gameData );

Bool_t Game_Init( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;

   Random_Seed();
   Clock_Init( &( gameData->clock ) );
   Input_Init( gameData->keyStates );

   if ( !Game_LoadAssets( gameData ) )
   {
      return False;
   }

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );

      if ( !Sprite_Init( &( star->sprite ), &( gameData->renderData.textures[TextureID_Star] ), 6, 6, 0.1f ) )
      {
         return False;
      }

      star->isResting = True;
   }

   gameData->isRunning = False;
   gameData->isEngineRunning = True;

   return True;
}

Bool_t Game_LoadAssets( GameData_t* gameData )
{
   char appDirectory[STRING_SIZE_DEFAULT];
   char backgroundBmpFilePath[STRING_SIZE_DEFAULT];
   char starBmpFilePath[STRING_SIZE_DEFAULT];
   char fontFilePath[STRING_SIZE_DEFAULT];

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( backgroundBmpFilePath, STRING_SIZE_DEFAULT, "%sassets\\background.bmp", appDirectory );
   snprintf( starBmpFilePath, STRING_SIZE_DEFAULT, "%sassets\\star.bmp", appDirectory );
   snprintf( fontFilePath, STRING_SIZE_DEFAULT, "%sassets\\fonts\\Consolas.gff", appDirectory );

   if ( !Texture_LoadFromFile( &( gameData->renderData.textures[TextureID_Background] ), backgroundBmpFilePath) ||
        !Texture_LoadFromFile( &( gameData->renderData.textures[TextureID_Star] ), starBmpFilePath ) ||
        !Font_LoadFromFile( &( gameData->renderData.font ), fontFilePath ) )
   {
      return False;
   }

   Font_SetColor( &( gameData->renderData.font ), 0x003333CC );

   return True;
}

void Game_Run( GameData_t* gameData )
{
   gameData->isRunning = True;

   while ( gameData->isRunning )
   {
      if ( gameData->isEngineRunning )
      {
         Clock_StartFrame( &( gameData->clock ) );
         Input_UpdateStates( gameData->keyStates );
         Platform_Tick();
         Game_HandleInput( gameData );
         Game_Tick( gameData );
         Game_Render( gameData );
         Clock_EndFrame( &( gameData->clock ) );
      }
      else
      {
         Platform_Tick();
      }
   }
}

void Game_PauseEngine( GameData_t* gameData )
{
   if ( gameData->isEngineRunning )
   {
      Clock_Pause( &( gameData->clock ) );
      gameData->isEngineRunning = False;
   }
}

void Game_ResumeEngine( GameData_t* gameData )
{
   if ( !gameData->isEngineRunning )
   {
      gameData->isEngineRunning = True;
      Clock_Resume( &( gameData->clock ) );
   }
}

void Game_EmergencySave( GameData_t* gameData )
{
   // NOTE: this means something went wrong, we should try to salvage whatever we can
   gameData->isRunning = False;
}

void Game_TryClose( GameData_t* gameData )
{
   // NOTE: the user could be trying to exit the game prematurely,
   // so give them the chance to save or whatever before it happens.
   gameData->isRunning = False;
}

internal void Game_HandleInput( GameData_t* gameData )
{
   if ( Input_WasKeyPressed( gameData->keyStates, KeyCode_Escape ) )
   {
      Game_TryClose( gameData );
   }
}

internal void Game_Tick( GameData_t* gameData )
{
   uint32_t i;
   float frameTimeAdjustment;

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      Star_t* star = &( gameData->stars[i] );

      if ( star->isResting )
      {
         star->restElapsedSeconds += gameData->clock.frameDeltaSeconds;

         if ( star->restElapsedSeconds > star->restSeconds )
         {
            star->isResting = False;
            star->movingLeft = Random_Bool();
            star->pixelsPerSecond = Random_UInt32( STAR_MIN_VELOCITY, STAR_MAX_VELOCITY );
            star->position.x = star->movingLeft ? SCREEN_WIDTH : -(float)( star->sprite.frameDimensions.x - 1 );
            star->position.y = (float)Random_UInt32( STAR_MIN_Y, STAR_MAX_Y );
            star->restSeconds = ( Random_UInt32( 0, STAR_MAX_RESTSECONDS * 1000 ) ) / 1000.0f;

            Sprite_Reset( &( star->sprite ) );
            frameTimeAdjustment = ( (float)Random_Percent() / 100 ) * 0.5f;
            star->scale = ( (float)Random_Percent() / 100 );
            Sprite_ScaleFrameTime( &( star->sprite ), 1.0f + ( Random_Bool() ? frameTimeAdjustment : -frameTimeAdjustment ) );
         }
      }
      else
      {
         star->position.x = star->movingLeft
            ? star->position.x - (float)star->pixelsPerSecond * gameData->clock.frameDeltaSeconds
            : star->position.x + (float)star->pixelsPerSecond * gameData->clock.frameDeltaSeconds;
         
         if ( star->position.x < -(float)( star->sprite.frameDimensions.x ) ||
              star->position.x > SCREEN_WIDTH )
         {
            star->isResting = True;
         }
         else
         {
            Sprite_Tick( &( star->sprite ), &( gameData->clock ) );
         }
      }
   }
}

internal void Game_Render( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;

   Render_Clear();
   Render_DrawTexture( &( gameData->renderData.textures[TextureID_Background] ), 1.0f, 0, 0 );
   Render_DrawTextLine( "(...But don't forget to brush your teeth!)", 0.18f, 40, 240, &( gameData->renderData.font ) );

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );
      Render_DrawSprite( &( star->sprite ), star->scale, (uint32_t)( star->position.x ), (uint32_t)( star->position.y ) );
   }

   Platform_RenderScreen();
}
