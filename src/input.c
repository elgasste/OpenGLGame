#include <math.h>

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

   for ( i = 0; i < 2; i++ )
   {
      inputState->mouseState.dragStates[i].isDragging = False;
      inputState->mouseState.dragStates[i].wasDragging = False;
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

   for ( i = 0; i < 2; i++ )
   {
      inputState->mouseState.buttonStates[i].wasDown = inputState->mouseState.buttonStates[i].isDown;
      inputState->mouseState.dragStates[i].wasDragging = inputState->mouseState.dragStates[i].isDragging;
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

   if ( !mouseState->dragStates[(uint32_t)MouseButtonCode_Left].isDragging &&
        !mouseState->dragStates[(uint32_t)MouseButtonCode_Right].isDragging )
   {
      mouseState->dragStates[(uint32_t)buttonCode].isDragging = True;
      mouseState->dragOrigin = mouseState->pointerPos;
   }
}

void Input_ReleaseMouseButton( MouseState_t* mouseState, MouseButtonCode_t buttonCode )
{
   mouseState->buttonStates[(uint32_t)buttonCode].isDown = False;
   mouseState->dragStates[(uint32_t)buttonCode].isDragging = False;
}

Bool_t Input_WasButtonPressed( InputState_t* inputState, ButtonCode_t buttonCode )
{
   return ( inputState->buttonStates[(uint32_t)buttonCode].isDown && !inputState->buttonStates[(uint32_t)buttonCode].wasDown ) ? True : False;
}

Bool_t Input_WasButtonReleased( InputState_t* inputState, ButtonCode_t buttonCode )
{
   return ( !inputState->buttonStates[(uint32_t)buttonCode].isDown && inputState->buttonStates[(uint32_t)buttonCode].wasDown ) ? True : False;
}

Bool_t Input_WasMouseButtonPressed( MouseState_t* mouseState, MouseButtonCode_t buttonCode )
{
   return ( mouseState->buttonStates[(uint32_t)buttonCode].isDown && mouseState->buttonStates[(uint32_t)buttonCode].wasDown ) ? True : False;
}

Bool_t Input_WasMouseButtonReleased( MouseState_t* mouseState, MouseButtonCode_t buttonCode )
{
   return ( !mouseState->buttonStates[(uint32_t)buttonCode].isDown && mouseState->buttonStates[(uint32_t)buttonCode].wasDown ) ? True : False;
}

Bool_t Input_WasMouseButtonClicked( MouseState_t* mouseState, MouseButtonCode_t buttonCode )
{
   uint32_t deltaX, deltaY;
   float delta;

   if ( Input_WasMouseButtonReleased( mouseState, buttonCode ) && mouseState->dragStates[(uint32_t)buttonCode].wasDragging )
   {
      deltaX = mouseState->pointerPos.x - mouseState->dragOrigin.x;
      deltaY = mouseState->pointerPos.y - mouseState->dragOrigin.y;
      delta = sqrtf( (float)( deltaX * deltaX ) + (float)( deltaY * deltaY ) );

      return ( delta <= MOUSE_CLICK_THRESHOLD ) ? True : False;
   }

   return False;
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
