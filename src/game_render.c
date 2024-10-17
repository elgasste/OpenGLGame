#include "game.h"
#include "blit.h"
#include "thread.h"

internal void Game_RenderWorld( GameData_t* gameData );
internal void Game_RenderDiagnostics( GameData_t* gameData );

void Game_Render( GameData_t* gameData )
{
   Blit_ClearScreen();
   Game_RenderWorld( gameData );

   if ( gameData->state == GameState_Menu )
   {
      Menu_Render( &( gameData->menus[gameData->curMenuID] ), 1.0f );
   }

   if ( gameData->diagnosticsData.showDiagnostics )
   {
      Game_RenderDiagnostics( gameData );
   }

   Platform_RenderScreen();
}

internal void Game_RenderWorld( GameData_t* gameData )
{
   UNUSED_PARAM( gameData );
}

internal void Game_RenderDiagnostics( GameData_t* gameData )
{
   float y;
   Font_t* font = &( gameData->assets.fonts[FontID_Consolas] );
   ThreadQueue_t* threadQueue = Platform_GetThreadQueue();
   Vector2f_t threadCountTextDimensions;

   char msg[STRING_SIZE_DEFAULT];

   Font_SetGlyphCollectionForHeight( font, 12.0f );
   Font_SetColor( font, 0xFFFFFFFF );

   y = (float)SCREEN_HEIGHT - font->curGlyphCollection->height - 10.0f;
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_FRAMETARGETMICRO, gameData->clock.targetFrameDurationMicro );
   Blit_FontLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_FRAMEDURATIONMICRO, gameData->clock.lastFrameDurationMicro );
   Blit_FontLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_LAGFRAMES, gameData->clock.lagFrames );
   Blit_FontLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_THREADCOUNT, threadQueue->numThreads );
   Blit_FontLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );

   threadCountTextDimensions = Font_GetTextDimensions( font, msg );
   gameData->diagnosticsData.threadJobsToggleArea.pos.x = 10.0f;
   gameData->diagnosticsData.threadJobsToggleArea.pos.y = y;
   gameData->diagnosticsData.threadJobsToggleArea.size.x = threadCountTextDimensions.x;
   gameData->diagnosticsData.threadJobsToggleArea.size.y = threadCountTextDimensions.y;

   if ( gameData->diagnosticsData.showThreadJobs )
   {
      for ( uint32_t i = 0; i < threadQueue->numThreads; i++ )
      {
         y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
         snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_THREADJOBSDONE, i, Platform_GetJobsDoneByThread( i ) );
         Blit_FontLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
      }
   }

   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_MOUSEPOS,
             gameData->inputState.mouseState.pointerPos.x,
             gameData->inputState.mouseState.pointerPos.y );
   Blit_FontLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
}
