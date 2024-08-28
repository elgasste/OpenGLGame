#if !defined( VECTOR_H )
#define VECTOR_H

#include "common.h"

#define VECTOR2I_RESET( v ) v.x = 0; v.y = 0
#define VECTOR2F_RESET( v ) v.x = 0.0f; v.y = 0.0f

typedef struct Vector2f_t
{
   float x;
   float y;
}
Vector2f_t;

typedef struct Vector4f_t
{
   float x;
   float y;
   float w;
   float h;
}
Vector4f_t;

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
