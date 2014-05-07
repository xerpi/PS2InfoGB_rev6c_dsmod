//--------------------------------------------------------------
//File name:    elf.c
//--------------------------------------------------------------
#include <stdio.h>
#include <tamtypes.h>
#include <loadfile.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <fileio.h>
#include <sys/stat.h>
#include <fileXio_rpc.h>
#include <sys/fcntl.h>

extern u8 *loader_elf;
extern int size_loader_elf;

extern u8 *fakehost_irx;
extern int size_fakehost_irx;

// ELF-loading stuff
#define ELF_MAGIC		0x464c457f
#define ELF_PT_LOAD		1

//------------------------------
typedef struct
{
	u8	ident[16];			// struct definition for ELF object header
	u16	type;
	u16	machine;
	u32	version;
	u32	entry;
	u32	phoff;
	u32	shoff;
	u32	flags;
	u16	ehsize;
	u16	phentsize;
	u16	phnum;
	u16	shentsize;
	u16	shnum;
	u16	shstrndx;
} elf_header_t;
//------------------------------
typedef struct
{
	u32	type;				// struct definition for ELF program section header
	u32	offset;
	void	*vaddr;
	u32	paddr;
	u32	filesz;
	u32	memsz;
	u32	flags;
	u32	align;
} elf_pheader_t;
//--------------------------------------------------------------
//End of data declarations
//--------------------------------------------------------------
// RunLoaderElf loads LOADER.ELF from program memory and passes
// args of selected ELF and partition to it
// Modified version of loader from Independence
//	(C) 2003 Marcus R. Brown <mrbrown@0xd6.org>
//------------------------------
void RunLoaderElf(char *filename, char *party)
{
	u8 *boot_elf;
	elf_header_t *eh;
	elf_pheader_t *eph;
	void *pdata;
	int ret, i;
	char *argv[2];

	if((!strncmp(party, "hdd0:", 5)) && (!strncmp(filename, "pfs0:", 5))){
		char fakepath[128], *p;
		if(0 > fileXioMount("pfs0:", party, FIO_MT_RDONLY)) {
		  //Some error occurred, it could be due to something else having used pfs0
			//unmountParty(0);  //So we try unmounting pfs0, to try again
			if(fileXioMount("pfs0:", party, FIO_MT_RDONLY) < 0)
			   return;  //If it still fails, we have to give up...
      }
		strcpy(fakepath,filename);
		p=strrchr(fakepath,'/');
		if(p==NULL) strcpy(fakepath,"pfs0:");
		else
		{
			p++;
			*p='\0';
		}
		//printf("Loading fakehost.irx %i bytes\n", size_fakehost_irx);
		//printf("Faking for path \"%s\" on partition \"%s\"\n", fakepath, party);
		SifExecModuleBuffer(&fakehost_irx, size_fakehost_irx, strlen(fakepath), fakepath, &ret);
		
	}

/* NB: LOADER.ELF is embedded  */
	boot_elf = (u8 *)&loader_elf;
	eh = (elf_header_t *)boot_elf;
	if (_lw((u32)&eh->ident) != ELF_MAGIC)
		while (1);

	eph = (elf_pheader_t *)(boot_elf + eh->phoff);

/* Scan through the ELF's program headers and copy them into RAM, then
									zero out any non-loaded regions.  */
	for (i = 0; i < eh->phnum; i++)
	{
		if (eph[i].type != ELF_PT_LOAD)
		continue;

		pdata = (void *)(boot_elf + eph[i].offset);
		memcpy(eph[i].vaddr, pdata, eph[i].filesz);

		if (eph[i].memsz > eph[i].filesz)
			memset(eph[i].vaddr + eph[i].filesz, 0,
					eph[i].memsz - eph[i].filesz);
	}

/* Let's go.  */
	fioExit();
	SifInitRpc(0);
	SifExitRpc();
	FlushCache(0);
	FlushCache(2);

	argv[0] = filename;
	argv[1] = party;

	ExecPS2((void *)eh->entry, 0, 2, argv);
}
//------------------------------
//End of func:  void RunLoaderElf(char *filename, char *party)
//--------------------------------------------------------------
//End of file:  elf.c
//--------------------------------------------------------------
