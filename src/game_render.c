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

   if ( gameData->showDiagnostics )
   {
      Game_RenderDiagnostics( gameData );
   }

   Platform_RenderScreen();
}

internal void Game_RenderWorld( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;
   Font_t* font = &( gameData->renderData.fonts[FontID_Papyrus] );

   Blit_Image( &( gameData->renderData.images[ImageID_Background] ), 0.0f, 0.0f, 1.0f );

   Font_SetGlyphCollectionForHeight( font, 48.0f );
   Font_SetColor( font, 0xFF3333CC );
   Blit_TextLine( STR_BRUSHTEETH, 1.0f, 65.0f, 240.0f, font, FontJustify_Left );

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );
      Blit_Sprite( &( star->sprite ), star->position.x, star->position.y, star->scale );
   }
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
      Blit_TextLine( item->text, textScale, renderData->position.x, y, font, FontJustify_Left );

      if ( menu->selectedItem == i )
      {
         Blit_Char( renderData->caratCodepoint, textScale, renderData->position.x + renderData->caratOffset, y, font );
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
   char msg[STRING_SIZE_DEFAULT];

   Font_SetGlyphCollectionForHeight( font, 12.0f );
   Font_SetColor( font, 0xFFFFFFFF );

   y = (float)SCREEN_HEIGHT - font->curGlyphCollection->height - 10.0f;
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_FRAMETARGETMICRO, gameData->clock.targetFrameDurationMicro );
   Blit_TextLine( msg, 1.0f, 10.0f, y, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_FRAMEDURATIONMICRO, gameData->clock.lastFrameDurationMicro );
   Blit_TextLine( msg, 1.0f, 10.0f, y, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_LAGFRAMES, gameData->clock.lagFrames );
   Blit_TextLine( msg, 1.0f, 10.0f, y, font, FontJustify_Left );
   y -= ( font->curGlyphCollection->height + font->curGlyphCollection->lineGap );
   snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_THREADCOUNT, threadQueue->numThreads );
   Blit_TextLine( msg, 1.0f, 10.0f, y, font, FontJustify_Left );
}
