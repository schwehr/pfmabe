#
# MOSAIC_IO Makefile
#

MOSAIC_INCLUDE    = .
UNISIPS_INCLUDE   = ../unisips_io
BYTE_SWAP_INCLUDE = ../byteswap

include makeflags.$(MACHINE)

OBJS             = mosaic_io.o mosaic_funcs.o

all:  mosaic_io.a

clean: 
	rm -f *.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c 


mosaic_io.a:  $(OBJS)
	$(AR) $(ARFLAGS) mosaic_io.a $(OBJS) 

mosaic_io.o:  mosaic_io.h
