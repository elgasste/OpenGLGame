#include "clock.h"
#include "platform.h"

void cClock_Init( cClock_t* clock )
{
   clock->totalFrames = 0;
   clock->lagFrames = 0;
   clock->frameDeltaSeconds = 1.0f / GAME_FPS;
   clock->targetFrameDurationMicro = (uint64_t)( 1000000 / GAME_FPS );
   clock->absoluteStartMicro = 0;
   clock->averageMsPerFrame = 0.0f;
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
   uint64_t totalTimeMicro;
   uint64_t frameStopMicro = Platform_GetTimeStampMicro();
   uint64_t frameDurationMicro = frameStopMicro - clock->frameStartMicro;
   clock->totalFrames++;

   totalTimeMicro = frameStopMicro - clock->absoluteStartMicro;
   clock->averageMsPerFrame = (float)( ( totalTimeMicro / (double)clock->totalFrames ) / 1000.0f );

   if ( frameDurationMicro < clock->targetFrameDurationMicro )
   {
      Platform_Sleep( frameStopMicro, clock->targetFrameDurationMicro - frameDurationMicro );
   }
   else
   {
      // if the frame happens to end EXACTLY on targetFrameDurationMicro, it'll be marked as
      // a lag frame, but whatever, at that point it might as well be.
      clock->lagFrames++;
   }
}
