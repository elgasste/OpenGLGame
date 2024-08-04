#if !defined( ENUMS_H )
#define ENUMS_H

typedef enum
{
   AssetsFileChunkID_Bitmaps = 0,
   AssetsFileChunkID_Fonts,
   AssetsFileChunkID_SpriteBases,

   AssetsFileChunkID_Count
}
AssetsFileChunkID_t;

typedef enum
{
   ImageID_Tileset = 0,
   ImageID_Star,

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
