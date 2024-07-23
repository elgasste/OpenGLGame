#if !defined( ENUMS_H )
#define ENUMS_H

typedef enum
{
   TextureID_Background = 0,
   TextureID_Star,

   TextureID_Count
}
TextureID_t;

typedef enum
{
   FontID_Consolas = 0,
   FontID_Papyrus,

   FontID_Count
}
FontID_t;

typedef enum
{
   GameState_Playing = 0,
   GameState_Menu,

   GameState_Count
}
GameState_t;

#endif
