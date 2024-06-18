#if !defined( COMMON_H )
#define COMMON_H

#include <stdint.h>

#include "strings.h"

#define internal static
#define global static
#define local_persist static

// use this to suppress unused parameter warnings
#define UNUSED_PARAM(x) (void)x

typedef int32_t cBool_t;
#define cTrue 1
#define cFalse 0
#define TOGGLE_BOOL( x ) x = ( x ) ? cFalse : cTrue

#define GRAPHICS_BPP             32
#define GRAPHICS_ALPHABITS       8
#define SCREEN_BUFFER_WIDTH      1280
#define SCREEN_BUFFER_HEIGHT     720

#define GAME_FPS                 60

#endif
