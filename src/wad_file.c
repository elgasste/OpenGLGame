#include "wad_file.h"
#include "platform.h"

internal Bool_t WadFile_ReadHeader( WadFile_t* wadFile, FileData_t* fileData );
internal Bool_t WadFile_ReadLumpTable( WadFile_t* wadFile, FileData_t* fileData );
internal Bool_t WadFile_ReadLumps( WadFile_t* wadFile, FileData_t* fileData );
internal Bool_t WadFile_ReadPalettesLump( WadFile_t* wadFile, WadFileLumpInfo_t* lumpInfo, FileData_t* fileData );
internal Bool_t WadFile_ReadTextureInfoLump( WadFile_t* wadFile, WadFileLumpInfo_t* lumpInfo, FileData_t* fileData );
internal Bool_t WadFile_ReadPatchNamesLump( WadFile_t* wadFile, WadFileLumpInfo_t* lumpInfo, FileData_t* fileData );
internal Bool_t WadFile_ReadPatchLumps( WadFile_t* wadFile, FileData_t* fileData, int32_t* lumpIndex );
internal Bool_t WadFile_ReadPatchLumpTable( WadFile_t* wadFile, FileData_t* fileData, int32_t* lumpIndex, const char* endName, int32_t* patchesRead );
internal Bool_t WadFile_ReadFlatLumps( WadFile_t* wadFile, FileData_t* fileData, int32_t* lumpIndex );
internal Bool_t WadFile_CountFlatLumps( WadFile_t* wadFile, int32_t* lumpIndex, const char* endName, int32_t* count );
internal Bool_t WadFile_ReadFlatLumpTable( WadFile_t* wadFile, FileData_t* fileData, int32_t* lumpIndex, int32_t flatIndex, const char* endName );

Bool_t WadFile_Load( WadFile_t* wadFile, const char* filePath )
{
   int32_t i, j;
   FileData_t fileData;
   Bool_t success = False;
   char msg[STRING_SIZE_DEFAULT];

   wadFile->header.lumpCount = 0;
   wadFile->lumpInfoTable = 0;
   wadFile->paletteTable = 0;
   wadFile->textureTable = 0;
   wadFile->textureCount = 0;
   wadFile->patchCount = 0;
   wadFile->patchTable = 0;
   wadFile->flatCount = 0;
   wadFile->flatTable = 0;

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      snprintf( msg, STRING_SIZE_DEFAULT, STR_WADERR_FILEOPEN, filePath );
      Platform_Log( msg );
      return False;
   }

   success =
      WadFile_ReadHeader( wadFile, &fileData ) &&
      WadFile_ReadLumpTable( wadFile, &fileData ) &&
      WadFile_ReadLumps( wadFile, &fileData );

   for ( i = 0; i < wadFile->textureCount; i++ )
   {
      for ( j = 0; j < wadFile->textureTable[i].header.patchInfoCount; j++ )
      {
         if ( wadFile->textureTable[i].patchInfoTable[j].patchID < 0 ||
              wadFile->textureTable[i].patchInfoTable[j].patchID >= wadFile->patchCount )
         {
            snprintf( msg, STRING_SIZE_DEFAULT, STR_WADERR_TEXTURETABLEINDEXRANGE, i, j );
            Platform_Log( msg );
            success = False;
         }
      }
   }

   if ( !success )
   {
      WadFile_Clear( wadFile );
   }

   Platform_ClearFileData( &fileData );
   return success;
}

void WadFile_Clear( WadFile_t* wadFile )
{
   int32_t i;

   if ( wadFile->paletteTable )
   {
      Platform_Free( wadFile->paletteTable, sizeof( WadFilePalette_t ) * WADFILE_PALETTE_COUNT );
      wadFile->paletteTable = 0;
   }

   if ( wadFile->lumpInfoTable )
   {
      Platform_Free( wadFile->lumpInfoTable, sizeof( WadFileLumpInfo_t ) * wadFile->header.lumpCount );
      wadFile->lumpInfoTable = 0;
   }

   if ( wadFile->textureTable )
   {
      for ( i = 0; i < wadFile->textureCount; i++ )
      {
         if ( wadFile->textureTable[i].patchInfoTable )
         {
            Platform_Free( wadFile->textureTable[i].patchInfoTable,
                           sizeof( WadFileTexturePatchInfo_t ) * wadFile->textureTable[i].header.patchInfoCount );
         }
      }

      Platform_Free( wadFile->textureTable, sizeof( WadFileTexture_t ) * wadFile->textureCount );
      wadFile->textureTable = 0;
   }

   if ( wadFile->patchTable )
   {
      for ( i = 0; i < wadFile->patchCount; i++ )
      {
         if ( wadFile->patchTable[i].memory )
         {
            Platform_Free( wadFile->patchTable[i].memory, wadFile->patchTable[i].size );
         }
      }

      Platform_Free( wadFile->patchTable, sizeof( WadFilePatch_t ) * wadFile->patchCount );
      wadFile->patchTable = 0;
   }

   if ( wadFile->flatTable )
   {
      for ( i = 0; i < wadFile->flatCount; i++ )
      {
         if ( wadFile->flatTable[i].memory )
         {
            Platform_Free( wadFile->flatTable[i].memory, wadFile->flatTable[i].size );
         }
      }

      Platform_Free( wadFile->flatTable, sizeof( WadFileFlat_t ) * wadFile->flatCount );
      wadFile->flatTable = 0;
   }

   wadFile->header.lumpCount = 0;
   wadFile->textureCount = 0;
   wadFile->patchCount = 0;
   wadFile->flatCount = 0;
}

internal Bool_t WadFile_ReadHeader( WadFile_t* wadFile, FileData_t* fileData )
{
   int32_t i;
   int32_t lumpTableEnd;

   if ( fileData->fileSize < sizeof( WadFileHeader_t ) )
   {
      Platform_Log( STR_WADERR_FILESIZEHEADER );
      return False;
   }

   Platform_MemCpy( &( wadFile->header ), fileData->contents, sizeof( WadFileHeader_t ) );

   for ( i = 0; i < 4; i++ )
   {
      wadFile->header.wadType[i] = (char)toupper( wadFile->header.wadType[i] );
   }

   if ( strncmp( wadFile->header.wadType, "IWAD", 4 ) != 0 &&
        strncmp( wadFile->header.wadType, "PWAD", 4 ) != 0 )
   {
      Platform_Log( STR_WADERR_WADTYPE );
      return False;
   }

   lumpTableEnd = wadFile->header.lumpTableOffset + ( wadFile->header.lumpCount * sizeof( WadFileLumpInfo_t ) );

   if ( wadFile->header.lumpCount <= 0 )
   {
      Platform_Log( STR_WADERR_LUMPCOUNTNEGATIVE );
      return False;
   }
   else if ( wadFile->header.lumpTableOffset < 0 )
   {
      Platform_Log( STR_WADERR_LUMPTABLEOFFSET );
      return False;
   }
   else if ( lumpTableEnd > ( int32_t )( fileData->fileSize ) )
   {
      Platform_Log( STR_WADERR_LUMPTABLESIZE );
      return False;
   }

   return True;
}

internal Bool_t WadFile_ReadLumpTable( WadFile_t* wadFile, FileData_t* fileData )
{
   int32_t i, j;
   char msg[STRING_SIZE_DEFAULT];
   WadFileLumpInfo_t* lumpInfo;
   uint8_t* memPos8 = (uint8_t*)( fileData->contents ) + wadFile->header.lumpTableOffset;
   Bool_t stillGood = True;

   wadFile->lumpInfoTable = (WadFileLumpInfo_t*)Platform_MAlloc( (uint64_t)( wadFile->header.lumpCount ) * sizeof( WadFileLumpInfo_t ) );
   lumpInfo = wadFile->lumpInfoTable;

   for ( i = 0; i < wadFile->header.lumpCount; i++ )
   {
      Platform_MemCpy( lumpInfo, memPos8, sizeof( WadFileLumpInfo_t ) );
      memPos8 += sizeof( WadFileLumpInfo_t );

      for ( j = 0; j < 8; j++ )
      {
         lumpInfo->name[j] = (char)toupper( lumpInfo->name[j] );
      }

      if ( lumpInfo->filePos < 0 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADERR_LUMPPOSITIONNEGATIVE, i );
         Platform_Log( msg );
         stillGood = False;
      }
      else if ( lumpInfo->size < 0 )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADERR_LUMPNUMBERNEGATIVE, i );
         Platform_Log( msg );
         stillGood = False;
      }
      else if ( ( lumpInfo->filePos + lumpInfo->size ) > ( int32_t )( fileData->fileSize ) )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADERR_LUMPNUMBERLENGTH, i );
         Platform_Log( msg );
         stillGood = False;
      }

      if ( !stillGood )
      {
         return False;
      }

      lumpInfo++;
   }

   return True;
}

internal Bool_t WadFile_ReadLumps( WadFile_t* wadFile, FileData_t* fileData )
{
   int32_t i;
   WadFileLumpInfo_t* lumpInfo;
   Bool_t stillGood = True;
   char lumpName[9];
   char msg[STRING_SIZE_DEFAULT];

   for ( i = 0; i < wadFile->header.lumpCount; i++ )
   {
      lumpInfo = &( wadFile->lumpInfoTable[i] );

      if ( lumpInfo->filePos + lumpInfo->size > (int32_t)( fileData->fileSize ) )
      {
         strncpy_s( lumpName, 9, lumpInfo->name, 8 );
         lumpName[8] = 0;
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADERR_LUMPNUMBERSIZE, i + 1, lumpName );
         Platform_Log( msg );
         return False;
      }

      if ( strncmp( lumpInfo->name, "PLAYPAL", 7 ) == 0 )
      {
         stillGood = WadFile_ReadPalettesLump( wadFile, lumpInfo, fileData );
      }
      else if ( strncmp( lumpInfo->name, "TEXTURE1", 8 ) == 0 || strncmp( lumpInfo->name, "TEXTURE2", 8 ) == 0 )
      {
         stillGood = WadFile_ReadTextureInfoLump( wadFile, lumpInfo, fileData );
      }
      else if ( strncmp( lumpInfo->name, "PNAMES", 6 ) == 0 )
      {
         stillGood = WadFile_ReadPatchNamesLump( wadFile, lumpInfo, fileData );
      }
      else if ( strncmp( lumpInfo->name, "P_START", 7 ) == 0 )
      {
         i++;
         stillGood = WadFile_ReadPatchLumps( wadFile, fileData, &i );
      }
      else if ( strncmp( lumpInfo->name, "F_START", 7 ) == 0 )
      {
         i++;
         stillGood = WadFile_ReadFlatLumps( wadFile, fileData, &i );
      }

      if ( !stillGood )
      {
         return False;
      }
   }

   return True;
}

internal Bool_t WadFile_ReadPalettesLump( WadFile_t* wadFile, WadFileLumpInfo_t* lumpInfo, FileData_t* fileData )
{
   int32_t i;
   uint8_t* memPos;
   WadFilePalette_t* palette;

   if ( wadFile->paletteTable != 0 )
   {
      Platform_Log( STR_WADERR_MULTIPLEPALETTES );
      return False;
   }
   else if ( lumpInfo->size != ( ( WADFILE_PALETTE_COLORCOUNT * sizeof( WadFilePaletteColor_t ) ) * WADFILE_PALETTE_COUNT ) )
   {
      Platform_Log( STR_WADERR_PALETTESIZE );
      return False;
   }

   wadFile->paletteTable = (WadFilePalette_t*)Platform_MAlloc( sizeof( WadFilePalette_t ) * WADFILE_PALETTE_COUNT );
   palette = wadFile->paletteTable;
   memPos = (uint8_t*)( fileData->contents ) + lumpInfo->filePos;

   for ( i = 0; i < WADFILE_PALETTE_COUNT; i++ )
   {
      Platform_MemCpy( palette->colorTable, memPos, sizeof( WadFilePaletteColor_t ) * WADFILE_PALETTE_COLORCOUNT );
      memPos += sizeof( WadFilePaletteColor_t ) * WADFILE_PALETTE_COLORCOUNT;
      palette++;
   }

   return True;
}

internal Bool_t WadFile_ReadTextureInfoLump( WadFile_t* wadFile, WadFileLumpInfo_t* lumpInfo, FileData_t* fileData )
{
   int32_t i, j, textureCount, prevTextureCount, textureOffset;
   uint8_t* memPos8;
   uint32_t* memPos32;
   WadFileTexture_t* texture;
   WadFileTexturePatchInfo_t* patch;

   if ( lumpInfo->size < 4 )
   {
      Platform_Log( STR_WADERR_TEXTURELUMPSIZEHEADER );
      return False;
   }

   memPos32 = (uint32_t*)( (uint8_t*)( fileData->contents ) + lumpInfo->filePos );
   textureCount = memPos32[0];

   if ( textureCount <= 0 )
   {
      Platform_Log( STR_WADERR_TEXTURECOUNTZERO );
      return False;
   }
   // lump should at least be big enough for the lump header and all the texture headers
   else if ( lumpInfo->size < ( 4 + ( textureCount * 4 ) + ( textureCount * sizeof( WadFileTextureHeader_t ) ) ) )
   {
      Platform_Log( STR_WADERR_TEXTURELUMPSIZE );
      return False;
   }

   prevTextureCount = wadFile->textureCount;
   wadFile->textureTable = (WadFileTexture_t*)Platform_ReAlloc( wadFile->textureTable,
                                                                sizeof( WadFileTexture_t ) * prevTextureCount,
                                                                sizeof( WadFileTexture_t ) * ( wadFile->textureCount + textureCount ) );
   wadFile->textureCount += textureCount;

   for ( i = prevTextureCount; i < wadFile->textureCount; i++ )
   {
      wadFile->textureTable[i].patchInfoTable = 0;
   }

   texture = wadFile->textureTable + prevTextureCount;

   for ( i = 0; i < textureCount; i++ )
   {
      memPos32++;
      textureOffset = *memPos32;

      if ( ( textureOffset + sizeof( WadFileTextureHeader_t ) ) > lumpInfo->size )
      {
         Platform_Log( STR_WADERR_TEXTUREOFFSET );
         return False;
      }

      memPos8 = (uint8_t*)( fileData->contents ) + lumpInfo->filePos + textureOffset;
      Platform_MemCpy( &( texture->header ), memPos8, sizeof( WadFileTextureHeader_t ) );
      memPos8 += sizeof( WadFileTextureHeader_t );

      for ( j = 0; j < 8; j++ )
      {
         texture->header.name[j] = (char)toupper( texture->header.name[j] );
      }

      if ( texture->header.patchInfoCount <= 0 )
      {
         Platform_Log( STR_WADERR_TEXTUREPATCHCOUNTZERO );
         return False;
      }

      if ( lumpInfo->filePos + sizeof( WadFileTextureHeader_t ) + ( texture->header.patchInfoCount * sizeof( WadFileTexturePatchInfo_t ) ) > fileData->fileSize )
      {
         Platform_Log( STR_WADERR_TEXTUREPATCHOVERFLOW );
         return False;
      }

      texture->patchInfoTable = (WadFileTexturePatchInfo_t*)Platform_MAlloc( sizeof( WadFileTexturePatchInfo_t ) * texture->header.patchInfoCount );
      patch = texture->patchInfoTable;

      for ( j = 0; j < texture->header.patchInfoCount; j++ )
      {
         Platform_MemCpy( patch, memPos8, sizeof( WadFileTexturePatchInfo_t ) );
         memPos8 += sizeof( WadFileTexturePatchInfo_t );
         patch++;
      }

      texture++;
   }

   return True;
}

internal Bool_t WadFile_ReadPatchNamesLump( WadFile_t* wadFile, WadFileLumpInfo_t* lumpInfo, FileData_t* fileData )
{
   int32_t i, j;
   uint8_t* memPos8;
   uint32_t* memPos32 = (uint32_t*)( (uint8_t*)( fileData->contents ) + lumpInfo->filePos );
   WadFilePatch_t* patch;

   if ( wadFile->patchCount != 0 )
   {
      Platform_Log( STR_WADERR_MULTIPLETEXTUREPATCHNAMES );
      return False;
   }

   wadFile->patchCount = *memPos32;

   if ( wadFile->patchCount <= 0 )
   {
      wadFile->patchCount = 0;
      Platform_Log( STR_WADERR_TEXTUREPATCHNAMECOUNTZERO );
      return False;
   }

   if ( lumpInfo->filePos + 4 + ( wadFile->patchCount * 8 ) > (int32_t)( fileData->fileSize ) )
   {
      Platform_Log( STR_WADERR_TEXTUREPATCHNAMEOVERFLOW );
      return False;
   }

   wadFile->patchTable = (WadFilePatch_t*)Platform_MAlloc( sizeof( WadFilePatch_t ) * wadFile->patchCount );
   memPos8 = (uint8_t*)( memPos32 + 1 );
   patch = wadFile->patchTable;

   for ( i = 0; i < wadFile->patchCount; i++ )
   {
      Platform_MemCpy( patch->name, memPos8, 8 );

      for ( j = 0; j < 8; j++ )
      {
         patch->name[j] = (char)toupper( patch->name[j] );
      }

      patch->memory = 0;
      patch->size = 0;
      memPos8 += 8;
      patch++;
   }

   return True;
}

internal Bool_t WadFile_ReadPatchLumps( WadFile_t* wadFile, FileData_t* fileData, int32_t* lumpIndex )
{
   int32_t i, patchesRead = 0;
   WadFileLumpInfo_t* lumpInfo;
   char lumpName[9];
   Bool_t stillGood = True;
   Bool_t cleanExit = False;
   char msg[STRING_SIZE_DEFAULT];

   if ( wadFile->patchCount <= 0 )
   {
      Platform_Log( STR_WADERR_PREMATUREPATCHTABLE );
      return False;
   }

   for ( i = 0; i < wadFile->patchCount; i++ )
   {
      wadFile->patchTable[i].memory = 0;
      wadFile->patchTable[i].size = 0;
   }

   while ( *lumpIndex < wadFile->header.lumpCount )
   {
      lumpInfo = &( wadFile->lumpInfoTable[*lumpIndex] );

      if ( strncmp( lumpInfo->name, "P1_START", 8 ) == 0 )  // shareware
      {
         *lumpIndex += 1;
         stillGood = WadFile_ReadPatchLumpTable( wadFile, fileData, lumpIndex, "P1_END", &patchesRead );
         *lumpIndex -= 1;
      }
      else if ( strncmp( lumpInfo->name, "P2_START", 8 ) == 0 )   // registered
      {
         *lumpIndex += 1;
         stillGood = WadFile_ReadPatchLumpTable( wadFile, fileData, lumpIndex, "P2_END", &patchesRead );
         *lumpIndex -= 1;
      }
      else if ( strncmp( lumpInfo->name, "P3_START", 8 ) == 0 )   // Doom 2
      {
         *lumpIndex += 1;
         stillGood = WadFile_ReadPatchLumpTable( wadFile, fileData, lumpIndex, "P3_END", &patchesRead );
         *lumpIndex -= 1;
      }
      else if ( strncmp( lumpInfo->name, "P_END", 8 ) == 0 )
      {
         cleanExit = True;
         break;
      }
      else
      {
         strncpy_s( lumpName, 9, lumpInfo->name, 8 );
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADWARN_INVALIDTEXTUREPATCHLUMP, lumpName );
         Platform_Log( msg );
      }

      if ( !stillGood )
      {
         return False;
      }

      *lumpIndex += 1;
   }

   if ( !cleanExit )
   {
      Platform_Log( STR_WADERR_TEXTUREPATCHTABLESHORT );
      return False;
   }
   else if ( patchesRead == 0 )
   {
      Platform_Log( STR_WADERR_TEXTUREPATCHCOUNTZERO );
      return False;
   }

   for ( i = 0; i < wadFile->patchCount; i++ )
   {
      if ( !wadFile->patchTable[i].memory )
      {
         strncpy_s( lumpName, 9, wadFile->patchTable[i].name, 8 );
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADWARN_DEFINEDTEXTUREPATCHNOTLOADED, lumpName );
         Platform_Log( msg );
      }
   }

   return True;
}

internal Bool_t WadFile_ReadPatchLumpTable( WadFile_t* wadFile, FileData_t* fileData, int32_t* lumpIndex, const char* endName, int32_t* patchesRead )
{
   int32_t i, patchIndex;
   WadFileLumpInfo_t* lumpInfo;
   char lumpName[9];
   char msg[STRING_SIZE_DEFAULT];

   if ( *lumpIndex >= wadFile->header.lumpCount )
   {
      Platform_Log( STR_WADERR_PATCHTABLEOVERFLOW );
      return False;
   }

   lumpInfo = &( wadFile->lumpInfoTable[*lumpIndex] );
   *lumpIndex += 1;

   while ( strncmp( lumpInfo->name, endName, 8 ) != 0 )
   {
      if ( *lumpIndex >= wadFile->header.lumpCount )
      {
         Platform_Log( STR_WADERR_PATCHTABLEOVERFLOW );
         return False;
      }
      else if ( lumpInfo->filePos + lumpInfo->size > (int32_t)( fileData->fileSize ) )
      {
         strncpy_s( lumpName, 9, lumpInfo->name, 8 );
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADERR_PATCHOVERFLOW, lumpName );
         Platform_Log( msg );
         return False;
      }
      else if ( *patchesRead >= wadFile->patchCount )
      {
         Platform_Log( STR_WADERR_PATCHCOUNTOVERFLOW );
         return False;
      }

      patchIndex = -1;

      for ( i = 0; i < wadFile->patchCount; i++ )
      {
         if ( strncmp( wadFile->patchTable[i].name, lumpInfo->name, 8 ) == 0 )
         {
            patchIndex = i;
            break;
         }
      }

      if ( patchIndex == -1 )
      {
         strncpy_s( lumpName, 9, lumpInfo->name, 8 );
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADWARN_UNDEFINEDTEXTUREPATCHLOADED, lumpName );
         Platform_Log( msg );

         patchIndex = wadFile->patchCount;
         wadFile->patchCount++;
         wadFile->patchTable = Platform_ReAlloc( wadFile->patchTable,
                                                 ( wadFile->patchCount - 1 ) * sizeof( WadFilePatch_t ),
                                                 wadFile->patchCount * sizeof( WadFilePatch_t ) );

         Platform_MemCpy( wadFile->patchTable[patchIndex].name, lumpInfo->name, 8 );
         wadFile->patchTable[patchIndex].size = lumpInfo->size;
         wadFile->patchTable[patchIndex].memory = Platform_MAlloc( (uint64_t)( lumpInfo->size ) );
         Platform_MemCpy( wadFile->patchTable[patchIndex].memory, (uint8_t*)( fileData->contents ) + lumpInfo->filePos, (uint64_t)( lumpInfo->size ) );
         *patchesRead += 1;
      }
      else
      {
         if ( wadFile->patchTable[patchIndex].memory )
         {
            strncpy_s( lumpName, 9, lumpInfo->name, 8 );
            snprintf( msg, STRING_SIZE_DEFAULT, STR_WADINFO_PATCHOVERWRITTEN, lumpName );
            Platform_Log( msg );
            Platform_Free( wadFile->patchTable[patchIndex].memory, wadFile->patchTable[patchIndex].size );
            wadFile->patchTable[patchIndex].memory = 0;
            wadFile->patchTable[patchIndex].size = 0;
            *patchesRead -= 1;
         }

         wadFile->patchTable[patchIndex].size = lumpInfo->size;
         wadFile->patchTable[patchIndex].memory = Platform_MAlloc( (uint64_t)( lumpInfo->size ) );
         Platform_MemCpy( wadFile->patchTable[patchIndex].memory, (uint8_t*)( fileData->contents ) + lumpInfo->filePos, (uint64_t)( lumpInfo->size ) );
         *patchesRead += 1;
      }

      lumpInfo = &( wadFile->lumpInfoTable[*lumpIndex] );
      *lumpIndex += 1;
   }

   return True;
}

internal Bool_t WadFile_ReadFlatLumps( WadFile_t* wadFile, FileData_t* fileData, int32_t* lumpIndex )
{
   int32_t i, f1Count = 0, f2Count = 0, flatIndex = 0;
   WadFileLumpInfo_t* lumpInfo;
   Bool_t cleanExit = False, stillGood = True;
   char msg[STRING_SIZE_DEFAULT];

   for ( i = *lumpIndex; i < wadFile->header.lumpCount; i++ )
   {
      lumpInfo = &( wadFile->lumpInfoTable[i] );

      if ( strncmp( lumpInfo->name, "F1_START", 8 ) == 0 )
      {
         i++;

         if ( !WadFile_CountFlatLumps( wadFile, &i, "F1_END", &f1Count ) )
         {
            return False;
         }
      }
      else if ( strncmp( lumpInfo->name, "F2_START", 8 ) == 0 )
      {
         i++;

         if ( !WadFile_CountFlatLumps( wadFile, &i, "F2_END", &f2Count ) )
         {
            return False;
         }
      }
      else if ( strncmp( lumpInfo->name, "F_END", 5 ) == 0 )
      {
         cleanExit = True;
         break;
      }
   }

   if ( !cleanExit )
   {
      Platform_Log( STR_WADERR_FLATTABLEOVERFLOW );
      return False;
   }

   wadFile->flatTable = (WadFileFlat_t*)Platform_MAlloc( sizeof( WadFileFlat_t ) * wadFile->flatCount );

   for ( i = 0; i < wadFile->flatCount; i++ )
   {
      wadFile->flatTable[i].size = 0;
      wadFile->flatTable[i].memory = 0;
   }

   while ( *lumpIndex < wadFile->header.lumpCount )
   {
      lumpInfo = &( wadFile->lumpInfoTable[*lumpIndex] );

      if ( strncmp( lumpInfo->name, "F1_START", 8 ) == 0 )
      {
         *lumpIndex += 1;
         stillGood = WadFile_ReadFlatLumpTable( wadFile, fileData, lumpIndex, flatIndex, "F1_END" );
         flatIndex += f1Count;
      }
      else if ( strncmp( lumpInfo->name, "F2_START", 8 ) == 0 )
      {
         *lumpIndex += 1;
         stillGood = WadFile_ReadFlatLumpTable( wadFile, fileData, lumpIndex, flatIndex, "F2_END" );
         flatIndex += f2Count;
      }
      else if ( strncmp( lumpInfo->name, "F_END", 8 ) == 0 )
      {
         break;
      }

      if ( !stillGood )
      {
         return False;
      }

      *lumpIndex += 1;
   }

   for ( i = 0; i < wadFile->flatCount; i++ )
   {
      if ( !wadFile->flatTable[i].memory )
      {
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADWARN_FLATNOTLOADED, i );
         Platform_Log( msg );
      }
   }

   return True;
}

internal Bool_t WadFile_CountFlatLumps( WadFile_t* wadFile, int32_t* lumpIndex, const char* endName, int32_t* count )
{
   int32_t flatCount = 0;
   Bool_t cleanExit = False;
   WadFileLumpInfo_t* lumpInfo;

   while ( *lumpIndex < wadFile->header.lumpCount )
   {
      lumpInfo = &( wadFile->lumpInfoTable[*lumpIndex] );

      if ( strncmp( lumpInfo->name, endName, 6 ) == 0 )
      {
         cleanExit = True;
         break;
      }
      else
      {
         flatCount++;
      }

      *lumpIndex += 1;
   }

   if ( !cleanExit )
   {
      Platform_Log( STR_WADERR_FLATTABLEOVERFLOW );
      return False;
   }
   else if ( flatCount == 0 )
   {
      Platform_Log( STR_WADERR_FLATTABLECOUNTZERO );
      return False;
   }

   wadFile->flatCount += flatCount;
   *count = flatCount;
   return True;
}

internal Bool_t WadFile_ReadFlatLumpTable( WadFile_t* wadFile, FileData_t* fileData, int32_t* lumpIndex, int32_t flatIndex, const char* endName )
{
   int32_t i, j;
   WadFileLumpInfo_t* lumpInfo;
   char lumpName[9];
   char msg[STRING_SIZE_DEFAULT];

   for ( i = flatIndex; i < wadFile->flatCount; i++, *lumpIndex += 1 )
   {
      lumpInfo = &( wadFile->lumpInfoTable[*lumpIndex] );

      if ( strncmp( lumpInfo->name, endName, 6 ) == 0 )
      {
         break;
      }

      if ( lumpInfo->filePos + lumpInfo->size > (int32_t)( fileData->fileSize ) )
      {
         strncpy_s( lumpName, 9, lumpInfo->name, 8 );
         snprintf( msg, STRING_SIZE_DEFAULT, STR_WADERR_FLATLUMP_OVERFLOW, lumpName );
         Platform_Log( msg );
         return False;
      }

      Platform_MemCpy( wadFile->flatTable[i].name, lumpInfo->name, 8 );

      for ( j = 0; j < 8; j++ )
      {
         wadFile->flatTable[i].name[j] = (char)toupper( wadFile->flatTable[i].name[j] );
      }

      wadFile->flatTable[i].size = lumpInfo->size;
      wadFile->flatTable[i].memory = Platform_MAlloc( lumpInfo->size );
      Platform_MemCpy( wadFile->flatTable[i].memory, (uint8_t*)( fileData->contents ) + lumpInfo->filePos, lumpInfo->size );
   }

   return True;
}
