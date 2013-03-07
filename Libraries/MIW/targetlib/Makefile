ARFLAGS=r


OS := $(shell uname)


ifeq ($(OS),Linux)

    CFLAGS = -O -ansi -Wall
    CXXFLAGS = -O -ansi -Wall

else

    CFLAGS = -O -Wall -DXML_LIBRARY
    CXXFLAGS = -O -Wall -DXML_LIBRARY

endif


.c.o:
	$(CC) -c $(CFLAGS) -I$(PFM_INCLUDE) $*.c

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -I$(PFM_INCLUDE) -o $@ $<



libstarget.a : 	libstarget.a(CNCTDocument.o) libstarget.a(CNCTErrorHandler.o)\
	        libstarget.a(initialize.o) libstarget.a(MIWFuncs.o) \
		libstarget.a(find_schema.o)
