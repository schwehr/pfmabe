#  Binary Feature Data Library Makefile
#

OS := $(shell uname)

CC = gcc

LINKER = gcc

OBJS = binaryFeatureData.o

ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CFLAGS = -O -ansi -Wall -c -DNVLinux -D_LARGEFILE64_SOURCE

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libBinaryFeatureData.a

        $(TGT):	$(TGT)($(OBJS))

	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp binaryFeatureData.h binaryFeatureData_nvtypes.h binaryFeatureData_macros.h $(PFM_INCLUDE)

    else

        TGT = libBinaryFeatureData.so

        MAKEFILE = Makefile

        CFLAGS = -fPIC -O -ansi -Wall -c -DNVLinux -D_LARGEFILE64_SOURCE

        LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

    .c.o:
	$(CC) $(CFLAGS) -I$(PFM_INCLUDE) $*.c

all: $(TGT)
{-c $(CC) $(CFLAGS) $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~
	mv $(TGT) $(PFM_LIB)
	cp binaryFeatureData.h binaryFeatureData_nvtypes.h binaryFeatureData_macros.h $(PFM_INCLUDE)

    endif


else


    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CFLAGS = -O -ansi -Wall -c -DNVWIN3X  -D_LARGEFILE64_SOURCE -DBFDATA_STATIC

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libBinaryFeatureData.a

        $(TGT):	$(TGT)($(OBJS))

	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp binaryFeatureData.h binaryFeatureData_nvtypes.h binaryFeatureData_macros.h $(PFM_INCLUDE)

    else

        TGT = libBinaryFeatureData.dll

        TGTa = libBinaryFeatureData.a

        CFLAGS = -O -ansi -Wall -c -DNVWIN3X  -D_LARGEFILE64_SOURCE -DBFDATA_DLL_EXPORT

        LINK_FLAGS = -shared -o $(TGT) -Wl,--out-implib,$(TGTa)

    .c.o:
	$(CC) $(CFLAGS) -I$(PFM_INCLUDE) $*.c

all: $(TGT)
{-c $(CC) $(CFLAGS) $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp binaryFeatureData.h binaryFeatureData_nvtypes.h binaryFeatureData_macros.h $(PFM_INCLUDE)

    endif


endif


binaryFeatureData.o:	binaryFeatureData.h binaryFeatureData_internals.h binaryFeatureData_functions.h binaryFeatureData_nvtypes.h binaryFeatureData_macros.h
