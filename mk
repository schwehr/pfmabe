#!/bin/bash


###########################################################################################################
#                                                                                                         #
#                                             IMPORTANT NOTE                                              #
#                                                                                                         #
#   Almost all of the Windows libraries have been built as static libraries to avoid a problem with       #
#   linking from one shared library to another (serious brain damage on the part of Micro$oft).           #
#   Due to this, all libraries and applications that link against the Xerces library must be compiled     #
#   with the -DXML_LIBRARY option.  To understand why we're doing this you can Google(TM) for             #
#   __declspec and MinGW.  The best explanation I have seen is at:                                        #
#                                                                                                         #
#                             http://www.haible.de/bruno/woe32dll.html                                    #
#                                                                                                         #
###########################################################################################################


#
#   The following is a pretty slick piece of work that changes the window title.  Unfortunately it
#   does absolutely nothing when run in an MSYS terminal but on Linux it's very helpful.
#

#   Set terminal title
#   @param string $1  Tab/window title
#   @param string $2  (optional) Separate window title
#   The latest version of this software can be obtained here:
#   See: http://fvue.nl/wiki/NameTerminal

function nameTerminal() {
    [ "${TERM:0:5}" = "xterm" ]   && local ansiNrTab=0
    [ "$TERM"       = "rxvt" ]    && local ansiNrTab=61
    [ "$TERM"       = "konsole" ] && local ansiNrTab=30 ansiNrWindow=0
        # Change tab title
    [ $ansiNrTab ] && echo -n $'\e'"]$ansiNrTab;$1"$'\a'
        # If terminal support separate window title, change window title as well
    [ $ansiNrWindow -a "$2" ] && echo -n $'\e'"]$ansiNrWindow;$2"$'\a'
} # nameTerminal()


SYS=`uname -s`


function Usage ()
{
    echo
    echo "Usage: `basename $0` [-h] [-p] [-l] [-6] [-a] [-c] [-s] [Destination directory]"
    echo
    echo "Where:"
    echo
    echo "    -h = Print this help text"
    echo "    -p = Skip build of the PFM libraries"
    echo "    -l = Only build the libraries"
    echo "    -6 = Build version 6 of the PFM library"
    echo "    -a = Build only the Area-Based Editor programs"
    echo "    -c = Build only the CZMIL Manual Editor programs"
    echo "    -s = Build all libraries statically"
    echo
    echo "    Destination directory = Optional destination directory for lib,"
    echo "                            include, and bin (defaults to /usr/local)."
    echo "                            The directory will be created if it doesn't"
    echo "                            exist."
    echo
    exit -1
}


SKIP_PFM="NO"
ONLY_LIBRARIES="NO"
CURRENT_PFM=5
CZMIL="NO"
ONLY_ABE="NO"
PFM_STATIC="NO"


while getopts ":hpl6acs" OPTION; do
    case $OPTION in
        h) Usage;;
        p) SKIP_PFM="YES";;
        l) ONLY_LIBRARIES="YES";;
        6) CURRENT_PFM=6;;
        a) ONLY_ABE="YES";;
        c) CZMIL="YES";;
        s) PFM_STATIC="YES";;
        *) echo;echo "Invalid option!";Usage;;
    esac
done


export PFM_STATIC


#  Export the CZMIL define for the version.hpp strings.

if [ $CZMIL = "YES" ]; then
    export CZMIL_DEF=OPTECH_CZMIL
    ONLY_ABE="YES"
else
    export CZMIL_DEF=""
fi


#  Decrement the argument pointer so it points to next argument.
#  $1 now references the first non option item supplied on the command line
#  if one exists.

shift $(($OPTIND - 1))


CHECK64="x86_32"
if [ $SYS = "Linux" ]; then
    CHECK64=`uname -m`
else

    #  First check the architecture

    C64=`echo $PROCESSOR_IDENTIFIER | cut -d" " -f 1 | grep 64`

    if [ $C64 ]; then

        #  Then make sure we have mingw64

	CHECK_MINGW64=`which x86_64-w64-mingw32-gcc.exe 2>&1 | grep unknown | cut -d: -f1`
	if [ ! $CHECK_MINGW64 ]; then
	    CHECK64=x86_64
	fi
    fi
fi

echo
echo
echo
echo "###############################################################################"
echo "# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #"
echo "###############################################################################"
echo "# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #"
echo "###############################################################################"
echo "#                                                                             #"
if [ $PFM_STATIC = "YES" ]; then
echo "           Building PFM_ABE with static libraries in "$CHECK64" mode           "
else
echo "           Building PFM_ABE with dynamic libraries in "$CHECK64" mode          "
fi
echo "#                                                                             #"
echo "###############################################################################"
echo "# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #"
echo "###############################################################################"
echo "# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #"
echo "###############################################################################"
echo
echo
echo
sleep 5


#  Check to see if the NAVO directory is present.  If it's not there we're going to set ONLY_ABE to YES.
#  The reason for this is that we distribute the Area-Based Editor without the NAVO specific code.

if [ ! -d NAVO ]; then
    ONLY_ABE="YES"
fi


export PFM_ABE_DEV=${1:-"/usr/local"}

export PFM_BIN=$PFM_ABE_DEV/bin
export PFM_LIB=$PFM_ABE_DEV/lib
export PFM_INCLUDE=$PFM_ABE_DEV/include
export PFM_SHARE=$PFM_ABE_DEV/share

if ! [ -d $PFM_ABE_DEV ]; then
    mkdir $PFM_ABE_DEV
fi

if ! [ -d $PFM_BIN ]; then
    mkdir $PFM_BIN
    chmod 755 $PFM_BIN
fi

if ! [ -d $PFM_LIB ]; then
    mkdir $PFM_LIB
    chmod 755 $PFM_LIB
fi

if ! [ -d $PFM_INCLUDE ]; then
    mkdir $PFM_INCLUDE
    chmod 755 $PFM_INCLUDE
fi


#  Set up the program documentation directory.

export PFM_DOC=$PWD/Documentation
if ! [ -d $PFM_DOC/APIs_and_Programs ]; then
    mkdir $PFM_DOC/APIs_and_Programs
fi


#  Set up a couple of things for Windoze

if [ $SYS != "Linux" ]; then


    #  This should always be set to release (debug is for experts only ;-)

    export WINMAKE=release


    #  Make sure we get the correct compiler in Windows

    export CC=gcc


    #  Make sure qmake knows what environment to use.

    export QMAKESPEC=win32-g++
fi


#  For some unknown reason, when I build on Windoze, some of the mk files 
#  permissions get hosed.

find . -name mk -exec chmod 755 {} \;
chmod 755 cleanit


#  Try to get the Qt distribution from the tar file in FOSS_libraries

QTDIST=`ls FOSS_libraries/qt-*.tar.gz | cut -d- -f5 | cut -dt -f1 | cut -d. --complement -f4`
export QTDIST

QT_TOP=Trolltech/Qt-$QTDIST
export QT_TOP

mkdir $PFM_ABE_DEV/Trolltech 2>/dev/null

QTDIR=$PFM_ABE_DEV/$QT_TOP
export QTDIR
PATH=$QTDIR/bin:$PATH
export PATH

export LD_LIBRARY_PATH=$PFM_LIB:$QTDIR/lib:$LD_LIBRARY_PATH


echo
echo
echo "Installing all PFM_ABE software in $PFM_ABE_DEV"
echo
echo


echo
echo
echo "###############################################################################"
echo "#                             FOSS Libraries                                  #"
echo "###############################################################################"
echo
echo


if [ $PFM_STATIC = "YES" ]; then
    BLOCK_NAME="Static FOSS Libraries : "
else
    BLOCK_NAME="Dynamic FOSS Libraries : "
fi


cd FOSS_libraries

#  Copy spacereplace and ipcclean to the bin directory

cp spacereplace $PFM_BIN

if [ $SYS = "Linux" ]; then
    cp ipcclean $PFM_BIN
fi


#  Check for Xerces library
#  Builds using MSYS make in Windoze

if [ -a $PFM_LIB/libxerces-c.so.28 ] || [ -a $PFM_LIB/libxerces-c.a ]; then
    echo
    echo "***************************************************"
    echo Skipping build of Xerces library
    echo "***************************************************"
    echo
else
    tar -xzf xerces-c-src_2_8_0.tar.gz
    cd xerces-c-src_2_8_0
    export XERCESCROOT=$PWD
    cd $XERCESCROOT/src/xercesc


    #  Check for x86_64 system

    if [ $CHECK64 = "x86_64" ]; then
        echo
        echo "***************************************************"
        echo "Building x86_64 Xerces library"
        nameTerminal "$BLOCK_NAME""Building x86_64 Xerces library"
        echo "***************************************************"
        echo

        if [ $SYS = "Linux" ]; then

            if [ $PFM_STATIC = "YES" ]; then
                ./runConfigure -plinux -cgcc -xg++ -minmem -nsocket -tnative -rnone -s -b64 -P$PFM_ABE_DEV
            else
                ./runConfigure -plinux -cgcc -xg++ -minmem -nsocket -tnative -rpthread -b64 -P$PFM_ABE_DEV
            fi

	else

	    #  Quick fix for a 64 bit bug (I hope it works ;-)

	    cp ../../../mingw/HashPtr.cpp util
	    cp ../../../mingw/XSerializeEngine.cpp internal


            ./runConfigure -pmingw-msys -cgcc -xg++ -nfileonly -rnone -s -b64 -z"-DXML_LIBRARY" -P$PFM_ABE_DEV

	fi

    else

	echo
	echo "***************************************************"
	echo "Building x86_32 Xerces library"
	nameTerminal "$BLOCK_NAME""Building x86_32 Xerces library"
	echo "***************************************************"
	echo

        if [ $SYS = "Linux" ]; then

            if [ $PFM_STATIC = "YES" ]; then
                ./runConfigure -plinux -cgcc -xg++ -minmem -nsocket -tnative -rnone -s -b32 -P$PFM_ABE_DEV
            else
                ./runConfigure -plinux -cgcc -xg++ -minmem -nsocket -tnative -rpthread -b32 -P$PFM_ABE_DEV
            fi

        else 

            ./runConfigure -pmingw-msys -cgcc -xg++ -nfileonly -rnone -s -z"-DXML_LIBRARY" -P$PFM_ABE_DEV

        fi
    fi

    make
    if [ $? != 0 ];then
        echo
        echo "Error building Xerces library, terminating"
        echo
        exit
    fi
    make install
    if [ $? != 0 ];then
        echo
        echo "Error installing Xerces library, terminating"
        echo
        exit
    fi
    cd ../../..
    find . -name xerces-c-src_\* -type d -maxdepth 1 -exec rm -rf {} \; 2>/dev/null
fi



###########################################################################################################
#   If we can ever move to 3.1.1 it will look like this:
###########################################################################################################

#if [ -a $PFM_LIB/libxerces-c-3.1.so ] || [ -a $PFM_LIB/libxerces-c.a ]; then
#    echo
#    echo "***************************************************"
#    echo Skipping build of Xerces library
#    echo "***************************************************"
#    echo
#else
#    echo
#    echo "***************************************************"
#    echo "Building Xerces library"
#    nameTerminal "$BLOCK_NAME""Building Xerces library"
#    echo "***************************************************"
#    echo
#
#
#    tar -xzf xerces-c-3.1.1.tar.gz
#    cd xerces-c-3.1.1
#
#
#    if [ $SYS = "Linux" ]; then
#
#        if [ $PFM_STATIC = "YES" ]; then
#            ./configure --prefix=$PFM_ABE_DEV --disable-shared
#        else
#            ./configure --prefix=$PFM_ABE_DEV
#        fi
#
#    else 
#
#        ./configure --prefix=$PFM_ABE_DEV --disable-shared --disable-threads --enable-transcode-windows
#
#    fi
#
#    make
#    if [ $? != 0 ];then
#        echo
#        echo "Error building Xerces library, terminating"
#        echo
#        exit
#    fi
#    make install
#    if [ $? != 0 ];then
#        echo
#        echo "Error installing Xerces library, terminating"
#        echo
#        exit
#    fi
#    cd ..
#    rm -rf xerces-c-3.1.1
#fi
###########################################################################################################



#  Check for leastsquares library (only for Linux at the moment).

if [ $SYS = "Linux" ]; then
    if [ -a $PFM_LIB/liblsq.so ] || [ -a $PFM_LIB/liblsq.a ]; then
        echo
        echo "***************************************************"
        echo "Skipping build of LSQ library"
        echo "***************************************************"
        echo
    else
        echo
        echo "***************************************************"
        echo "Building LSQ library"
        nameTerminal "$BLOCK_NAME""Building LSQ library"
        echo "***************************************************"
        echo
        tar -xzf leastsquares.tgz
        cd leastsquares

        if [ $PFM_STATIC = "YES" ]; then
            make -f Makefile.static
            make -f Makefile.static install
        else
            make -f Makefile.shared
            make -f Makefile.shared install
        fi


        cd ..
        rm -rf leastsquares
    fi
fi


#  Check for Meschach library.

if [ -a $PFM_LIB/meschach.a ]; then
    echo
    echo "***************************************************"
    echo "Skipping build of Meschach library"
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building Meschach library"
    nameTerminal "$BLOCK_NAME""Building Meschach library"
    echo "***************************************************"
    echo
    tar -xzf meschach-1.2.tar.gz
    cd meschach-1.2

    export CC=gcc
    ./configure --prefix=$PFM_ABE_DEV --with-complex --with-sparse --with-unroll --with-munroll


    #  This works just fine on Windows using MinGW (32 or 64)

    cp MACHINES/Linux/machine.h .


    make
    mv meschach.a $PFM_LIB
    cp *.h $PFM_INCLUDE

    cd ..
    rm -rf meschach-1.2
fi


#  Check for SQLITE library.

if [ -a $PFM_LIB/libsqlite3.a ]; then
    echo
    echo "***************************************************"
    echo "Skipping build of SQLITE library"
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building SQLITE library"
    nameTerminal "$BLOCK_NAME""Building SQLITE library"
    echo "***************************************************"
    echo
    tar -xzf sqlite-amalgamation-3.6.23.1.tar.gz
    cd sqlite-3.6.23.1

    if [ $SYS = "Linux" ]; then

        if [ $PFM_STATIC = "YES" ]; then
            ./configure --prefix=$PFM_ABE_DEV --enable-shared=no
        else
            ./configure --prefix=$PFM_ABE_DEV
        fi

    else

        ./configure --prefix=$PFM_ABE_DEV --enable-shared=no

    fi

    make
    make install

    cd ..
    rm -rf sqlite-3.6.23.1
fi


#  Check for shapefile library
#  Builds using MSYS make in Windoze

if [ -a $PFM_LIB/libshp.so ] || [ -a $PFM_LIB/libshp.a ]; then
    echo
    echo "***************************************************"
    echo Skipping build of shape library
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building shape library"
    nameTerminal "$BLOCK_NAME""Building shape library"
    echo "***************************************************"
    echo

    tar -xzf shapelib-1.2.10_modified.tar.gz
    cd shapelib-1.2.10

    if [ $SYS = "Linux" ]; then
        make lib
        if [ $? != 0 ];then
            echo
            echo "Error building shape library, terminating"
            echo
            exit
        fi
        make lib_install
        if [ $? != 0 ];then
            echo
            echo "Error installing shape library, terminating"
            echo
            exit
        fi


        #  Remove the shared libs if we're building static

        if [ $PFM_STATIC = "YES" ]; then
            rm $PFM_LIB/libshp.so*
        fi


    else

        make -f Makefile.MinGW
        if [ $? != 0 ];then
            echo
            echo "Error building shape library, terminating"
            echo
            exit
        fi
        cp libshp.a $PFM_LIB
        mkdir $PFM_INCLUDE/libshp 2>/dev/null
        cp shapefil.h $PFM_INCLUDE/libshp

    fi

    cd ..
    rm -rf shapelib-1.2.10
fi




#  Check for proj.4 library.

if [ -a $PFM_LIB/libproj.so ] || [ -a $PFM_LIB/libproj.a ]; then
    echo
    echo "***************************************************"
    echo Skipping build of proj-4 library
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building proj-4 library"
    nameTerminal "$BLOCK_NAME""Building proj-4 library"
    echo "***************************************************"
    echo
    tar -xzf proj-4.7.0.tar.gz
    cd proj-4.7.0

    if [ $SYS = "Linux" ]; then

        if [ $PFM_STATIC = "YES" ]; then
            ./configure --prefix=$PFM_ABE_DEV --disable-shared
        else
            ./configure --prefix=$PFM_ABE_DEV
        fi

    else

        ./configure --prefix=$PFM_ABE_DEV --without-mutex --disable-shared


        #  Replace src/pj_mutex.c with file modified for MinGW

        cp ../mingw/proj4_modified_pj_mutex.c src/pj_mutex.c
    fi

    make
    if [ $? != 0 ];then
        echo
        echo "Error building proj-4 library, terminating"
        echo
        exit
    fi
    make install
    if [ $? != 0 ];then
        echo
        echo "Error installing proj-4 library, terminating"
        echo
        exit
    fi
    cd ..
    rm -rf proj-4.7.0
fi



#  Check for zlib library

if [ -a $PFM_LIB/libz.a ] || [ -a $PFM_LIB/libz.so ]; then
    echo
    echo "***************************************************"
    echo Skipping build of zlib library
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building zlib library"
    nameTerminal "$BLOCK_NAME""Building zlib library"
    echo "***************************************************"
    echo
    tar -xzf zlib-1.2.5.tar.gz
    cd zlib-1.2.5

    if [ $SYS = "Linux" ]; then

        ./configure --prefix=$PFM_ABE_DEV

        make

        if [ $? != 0 ];then
            echo
            echo "Error installing zlib library, terminating"
            echo
            exit
        fi

        make install

        if [ $? != 0 ];then
            echo
            echo "Error installing zlib library, terminating"
            echo
            exit
        fi

    else

        make -f win32/Makefile.gcc BINARY_PATH=$PFM_ABE_DEV/bin INCLUDE_PATH=$PFM_ABE_DEV/include LIBRARY_PATH=$PFM_ABE_DEV/lib install

        if [ $? != 0 ];then
            echo
            echo "Error installing zlib library, terminating"
            echo
            exit
        fi

    fi

    cd ..
    rm -rf zlib-1.2.5
fi



#  Check for jpeg library (static build only - for static Qt)

#if [ $PFM_STATIC = "YES" ]; then
#    if [ -a $PFM_LIB/libjpeg.a ]; then
#	echo
#	echo "***************************************************"
#	echo Skipping build of jpeg library
#	echo "***************************************************"
#	echo
#    else
#	echo
#	echo "***************************************************"
#	echo "Building jpeg library"
#	nameTerminal "$BLOCK_NAME""Building jpeg library"
#	echo "***************************************************"
#	echo
#	tar -xzf jpegsrc.v8c.tar.gz
#	cd jpeg-8c

#	./configure --prefix=$PFM_ABE_DEV --disable-shared

#        make
#        if [ $? != 0 ];then
#            echo
#            echo "Error building jpeg library, terminating"
#            echo
#            exit
#        fi
#        make install
#        if [ $? != 0 ];then
#            echo
#            echo "Error installing jpeg library, terminating"
#            echo
#            exit
#        fi

#	cd ..
#	rm -rf jpeg-8c
#    fi
#fi




#  Check for BEECRYPT library

if [ -a $PFM_LIB/libbeecrypt.a ] || [ -a $PFM_ABE_DEV/lib64/libbeecrypt.a ] || [ -a $PFM_LIB/libbeecrypt.a ]; then
    echo
    echo "***************************************************"
    echo Skipping build of beecrypt library
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building beecrypt library"
    nameTerminal "$BLOCK_NAME""Building beecrypt library"
    echo "***************************************************"
    echo

    tar -xzf beecrypt-4.2.1_modified.tar.gz
    cd beecrypt-4.2.1


    if [ $SYS = "Linux" ]; then

        if [ $PFM_STATIC = "YES" ]; then
            ./configure --prefix=$PFM_ABE_DEV --without-java --without-python --enable-shared=no
        else
            ./configure --prefix=$PFM_ABE_DEV --without-java --without-python
        fi

    else

        if [ $CHECK64 = "x86_64" ]; then

            #  Using --with-arch=x86_64 doesn't work but this does.

            SAVE_CFLAGS=$CFLAGS
            export CFLAGS=-m64
            ./configure --prefix=$PFM_ABE_DEV --without-java --without-python --enable-threads=no --enable-shared=no
            export CFLAGS=$SAVE_CFLAGS
        else
            ./configure --prefix=$PFM_ABE_DEV --without-java --without-python --enable-threads=no --enable-shared=no
        fi
    fi


    make
    if [ $? != 0 ];then
        echo
        echo "Error building beecrypt library, terminating"
        echo
        exit
    fi
    make install
    if [ $? != 0 ];then
        echo
        echo "Error installing beecrypt library, terminating"
        echo
        exit
    fi

    cd ..
    rm -rf beecrypt-4.2.1


    #  Check for x86_64 system (copy lib64 libraries to the normal place)

    if [ $CHECK64 = "x86_64" ] && [ $SYS == "Linux" ] && [ $PFM_STATIC != "YES" ]; then
        cp $PFM_ABE_DEV/lib64/* $PFM_ABE_DEV/lib
    fi
fi


#  Check for HDF5 library

if [ -a $PFM_LIB/libhdf5.so ] || [ -a $PFM_LIB/libhdf5.a ]; then
    echo
    echo "***************************************************"
    echo Skipping build of hdf5 library
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building hdf5 library"
    nameTerminal "$BLOCK_NAME""Building hdf5 library"
    echo "***************************************************"
    echo

    tar -xzf hdf5-1.8.7.tar.gz
    cd hdf5-1.8.7


    if [ $SYS = "Linux" ]; then

        if [ $PFM_STATIC = "YES" ]; then
            ./configure --prefix=$PFM_ABE_DEV --enable-cxx --with-default-api-version=v16 --enable-shared=no
        else
            ./configure --prefix=$PFM_ABE_DEV --enable-cxx --with-default-api-version=v16
        fi

        make
        if [ $? != 0 ];then
            echo
            echo "Error building hdf5 library, terminating"
            echo
            exit
        fi
        make install
        if [ $? != 0 ];then
            echo
            echo "Error installing hdf5 library, terminating"
            echo
            exit
        fi

        cd ..

    else

	#  Replace H5win32defs.h to correct multiple definitions of struct timezone

	cp ../mingw/H5win32defs.h src


        #  Bug fixes so that the test suite will build.

	cp ../mingw/tfile.c test
	cp ../mingw/testhdf5.h test
	cp ../mingw/h5test.c test
	cp ../mingw/vfd.c test


        ./configure --prefix=$PFM_ABE_DEV --enable-cxx --enable-shared=no --with-default-api-version=v16

        make

        if [ $? != 0 ];then
            echo
            echo "Error building hdf5 library, terminating"
            echo
            exit
        fi

        make install

        if [ $? != 0 ];then
            echo
            echo "Error installing hdf5 library, terminating"
            echo
            exit
        fi

        cd ..

    fi

    rm -rf hdf5-1.8.7

fi



#  Check for gdal library

if [ -a $PFM_LIB/libgdal.a ]; then
    echo
    echo "***************************************************"
    echo Skipping build of gdal library
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building gdal library"
    nameTerminal "$BLOCK_NAME""Building gdal library"
    echo "***************************************************"
    echo

    GDAL_DIST=gdal-1.8.1

    tar -xzf $GDAL_DIST.tar.gz
    cd $GDAL_DIST

    if [ $SYS = "Linux" ]; then

        if [ $CHECK64 = "x86_64" ]; then

            if [ $PFM_STATIC = "YES" ]; then
                ./configure --prefix=$PFM_ABE_DEV --with-xerces-inc=$PFM_INCLUDE --with-xerces-lib=$PFM_LIB --with-libz=internal --with-libtiff=internal --without-geos --with-oci=no --with-geotiff=internal --without-jpeg --without-gif --without-grass --without-libgrass --without-cfitsio --without-pcraster --without-netcdf --without-png --without-ogdi --without-fme --without-hdf4 --without-hdf5 --without-jasper --without-ecw --without-kakadu --without-mrsid --without-jp2mrsid --without-bsb --without-grib --without-mysql --without-ingres --without-expat --without-odbc --without-curl --without-sqlite3 --without-dwgdirect --without-panorama --without-idb --without-sde --without-perl --without-php --without-ruby --without-python --without-ogpython --without-pg --without-pcidsk --with-threads=no --with-hide-internal-symbols --enable-shared=no --without-ld-shared
            else
                ./configure --prefix=$PFM_ABE_DEV --with-xerces-inc=$PFM_INCLUDE --with-xerces-lib=$PFM_LIB --with-libz=internal --with-libtiff=internal --without-geos --with-oci=no --with-geotiff=internal --without-jpeg --without-gif --without-grass --without-libgrass --without-cfitsio --without-pcraster --without-netcdf --without-png --without-ogdi --without-fme --without-hdf4 --without-hdf5 --without-jasper --without-ecw --without-kakadu --without-mrsid --without-jp2mrsid --without-bsb --without-grib --without-mysql --without-ingres --without-expat --without-odbc --without-curl --without-sqlite3 --without-dwgdirect --without-panorama --without-idb --without-sde --without-perl --without-php --without-ruby --without-python --without-ogpython --without-pg --without-pcidsk --with-threads=no --with-hide-internal-symbols
            fi

        else

            if [ $PFM_STATIC = "YES" ]; then
                ./configure --prefix=$PFM_ABE_DEV --with-xerces-inc=$PFM_INCLUDE --with-xerces-lib=$PFM_LIB --with-libz=internal --with-libtiff=internal --without-geos --with-oci=no --with-geotiff=internal --without-jpeg --without-gif --without-grass --without-libgrass --without-cfitsio --without-pcraster --without-netcdf --without-png --without-ogdi --without-fme --without-hdf4 --without-hdf5 --without-jasper --without-ecw --without-kakadu --without-mrsid --without-jp2mrsid --without-bsb --without-grib --without-mysql --without-ingres --without-expat --without-odbc --without-curl --without-sqlite3 --without-dwgdirect --without-panorama --without-idb --without-sde --without-perl --without-php --without-ruby --without-python --without-ogpython --without-pg --without-pcidsk --with-threads=no --with-hide-internal-symbols --enable-shared=no --without-ld-shared
            else
                ./configure --prefix=$PFM_ABE_DEV --with-xerces-inc=$PFM_INCLUDE --with-xerces-lib=$PFM_LIB --with-libz=internal --with-libtiff=internal --without-geos --with-oci=no --with-geotiff=internal --without-jpeg --without-gif --without-grass --without-libgrass --without-cfitsio --without-pcraster --without-netcdf --without-png --without-ogdi --without-fme --without-hdf4 --without-hdf5 --without-jasper --without-ecw --without-kakadu --without-mrsid --without-jp2mrsid --without-bsb --without-grib --without-mysql --without-ingres --without-expat --without-odbc --without-curl --without-sqlite3 --without-dwgdirect --without-panorama --without-idb --without-sde --without-perl --without-php --without-ruby --without-python --without-ogpython --without-pg --without-pcidsk --with-threads=no --with-hide-internal-symbols
            fi

        fi

        make
        if [ $? != 0 ];then
            echo
            echo "Error building gdal library, terminating"
            echo
            exit
        fi
        make install
        if [ $? != 0 ];then
            echo
            echo "Error installing gdal library, terminating"
            echo
            exit
        fi
    else

        if [ $CHECK64 = "x86_64" ]; then

	    #  Quick fix for a 64 bit bug (it was assuming MSVC)

	    cp ../mingw/cpl_vsil_win32.cpp port


	    ./configure --prefix=$PFM_ABE_DEV --with-xerces-inc=$PFM_INCLUDE --with-xerces-lib=$PFM_LIB --host=i686-pc-mingw32 --with-libz=internal --with-libtiff=internal --without-geos --with-oci=no --with-geotiff=internal --without-jpeg --without-gif --without-grass --without-libgrass --without-cfitsio --without-pcraster --without-netcdf --without-png --without-ogdi --without-fme --without-hdf4 --without-hdf5 --without-jasper --without-ecw --without-kakadu --without-mrsid --without-jp2mrsid --without-bsb --without-grib --without-mysql --without-ingres --without-expat --without-odbc --without-curl --without-sqlite3 --without-dwgdirect --without-panorama --without-idb --without-sde --without-perl --without-php --without-ruby --without-python --without-ogpython --without-pg --without-pcidsk --with-threads=no --with-hide-internal-symbols --enable-shared=no --without-ld-shared
	else
	    ./configure --prefix=$PFM_ABE_DEV --with-xerces-inc=$PFM_INCLUDE --with-xerces-lib=$PFM_LIB --host=i686-pc-mingw32 --with-libz=internal --with-libtiff=internal --without-geos --with-oci=no --with-geotiff=internal --without-jpeg --without-gif --without-grass --without-libgrass --without-cfitsio --without-pcraster --without-netcdf --without-png --without-ogdi --without-fme --without-hdf4 --without-hdf5 --without-jasper --without-ecw --without-kakadu --without-mrsid --without-jp2mrsid --without-bsb --without-grib --without-mysql --without-ingres --without-expat --without-odbc --without-curl --without-sqlite3 --without-dwgdirect --without-panorama --without-idb --without-sde --without-perl --without-php --without-ruby --without-python --without-ogpython --without-pg --without-pcidsk --with-threads=no --with-hide-internal-symbols --enable-shared=no --without-ld-shared
	fi


        #  Now we have to edit GNUmakefile and change $(GDAL_ROOT) to . so that the ar command line won't be too long (causing "Bad file number" error).

        sed "s/\$(GDAL_ROOT)/./g" GNUmakefile >tmp.h
        cp tmp.h GNUmakefile

        make
        if [ $? != 0 ];then
            echo
            echo "Error building gdal library, terminating"
            echo
            exit
        fi
        make install
        if [ $? != 0 ];then
            echo
            echo "Error installing gdal library, terminating"
            echo
            exit
        fi
    fi

    cd ..
    rm -rf $GDAL_DIST
fi



#  Don't check for OpenNS library.  Due to the rapid development of BAG we want to rebuild it every time.

#if [ -a $PFM_LIB/libbag.a ]; then
#    echo
#    echo "***************************************************"
#    echo "Skipping build of OpenNS library"
#    echo "***************************************************"
#    echo
#else
    echo
    echo "***************************************************"
    echo "Building OpenNS library"
    nameTerminal "$BLOCK_NAME""Building OpenNS library"
    echo "***************************************************"
    echo


    #  The version of beecrypt used for OpenNS is OOOOLLLLLDDDDD.  The new version of Beecrypt places all of its .h files in a directory
    #  called beecrypt in the include directory (default = /usr/local/include).  The old version just slapped them into the default include 
    #  directory.  We're going to copy them up a level and leave them in the beecrypt directory because Beecrypt itself expects them to
    #  be there.  If it wasn't for Windoze I'd just link the directory.

    cp $PFM_INCLUDE/beecrypt/* $PFM_INCLUDE 2>&1 >/dev/null


    tar -xzf OpenNS_Release_1.4.0.tar.gz
    cd OpenNS_Release_1.4.0/api

    make -f Makefile_generic

    cp -r ../configdata $PFM_SHARE

    cd ../..
    rm -rf OpenNS_Release_1.4.0
#fi



#  Check for liblas

if [ -a $PFM_LIB/liblas.so ] || [ -a $PFM_LIB/liblas.a ]; then
    echo
    echo "***************************************************"
    echo Skipping build of liblas
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building liblas library"
    nameTerminal "$BLOCK_NAME""Building liblas library"
    echo "***************************************************"
    echo


    tar -xjf liblas-src-1.2.1_modified.tar.bz2
    cd liblas-src-1.2.1

    if [ $SYS = "Linux" ]; then

        if [ $PFM_STATIC = "YES" ]; then
            SAVE_LIBS=$LIBS
            export LIBS=-lstdc++
            ./configure --prefix=$PFM_ABE_DEV --with-gdal=no --disable-shared
            export LIBS=$SAVE_LIBS
        else
            ./configure --prefix=$PFM_ABE_DEV --with-gdal=no
        fi

    else

        SAVE_LIBS=$LIBS
        export LIBS=-lstdc++
        ./configure --prefix=$PFM_ABE_DEV --disable-shared
        export LIBS=$SAVE_LIBS

    fi

    make
    make install


    #  For some unknown reason the capi include files don't get put in the include directory on 32 bit linux (????)

    cp -r include/liblas/capi $PFM_ABE_DEV/include/liblas
    cp -r include/liblas/detail $PFM_ABE_DEV/include/liblas


    cd ..
    rm -rf liblas-src-1.2.1
fi


#  Check for Qt

if [ -a $QTDIR/lib/libQtCore.a ] || [ -a $QTDIR/lib/libQtCore.so ] || [ -a $QTDIR/bin/QtCore4.dll ]; then
    echo
    echo "***************************************************"
    echo Skipping build of Qt $QTDIST library
    echo "***************************************************"
    echo
else
    echo
    echo "***************************************************"
    echo "Building Qt $QTDIST library"
    nameTerminal "$BLOCK_NAME""Building Qt $QTDIST library"
    echo "***************************************************"
    echo

    if [ $SYS = "Linux" ]; then

        tar -xzf qt-everywhere-opensource-src-$QTDIST.tar.gz
        cd qt-everywhere-opensource-src-$QTDIST


        #  Static linking.

        if [ $PFM_STATIC = "YES" ]; then

            ./configure --prefix=$PFM_ABE_DEV/$QT_TOP -static -opensource -confirm-license -qt-zlib -qt-gif -qt-libtiff -qt-libpng -qt-libmng -qt-libjpeg -qt-sql-sqlite -no-qt3support -largefile -opengl -no-sql-mysql -no-sql-psql -no-webkit -no-script -no-exceptions -no-phonon -nomake examples -nomake demos -I $PFM_ABE_DEV/include -L $PFM_ABE_DEV/lib

            make sub-src
            if [ $? != 0 ];then
                echo
                echo "Error building Qt $QTDIST library, terminating"
                echo
                exit
            fi


        #  Shared linking.

        else

            ./configure --prefix=$PFM_ABE_DEV/$QT_TOP -opensource -confirm-license -qt-gif -qt-sql-sqlite -no-qt3support -largefile -opengl -no-sql-mysql -no-sql-psql -no-webkit -no-exceptions -no-phonon -nomake examples -nomake demos

            make
            if [ $? != 0 ];then
                echo
                echo "Error building Qt $QTDIST library, terminating"
                echo
                exit
            fi
        fi

        make install
        if [ $? != 0 ];then
            echo
            echo "Error installing Qt $QTDIST library, terminating"
            echo
            exit
        fi


        #  Get the plugins and other goodies and put them in the "bin" directory.

        cp -r $PFM_ABE_DEV/$QT_TOP/plugins/* $PFM_BIN 2>/dev/null


        cd ..
        rm -rf qt-everywhere-opensource-src-$QTDIST
    else

        #  Force the proper shell for building Qt under MinGW

        export QMAKE_SH=sh


        #  Save this location so we can come back to it

        export COMEBACK=$PWD

        cd $PFM_ABE_DEV/Trolltech

        tar -xzf $COMEBACK/qt-everywhere-opensource-src-$QTDIST.tar.gz
        mv qt-everywhere-opensource-src-$QTDIST Qt-$QTDIST
        cd Qt-$QTDIST


        #  64 bit fix for Qt 4.7.1/2 (http://bugreports.qt.nokia.com/browse/QTBUG-15585)

        if [ $CHECK64 = "x86_64" ]; then
	    cp $COMEBACK/mingw/qsimd.cpp src/corelib/tools
	fi


	#  Forcing it to not make the demos or examples by removing the examples and demos directories.  We don't need them
	#  and -nomake examples and -nomake demos doesn't work as far as I can tell.  This will generate errors but we really 
	#  don't care as long as the libraries get built.

	#rm -rf examples demos


        #  Static linking.

        if [ $PFM_STATIC = "YES" ]; then


            #  Static fix for Qt 4.7.2 (http://bugreports.qt.nokia.com/browse/QTBUG-18184)

            cp $COMEBACK/mingw/qpaintengineex_opengl2.cpp src/opengl/gl2paintengineex
            cp $COMEBACK/mingw/qglframebufferobject.cpp src/opengl
            cp $COMEBACK/mingw/qglpixelbuffer.cpp src/opengl
            cp $COMEBACK/mingw/qglpixmapfilter.cpp src/opengl
            cp $COMEBACK/mingw/qpaintengine_opengl.cpp src/opengl
            cp $COMEBACK/mingw/qpixmapdata_gl.cpp src/opengl
            cp $COMEBACK/mingw/qpaintengine_vg.cpp src/openvg
            cp $COMEBACK/mingw/qpixmapdata_vg.cpp src/openvg


            configure.exe -platform win32-g++ -release -static -no-exceptions -opensource -confirm-license -fast -qt-zlib -qt-gif -qt-libtiff -qt-libpng -qt-libmng -qt-libjpeg -qt-sql-sqlite -no-qt3support -no-accessibility -no-sql-mysql -no-sql-psql -no-webkit -no-script -no-phonon -nomake examples -nomake demos -L $PFM_ABE_DEV/lib

            mingw32-make


	    mingw32-make install


            #  Get the executables and put them in the bin directory

	    find bin ! -name \*.dll -a -type f -exec cp {} $PFM_BIN \;


            #  Get the libraries and put them in the lib directory

            cp -r lib/*.a $PFM_LIB

        else

            configure.exe -platform win32-g++ -release -disable-static -opensource -confirm-license -fast -qt-sql-sqlite -no-qt3support -no-accessibility -qt-gif -no-sql-mysql -no-sql-psql -no-webkit -no-phonon -nomake examples -nomake demos

            mingw32-make


	    mingw32-make install


            #  Get the executables and put them in the bin directory

	    find bin ! -name \*.dll -a -type f -exec cp {} $PFM_BIN \;


            #  Get the plugins and other goodies and put them in the "bin" directory.

            cp -r plugins/* $PFM_BIN 2>/dev/null


            #  Get the libraries and put them in the lib directory

            cp -r lib/*.dll $PFM_LIB

        fi


        cd $COMEBACK
    fi
fi



cd ..


#  Check to see if doxygen is available (and the right version) so we can document some of the programs and APIs

DOXYTEST=`which doxygen 2>/dev/null`
if [ -x "$DOXYTEST" ]; then
    DOXYVERSION=`doxygen --version`

    if [ $DOXYVERSION != "1.6.1" ]; then
        DOXYTEST="NOT_A_FREAKING_COMMAND"
    fi
fi


#  Skip PFM libraries build?

if [ $SKIP_PFM != "YES" ]; then

    cd Libraries

    echo
    echo
    echo "###############################################################################"
    echo "#                           PFM_ABE Libraries                                 #"
    echo "###############################################################################"
    echo
    echo


    if [ $PFM_STATIC = "YES" ]; then
        BLOCK_NAME="Static PFM ABE Libraries:"
    else
        BLOCK_NAME="Dynamic PFM ABE Libraries:"
    fi


    #  Always build pfm_lib first followed by utility.  Too many things depend upon it.
    #  Everything else is in alphabetical order.

    echo
    echo "***************************************************"
    echo "Building PFM library"
    nameTerminal "$BLOCK_NAME""Building PFM library"
    echo "***************************************************"
    echo
    cd pfm_lib_$CURRENT_PFM
    make
    if [ $? != 0 ];then
        echo
        echo "Error building PFM library, terminating"
        echo
        exit
    fi

    #  Run doxygen to generate the programmer documentation (and only if it's there)

    if [ -x "$DOXYTEST" ]; then
        rm -rf $PFM_DOC/APIs_and_Programs/PFM_API_Documentation
        mkdir PFM_API_Documentation
        doxygen 2>&1>/dev/null
        cp PFM.png PFM_API_Documentation/html
        mv PFM_API_Documentation $PFM_DOC/APIs_and_Programs
    fi


    echo
    echo "***************************************************"
    echo "Building utility library"
    nameTerminal "$BLOCK_NAME""Building utility library"
    echo "***************************************************"
    echo
    cd ../utility
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building utility library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building BAG_XML_META library"
    nameTerminal "$BLOCK_NAME""Building BAG_XML_META library"
    echo "***************************************************"
    echo
    cd ../bag_xml_meta_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building BAG_XML_META library, terminating"
        echo
        exit
    fi

    #  Run doxygen to generate the programmer documentation (and only if it's there)

    if [ -x "$DOXYTEST" ]; then
        rm -rf $PFM_DOC/APIs_and_Programs/BAG_XML_META_API_Documentation
        mkdir BAG_XML_META_API_Documentation
        doxygen 2>&1>/dev/null
        mv BAG_XML_META_API_Documentation $PFM_DOC/APIs_and_Programs
    fi


    echo
    echo "***************************************************"
    echo "Building BFD library"
    nameTerminal "$BLOCK_NAME""Building BFD library"
    echo "***************************************************"
    echo
    cd ../bfd_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building BFD library, terminating"
        echo
        exit
    fi


    #  Run doxygen to generate the programmer documentation (and only if it's there)

    if [ -x "$DOXYTEST" ]; then
        rm -rf $PFM_DOC/APIs_and_Programs/BFD_API_Documentation
        mkdir BFD_API_Documentation
        doxygen 2>&1>/dev/null
        mv BFD_API_Documentation $PFM_DOC/APIs_and_Programs
    fi


    #  If we're building statically on Windows we want to strip the definitions of __declspec from binaryFeatureData.h.
    #  This way all subsequent programs that need this library won't try to import from a non-existent DLL.

    if [ $SYS != "Linux" ] && [ $PFM_STATIC = "YES" ]; then
	sed "s/__declspec(dllexport)//g" binaryFeatureData.h >tmp.h
	sed "s/__declspec(dllimport)//g" tmp.h >tmp2.h
	cp tmp2.h $PFM_INCLUDE/binaryFeatureData.h
	rm tmp.h tmp2.h
    fi


    echo
    echo "***************************************************"
    echo "Building CHARTS library"
    nameTerminal "$BLOCK_NAME""Building CHARTS library"
    echo "***************************************************"
    echo
    cd ../charts
    make
    if [ $? != 0 ];then
        echo
        echo "Error building CHARTS library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building CHRTR2 library"
    nameTerminal "$BLOCK_NAME""Building CHRTR2 library"
    echo "***************************************************"
    echo
    cd ../chrtr2_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building CHRTR2 library, terminating"
        echo
        exit
    fi


    #  Doing this here so I can keep the CHRTR2 library standalone

    if [ $SYS = "Linux" ]; then
        if [ $PFM_STATIC = "YES" ]; then
            mv libchrtr2.a $PFM_LIB
        else
            mv libchrtr2.so $PFM_LIB
        fi
        cp chrtr2.h $PFM_INCLUDE
    else
        if [ $PFM_STATIC = "YES" ]; then
	    cp libchrtr2.a $PFM_LIB
	    rm libchrtr2.a


            #  If we're building statically on Windows we want to strip the definitions of __declspec from chrtr2.h.
            #  This way all subsequent programs that need this library won't try to import from a non-existent DLL.

	    sed "s/__declspec(dllexport)//g" chrtr2.h >tmp.h
	    sed "s/__declspec(dllimport)//g" tmp.h >tmp2.h
	    cp tmp2.h $PFM_INCLUDE/chrtr2.h
	    rm tmp.h tmp2.h
        else
	    cp libchrtr2.dll $PFM_LIB
	    rm libchrtr2.dll
	    cp chrtr2.h $PFM_INCLUDE
	fi
    fi
    cp chrtr2_shared.h chrtr2_macros.h chrtr2_nvtypes.h $PFM_INCLUDE


    #  Run doxygen to generate the programmer documentation (and only if it's there)

    if [ -x "$DOXYTEST" ]; then
        rm -rf $PFM_DOC/APIs_and_Programs/CHRTR2_API_Documentation
        mkdir CHRTR2_API_Documentation
        doxygen 2>&1>/dev/null
        mv CHRTR2_API_Documentation $PFM_DOC/APIs_and_Programs
    fi


    echo
    echo "***************************************************"
    echo "Building CZMIL library"
    nameTerminal "$BLOCK_NAME""Building CZMIL library"
    echo "***************************************************"
    echo
    cd ../czmil
    make
    if [ $? != 0 ];then
        echo
        echo "Error building CZMIL library, terminating"
        echo
        exit
    fi


    #  Doing this here so I can keep the CZMIL library standalone

    if [ $SYS = "Linux" ]; then
        if [ $PFM_STATIC = "YES" ]; then
            mv libCZMIL.a $PFM_LIB
        else
            mv libCZMIL.so $PFM_LIB
        fi
        cp czmil.h $PFM_INCLUDE
    else
        if [ $PFM_STATIC = "YES" ]; then
	    cp libCZMIL.a $PFM_LIB
	    rm libCZMIL.a


            #  If we're building statically on Windows we want to strip the definitions of __declspec from czmil.h.
            #  This way all subsequent programs that need this library won't try to import from a non-existent DLL.

	    sed "s/__declspec(dllexport)//g" czmil.h >tmp.h
	    sed "s/__declspec(dllimport)//g" tmp.h >tmp2.h
	    cp tmp2.h $PFM_INCLUDE/czmil.h
	    rm tmp.h tmp2.h
	else
	    cp libCZMIL.dll $PFM_LIB
	    rm libCZMIL.dll
	    cp czmil.h $PFM_INCLUDE
	fi
    fi
    cp czmil_nvtypes.h czmil_macros.h $PFM_INCLUDE


    #  Run doxygen to generate the programmer documentation (and only if it's there)

    if [ -x "$DOXYTEST" ]; then
        rm -rf $PFM_DOC/APIs_and_Programs/CZMIL_API_Documentation
        mkdir CZMIL_API_Documentation
        doxygen 2>&1>/dev/null
        mv CZMIL_API_Documentation $PFM_DOC/APIs_and_Programs
    fi


    echo
    echo "***************************************************"
    echo "Building DNC library"
    nameTerminal "$BLOCK_NAME""Building DNC library"
    echo "***************************************************"
    echo
    cd ../dnc_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building DNC library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building GSF library"
    nameTerminal "$BLOCK_NAME""Building GSF library"
    echo "***************************************************"
    echo
    cd ../gsf
    make
    if [ $? != 0 ];then
        echo
        echo "Error building GSF library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building Hawkeye library"
    nameTerminal "$BLOCK_NAME""Building Hawkeye library"
    echo "***************************************************"
    echo
    cd ../hawkeye_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building Hawkeye library, terminating"
        echo
        exit
    fi


    #  Doing this here so I can keep the Hawkeye library standalone

    if [ $SYS = "Linux" ]; then
        if [ $PFM_STATIC = "YES" ]; then
            mv libhawkeye.a $PFM_LIB
        else
            mv libhawkeye.so $PFM_LIB
        fi
        cp hawkeye.h $PFM_INCLUDE
    else
        if [ $PFM_STATIC = "YES" ]; then
	    cp libhawkeye.a $PFM_LIB
	    rm libhawkeye.a


            #  If we're building statically on Windows we want to strip the definitions of __declspec from hawkeye.h.
            #  This way all subsequent programs that need this library won't try to import from a non-existent DLL.

	    sed "s/__declspec(dllexport)//g" hawkeye.h >tmp.h
	    sed "s/__declspec(dllimport)//g" tmp.h >tmp2.h
	    cp tmp2.h $PFM_INCLUDE/hawkeye.h
	    rm tmp.h tmp2.h
	else
	    cp libhawkeye.dll $PFM_LIB
	    rm libhawkeye.dll
	    cp hawkeye.h $PFM_INCLUDE
	fi
    fi
    cp hawkeye_nvtypes.h $PFM_INCLUDE


    echo
    echo "***************************************************"
    echo "Building HMPS GSF library"
    nameTerminal "$BLOCK_NAME""Building HMPS GSF library"
    echo "***************************************************"
    echo
    cd ../hmps_gsf_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building HMPS GSF library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building LLZ library"
    nameTerminal "$BLOCK_NAME""Building LLZ library"
    echo "***************************************************"
    echo
    cd ../llz_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building LLZ library, terminating"
        echo
        exit
    fi


    #  Run doxygen to generate the programmer documentation (and only if it's there)

    if [ -x "$DOXYTEST" ]; then
        rm -rf $PFM_DOC/APIs_and_Programs/LLZ_API_Documentation
        mkdir LLZ_API_Documentation
        doxygen 2>&1>/dev/null
        mv LLZ_API_Documentation $PFM_DOC/APIs_and_Programs
    fi


    echo
    echo "***************************************************"
    echo "Building MISP library"
    nameTerminal "$BLOCK_NAME""Building MISP library"
    echo "***************************************************"
    echo
    cd ../misp_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building MISP library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building MIW library"
    nameTerminal "$BLOCK_NAME""Building MIW library"
    echo "***************************************************"
    echo
    cd ../MIW
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building MIW library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfmWDB library"
    nameTerminal "$BLOCK_NAME""Building pfmWDB library"
    echo "***************************************************"
    echo
    cd ../pfmWDB_lib
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfmWDB library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building SHOALS library"
    nameTerminal "$BLOCK_NAME""Building SHOALS library"
    echo "***************************************************"
    echo
    cd ../shoals
    make
    if [ $? != 0 ];then
        echo
        echo "Error building SHOALS library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building SRTM library"
    nameTerminal "$BLOCK_NAME""Building SRTM library"
    echo "***************************************************"
    echo
    cd ../srtm_mask
    make
    if [ $? != 0 ];then
        echo
        echo "Error building SRTM library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building UNISIPS library"
    nameTerminal "$BLOCK_NAME""Building UNISIPS library"
    echo "***************************************************"
    echo
    cd ../UNISIPS
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building UNISIPS library, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building WLF library"
    nameTerminal "$BLOCK_NAME""Building WLF library"
    echo "***************************************************"
    echo
    cd ../wlf_lib
    make
    if [ $? != 0 ];then
        echo
        echo "Error building WLF library, terminating"
        echo
        exit
    fi

    #  Doing this here so I can keep the WLF library standalone

    if [ $SYS = "Linux" ]; then
        if [ $PFM_STATIC = "YES" ]; then
            mv libwlf.a $PFM_LIB
        else
            mv libwlf.so $PFM_LIB
        fi
        cp wlf.h $PFM_INCLUDE
    else
        if [ $PFM_STATIC = "YES" ]; then
	    cp libwlf.a $PFM_LIB
	    rm libwlf.a


            #  If we're building statically on Windows we want to strip the definitions of __declspec from wlf.h.
            #  This way all subsequent programs that need this library won't try to import from a non-existent DLL.

	    sed "s/__declspec(dllexport)//g" wlf.h >tmp.h
	    sed "s/__declspec(dllimport)//g" tmp.h >tmp2.h
	    cp tmp2.h $PFM_INCLUDE/wlf.h
	    rm tmp.h tmp2.h
	else
	    cp libwlf.dll $PFM_LIB
	    rm libwlf.dll
	    cp wlf.h $PFM_INCLUDE
	fi
    fi
    cp wlf_class.h $PFM_INCLUDE
    cp wlf_nvtypes.h $PFM_INCLUDE


    if [ $ONLY_LIBRARIES = "YES" ]; then
        echo
        echo "***************************************************"
        echo Finished building libraries
        echo "***************************************************"
        echo
        exit
    fi

    cd ../..
fi



echo
echo
echo "###############################################################################"
echo "#                               PFM_ABE Programs                              #"
echo "###############################################################################"
echo
echo


if [ $PFM_STATIC = "YES" ]; then
    BLOCK_NAME="Static build PFM ABE Programs : "
else
    BLOCK_NAME="Dynamic build PFM ABE Programs : "
fi


cd ABE


#  Check to see if we need to force a rebuild because we are switching between CZMIL and PFM version names.

CME_SEARCH=""
if [ $SYS = "Linux" ]; then 
    if [ -a $PFM_BIN/pfmView ]; then
	CME_SEARCH=`grep CME $PFM_BIN/pfmView | cut -d" " -f3` 2>/dev/null
    fi
else
    if [ -a $PFM_BIN/pfmView.exe ]; then
	CME_SEARCH=`grep CME $PFM_BIN/pfmView.exe | cut -d" " -f3` 2>/dev/null
    fi
fi

if [ $CZMIL = "NO" ]; then
    if [ $CME_SEARCH ]; then
        find . -name \*version\* -exec touch {} \;
    fi
else
    if [ ! $CME_SEARCH ]; then
        find . -name \*version\* -exec touch {} \;
    fi
fi


echo
echo "***************************************************"
echo "Building abe"
nameTerminal "$BLOCK_NAME""Building abe"
echo "***************************************************"
echo
cd abe
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building abe, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building areaCheck"
nameTerminal "$BLOCK_NAME""Building areaCheck"
echo "***************************************************"
echo
cd ../areaCheck
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building areaCheck, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building attributeViewer"
nameTerminal "$BLOCK_NAME""Building attributeViewer"
echo "***************************************************"
echo
cd ../attributeViewer
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building attributeViewer, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building bagViewer"
nameTerminal "$BLOCK_NAME""Building bagViewer"
echo "***************************************************"
echo
cd ../bagViewer
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building bagViewer, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building chartsLAS"
nameTerminal "$BLOCK_NAME""Building chartsLAS"
echo "***************************************************"
echo
cd ../chartsLAS
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building chartsLAS, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building charts2LAS"
nameTerminal "$BLOCK_NAME""Building charts2LAS"
echo "***************************************************"
echo
cd ../charts2LAS
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building charts2LAS, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building chartsPic"
nameTerminal "$BLOCK_NAME""Building chartsPic"
echo "***************************************************"
echo
cd ../chartsPic
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building chartsPic, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building CZMILwaveMonitor"
nameTerminal "$BLOCK_NAME""Building CZMILwaveMonitor"
echo "***************************************************"
echo
cd ../CZMILwaveMonitor
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building CZMILwaveMonitor, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building examGSF"
nameTerminal "$BLOCK_NAME""Building examGSF"
echo "***************************************************"
echo
cd ../examGSF
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building examGSF, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building featurePic"
nameTerminal "$BLOCK_NAME""Building featurePic"
echo "***************************************************"
echo
cd ../featurePic
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building featurePic, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building geoSwath3D"
nameTerminal "$BLOCK_NAME""Building geoSwath3D"
echo "***************************************************"
echo
cd ../geoSwath3D
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building geoSwath3D, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building gsfMonitor"
nameTerminal "$BLOCK_NAME""Building gsfMonitor"
echo "***************************************************"
echo
cd ../gsfMonitor
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building gsfMonitor, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building hawkeyeMonitor"
nameTerminal "$BLOCK_NAME""Building hawkeyeMonitor"
echo "***************************************************"
echo
cd ../hawkeyeMonitor
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building hawkeyeMonitor, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building hofReturnKill"
nameTerminal "$BLOCK_NAME""Building hofReturnKill"
echo "***************************************************"
echo
cd ../hofReturnKill
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building hofReturnKill, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building hofWaveFilter"
nameTerminal "$BLOCK_NAME""Building hofWaveFilter"
echo "***************************************************"
echo
cd ../hofWaveFilter
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building hofWaveFilter, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building lidarMonitor"
nameTerminal "$BLOCK_NAME""Building lidarMonitor"
echo "***************************************************"
echo
cd ../lidarMonitor
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building lidarMonitor, terminating"
    echo
    exit
fi


#  This only builds on Linux at the moment

if [ $SYS == "Linux" ]; then
    echo
    echo "***************************************************"
    echo "Building mosaic"
    nameTerminal "$BLOCK_NAME""Building mosaic"
    echo "***************************************************"
    echo
    cd ../tmosaic
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building mosaic, terminating"
        echo
        exit
    fi
fi


echo
echo "***************************************************"
echo "Building mosaicView"
nameTerminal "$BLOCK_NAME""Building mosaicView"
echo "***************************************************"
echo
cd ../mosaicView
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building mosaicView, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfm3D"
nameTerminal "$BLOCK_NAME""Building pfm3D"
echo "***************************************************"
echo
cd ../pfm3D
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfm3D, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmBag"
nameTerminal "$BLOCK_NAME""Building pfmBag"
echo "***************************************************"
echo
cd ../pfmBag
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmBag, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmChartsImage"
nameTerminal "$BLOCK_NAME""Building pfmChartsImage"
echo "***************************************************"
echo
cd ../pfmChartsImage
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmChartsImage, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfm_charts_image"
nameTerminal "$BLOCK_NAME""Building pfm_charts_image"
echo "***************************************************"
echo
cd ../pfm_charts_image
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfm_charts_image, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmEdit"
nameTerminal "$BLOCK_NAME""Building pfmEdit"
echo "***************************************************"
echo
cd ../pfmEdit
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmEdit, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmEdit3D"
nameTerminal "$BLOCK_NAME""Building pfmEdit3D"
echo "***************************************************"
echo
cd ../pfmEdit3D
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmEdit3D, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmEditShell"
nameTerminal "$BLOCK_NAME""Building pfmEditShell"
echo "***************************************************"
echo
cd ../pfmEditShell
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmEditShell, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmExtract"
nameTerminal "$BLOCK_NAME""Building pfmExtract"
echo "***************************************************"
echo
cd ../pfmExtract
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmExtract, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmFeature"
nameTerminal "$BLOCK_NAME""Building pfmFeature"
echo "***************************************************"
echo
cd ../pfmFeature
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmFeature, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmGeotiff"
nameTerminal "$BLOCK_NAME""Building pfmGeotiff"
echo "***************************************************"
echo
cd ../pfmGeotiff
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmGeotiff, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmLoad"
nameTerminal "$BLOCK_NAME""Building pfmLoad"
echo "***************************************************"
echo
cd ../pfmLoad
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmLoad, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmLoader"
nameTerminal "$BLOCK_NAME""Building pfmLoader"
echo "***************************************************"
echo
cd ../pfmLoader
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmLoader, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmMisp"
nameTerminal "$BLOCK_NAME""Building pfmMisp"
echo "***************************************************"
echo
cd ../pfmMisp
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmMisp, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfm_recompute"
nameTerminal "$BLOCK_NAME""Building pfm_recompute"
echo "***************************************************"
echo
cd ../pfm_recompute
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfm_recompute, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfm_unload"
nameTerminal "$BLOCK_NAME""Building pfm_unload"
echo "***************************************************"
echo
cd ../pfm_unload
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfm_unload, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building pfmView"
nameTerminal "$BLOCK_NAME""Building pfmView"
echo "***************************************************"
echo
cd ../pfmView
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building pfmView, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building rmsMonitor"
nameTerminal "$BLOCK_NAME""Building rmsMonitor"
echo "***************************************************"
echo
cd ../rmsMonitor
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building rmsMonitor, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building trackLine"
nameTerminal "$BLOCK_NAME""Building trackLine"
echo "***************************************************"
echo
cd ../trackLine
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building trackLine, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building waveformMonitor"
nameTerminal "$BLOCK_NAME""Building waveformMonitor"
echo "***************************************************"
echo
cd ../waveformMonitor
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building waveformMonitor, terminating"
    echo
    exit
fi


echo
echo "***************************************************"
echo "Building waveWaterfall"
nameTerminal "$BLOCK_NAME""Building waveWaterfall"
echo "***************************************************"
echo
cd ../waveWaterfall
sh mk
if [ $? != 0 ];then
    echo
    echo "Error building waveWaterfall, terminating"
    echo
    exit
fi

cd ../..



if [ $ONLY_ABE = "NO" ]; then


    echo
    echo
    echo "###############################################################################"
    echo "#                              NAVOCEANO Programs                             #"
    echo "###############################################################################"
    echo
    echo


    if [ $PFM_STATIC = "YES" ]; then
        BLOCK_NAME="Static build NAVOCEANO Programs : "
    else
        BLOCK_NAME="Dynamic build NAVOCEANO Programs : "
    fi


    cd NAVO


    echo
    echo "***************************************************"
    echo "Building apply_tides"
    nameTerminal "$BLOCK_NAME""Building apply_tides"
    echo "***************************************************"
    echo
    cd apply_tides
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building apply_tides, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building are2shp"
    nameTerminal "$BLOCK_NAME""Building are2shp"
    echo "***************************************************"
    echo
    cd ../are2shp
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building are2shp, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building axtgPlot"
    nameTerminal "$BLOCK_NAME""Building axtgPlot"
    echo "***************************************************"
    echo
    cd ../axtgPlot
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building axtgPlot, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building bagGeotiff"
    nameTerminal "$BLOCK_NAME""Building bagGeotiff"
    echo "***************************************************"
    echo
    cd ../bagGeotiff
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building bagGeotiff, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building bag_residual"
    nameTerminal "$BLOCK_NAME""Building bag_residual"
    echo "***************************************************"
    echo
    cd ../bag_residual
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building bag_residual, terminating"
        echo
        exit
    fi


    #  Only compile this if we have the Oracle precompiler on the system

    if [ -a $ORACLE_HOME/precomp/lib/env_precomp.mk ];then
        echo
        echo "***************************************************"
        echo "Building bathyQuery"
        nameTerminal "$BLOCK_NAME""Building bathyQuery"
        echo "***************************************************"
        echo
        cd ../bathyQuery
        sh mk
        if [ $? != 0 ];then
            echo
            echo "Error building bathyQuery"
            echo
        fi
    else
        echo
        echo "***************************************************"
        echo Not building bathyQuery
        echo No Oracle precompiler available
        echo "***************************************************"
        echo
    fi


    echo
    echo "***************************************************"
    echo "Building build_coast"
    nameTerminal "$BLOCK_NAME""Building build_coast"
    echo "***************************************************"
    echo
    cd ../build_coast
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building build_coast, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building bfd2miw"
    nameTerminal "$BLOCK_NAME""Building bfd2miw"
    echo "***************************************************"
    echo
    cd ../bfd2miw
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building bfd2miw, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building build_feature"
    nameTerminal "$BLOCK_NAME""Building build_feature"
    echo "***************************************************"
    echo
    cd ../build_feature
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building build_feature, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building build_tide_table"
    nameTerminal "$BLOCK_NAME""Building build_tide_table"
    echo "***************************************************"
    echo
    cd ../build_tide_table
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building build_tide_table, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building charts2wlf"
    nameTerminal "$BLOCK_NAME""Building charts2wlf"
    echo "***************************************************"
    echo
    cd ../charts2wlf
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building charts2wlf, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building charts_list"
    nameTerminal "$BLOCK_NAME""Building charts_list"
    echo "***************************************************"
    echo
    cd ../charts_list
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building charts_list, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building charts_security_tag"
    nameTerminal "$BLOCK_NAME""Building charts_security_tag"
    echo "***************************************************"
    echo
    cd ../charts_security_tag
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building charts_security_tag, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building chrtr"
    nameTerminal "$BLOCK_NAME""Building chrtr"
    echo "***************************************************"
    echo
    cd ../chrtr
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building chrtr, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building chrtr2"
    nameTerminal "$BLOCK_NAME""Building chrtr2"
    echo "***************************************************"
    echo
    cd ../chrtr2
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building chrtr2, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building chrtr2_filter5"
    nameTerminal "$BLOCK_NAME""Building chrtr2_filter5"
    echo "***************************************************"
    echo
    cd ../chrtr2_filter5
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building chrtr2_filter5, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building chrtr2Extract"
    nameTerminal "$BLOCK_NAME""Building chrtr2Extract"
    echo "***************************************************"
    echo
    cd ../chrtr2Extract
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building chrtr2Extract, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building chrtr2Mask"
    nameTerminal "$BLOCK_NAME""Building chrtr2Mask"
    echo "***************************************************"
    echo
    cd ../chrtr2Mask
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building chrtr2Mask, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building chrtr2_merge"
    nameTerminal "$BLOCK_NAME""Building chrtr2_merge"
    echo "***************************************************"
    echo
    cd ../chrtr2_merge
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building chrtr2_merge, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building chrtrGeotiff"
    nameTerminal "$BLOCK_NAME""Building chrtrGeotiff"
    echo "***************************************************"
    echo
    cd ../chrtrGeotiff
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building chrtrGeotiff, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building chrtrGUI"
    nameTerminal "$BLOCK_NAME""Building chrtrGUI"
    echo "***************************************************"
    echo
    cd ../chrtrGUI
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building chrtrGUI, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building czmil_list"
    nameTerminal "$BLOCK_NAME""Building czmil_list"
    echo "***************************************************"
    echo
    cd ../czmil_list
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building czmil_list, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building datum_shift"
    nameTerminal "$BLOCK_NAME""Building datum_shift"
    echo "***************************************************"
    echo
    cd ../datum_shift
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building datum_shift, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building datumSurface"
    nameTerminal "$BLOCK_NAME""Building datumSurface"
    echo "***************************************************"
    echo
    cd ../datumSurface
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building datumSurface, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building dump_charts_image"
    nameTerminal "$BLOCK_NAME""Building dump_charts_image"
    echo "***************************************************"
    echo
    cd ../dump_charts_image
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building dump_charts_image, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building dump_dnc"
    nameTerminal "$BLOCK_NAME""Building dump_dnc"
    echo "***************************************************"
    echo
    cd ../dump_dnc
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building dump_dnc, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building dump_dop"
    nameTerminal "$BLOCK_NAME""Building dump_dop"
    echo "***************************************************"
    echo
    cd ../dump_dop
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building dump_dop, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building dump_egm08"
    nameTerminal "$BLOCK_NAME""Building dump_egm08"
    echo "***************************************************"
    echo
    cd ../dump_egm08
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building dump_egm08, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building dump_pos"
    nameTerminal "$BLOCK_NAME""Building dump_pos"
    echo "***************************************************"
    echo
    cd ../dump_pos
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building dump_pos, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building dump_feature"
    nameTerminal "$BLOCK_NAME""Building dump_feature"
    echo "***************************************************"
    echo
    cd ../dump_feature
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building dump_feature, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building dump_tod2"
    nameTerminal "$BLOCK_NAME""Building dump_tod2"
    echo "***************************************************"
    echo
    cd ../dump_tod2
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building dump_tod2, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building dump_waveforms"
    nameTerminal "$BLOCK_NAME""Building dump_waveforms"
    echo "***************************************************"
    echo
    cd ../dump_waveforms
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building dump_waveforms, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building grid3D"
    nameTerminal "$BLOCK_NAME""Building grid3D"
    echo "***************************************************"
    echo
    cd ../grid3D
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building grid3D, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building gridEdit"
    nameTerminal "$BLOCK_NAME""Building gridEdit"
    echo "***************************************************"
    echo
    cd ../gridEdit
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building gridEdit, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building gridPointEdit"
    nameTerminal "$BLOCK_NAME""Building gridPointEdit"
    echo "***************************************************"
    echo
    cd ../gridPointEdit
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building gridPointEdit, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building gridPointEdit3D"
    nameTerminal "$BLOCK_NAME""Building gridPointEdit3D"
    echo "***************************************************"
    echo
    cd ../gridPointEdit3D
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building gridPointEdit3D, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building gsf_class"
    nameTerminal "$BLOCK_NAME""Building gsf_class"
    echo "***************************************************"
    echo
    cd ../gsf_class
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building gsf_class, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building gsf_filter"
    nameTerminal "$BLOCK_NAME""Building gsf_filter"
    echo "***************************************************"
    echo
    cd ../gsf_filter
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building gsf_filter, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building gsf_shift"
    nameTerminal "$BLOCK_NAME""Building gsf_shift"
    echo "***************************************************"
    echo
    cd ../gsf_shift
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building gsf_shift, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building gsf2shp"
    nameTerminal "$BLOCK_NAME""Building gsf2shp"
    echo "***************************************************"
    echo
    cd ../gsf2shp
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building gsf2shp, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building line_area"
    nameTerminal "$BLOCK_NAME""Building line_area"
    echo "***************************************************"
    echo
    cd ../line_area
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building line_area, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building miw2bfd"
    nameTerminal "$BLOCK_NAME""Building miw2bfd"
    echo "***************************************************"
    echo
    cd ../miw2bfd
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building miw2bfd, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building navo_pfm_cube"
    nameTerminal "$BLOCK_NAME""Building navo_pfm_cube"
    echo "***************************************************"
    echo
    cd ../navo_pfm_cube
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building navo_pfm_cube, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building navoABE"
    nameTerminal "$BLOCK_NAME""Building navoABE"
    echo "***************************************************"
    echo
    cd ../navoABE
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building navoABE, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building navTrack"
    nameTerminal "$BLOCK_NAME""Building navTrack"
    echo "***************************************************"
    echo
    cd ../navTrack
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building navTrack, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfm2chrtr2"
    nameTerminal "$BLOCK_NAME""Building pfm2chrtr2"
    echo "***************************************************"
    echo
    cd ../pfm2chrtr2
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfm2chrtr2, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfm2dted"
    nameTerminal "$BLOCK_NAME""Building pfm2dted"
    echo "***************************************************"
    echo
    cd ../pfm2dted
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfm2dted, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfm2rdp"
    nameTerminal "$BLOCK_NAME""Building pfm2rdp"
    echo "***************************************************"
    echo
    cd ../pfm2rdp
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfm2rdp, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfm_beamstats"
    nameTerminal "$BLOCK_NAME""Building pfm_beamstats"
    echo "***************************************************"
    echo
    cd ../pfm_beamstats
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfm_beamstats, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfmCube"
    nameTerminal "$BLOCK_NAME""Building pfmCube"
    echo "***************************************************"
    echo
    cd ../pfmCube
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfmCube, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfm_deconflict"
    nameTerminal "$BLOCK_NAME""Building pfm_deconflict"
    echo "***************************************************"
    echo
    cd ../pfm_deconflict
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfm_deconflict, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfm_gsf_resid"
    nameTerminal "$BLOCK_NAME""Building pfm_gsf_resid"
    echo "***************************************************"
    echo
    cd ../pfm_gsf_resid
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfm_gsf_resid, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfmMask"
    nameTerminal "$BLOCK_NAME""Building pfmMask"
    echo "***************************************************"
    echo
    cd ../pfmMask
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfmMask, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfm_residual"
    nameTerminal "$BLOCK_NAME""Building pfm_residual"
    echo "***************************************************"
    echo
    cd ../pfm_residual
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfm_residual, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfm_unedit"
    nameTerminal "$BLOCK_NAME""Building pfm_unedit"
    echo "***************************************************"
    echo
    cd ../pfm_unedit
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfm_unedit, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfmWDBCleanup"
    nameTerminal "$BLOCK_NAME""Building pfmWDBCleanup"
    echo "***************************************************"
    echo
    cd ../pfmWDBCleanup
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfmWDBCleanup, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfmWDBExtract"
    nameTerminal "$BLOCK_NAME""Building pfmWDBExtract"
    echo "***************************************************"
    echo
    cd ../pfmWDBExtract
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfmWDBExtract, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfmWDBLoad"
    nameTerminal "$BLOCK_NAME""Building pfmWDBLoad"
    echo "***************************************************"
    echo
    cd ../pfmWDBLoad
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfmWDBLoad, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfmWDBMaintenance"
    nameTerminal "$BLOCK_NAME""Building pfmWDBMaintenance"
    echo "***************************************************"
    echo
    cd ../pfmWDBMaintenance
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfmWDBMaintenance, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building pfmWDBView"
    nameTerminal "$BLOCK_NAME""Building pfmWDBView"
    echo "***************************************************"
    echo
    cd ../pfmWDBView
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building pfmWDBView, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building tdz2zne"
    nameTerminal "$BLOCK_NAME""Building tdz2zne"
    echo "***************************************************"
    echo
    cd ../tdz2zne
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building tdz2zne, terminating"
        echo
        exit
    fi


    echo
    echo "***************************************************"
    echo "Building xyz2llz"
    nameTerminal "$BLOCK_NAME""Building xyz2llz"
    echo "***************************************************"
    echo
    cd ../xyz2llz
    sh mk
    if [ $? != 0 ];then
        echo
        echo "Error building xyz2llz, terminating"
        echo
        exit
    fi

    cd ../..
fi


echo
echo
echo "###############################################################################"
echo "#                                 Cleanup                                     #"
echo "###############################################################################"
echo
echo


#  The release file is built by the make_new_edit_tgz script in the PFM_ABE directory
#  so that it is only updated when we make a new release not when we're testing.

echo
cat release
echo

cat >$PFM_BIN/pfm_release <<EOF
echo
echo
cat <<EOD
EOF
cat release >>$PFM_BIN/pfm_release
cat >>$PFM_BIN/pfm_release <<EOF
EOD
echo
echo
EOF

chmod 755 $PFM_BIN/pfm_release


#  Make a PFM_ABE_README.txt file for use when building installs with InstallJammer (or whatever)

cp release $PFM_DOC/PFM/PFM_ABE_README.txt
cat $PFM_DOC/PFM/abechange.txt >>$PFM_DOC/PFM/PFM_ABE_README.txt


#  Make a sample .pfm_cfg file and put it in the bin directory

cat >$PFM_BIN/.pfm_cfg <<EOF
#
#   Some clarification of the [SUBSTITUTE PATH] statement:
#
#   The [SUBSTITUTE PATH] statement will contain two or three path names separated by commas.
#   The first field is a Windows folder name such as X:\data1 or even just X:.  The second
#   and third fields will be UNIX directory names (the third field is not required).  These
#   fields are used to allow transparent file access on heterogeneous networks (those
#   containing both UNIX and Windows systems).  When a PFM strucure is opened the .pfm_cfg
#   file is read in.  The file may be located, in order, in the current working directory/folder,
#   the user's home directory, or anywhere in the PATH (first come first served).  [SUBSTITUTE PATH]
#   statements are read at that time.  If there are any [SUBSTITUTE PATH] statements then
#   all file names subsequently read from the list file (e.g. bin, index, mosaic, target, input)
#   will be compared against these statements.  If there is a match then the paths will be replaced
#   with the matching path from the statement.  A normal [SUBSTITUTE PATH] statement might look like
#   this:
#
#   [SUBSTITUTE PATH] = X:,/net/alh-pogo1/data3/datasets,/data3/datasets
#
#   On UNIX systems, the first of the two UNIX fields will have precedence over the second.  For
#   this reason it is best to place a networked path name (name containing /net or /.automount)
#   first since it will usually contain the entirety of the non-networked path name.  In addition,
#   the networked name will work on all systems, even, in this case, alh-pogo1.  Obviously, if
#   you place them the other way 'round, you could end up with some very interesting path
#   names ;-)  Make sure that you don't have conflicting substitute paths in the .pfm_cfg file.
#   The first match will take precedence.
#
EOF


#  JALBTCX setup - On altura2 (CentOS 5 x86_64), evildead (CentOS 6 x86_64), EVILLIVE (Windows 7), alh-pogo4/5 (Fedora x86),
#  alh-pogo6 (CentOS x86), and ALH-ALIEN1 (Windows XP) we want to make a working .pfm_cfg

NODENAME=`uname -n`

if [ $NODENAME = "altura1.jalbtcx.lan" ] || [ $NODENAME = "evildead.jalbtcx.lan" ] || [ $NODENAME = "EVILLIVE" ] || [ $NODENAME = "alh-pogo4" ] || [ $NODENAME = "alh-pogo5" ] || [ $NODENAME = "alh-pogo6" ] || [ $NODENAME = "ALH-ALIEN1" ]; then
    cat >>$PFM_BIN/.pfm_cfg <<EOF
[SUBSTITUTE PATH] = O:,/net/coe/cold/cold,/coe/cold
[SUBSTITUTE PATH] = Q:,/net/alh-pogo3/pogo3,/pogo3
[SUBSTITUTE PATH] = R:,/net/alh-pogo4/pogo4,/pogo4
[SUBSTITUTE PATH] = S:,/net/alh-pogo5/pogo5,/pogo5
[SUBSTITUTE PATH] = T:,/net/alh-pogo6/pogo6,/pogo6
[SUBSTITUTE PATH] = U:,/net/alh-pogo7/pogo7,/pogo7
[SUBSTITUTE PATH] = V:,/net/alh-pogo8/pogo8,/pogo8
EOF
fi


chmod 644 $PFM_BIN/.pfm_cfg


rm -rf $PFM_ABE_DEV/doc/PFM 2>/dev/null
mkdir $PFM_ABE_DEV/doc 2>/dev/null
cp -r $PFM_DOC/PFM $PFM_ABE_DEV/doc


#  For some unknown reason, when I build on Windoze, some of the mk files 
#  permissions get hosed.

find . -name mk -exec chmod 755 {} \;
chmod 755 cleanit


#  Defeating the evil STIG forces

chmod 755 $PFM_ABE_DEV

chmod 755 $PFM_BIN
chmod 755 $PFM_LIB
chmod 755 $PFM_INCLUDE

chmod 755 $PFM_BIN/*
find $PFM_LIB -type d -exec chmod 755 {} \;
find $PFM_LIB -type f -exec chmod 755 {} \;
find $PFM_INCLUDE -type d -exec chmod 755 {} \;
find $PFM_INCLUDE -type f -exec chmod 755 {} \;
find $PFM_ABE_DEV/doc -type d -exec chmod 755 {} \;
find $PFM_ABE_DEV/doc -type f -exec chmod 644 {} \;


#  On Windows we want to remove the Qt debug libraries if we built in "release" mode.
#  We also don't need QtDesigner libraries.  We don't really need the gdal, hdf5, las,
#  Qt executables and a bunch of other stuff either so we're going to remove those.
#  All together, this saves us about 400MB of space in the installer.

if [ $SYS != "Linux" ]; then
    if [ $WINMAKE != "debug" ]; then
	rm $PFM_LIB/Qt*d4.dll 2>/dev/null
    fi

    rm $PFM_LIB/QtDesigner* 2>/dev/null
    rm $PFM_BIN/designer* $PFM_BIN/assistant.exe $PFM_BIN/lconvert.exe $PFM_BIN/moc.exe 2>/dev/null
    rm $PFM_BIN/gdal*.exe $PFM_BIN/*h5* $PFM_BIN/las*.exe $PFM_BIN/idc.exe $PFM_BIN/qt*.exe 2>/dev/null
    rm $PFM_BIN/qcollect*.exe $PFM_BIN/qdoc3.exe $PFM_BIN/qhelp*.exe $PFM_BIN/qmake.exe 2>/dev/null
    rm $PFM_BIN/linguist.exe $PFM_BIN/nearblack.exe $PFM_BIN/lrelease.exe $PFM_BIN/lupdate.exe 2>/dev/null
    rm $PFM_BIN/ogr*.exe $PFM_BIN/nad*.exe $PFM_BIN/pixeltool.exe $PFM_BIN/proj.exe $PFM_BIN/rcc.exe 2>/dev/null
    rm $PFM_BIN/rdjpg*.exe $PFM_BIN/testepsg.exe $PFM_BIN/uic.exe 2>/dev/null
else

#  Get rid of some of the same stuff on Linux.

    rm $PFM_BIN/gdal* $PFM_BIN/*h5* $PFM_BIN/las* $PFM_BIN/nearblack $PFM_BIN/nad* 2>/dev/null
    rm $PFM_BIN/ogr* $PFM_BIN/proj $PFM_BIN/invproj $PFM_BIN/rdjpg* $PFM_BIN/testepsg 2>/dev/null
fi
