#  HAWKEYE Library Makefile
#

OS := $(shell uname)

CC = gcc

LINKER = gcc

OBJS = hawkeye.o


ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -DNVLinux -D_LARGEFILE64_SOURCE

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libhawkeye.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~

    else

        TGT = libhawkeye.so

        MAKEFILE = Makefile

        CFLAGS = -fPIC -O -ansi -Wall -c -DNVLinux -D_LARGEFILE64_SOURCE

        LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

    .c.o:
	$(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(CC) $(CFLAGS) $(LINKER)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~

    endif


else


    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -DNVWIN3X  -D_LARGEFILE64_SOURCE -DHAWKEYE_STATIC

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libhawkeye.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~

    else

        TGT = libhawkeye.dll

        TGTa = libhawkeye.a

        CFLAGS = -O -ansi -Wall -c -DNVWIN3X  -D_LARGEFILE64_SOURCE -DHAWKEYE_DLL_EXPORT

        LINK_FLAGS = -shared -o $(TGT) -Wl,--out-implib,$(TGTa)

    .c.o:
	$(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(CC) $(CFLAGS) $(LINKER)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~

    endif


endif


hawkeye.o:			hawkeye.h hawkeye_nvtypes.h
