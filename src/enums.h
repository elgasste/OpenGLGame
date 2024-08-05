#if !defined( ENUMS_H )
#define ENUMS_H

typedef enum
{
   AssetsFileChunkID_Bitmaps = 0,
   AssetsFileChunkID_Fonts,
   AssetsFileChunkID_SpriteBases,
   AssetsFileChunkID_TileSets,
   AssetsFileChunkID_TileMaps,

   AssetsFileChunkID_Count
}
AssetsFileChunkID_t;

typedef enum
{
   ImageID_Background = 0,
   ImageID_Star,
   ImageID_WorldTileSet,

   ImageID_Count
}
ImageID_t;

typedef enum
{
   SpriteBaseID_Star,

   SpriteBaseID_Count
}
SpriteBaseID_t;

typedef enum
{
   FontID_Consolas = 0,
   FontID_Papyrus,

   FontID_Count
}
FontID_t;

typedef enum
{
   FontJustify_Left = 0,
   FontJustify_Center,
   FontJustify_Right
}
FontJustify_t;

typedef enum
{
   TileSetID_World = 0,

   TileSetID_Count
}
TileSetID_t;

typedef enum
{
   TileMapID_World = 0,

   TileMapID_Count
}
TileMapID_t;

typedef enum
{
   MenuID_Playing = 0,

   MenuID_Count
}
MenuID_t;

typedef enum
{
   MenuItemID_KeepPlaying = 0,
   MenuItemID_Quit,

   MenuItemID_Count
}
MenuItemID_t;

typedef enum
{
   GameState_Playing = 0,
   GameState_Menu,

   GameState_Count
}
GameState_t;

#endif
