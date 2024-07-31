#if !defined( RECT_H )
#define RECT_H

#include "common.h"
#include "vector.h"

typedef struct RectF_t
{
   Vector2f_t pos;
   Vector2f_t size;
}
RectF_t;

inline Bool_t Rect_PointInRectF( RectF_t* rect, float x, float y )
{
   return ( x >= rect->pos.x ) && ( x < ( rect->pos.x + rect->size.x ) ) &&
          ( y >= rect->pos.y ) && ( y < ( rect->pos.y + rect->size.y ) );
}

#endif
