#if !defined( GAME_H )
#define GAME_H

#define STAR_COUNT            1024
#define STAR_MIN_Y            146
#define STAR_MAX_Y            767
#define STAR_MIN_VELOCITY     20
#define STAR_MAX_VELOCITY     500
#define STAR_MAX_RESTSECONDS  5

#include "common.h"
#include "clock.h"
#include "input.h"
#include "render.h"
#include "vector.h"
#include "menu.h"

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
   KeyState_t keyStates[KeyCode_Count];
   RenderData_t renderData;
   Menu_t menus[MenuID_Count];
   MenuID_t curMenuID;

   Bool_t isRunning;
   Bool_t isEngineRunning;
   Bool_t showDiagnostics;

   GameState_t state;
   void (*stateInputHandlers[GameState_Count])( void* );
   Star_t stars[STAR_COUNT];
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

#endif
