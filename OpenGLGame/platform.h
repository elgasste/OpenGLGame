#if !defined( PLATFORM_H )
#define PLATFORM_H

#include "common.h"
#include "screen_buffer.h"
#include "file_data.h"

void Platform_Tick();
cScreenBuffer_t* Platform_GetScreenBuffer();
void Platform_RenderScreen();
uint64_t Platform_GetTimeStampMicro();
void Platform_Sleep( uint64_t micro );
cBool_t Platform_ReadFileData( const char* filePath, cFileData_t* fileData );
cBool_t Platform_WriteFileData( const char* filePath, cFileData_t* fileData );
void Platform_ClearFileData( cFileData_t* fileData );

#endif
