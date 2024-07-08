#if !defined( GAME_H )
#define GAME_H

#define STAR_COUNT            8
#define STAR_MIN_Y            146
#define STAR_MAX_Y            767
#define STAR_MIN_VELOCITY     10
#define STAR_MAX_VELOCITY     500

#include "common.h"
#include "clock.h"
#include "input.h"
#include "render.h"
#include "data_types.h"

typedef struct
{
   Point_t position;
   uint32_t pixelsPerSecond;
   Bool_t movingLeft;
   Bool_t isResting;
   float restSeconds;
}
Star_t;

typedef struct
{
   Clock_t clock;
   KeyState_t keyStates[KeyCode_Count];
   RenderData_t renderData;

   Bool_t isRunning;
   Bool_t isEngineRunning;

   Star_t stars[STAR_COUNT];
}
GameData_t;

Bool_t Game_Init( GameData_t* gameData );
void Game_Run( GameData_t* gameData );
void Game_PauseEngine( GameData_t* gameData );
void Game_ResumeEngine( GameData_t* gameData );
void Game_EmergencySave( GameData_t* gameData );
void Game_TryClose( GameData_t* gameData );

#endif
