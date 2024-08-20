#if !defined( INPUT_H )
#define INPUT_H

#include "common.h"
#include "vector.h"

typedef struct ButtonState_t
{
   Bool_t isDown;
   Bool_t wasDown;
}
ButtonState_t;

typedef struct MouseState_t
{
   Vector2i32_t pointerPos;
   Vector2i32_t pointerDelta;
   ButtonState_t buttonStates[2];
   Bool_t isButtonDragging[2];
   Vector2i32_t dragOrigin;
}
MouseState_t;

typedef struct InputState_t
{
   ButtonState_t buttonStates[ButtonCode_Count];
   MouseState_t mouseState;
}
InputState_t;

void Input_Init( InputState_t* inputState );
void Input_UpdateState( InputState_t* inputState );
void Input_PressButton( InputState_t* inputState, ButtonCode_t buttonCode );
void Input_ReleaseButton( InputState_t* inputState, ButtonCode_t buttonCode );
void Input_PressMouseButton( MouseState_t* mouseState, MouseButtonCode_t buttonCode );
void Input_ReleaseMouseButton( MouseState_t* mouseState, MouseButtonCode_t buttonCode );
Bool_t Input_WasButtonPressed( InputState_t* inputState, ButtonCode_t buttonCode );
Bool_t Input_WasButtonReleased( InputState_t* inputState, ButtonCode_t buttonCode );
Bool_t Input_WasMouseButtonPressed( MouseState_t* mouseState, MouseButtonCode_t buttonCode );
Bool_t Input_WasMouseButtonReleased( MouseState_t* mouseState, MouseButtonCode_t buttonCode );
Bool_t Input_IsAnyButtonDown( InputState_t* inputState );
Bool_t Input_WasAnyButtonPressed( InputState_t* inputState );
void Input_SetMousePos( MouseState_t* mouseState, int32_t x, int32_t y );

#endif
