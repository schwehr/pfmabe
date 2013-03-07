OS := $(shell uname)

ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -DNVLinux -DUNIX -DLINUX

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libpfm.a

        $(TGT):	$(TGT)(pfm_io.o) $(TGT)(bit_pack.o) $(TGT)(huge_io.o) $(TGT)(gp.o) $(TGT)(pfm_extras.o) $(TGT)(hyp.o)

	rm -f *~
	mv $(TGT) $(PFM_LIB)
	cp pfm.h $(PFM_INCLUDE)
	cp huge_io.h $(PFM_INCLUDE)
	cp pfm_nvtypes.h $(PFM_INCLUDE)
	cp pfm_extras.h $(PFM_INCLUDE)
	cp hyp.h $(PFM_INCLUDE)

    else

        CC = gcc

        LINKER = gcc

        OBJS = pfm_io.o bit_pack.o huge_io.o gp.o pfm_extras.o hyp.o

        .c.o:
	    $(CC) $(CFLAGS) $*.c

        TGT = libpfm.so

        MAKEFILE = Makefile

        CFLAGS = -fPIC -O -ansi -Wall -c -DNVLinux -DUNIX -DLINUX

        LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

all: $(TGT)
{-c $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~
	mv $(TGT) $(PFM_LIB)
	cp pfm.h $(PFM_INCLUDE)
	cp huge_io.h $(PFM_INCLUDE)
	cp pfm_nvtypes.h $(PFM_INCLUDE)
	cp pfm_extras.h $(PFM_INCLUDE)
	cp hyp.h $(PFM_INCLUDE)

    endif

else


# Due to the insanity that is the Windows OS I am no longer building this as a shared library in Windows.
# If you'd like to know why just Google (TM) for __declspec and MinGW.

    ARFLAGS=r

    CC = gcc

    CFLAGS = -O -ansi -Wall -c -DNVWIN3X

    .c.o:
	$(CC) -c $(CFLAGS) $*.c

    TGT = libpfm.a

    $(TGT):	$(TGT)(pfm_io.o) $(TGT)(bit_pack.o) $(TGT)(huge_io.o) $(TGT)(gp.o) $(TGT)(pfm_extras.o) $(TGT)(hyp.o)

	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp pfm.h $(PFM_INCLUDE)
	cp huge_io.h $(PFM_INCLUDE)
	cp pfm_nvtypes.h $(PFM_INCLUDE)
	cp pfm_extras.h $(PFM_INCLUDE)
	cp hyp.h $(PFM_INCLUDE)

endif


pfm_io.o:     	     pfm.h pfm_header.h pfm_version.h huge_io.h pfm_nvtypes.h pfm_extras.h pfm_cached_io.c
pfm_cached_io.o:     pfm.h pfm_header.h pfm_version.h huge_io.h pfm_nvtypes.h pfm_extras.h
bit_pack.o:          pfm_nvtypes.h
huge_io.o:           huge_io.h pfm_nvtypes.h
hyp.o:               hyp.h huge_io.h pfm_nvtypes.h
