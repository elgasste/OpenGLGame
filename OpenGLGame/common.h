#if !defined( COMMON_H )
#define COMMON_H

#include <stdint.h>
#include <stdio.h>

#include "strings.h"

// not really necessary, but makes things easier to distinguish
#define internal static
#define global static
#define local_persist static

// use this to suppress unused parameter warnings
#define UNUSED_PARAM( x ) (void)x

typedef int32_t cBool_t;
#define cTrue 1
#define cFalse 0
#define TOGGLE_BOOL( x ) x = ( x ) ? cFalse : cTrue

#define STRING_SIZE_DEFAULT      1024

#define GRAPHICS_BPP             32
#define GRAPHICS_ALPHABITS       8
// TODO: set this back to 1280x720 when bitmap testing is done
#define SCREEN_BUFFER_WIDTH      1920
#define SCREEN_BUFFER_HEIGHT     1080

#define GAME_FPS                 60

#endif
