#include "input.h"

void Input_Init( KeyState_t* keyStates )
{
   int i;
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
   int i;
   KeyState_t* state = keyStates;

   for ( i = 0; i < (int)KeyCode_Count; i++ )
   {
      state->wasDown = state->isDown;
      state++;
   }
}

void Input_PressKey( KeyState_t* keyStates, KeyCode_t keyCode )
{
   keyStates[(int)keyCode].isDown = True;
}

void Input_ReleaseKey( KeyState_t* keyStates, KeyCode_t keyCode )
{
   keyStates[(int)keyCode].isDown = False;
}

Bool_t Input_WasKeyPressed( KeyState_t* keyStates, KeyCode_t keyCode )
{
   return keyStates[(int)keyCode].isDown && !keyStates[(int)keyCode].wasDown;
}

Bool_t Input_WasKeyReleased( KeyState_t* keyStates, KeyCode_t keyCode )
{
   return !keyStates[(int)keyCode].isDown && keyStates[(int)keyCode].wasDown;
}

Bool_t Input_IsAnyKeyDown( KeyState_t* keyStates )
{
   int i;

   for ( i = 0; i < (int)KeyCode_Count; i++ )
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
   int i;

   for ( i = 0; i < (int)KeyCode_Count; i++ )
   {
      if ( Input_WasKeyPressed( keyStates, (KeyCode_t)i ) )
      {
         return True;
      }
   }

   return False;
}
