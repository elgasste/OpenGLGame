#if !defined( GAME_H )
#define GAME_H

#include "common.h"
#include "clock.h"
#include "input.h"
#include "render.h"

typedef struct
{
   Clock_t clock;
   KeyState_t keyStates[KeyCode_Count];
   RenderData_t renderData;

   Bool_t isRunning;
   Bool_t isEngineRunning;
}
GameData_t;

void Game_Init( GameData_t* gameData );
void Game_Run( GameData_t* gameData );
void Game_PauseEngine( GameData_t* gameData );
void Game_ResumeEngine( GameData_t* gameData );
void Game_EmergencySave( GameData_t* gameData );
void Game_TryClose( GameData_t* gameData );

#endif
