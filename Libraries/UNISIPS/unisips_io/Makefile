#
# UNISIPS_IO make file (New UNISIPS Format)
#

include makeflags.$(MACHINE)


UNISIPS_INCLUDE   = .
BYTE_SWAP_INCLUDE = ../byteswap

OBJS= unisips_io.o

all:  unisips_io.a

clean: 
	rm -f *.o

unisips_io.o:
	$(CC) $(CFLAGS) -c $*.c  

unisips_io.a:  $(OBJS)
	$(AR) $(ARFLAGS) unisips_io.a $(OBJS) 

unisips_io.a(unisips_io.o):  unisips_io.h
