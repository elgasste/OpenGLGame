#if !defined( ENUMS_H )
#define ENUMS_H

typedef enum
{
   AssetsFileChunkID_Bitmaps = 0,
   AssetsFileChunkID_Fonts,
   AssetsFileChunkID_SpriteBases,
   AssetsFileChunkID_Sprites,

   AssetsFileChunkID_Count
}
AssetsFileChunkID_t;

typedef enum
{
   ImageID_Background = 0,
   ImageID_PlayerSpriteIdleLeft,
   ImageID_PlayerSpriteIdleRight,
   ImageID_PlayerSpriteRunLeft,
   ImageID_PlayerSpriteRunRight,
   ImageID_PlayerSpriteJumpLeft,
   ImageID_PlayerSpriteJumpRight,
   ImageID_PlayerSpriteAttackLeft1,
   ImageID_PlayerSpriteAttackLeft2,
   ImageID_PlayerSpriteAttackLeft3,
   ImageID_PlayerSpriteAttackRight1,
   ImageID_PlayerSpriteAttackRight2,

   ImageID_Count
}
ImageID_t;

typedef enum
{
   SpriteBaseID_PlayerIdleLeft = 0,
   SpriteBaseID_PlayerIdleRight,
   SpriteBaseID_PlayerRunLeft,
   SpriteBaseID_PlayerRunRight,
   SpriteBaseID_PlayerJumpLeft,
   SpriteBaseID_PlayerJumpRight,
   SpriteBaseID_PlayerAttackLeft1,
   SpriteBaseID_PlayerAttackLeft2,
   SpriteBaseID_PlayerAttackLeft3,
   SpriteBaseID_PlayerAttackRight1,
   SpriteBaseID_PlayerAttackRight2,

   SpriteBaseID_Count
}
SpriteBaseID_t;

typedef enum
{
   SpriteID_PlayerIdleLeft,
   SpriteID_PlayerIdleRight,
   SpriteID_PlayerRunLeft,
   SpriteID_PlayerRunRight,
   SpriteID_PlayerJumpLeft,
   SpriteID_PlayerJumpRight,
   SpriteID_PlayerAttackLeft1,
   SpriteID_PlayerAttackLeft2,
   SpriteID_PlayerAttackLeft3,
   SpriteID_PlayerAttackRight1,
   SpriteID_PlayerAttackRight2,

   SpriteID_Count
}
SpriteID_t;

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

   ButtonCode_Space,
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

typedef enum
{
   PlayerDirection_Left = 0,
   PlayerDirection_Right
}
PlayerDirection_t;

#endif
