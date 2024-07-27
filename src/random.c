#include "random.h"

uint32_t Random_UInt32( uint32_t min, uint32_t max )
{
   uint32_t number;
   rand_s( &number );
   return ( number % ( ( max + 1 ) - min ) ) + min;
}

uint8_t Random_Percent()
{
   uint32_t number;
   rand_s( &number );
   return (uint8_t)number % 101;
}

Bool_t Random_Bool()
{
   uint32_t number;
   rand_s( &number );
   return (Bool_t)number % 2;
}
