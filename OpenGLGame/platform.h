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
FileData_t;

void Platform_Log( const char* message );
void* Platform_MemAlloc( uint64_t size );
void Platform_MemFree( void* memory );
void Platform_Tick();
PixelBuffer_t* Platform_GetScreenBuffer();
void Platform_RenderScreen();
uint64_t Platform_GetTimeStampMicro();
void Platform_Sleep( uint64_t micro );
Bool_t Platform_ReadFileData( const char* filePath, FileData_t* fileData );
Bool_t Platform_WriteFileData( const char* filePath, FileData_t* fileData );
void Platform_ClearFileData( FileData_t* fileData );

#endif
