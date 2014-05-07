/*
 *  Copyright (C) 2010  Krystian Karaœ <k4rasq@gmail.com>
 *
 *  I release it for fully free use by anyone, anywhere, for any purpose.
 *  But I still retain the copyright, so no one else can limit this release.
 */

#ifndef _PS2_BROWSER_
#define _PS2_BROWSER_

// ------- Browser Config ------- //

#define MAX_FILENAME_CHARS 64
#define MAX_FILEPATH_CHARS 1024

#define MAX_FILES_ALLOWED  256
#define MAX_DIRS_ALLOWED   128

// ------------------------------ //

extern const char *DeviceID[];

enum PS2ContentType {
   FILES = 0,
   DIRS
};

enum PS2Device {
   DEV_MC0 = 0,
   DEV_MC1,
   DEV_USBMASS,
   DEV_CDVD
};

typedef struct _PS2DataFile {
   char FileName[MAX_FILENAME_CHARS+1];
} PS2DataFile;

class Browser {
   public:
      // Get file name by index from
      // current list of files
      char *getEntryFileName(int FileIndex) {
         return ( FileIndex < CurrentFilesNum && FileIndex < MAX_FILES_ALLOWED ) ?
            CurrentFilesList[FileIndex].FileName : 0;
      }

      // Get dir name by index from
      // current list of dirs
      char *getEntryDirName(int DirIndex) {
         return ( DirIndex < CurrentDirsNum && DirIndex < MAX_DIRS_ALLOWED ) ?
            CurrentDirsList[DirIndex].FileName : 0;
      }

      // Load content of lists for current device
      // (also kind of refreash option)
      void getCurrentDeviceContent();

      // Returns current path
      const char *getCurrentFilesPath() {
         return CurrentFilesPath;
      }

      // Back to upper dir
      // (one level upper if no root)
      void setToUpperDir();

      // Set current path; DirIndex is an index of
      // dir list (CurrentFilesList)
      void setCurrentFilesPath(int DirIndex);

      // Change current device
      void setCurrentDevice(PS2Device device, bool Update = true);

      bool setCurrentSaveDevice(PS2Device device);

      // Return current device
      PS2Device getCurrentDevice() {
         return CurrentDevice;
      }

      PS2Device getCurrentSaveDevice() {
         return CurrentSaveDevice;
      }

      // Return number of file in current location
      int getCurrentFilesNum() {
         return CurrentFilesNum;
      }

      // Return number of dirs in current location
      int getCurrentDirsNum() {
         return CurrentDirsNum;
      }

      // Check if entry file name has
      // one of the type defined in *EntryTypes
      bool isEntryTypeAllowed(char *EntryName);

      // Return full save path to file *FileName.If you pass extenstion
      // *SaveExt (with dot) and *FileName doesn't have any extension, it will add
      // *SaveExt to file name, otherwise it will replace current *FileName
      // extension to *SaveExt
      char *getSavePath(const char *FileName, const char *SaveExt = 0);

      Browser();
     ~Browser();

   private:
      char        CurrentFilesPath[MAX_FILEPATH_CHARS+1];
      PS2DataFile CurrentFilesList[MAX_FILES_ALLOWED];
      PS2DataFile CurrentDirsList[MAX_DIRS_ALLOWED];
      PS2Device   CurrentDevice;
      PS2Device   CurrentSaveDevice;

      // Number of files and dirs for current location
      int CurrentFilesNum;
      int CurrentDirsNum;

      // Sort files and dirs list
      void SortList(PS2DataFile* data, int endIndex, int startIndex);

      // Drivers for browser
      int ReadMass(PS2ContentType Type);
      int ReadCDVD(PS2ContentType Type);
      int ReadMC(PS2ContentType Type);

      // Set extension (used in getSavePath)
      char *setFileExt(char *fname, /*int length,*/ const char *ext);
};

#endif
