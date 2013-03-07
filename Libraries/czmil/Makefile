OS := $(shell uname)

CC = gcc

LINKER = gcc

OBJS = czmil.o


ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -D_LARGEFILE64_SOURCE -DNVLinux -DUNIX -I $(PFM_INCLUDE)

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libCZMIL.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~

    else

        CC = gcc

        LINKER = gcc

        TGT = libCZMIL.so

        MAKEFILE = Makefile

        CFLAGS = -fPIC -O -ansi -Wall -c -D_LARGEFILE64_SOURCE -DNVLinux -DUNIX -I $(PFM_INCLUDE)

        LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

    .c.o:
	$(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~

    endif


else


    ARFLAGS=r

    CC = gcc

    OS := $(shell uname)

    CFLAGS = -ansi -O -Wall -c -DNVWIN3X -D_LARGEFILE64_SOURCE -DCZMIL_STATIC -I $(PFM_INCLUDE)


    .c.o:
	$(CC) -c $(CFLAGS) -I$(PFM_INCLUDE) $*.c


    TGT = libCZMIL.a


    $(TGT):	$(TGT)($(OBJS))
	rm -f *~

endif



czmil.o:  	czmil.h czmil_macros.h czmil_nvtypes.h czmil_internals.h czmil_functions.h
