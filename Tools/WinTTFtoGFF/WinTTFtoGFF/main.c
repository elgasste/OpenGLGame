#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "font.h"
#include "platform.h"

internal void LoadTTF( const char* filePath, Font_t* font );
internal void WriteGFF( const char* filePath, Font_t* font );

int main( int argc, char** argv )
{
   HANDLE hFile;
   WIN32_FIND_DATAA findData;
   char sourceDir[STRING_SIZE_DEFAULT];
   char destDir[STRING_SIZE_DEFAULT];
   char fileFilter[STRING_SIZE_DEFAULT];
   char sourceFilePath[STRING_SIZE_DEFAULT];
   char destFilePath[STRING_SIZE_DEFAULT];
   Font_t font;

   if ( argc < 3 )
   {
      printf( "ERROR: not enough arguments, arg1 should be source TTF directory, arg2 should be destination GFF directory.\n\n" );
      exit( 1 );
   }

   strcpy_s( sourceDir, STRING_SIZE_DEFAULT, argv[1] );
   if ( sourceDir[strlen( sourceDir - 1 )] != '\\' )
   {
      strcat_s( sourceDir, STRING_SIZE_DEFAULT, "\\" );
   }

   strcpy_s( destDir, STRING_SIZE_DEFAULT, argv[2] );
   if ( destDir[strlen( destDir - 1 )] != '\\' )
   {
      strcat_s( destDir, STRING_SIZE_DEFAULT, "\\" );
   }

   strcpy_s( fileFilter, STRING_SIZE_DEFAULT, sourceDir );
   strcat_s( fileFilter, STRING_SIZE_DEFAULT, "*.ttf" );
   hFile = FindFirstFileA( fileFilter, &findData );

   if ( hFile == INVALID_HANDLE_VALUE )
   {
      printf( "ERROR: could not find source folder, or no files exist in source folder.\n\n" );
      exit( 1 );
   }

   font.glyphs = 0;

   do
   {
      if ( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
      {
         font.codepointOffset = FONT_STARTCODEPOINT;
         font.numGlyphs = (uint32_t)( ( FONT_ENDCODEPOINT - FONT_STARTCODEPOINT ) + 1 );
         if ( font.glyphs )
         {
            Platform_MemFree( font.glyphs );
         }
         font.glyphs = (FontGlyph_t*)Platform_MemAlloc( font.numGlyphs * sizeof( PixelBuffer_t ) );

         strcpy_s( sourceFilePath, STRING_SIZE_DEFAULT, sourceDir );
         strcat_s( sourceFilePath, STRING_SIZE_DEFAULT, findData.cFileName );
         strcpy_s( destFilePath, STRING_SIZE_DEFAULT, destDir );
         strcat_s( destFilePath, STRING_SIZE_DEFAULT, findData.cFileName );

         destFilePath[strlen( destFilePath ) - 3] = 'g';
         destFilePath[strlen( destFilePath ) - 2] = 'f';
         destFilePath[strlen( destFilePath ) - 1] = 'f';

         LoadTTF( sourceFilePath, &font );
         WriteGFF( destFilePath, &font );
      }
   }
   while( FindNextFileA( hFile, &findData ) != 0 );

   return 0;
}

internal void LoadTTF( const char* filePath, Font_t* font )
{
   FileData_t fileData;
   uint8_t* filePos;
   stbtt_fontinfo fontInfo;
   int32_t width, height, xOffset, yOffset, pitch, x, y, codepoint, glyphIndex;
   uint8_t *monoCodepointMemory, *codepointMemory, *source, *destRow;
   uint32_t* dest;
   uint8_t alpha;
   float scale;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, "ERROR: could not open file: %s\n\n", filePath );
      printf( errorMsg );
      exit( 1 );
   }

   filePos = (uint8_t*)( fileData.contents );

   if ( !stbtt_InitFont( &fontInfo, filePos, stbtt_GetFontOffsetForIndex( filePos, 0 ) ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, "ERROR: could not load font data: %s\n\n", filePath );
      printf( errorMsg );
      exit( 1 );
   }

   // TODO: we should probably offer the option of passing in a raw pixel height as a parameter,
   // or even allow several different glyph heights in the same font.
   scale = stbtt_ScaleForPixelHeight( &fontInfo, FONT_RAWPIXELHEIGHT );
   stbtt_GetFontVMetrics( &fontInfo, 0, &( font->descent ), &( font->lineGap ) );
   font->descent = (int32_t)( font->descent * scale );
   font->lineGap = (int32_t)( font->lineGap * scale );

   printf( "Reading glyphs..." );

   for ( codepoint = FONT_STARTCODEPOINT; codepoint <= FONT_ENDCODEPOINT; codepoint++ )
   {
      monoCodepointMemory = stbtt_GetCodepointBitmap( &fontInfo, 0, scale, codepoint, &width, &height, &xOffset, &yOffset );
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
            *dest = 0x00FFFFFF | ( (uint32_t)alpha << 24 );
            dest++;
         }

         destRow -= pitch;
      }

      glyphIndex = codepoint - FONT_STARTCODEPOINT;

      stbtt_GetGlyphHMetrics( &fontInfo, codepoint, &( font->glyphs[glyphIndex].advance ), 0);
      font->glyphs[glyphIndex].advance = (int32_t)( font->glyphs[glyphIndex].advance * scale );

      stbtt_GetGlyphBox( &fontInfo, codepoint,
                         &( font->glyphs[glyphIndex].offset.x ), &( font->glyphs[glyphIndex].offset.y ),
                         0, 0 );
      font->glyphs[glyphIndex].offset.x = (int32_t)( font->glyphs[glyphIndex].offset.x * scale );
      font->glyphs[glyphIndex].offset.y = (int32_t)( font->glyphs[glyphIndex].offset.y * scale );

      font->glyphs[glyphIndex].pixelBuffer.memory = (uint8_t*)codepointMemory;
      font->glyphs[glyphIndex].pixelBuffer.dimensions.x = (uint32_t)width;
      font->glyphs[glyphIndex].pixelBuffer.dimensions.y = (uint32_t)height;

      stbtt_FreeBitmap( monoCodepointMemory, 0 );
   }

   printf( "done!\n" );
   Platform_ClearFileData( &fileData );
}

internal void WriteGFF( const char* filePath, Font_t* font )
{
   FileData_t fileData;
   uint32_t i, j;
   FontGlyph_t* glyph;
   char errorMsg[STRING_SIZE_DEFAULT];
   uint32_t* filePos32;

   printf( "Writing GFF data..." );

   strcpy_s( fileData.filePath, STRING_SIZE_DEFAULT, filePath );

   // codepoint offset, descent, line gap, and number of glyphs, all 4 bytes
   fileData.fileSize = 16;
   glyph = font->glyphs;

   for ( i = 0; i < font->numGlyphs; i++ )
   {
      // offset (8 bytes), advance (4 bytes), pixel buffer dimensions (8 bytes), and pixel buffer size
      fileData.fileSize += 20;
      fileData.fileSize += ( ( glyph->pixelBuffer.dimensions.x * glyph->pixelBuffer.dimensions.y ) * ( GRAPHICS_BPP / 8 ) );
      glyph++;
   }

   fileData.contents = Platform_MemAlloc( fileData.fileSize );

   filePos32 = (uint32_t*)fileData.contents;
   filePos32[0] = font->codepointOffset;
   filePos32[1] = font->descent;
   filePos32[2] = font->lineGap;
   filePos32[3] = font->numGlyphs;
   filePos32 += 4;

   glyph = font->glyphs;

   for ( i = 0; i < font->numGlyphs; i++ )
   {
      filePos32[0] = glyph->offset.x;
      filePos32[1] = glyph->offset.y;
      filePos32[2] = glyph->advance;
      filePos32[3] = glyph->pixelBuffer.dimensions.x;
      filePos32[4] = glyph->pixelBuffer.dimensions.y;
      filePos32 += 5;

      for ( j = 0; j < ( glyph->pixelBuffer.dimensions.x * glyph->pixelBuffer.dimensions.y ); j++ )
      {
         filePos32[j] = ( (uint32_t*)( glyph->pixelBuffer.memory ) )[j];
      }

      filePos32 += ( glyph->pixelBuffer.dimensions.x * glyph->pixelBuffer.dimensions.y );
      glyph++;
   }

   if ( !Platform_WriteFileData( &fileData ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, "ERROR: could not write to file: %s\n\n", filePath );
      printf( errorMsg );
      exit( 1 );
   }

   printf( "done!\n\n" );
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
