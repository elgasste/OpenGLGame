#if !defined( VECTOR_H )
#define VECTOR_H

#include "common.h"

#define VECTORI_RESET( v ) v.x = 0; v.y = 0
#define VECTORF_RESET( v ) v.x = 0.0f; v.y = 0.0f

typedef struct Vector2f_t
{
   float x;
   float y;
}
Vector2f_t;

typedef struct Vector2i8_t
{
   int8_t x;
   int8_t y;
}
Vector2i8_t;

typedef struct Vector2ui8_t
{
   uint8_t x;
   uint8_t y;
}
Vector2ui8_t;

typedef struct Vector2i16_t
{
   int16_t x;
   int16_t y;
}
Vector2i16_t;

typedef struct Vector2ui16_t
{
   uint16_t x;
   uint16_t y;
}
Vector2ui16_t;

typedef struct Vector2i32_t
{
   int32_t x;
   int32_t y;
}
Vector2i32_t;

typedef struct Vector2ui32_t
{
   uint32_t x;
   uint32_t y;
}
Vector2ui32_t;

#endif
