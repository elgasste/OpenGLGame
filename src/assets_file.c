#include "assets_file.h"
#include "game.h"
#include "platform.h"

#define ERROR_RETURN_FALSE() \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_GDFERR_FILECORRUPT, filePath ); \
   Platform_Log( errorMsg ); \
   AssetsFile_ClearData( assetsFileData ); \
   return False;

internal Bool_t AssetsFile_LoadFile( AssetsFileData_t* assetsFileData, const char* filePath );
internal void AssetsFile_ClearData( AssetsFileData_t* assetsFileData );
internal Bool_t AssetsFile_ReadLump( AssetsFileLump_t* lump, FileData_t* fileData, uint32_t lumpOffset );
internal Bool_t AssetsFile_Interpret( AssetsFileData_t* assetsFileData, GameData_t* gameData );
internal void AssetsFile_ClearOffsetTable( AssetsFileOffsetTable_t* offsetTable, uint32_t numOffsets );
internal Bool_t AssetsFile_InterpretBitmapsLump( GameData_t* gameData, AssetsFileLump_t* lump );
internal Bool_t AssetsFile_InterpretFontsLump( GameData_t* gameData, AssetsFileLump_t* lump );
internal Bool_t AssetsFile_InterpretTextLump( GameData_t* gameData, AssetsFileLump_t* lump );
internal Bool_t AssetsFile_InterpretSpriteBasesLump( GameData_t* gameData, AssetsFileLump_t* lump );
internal Bool_t AssetsFile_InterpretSpritesLump( GameData_t* gameData, AssetsFileLump_t* lump );

Bool_t AssetsFile_Load( GameData_t* gameData )
{
   AssetsFileData_t assetsFileData = { 0 };
   Bool_t success;
   char appDirectory[STRING_SIZE_DEFAULT];
   char dataFilePath[STRING_SIZE_DEFAULT];

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( dataFilePath, STRING_SIZE_DEFAULT, "%s%s", appDirectory, ASSETS_FILENAME );

   if ( !AssetsFile_LoadFile( &assetsFileData, dataFilePath ) )
   {
      return False;
   }

   success = AssetsFile_Interpret( &assetsFileData, gameData );
   AssetsFile_ClearData( &assetsFileData );
   return success;
}

internal Bool_t AssetsFile_LoadFile( AssetsFileData_t* assetsFileData, const char* filePath )
{
   uint32_t bytesRead, i, lumpOffset;
   FileData_t fileData;
   uint32_t* filePos32;
   AssetsFileLump_t* lump;
   char errorMsg[STRING_SIZE_DEFAULT];

   assetsFileData->numLumps = 0;
   assetsFileData->lumps = 0;

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      return False;
   }
   else if ( fileData.fileSize < 4 ) // 4 bytes for the number of lumps
   {
      ERROR_RETURN_FALSE();
   }

   // first 4 bytes are the number of lumps
   filePos32 = (uint32_t*)fileData.contents;
   assetsFileData->numLumps = filePos32[0];
   filePos32++;
   bytesRead = 4;

   // make sure there's enough room to read all the lump offsets
   if ( fileData.fileSize < ( bytesRead + ( assetsFileData->numLumps * 4 ) ) )
   {
      ERROR_RETURN_FALSE();
   }

   assetsFileData->lumps = (AssetsFileLump_t*)Platform_MAlloc( sizeof( AssetsFileLump_t ) * assetsFileData->numLumps );
   lump = assetsFileData->lumps;

   for ( i = 0; i < assetsFileData->numLumps; i++ )
   {
      lump->entries = 0;
      lump->numEntries = 0;
      lump++;
   }

   lump = assetsFileData->lumps;

   for ( i = 0; i < assetsFileData->numLumps; i++ )
   {
      lumpOffset = filePos32[0];
      filePos32++;
      bytesRead += 4;

      // make sure there's enough room to read at least the lump ID and entry count
      if ( fileData.fileSize < ( lumpOffset + 8 ) )
      {
         ERROR_RETURN_FALSE();
      }

      if ( !AssetsFile_ReadLump( lump, &fileData, lumpOffset ) )
      {
         ERROR_RETURN_FALSE();
      }

      lump++;
   }

   return True;
}

internal void AssetsFile_ClearData( AssetsFileData_t* assetsFileData )
{
   uint32_t i, j;
   AssetsFileLump_t* lump = assetsFileData->lumps;
   AssetsFileEntry_t* entry;

   if ( lump )
   {
      for ( i = 0; i < assetsFileData->numLumps; i++ )
      {
         entry = lump->entries;

         if ( entry )
         {
            for ( j = 0; j < lump->numEntries; j++ )
            {
               if ( entry->memory )
               {
                  Platform_Free( entry->memory, entry->size );
               }

               entry++;
            }
         }

         Platform_Free( lump->entries, sizeof( AssetsFileEntry_t ) * lump->numEntries );
         lump++;
      }

      Platform_Free( assetsFileData->lumps, sizeof( AssetsFileLump_t ) * assetsFileData->numLumps );
      assetsFileData->numLumps = 0;
      assetsFileData->lumps = 0;
   }
}

internal Bool_t AssetsFile_ReadLump( AssetsFileLump_t* lump, FileData_t* fileData, uint32_t lumpOffset )
{
   uint32_t i, j, bytesRead;
   uint8_t* filePos = (uint8_t*)( fileData->contents ) + lumpOffset;
   AssetsFileEntry_t* entry;

   lump->ID = ( (uint32_t*)filePos )[0];
   lump->numEntries = ( (uint32_t*)filePos )[1];
   filePos += 8;
   bytesRead = 8;

   // make sure there's enough room to read at least the ID and size of each entry
   if ( fileData->fileSize < ( lumpOffset + bytesRead + ( lump->numEntries * 8 ) ) )
   {
      return False;
   }

   lump->entries = (AssetsFileEntry_t*)Platform_MAlloc( sizeof( AssetsFileEntry_t ) * lump->numEntries );
   entry = lump->entries;

   for ( i = 0; i < lump->numEntries; i++ )
   {
      entry->ID = 0;
      entry->size = 0;
      entry->memory = 0;
      entry++;
   }

   entry = lump->entries;

   for ( i = 0; i < lump->numEntries; i++ )
   {
      entry->ID = ( ( uint32_t*)filePos )[0];
      entry->size = ( ( uint32_t*)filePos )[1];
      filePos += 8;
      bytesRead += 8;

      if ( fileData->fileSize < ( lumpOffset + bytesRead + entry->size ) )
      {
         return False;
      }

      entry->memory = (uint8_t*)Platform_CAlloc( 1, entry->size );

      for ( j = 0; j < entry->size; j++ )
      {
         entry->memory[j] = filePos[j];
      }

      filePos += entry->size;
      bytesRead += entry->size;
      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_Interpret( AssetsFileData_t* assetsFileData, GameData_t* gameData )
{
   uint32_t i, j, numOffsets, lumpID;
   AssetsFileLump_t* lump;
   AssetsFileOffsetTable_t lumpIDOffsets[AssetsFileLumpID_Count];
   char msg[STRING_SIZE_DEFAULT];
   uint32_t lumpIDOrder[] = {
      (uint32_t)AssetsFileLumpID_Fonts,
      (uint32_t)AssetsFileLumpID_Bitmaps,
      (uint32_t)AssetsFileLumpID_Text,
      (uint32_t)AssetsFileLumpID_SpriteBases,
      (uint32_t)AssetsFileLumpID_Sprites
   };
   Bool_t ( *lumpLoaders[] )( GameData_t*, AssetsFileLump_t* ) = {
      AssetsFile_InterpretFontsLump,
      AssetsFile_InterpretBitmapsLump,
      AssetsFile_InterpretTextLump,
      AssetsFile_InterpretSpriteBasesLump,
      AssetsFile_InterpretSpritesLump
   };

   for ( i = 0; i < (uint32_t)AssetsFileLumpID_Count; i++ )
   {
      lumpIDOffsets[i].numOffsets = 0;
      lumpIDOffsets[i].offsets = 0;
   }

   lump = assetsFileData->lumps;

   for ( i = 0; i < assetsFileData->numLumps; i++ )
   {
      if ( (AssetsFileLumpID_t)( lump->ID ) < AssetsFileLumpID_Count )
      {
         numOffsets = lumpIDOffsets[lump->ID].numOffsets;
         lumpIDOffsets[lump->ID].offsets = (uint32_t*)Platform_ReAlloc( lumpIDOffsets[lump->ID].offsets,
                                                                          4 * numOffsets,
                                                                          4 * ( numOffsets + 1 ) );
         lumpIDOffsets[lump->ID].offsets[numOffsets] = i;
         lumpIDOffsets[lump->ID].numOffsets++;
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNLUMPID, lump->ID );
         Platform_Log( msg );
      }

      lump++;
   }

   for ( i = 0; i < (uint32_t)AssetsFileLumpID_Count; i++ )
   {
      lumpID = lumpIDOrder[i];

      if ( lumpIDOffsets[lumpID].numOffsets == 0 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_NOLUMPSFOUNDFORID, i );
         Platform_Log( msg );
         AssetsFile_ClearOffsetTable( lumpIDOffsets, (uint32_t)AssetsFileLumpID_Count );
         return False;
      }

      for ( j = 0; j < lumpIDOffsets[lumpID].numOffsets; j++ )
      {
         lump = assetsFileData->lumps + lumpIDOffsets[lumpID].offsets[j];
         if ( !lumpLoaders[i]( gameData, lump ) )
         {
            AssetsFile_ClearOffsetTable( lumpIDOffsets, (uint32_t)AssetsFileLumpID_Count );
            return False;
         }
      }
   }

   AssetsFile_ClearOffsetTable( lumpIDOffsets, (uint32_t)AssetsFileLumpID_Count );
   return True;
}

internal void AssetsFile_ClearOffsetTable( AssetsFileOffsetTable_t* offsetTable, uint32_t numOffsets )
{
   uint32_t i;

   for ( i = 0; i < numOffsets; i++ )
   {
      Platform_Free( offsetTable[i].offsets, 4 * offsetTable[i].numOffsets );
      offsetTable[i].offsets = 0;
      offsetTable[i].numOffsets = 0;
   }
}

internal Bool_t AssetsFile_InterpretBitmapsLump( GameData_t* gameData, AssetsFileLump_t* lump )
{
   uint32_t i;
   ImageID_t imageID;
   AssetsFileEntry_t* entry = lump->entries;
   Image_t* image;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < lump->numEntries; i++ )
   {
      imageID = (ImageID_t)entry->ID;

      if ( imageID < ImageID_Count )
      {
         image = &( gameData->renderData.images[entry->ID] );
         Image_ClearData( image );

         if ( !Image_LoadFromBitmapMemory( image, entry->memory, entry->size ) )
         {
            return False;
         }
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNBITMAPENTRYID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_InterpretFontsLump( GameData_t* gameData, AssetsFileLump_t* lump )
{
   uint32_t i;
   FontID_t fontID;
   AssetsFileEntry_t* entry = lump->entries;
   Font_t* font;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < lump->numEntries; i++ )
   {
      fontID = (FontID_t)entry->ID;

      if ( fontID < FontID_Count )
      {
         font = &( gameData->renderData.fonts[entry->ID] );
         Font_ClearData( font );

         if ( !Font_LoadFromMemory( font, entry->memory, entry->size ) )
         {
            return False;
         }
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNFONTENTRYID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_InterpretTextLump( GameData_t* gameData, AssetsFileLump_t* lump )
{
   uint32_t i;
   AssetsFileEntry_t* entry = lump->entries;
   uint32_t* memPos32;
   TextMap_t* textMap = &( gameData->renderData.textMap );
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < lump->numEntries; i++ )
   {
      memPos32 = (uint32_t*)entry->memory;

      if ( memPos32[0] >= (uint32_t)ImageID_Count )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_TEXTLUMPCORRUPT, entry->ID );
         Platform_Log( msg );
         return False;
      }

      textMap->image = &( gameData->renderData.images[memPos32[0]] );
      textMap->charSize.x = memPos32[1];
      textMap->charSize.y = memPos32[2];
      textMap->startChar = (char)entry->memory[12];
      textMap->endChar = (char)entry->memory[13];

      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_InterpretSpriteBasesLump( GameData_t* gameData, AssetsFileLump_t* lump )
{
   uint32_t i, imageID;
   SpriteBaseID_t baseID;
   AssetsFileEntry_t* entry = lump->entries;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < lump->numEntries; i++ )
   {
      baseID = (SpriteBaseID_t)entry->ID;

      if ( entry->size < 4 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_SPRITEBASECORRUPT, entry->ID );
         Platform_Log( msg );
         return False;
      }

      imageID = ( (uint32_t*)( entry->memory ) )[0];

      if ( imageID >= ImageID_Count )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_SPRITEBASEIMAGENOTFOUND, imageID );
         Platform_Log( msg );
         return False;
      }

      if ( baseID < SpriteBaseID_Count )
      {
         if ( !Sprite_LoadBaseFromMemory( &( gameData->renderData.spriteBases[entry->ID] ),
                                          &( gameData->renderData.images[imageID] ),
                                          entry->memory,
                                          entry->size ) )
         {
            return False;
         }
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNSPRITEBASEID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}

internal Bool_t AssetsFile_InterpretSpritesLump( GameData_t* gameData, AssetsFileLump_t* lump )
{
   uint32_t i, spriteBaseID;
   SpriteID_t spriteID;
   AssetsFileEntry_t* entry = lump->entries;
   Sprite_t* sprite;
   float* memF;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < lump->numEntries; i++ )
   {
      spriteID = (SpriteID_t)entry->ID;

      if ( entry->size < 4 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_SPRITECORRUPT, entry->ID );
         Platform_Log( msg );
         return False;
      }

      spriteBaseID = ( (uint32_t*)( entry->memory ) )[0];

      if ( spriteBaseID >= SpriteBaseID_Count )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_SPRITEBASENOTFOUND, spriteBaseID );
         Platform_Log( msg );
         return False;
      }

      if ( spriteID < SpriteID_Count )
      {
         sprite = &( gameData->renderData.sprites[entry->ID] );
         memF = (float*)( entry->memory );
         Sprite_LoadFromBase( sprite, &( gameData->renderData.spriteBases[spriteBaseID] ), memF[5] );
         sprite->hitBox.x = memF[1];
         sprite->hitBox.y = memF[2];
         sprite->hitBox.w = memF[3];
         sprite->hitBox.h = memF[4];
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNSPRITEID, entry->ID );
         Platform_Log( msg );
      }

      entry++;
   }

   return True;
}
