#if !defined( ENUMS_H )
#define ENUMS_H

typedef enum
{
   GameDataFileChunkID_Bitmaps = 0,
   GameDataFileChunkID_Fonts,

   GameDataFileChunkID_Count
}
GameDataFileChunkID_t;

typedef enum
{
   ImageID_Background = 0,
   ImageID_Star,

   ImageID_Count
}
ImageID_t;

typedef enum
{
   FontID_Consolas = 0,
   FontID_Papyrus,

   FontID_Count
}
FontID_t;

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