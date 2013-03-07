#  WLF Library Makefile
#

OS := $(shell uname)

CC = gcc

LINKER = gcc

OBJS = wlf.o wlf_class.o wlf_utilities.o


ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -DNVLinux -D_LARGEFILE64_SOURCE

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libwlf.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~

    else

        TGT = libwlf.so

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

        CFLAGS = -O -ansi -Wall -c -DNVWIN3X  -D_LARGEFILE64_SOURCE -DWLF_STATIC

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libwlf.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~

    else

        TGT = libwlf.dll

        TGTa = libwlf.a

        CFLAGS = -O -ansi -Wall -c -DNVWIN3X  -D_LARGEFILE64_SOURCE -DWLF_DLL_EXPORT

        LINK_FLAGS = -shared -o $(TGT) -Wl,--out-implib,$(TGTa)

    .c.o:
	$(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~

    endif


endif


wlf.o:			wlf.h wlf_nvtypes.h wlf_class.h

wlf_class.o:		wlf.h wlf_nvtypes.h wlf_class.h

wlf_utilities.o:	wlf.h wlf_nvtypes.h
