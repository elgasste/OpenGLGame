#include "game.h"
#include "platform.h"

internal void cGame_HandleInput( cGameData_t* gameData );
internal void cGame_Tick( cGameData_t* gameData );
internal void cGame_Render( cGameData_t* gameData );

void cGame_Init( cGameData_t* gameData )
{
   cClock_Init( &( gameData->clock ) );
   cInput_Init( gameData->keyStates );

   gameData->isRunning = cFalse;
   gameData->isEngineRunning = cTrue;
}

void cGame_Run( cGameData_t* gameData )
{
   gameData->isRunning = cTrue;

   while ( gameData->isRunning )
   {
      if ( gameData->isEngineRunning )
      {
         cClock_StartFrame( &( gameData->clock ) );
         cInput_UpdateStates( gameData->keyStates );
         Platform_Tick();
         cGame_HandleInput( gameData );
         cGame_Tick( gameData );
         cGame_Render( gameData );
         cClock_EndFrame( &( gameData->clock ) );
      }
      else
      {
         Platform_Tick();
      }
   }
}

void cGame_PauseEngine( cGameData_t* gameData )
{
   if ( gameData->isEngineRunning )
   {
      cClock_Pause( &( gameData->clock ) );
      gameData->isEngineRunning = cFalse;
   }
}

void cGame_ResumeEngine( cGameData_t* gameData )
{
   if ( !gameData->isEngineRunning )
   {
      gameData->isEngineRunning = cTrue;
      cClock_Resume( &( gameData->clock ) );
   }
}

void cGame_EmergencySave( cGameData_t* gameData )
{
   // NOTE: this means something went wrong, we should try to salvage whatever we can
   gameData->isRunning = cFalse;
}

void cGame_TryClose( cGameData_t* gameData )
{
   // NOTE: this means the user is trying to exit the game prematurely,
   // so give them the chance to save or whatever before it happens.
   gameData->isRunning = cFalse;
}

internal void cGame_HandleInput( cGameData_t* gameData )
{
   if ( cInput_WasKeyPressed( gameData->keyStates, cKeyCode_Escape ) )
   {
      cGame_TryClose( gameData );
   }
}

internal void cGame_Tick( cGameData_t* gameData )
{
   UNUSED_PARAM( gameData );
}

internal void cGame_Render( cGameData_t* gameData )
{
   // NOTE: the pixel layout is 0xAARRGGBB
   uint32_t* pixel;
   uint8_t* rowBuffer;
   cScreenBuffer_t* buffer;
   uint32_t r, g, b;
   int x, y;

   UNUSED_PARAM( gameData );

   buffer = Platform_GetScreenBuffer();
   rowBuffer = (uint8_t*)buffer->memory;

   for ( y = 0; y < SCREEN_BUFFER_HEIGHT; y++ )
   {
      b = (int)( (float)y / ( SCREEN_BUFFER_HEIGHT + 1 ) * 255 );
      r = 255 - b;
      pixel = (uint32_t*)rowBuffer;

      for ( x = 0; x < SCREEN_BUFFER_WIDTH; x++ )
      {
         g = (int)( (float)x / ( SCREEN_BUFFER_WIDTH + 1 ) * 255 );

         *pixel = 0xFF000000 | b | ( g << 8 ) | ( r << 16 );
         pixel++;
      }

      rowBuffer += buffer->pitch;
   }

   Platform_RenderScreen();
}
