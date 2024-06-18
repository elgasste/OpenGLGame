#if !defined( SCREEN_BUFFER_H )
#define SCREEN_BUFFER_H

#include "common.h"

typedef struct
{
   void* memory;
   int32_t pitch;
}
cScreenBuffer_t;

#endif
