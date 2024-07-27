#if !defined( COMMON_H )
#define COMMON_H

#define _CRT_RAND_S

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "enums.h"
#include "strings.h"

// not really necessary, but makes things easier to distinguish
#define internal static
#define global static
#define local_persist static

// use this to suppress unused parameter warnings
#define UNUSED_PARAM( x ) (void)x

typedef int32_t Bool_t;
#define True 1
#define False 0
#define TOGGLE_BOOL( x ) x = ( x ) ? False : True

#define GAME_DATA_FILENAME       "gamedata.dat"

#define MAX_THREADQUEUE_SIZE     256

#define STRING_SIZE_DEFAULT      1024

#define SCREEN_WIDTH             1280
#define SCREEN_HEIGHT            720

#define GAME_FPS                 60

#define FONT_DEFAULT_HEIGHT      24

#endif
