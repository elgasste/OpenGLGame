#include "game.h"
#include "blit.h"
#include "thread.h"

internal void Game_RenderWorld( GameData_t* gameData );
internal void Game_RenderMenu( GameData_t* gameData );
internal void Game_RenderDiagnostics( GameData_t* gameData );

void Game_Render( GameData_t* gameData )
{
   Blit_ClearScreen();
   Game_RenderWorld( gameData );

   if ( gameData->state == GameState_Menu )
   {
      Blit_Rect( 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0x99000000 );
      Game_RenderMenu( gameData );
   }

   if ( gameData->diagnosticsData.showDiagnostics )
   {
      Game_RenderDiagnostics( gameData );
   }

   Platform_RenderScreen();
}

internal void Game_RenderWorld( GameData_t* gameData )
{
   Font_t* font = &( gameData->renderData.fonts[FontID_Papyrus] );

   Blit_Image( &( gameData->renderData.images[ImageID_Background] ), 0.0f, 0.0f, 1.0f );

   Font_SetGlyphCollectionForHeight( font, 48.0f );
   Font_SetColor( font, 0xFF3333CC );
   Blit_TextLine( STR_FLYINGWHALES, 65.0f, 240.0f, 1.0f, font, FontJustify_Left );

   // TODO: in a real game we wouldn't be scaling this here
   Blit_Sprite( gameData->player.activeSprite, gameData->player.position.x, gameData->player.position.y, 2.0f );
}

internal void Game_RenderMenu( GameData_t* gameData )
{
   uint32_t i;
   Menu_t* menu = &( gameData->menus[gameData->curMenuID] );
   float y = menu->renderData.position.y;
   float textScale;
   MenuItem_t* item = menu->items;
   MenuRenderData_t* renderData = &( menu->renderData );
   Font_t* font = renderData->font;

   Font_SetGlyphCollectionForHeight( font, menu->renderData.textHeight );
   Font_SetColor( font, menu->renderData.textColor );
   Font_SetCharColor( font, menu->renderData.caratCodepoint, menu->renderData.caratColor );

   textScale = menu->renderData.textHeight / font->curGlyphCollection->height;

   for ( i = 0; i < menu->numItems; i++ )
   {
      Blit_TextLine( item->text, renderData->position.x, y, textScale, font, FontJustify_Left );

      if ( menu->selectedItem == i )
      {
         Blit_Char( renderData->caratCodepoint, renderData->position.x + renderData->caratOffset, y, textScale, font );
      }

      y -= ( font->curGlyphCollection->height + renderData->lineGap );
      item++;
   }
}

internal void Game_RenderDiagnostics( GameData_t* gameData )
{
   float y;
   Font_t* font = &( gameData->renderData.fonts[FontID_Consolas] );
   ThreadQueue_t* threadQueue = Platform_GetThreadQueue();
   Vector2f_t threadCountTextDimensions;

   char msg[STRING_SIZE_DEFAULT];

   Font_SetGlyphCollectionForHeight( font, 12.0f );
   Font_SetColor( font, 0xFFFFFFFF );

   y = (float)SCREEN_HEIGHT - font->curGlyphCollection->height - 10.0f;
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_FRAMETARGETMICRO, gameData->clock.targetFrameDurationMicro );
   Blit_TextLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_FRAMEDURATIONMICRO, gameData->clock.lastFrameDurationMicro );
   Blit_TextLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_LAGFRAMES, gameData->clock.lagFrames );
   Blit_TextLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_THREADCOUNT, threadQueue->numThreads );
   Blit_TextLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );

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
         Blit_TextLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
      }
   }

   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_MOUSEPOS,
             gameData->inputState.mouseState.pointerPos.x,
             gameData->inputState.mouseState.pointerPos.y );
   Blit_TextLine( msg, 10.0f, y, 1.0f, font, FontJustify_Left );
}
