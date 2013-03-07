#  CHRTR2 Library Makefile
#

OS := $(shell uname)

CC = gcc

LINKER = gcc

OBJS = chrtr2.o


ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -DNVLinux -D_LARGEFILE64_SOURCE

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libchrtr2.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~

    else

        TGT = libchrtr2.so

        MAKEFILE = Makefile

        CFLAGS = -fPIC -O -ansi -Wall -c -DNVLinux -D_LARGEFILE64_SOURCE

        LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

    .c.o:
	$(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~

    endif


else


    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -DNVWIN3X  -D_LARGEFILE64_SOURCE -DCHRTR2_STATIC

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libchrtr2.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~

    else

        TGT = libchrtr2.dll

        TGTa = libchrtr2.a

        CFLAGS = -O -ansi -Wall -c -DNVWIN3X  -D_LARGEFILE64_SOURCE -DCHRTR2_DLL_EXPORT

        LINK_FLAGS = -shared -o $(TGT) -Wl,--out-implib,$(TGTa)

    .c.o:
	$(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(CC) $(CFLAGS) $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~

    endif


endif


chrtr2.o:		chrtr2.h chrtr2_nvtypes.h chrtr2_internals.h chrtr2_macros.h chrtr2_shared.h chrtr2_functions.h
