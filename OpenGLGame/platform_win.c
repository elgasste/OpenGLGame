#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <gl/GL.h>

#include "game.h"
#include "platform.h"

typedef struct
{
   HWND hWndMain;
   cScreenBuffer_t screenBuffer;
   cGameData_t gameData;
   LARGE_INTEGER performanceFrequency;
   uint32_t keyCodeMap[(int)cKeyCode_Count];
}
cGlobalObjects_t;

global cGlobalObjects_t g_globals;

internal void FatalError( const wchar_t* message );
internal void InitKeyCodeMap();
internal void InitOpenGL( HWND hWnd );
internal LRESULT CALLBACK MainWindowProc( _In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam );
internal void HandleKeyboardInput( uint32_t keyCode, LPARAM flags );

int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
{
   TIMECAPS timeCaps;
   UINT timerResolution;
   WNDCLASS mainWindowClass = { 0 };
   int bytesPerPixel;

   UNUSED_PARAM( hPrevInstance );
   UNUSED_PARAM( lpCmdLine );
   UNUSED_PARAM( nCmdShow );

   InitKeyCodeMap();

   if ( !QueryPerformanceFrequency( &( g_globals.performanceFrequency ) ) )
   {
      FatalError( _T( STR_WINERR_PERFORMANCEFREQUENCY ) );
   }

   if ( timeGetDevCaps( &timeCaps, sizeof( TIMECAPS ) ) != TIMERR_NOERROR )
   {
      FatalError( _T( STR_WINERR_TIMERRESOLUTION ) );
   }

   timerResolution = min( max( timeCaps.wPeriodMin, 1 ), timeCaps.wPeriodMax );
   timeBeginPeriod( timerResolution );

   bytesPerPixel = GRAPHICS_BPP / 8;
   g_globals.screenBuffer.memory = VirtualAlloc( 0, (SIZE_T)( SCREEN_BUFFER_WIDTH * SCREEN_BUFFER_HEIGHT ) * bytesPerPixel, MEM_COMMIT, PAGE_READWRITE );
   g_globals.screenBuffer.pitch = SCREEN_BUFFER_WIDTH * bytesPerPixel;

   mainWindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
   mainWindowClass.lpfnWndProc = MainWindowProc;
   mainWindowClass.hInstance = hInstance;
   mainWindowClass.lpszClassName = _T( "mainWindowClass" );

   if ( !RegisterClass( &mainWindowClass ) )
   {
      FatalError( _T( STR_WINERR_REGISTERWINDOW ) );
   }

   g_globals.hWndMain = CreateWindowEx( 0,
                                        mainWindowClass.lpszClassName,
                                        _T( STR_WIN_WINDOWTITLE ),
                                        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        SCREEN_BUFFER_WIDTH,
                                        SCREEN_BUFFER_HEIGHT,
                                        0,
                                        0,
                                        hInstance,
                                        0 );

   if ( !g_globals.hWndMain )
   {
      FatalError( _T( STR_WINERR_CREATEWINDOW ) );
   }

   InitKeyCodeMap();
   InitOpenGL( g_globals.hWndMain );
   cGame_Init( &( g_globals.gameData ) );
   cGame_Run( &( g_globals.gameData ) );

   return 0;
}

internal void FatalError( const wchar_t* message )
{
   // TODO: logging
   cGame_EmergencySave( &( g_globals.gameData ) );
   MessageBox( 0, message, _T( STR_WINERR_HEADER ), MB_OK | MB_ICONERROR );
   exit( 1 );
}

internal void InitKeyCodeMap()
{
   g_globals.keyCodeMap[(int)cKeyCode_Left] = VK_LEFT;
   g_globals.keyCodeMap[(int)cKeyCode_Up] = VK_UP;
   g_globals.keyCodeMap[(int)cKeyCode_Right] = VK_RIGHT;
   g_globals.keyCodeMap[(int)cKeyCode_Down] = VK_DOWN;
   g_globals.keyCodeMap[(int)cKeyCode_Escape] = VK_ESCAPE;
}

internal void InitOpenGL( HWND hWnd )
{
   PIXELFORMATDESCRIPTOR desiredPixelFormat = { 0 };
   PIXELFORMATDESCRIPTOR suggestedPixelFormat;
   int suggestedPixelFormatIndex;
   HGLRC glRC;
   GLuint textureHandle;
   HDC dc = GetDC( hWnd );

   desiredPixelFormat.nSize = sizeof( desiredPixelFormat );
   desiredPixelFormat.nVersion = 1;
   desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
   desiredPixelFormat.cColorBits = GRAPHICS_BPP;
   desiredPixelFormat.cAlphaBits = GRAPHICS_ALPHABITS;
   desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

   suggestedPixelFormatIndex = ChoosePixelFormat( dc, &desiredPixelFormat );

   if ( !suggestedPixelFormatIndex )
   {
      FatalError( _T( STR_WINERR_NOPIXELFORMAT ) );
   }

   DescribePixelFormat( dc, suggestedPixelFormatIndex, sizeof( suggestedPixelFormat ), &suggestedPixelFormat );

   if ( suggestedPixelFormat.cColorBits != GRAPHICS_BPP || suggestedPixelFormat.cAlphaBits != GRAPHICS_ALPHABITS )
   {
      FatalError( _T( STR_WINERR_UNSUITABLEPIXELFORMAT ) );
   }

   SetPixelFormat( dc, suggestedPixelFormatIndex, &suggestedPixelFormat );
   glRC = wglCreateContext( dc );

   if ( !wglMakeCurrent( dc, glRC ) )
   {
      FatalError( _T( STR_WINERR_RENDERINGCONTEXT ) );
   }

   glGenTextures( 1, &textureHandle );
   glBindTexture( GL_TEXTURE_2D, textureHandle );

   ReleaseDC( hWnd, dc );
}

internal LRESULT CALLBACK MainWindowProc( _In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam )
{
   LRESULT result = 0;

   switch ( uMsg )
   {
      case WM_CLOSE:
         cGame_TryClose( &( g_globals.gameData ) );
         break;
      case WM_DESTROY:
         if ( g_globals.gameData.isRunning )
         {
            cGame_EmergencySave( &( g_globals.gameData ) );
            g_globals.gameData.isRunning = cFalse;
         }
         break;
      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
         HandleKeyboardInput( (uint32_t)wParam, lParam );
         break;
      default:
         result = DefWindowProc( hWnd, uMsg, wParam, lParam );
   }

   return result;
}

void Platform_Tick()
{
   MSG msg;

   while ( PeekMessage( &msg, g_globals.hWndMain, 0, 0, PM_REMOVE ) )
   {
      if ( msg.message == WM_QUIT )
      {
         cGame_TryClose( &( g_globals.gameData ) );
         break;
      }
      else
      {
         TranslateMessage( &msg );
         DispatchMessage( &msg );
      }
   }
}

cScreenBuffer_t* Platform_GetScreenBuffer()
{
   return &( g_globals.screenBuffer );
}

void Platform_RenderScreen()
{
   GLfloat modelMatrix[] = 
   {
      2.0f / SCREEN_BUFFER_WIDTH, 0.0f, 0.0f, 0.0f,
      0, 2.0f / SCREEN_BUFFER_HEIGHT, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, 0.0f, 1.0f
   };
   HDC dc = GetDC( g_globals.hWndMain );

   glViewport( 0, 0, SCREEN_BUFFER_WIDTH, SCREEN_BUFFER_HEIGHT );

   glTexImage2D( GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 SCREEN_BUFFER_WIDTH,
                 SCREEN_BUFFER_HEIGHT,
                 0,
                 GL_BGRA_EXT,
                 GL_UNSIGNED_BYTE,
                 g_globals.screenBuffer.memory );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

   glEnable( GL_TEXTURE_2D );

   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glEnable( GL_BLEND );

   glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   glClear( GL_COLOR_BUFFER_BIT );

   glMatrixMode( GL_TEXTURE );
   glLoadIdentity();

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();

   glMatrixMode( GL_PROJECTION );
   glLoadMatrixf( modelMatrix );

   glBegin( GL_TRIANGLES );

   // lower triangle
   glTexCoord2f( 0.0f, 0.0f );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( 1.0f, 0.0f );
   glVertex2f( SCREEN_BUFFER_WIDTH, 0.0f );
   glTexCoord2f( 1.0f, 1.0f );
   glVertex2f( SCREEN_BUFFER_WIDTH, SCREEN_BUFFER_HEIGHT );

   // upper triangle
   glTexCoord2f( 0.0f, 0.0f );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( 1.0f, 1.0f );
   glVertex2f( SCREEN_BUFFER_WIDTH, SCREEN_BUFFER_HEIGHT );
   glTexCoord2f( 0.0f, 1.0f );
   glVertex2f( 0.0f, SCREEN_BUFFER_HEIGHT );

   glEnd();

   SwapBuffers( dc );

   ReleaseDC( g_globals.hWndMain, dc );
}

uint64_t Platform_GetTimeStampMicro()
{
   LARGE_INTEGER ticks;
   QueryPerformanceCounter( &ticks );

   return (uint64_t)( ( (double)( ticks.QuadPart ) / (double)( g_globals.performanceFrequency.QuadPart ) ) * (uint64_t)1000000 );
}

void Platform_Sleep( uint64_t startMicro, uint64_t sleepMicro )
{
   uint64_t timeStampMicro;
   uint64_t milli = ( sleepMicro / 1000 ) - 1;
   uint64_t targetEndMicro = startMicro + sleepMicro;

   // TODO: Windows' Sleep() is super inaccurate, if a frame goes by really fast it
   // has a tendency to sleep too long. Subtracting one millisecond is a band-aid,
   // and should probably be revisited later. Subtracting two milliseconds makes it
   // even more accurate, but I'm not sure it matters enough at this point.
   if ( milli > 0 )
   {
      Sleep( (DWORD)( milli ) );
   }

   do
   {
      timeStampMicro = Platform_GetTimeStampMicro();
   }
   while ( timeStampMicro < targetEndMicro );
}

internal void HandleKeyboardInput( uint32_t keyCode, LPARAM flags )
{
   cBool_t keyWasDown = ( flags & ( (LONG_PTR)1 << 30 ) ) != 0 ? cTrue : cFalse;
   cBool_t keyIsDown = ( flags & ( (LONG_PTR)1 << 31 ) ) == 0 ? cTrue : cFalse;
   int i;

   // ignore repeat presses
   if ( keyWasDown != keyIsDown )
   {
      if ( keyIsDown )
      {
         // ensure alt+F4 still closes the window
         if ( keyCode == VK_F4 && ( flags & ( (LONG_PTR)1 << 29 ) ) )
         {
            cGame_TryClose( &( g_globals.gameData ) );
            return;
         }

         for ( i = 0; i < cKeyCode_Count; i++ )
         {
            if ( g_globals.keyCodeMap[i] == keyCode )
            {
               cInput_PressKey( g_globals.gameData.keyStates, (cKeyCode_t)i );
               break;
            }
         }
      }
      else
      {
         for ( i = 0; i < cKeyCode_Count; i++ )
         {
            if ( g_globals.keyCodeMap[i] == keyCode )
            {
               cInput_ReleaseKey( g_globals.gameData.keyStates, (cKeyCode_t)i );
               break;
            }
         }
      }
   }
}
