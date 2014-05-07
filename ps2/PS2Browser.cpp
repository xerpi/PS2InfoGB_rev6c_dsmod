/*
 *  Copyright (C) 2010  Krystian Karaœ <k4rasq@gmail.com>
 *
 *  I release it for fully free use by anyone, anywhere, for any purpose.
 *  But I still retain the copyright, so no one else can limit this release.
 */

#include <tamtypes.h>
#include <string.h>
#include <fileio.h>
#include <stdio.h>
#include <libmc.h>

#include "PS2Browser.h"
#include "lib/libcdvd/ee/cdvd_rpc.h"

///////////////////////////////////////////////////////////////////
// File mode flags (for mode in io_stat_t)
#define FIO_SO_IFMT		0x0038		// Format mask
#define FIO_SO_IFLNK		0x0008		// Symbolic link
#define FIO_SO_IFREG		0x0010		// Regular file
#define FIO_SO_IFDIR		0x0020		// Directory

// File mode checking macros
#define FIO_SO_ISLNK(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFLNK)
#define FIO_SO_ISREG(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFREG)
#define FIO_SO_ISDIR(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFDIR)
///////////////////////////////////////////////////////////////////

static mcTable McEntries[MAX_FILES_ALLOWED] __attribute__((aligned(64)));
static struct TocEntry TocEntries[MAX_FILES_ALLOWED] __attribute__((aligned(64)));

const char *DeviceID[] = {
   "mc0:/", "mc1:/", "mass:/", "cdfs:/"
};

const int numEntryTypes = 3;
const char *EntryTypes[numEntryTypes] = {
   ".gb", ".gbc", ".zip"
};

const int numSize = (sizeof(char)*4*numEntryTypes) + numEntryTypes;
char CdvdEntryTypesTemp[numSize];

////////////////////////////////////////////////////////
Browser::Browser() :
   CurrentSaveDevice(DEV_MC0),
   CurrentFilesNum(0),
   CurrentDirsNum(0)
{
   // Set to default device

   /* with false arg (to not update content) cause MC is not inited yet so
    * we can't read from this device (CPU throws exception) */

   setCurrentDevice(DEV_MC0, false);

   // Fixing cdvd types
   strcpy(CdvdEntryTypesTemp, "");

   for ( int i = 0; i < numEntryTypes; i++ ) {
      strncat(CdvdEntryTypesTemp, EntryTypes[i], 4);
      strncat(CdvdEntryTypesTemp, " ", 1);
   }
}

Browser::~Browser() {
}

////////////////////////////////////////////////////////
void Browser::SortList(PS2DataFile* List, int IndexStart, int IndexEnd) {
   int i = IndexStart,
       j = IndexEnd;

   PS2DataFile x = List[(IndexStart + IndexEnd) / 2];

   do {
      while ( strcmp(List[i].FileName, x.FileName) < 0 )
         ++i;
      while ( strcmp(List[j].FileName, x.FileName) > 0 )
         --j;

      if ( i <= j ) {
         PS2DataFile Temp = List[i];

         List[i] = List[j];
         List[j] = Temp;

         ++i; --j;
      }
   } while ( i <= j );

   if ( IndexStart < j )
      SortList(List, IndexStart, j);

   if ( IndexEnd > i )
      SortList(List, i, IndexEnd);
}

////////////////////////////////////////////////////////
void Browser::setToUpperDir() {
   int i = 0, numBackLevel = 0;

   while ( CurrentFilesPath[i] != 0 ) {
      if ( CurrentFilesPath[i++] == '/' && CurrentFilesPath[i] != 0 )
         numBackLevel = i;
   }

   if ( numBackLevel > 0 ) {
      CurrentFilesPath[numBackLevel] = 0;
   }

   getCurrentDeviceContent();
}

////////////////////////////////////////////////////////
void Browser::setCurrentFilesPath(int DirIndex) {
   if ( DirIndex < CurrentDirsNum && DirIndex < MAX_DIRS_ALLOWED ) {
      /* what? this way sometimes sign "/" is missed... why?
      strncat(CurrentFilesPath, CurrentDirsList[DirIndex].FileName, MAX_FILENAME_CHARS);
      strncat(CurrentFilesPath, "/", 1);
      */

      char Temp[MAX_FILENAME_CHARS];

      strncpy(Temp, CurrentDirsList[DirIndex].FileName, MAX_FILENAME_CHARS);
      strncat(CurrentFilesPath, strncat(Temp, "/", 1), MAX_FILENAME_CHARS);

      getCurrentDeviceContent();
   }
}

////////////////////////////////////////////////////////
bool Browser::setCurrentSaveDevice(PS2Device device) {
   int ret = 0;
   char temp[32];

   if ( device == DEV_CDVD ) {
      return false;
   }

   sprintf(temp, "%sPS2GB", DeviceID[(int)device]);
   ret = fioDopen(temp);

   if ( ret < 0 ) {
      ret = fioMkdir(temp);
      if ( ret < 0 ) {
         return false;
      }
   } else {
      fioDclose(ret);
   }

   CurrentSaveDevice = device;
   return true;
}

////////////////////////////////////////////////////////
void Browser::setCurrentDevice(PS2Device device, bool Update) {
   CurrentFilesNum = CurrentDirsNum = 0;

   CurrentDevice = device;
   strcpy(CurrentFilesPath, DeviceID[device]);

   if ( Update )
      getCurrentDeviceContent();
}

////////////////////////////////////////////////////////
void Browser::getCurrentDeviceContent() {
   switch ( CurrentDevice ) {
      case DEV_MC0:
      case DEV_MC1:
         CurrentFilesNum = ReadMC(FILES);
         CurrentDirsNum  = ReadMC(DIRS);
         break;

      case DEV_CDVD:
         CurrentFilesNum = ReadCDVD(FILES);
         CurrentDirsNum  = ReadCDVD(DIRS);
         break;

      case DEV_USBMASS:
         CurrentFilesNum = ReadMass(FILES);
         CurrentDirsNum  = ReadMass(DIRS);
         break;
   }

   SortList(CurrentFilesList, 0, CurrentFilesNum - 1);
   SortList(CurrentDirsList, 0, CurrentDirsNum - 1);
}

////////////////////////////////////////////////////////
bool Browser::isEntryTypeAllowed(char *EntryName) {
   char *p = strrchr(EntryName, '.');

   if ( p != 0 ) {
      for ( int i = 0; i < numEntryTypes; i++ ) {
         if ( stricmp(p, EntryTypes[i]) == 0 )
            return true;
      }
   }
   return false;
}

char *Browser::getSavePath(const char *FileName, const char *SaveExt) {
   static char SavePathBuffer[256];

   if ( CurrentSaveDevice == DEV_CDVD ) {
      return 0;
   }

   if ( SaveExt ) {
      char TempFileName[128];

      strncpy(TempFileName, FileName, 128);

      if ( !setFileExt(TempFileName, SaveExt) ) {
         strncat(TempFileName, SaveExt, strlen(SaveExt));
      }
      sprintf(SavePathBuffer, "%sPS2GB/%s", DeviceID[CurrentSaveDevice], TempFileName);
   } else {
      sprintf(SavePathBuffer, "%sPS2GB/%s", DeviceID[CurrentSaveDevice], FileName);
   }
   return SavePathBuffer;
}

////////////////////////////////////////////////////////
char *Browser::setFileExt(char *fname, /*int length,*/ const char *ext) {
   char *p = strrchr(fname, '.');
   //int sp, sext;

   if ( !p ) {
      return 0;
   }

   /*
   sp = strlen(p);
   sext = strlen(ext);

   if ( sp < sext ) {
      int sfname = strlen(fname);
      if ( sfname + ( sext - sp ) > length ) {
         return 0;
      }
   }
   */

   *p = 0;
   strncat(p, ext, strlen(ext));

   return fname;
}

////////////////////////////////////////////////////////
//                Get Devices Content                 //
////////////////////////////////////////////////////////

int Browser::ReadMC(PS2ContentType Type) {
   char *MCPath = &CurrentFilesPath[5];
   char MCTempPath[MAX_FILEPATH_CHARS];
   int numFiles = 0, numDirs = 0, numRet;
   int i = 0;

   strcpy(MCTempPath, MCPath);
   strcat(MCTempPath, "*");

   mcGetDir((int)CurrentFilesPath[2]-'0', 0, MCTempPath, 0, MAX_FILES_ALLOWED - 2, McEntries);
	mcSync(0, NULL, &numRet);

   if ( Type == DIRS ) {
      // Skip pseudo-folder "." or ".."
      if ( !strcmp((const char*)McEntries[0].name, ".") ) {
         i = 2;
      }

      for ( ; i < numRet; i++) {
         if ( McEntries[i].attrFile & MC_ATTR_SUBDIR ) {
            strncpy(CurrentDirsList[numDirs].FileName, (const char *)McEntries[i].name, MAX_FILENAME_CHARS);
            numDirs++;
         }
      }
      return numDirs;
   } else {
      for ( ; i < numRet; i++) {
        if ( !(McEntries[i].attrFile & MC_ATTR_SUBDIR) ) {
            if ( isEntryTypeAllowed((char *)McEntries[i].name) ) {
               strncpy(CurrentFilesList[numFiles].FileName, (const char *)McEntries[i].name, MAX_FILENAME_CHARS);
               numFiles++;
            }
         }
      }
      return numFiles;
   }
}

////////////////////////////////////////////////////////
int Browser::ReadCDVD(PS2ContentType Type) {
   int Result = 0;
   bool SubDir = false;
   char *CdPath = &CurrentFilesPath[5];

   while ( CDVD_DiskReady(CdBlock) == CdNotReady );

   if ( Type == DIRS ) {
      Result = CDVD_GetDir(CdPath, NULL, CDVD_GET_DIRS_ONLY, TocEntries,
         MAX_DIRS_ALLOWED, NULL);

      if ( !strcmp(TocEntries[0].filename, "..") ) {
         SubDir = true;
      }

      for ( int i = 0; i < Result; i++ )
         strncpy(CurrentDirsList[i].FileName, TocEntries[i+SubDir].filename, MAX_FILENAME_CHARS);
	} else {
      Result = CDVD_GetDir(CdPath, CdvdEntryTypesTemp, CDVD_GET_FILES_ONLY,
         TocEntries, MAX_FILES_ALLOWED, NULL);

      for ( int i = 0; i < Result; i++ )
         strncpy(CurrentFilesList[i].FileName, TocEntries[i].filename, MAX_FILENAME_CHARS);
   }
   return  SubDir ? (Result - 1) : Result;
}

////////////////////////////////////////////////////////
int Browser::ReadMass(PS2ContentType Type) {
   fio_dirent_t record;
   int current_result = 0, dirs = -1;

   if ( ( dirs = fioDopen(CurrentFilesPath) ) >= 0 ) {
      if ( Type == DIRS ) {
         while ( fioDread(dirs, &record) > 0 ) {
            if ( FIO_SO_ISDIR(record.stat.mode) ) {
               // Skip entry if pseudo-folder "." or ".."
               if ( !strcmp(record.name, ".") || !strcmp(record.name, "..") ) {
                  continue;
               }
               strncpy(CurrentDirsList[current_result++].FileName, record.name, MAX_FILENAME_CHARS);
            } else if ( !FIO_SO_ISREG(record.stat.mode) ) {
               continue; // Skip entry which is neither a file nor a folder
            }

            if ( current_result == MAX_DIRS_ALLOWED )
               break;
         }
      } else {
         while ( fioDread(dirs, &record) > 0 ) {
            if ( FIO_SO_ISDIR(record.stat.mode) ) {
               continue; // Skip Dirs
            } else if ( !FIO_SO_ISREG(record.stat.mode) ) {
               continue; // Skip entry which is neither a file nor a folder
            }

            if ( isEntryTypeAllowed(record.name) )
               strncpy(CurrentFilesList[current_result++].FileName, record.name, MAX_FILENAME_CHARS);

            if ( current_result == MAX_FILES_ALLOWED )
               break;
         }
      }

      // Close directory
      fioDclose(dirs);
   }

   return current_result;
}
////////////////////////////////////////////////////////
