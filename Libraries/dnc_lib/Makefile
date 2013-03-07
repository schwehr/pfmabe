#  DNC Library Makefile
#  Linux Systems
#

OS := $(shell uname)

ifeq ($(OS),Linux)

    CFLAGS = -O -ansi -Wall -I $(PFM_INCLUDE) -DNVLinux

else

    CFLAGS = -O -ansi -Wall -I $(PFM_INCLUDE) -DXML_LIBRARY -DNVWIN3X

endif


ARFLAGS=r

CC = gcc

.c.o:
	$(CC) -c $(CFLAGS) $*.c

libdnc.a : 	libdnc.a(dnc_general.o) libdnc.a(edge.o) libdnc.a(dht.o) libdnc.a(reefa_aft.o) libdnc.a(coaarea_aft.o)\
	        libdnc.a(entity.o) libdnc.a(hydline_lft.o) libdnc.a(soundp_pft.o) libdnc.a(face.o) libdnc.a(coaline_lft.o)\
	        libdnc.a(table_header.o) libdnc.a(tileref_aft.o) libdnc.a(variable_index.o) libdnc.a(ring.o)\
		libdnc.a(obstruction_pft.o) libdnc.a(char_vdt.o) libdnc.a(navaid_pft.o) libdnc.a(coapoint_pft.o)\
		libdnc.a(coastl_lft.o) libdnc.a(int_vdt.o) libdnc.a(ecrarea_aft.o) libdnc.a(hydrol_lft.o)\
		libdnc.a(hydrol_ljt.o) libdnc.a(lat.o) libdnc.a(tod2_dht.o) libdnc.a(hazarda_aft.o)\
		libdnc.a(dangera_aft.o) libdnc.a(obstruca_aft.o) libdnc.a(hazardl_lft.o) libdnc.a(pipelinel_lft.o)\
		libdnc.a(tunnell_lft.o)

	cp libdnc.a $(PFM_LIB)
