#include "input.h"

void cInput_Init( cKeyState_t* keyStates )
{
   int i;
   cKeyState_t* state = keyStates;

   for ( i = 0; i < (int)cKeyCode_Count; i++ )
   {
      state->isDown = cFalse;
      state->wasDown = cFalse;
      state++;
   }
}

void cInput_UpdateStates( cKeyState_t* keyStates )
{
   int i;
   cKeyState_t* state = keyStates;

   for ( i = 0; i < (int)cKeyCode_Count; i++ )
   {
      state->wasDown = state->isDown;
      state++;
   }
}

void cInput_PressKey( cKeyState_t* keyStates, cKeyCode_t keyCode )
{
   keyStates[(int)keyCode].isDown = cTrue;
}

void cInput_ReleaseKey( cKeyState_t* keyStates, cKeyCode_t keyCode )
{
   keyStates[(int)keyCode].isDown = cFalse;
}

cBool_t cInput_WasKeyPressed( cKeyState_t* keyStates, cKeyCode_t keyCode )
{
   return keyStates[(int)keyCode].isDown && !keyStates[(int)keyCode].wasDown;
}

cBool_t cInput_WasKeyReleased( cKeyState_t* keyStates, cKeyCode_t keyCode )
{
   return !keyStates[(int)keyCode].isDown && keyStates[(int)keyCode].wasDown;
}

cBool_t cInput_IsAnyKeyDown( cKeyState_t* keyStates )
{
   int i;

   for ( i = 0; i < (int)cKeyCode_Count; i++ )
   {
      if ( keyStates[i].isDown )
      {
         return cTrue;
      }
   }

   return cFalse;
}

cBool_t cInput_WasAnyKeyPressed( cKeyState_t* keyStates )
{
   int i;

   for ( i = 0; i < (int)cKeyCode_Count; i++ )
   {
      if ( cInput_WasKeyPressed( keyStates, (cKeyCode_t)i ) )
      {
         return cTrue;
      }
   }

   return cFalse;
}
