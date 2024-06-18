#if !defined( PLATFORM_H )
#define PLATFORM_H

#include "common.h"
#include "screen_buffer.h"

void Platform_Tick();
cScreenBuffer_t* Platform_GetScreenBuffer();
void Platform_RenderScreen();
uint64_t Platform_GetTimeStampMicro();
void Platform_Sleep( uint64_t startMicro, uint64_t sleepMicro );

#endif
