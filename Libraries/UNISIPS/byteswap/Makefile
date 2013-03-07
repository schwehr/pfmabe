#
# Byteswap Makefile
#


include makeflags.$(MACHINE)

OBJS= bytes2func.o

all:
	@make clean
	@make byteswap.a
	@cp byteswap.a $(PFM_INCLUDE)
	@echo " "

byteswap.a: $(OBJS)
		$(AR) $(ARFLAGS) byteswap.a $(OBJS)

bytes2func.o:	
	$(CC) $(CFLAGS) -c $*.c
clean:
	rm -f *.a


