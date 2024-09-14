#if !defined( ENTITY_H )
#define ENTITY_H

#include "common.h"
#include "vector.h"

typedef struct Entity_t
{
   Vector2f_t position;
   Vector2f_t velocity;
   Vector2f_t maxVelocity;
   Bool_t isAirborne;
   float gravityDeceleration;
}
Entity_t;

#endif
