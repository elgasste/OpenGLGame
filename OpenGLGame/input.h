#if !defined( INPUT_H )
#define INPUT_H

#include "common.h"

typedef enum
{
   cKeyCode_Left = 0,
   cKeyCode_Up,
   cKeyCode_Right,
   cKeyCode_Down,

   cKeyCode_Escape,

   cKeyCode_Count
}
cKeyCode_t;

typedef struct
{
   cBool_t isDown;
   cBool_t wasDown;
}
cKeyState_t;

void cInput_Init( cKeyState_t* keyStates );
void cInput_UpdateStates( cKeyState_t* keyStates );
void cInput_PressKey( cKeyState_t* keyStates, cKeyCode_t keyCode );
void cInput_ReleaseKey( cKeyState_t* keyStates, cKeyCode_t keyCode );
cBool_t cInput_WasKeyPressed( cKeyState_t* keyStates, cKeyCode_t keyCode );
cBool_t cInput_WasKeyReleased( cKeyState_t* keyStates, cKeyCode_t keyCode );
cBool_t cInput_IsAnyKeyDown( cKeyState_t* keyStates );
cBool_t cInput_WasAnyKeyPressed( cKeyState_t* keyStates );

#endif
