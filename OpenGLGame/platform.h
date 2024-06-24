#if !defined( PLATFORM_H )
#define PLATFORM_H

#include "common.h"
#include "screen_buffer.h"
#include "file_data.h"

void Platform_Log( const char* message );
void* Platform_MemAlloc( uint64_t size );
void Platform_MemFree( void* memory );
void Platform_Tick();
cScreenBuffer_t* Platform_GetScreenBuffer();
void Platform_RenderScreen();
uint64_t Platform_GetTimeStampMicro();
void Platform_Sleep( uint64_t micro );
cBool_t Platform_ReadFileData( cFileData_t* fileData );
cBool_t Platform_WriteFileData( cFileData_t* fileData );
void Platform_ClearFileData( cFileData_t* fileData );

#endif
