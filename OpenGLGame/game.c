#include "game.h"
#include "random.h"

typedef struct
{
   GameData_t* gameData;
   Star_t* star;
}
StarUpdateData_t;

internal void Game_HandleInput( GameData_t* gameData );
internal void Game_HandleStateInput_Playing( GameData_t* gameData );
internal void Game_HandleStateInput_Menu( GameData_t* gameData );
internal void Game_Tick( GameData_t* gameData );
internal void Game_Render( GameData_t* gameData );
internal void Game_RenderWorld( GameData_t* gameData );
internal void Game_RenderMenu( GameData_t* gameData );
internal void Game_UpdateStarAsync( StarUpdateData_t* data );

Bool_t Game_Init( GameData_t* gameData )
{
   if ( !Game_LoadData( gameData ) )
   {
      return False;
   }

   Clock_Init( &( gameData->clock ) );
   Input_Init( gameData->keyStates );

   gameData->stateInputHandlers[GameState_Playing] = Game_HandleStateInput_Playing;
   gameData->stateInputHandlers[GameState_Menu] = Game_HandleStateInput_Menu;

   gameData->isRunning = False;
   gameData->isEngineRunning = True;
   gameData->showDiagnostics = False;
   gameData->state = GameState_Playing;
   gameData->curMenuID = (MenuID_t)0;

   return True;
}

void Game_ClearData( GameData_t* gameData )
{
   uint32_t i;
   Menu_t* menu = gameData->menus;

   for ( i = 0; i < (uint32_t)MenuID_Count; i++ )
   {
      Menu_ClearItems( menu );
      menu++;
   }

   Render_ClearData( &( gameData->renderData ) );
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
   if ( Input_WasKeyPressed( gameData->keyStates, KeyCode_F8 ) )
   {
      TOGGLE_BOOL( gameData->showDiagnostics );
   }

   gameData->stateInputHandlers[gameData->state]( gameData );
}

internal void Game_HandleStateInput_Playing( GameData_t* gameData )
{
   if ( Input_WasKeyPressed( gameData->keyStates, KeyCode_Escape ) )
   {
      gameData->state = GameState_Menu;
      gameData->curMenuID = MenuID_Playing;
   }
}

internal void Game_HandleStateInput_Menu( GameData_t* gameData )
{
   // TODO: handle scrolling and selection

   if ( Input_WasKeyPressed( gameData->keyStates, KeyCode_Escape ) )
   {
      gameData->state = GameState_Playing;
   }
}

internal void Game_Tick( GameData_t* gameData )
{
   uint32_t i, entryCounter = 0;
   StarUpdateData_t dataArray[MAX_THREADQUEUE_SIZE];

   if ( gameData->state == GameState_Playing )
   {
      for ( i = 0; i < STAR_COUNT; i++ )
      {
         dataArray[entryCounter].gameData = gameData;
         dataArray[entryCounter].star = &( gameData->stars[i] );
         Platform_AddThreadQueueEntry( Game_UpdateStarAsync, (void*)( &( dataArray[entryCounter] ) ) );
         entryCounter++;

         if ( entryCounter == MAX_THREADQUEUE_SIZE || i == ( STAR_COUNT - 1 ) )
         {
            Platform_RunThreadQueue();
            entryCounter = 0;
         }
      }
   }
}

internal void Game_Render( GameData_t* gameData )
{
   switch ( gameData->state )
   {
      case GameState_Playing:
         Game_RenderWorld( gameData );
         break;
      case GameState_Menu:
         Game_RenderWorld( gameData );
         Game_RenderMenu( gameData );
         break;
   }

   Platform_RenderScreen();
}

internal void Game_RenderWorld( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;
   Font_t* consolasFont = & ( gameData->renderData.fonts[FontID_Consolas] );
   Font_t* papyrusFont = &( gameData->renderData.fonts[FontID_Papyrus] );
   float y;
   char msg[STRING_SIZE_DEFAULT];

   Render_ClearScreen();
   Render_DrawTexture( &( gameData->renderData.textures[TextureID_Background] ), 1.0f, 0.0f, 0.0f );
   Render_DrawTextLine( STR_BRUSHTEETH, 1.0f, 65.0f, 240.0f, papyrusFont );

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );
      Render_DrawSprite( &( star->sprite ), star->scale, star->position.x, star->position.y );
   }

   if ( gameData->showDiagnostics )
   {
      y = (float)SCREEN_HEIGHT - consolasFont->curGlyphCollection->height - 10.0f;
      snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_FRAMETARGETMICRO, gameData->clock.targetFrameDurationMicro );
      Render_DrawTextLine( msg, 1.0f, 10.0f, y, consolasFont );
      y -= ( consolasFont->curGlyphCollection->height + consolasFont->curGlyphCollection->lineGap );
      snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_FRAMEDURATIONMICRO, gameData->clock.lastFrameDurationMicro );
      Render_DrawTextLine( msg, 1.0f, 10.0f, y, consolasFont );
      y -= ( consolasFont->curGlyphCollection->height + consolasFont->curGlyphCollection->lineGap );
      snprintf( msg, STRING_SIZE_DEFAULT, STR_DIAG_LAGFRAMES, gameData->clock.lagFrames );
      Render_DrawTextLine( msg, 1.0f, 10.0f, y, consolasFont );
   }
}

internal void Game_RenderMenu( GameData_t* gameData )
{
   // TODO: actually draw the menu
   UNUSED_PARAM( gameData );
}

internal void Game_UpdateStarAsync( StarUpdateData_t* data )
{
   float frameTimeAdjustment;
   GameData_t* gameData = data->gameData;
   Star_t* star = data->star;

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
