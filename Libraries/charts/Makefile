OS := $(shell uname)


OBJS = swap_NV.o hof_io.o tof_io.o image_io.o wave_io.o pos_io.o charts_cvtime.o gps_io.o rms_io.o


ifeq ($(OS),Linux)

    ifeq ($(PFM_STATIC),YES)

        ARFLAGS=r

        CC = gcc

        CFLAGS = -ansi -Wall -c -D_LARGEFILE64_SOURCE -DNVLinux -DUNIX -I $(PFM_INCLUDE)

        .c.o:
	    $(CC) -c $(CFLAGS) $*.c

        TGT = libCHARTS.a

        $(TGT):	$(TGT)($(OBJS))
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp charts.h FileHydroOutput.h FilePOSOutput.h FileWave.h FileImage.h FileRMSOutput.h hof_errors.h $(PFM_INCLUDE)
	cp FileGPSOutput.h FileLAS.h FileTopoOutput.h warns.h $(PFM_INCLUDE)

    else

        CC = gcc

        LINKER = gcc

        TGT = libCHARTS.so

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
	cp charts.h FileHydroOutput.h FilePOSOutput.h FileWave.h FileImage.h FileRMSOutput.h hof_errors.h $(PFM_INCLUDE)
	cp FileGPSOutput.h FileLAS.h FileTopoOutput.h warns.h $(PFM_INCLUDE)

    endif


else


    ARFLAGS=r

    CC = gcc

    OS := $(shell uname)

    CFLAGS = -ansi -O -Wall -c -DNVWIN3X -I $(PFM_INCLUDE)


    .c.o:
	$(CC) -c $(CFLAGS) -I$(PFM_INCLUDE) $*.c


    TGT = libCHARTS.a


    $(TGT):	$(TGT)($(OBJS))
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp charts.h FileHydroOutput.h FilePOSOutput.h FileWave.h FileImage.h FileRMSOutput.h hof_errors.h $(PFM_INCLUDE)
	cp FileGPSOutput.h FileLAS.h FileTopoOutput.h warns.h $(PFM_INCLUDE)

endif



hof_io.o:  		FileHydroOutput.h charts.h

tof_io.o:  		FileTopoOutput.h charts.h

image_io.o:  		FileImage.h charts.h

wave_io.o:  		FileWave.h charts.h

pos_io.o:  		FilePOSOutput.h charts.h

charts_cvtime.o:	charts.h

gps_io.o:  		FileGPSOutput.h charts.h

rms_io.o:  		FileRMSOutput.h charts.h
