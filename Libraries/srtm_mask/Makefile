OS := $(shell uname)


OBJS = dted.o read_srtm1_topo.o read_srtm3_topo.o read_srtm2_topo.o read_srtm30_topo.o read_srtm_topo.o srtm_bit_pack.o

ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

	CFLAGS = -O -ansi -Wall -c -D_LARGEFILE64_SOURCE -DNVLinux -I $(PFM_INCLUDE)

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libsrtm.a

        $(TGT):	$(TGT)(dted.o) $(TGT)(read_srtm1_topo.o) $(TGT)(read_srtm3_topo.o) $(TGT)(read_srtm2_topo.o) $(TGT)(read_srtm30_topo.o) $(TGT)(read_srtm_topo.o) $(TGT)(srtm_bit_pack.o)

	rm -f *~
	mv $(TGT) $(PFM_LIB)
	cp dted.h read_srtm_topo.h read_srtm1_topo.h read_srtm2_topo.h read_srtm3_topo.h read_srtm30_topo.h $(PFM_INCLUDE)

    else

        CC = gcc

        LINKER = gcc

	TGT = libsrtm.so

        MAKEFILE = Makefile

	CFLAGS = -fPIC -O -ansi -Wall -c -D_LARGEFILE64_SOURCE -DNVLinux -I $(PFM_INCLUDE)

	LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

        .c.o:
	    $(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~
	mv $(TGT) $(PFM_LIB)
	cp dted.h read_srtm_topo.h read_srtm1_topo.h read_srtm2_topo.h read_srtm3_topo.h read_srtm30_topo.h $(PFM_INCLUDE)

    endif

else


    ARFLAGS=r

    CC = gcc

    CFLAGS = -ansi -O -Wall -c -DNVWIN3X -I $(PFM_INCLUDE)


    .c.o:
	$(CC) -c $(CFLAGS) -I$(PFM_INCLUDE) $*.c


    TGT = libsrtm.a


    $(TGT):	$(TGT)($(OBJS))
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp dted.h read_srtm_topo.h read_srtm1_topo.h read_srtm2_topo.h read_srtm3_topo.h read_srtm30_topo.h $(PFM_INCLUDE)

endif




dted.o:			dted.h

read_srtm_topo.o:	read_srtm_topo.h read_srtm1_topo.h read_srtm2_topo.h read_srtm3_topo.h read_srtm30_topo.h

read_srtm1_topo.o:	read_srtm1_topo.h

read_srtm2_topo.o:	read_srtm2_topo.h

read_srtm3_topo.o:	read_srtm3_topo.h

read_srtm30_topo.o:	read_srtm30_topo.h
