#include <assert.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "font.h"
#include "platform.h"

#define STARTCODEPOINT     32       // space
#define ENDCODEPOINT       126      // tilde

#define NUM_CHUNKS         2

typedef struct
{
   char path[STRING_SIZE_DEFAULT];
   char dir[STRING_SIZE_DEFAULT];
   char name[STRING_SIZE_DEFAULT];
}
FileParts_t;

typedef struct
{
   char fileName[STRING_SIZE_DEFAULT];
   FontID_t ID;
}
ResourceIDMapping_t;

typedef struct
{
   char fileName[STRING_SIZE_DEFAULT];
   Font_t font;
}
FontData_t;

typedef struct
{
   char fileName[STRING_SIZE_DEFAULT];
   uint32_t size;
   uint8_t* memory;
}
BitmapData_t;

typedef struct
{
   uint32_t numFonts;
   FontData_t* fontDatas;
   uint32_t numBitmaps;
   BitmapData_t* bitmapDatas;
}
GameResources_t;

global float g_glyphHeights[] = { 12.0f, 16.0f, 24.0f, 48.0f, 72.0f };
global ResourceIDMapping_t g_fontIDMap[] = {
   { "Consolas.ttf", (uint32_t)FontID_Consolas },
   { "Papyrus.ttf", (uint32_t)FontID_Papyrus }
};
global ResourceIDMapping_t g_bitmapIDMap[] = {
   { "background.bmp", (uint32_t)ImageID_Background },
   { "star_sprite.bmp", (uint32_t)ImageID_Star }
};

internal FileParts_t* GetFiles( const char* dir, const char* filter, uint32_t* numFiles );
internal FontData_t* LoadFontsFromDir( const char* dir, const char* filter, uint32_t* numFonts );
internal void LoadFontFromFile( FontData_t* fontData, const char* filePath );
internal BitmapData_t* LoadBitmapsFromDir( const char* dir, const char* filter, uint32_t* numBitmaps );
internal void WriteGameDataFile( GameResources_t* resources, const char* dir );
internal uint32_t GetGameDataFileSize( GameResources_t* resources );
internal uint32_t GetFontDataMemSize( FontData_t* fontData );
internal void WriteFontData( FontData_t* fontData, FileData_t* fileData, uint32_t fileOffset );
internal uint32_t GetResourceIDForFileName( ResourceIDMapping_t mappings[], uint32_t numMappings, const char* fileName );

int main( int argc, char** argv )
{
   GameResources_t resources;
   char resourcesDir[STRING_SIZE_DEFAULT];
   char fontsDir[STRING_SIZE_DEFAULT];
   char bitmapsDir[STRING_SIZE_DEFAULT];
   char destDir[STRING_SIZE_DEFAULT];
   char fontFilter[STRING_SIZE_DEFAULT];
   char bitmapFilter[STRING_SIZE_DEFAULT];

   if ( argc < 3 )
   {
      printf( "ERROR: not enough arguments, arg1 should be the Resources directory, arg2 should be the game source directory.\n\n" );
      exit( 1 );
   }

   strcpy_s( resourcesDir, STRING_SIZE_DEFAULT, argv[1] );
   if ( resourcesDir[strlen( resourcesDir ) - 1] != '\\' )
   {
      strcat_s( resourcesDir, STRING_SIZE_DEFAULT, "\\" );
   }

   strcpy_s( destDir, STRING_SIZE_DEFAULT, argv[2] );
   if ( destDir[strlen( destDir ) - 1] != '\\' )
   {
      strcat_s( destDir, STRING_SIZE_DEFAULT, "\\" );
   }

   strcpy_s( fontsDir, STRING_SIZE_DEFAULT, resourcesDir );
   strcat_s( fontsDir, STRING_SIZE_DEFAULT, "fonts\\" );
   strcpy_s( bitmapsDir, STRING_SIZE_DEFAULT, resourcesDir );
   strcat_s( bitmapsDir, STRING_SIZE_DEFAULT, "bitmaps\\" );

   strcpy_s( fontFilter, STRING_SIZE_DEFAULT, fontsDir );
   strcat_s( fontFilter, STRING_SIZE_DEFAULT, "*.ttf" );
   strcpy_s( bitmapFilter, STRING_SIZE_DEFAULT, bitmapsDir );
   strcat_s( bitmapFilter, STRING_SIZE_DEFAULT, "*.bmp" );

   resources.fontDatas = LoadFontsFromDir( fontsDir, fontFilter, &( resources.numFonts ) );
   resources.bitmapDatas = LoadBitmapsFromDir( bitmapsDir, bitmapFilter, &( resources.numBitmaps ) );

   printf( "\n" );

   WriteGameDataFile( &resources, destDir );

   printf( "\n" );

   return 0;
}

void* Platform_MemAlloc( uint64_t size )
{
   return VirtualAlloc( 0, size, MEM_COMMIT, PAGE_READWRITE );
}

void Platform_MemFree( void* memory, uint64_t size )
{
   UNUSED_PARAM( size );
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

internal FileParts_t* GetFiles( const char* dir, const char* filter, uint32_t* numFiles )
{
   HANDLE hFile;
   WIN32_FIND_DATAA findData;
   uint32_t i = 0;
   FileParts_t* files;
   char msg[STRING_SIZE_DEFAULT];

   *numFiles = 0;
   hFile = FindFirstFileA( filter, &findData );

   if ( hFile == INVALID_HANDLE_VALUE )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: could not retrieve files from filter %s\n\n", filter );
      printf( msg );
      exit( 1 );
   }

   do
   {
      if ( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
      {
         ( *numFiles )++;
      }
   }
   while( FindNextFileA( hFile, &findData ) != 0 );

   files = (FileParts_t*)Platform_MemAlloc( sizeof( FileParts_t ) * ( *numFiles ) );
   hFile = FindFirstFileA( filter, &findData );

   do
   {
      if ( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
      {
         strcpy_s( files[i].dir, STRING_SIZE_DEFAULT, dir );
         strcpy_s( files[i].name, STRING_SIZE_DEFAULT, findData.cFileName );
         strcpy_s( files[i].path, STRING_SIZE_DEFAULT, files[i].dir );
         strcat_s( files[i].path, STRING_SIZE_DEFAULT, files[i].name );
         i++;
      }
   }
   while( FindNextFileA( hFile, &findData ) != 0 );

   return files;
}

internal FontData_t* LoadFontsFromDir( const char* dir, const char* filter, uint32_t* numFonts )
{
   uint32_t i;
   FileParts_t* files;
   FontData_t* fontDatas;
   FontData_t* fontData;

   files = GetFiles( dir, filter, numFonts );
   fontDatas = (FontData_t*)Platform_MemAlloc( sizeof( FontData_t ) * ( *numFonts ) );
   fontData = fontDatas;

   for ( i = 0; i < ( *numFonts ); i++ )
   {
      strcpy_s( fontData->fileName, STRING_SIZE_DEFAULT, files[i].name );
      LoadFontFromFile( fontData, files[i].path );
      fontData++;
   }

   return fontDatas;
}

internal void LoadFontFromFile( FontData_t* fontData, const char* filePath )
{
   FileData_t fileData;
   uint8_t* filePos;
   stbtt_fontinfo fontInfo;
   int32_t width, height, xOffset, yOffset, pitch, x, y, codepoint, glyphIndex, baseline, lineGap, baselineOffset, advance, leftBearing;
   uint8_t *monoCodepointMemory, *codepointMemory, *source, *destRow;
   uint32_t* dest;
   uint8_t alpha;
   uint32_t i;
   float scale;
   FontGlyphCollection_t* collection;
   char msg[STRING_SIZE_DEFAULT];

   fontData->font.codepointOffset = STARTCODEPOINT;
   fontData->font.numGlyphCollections = (uint32_t)( sizeof( g_glyphHeights ) / 4 );
   fontData->font.numGlyphs = (uint32_t)( ( ENDCODEPOINT - STARTCODEPOINT ) + 1 );

   snprintf( msg, STRING_SIZE_DEFAULT, "Reading font data from %s...", filePath );
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

   fontData->font.glyphCollections = (FontGlyphCollection_t*)Platform_MemAlloc( sizeof( FontGlyphCollection_t ) * fontData->font.numGlyphCollections );
   collection = fontData->font.glyphCollections;

   for ( i = 0; i < fontData->font.numGlyphCollections; i++ )
   {
      collection->height = g_glyphHeights[i];
      scale = stbtt_ScaleForPixelHeight( &fontInfo, (float)( collection->height ) );
      stbtt_GetFontVMetrics( &fontInfo, 0, &baseline, &lineGap);
      collection->baseline = -baseline * scale;
      collection->lineGap = lineGap * scale;
      collection->glyphs = (FontGlyph_t*)Platform_MemAlloc( sizeof( FontGlyph_t ) * fontData->font.numGlyphs );

      for ( codepoint = STARTCODEPOINT; codepoint <= ENDCODEPOINT; codepoint++ )
      {
         monoCodepointMemory = stbtt_GetCodepointBitmap( &fontInfo, 0, scale, codepoint, &width, &height, &xOffset, &yOffset );
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
         collection->glyphs[glyphIndex].leftBearing = ( leftBearing + xOffset ) * scale;

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

internal BitmapData_t* LoadBitmapsFromDir( const char* dir, const char* filter, uint32_t* numBitmaps )
{
   uint32_t i, j;
   FileParts_t* files;
   FileData_t fileData;
   BitmapData_t* bitmapDatas;
   BitmapData_t* bitmapData;
   char msg[STRING_SIZE_DEFAULT];

   files = GetFiles( dir, filter, numBitmaps );
   bitmapDatas = (BitmapData_t*)Platform_MemAlloc( sizeof( BitmapData_t ) * ( *numBitmaps ) );
   bitmapData = bitmapDatas;

   for ( i = 0; i < ( *numBitmaps ); i++ )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, "Reading bitmap data from %s...", files[i].path );
      printf( msg );

      if ( !Platform_ReadFileData( files[i].path, &fileData) )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: could not open file: %s\n\n", files[i].path );
         printf( msg );
         exit( 1 );
      }

      strcpy_s( bitmapData->fileName, STRING_SIZE_DEFAULT, files[i].name );
      bitmapData->size = fileData.fileSize;
      bitmapData->memory = (uint8_t*)Platform_MemAlloc( fileData.fileSize );

      for ( j = 0; j < fileData.fileSize; j++ )
      {
         bitmapData->memory[j] = ( (uint8_t*)( fileData.contents ) )[j];
      }

      printf( "done!\n" );
      Platform_ClearFileData( &fileData );
      bitmapData++;
   }

   return bitmapDatas;
}

internal void WriteGameDataFile( GameResources_t* resources, const char* dir )
{
   uint32_t i, j, fileOffset, dataSize;
   uint8_t* filePos8;
   uint32_t* filePos32;
   FileData_t fileData;
   FontData_t* fontData;
   BitmapData_t* bitmapData;
   char msg[STRING_SIZE_DEFAULT];

   fileData.fileSize = GetGameDataFileSize( resources );
   snprintf( msg, STRING_SIZE_DEFAULT, "Game data file size: %u bytes\n", fileData.fileSize );
   printf( msg );

   strcpy_s( fileData.filePath, STRING_SIZE_DEFAULT, dir );
   strcat_s( fileData.filePath, STRING_SIZE_DEFAULT, GAME_DATA_FILENAME );
   fileData.contents = Platform_MemAlloc( fileData.fileSize );

   printf( "Writing game data to destination..." );

   // chunk count and first chunk offset
   filePos32 = (uint32_t*)( fileData.contents );
   filePos32[0] = NUM_CHUNKS;
   fileOffset = 4 + ( NUM_CHUNKS * 4 );
   filePos32[1] = fileOffset;

   // fonts chunk
   filePos32 = (uint32_t*)( (uint8_t*)fileData.contents + fileOffset );
   filePos32[0] = (uint32_t)GameDataFileChunkID_Fonts;
   filePos32[1] = resources->numFonts;
   filePos32 += 2;
   fileOffset += 8;

   fontData = resources->fontDatas;

   for ( i = 0; i < resources->numFonts; i++ )
   {
      dataSize = GetFontDataMemSize( fontData );
      filePos32[0] = GetResourceIDForFileName( g_fontIDMap, sizeof( g_fontIDMap ) / sizeof( ResourceIDMapping_t ), fontData->fileName );
      filePos32[1] = dataSize;
      filePos32 += 2;
      fileOffset += 8;

      WriteFontData( fontData, &fileData, fileOffset );

      filePos32 += ( dataSize / 4 );
      fileOffset += dataSize;
      fontData++;
   }

   // bitmaps chunk
   ( (uint32_t*)( fileData.contents ) )[2] = fileOffset;  // chunk offset
   filePos32[0] = (uint32_t)GameDataFileChunkID_Bitmaps;
   filePos32[1] = resources->numBitmaps;
   filePos32 += 2;
   fileOffset += 8;

   bitmapData = resources->bitmapDatas;

   for ( i = 0; i < resources->numBitmaps; i++ )
   {
      filePos32[0] = GetResourceIDForFileName( g_bitmapIDMap, sizeof( g_bitmapIDMap ) / sizeof( ResourceIDMapping_t ), bitmapData->fileName );
      filePos32[1] = bitmapData->size;
      filePos32 += 2;
      fileOffset += 8;
      filePos8 = (uint8_t*)( fileData.contents ) + fileOffset;

      for ( j = 0; j < bitmapData->size; j++ )
      {
         filePos8[j] = bitmapData->memory[j];
      }

      filePos8 += bitmapData->size;
      fileOffset += bitmapData->size;
      filePos32 = (uint32_t*)filePos8;
      bitmapData++;
   }

   assert( fileOffset == fileData.fileSize );

   if ( !Platform_WriteFileData( &fileData ) )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: could not write to file: %s\n\n", fileData.filePath );
      printf( msg );
      exit( 1 );
   }

   snprintf( msg, STRING_SIZE_DEFAULT, "done!\n\nGame data written to %s\n", fileData.filePath );
   printf( msg );

   Platform_ClearFileData( &fileData );
}

internal uint32_t GetGameDataFileSize( GameResources_t* resources )
{
   uint32_t fileSize, i;
   FontData_t* fontData;
   BitmapData_t* bitmapData;

   fileSize = 4;                    // chunk count
   fileSize += ( 4 * NUM_CHUNKS );  // chunk offsets
   fileSize += ( 8 * NUM_CHUNKS );  // chunk IDs and entry counts

   fontData = resources->fontDatas;

   for ( i = 0; i < resources->numFonts; i++ )
   {
      fileSize += 8;    // entry ID and size
      fileSize += GetFontDataMemSize( fontData );
      fontData++;
   }

   bitmapData = resources->bitmapDatas;

   for ( i = 0; i < resources->numBitmaps; i++ )
   {
      fileSize += 8;    // entry ID and size
      fileSize += bitmapData->size;
      bitmapData++;
   }

   return fileSize;
}

internal uint32_t GetFontDataMemSize( FontData_t* fontData )
{
   uint32_t size, i, j;
   FontGlyphCollection_t* collection;
   FontGlyph_t* glyph;

   // codepoint offset, number of glyph collections, and number of glyphs, each 4 bytes
   size = 12;
   collection = fontData->font.glyphCollections;

   for ( i = 0; i < fontData->font.numGlyphCollections; i++ )
   {
      // height, baseline, and lineGap, each 4 bytes
      size += 12;
      glyph = collection->glyphs;

      for ( j = 0; j < fontData->font.numGlyphs; j++ )
      {
         // left bearing (4 bytes), baseline offset (4 bytes), advance (4 bytes),
         // pixel buffer dimensions (8 bytes), and pixel buffer size
         size += 20;
         size += ( glyph->pixelBuffer.dimensions.x * glyph->pixelBuffer.dimensions.y * 4 );
         glyph++;
      }

      collection++;
   }

   return size;
}

internal void WriteFontData( FontData_t* fontData, FileData_t* fileData, uint32_t fileOffset )
{
   uint32_t i, j, k;
   uint32_t* filePos32;
   FontGlyphCollection_t* collection;
   FontGlyph_t* glyph;

   filePos32 = (uint32_t*)( (uint8_t*)( fileData->contents ) + fileOffset );
   filePos32[0] = fontData->font.codepointOffset;
   filePos32[1] = fontData->font.numGlyphCollections;
   filePos32[2] = fontData->font.numGlyphs;
   filePos32 += 3;
   collection = fontData->font.glyphCollections;

   for ( i = 0; i < fontData->font.numGlyphCollections; i++ )
   {
      ( (float*)filePos32 )[0] = collection->height;
      ( (float*)filePos32 )[1] = collection->baseline;
      ( (float*)filePos32 )[2] = collection->lineGap;
      filePos32 += 3;
      glyph = collection->glyphs;

      for ( j = 0; j < fontData->font.numGlyphs; j++ )
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
}

internal uint32_t GetResourceIDForFileName( ResourceIDMapping_t mappings[], uint32_t numMappings, const char* fileName )
{
   uint32_t i;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < numMappings; i++ )
   {
      if ( strcmp( fileName, mappings[i].fileName ) == 0 )
      {
         return mappings[i].ID;
      }
   }

   snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: no resource ID found for file: %s\n\n", fileName );
   printf( msg );
   exit( 1 );
}
