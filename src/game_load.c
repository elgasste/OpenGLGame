#include "game.h"
#include "game_data_file.h"

internal Bool_t Game_LoadGameDataFile( GameData_t* gameData );
internal Bool_t Game_ReadBitmapsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );
internal Bool_t Game_ReadFontsChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );
internal Bool_t Game_ReadSpriteBasesChunk( GameData_t* gameData, GameDataFileChunk_t* chunk );
internal void Game_LoadMenus( GameData_t* gameData );

Bool_t Game_LoadData( GameData_t* gameData )
{
   uint32_t i;
   Star_t* star;

   if ( !Game_LoadGameDataFile( gameData ) )
   {
      return False;
   }

   Game_LoadMenus( gameData );

   // TODO: remove this after updating the game data file
   SpriteBase_t* starSpriteBase = &( gameData->renderData.spriteBases[SpriteBaseID_Star] );
   uint32_t* mem = (uint32_t*)Platform_MAlloc( 16 );
   mem[1] = 6;
   mem[2] = 6;
   ( (float*)mem )[3] = 0.1f;
   Sprite_LoadBaseFromMemory( starSpriteBase,
                              SpriteBaseID_Star,
                              &( gameData->renderData.images[ImageID_Star] ),
                              ImageID_Star,
                              (uint8_t*)mem,
                              16 );
   Platform_Free( mem, 16 );

   for ( i = 0; i < STAR_COUNT; i++ )
   {
      star = &( gameData->stars[i] );

      if ( !Sprite_LoadFromBase( &( star->sprite ), starSpriteBase ) )
      {
         return False;
      }

      star->isResting = True;
   }

   return True;
}

internal Bool_t Game_LoadGameDataFile( GameData_t* gameData )
{
   uint32_t i;
   GameDataFile_t dataFile = { 0 };
   GameDataFileChunk_t* chunk;
   char appDirectory[STRING_SIZE_DEFAULT];
   char dataFilePath[STRING_SIZE_DEFAULT];
   char msg[STRING_SIZE_DEFAULT];

   if ( !Platform_GetAppDirectory( appDirectory, STRING_SIZE_DEFAULT ) )
   {
      return False;
   }

   snprintf( dataFilePath, STRING_SIZE_DEFAULT, "%s%s", appDirectory, GAME_DATA_FILENAME );

   if ( !GameDataFile_Load( &dataFile, dataFilePath ) )
   {
      return False;
   }

   chunk = dataFile.chunks;

   // TODO: update this to read in all the chunks to arrays first, then
   // cycle through them in the correct order (to make sure images are
   // loaded before sprite bases, etc).
   for ( i = 0; i < dataFile.numChunks; i++ )
   {
      switch ( ( GameDataFileChunkID_t )( chunk->ID ) )
      {
         case GameDataFileChunkID_Bitmaps:
            if ( !Game_ReadBitmapsChunk( gameData, chunk ) )
            {
               GameDataFile_ClearData( &dataFile );
               return False;
            }
            break;
         case GameDataFileChunkID_Fonts:
            if ( !Game_ReadFontsChunk( gameData, chunk ) )
            {
               GameDataFile_ClearData( &dataFile );
               return False;
            }
            break;
         case GameDataFileChunkID_SpriteBases:
            if ( !Game_ReadSpriteBasesChunk( gameData, chunk ) )
            {
               GameDataFile_ClearData( &dataFile );
               return False;
            }
         default:
            snprintf( msg, STRING_SIZE_DEFAULT, STR_GDFWARN_UNKNOWNCHUNKID, chunk->ID );
            Platform_Log( msg );
            break;
      }

      chunk++;
   }

   GameDataFile_ClearData( &dataFile );
   return True;
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
