#if !defined( BMP_H )
#define BMP_H

#include "common.h"

typedef struct
{
   int32_t width;
   int32_t height;
}
cBmpData_t;

cBool_t cBmp_LoadFromFile( cBmpData_t* bmpData, const char* filePath );

#endif
