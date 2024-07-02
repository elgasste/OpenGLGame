#if !defined( PLATFORM_H )
#define PLATFORM_H

#include "common.h"
#include "pixel_buffer.h"

typedef struct
{
   char filePath[STRING_SIZE_DEFAULT];
   uint32_t fileSize;
   void* contents;
}
cFileData_t;

void Platform_Log( const char* message );
void* Platform_MemAlloc( uint64_t size );
void Platform_MemFree( void* memory );
void Platform_Tick();
cPixelBuffer_t* Platform_GetScreenBuffer();
void Platform_RenderScreen();
uint64_t Platform_GetTimeStampMicro();
void Platform_Sleep( uint64_t micro );
cBool_t Platform_ReadFileData( const char* filePath, cFileData_t* fileData );
cBool_t Platform_WriteFileData( const char* filePath, cFileData_t* fileData );
void Platform_ClearFileData( cFileData_t* fileData );

#endif
