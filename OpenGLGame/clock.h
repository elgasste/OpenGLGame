#if !defined( CLOCK_H )
#define CLOCK_H

#include "common.h"

typedef struct
{
   uint32_t totalFrames;
   uint32_t lagFrames;
   uint64_t absoluteStartMicro;
   uint64_t frameStartMicro;
   uint64_t totalTimeMicro;
   uint64_t lastFrameDurationMicro;
   uint64_t targetFrameDurationMicro;
   float frameDeltaSeconds;
}
cClock_t;

void cClock_Init( cClock_t* clock );
void cClock_StartFrame( cClock_t* clock );
void cClock_EndFrame( cClock_t* clock );

#endif
