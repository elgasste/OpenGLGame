#include <time.h>

#include "random.h"

void Random_Seed()
{
   srand( (uint32_t)time( 0 ) );
}

uint32_t Random_UInt32( uint32_t min, uint32_t max )
{
   // min and max inclusive
   return ( (uint32_t)rand() % ( ( max + 1 ) - min ) ) + min;
}

uint8_t Random_Percent()
{
   return (uint8_t)rand() % 101;
}

Bool_t Random_Bool()
{
   return (Bool_t)rand() % 2;
}
