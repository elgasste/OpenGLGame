#include "png.h"

cBool_t cPng_LoadImageData( cFileData_t* fileData )
{
   uint8_t* fileContents;

   if ( fileData->size < 8 )
   {
      // TODO: logging
      return cFalse;
   }

   fileContents = (uint8_t*)( fileData->contents );

   if ( ( (uint64_t*)fileContents )[0] != PNG_SIGNATURE )
   {
      // TODO: logging
      return cFalse;
   }

   // TODO: the file signature is legit, now read the chunks

   return cTrue;
}
