#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "font.h"
#include "platform.h"

#define STARTCODEPOINT      32       // space
#define ENDCODEPOINT        126      // tilde

internal void LoadTTF( const char* filePath, const char* fileName, Font_t* font );
internal void WriteGFF( const char* filePath, const char* fileName, Font_t* font );

// TODO: we probably want this to come from the command line, so we can have
// different sizes for different TTF files.
global float g_glyphHeights[] = { 12.0f, 16.0f, 24.0f, 48.0f, 72.0f };

int main( int argc, char** argv )
{
   HANDLE hFile;
   WIN32_FIND_DATAA findData;
   char sourceDir[STRING_SIZE_DEFAULT];
   char destDir[STRING_SIZE_DEFAULT];
   char fileFilter[STRING_SIZE_DEFAULT];
   char sourceFilePath[STRING_SIZE_DEFAULT];
   char destFilePath[STRING_SIZE_DEFAULT];
   char destFileName[STRING_SIZE_DEFAULT];
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

   font.glyphCollections = 0;

   do
   {
      if ( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
      {
         font.codepointOffset = STARTCODEPOINT;
         font.numGlyphCollections = (uint32_t)( sizeof( g_glyphHeights ) / 4 );
         font.numGlyphs = (uint32_t)( ( ENDCODEPOINT - STARTCODEPOINT ) + 1 );

         strcpy_s( sourceFilePath, STRING_SIZE_DEFAULT, sourceDir );
         strcat_s( sourceFilePath, STRING_SIZE_DEFAULT, findData.cFileName );
         strcpy_s( destFilePath, STRING_SIZE_DEFAULT, destDir );
         strcat_s( destFilePath, STRING_SIZE_DEFAULT, findData.cFileName );
         strcpy_s( destFileName, STRING_SIZE_DEFAULT, findData.cFileName );

         destFilePath[strlen( destFilePath ) - 3] = 'g';
         destFilePath[strlen( destFilePath ) - 2] = 'f';
         destFilePath[strlen( destFilePath ) - 1] = 'f';

         destFileName[strlen( destFileName ) - 3] = 'g';
         destFileName[strlen( destFileName ) - 2] = 'f';
         destFileName[strlen( destFileName ) - 1] = 'f';

         LoadTTF( sourceFilePath, findData.cFileName, &font );
         WriteGFF( destFilePath, destFileName, &font );
      }
   }
   while( FindNextFileA( hFile, &findData ) != 0 );

   printf( "\nFont conversion complete!\n\n" );

   return 0;
}

internal void LoadTTF( const char* filePath, const char* fileName, Font_t* font )
{
   FileData_t fileData;
   uint8_t* filePos;
   stbtt_fontinfo fontInfo;
   int32_t width, height, yOffset, pitch, x, y, codepoint, glyphIndex, baseline, lineGap, baselineOffset, advance, leftBearing;
   uint8_t *monoCodepointMemory, *codepointMemory, *source, *destRow;
   uint32_t* dest;
   uint8_t alpha;
   uint32_t i;
   float scale;
   FontGlyphCollection_t* collection;
   char msg[STRING_SIZE_DEFAULT];

   snprintf( msg, STRING_SIZE_DEFAULT, "Reading glyphs from %s...", fileName );
   printf( msg );

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: could not open file: %s\n\n", filePath );
      printf( msg );
      exit( 1 );
   }

   filePos = (uint8_t*)( fileData.contents );

   if ( !stbtt_InitFont( &fontInfo, filePos, stbtt_GetFontOffsetForIndex( filePos, 0 ) ) )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: could not load font data: %s\n\n", filePath );
      printf( msg );
      exit( 1 );
   }

   font->glyphCollections = (FontGlyphCollection_t*)Platform_MemAlloc( sizeof( FontGlyphCollection_t ) * font->numGlyphCollections );
   collection = font->glyphCollections;

   for ( i = 0; i < font->numGlyphCollections; i++ )
   {
      collection->height = g_glyphHeights[i];
      scale = stbtt_ScaleForPixelHeight( &fontInfo, (float)( collection->height ) );
      stbtt_GetFontVMetrics( &fontInfo, 0, &baseline, &lineGap);
      collection->baseline = -baseline * scale;
      collection->lineGap = lineGap * scale;
      collection->glyphs = (FontGlyph_t*)Platform_MemAlloc( sizeof( FontGlyph_t ) * font->numGlyphs );

      for ( codepoint = STARTCODEPOINT; codepoint <= ENDCODEPOINT; codepoint++ )
      {
         monoCodepointMemory = stbtt_GetCodepointBitmap( &fontInfo, 0, scale, codepoint, &width, &height, 0, &yOffset );
         pitch = width * 4;
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

         glyphIndex = codepoint - STARTCODEPOINT;

         stbtt_GetCodepointBox( &fontInfo, codepoint, 0, &baselineOffset, 0, 0 );
         collection->glyphs[glyphIndex].baselineOffset = ( baselineOffset + yOffset ) * scale;

         stbtt_GetCodepointHMetrics( &fontInfo, codepoint, &advance, &leftBearing );
         collection->glyphs[glyphIndex].advance = advance * scale;
         collection->glyphs[glyphIndex].leftBearing = leftBearing * scale;

         collection->glyphs[glyphIndex].pixelBuffer.memory = (uint8_t*)codepointMemory;
         collection->glyphs[glyphIndex].pixelBuffer.dimensions.x = (uint32_t)width;
         collection->glyphs[glyphIndex].pixelBuffer.dimensions.y = (uint32_t)height;

         stbtt_FreeBitmap( monoCodepointMemory, 0 );
      }

      collection++;
   }

   printf( "done!\n" );
   Platform_ClearFileData( &fileData );
}

internal void WriteGFF( const char* filePath, const char* fileName, Font_t* font )
{
   FileData_t fileData;
   uint32_t i, j, k;
   FontGlyphCollection_t* collection;
   FontGlyph_t* glyph;
   uint32_t* filePos32;
   char msg[STRING_SIZE_DEFAULT];

   snprintf( msg, STRING_SIZE_DEFAULT, "Writing GFF data to %s...", fileName );
   printf( msg );

   strcpy_s( fileData.filePath, STRING_SIZE_DEFAULT, filePath );

   // codepoint offset, number of glyph collections, and number of glyphs, each 4 bytes
   fileData.fileSize = 12;
   collection = font->glyphCollections;

   for ( i = 0; i < font->numGlyphCollections; i++ )
   {
      // height, baseline, and lineGap, each 4 bytes
      fileData.fileSize += 12;
      glyph = collection->glyphs;

      for ( j = 0; j < font->numGlyphs; j++ )
      {
         // left bearing (4 bytes), baseline offset (4 bytes), advance (4 bytes),
         // pixel buffer dimensions (8 bytes), and pixel buffer size
         fileData.fileSize += 20;
         fileData.fileSize += ( glyph->pixelBuffer.dimensions.x * glyph->pixelBuffer.dimensions.y * 4 );
         glyph++;
      }

      collection++;
   }

   fileData.contents = Platform_MemAlloc( fileData.fileSize );

   filePos32 = (uint32_t*)fileData.contents;
   filePos32[0] = font->codepointOffset;
   filePos32[1] = font->numGlyphCollections;
   filePos32[2] = font->numGlyphs;
   filePos32 += 3;
   collection = font->glyphCollections;

   for ( i = 0; i < font->numGlyphCollections; i++ )
   {
      ( (float*)filePos32 )[0] = collection->height;
      ( (float*)filePos32 )[1] = collection->baseline;
      ( (float*)filePos32 )[2] = collection->lineGap;
      filePos32 += 3;
      glyph = collection->glyphs;

      for ( j = 0; j < font->numGlyphs; j++ )
      {
         ( (float*)filePos32 )[0] = glyph->leftBearing;
         ( (float*)filePos32 )[1] = glyph->baselineOffset;
         ( (float*)filePos32 )[2] = glyph->advance;
         filePos32[3] = glyph->pixelBuffer.dimensions.x;
         filePos32[4] = glyph->pixelBuffer.dimensions.y;
         filePos32 += 5;

         for ( k = 0; k < ( glyph->pixelBuffer.dimensions.x * glyph->pixelBuffer.dimensions.y ); k++ )
         {
            filePos32[k] = ( (uint32_t*)( glyph->pixelBuffer.memory ) )[k];
         }

         filePos32 += ( glyph->pixelBuffer.dimensions.x * glyph->pixelBuffer.dimensions.y );
         glyph++;
      }

      collection++;
   }

   if ( !Platform_WriteFileData( &fileData ) )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: could not write to file: %s\n\n", filePath );
      printf( msg );
      exit( 1 );
   }

   printf( "done!\n" );
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

// unused platform functions
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
