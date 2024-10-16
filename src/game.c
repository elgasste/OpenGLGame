#include "game.h"
#include "image.h"
#include "random.h"
#include "rect.h"

internal void Game_HandleInput( GameData_t* gameData );
internal void Game_HandleStateInput_Playing( GameData_t* gameData );
internal void Game_HandleStateInput_Menu( GameData_t* gameData );
internal void Game_HandleMenuItem_KeepPlaying( GameData_t* gameData );
internal void Game_HandleMenuItem_Quit( GameData_t* gameData );
internal void Game_Tick( GameData_t* gameData );

Bool_t Game_Init( GameData_t* gameData )
{
   Assets_Init( &( gameData->assets ) );

   if ( !Game_LoadData( gameData ) )
   {
      return False;
   }

   Clock_Init( &( gameData->clock ) );
   Input_Init( &( gameData->inputState ) );

   gameData->stateInputHandlers[GameState_Playing] = Game_HandleStateInput_Playing;
   gameData->stateInputHandlers[GameState_Menu] = Game_HandleStateInput_Menu;

   gameData->menuItemInputHandlers[MenuItemID_KeepPlaying] = Game_HandleMenuItem_KeepPlaying;
   gameData->menuItemInputHandlers[MenuItemID_Quit] = Game_HandleMenuItem_Quit;

   gameData->isRunning = False;
   gameData->isEngineRunning = True;
   gameData->diagnosticsData.showDiagnostics = False;
   gameData->diagnosticsData.showThreadJobs = False;
   gameData->state = GameState_Playing;
   gameData->curMenuID = (MenuID_t)0;

   return True;
}

void Game_ClearData( GameData_t* gameData )
{
   uint32_t i;
   Image_t* image = gameData->assets.images;
   Font_t* font = gameData->assets.fonts;
   Menu_t* menu = gameData->menus;

   for ( i = 0; i < (uint32_t)ImageID_Count; i++ )
   {
      Image_ClearData( image );
      image++;
   }

   for ( i = 0; i < (uint32_t)FontID_Count; i++ )
   {
      Font_ClearData( font );
      font++;
   }

   for ( i = 0; i < (uint32_t)MenuID_Count; i++ )
   {
      Menu_ClearItems( menu );
      menu++;
   }
}

void Game_Run( GameData_t* gameData )
{
   gameData->isRunning = True;

   while ( gameData->isRunning )
   {
      if ( gameData->isEngineRunning )
      {
         Clock_StartFrame( &( gameData->clock ) );
         Input_UpdateState( &( gameData->inputState ) );
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
   if ( Input_WasButtonPressed( &( gameData->inputState ), ButtonCode_F8 ) )
   {
      TOGGLE_BOOL( gameData->diagnosticsData.showDiagnostics );
   }

   if ( gameData->diagnosticsData.showDiagnostics &&
        Input_WasMouseButtonClicked( &( gameData->inputState.mouseState ), MouseButtonCode_Left ) &&
        Rect_PointInRectF( &( gameData->diagnosticsData.threadJobsToggleArea ),
                           (float)( gameData->inputState.mouseState.pointerPos.x ),
                           (float)( gameData->inputState.mouseState.pointerPos.y ) ) )
   {
      TOGGLE_BOOL( gameData->diagnosticsData.showThreadJobs );
   }

   gameData->stateInputHandlers[gameData->state]( gameData );
}

internal void Game_HandleStateInput_Playing( GameData_t* gameData )
{
   if ( Input_WasButtonPressed( &( gameData->inputState ), ButtonCode_Escape ) )
   {
      gameData->curMenuID = MenuID_Playing;
      Menu_Reset( &( gameData->menus[gameData->curMenuID] ) );
      gameData->state = GameState_Menu;
      gameData->curMenuID = MenuID_Playing;
      return;
   }
}

internal void Game_HandleStateInput_Menu( GameData_t* gameData )
{
   Menu_t* menu = &( gameData->menus[gameData->curMenuID] );

   if ( Input_WasButtonPressed( &( gameData->inputState ), ButtonCode_Escape ) )
   {
      gameData->state = GameState_Playing;
      return;
   }
   else if ( Input_WasButtonPressed( &( gameData->inputState ), ButtonCode_Up ) )
   {
      Menu_DecrementSelectedItem( menu );
   }
   else if ( Input_WasButtonPressed( &( gameData->inputState ), ButtonCode_Down ) )
   {
      Menu_IncrementSelectedItem( menu );
   }
   else if ( Input_WasButtonPressed( &( gameData->inputState ), ButtonCode_Enter ) )
   {
      gameData->menuItemInputHandlers[menu->items[menu->selectedItem].ID]( gameData );
   }
}

internal void Game_HandleMenuItem_KeepPlaying( GameData_t* gameData )
{
   gameData->state = GameState_Playing;
}

internal void Game_HandleMenuItem_Quit( GameData_t* gameData )
{
   Game_TryClose( gameData );
}

internal void Game_Tick( GameData_t* gameData )
{
   switch ( gameData->state )
   {
      case GameState_Menu:
         Menu_Tick( &( gameData->menus[gameData->curMenuID] ), &( gameData->clock ) );
         break;
   }
}
