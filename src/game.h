#if !defined( GAME_H )
#define GAME_H

#include "common.h"
#include "clock.h"
#include "input.h"
#include "image.h"
#include "sprite.h"
#include "font.h"
#include "menu.h"
#include "rect.h"
#include "tileset.h"
#include "tilemap.h"

typedef struct DiagnosticsData_t
{
   Bool_t showDiagnostics;
   Bool_t showThreadJobs;

   RectF_t threadJobsToggleArea;
}
DiagnosticsData_t;

typedef struct GameRenderData_t
{
   Image_t images[ImageID_Count];
   Font_t fonts[FontID_Count];
   SpriteBase_t spriteBases[SpriteBaseID_Count];
   Tileset_t tileset;
}
GameRenderData_t;

typedef struct GameData_t
{
   Clock_t clock;
   InputState_t inputState;
   DiagnosticsData_t diagnosticsData;
   GameRenderData_t renderData;

   Menu_t menus[MenuID_Count];
   MenuID_t curMenuID;
   void (*menuItemInputHandlers[MenuItemID_Count])( void* );

   Tilemap_t tilemap;

   Bool_t isRunning;
   Bool_t isEngineRunning;

   GameState_t state;
   void (*stateInputHandlers[GameState_Count])( void* );
}
GameData_t;

Bool_t Game_Init( GameData_t* gameData );
void Game_ClearData( GameData_t* gameData );
void Game_Run( GameData_t* gameData );
void Game_PauseEngine( GameData_t* gameData );
void Game_ResumeEngine( GameData_t* gameData );
void Game_EmergencySave( GameData_t* gameData );
void Game_TryClose( GameData_t* gameData );

// game_loader.c
Bool_t Game_LoadData( GameData_t* gameData );

// game_render.c
void Game_Render( GameData_t* gameData );

#endif
