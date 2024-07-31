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
}

void Input_PressButton( InputState_t* inputState, ButtonCode_t buttonCode )
{
   inputState->buttonStates[(uint32_t)buttonCode].isDown = True;
}

void Input_ReleaseButton( InputState_t* inputState, ButtonCode_t buttonCode )
{
   inputState->buttonStates[(uint32_t)buttonCode].isDown = False;
}

Bool_t Input_WasButtonPressed( InputState_t* inputState, ButtonCode_t buttonCode )
{
   return inputState->buttonStates[(uint32_t)buttonCode].isDown && !inputState->buttonStates[(uint32_t)buttonCode].wasDown;
}

Bool_t Input_WasButtonReleased( InputState_t* inputState, ButtonCode_t buttonCode )
{
   return !inputState->buttonStates[(uint32_t)buttonCode].isDown && inputState->buttonStates[(uint32_t)buttonCode].wasDown;
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

void Input_SetMousePos( InputState_t* inputState, int32_t x, int32_t y )
{
   inputState->mouseDelta.x = x - inputState->mousePos.x;
   inputState->mouseDelta.y = y - inputState->mousePos.y;
   inputState->mousePos.x = x;
   inputState->mousePos.y = y;
}
