LIB_UNZLIB = lib/libunzlib.a

EE_INCS  = -I../zlib/include -I. -I./include
EE_CFLAGS =
EE_OBJS = aio-fio.o src/explode.o src/unreduce.o src/unshrink.o src/unzip.o   

EE_LIB = $(LIB_UNZLIB)

all: $(EE_LIB)

clean:
	rm -f $(EE_LIB) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal