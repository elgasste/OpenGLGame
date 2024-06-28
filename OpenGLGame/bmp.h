#if !defined( BMP_H )
#define BMP_H

#include "common.h"

#define BMP_HEADER_SIZE    14

typedef struct
{
   uint32_t imageOffset;
}
cBmpData_t;

cBool_t cBmp_LoadFromFile( cBmpData_t* bmpData, const char* filePath );

#endif
