#if !defined( CLOCK_H )
#define CLOCK_H

#include "common.h"

typedef struct Clock_t
{
   Bool_t isRunning;
   uint64_t pauseTimeMicro;

   uint32_t totalFrames;
   uint32_t lagFrames;
   uint64_t absoluteStartMicro;
   uint64_t frameStartMicro;
   uint64_t totalTimeMicro;
   uint64_t lastFrameDurationMicro;
   uint64_t targetFrameDurationMicro;

   float frameDeltaSeconds;
}
Clock_t;

void Clock_Init( Clock_t* clock );
void Clock_StartFrame( Clock_t* clock );
void Clock_EndFrame( Clock_t* clock );
void Clock_Pause( Clock_t* clock );
void Clock_Resume( Clock_t* clock );

#endif
