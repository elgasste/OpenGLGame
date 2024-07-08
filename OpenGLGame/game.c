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

   Clock_Init( &( gameData->clock ) );
   Input_Init( gameData->keyStates );

   if ( !Game_LoadAssets( gameData ) )
   {
      return False;
   }

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );

      star->isResting = Random_Bool();

      if ( star->isResting )
      {
         star->restElapsedSeconds = 0.0f;
         star->restSeconds = ( Random_UInt32( 0, STAR_MAX_RESTSECONDS * 1000 ) ) / 1000.0f;
      }

      star->movingLeft = Random_Bool();
      star->pixelsPerSecond = Random_UInt32( STAR_MIN_VELOCITY, STAR_MAX_VELOCITY );
      star->position.x = (float)Random_UInt32( 0, SCREEN_WIDTH - 1 );
      star->position.y = (float)Random_UInt32( STAR_MIN_Y, STAR_MAX_Y );
   }

   gameData->isRunning = False;
   gameData->isEngineRunning = True;

   return True;
}

Bool_t Game_LoadAssets( GameData_t* gameData )
{
   char appDirectory[STRING_SIZE_DEFAULT];
   char backgroundFilePath[STRING_SIZE_DEFAULT];
   char starFilePath[STRING_SIZE_DEFAULT];

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( backgroundFilePath, STRING_SIZE_DEFAULT, "%sassets\\background.bmp", appDirectory );
   snprintf( starFilePath, STRING_SIZE_DEFAULT, "%sassets\\star.bmp", appDirectory );

   if ( !Render_LoadTextureFromFile( &( gameData->renderData.backgroundTexture ), backgroundFilePath ) ||
        !Render_LoadTextureFromFile( &( gameData->renderData.starTexture ), starFilePath ) )
   {
      return False;
   }

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
            star->position.x = star->movingLeft ? SCREEN_WIDTH : -(float)( gameData->renderData.starTexture.pixelBuffer.width - 1 );
            star->position.y = (float)Random_UInt32( STAR_MIN_Y, STAR_MAX_Y );
            star->restSeconds = ( Random_UInt32( 0, STAR_MAX_RESTSECONDS * 1000 ) ) / 1000.0f;
         }
      }
      else
      {
         star->position.x = star->movingLeft
            ? star->position.x - (float)star->pixelsPerSecond * gameData->clock.frameDeltaSeconds
            : star->position.x + (float)star->pixelsPerSecond * gameData->clock.frameDeltaSeconds;
         
         if ( star->position.x < -(float)( gameData->renderData.starTexture.pixelBuffer.width ) ||
              star->position.x > SCREEN_WIDTH )
         {
            star->isResting = True;
         }
      }
   }
}

internal void Game_Render( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;

   Render_Clear();
   Render_DrawTexture( 0, 0, &( gameData->renderData.backgroundTexture ) );

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );
      Render_DrawTexture( (uint32_t)( star->position.x ), (uint32_t)( star->position.y ), &( gameData->renderData.starTexture ) );
   }

   Platform_RenderScreen();
}
