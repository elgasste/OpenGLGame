#if !defined( ENUMS_H )
#define ENUMS_H

typedef enum
{
   AssetsFileLumpID_Bitmaps = 0,
   AssetsFileLumpID_Fonts,
   AssetsFileLumpID_Text,
   AssetsFileLumpID_SpriteBases,
   AssetsFileLumpID_Sprites,

   AssetsFileLumpID_Count
}
AssetsFileLumpID_t;

typedef enum
{
   ImageID_Text = 0,
   ImageID_MessageBoxBorders,
   ImageID_Player,

   ImageID_Count
}
ImageID_t;

typedef enum
{
   SpriteBaseID_Player = 0,

   SpriteBaseID_Count
}
SpriteBaseID_t;

typedef enum
{
   SpriteID_Player = 0,

   SpriteID_Count
}
SpriteID_t;

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
   MenuBorderSide_Left = 0,
   MenuBorderSide_Top,
   MenuBorderSide_Right,
   MenuBorderSide_Bottom,
   MenuBorderSide_TopLeft,
   MenuBorderSide_TopRight,
   MenuBorderSide_BottomRight,
   MenuBorderSide_BottomLeft
}
MenuBorderSide_t;

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
