#include "input.h"

void Input_Init( InputState_t* inputState )
{
   uint32_t i;
   ButtonState_t* state = inputState->buttonStates;

   for ( i = 0; i < (int)ButtonCode_Count; i++ )
   {
      state->isDown = False;
      state->wasDown = False;
      state++;
   }

   inputState->mouseState.isButtonDragging[MouseButtonCode_Left] = False;
   inputState->mouseState.isButtonDragging[MouseButtonCode_Right] = False;
}

void Input_UpdateState( InputState_t* inputState )
{
   uint32_t i;
   ButtonState_t* state = inputState->buttonStates;

   for ( i = 0; i < (uint32_t)ButtonCode_Count; i++ )
   {
      state->wasDown = state->isDown;
      state++;
   }

   for ( i = 0; i < 2; i++ )
   {
      inputState->mouseState.buttonStates[i].wasDown = inputState->mouseState.buttonStates[i].isDown;
   }
}

void Input_PressButton( InputState_t* inputState, ButtonCode_t buttonCode )
{
   inputState->buttonStates[(uint32_t)buttonCode].isDown = True;
}

void Input_ReleaseButton( InputState_t* inputState, ButtonCode_t buttonCode )
{
   inputState->buttonStates[(uint32_t)buttonCode].isDown = False;
}

void Input_PressMouseButton( MouseState_t* mouseState, MouseButtonCode_t buttonCode )
{
   mouseState->buttonStates[(uint32_t)buttonCode].isDown = True;

   if ( !mouseState->isButtonDragging[(uint32_t)MouseButtonCode_Left] &&
        !mouseState->isButtonDragging[(uint32_t)MouseButtonCode_Right] )
   {
      mouseState->isButtonDragging[(uint32_t)buttonCode] = True;
      mouseState->dragOrigin = mouseState->pointerPos;
   }
}

void Input_ReleaseMouseButton( MouseState_t* mouseState, MouseButtonCode_t buttonCode )
{
   mouseState->buttonStates[(uint32_t)buttonCode].isDown = False;
   mouseState->isButtonDragging[(uint32_t)buttonCode] = False;
}

Bool_t Input_WasButtonPressed( InputState_t* inputState, ButtonCode_t buttonCode )
{
   return inputState->buttonStates[(uint32_t)buttonCode].isDown && !inputState->buttonStates[(uint32_t)buttonCode].wasDown;
}

Bool_t Input_WasButtonReleased( InputState_t* inputState, ButtonCode_t buttonCode )
{
   return !inputState->buttonStates[(uint32_t)buttonCode].isDown && inputState->buttonStates[(uint32_t)buttonCode].wasDown;
}

Bool_t Input_WasMouseButtonPressed( MouseState_t* mouseState, MouseButtonCode_t buttonCode )
{
   return mouseState->buttonStates[(uint32_t)buttonCode].isDown && mouseState->buttonStates[(uint32_t)buttonCode].wasDown;
}

Bool_t Input_WasMouseButtonReleased( MouseState_t* mouseState, MouseButtonCode_t buttonCode )
{
   return !mouseState->buttonStates[(uint32_t)buttonCode].isDown && mouseState->buttonStates[(uint32_t)buttonCode].wasDown;
}

Bool_t Input_IsAnyButtonDown( InputState_t* inputState )
{
   uint32_t i;

   for ( i = 0; i < (uint32_t)ButtonCode_Count; i++ )
   {
      if ( inputState->buttonStates[i].isDown )
      {
         return True;
      }
   }

   return False;
}

Bool_t Input_WasAnyButtonPressed( InputState_t* inputState )
{
   uint32_t i;

   for ( i = 0; i < (uint32_t)ButtonCode_Count; i++ )
   {
      if ( Input_WasButtonPressed( inputState, (ButtonCode_t)i ) )
      {
         return True;
      }
   }

   return False;
}

void Input_SetMousePos( MouseState_t* mouseState, int32_t x, int32_t y )
{
   mouseState->pointerDelta.x = x - mouseState->pointerPos.x;
   mouseState->pointerDelta.y = y - mouseState->pointerPos.y;
   mouseState->pointerPos.x = x;
   mouseState->pointerPos.y = y;
}
