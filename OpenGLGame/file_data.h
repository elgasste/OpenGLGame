#if !defined( FILE_DATA_H )
#define FILE_DATA_H

typedef struct
{
   char filePath[STRING_SIZE_DEFAULT];
   uint32_t size;
   void* contents;
}
cFileData_t;

#endif
