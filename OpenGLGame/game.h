#if !defined( GAME_H )
#define GAME_H

#include "common.h"
#include "clock.h"
#include "input.h"

typedef struct
{
   cClock_t clock;
   cKeyState_t keyStates[cKeyCode_Count];
   cBool_t isRunning;
   cBool_t isEngineRunning;
}
cGameData_t;

void cGame_Init( cGameData_t* gameData );
void cGame_Run( cGameData_t* gameData );
void cGame_PauseEngine( cGameData_t* gameData );
void cGame_ResumeEngine( cGameData_t* gameData );
void cGame_EmergencySave( cGameData_t* gameData );
void cGame_TryClose( cGameData_t* gameData );

#endif
