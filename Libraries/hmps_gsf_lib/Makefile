ARFLAGS=r

CFLAGS = -O -ansi -Wall -I $(PFM_INCLUDE)

CC=gcc

.c.o:
	$(CC) -c $(CFLAGS) $*.c

libhmpsgsf.a:	libhmpsgsf.a(check_flag.o) \
		libhmpsgsf.a(hmpsflag.o)  \
		libhmpsgsf.a(hmpsparm.o)  \
		libhmpsgsf.a(hmpsutil.o)

	cp libhmpsgsf.a $(PFM_LIB)
	cp hmpsflag.h hmpsparm.h hmpsutil.h check_flag.h $(PFM_INCLUDE)


libhmpsgsf.a(hmpsflag.o):   hmpsflag.h
libhmpsgsf.a(hmpsparm.o):   hmpsflag.h
libhmpsgsf.a(hmpsutil.o):   hmpsflag.h snptime.h snputil.h
libhmpsgsf.a(check_flag.o): check_flag.h hmpsflag.h

