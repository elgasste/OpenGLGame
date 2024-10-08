#if !defined( GAME_H )
#define GAME_H

#define STAR_COUNT            128
#define STAR_MIN_Y            146
#define STAR_MAX_Y            767
#define STAR_MIN_VELOCITY     20
#define STAR_MAX_VELOCITY     500
#define STAR_MAX_RESTSECONDS  5

#include "common.h"
#include "clock.h"
#include "input.h"
#include "render_data.h"
#include "menu.h"
#include "rect.h"
#include "player.h"

typedef struct DiagnosticsData_t
{
   Bool_t showDiagnostics;
   Bool_t showThreadJobs;

   RectF_t threadJobsToggleArea;
}
DiagnosticsData_t;

typedef struct Star_t
{
   Vector2f_t position;
   uint32_t pixelsPerSecond;
   Bool_t movingLeft;
   Bool_t isResting;
   float restSeconds;
   float restElapsedSeconds;
   Sprite_t sprite;
   float scale;
}
Star_t;

typedef struct GameData_t
{
   Clock_t clock;
   InputState_t inputState;
   DiagnosticsData_t diagnosticsData;
   RenderData_t renderData;

   Menu_t menus[MenuID_Count];
   MenuID_t curMenuID;
   void (*menuItemInputHandlers[MenuItemID_Count])( void* );

   Bool_t isRunning;
   Bool_t isEngineRunning;

   GameState_t state;
   void (*stateInputHandlers[GameState_Count])( void* );

   Star_t stars[STAR_COUNT];
   Player_t player;
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
