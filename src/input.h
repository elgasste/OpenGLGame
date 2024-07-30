#if !defined( INPUT_H )
#define INPUT_H

#include "common.h"
#include "vector.h"

typedef enum
{
   ButtonCode_Left = 0,
   ButtonCode_Up,
   ButtonCode_Right,
   ButtonCode_Down,

   ButtonCode_Enter,
   ButtonCode_Escape,

   ButtonCode_F8,

   ButtonCode_MouseLeft,
   ButtonCode_MouseRight,

   ButtonCode_Count
}
ButtonCode_t;

typedef struct ButtonState_t
{
   Bool_t isDown;
   Bool_t wasDown;
}
ButtonState_t;

typedef struct InputState_t
{
   ButtonState_t buttonStates[ButtonCode_Count];
   Vector2i32_t mousePos;
   Vector2i32_t mouseDelta;
}
InputState_t;

void Input_Init( InputState_t* inputState );
void Input_UpdateState( InputState_t* inputState );
void Input_PressButton( InputState_t* inputState, ButtonCode_t buttonCode );
void Input_ReleaseButton( InputState_t* inputState, ButtonCode_t buttonCode );
Bool_t Input_WasButtonPressed( InputState_t* inputState, ButtonCode_t buttonCode );
Bool_t Input_WasButtonReleased( InputState_t* inputState, ButtonCode_t buttonCode );
Bool_t Input_IsAnyButtonDown( InputState_t* inputState );
Bool_t Input_WasAnyButtonPressed( InputState_t* inputState );
void Input_SetMousePos( InputState_t* inputState, int32_t x, int32_t y );

#endif
