#if !defined( VECTOR_H )
#define VECTOR_H

#include "common.h"

typedef struct
{
   float x;
   float y;
}
Vector2f_t;

typedef struct
{
   int8_t x;
   int8_t y;
}
Vector2i8_t;

typedef struct
{
   uint8_t x;
   uint8_t y;
}
Vector2ui8_t;

typedef struct
{
   int16_t x;
   int16_t y;
}
Vector2i16_t;

typedef struct
{
   uint16_t x;
   uint16_t y;
}
Vector2ui16_t;

typedef struct
{
   int32_t x;
   int32_t y;
}
Vector2i32_t;

typedef struct
{
   uint32_t x;
   uint32_t y;
}
Vector2ui32_t;

#endif
