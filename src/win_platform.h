#if !defined( WIN_PLATFORM_H )
#define WIN_PLATFORM_H

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <gl/GL.h>
#include <Shlwapi.h>
#include <time.h>

#define LOG_FILE_NAME "log.txt"

typedef struct ThreadQueue_t ThreadQueue_t;

typedef struct
{
   ThreadQueue_t* queue;
   uint32_t threadIndex;
   uint64_t jobsDone;
}
Win32ThreadInfo_t;

#endif
