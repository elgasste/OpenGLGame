#include "clock.h"
#include "platform.h"

void cClock_Init( cClock_t* clock )
{
   clock->totalFrames = 0;
   clock->lagFrames = 0;
   clock->frameDeltaSeconds = 1.0f / GAME_FPS;
   clock->targetFrameDurationMicro = (uint64_t)( 1000000 / GAME_FPS );
   clock->absoluteStartMicro = 0;
   clock->totalTimeMicro = 0;
   clock->lastFrameDurationMicro = 0;
}

void cClock_StartFrame( cClock_t* clock )
{
   clock->frameStartMicro = Platform_GetTimeStampMicro();

   if ( clock->absoluteStartMicro == 0 )
   {
      clock->absoluteStartMicro = clock->frameStartMicro;
   }
}

void cClock_EndFrame( cClock_t* clock )
{
   uint64_t frameStopMicro = Platform_GetTimeStampMicro();

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
