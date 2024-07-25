#include "game.h"

typedef struct
{
   HWND hWndMain;
   GameData_t gameData;
   LARGE_INTEGER performanceFrequency;
   uint32_t keyCodeMap[(int)KeyCode_Count];
   ThreadQueue_t threadQueue;
   HANDLE threadSemaphoreHandle;
}
cGlobalObjects_t;

global cGlobalObjects_t g_globals;

internal void FatalError( const char* message );
internal void InitThreads();
internal void InitKeyCodeMap();
internal void InitOpenGL( HWND hWnd );
internal LRESULT CALLBACK MainWindowProc( _In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam );
internal void HandleKeyboardInput( uint32_t keyCode, LPARAM flags );
internal DWORD WINAPI ThreadProc( LPVOID lpParam );
internal Bool_t DoNextThreadQueueEntry();

int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
{
   TIMECAPS timeCaps;
   UINT timerResolution;
   WNDCLASSA mainWindowClass = { 0 };
   DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
   RECT expectedWindowRect = { 0 };
   LONG clientPaddingTop, clientPaddingRight;

   UNUSED_PARAM( hPrevInstance );
   UNUSED_PARAM( lpCmdLine );
   UNUSED_PARAM( nCmdShow );

   if ( !QueryPerformanceFrequency( &( g_globals.performanceFrequency ) ) )
   {
      FatalError( STR_WINERR_PERFORMANCEFREQUENCY );
   }

   if ( timeGetDevCaps( &timeCaps, sizeof( TIMECAPS ) ) != TIMERR_NOERROR )
   {
      FatalError( STR_WINERR_TIMERRESOLUTION );
   }

   timerResolution = min( max( timeCaps.wPeriodMin, 1 ), timeCaps.wPeriodMax );
   timeBeginPeriod( timerResolution );

   mainWindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
   mainWindowClass.lpfnWndProc = MainWindowProc;
   mainWindowClass.hInstance = hInstance;
   mainWindowClass.lpszClassName = "mainWindowClass";

   if ( !RegisterClassA( &mainWindowClass ) )
   {
      FatalError( STR_WINERR_REGISTERWINDOW );
   }

   expectedWindowRect.right = SCREEN_WIDTH;
   expectedWindowRect.bottom = SCREEN_HEIGHT;

   if ( !AdjustWindowRect( &expectedWindowRect, windowStyle, 0 ) )
   {
      FatalError( STR_WINERR_ADJUSTCLIENTRECT );
   }

   clientPaddingRight = ( expectedWindowRect.right - expectedWindowRect.left ) - SCREEN_WIDTH;
   clientPaddingTop = ( expectedWindowRect.bottom - expectedWindowRect.top ) - SCREEN_HEIGHT;

   g_globals.hWndMain = CreateWindowExA( 0,
                                         mainWindowClass.lpszClassName,
                                         STR_WIN_WINDOWTITLE,
                                         windowStyle,
                                         CW_USEDEFAULT,
                                         CW_USEDEFAULT,
                                         SCREEN_WIDTH + clientPaddingRight,
                                         SCREEN_HEIGHT + clientPaddingTop,
                                         0,
                                         0,
                                         hInstance,
                                         0 );

   if ( !g_globals.hWndMain )
   {
      FatalError( STR_WINERR_CREATEWINDOW );
   }

   InitKeyCodeMap();
   InitOpenGL( g_globals.hWndMain );
   InitThreads();

   if ( !Game_Init( &( g_globals.gameData ) ) )
   {
      FatalError( STR_WINERR_INITGAME );
   }

   Game_Run( &( g_globals.gameData ) );
   Game_ClearData( &( g_globals.gameData ) );

   return 0;
}

internal void FatalError( const char* message )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_WINERR_LOGMESSAGE, message );
   Platform_Log( errorMsg );

   Game_EmergencySave( &( g_globals.gameData ) );
   MessageBoxA( 0, message, STR_WINERR_HEADER, MB_OK | MB_ICONERROR );
   exit( 1 );
}

internal void InitThreads()
{
   Win32ThreadInfo_t* threadInfo;
   uint32_t i;
   DWORD numThreads, threadId;
   HANDLE threadHandle;
   SYSTEM_INFO sysInfo;

   GetSystemInfo( &sysInfo );
   numThreads = ( sysInfo.dwNumberOfProcessors == 0 ) ? 1 : sysInfo.dwNumberOfProcessors;
   threadInfo = (Win32ThreadInfo_t*)Platform_MemAlloc( sizeof( Win32ThreadInfo_t ) * numThreads );

   g_globals.threadQueue.numThreads = numThreads;
   g_globals.threadQueue.completionGoal = 0;
   g_globals.threadQueue.completionCount = 0;
   g_globals.threadQueue.nextEntryToRead = 0;
   g_globals.threadQueue.nextEntryToWrite = 0;
   g_globals.threadSemaphoreHandle = CreateSemaphoreExA( 0, 0, numThreads, 0, 0, SEMAPHORE_ALL_ACCESS );

   if ( !g_globals.threadSemaphoreHandle || g_globals.threadSemaphoreHandle == INVALID_HANDLE_VALUE )
   {
      FatalError( STR_WINERR_INITTHREADS );
   }

   for ( i = 0; i < numThreads; i++ )
   {
      threadInfo[i].queue = &( g_globals.threadQueue );
      threadInfo[i].logicalThreadIndex = i;
      threadHandle = CreateThread( 0, 0, ThreadProc, &( threadInfo[i] ), 0, &threadId );

      if ( !threadHandle || threadHandle == INVALID_HANDLE_VALUE )
      {
         FatalError( STR_WINERR_INITTHREADS );
      }
      else
      {
#pragma warning(suppress : 6001)
         CloseHandle( threadHandle );
      }
   }
}

internal void InitKeyCodeMap()
{
   g_globals.keyCodeMap[(int)KeyCode_Left] = VK_LEFT;
   g_globals.keyCodeMap[(int)KeyCode_Up] = VK_UP;
   g_globals.keyCodeMap[(int)KeyCode_Right] = VK_RIGHT;
   g_globals.keyCodeMap[(int)KeyCode_Down] = VK_DOWN;
   g_globals.keyCodeMap[(int)KeyCode_Enter] = VK_RETURN;
   g_globals.keyCodeMap[(int)KeyCode_Escape] = VK_ESCAPE;
   g_globals.keyCodeMap[(int)KeyCode_F8] = VK_F8;
}

internal void InitOpenGL( HWND hWnd )
{
   PIXELFORMATDESCRIPTOR desiredPixelFormat = { 0 };
   PIXELFORMATDESCRIPTOR suggestedPixelFormat;
   int suggestedPixelFormatIndex;
   HGLRC glRC;
   HDC dc = GetDC( hWnd );

   desiredPixelFormat.nSize = sizeof( desiredPixelFormat );
   desiredPixelFormat.nVersion = 1;
   desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
   desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
   desiredPixelFormat.cColorBits = 32;
   desiredPixelFormat.cAlphaBits = 8;
   desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

   suggestedPixelFormatIndex = ChoosePixelFormat( dc, &desiredPixelFormat );

   if ( !suggestedPixelFormatIndex )
   {
      FatalError( STR_WINERR_NOPIXELFORMAT );
   }

   DescribePixelFormat( dc, suggestedPixelFormatIndex, sizeof( suggestedPixelFormat ), &suggestedPixelFormat );

   if ( suggestedPixelFormat.cColorBits != 32 || suggestedPixelFormat.cAlphaBits != 8 )
   {
      FatalError( STR_WINERR_UNSUITABLEPIXELFORMAT );
   }

   SetPixelFormat( dc, suggestedPixelFormatIndex, &suggestedPixelFormat );
   glRC = wglCreateContext( dc );

   if ( !wglMakeCurrent( dc, glRC ) )
   {
      FatalError( STR_WINERR_RENDERINGCONTEXT );
   }

   ReleaseDC( hWnd, dc );
}

internal LRESULT CALLBACK MainWindowProc( _In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam )
{
   LRESULT result = 0;

   switch ( uMsg )
   {
      case WM_QUIT:
      case WM_CLOSE:
         Game_TryClose( &( g_globals.gameData ) );
         break;
      case WM_DESTROY:
         if ( g_globals.gameData.isRunning )
         {
            Game_EmergencySave( &( g_globals.gameData ) );
            g_globals.gameData.isRunning = False;
         }
         break;
      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
         HandleKeyboardInput( (uint32_t)wParam, lParam );
         break;
      case WM_KILLFOCUS:
         Game_PauseEngine( &( g_globals.gameData ) );
         DefWindowProc( hWnd, uMsg, wParam, lParam );
         break;
      case WM_SETFOCUS:
         Game_ResumeEngine( &( g_globals.gameData ) );
         DefWindowProc( hWnd, uMsg, wParam, lParam );
         break;
      default:
         result = DefWindowProcA( hWnd, uMsg, wParam, lParam );
   }

   return result;
}

internal void HandleKeyboardInput( uint32_t keyCode, LPARAM flags )
{
   Bool_t keyWasDown = ( flags & ( (LONG_PTR)1 << 30 ) ) != 0 ? True : False;
   Bool_t keyIsDown = ( flags & ( (LONG_PTR)1 << 31 ) ) == 0 ? True : False;
   uint32_t i;

   // ignore repeat presses
   if ( keyWasDown != keyIsDown )
   {
      if ( keyIsDown )
      {
         // ensure alt+F4 still closes the window
         if ( keyCode == VK_F4 && ( flags & ( (LONG_PTR)1 << 29 ) ) )
         {
            Game_TryClose( &( g_globals.gameData ) );
            return;
         }

         for ( i = 0; i < KeyCode_Count; i++ )
         {
            if ( g_globals.keyCodeMap[i] == keyCode )
            {
               Input_PressKey( g_globals.gameData.keyStates, (KeyCode_t)i );
               break;
            }
         }
      }
      else
      {
         for ( i = 0; i < KeyCode_Count; i++ )
         {
            if ( g_globals.keyCodeMap[i] == keyCode )
            {
               Input_ReleaseKey( g_globals.gameData.keyStates, (KeyCode_t)i );
               break;
            }
         }
      }
   }
}

internal DWORD WINAPI ThreadProc( LPVOID lpParam )
{
   UNUSED_PARAM( lpParam );

   while ( 1 )
   {
      if ( DoNextThreadQueueEntry() )
      {
         WaitForSingleObjectEx( g_globals.threadSemaphoreHandle, INFINITE, FALSE );
      }
   }
}

internal Bool_t DoNextThreadQueueEntry()
{
   Bool_t shouldSleep = False;
   uint32_t entryIndex, originalNextEntry = g_globals.threadQueue.nextEntryToRead;
   uint32_t newNextEntryToRead = ( originalNextEntry + 1 ) % MAX_THREADQUEUE_SIZE;

   if ( originalNextEntry != g_globals.threadQueue.nextEntryToWrite )
   {
      entryIndex = InterlockedCompareExchange( (LONG volatile *)( &( g_globals.threadQueue.nextEntryToRead ) ),
                                               newNextEntryToRead,
                                               originalNextEntry );

      if ( entryIndex == originalNextEntry )
      {        
         g_globals.threadQueue.entries[entryIndex].workerFnc( g_globals.threadQueue.entries[entryIndex].data );
         InterlockedIncrement( (LONG volatile *)( &( g_globals.threadQueue.completionCount ) ) );
      }
   }
   else
   {
      shouldSleep = True;
   }

   return shouldSleep;
}

void Platform_Log( const char* message )
{
   HANDLE hFile;
   time_t now;
   struct tm timeInfo;
   char formattedTime[STRING_SIZE_DEFAULT];
   char timestampedMessage[STRING_SIZE_DEFAULT];
   char appDirectory[STRING_SIZE_DEFAULT];
   char logFilePath[STRING_SIZE_DEFAULT];
   DWORD bytesToWrite;
   DWORD bytesWritten;

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return;
   }

   time( &now );
   localtime_s( &timeInfo, &now );
   strftime( formattedTime, STRING_SIZE_DEFAULT, "%d-%m-%Y %H:%M:%S", &timeInfo );
   snprintf( timestampedMessage, STRING_SIZE_DEFAULT, "%s - %s\n", formattedTime, message );
   snprintf( logFilePath, STRING_SIZE_DEFAULT, "%s%s", appDirectory, LOG_FILE_NAME );
   hFile = CreateFileA( logFilePath, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

   if ( hFile == INVALID_HANDLE_VALUE )
   {
      MessageBoxA( 0, STR_WINERR_CREATELOGFILE, STR_WINERR_HEADER, MB_OK | MB_ICONERROR );
      return;
   }

   bytesToWrite = (DWORD)strlen( timestampedMessage );

   if ( !WriteFile( hFile, timestampedMessage, bytesToWrite, &bytesWritten, 0 ) || bytesWritten != bytesToWrite )
   {
      MessageBoxA( 0, STR_WINERR_WRITELOGFILE, STR_WINERR_HEADER, MB_OK | MB_ICONERROR );
   }

   CloseHandle( hFile );
   return;
}

void* Platform_MemAlloc( uint64_t size )
{
   return VirtualAlloc( 0, size, MEM_COMMIT, PAGE_READWRITE );
}

void Platform_MemFree( void* memory )
{
   VirtualFree( memory, 0, MEM_RELEASE );
}

void Platform_Tick()
{
   MSG msg;

   while ( PeekMessageA( &msg, g_globals.hWndMain, 0, 0, PM_REMOVE ) )
   {
      TranslateMessage( &msg );
      DispatchMessageA( &msg );
   }
}

void Platform_RenderScreen()
{
   HDC dc = GetDC( g_globals.hWndMain );
   SwapBuffers( dc );
   ReleaseDC( g_globals.hWndMain, dc );
}

uint64_t Platform_GetTimeStampMicro()
{
   LARGE_INTEGER ticks;
   QueryPerformanceCounter( &ticks );

   return (uint64_t)( ( (double)( ticks.QuadPart ) / (double)( g_globals.performanceFrequency.QuadPart ) ) * (uint64_t)1000000 );
}

void Platform_Sleep( uint64_t micro )
{
   DWORD milli = (DWORD)( micro / 1000 );

   // TODO: this isn't super accurate, especially with very fast frames. it'd be
   // great to find a better way to sleep (something like std::chrono).
   if ( milli > 0 )
   {
      Sleep( milli );
   }
}

Bool_t Platform_ReadFileData( const char* filePath, FileData_t* fileData )
{
   HANDLE hFile;
   LARGE_INTEGER fileSize;
   OVERLAPPED overlapped = { 0 };
   char errorMsg[STRING_SIZE_DEFAULT];

   strcpy_s( fileData->filePath, STRING_SIZE_DEFAULT, filePath );
   fileData->contents = 0;
   fileData->fileSize = 0;

   hFile = CreateFileA( filePath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

   if ( hFile == INVALID_HANDLE_VALUE )
   {
      // TODO: maybe log the reason it couldn't be opened (file not found, etc)?
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FILEERR_OPENFILEFAILED, filePath );
      Platform_Log( errorMsg );
      return False;
   }

   if ( !GetFileSizeEx( hFile, &fileSize ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FILEERR_GETFILESIZEFAILED, filePath );
      Platform_Log( errorMsg );
      CloseHandle( hFile );
      return False;
   }

   fileData->fileSize = fileSize.LowPart;
   fileData->contents = VirtualAlloc( 0, (SIZE_T)( fileData->fileSize ), MEM_COMMIT, PAGE_READWRITE );

// not sure why it shows this warning, according to the docs the 5th param can be null
#pragma warning(suppress : 6387)
   if ( !ReadFileEx( hFile, fileData->contents, fileData->fileSize, &overlapped, 0 ) )
   {
      Platform_ClearFileData( fileData );
      CloseHandle( hFile );
      return False;
   }

   CloseHandle( hFile );
   return True;
}

Bool_t Platform_WriteFileData( FileData_t* fileData )
{
   HANDLE hFile;
   OVERLAPPED overlapped = { 0 };

   hFile = CreateFileA( fileData->filePath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

   if ( !hFile )
   {
      return False;
   }

// again, not sure why it shows this warning, according to the docs the 5th param can be null
#pragma warning(suppress : 6387)
   if ( !WriteFileEx( hFile, fileData->contents, fileData->fileSize, &overlapped, 0 ) )
   {
      CloseHandle( hFile );
      return False;
   }

   CloseHandle( hFile );
   return True;
}

void Platform_ClearFileData( FileData_t* fileData )
{
   if ( fileData->contents )
   {
      VirtualFree( fileData->contents, 0, MEM_RELEASE );
   }

   fileData->contents = 0;
   fileData->fileSize = 0;
}

Bool_t Platform_GetAppDirectory( char* directory, uint32_t stringSize )
{
   LPSTR fileName;
   DWORD stringLength = GetModuleFileNameA( NULL, directory, stringSize );

   if ( stringLength != strlen( directory ) )
   {
      MessageBoxA( 0, STR_WINERR_APPDIRECTORY, STR_WINERR_HEADER, MB_OK | MB_ICONERROR );
      return False;
   }

   fileName = PathFindFileNameA( directory );
   directory[stringLength - strlen( fileName )] = '\0';

   return True;
}

ThreadQueue_t* Platform_GetThreadQueue()
{
   return &( g_globals.threadQueue );
}

Bool_t Platform_AddThreadQueueEntry( void ( *workerFnc )(), void* data )
{
   if ( g_globals.threadQueue.completionGoal >= ( MAX_THREADQUEUE_SIZE - 1 ) )
   {
      return False;
   }

   uint32_t newNextEntryToWrite = ( g_globals.threadQueue.nextEntryToWrite + 1 ) % MAX_THREADQUEUE_SIZE;
   ThreadQueueEntry_t* entry = g_globals.threadQueue.entries + g_globals.threadQueue.nextEntryToWrite;
   entry->workerFnc = workerFnc;
   entry->data = data;
   g_globals.threadQueue.completionGoal++;
   _WriteBarrier();
   _mm_sfence();
   g_globals.threadQueue.nextEntryToWrite = newNextEntryToWrite;
   ReleaseSemaphore( g_globals.threadSemaphoreHandle, 1, 0 );
   return True;
}

void Platform_RunThreadQueue()
{
   while( g_globals.threadQueue.completionGoal != g_globals.threadQueue.completionCount )
   {
      DoNextThreadQueueEntry();
   }

   g_globals.threadQueue.completionGoal = 0;
   g_globals.threadQueue.completionCount = 0;
}
