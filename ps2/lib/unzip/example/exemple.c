

#include "../../testz/unzip/unzip.h"
unz_file_info info;



int check_zip(char *filename)
{
    char *extension = strrchr(filename, '.');
	if(!extension)
		return 0;

	if(!strcasecmp(extension, ".zip"))return 1;
	else if(!strcasecmp(extension, ".ZIP"))return 1;
	else return 0;
}
       
int load_rom(char *filename)
{
    
     unsigned long file_length;
      int fd,fd_size;  
      int n;	
      int ret = 0;
      int ok=0;
      char tmpnom[0x100];    
  
 if(check_zip(filename))
    { 
        unzFile fd2=unzOpen(filename);        
         
        if(fd2==NULL)return (1);
                
        // Go to first file in archive 
        ret = unzGoToFirstFile(fd2);
        if(ret != UNZ_OK) {
            unzClose(fd2);
            return (1);
        }
        // Get information on the file 
        ret = unzGetCurrentFileInfo(fd2, &info, tmpnom, 0x100, NULL, 0, NULL, 0);
        if(ret != UNZ_OK) {
            unzClose(fd2);
            return (1);
        }
      
        //Open the file for reading 
        ret = unzOpenCurrentFile(fd2);
        if(ret != UNZ_OK) {
            unzClose(fd2);
            return (1);
        }       
        // Allocate file data buffer 
        fd_size = info.uncompressed_size;
       
        // Read (decompress) the file 
       // cartridge_rom = (unsigned char *)malloc(fd_size);
        cartridge_rom = (u8 *)memalign(64, fd_size);
        ret = unzReadCurrentFile(fd2,/*(char *)*/cartridge_rom, info.uncompressed_size);
        if(ret != info.uncompressed_size)
        {
            //free(buf2);
            unzCloseCurrentFile(fd2);
            unzClose(fd2);
            return (1);
        }
        //printf("zip decomp %d \n",(int)info.uncompressed_size);

        // Close the current file 
        ret = unzCloseCurrentFile(fd2);
        if(ret != UNZ_OK) {           
            unzClose(fd2);
            return (1);
        }
        // printf("zip close file\n");

        // Close the archive 
        ret = unzClose(fd2);
        if(ret != UNZ_OK) {            
            return (1);
        }
        // printf("zip close archive\n");

    // Check for 512-byte header    
   // ok=0;
   // cartridge_rom = (unsigned char *)malloc(fd_size);
   // for(n = 0; n <fd_size;n++)cartridge_rom[n]=buf[n+ok];  
    printf("zip header / rom copy %d %d %s \n",fd_size,ok,tmpnom);
    
 } 
 else{
  
   fd = fioOpen(filename, O_RDONLY);
   if(fd <= 0) {
	//	display_error("Error opening file.",0);
      printf("%s not found.\n",filename);
		return 0;
	}
 
	file_length = fioLseek(fd,0,SEEK_END);
	fioLseek(fd,0,SEEK_SET);

    cartridge_rom = (unsigned char *)malloc(file_length);
	fioRead(fd, (char *)cartridge_rom, file_length);

    fioClose(fd);
    
 }
 
 // traitement du fich rom ....
 
 return 0; 
 
 }
   
