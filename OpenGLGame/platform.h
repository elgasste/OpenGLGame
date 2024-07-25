#if !defined( PLATFORM_H )
#define PLATFORM_H

#include "common.h"
#include "platform_includes.h"

typedef struct ThreadQueue_t ThreadQueue_t;

typedef struct FileData_t
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
void Platform_RenderScreen();
uint64_t Platform_GetTimeStampMicro();
void Platform_Sleep( uint64_t micro );
Bool_t Platform_ReadFileData( const char* filePath, FileData_t* fileData );
Bool_t Platform_WriteFileData( FileData_t* fileData );
void Platform_ClearFileData( FileData_t* fileData );
Bool_t Platform_GetAppDirectory( char* directory, uint32_t stringSize );
ThreadQueue_t* Platform_GetThreadQueue();
Bool_t Platform_AddThreadQueueEntry( void ( *workerFnc )(), void* data );
void Platform_RunThreadQueue();

#endif
