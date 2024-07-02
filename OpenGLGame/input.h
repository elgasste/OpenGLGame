#if !defined( INPUT_H )
#define INPUT_H

#include "common.h"

typedef enum
{
   KeyCode_Left = 0,
   KeyCode_Up,
   KeyCode_Right,
   KeyCode_Down,

   KeyCode_Escape,

   KeyCode_Count
}
KeyCode_t;

typedef struct
{
   Bool_t isDown;
   Bool_t wasDown;
}
KeyState_t;

void Input_Init( KeyState_t* keyStates );
void Input_UpdateStates( KeyState_t* keyStates );
void Input_PressKey( KeyState_t* keyStates, KeyCode_t keyCode );
void Input_ReleaseKey( KeyState_t* keyStates, KeyCode_t keyCode );
Bool_t Input_WasKeyPressed( KeyState_t* keyStates, KeyCode_t keyCode );
Bool_t Input_WasKeyReleased( KeyState_t* keyStates, KeyCode_t keyCode );
Bool_t Input_IsAnyKeyDown( KeyState_t* keyStates );
Bool_t Input_WasAnyKeyPressed( KeyState_t* keyStates );

#endif
