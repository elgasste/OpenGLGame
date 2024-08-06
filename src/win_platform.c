#include <assert.h>

#include "game.h"
#include "thread.h"

typedef struct
{
   HWND hWndMain;
   GameData_t gameData;
   LARGE_INTEGER performanceFrequency;
   uint32_t buttonCodeMap[(int)ButtonCode_Count];
   Win32ThreadInfo_t* threadInfoArray;
   ThreadQueue_t threadQueue;
   HANDLE threadSemaphoreHandle;
   uint64_t memAllocated;
   uint64_t memFreed;
}
cGlobalObjects_t;

global cGlobalObjects_t g_globals;

internal void FatalError( const char* message );
internal void InitThreads();
internal void InitKeyCodeMap();
internal void InitOpenGL( HWND hWnd );
internal LRESULT CALLBACK MainWindowProc( _In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam );
internal void HandleKeyboardInput( uint32_t keyCode, LPARAM flags );
internal void HandleMouseMove( LPARAM posData );
internal void HandleMouseButton( ButtonCode_t buttonCode, Bool_t buttonDown );
internal void HandleMouseLeaveClient();
internal DWORD WINAPI ThreadProc( LPVOID lpParam );
internal Bool_t DoNextThreadQueueEntry( Win32ThreadInfo_t* threadInfo );

int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
{
   TIMECAPS timeCaps;
   UINT timerResolution;
   WNDCLASSA mainWindowClass = { 0 };
   DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
   RECT expectedWindowRect = { 0 };
   LONG clientPaddingRight, clientPaddingTop;

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

   SetCursor( LoadCursor( 0, IDC_ARROW ) );

   InitKeyCodeMap();
   InitOpenGL( g_globals.hWndMain );
   InitThreads();

   if ( !Game_Init( &( g_globals.gameData ) ) )
   {
      FatalError( STR_WINERR_INITGAME );
   }

   Game_Run( &( g_globals.gameData ) );
   Game_ClearData( &( g_globals.gameData ) );

   Platform_Free( g_globals.threadInfoArray, sizeof( Win32ThreadInfo_t ) * g_globals.threadQueue.numThreads );

   assert( g_globals.memAllocated == g_globals.memFreed );

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
   uint32_t i;
   DWORD numThreads, threadId;
   HANDLE threadHandle;
   SYSTEM_INFO sysInfo;

   GetSystemInfo( &sysInfo );
   numThreads = ( sysInfo.dwNumberOfProcessors == 0 ) ? 1 : sysInfo.dwNumberOfProcessors;
   g_globals.threadInfoArray = (Win32ThreadInfo_t*)Platform_MAlloc( sizeof( Win32ThreadInfo_t ) * numThreads );

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

   // thread 0 is the main thread
   g_globals.threadInfoArray[0].queue = &( g_globals.threadQueue );
   g_globals.threadInfoArray[0].threadIndex = 0;
   g_globals.threadInfoArray[0].jobsDone = 0;

   for ( i = 1; i < numThreads; i++ )
   {
      g_globals.threadInfoArray[i].queue = &( g_globals.threadQueue );
      g_globals.threadInfoArray[i].threadIndex = i;
      g_globals.threadInfoArray[i].jobsDone = 0;
      threadHandle = CreateThread( 0, 0, ThreadProc, &( g_globals.threadInfoArray[i] ), 0, &threadId );

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
   g_globals.buttonCodeMap[(int)ButtonCode_Left] = VK_LEFT;
   g_globals.buttonCodeMap[(int)ButtonCode_Up] = VK_UP;
   g_globals.buttonCodeMap[(int)ButtonCode_Right] = VK_RIGHT;
   g_globals.buttonCodeMap[(int)ButtonCode_Down] = VK_DOWN;
   g_globals.buttonCodeMap[(int)ButtonCode_Enter] = VK_RETURN;
   g_globals.buttonCodeMap[(int)ButtonCode_Escape] = VK_ESCAPE;
   g_globals.buttonCodeMap[(int)ButtonCode_F8] = VK_F8;
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
      case WM_MOUSEMOVE:
         HandleMouseMove( lParam );
         break;
      case WM_LBUTTONDOWN:
         HandleMouseButton( ButtonCode_MouseLeft, True );
         break;
      case WM_RBUTTONDOWN:
         HandleMouseButton( ButtonCode_MouseRight, True );
         break;
      case WM_LBUTTONUP:
         HandleMouseButton( ButtonCode_MouseLeft, False );
         break;
      case WM_RBUTTONUP:
         HandleMouseButton( ButtonCode_MouseRight, False );
         break;
      case WM_KILLFOCUS:
         HandleMouseLeaveClient();
         Game_PauseEngine( &( g_globals.gameData ) );
         DefWindowProc( hWnd, uMsg, wParam, lParam );
         break;
      case WM_SETFOCUS:
         HandleMouseLeaveClient();
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

         for ( i = 0; i < ButtonCode_Count; i++ )
         {
            if ( g_globals.buttonCodeMap[i] == keyCode )
            {
               Input_PressButton( &( g_globals.gameData.inputState ), (ButtonCode_t)i );
               break;
            }
         }
      }
      else
      {
         for ( i = 0; i < ButtonCode_Count; i++ )
         {
            if ( g_globals.buttonCodeMap[i] == keyCode )
            {
               Input_ReleaseButton( &( g_globals.gameData.inputState ), (ButtonCode_t)i );
               break;
            }
         }
      }
   }
}

internal void HandleMouseButton( ButtonCode_t buttonCode, Bool_t buttonDown )
{
   void (*inputFunc)( InputState_t*, ButtonCode_t ) = buttonDown ? Input_PressButton : Input_ReleaseButton;

   inputFunc( &( g_globals.gameData.inputState ), buttonCode );

   if ( buttonDown )
   {
      SetCapture( g_globals.hWndMain );
   }
   else
   {
      if ( !g_globals.gameData.inputState.buttonStates[ButtonCode_MouseLeft].isDown &&
           !g_globals.gameData.inputState.buttonStates[ButtonCode_MouseRight].isDown )
      {
         ReleaseCapture();
      }
   }

   inputFunc( &( g_globals.gameData.inputState ), buttonCode );
}

internal void HandleMouseLeaveClient()
{
   Input_ReleaseButton( &( g_globals.gameData.inputState ), ButtonCode_MouseLeft );
   Input_ReleaseButton( &( g_globals.gameData.inputState ), ButtonCode_MouseRight );
}

internal void HandleMouseMove( LPARAM posData )
{
   int16_t clientX = (int16_t)posData;
   int16_t clientY = SCREEN_HEIGHT - (int16_t)( posData >> 16 );

   if ( clientX >= 0 && clientX < SCREEN_WIDTH && clientY >= 0 && clientY < SCREEN_HEIGHT )
   {
      Input_SetMousePos( &( g_globals.gameData.inputState ), (int32_t)clientX, (int32_t)clientY );
   }
}

internal DWORD WINAPI ThreadProc( LPVOID lpParam )
{
   while ( 1 )
   {
      if ( DoNextThreadQueueEntry( (Win32ThreadInfo_t*)lpParam ) )
      {
         WaitForSingleObjectEx( g_globals.threadSemaphoreHandle, INFINITE, FALSE );
      }
   }
}

internal Bool_t DoNextThreadQueueEntry( Win32ThreadInfo_t* threadInfo )
{
   Bool_t shouldSleep = False;
   uint32_t entryIndex, originalNextEntryToRead = g_globals.threadQueue.nextEntryToRead;
   uint32_t newNextEntryToRead = ( originalNextEntryToRead + 1 ) % MAX_THREADQUEUE_SIZE;

   if ( originalNextEntryToRead != g_globals.threadQueue.nextEntryToWrite )
   {
      entryIndex = InterlockedCompareExchange( (LONG volatile *)( &( g_globals.threadQueue.nextEntryToRead ) ),
                                               newNextEntryToRead,
                                               originalNextEntryToRead );

      if ( entryIndex == originalNextEntryToRead )
      {        
         g_globals.threadQueue.entries[entryIndex].workerFnc( g_globals.threadQueue.entries[entryIndex].data );
         threadInfo->jobsDone++;
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
   strftime( formattedTime, STRING_SIZE_DEFAULT, "%Y-%m-%d %H:%M:%S", &timeInfo );
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

void* Platform_MAlloc( uint64_t size )
{
   g_globals.memAllocated += size;
   return malloc( size );
}

void* Platform_CAlloc( uint64_t count, uint64_t size )
{
   g_globals.memAllocated += ( count * size );
   return calloc( count, size );
}

void* Platform_ReAlloc( void* memory, uint64_t oldSize, uint64_t newSize )
{
   if ( newSize < oldSize )
   {
      g_globals.memAllocated -= oldSize - newSize;
   }
   else
   {
      g_globals.memAllocated += ( newSize - oldSize );
   }

   return realloc( memory, newSize );
}

void Platform_Free( void* memory, uint64_t size )
{
   g_globals.memFreed += size;
   free( memory );
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
   uint32_t newNextEntryToWrite;
   ThreadQueueEntry_t* entry;

   if ( g_globals.threadQueue.completionGoal >= ( MAX_THREADQUEUE_SIZE - 1 ) )
   {
      return False;
   }

   newNextEntryToWrite = ( g_globals.threadQueue.nextEntryToWrite + 1 ) % MAX_THREADQUEUE_SIZE;
   entry = g_globals.threadQueue.entries + g_globals.threadQueue.nextEntryToWrite;
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
      DoNextThreadQueueEntry( g_globals.threadInfoArray );
   }

   g_globals.threadQueue.completionGoal = 0;
   g_globals.threadQueue.completionCount = 0;
}

uint64_t Platform_GetJobsDoneByThread( uint32_t threadIndex )
{
   return g_globals.threadInfoArray[threadIndex].jobsDone;
}
