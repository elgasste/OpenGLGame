#include "clock.h"
#include "platform.h"

void Clock_Init( Clock_t* clock )
{
   clock->isRunning = True;
   clock->pauseTimeMicro = 0;

   clock->totalFrames = 0;
   clock->lagFrames = 0;
   clock->frameDeltaSeconds = 1.0f / GAME_FPS;
   clock->targetFrameDurationMicro = (uint64_t)( 1000000 / GAME_FPS );
   clock->absoluteStartMicro = 0;
   clock->totalTimeMicro = 0;
   clock->lastFrameDurationMicro = 0;
}

void Clock_StartFrame( Clock_t* clock )
{
   if ( clock->isRunning )
   {
      clock->frameStartMicro = Platform_GetTimeStampMicro();

      if ( clock->absoluteStartMicro == 0 )
      {
         clock->absoluteStartMicro = clock->frameStartMicro;
      }
   }
}

void Clock_EndFrame( Clock_t* clock )
{
   uint64_t frameStopMicro;

   if ( clock->isRunning )
   {
      frameStopMicro = Platform_GetTimeStampMicro();

      clock->lastFrameDurationMicro = frameStopMicro - clock->frameStartMicro;
      clock->totalFrames++;
      clock->totalTimeMicro = frameStopMicro - clock->absoluteStartMicro;

      if ( clock->lastFrameDurationMicro < clock->targetFrameDurationMicro )
      {
         Platform_Sleep( clock->targetFrameDurationMicro - clock->lastFrameDurationMicro );
      }
      else
      {
         // if the frame happens to end EXACTLY on targetFrameDurationMicro, it'll be marked as
         // a lag frame, but whatever, at that point it might as well be.
         clock->lagFrames++;
      }
   }
}

void Clock_Pause( Clock_t* clock )
{
   if ( clock->isRunning )
   {
      clock->isRunning = False;
      clock->pauseTimeMicro = Platform_GetTimeStampMicro();
   }
}

void Clock_Resume( Clock_t* clock )
{
   uint64_t pauseDurationMicro;

   if ( !clock->isRunning )
   {
      pauseDurationMicro = Platform_GetTimeStampMicro() - clock->pauseTimeMicro;

      clock->absoluteStartMicro += pauseDurationMicro;
      clock->frameStartMicro += pauseDurationMicro;
      clock->isRunning = True;
   }
}
