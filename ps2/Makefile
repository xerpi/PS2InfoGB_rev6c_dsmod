# Programs
EE_PACKER = ps2-packer-lite
EE_BIN2S  = bin2s
EE_STRIP  = ee-strip

# Set dir(s)
IRX_DIR = ./irx
BIN_DIR = ../bin

# PS2InfoGB elf file
EE_BIN = $(BIN_DIR)/PS2InfoGB.elf

# Include headers/libs
EE_INCS 	+= -I./gfxpipe -I./lib/zlib/include -I./lib/unzip/include 
EE_LDFLAGS 	+= -L./lib

# Compilers flags
EE_CXXFLAGS += -finline-functions -fstrict-aliasing -funsigned-char -fomit-frame-pointer 
EE_CXXFLAGS += -s
EE_CFLAGS 	= $(EE_CXXFLAGS)

# standard libs
EE_LIBS += -lgcc -lstdc++ -lmc -lpad -lfileXio -lpatches

# additional libs
EE_LIBS += -lcdvdfs -lunzlib -lz -lcdvd -laudsrv

# IRX modules - modules have to be in IRX_DIR
EE_IRX = cdvd.irx usbd.irx usbhdfsd.irx audsrv.irx fakehost.irx
EE_IRX_OBJ = $(EE_IRX:.irx=.o)

# Objects
EE_OBJS  = $(EE_IRX_OBJ)
EE_OBJS += ./../daa.o ./../mem.o ./../cpu.o ./../joypad.o ./../rom.o ./../globals.o ./../sound.o ./../vram.o ./../RTC.o ./../noise.o \
	     ./hw.o ./gs.o ./fontmsx.o ./gfxpipe/gfxpipe.o \
	     ./PS2Print.o ./PS2Input.o ./PS2Init.o ./PS2GUI.o ./PS2InfoGB.o ./PS2Time.o ./PS2Browser.o ./Reboot_ELF.o\
	     ./loader.o

all: $(EE_BIN)
	$(EE_STRIP) $(EE_BIN)
	@echo Done!

release: clean all pack

$(EE_IRX_OBJ:.o=.s):
	$(EE_BIN2S) $(IRX_DIR)/$(@:.s=.irx) $@ $(@:.s=_irx)

loader.s:
	$(MAKE) -C lib/loader
	$(EE_BIN2S) lib/loader/loader.elf loader.s loader_elf

clean:
	rm -f $(EE_OBJS) $(EE_IRX:.irx=.s) loader.s ./lib/loader/*.o ./lib/loader/*.elf

pack:
	$(EE_PACKER) $(EE_BIN) $(EE_BIN:.elf=-packed.elf)


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal