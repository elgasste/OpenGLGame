#include "game.h"
#include "platform.h"

internal void Game_HandleInput( GameData_t* gameData );
internal void Game_Tick( GameData_t* gameData );
internal void Game_Render( GameData_t* gameData );

void Game_Init( GameData_t* gameData )
{
   Clock_Init( &( gameData->clock ) );
   Input_Init( gameData->keyStates );

   gameData->isRunning = False;
   gameData->isEngineRunning = True;
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
   UNUSED_PARAM( gameData );
}

internal void Game_Render( GameData_t* gameData )
{
   // NOTE: the pixel layout is ARGB
   uint32_t* pixel;
   PixelBuffer_t* screenBuffer;
   uint32_t r, g, b;
   int x, y;

   UNUSED_PARAM( gameData );

   screenBuffer = Platform_GetScreenBuffer();
   pixel = (uint32_t*)screenBuffer->buffer;

   for ( y = 0; y < SCREEN_HEIGHT; y++ )
   {
      b = (int)( (float)y / ( SCREEN_HEIGHT + 1 ) * 255 );
      r = 255 - b;

      for ( x = 0; x < SCREEN_WIDTH; x++ )
      {
         g = (int)( (float)x / ( SCREEN_WIDTH + 1 ) * 255 );
         *pixel = 0xFF000000 | b | ( g << 8 ) | ( r << 16 );
         pixel++;
      }
   }

   Platform_RenderScreen();
}
