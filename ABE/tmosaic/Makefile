CPPFLAGS = -c -g -O3 -Wno-deprecated -I $(PFM_INCLUDE)

all: mosaic

mosaic: mosaic.o params.o rgb_image.o algebra3.o process_mgr.o
	g++ -o mosaic mosaic.o params.o rgb_image.o algebra3.o process_mgr.o \
	    -L $(PFM_LIB) -lgdal -llsq -lproj -lxerces-c -ldl

mosaic.o: mosaic.cpp image.t rgb_image.h params.h algebra3.h process_mgr.h

params.o: params.cpp params.h

rgb_image.o: rgb_image.cpp rgb_image.h image.t params.h

algebra3.o: algebra3.cpp algebra3.h

process_mgr.o: process_mgr.cpp process_mgr.h
