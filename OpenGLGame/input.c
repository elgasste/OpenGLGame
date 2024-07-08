#include "input.h"

void Input_Init( KeyState_t* keyStates )
{
   uint32_t i;
   KeyState_t* state = keyStates;

   for ( i = 0; i < (int)KeyCode_Count; i++ )
   {
      state->isDown = False;
      state->wasDown = False;
      state++;
   }
}

void Input_UpdateStates( KeyState_t* keyStates )
{
   uint32_t i;
   KeyState_t* state = keyStates;

   for ( i = 0; i < (uint32_t)KeyCode_Count; i++ )
   {
      state->wasDown = state->isDown;
      state++;
   }
}

void Input_PressKey( KeyState_t* keyStates, KeyCode_t keyCode )
{
   keyStates[(uint32_t)keyCode].isDown = True;
}

void Input_ReleaseKey( KeyState_t* keyStates, KeyCode_t keyCode )
{
   keyStates[(uint32_t)keyCode].isDown = False;
}

Bool_t Input_WasKeyPressed( KeyState_t* keyStates, KeyCode_t keyCode )
{
   return keyStates[(uint32_t)keyCode].isDown && !keyStates[(uint32_t)keyCode].wasDown;
}

Bool_t Input_WasKeyReleased( KeyState_t* keyStates, KeyCode_t keyCode )
{
   return !keyStates[(uint32_t)keyCode].isDown && keyStates[(uint32_t)keyCode].wasDown;
}

Bool_t Input_IsAnyKeyDown( KeyState_t* keyStates )
{
   uint32_t i;

   for ( i = 0; i < (uint32_t)KeyCode_Count; i++ )
   {
      if ( keyStates[i].isDown )
      {
         return True;
      }
   }

   return False;
}

Bool_t Input_WasAnyKeyPressed( KeyState_t* keyStates )
{
   uint32_t i;

   for ( i = 0; i < (uint32_t)KeyCode_Count; i++ )
   {
      if ( Input_WasKeyPressed( keyStates, (KeyCode_t)i ) )
      {
         return True;
      }
   }

   return False;
}
