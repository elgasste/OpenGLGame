#include "game.h"
#include "game_data_file.h"

typedef struct
{
   uint32_t* offsets;
   uint32_t numOffsets;
}
ChunkIDOffsetArray_t;

internal Bool_t Game_ReadGameDataFile( GameData_t* gameData );
internal void Game_ClearChunkIDOffsets( ChunkIDOffsetArray_t* offsets, uint32_t numOffsets );
internal Bool_t Game_ReadBitmapsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );
internal Bool_t Game_ReadFontsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );
internal Bool_t Game_ReadSpriteBasesChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );
internal void Game_LoadMenus( GameData_t* gameData );

Bool_t Game_LoadData( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;
   SpriteBase_t* starSpriteBase;

   if ( !Game_ReadGameDataFile( gameData ) )
   {
      return False;
   }

   Game_LoadMenus( gameData );

   starSpriteBase = &( gameData->renderData.spriteBases[SpriteBaseID_Star] );

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );

      if ( !Sprite_LoadFromBase( &( star->sprite ), starSpriteBase, 0.1f ) )
      {
         return False;
      }

      star->isResting = True;
   }

   return True;
}

internal Bool_t Game_ReadGameDataFile( GameData_t* gameData )
{
   uint32_t i, j, numOffsets, chunkID;
   GameDataFile_t dataFile = { 0 };
   GameDataFileChunk_t* chunk;
   ChunkIDOffsetArray_t chunkIDOffsets[GameDataFileChunkID_Count];
   char appDirectory[STRING_SIZE_DEFAULT];
   char dataFilePath[STRING_SIZE_DEFAULT];
   char msg[STRING_SIZE_DEFAULT];
   uint32_t chunkIDOrder[] = {
      (uint32_t)GameDataFileChunkID_Fonts,
      (uint32_t)GameDataFileChunkID_Bitmaps,
      (uint32_t)GameDataFileChunkID_SpriteBases
   };
   Bool_t ( *chunkLoaders[] )( GameData_t*, GameDataFileChunk_t* ) = {
      Game_ReadFontsChunk,
      Game_ReadBitmapsChunk,
      Game_ReadSpriteBasesChunk
   };

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( dataFilePath, STRING_SIZE_DEFAULT, "%s%s", appDirectory, GAME_DATA_FILENAME );

   if ( !GameDataFile_Load( &dataFile, dataFilePath ) )
   {
      return False;
   }

   for ( i = 0; i < (uint32_t)GameDataFileChunkID_Count; i++ )
   {
      chunkIDOffsets[i].offsets = 0;
      chunkIDOffsets[i].numOffsets = 0;
   }

   chunk = dataFile.chunks;

   for ( i = 0; i < dataFile.numChunks; i++ )
   {
      if ( (GameDataFileChunkID_t)( chunk->ID ) < GameDataFileChunkID_Count )
      {
         numOffsets = chunkIDOffsets[chunk->ID].numOffsets;
         chunkIDOffsets[chunk->ID].offsets = (uint32_t*)Platform_ReAlloc( chunkIDOffsets[chunk->ID].offsets,
                                                                          4 * numOffsets,
                                                                          4 * ( numOffsets + 1 ) );
         chunkIDOffsets[chunk->ID].offsets[numOffsets] = i;
         chunkIDOffsets[chunk->ID].numOffsets++;
      }
      else
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNCHUNKID, chunk->ID );
         Platform_Log( msg );
      }

      chunk++;
   }

   for ( i = 0; i < (uint32_t)GameDataFileChunkID_Count; i++ )
   {
      chunkID = chunkIDOrder[i];

      if ( chunkIDOffsets[chunkID].numOffsets == 0 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFERR_NOCHUNKSFOUNDFORID, i );
         Platform_Log( msg );
         Game_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)GameDataFileChunkID_Count );
         GameDataFile_ClearData( &dataFile );
         return False;
      }

      for ( j = 0; j < chunkIDOffsets[chunkID].numOffsets; j++ )
      {
         chunk = dataFile.chunks + chunkIDOffsets[chunkID].offsets[j];
         if ( !chunkLoaders[i]( gameData, chunk ) )
         {
            Game_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)GameDataFileChunkID_Count );
            GameDataFile_ClearData( &dataFile );
            return False;
         }
      }
   }

   Game_ClearChunkIDOffsets( chunkIDOffsets, (uint32_t)GameDataFileChunkID_Count );
   GameDataFile_ClearData( &dataFile );
   return True;
}

internal void Game_ClearChunkIDOffsets( ChunkIDOffsetArray_t* offsets, uint32_t numOffsets )
{
   uint32_t i;

   for ( i = 0; i < numOffsets; i++ )
   {
      Platform_Free( offsets[i].offsets, 4 * offsets[i].numOffsets );
   }
}

internal Bool_t Game_ReadBitmapsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk )
{
   uint32_t i;
   ImageID_t imageID;
   GameDataFileChunkEntry_t* entry = chunk->entries;
   Image_t* image;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      imageID = (ImageID_t)entry->ID;

      if ( imageID < ImageID_Count )
      {
         image = &( gameData->renderData.images[entry->ID] );
         Image_ClearData( image );

         if ( !Image_LoadFromBitmapMemory( image, entry->memory, entry->size, entry->ID ) )
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

internal Bool_t Game_ReadFontsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk )
{
   uint32_t i;
   FontID_t fontID;
   GameDataFileChunkEntry_t* entry = chunk->entries;
   Font_t* font;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
   {
      fontID = (FontID_t)entry->ID;

      if ( fontID < FontID_Count )
      {
         font = &( gameData->renderData.fonts[entry->ID] );
         Font_ClearData( font );

         if ( !Font_LoadFromMemory( font, entry->memory, entry->size, entry->ID ) )
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

internal Bool_t Game_ReadSpriteBasesChunk( GameData_t* gameData, GameDataFileChunk_t* chunk )
{
   uint32_t i, imageID;
   SpriteBaseID_t baseID;
   GameDataFileChunkEntry_t* entry = chunk->entries;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < chunk->numEntries; i++ )
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
                                          baseID,
                                          &( gameData->renderData.images[imageID] ),
                                          (ImageID_t)imageID,
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

internal void Game_LoadMenus( GameData_t* gameData )
{
   Menu_t* playingMenu = &( gameData->menus[MenuID_Playing] );

   playingMenu->numItems = 2;
   playingMenu->items = (MenuItem_t*)Platform_MAlloc( sizeof( MenuItem_t ) * playingMenu->numItems );
   playingMenu->items[0].ID = MenuItemID_KeepPlaying;
   snprintf( playingMenu->items[0].text, STRING_SIZE_DEFAULT, STR_MENU_KEEPPLAYING );
   playingMenu->items[1].ID = MenuItemID_Quit;
   snprintf( playingMenu->items[1].text, STRING_SIZE_DEFAULT, STR_MENU_QUIT );
   playingMenu->renderData.font = &( gameData->renderData.fonts[FontID_Consolas] );
   playingMenu->renderData.caratCodepoint = (uint32_t)( '>' );
   playingMenu->renderData.position.x = 200.0f;
   playingMenu->renderData.position.y = 500.0f;
   playingMenu->renderData.textHeight = 24.0f;
   playingMenu->renderData.lineGap = playingMenu->renderData.font->curGlyphCollection->lineGap;
   playingMenu->renderData.caratOffset = -30.0f;
   playingMenu->renderData.textColor = 0xFFFF8800;
   playingMenu->renderData.caratColor = 0xFFFF8800;
   playingMenu->caratFadeSeconds = 0.25f;
}
