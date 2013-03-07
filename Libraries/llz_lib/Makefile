#  LLZ Library Makefile
#
OS := $(shell uname)


OBJS = llz.o


ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -D_LARGEFILE64_SOURCE -DNVLinux -DUNIX -I $(PFM_INCLUDE)

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libllz.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp llz.h $(PFM_INCLUDE)

    else

        CC = gcc

        LINKER = gcc

        TGT = libllz.so

        MAKEFILE = Makefile

        CFLAGS = -fPIC -O -ansi -Wall -c -D_LARGEFILE64_SOURCE -DNVLinux -DUNIX -I $(PFM_INCLUDE)

        LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

    .c.o:
	$(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~
	mv $(TGT) $(PFM_LIB)
	cp llz.h $(PFM_INCLUDE)

    endif


else


    ARFLAGS=r

    CC = gcc

    OS := $(shell uname)

    CFLAGS = -ansi -O -Wall -c -DNVWIN3X -I $(PFM_INCLUDE)


    .c.o:
	$(CC) -c $(CFLAGS) -I$(PFM_INCLUDE) $*.c


    TGT = libllz.a


    $(TGT):	$(TGT)($(OBJS))
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp llz.h $(PFM_INCLUDE)

endif



llz.o: 	llz.h
