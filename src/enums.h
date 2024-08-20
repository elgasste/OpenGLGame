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
   ImageID_Background = 0,
   ImageID_StarSprite,
   ImageID_PlayerSpriteIdleLeft,
   ImageID_PlayerSpriteIdleRight,
   ImageID_PlayerSpriteRunLeft,
   ImageID_PlayerSpriteRunRight,

   ImageID_Count
}
ImageID_t;

typedef enum
{
   SpriteBaseID_Star,
   SpriteBaseID_PlayerIdleLeft,
   SpriteBaseID_PlayerIdleRight,
   SpriteBaseID_PlayerRunLeft,
   SpriteBaseID_PlayerRunRight,

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

typedef enum
{
   ButtonCode_Left = 0,
   ButtonCode_Up,
   ButtonCode_Right,
   ButtonCode_Down,

   ButtonCode_Enter,
   ButtonCode_Escape,

   ButtonCode_F8,

   ButtonCode_Count
}
ButtonCode_t;

typedef enum
{
   MouseButtonCode_Left = 0,
   MouseButtonCode_Right
}
MouseButtonCode_t;

#endif
