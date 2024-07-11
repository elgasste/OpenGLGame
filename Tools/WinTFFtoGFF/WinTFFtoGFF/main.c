#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "platform.h"

#define FONT_PIXEL_HEIGHT 128.0f

internal void LoadCodepoints( const char* filePath );

int main( int argc, char** argv )
{
   if ( argc < 2 )
   {
      printf( "ERROR: not enough arguments, expecting the path to a TTF file!\n\n" );
      exit( 1 );
   }

   printf( "Loading codepoints..." );
   LoadCodepoints( argv[1] );
   printf( "done!\n" );

   return 0;
}

internal void LoadCodepoints( const char* filePath )
{
   FileData_t fileData;
   uint8_t* fileDataPos;
   stbtt_fontinfo fontInfo;
   int32_t width, height, xOffset, yOffset, pitch, x, y, codepoint;
   uint8_t *monoCodepointMemory, *codepointMemory, *source, *destRow;
   uint32_t* dest;
   uint8_t alpha;
   float scaleForPixelHeight;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, "ERROR: could not open file: %s\n\n", filePath );
      printf( errorMsg );
      exit( 1 );
   }

   fileDataPos = (uint8_t*)( fileData.contents );

   if ( !stbtt_InitFont( &fontInfo, fileDataPos, stbtt_GetFontOffsetForIndex( fileDataPos, 0 ) ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, "ERROR: could not initialize font: %s\n\n", filePath );
      printf( errorMsg );
      exit( 1 );
   }

   // from space to tilde, should be everything we would need
   for ( codepoint = 32; codepoint <= 126; codepoint++ )
   {
      scaleForPixelHeight = stbtt_ScaleForPixelHeight( &fontInfo, FONT_PIXEL_HEIGHT );
      monoCodepointMemory = stbtt_GetCodepointBitmap( &fontInfo, 0, scaleForPixelHeight, codepoint, &width, &height, &xOffset, &yOffset );
      pitch = width * ( GRAPHICS_BPP / 8 );
      codepointMemory = Platform_MemAlloc( height * pitch );
      source = monoCodepointMemory;
      destRow = codepointMemory + ( ( height - 1 ) * pitch );

      for ( y = 0; y < height; y++ )
      {
         dest = (uint32_t*)destRow;

         for ( x = 0; x < width; x++ )
         {
            alpha = *source;
            source++;
            *dest = ( (uint32_t)alpha << 24 ) | ( (uint32_t)alpha << 16 ) | ( (uint32_t)alpha << 8 ) | (uint32_t)alpha;
            dest++;
         }

         destRow -= pitch;
      }

      // TODO: we have the codepoint image, what do we do with it?

      Platform_MemFree( codepointMemory );
      stbtt_FreeBitmap( monoCodepointMemory, 0 );
   }

   Platform_ClearFileData( &fileData );
}

void* Platform_MemAlloc( uint64_t size )
{
   return VirtualAlloc( 0, size, MEM_COMMIT, PAGE_READWRITE );
}

void Platform_MemFree( void* memory )
{
   VirtualFree( memory, 0, MEM_RELEASE );
}

Bool_t Platform_ReadFileData( const char* filePath, FileData_t* fileData )
{
   HANDLE hFile;
   LARGE_INTEGER fileSize;
   OVERLAPPED overlapped = { 0 };

   strcpy_s( fileData->filePath, STRING_SIZE_DEFAULT, filePath );
   fileData->contents = 0;
   fileData->fileSize = 0;

   hFile = CreateFileA( filePath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

   if ( hFile == INVALID_HANDLE_VALUE )
   {
      return False;
   }

   if ( !GetFileSizeEx( hFile, &fileSize ) )
   {
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

Bool_t Platform_WriteFileData( const char* filePath, FileData_t* fileData )
{
   HANDLE hFile;
   OVERLAPPED overlapped = { 0 };

   hFile = CreateFileA( filePath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

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

void Platform_Log( const char* message ) { UNUSED_PARAM( message ); }
void Platform_Tick() { }
void Platform_RenderScreen() { }
uint64_t Platform_GetTimeStampMicro() { return 0; }
void Platform_Sleep( uint64_t micro ) { UNUSED_PARAM( micro ); }
Bool_t Platform_GetAppDirectory( char* directory, uint32_t stringSize )
{
   UNUSED_PARAM( directory );
   UNUSED_PARAM( stringSize );
   return 0;
}
