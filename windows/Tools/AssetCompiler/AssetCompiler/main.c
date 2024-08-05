#include <assert.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "main.h"
#include "sprite.h"
#include "platform.h"

// TODO: this is all manually-entered for now, but ultimately it would be
// nice to have some kind of external tool that can compile this data.
global FontIDToGlyphHeightsMapping_t g_glyphHeightsMap[] = {
   { (uint32_t)FontID_Consolas, { 12.0f, 24.0f }, 2 },
   { (uint32_t)FontID_Papyrus, { 48.0f }, 1 }
};
global AssetFileToIDMapping_t g_fontIDMap[] = {
   { "consolas.ttf", (uint32_t)FontID_Consolas },
   { "papyrus.ttf", (uint32_t)FontID_Papyrus }
};
global AssetFileToIDMapping_t g_bitmapIDMap[] = {
   { "background.bmp", (uint32_t)ImageID_Background },
   { "star_sprite.bmp", (uint32_t)ImageID_Star },
   { "world_tileset.bmp", (uint32_t)ImageID_WorldTileSet }
};
global SpriteBaseData_t g_spriteBaseDatas[] = {
   { (uint32_t)SpriteBaseID_Star, (uint32_t)ImageID_Star, { 6, 6 } }
};
global TileSetData_t g_tileSetDatas[] = {
   { (uint32_t)TileSetID_World, (uint32_t)ImageID_WorldTileSet, { 32, 32 } }
};

internal FileInfo_t* GetFiles( const char* dir, const char* filter, uint32_t* numFiles );
internal FontData_t* LoadFontsFromDir( const char* dir, const char* filter, uint32_t* numFonts );
internal void LoadFontFromFile( FontData_t* fontData, FileInfo_t* fileInfo );
internal BitmapData_t* LoadBitmapsFromDir( const char* dir, const char* filter, uint32_t* numBitmaps );
internal void WriteAssetsFile( GameAssets_t* assets, const char* dir );
internal uint32_t GetAssetsFileSize( GameAssets_t* assets );
internal uint32_t GetFontDataMemSize( FontData_t* fontData );
internal void WriteFontData( FontData_t* fontData, FileData_t* fileData, uint32_t fileOffset );
internal uint32_t GetAssetIDForFileName( AssetFileToIDMapping_t mappings[], uint32_t numMappings, const char* fileName );

int main( int argc, char** argv )
{
   GameAssets_t assets;
   char assetsDir[STRING_SIZE_DEFAULT];
   char fontsDir[STRING_SIZE_DEFAULT];
   char bitmapsDir[STRING_SIZE_DEFAULT];
   char fontFilter[STRING_SIZE_DEFAULT];
   char bitmapFilter[STRING_SIZE_DEFAULT];

   if ( argc < 2 )
   {
      printf( "ERROR: not enough arguments, arg1 should be the assets directory.\n\n" );
      exit( 1 );
   }

   strcpy_s( assetsDir, STRING_SIZE_DEFAULT, argv[1] );
   if ( assetsDir[strlen( assetsDir ) - 1] != '\\' )
   {
      strcat_s( assetsDir, STRING_SIZE_DEFAULT, "\\" );
   }

   strcpy_s( fontsDir, STRING_SIZE_DEFAULT, assetsDir );
   strcat_s( fontsDir, STRING_SIZE_DEFAULT, "fonts\\" );
   strcpy_s( bitmapsDir, STRING_SIZE_DEFAULT, assetsDir );
   strcat_s( bitmapsDir, STRING_SIZE_DEFAULT, "bitmaps\\" );

   strcpy_s( fontFilter, STRING_SIZE_DEFAULT, fontsDir );
   strcat_s( fontFilter, STRING_SIZE_DEFAULT, "*.ttf" );
   strcpy_s( bitmapFilter, STRING_SIZE_DEFAULT, bitmapsDir );
   strcat_s( bitmapFilter, STRING_SIZE_DEFAULT, "*.bmp" );

   assets.fontDatas = LoadFontsFromDir( fontsDir, fontFilter, &( assets.numFonts ) );
   assets.bitmapDatas = LoadBitmapsFromDir( bitmapsDir, bitmapFilter, &( assets.numBitmaps ) );

   printf( "\n" );

   WriteAssetsFile( &assets, assetsDir );

   printf( "\n" );

   return 0;
}

void* Platform_MemAlloc( uint64_t size )
{
   return malloc( size );
}

void Platform_MemFree( void* memory, uint64_t size )
{
   UNUSED_PARAM( size );
   free( memory );
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

internal FileInfo_t* GetFiles( const char* dir, const char* filter, uint32_t* numFiles )
{
   HANDLE hFile;
   WIN32_FIND_DATAA findData;
   uint32_t i = 0;
   FileInfo_t* files;
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

   files = (FileInfo_t*)Platform_MemAlloc( sizeof( FileInfo_t ) * ( *numFiles ) );
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
   FileInfo_t* files;
   FontData_t* fontDatas;
   FontData_t* fontData;

   files = GetFiles( dir, filter, numFonts );
   fontDatas = (FontData_t*)Platform_MemAlloc( sizeof( FontData_t ) * ( *numFonts ) );
   fontData = fontDatas;

   for ( i = 0; i < ( *numFonts ); i++ )
   {
      strcpy_s( fontData->fileName, STRING_SIZE_DEFAULT, files[i].name );
      LoadFontFromFile( fontData, &( files[i] ) );
      fontData++;
   }

   return fontDatas;
}

internal void LoadFontFromFile( FontData_t* fontData, FileInfo_t* fileInfo )
{
   FileData_t fileData;
   uint8_t* filePos;
   stbtt_fontinfo fontInfo;
   int32_t width, height, xOffset, pitch, x, y, codepoint, glyphIndex, baseline, lineGap, baselineOffset, advance, leftBearing;
   uint8_t *monoCodepointMemory, *codepointMemory, *source, *destRow;
   uint32_t* dest;
   uint8_t alpha;
   uint32_t i, fontID;
   float scale;
   FontGlyphCollection_t* collection;
   char msg[STRING_SIZE_DEFAULT];

   fontData->font.codepointOffset = STARTCODEPOINT;
   fontData->font.numGlyphs = (uint32_t)( ( ENDCODEPOINT - STARTCODEPOINT ) + 1 );

   snprintf( msg, STRING_SIZE_DEFAULT, "Reading font data from %s...", fileInfo->path );
   printf( msg );

   if ( !Platform_ReadFileData( fileInfo->path, &fileData ) )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: could not open file: %s\n\n", fileInfo->path );
      printf( msg );
      exit( 1 );
   }

   filePos = (uint8_t*)( fileData.contents );

   if ( !stbtt_InitFont( &fontInfo, filePos, stbtt_GetFontOffsetForIndex( filePos, 0 ) ) )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: could not load font data: %s\n\n", fileInfo->path );
      printf( msg );
      exit( 1 );
   }

   fontID = GetAssetIDForFileName( g_fontIDMap, sizeof( g_fontIDMap ) / sizeof( AssetFileToIDMapping_t ), fileInfo->name );
   fontData->font.numGlyphCollections = g_glyphHeightsMap[fontID].numGlyphHeights;
   fontData->font.glyphCollections = (FontGlyphCollection_t*)Platform_MemAlloc( sizeof( FontGlyphCollection_t ) * fontData->font.numGlyphCollections );
   collection = fontData->font.glyphCollections;

   for ( i = 0; i < fontData->font.numGlyphCollections; i++ )
   {
      collection->height = g_glyphHeightsMap[fontID].glyphHeights[i];
      scale = stbtt_ScaleForPixelHeight( &fontInfo, (float)( collection->height ) );
      stbtt_GetFontVMetrics( &fontInfo, 0, &baseline, &lineGap);
      collection->baseline = -baseline * scale;
      collection->lineGap = lineGap * scale;
      collection->glyphs = (FontGlyph_t*)Platform_MemAlloc( sizeof( FontGlyph_t ) * fontData->font.numGlyphs );

      for ( codepoint = STARTCODEPOINT; codepoint <= ENDCODEPOINT; codepoint++ )
      {
         monoCodepointMemory = stbtt_GetCodepointBitmap( &fontInfo, 0, scale, codepoint, &width, &height, &xOffset, 0 );
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
         collection->glyphs[glyphIndex].baselineOffset = baselineOffset * scale;

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
   FileInfo_t* files;
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

internal void WriteAssetsFile( GameAssets_t* assets, const char* dir )
{
   uint32_t i, j, fileOffset, dataSize, numSpriteBases, numTileSets;
   uint8_t* filePos8;
   uint32_t* filePos32;
   FileData_t fileData;
   FontData_t* fontData;
   BitmapData_t* bitmapData;
   SpriteBaseData_t* spriteBaseData;
   TileSetData_t* tileSetData;
   char msg[STRING_SIZE_DEFAULT];

   fileData.fileSize = GetAssetsFileSize( assets );
   snprintf( msg, STRING_SIZE_DEFAULT, "Game data file size: %u bytes\n", fileData.fileSize );
   printf( msg );

   strcpy_s( fileData.filePath, STRING_SIZE_DEFAULT, dir );
   strcat_s( fileData.filePath, STRING_SIZE_DEFAULT, ASSETS_FILENAME );
   fileData.contents = Platform_MemAlloc( fileData.fileSize );

   printf( "Writing game data to destination..." );

   // chunk count and first chunk offset
   filePos32 = (uint32_t*)( fileData.contents );
   filePos32[0] = NUM_CHUNKS;
   fileOffset = 4 + ( NUM_CHUNKS * 4 );
   filePos32[1] = fileOffset;

   // fonts chunk
   filePos32 = (uint32_t*)( (uint8_t*)fileData.contents + fileOffset );
   filePos32[0] = (uint32_t)AssetsFileChunkID_Fonts;
   filePos32[1] = assets->numFonts;
   filePos32 += 2;
   fileOffset += 8;

   fontData = assets->fontDatas;

   for ( i = 0; i < assets->numFonts; i++ )
   {
      dataSize = GetFontDataMemSize( fontData );
      filePos32[0] = GetAssetIDForFileName( g_fontIDMap, sizeof( g_fontIDMap ) / sizeof( AssetFileToIDMapping_t ), fontData->fileName );
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
   filePos32[0] = (uint32_t)AssetsFileChunkID_Bitmaps;
   filePos32[1] = assets->numBitmaps;
   filePos32 += 2;
   fileOffset += 8;

   bitmapData = assets->bitmapDatas;

   for ( i = 0; i < assets->numBitmaps; i++ )
   {
      filePos32[0] = GetAssetIDForFileName( g_bitmapIDMap, sizeof( g_bitmapIDMap ) / sizeof( AssetFileToIDMapping_t ), bitmapData->fileName );
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

   // sprite bases chunk
   numSpriteBases = (uint32_t)( sizeof( g_spriteBaseDatas ) / sizeof( SpriteBaseData_t ) );
   ( (uint32_t*)( fileData.contents ) )[3] = fileOffset;  // chunk offset
   filePos32[0] = (uint32_t)AssetsFileChunkID_SpriteBases;
   filePos32[1] = numSpriteBases;
   filePos32 += 2;
   fileOffset += 8;

   spriteBaseData = g_spriteBaseDatas;

   for ( i = 0; i < numSpriteBases; i++ )
   {
      filePos32[0] = spriteBaseData->baseID;
      filePos32[1] = 12;
      filePos32[2] = spriteBaseData->imageID;
      filePos32[3] = spriteBaseData->frameDimensions.x;
      filePos32[4] = spriteBaseData->frameDimensions.y;

      filePos32 += 5;
      fileOffset += 20;
      spriteBaseData++;
   }

   // tilesets chunk
   numTileSets = (uint32_t)( sizeof( g_tileSetDatas ) / sizeof( TileSetData_t ) );
   ( (uint32_t*)( fileData.contents ) )[4] = fileOffset;  // chunk offset
   filePos32[0] = (uint32_t)AssetsFileChunkID_TileSets;
   filePos32[1] = numTileSets;
   filePos32 += 2;
   fileOffset += 8;

   tileSetData = g_tileSetDatas;

   for ( i = 0; i < numTileSets; i++ )
   {
      filePos32[0] = tileSetData->tileSetID;
      filePos32[1] = 12;
      filePos32[2] = tileSetData->imageID;
      filePos32[3] = tileSetData->tileDimensions.x;
      filePos32[4] = tileSetData->tileDimensions.y;

      filePos32 += 5;
      fileOffset += 20;
      tileSetData++;
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

internal uint32_t GetAssetsFileSize( GameAssets_t* assets )
{
   uint32_t fileSize, i;
   FontData_t* fontData;
   BitmapData_t* bitmapData;

   fileSize = 4;                    // chunk count
   fileSize += ( 4 * NUM_CHUNKS );  // chunk offsets
   fileSize += ( 8 * NUM_CHUNKS );  // chunk IDs and entry counts

   // fonts chunk
   fontData = assets->fontDatas;

   for ( i = 0; i < assets->numFonts; i++ )
   {
      fileSize += 8;    // entry ID and size
      fileSize += GetFontDataMemSize( fontData );
      fontData++;
   }

   // bitmaps chunk
   bitmapData = assets->bitmapDatas;

   for ( i = 0; i < assets->numBitmaps; i++ )
   {
      fileSize += 8;    // entry ID and size
      fileSize += bitmapData->size;
      bitmapData++;
   }

   // sprite bases chunk
   for ( i = 0; i < (uint32_t)( sizeof( g_spriteBaseDatas ) / sizeof( SpriteBaseData_t ) ); i++ )
   {
      fileSize += 8;    // entry ID and size
      fileSize += 12;   // image ID and frame dimensions
   }

   // tilesets chunk
   for ( i = 0; i < (uint32_t)( sizeof( g_tileSetDatas ) / sizeof( TileSetData_t ) ); i++ )
   {
      fileSize += 8;    // entry ID and size
      fileSize += 12;   // image ID and tile dimensions
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

internal uint32_t GetAssetIDForFileName( AssetFileToIDMapping_t mappings[], uint32_t numMappings, const char* fileName )
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

   snprintf( msg, STRING_SIZE_DEFAULT, "ERROR: no asset ID found for file: %s\n\n", fileName );
   printf( msg );
   exit( 1 );
}
