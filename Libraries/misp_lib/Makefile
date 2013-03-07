#  MISP Library Makefile
#

OS := $(shell uname)

CC = gcc

LINKER = gcc

OBJS =  misp_funcs.o bit_funcs.o corner.o edge.o interpolate.o misp_iterate.o loadsparse.o misps.o merge.o merge_grid.o min_curve.o readsparse.o \
	shift.o sortdata.o spline.o spline_cof.o weight_mean.o


ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -O -ansi -Wall -c -DNVLinux -I $(PFM_INCLUDE)

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libmisp.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp misp.h $(PFM_INCLUDE)

    else

        TGT = libmisp.so

        MAKEFILE = Makefile

        CFLAGS = -fPIC -O -ansi -Wall -c -DNVLinux -I $(PFM_INCLUDE)

        LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

    .c.o:
	$(CC) $(CFLAGS) $*.c

all: $(TGT)
{-c $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~

	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp misp.h $(PFM_INCLUDE)

    endif


else


    ARFLAGS=r

    CC = gcc

    OS := $(shell uname)

    CFLAGS = -ansi -O -Wall -c -DNVWIN3X -I $(PFM_INCLUDE)

    .c.o:
	$(CC) -c $(CFLAGS) -I$(PFM_INCLUDE) $*.c


    TGT = libmisp.a


    $(TGT):	$(TGT)($(OBJS))
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp misp.h $(PFM_INCLUDE)

endif


misp_funcs.o :	  misp.h

corner.o :        misp.h

edge.o :          misp.h

interpolate.o :   misp.h

misp_iterate.o :  misp.h

loadsparse.o :    misp.h

merge.o :         misp.h

merge_grid.o :    misp.h

min_curve.o :     misp.h

misps.o :	  misp.h

readsparse.o :    misp.h

reorder.o :	  misp.h

sortdata.o :      misp.h

spline.o :        misp.h

spline_cof.o :    misp.h
