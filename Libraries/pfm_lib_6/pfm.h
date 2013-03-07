
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



  /*! \mainpage Pure FIle Magic (PFM) API

       <br><br>\section disclaimer Disclaimer

       This is a work of the US Government. In accordance with 17 USC 105, copyright
       protection is not available for any work of the US Government.

       Neither the United States Government nor any employees of the United States
       Government, makes any warranty, express or implied, without even the implied
       warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes
       any liability or responsibility for the accuracy, completeness, or usefulness
       of any information, apparatus, product, or process disclosed, or represents
       that its use would not infringe privately-owned rights. Reference herein to
       any specific commercial products, process, or service by trade name, trademark,
       manufacturer, or otherwise, does not necessarily constitute or imply its
       endorsement, recommendation, or favoring by the United States Government. The
       views and opinions of authors expressed herein do not necessarily state or
       reflect those of the United States Government, and shall not be used for
       advertising or product endorsement purposes.


       <br><br>\section date Introduction

       This library provides all of the I/O functions for the PFM data editor.  The PFM
       files consist of a file list (or control) file, a line file, a bin file, and an
       index file.  The files may be actual "large" files or they may be "huge" files.
       That is, they may actually be one or more files up to 2GB in size that are handled
       "transparently" by the "huge" I/O package.  The "huge" I/O package was written
       prior to Microsoft Windows being able to support files larger than 4GB.<br><br>
       PFM, or Pure File Magic, was conceived (watch it) on a recording trip to Nashville.
       The design was refined over the next year or so by the usual band of suspects.
       The original purpose of this little piece of work was to allow hydrographers to
       geographically view minimum, maximum, and average binned surfaces, of whatever bin
       size they chose, and then allow them to edit the original depth data.  After editing
       the depth data, the bins would be recomputed and the binned surface redisplayed.
       The idea being that the hydrographer could view the min or max binned surface to find
       flyers and then just edit those areas that required it.  In addition to manual viewing
       and hand editing, the PFM format is helpful for automatic filtering in places where
       data from different files overlaps.  After editing of data is complete the status
       information can be loaded back into the original data files be they GSF, SHOALS, HOF,
       TOF, or any other file type for which a loader has been developed.  In this way you
       aren't forced to use a specific format to store/manage your data.

       I would like to acknowledge the contributions to the PFM design of the usual band
       of suspects:
           - Jim Hammack, NAVO
           - Dave Fabre, Neptune Sciences, Inc.
           - Becky Martinolich, NAVO
           - Dwight Johnson, NAVO
           - Shannon Byrne, SAIC
           - Mark Paton, IVS

           - Jan 'Evil Twin' Depner
           - Naval Oceanographic Office
           - depnerj@navo.navy.mil, eviltwin69@cableone.net


       More recent contributors to the project (without whom this dog wouldn't hunt):
           - Danny Neville, IVS
           - Jeff Parker, SAIC
           - Graeme Sweet, IVS
           - Webb McDonald, SAIC
           - William Burrow, IVS
           - R. Wade Ladner, NAVO


       <br><br>\section format PFM Format

       The PFM data structure consists of a number of directories and files. At it's simplest it can be
       viewed as a single .pfm handle file and a .pfm.data data directory. Inside the data directory are
       a number of other files and directories. In general terms, the PFM structure consists of an ASCII
       control file (.ctl) containing the names of all of the associated PFM and non-PFM files and
       directories, a binned surface file (.bin) containing all of the binned surfaces and links to the
       indexed data, an indexed file (.ndx) containing the original input data and status information, and
       an optional line file (.lin) containing line names. A simple graphical overview of the structure
       follows. Note that the diagram does not show the handle file. Also, all binary data in the bin and
       ndx files are bit packed (the number of bits used for each field is stored in the bin header).

       \image html PFM.png

       As can be seen by the above diagram, PFM allows for very quick access to any data based on its
       geographic location within the binned surface. In addition, the original input file/record/subrecord
       can be accessed easily by using the file number, ping (record) number, and beam (subrecord) number
       from the index file in combination with the input file name from the ctl file.


       <br><br>\section huge Huge I/O Package

       The Huge I/O package provides most of the stream I/O functions for 'files' larger than 2147483647 bytes.
       Standard ANSI C will not allow addressing of files larger than 2147483647 bytes (signed long int
       offsets).  These functions use long long (non-ANSI) offsets and multiple files to provide virtual huge
       file I/O.  The interface is almost identical to the standard stream I/O except thatall offsets are double precision
       and instead of a FILE pointer, an integer handle is used.  Specific differences are described in the comments.
       At present this library is only set up to handle 384 concurrently opened huge files and 128 sub-files.  It is
       assumed that you have enough space on a single disk (or logical volume) to hold all of the separate files
       that are needed.  Placing files on separate disks is not supported.  Also, the 'append' I/O mode is not
       supported.  The max file size is set to 2148347647 - 65536 due to the fact that there is some overhead
       associated with the file itself.  The functions that are provided are:
           - hfopen
           - hfclose
           - hfseek
           - hftell
           - hfread
           - hfwrite
           - hrewind
           - hftruncate


       <br><br>\section large Large I/O Package

       The Large I/O package mimics the huge_io library but uses _LARGEFILE64_SOURCE I/O.  The only reason for
       providing this library is so that we can use arrays of function calls in the PFM I/O API.  This
       allows us to support both the old huge_io files and new files using _LARGEFILE64_SOURCE I/O while
       avoiding "if" statements for every I/O call.  We do this by implementing all of the I/O calls (e.g.
       lfseek, hfseek) as two member arrays of functions.  At present this library is only supported under gcc
       on Linux/UNIX, Mac OS/X, and MinGW on Windows.  There is an arbitrary limit of 384 "large" files
       that can be open at once.


       <br><br>\section handle PFM Handle File (.pfm)

           - All records CR separated

           - First record :
               - [VERSION] = software version information

           - Subsequent records:
               - ASCII comments : all lines must start with a # character.
               There is a boilerplate section of comments that gets added
               just to let people know that this is a handle file and not
               a list file.  After that you can add any comments you want.
               This file is handy for saving load information.  Just add
               comments to the end.


       <br><br>\section control PFM Control File (.ctl)

           - All records CR separated

           - First record :
               - ASCII bin (huge) file name (/net/hihp05/disk1/usr3/...)

           - Second record:
               - ASCII point (huge) file name (/net/hihp05/disk1/usr3/...)

           - Third record:
               - ASCII image mosaic (unisips) file name (/net/hihp05/disk1/usr3/...)

           - Fourth record:
               - ASCII target (ASCII) file name (/net/hihp05/disk1/usr3/...)

           - Subsequent records:
               - A + (non-deleted files) or - (deleted files), a space, a five
                 digit sequence number, a space, a two digit data type number,
                 and the ASCII input file name (/net/hihp05/disk1/usr3/...)
                 NOTE : The sequence number is used as a check for an improperly
                 modified or corrupted PFM list file.  Since the file is ASCII (so
                 that the user can move the input and PFM files) it may be modified
                 by the user.  The files must remain in the same sequence in which
                 they were loaded since the file number is stored in the PFM index
                 file.


       <br><br>\section line PFM Line File (.lin)

           - All records CR separated

           - All records :
               - ASCII line descriptor (no more than 512 characters)


       <br><br>\section bin PFM Bin File (.bin)

           - Header block (address 0) :
               - 16384 byte header information
               - All records CR separated and tagged
               - Including, but not limited to :
                   - [VERSION] = software version information
                   - [RECORD LENGTH] = (in number of soundings)
                   - [DATE] =
                   - [CLASSIFICATION] =
                   - ************************* Version Dependency ****************************
                       - The following section supports pre-4.0 format files.
                           - [SOUTH LAT] = (in degrees)
                           - [WEST LON] = (in degrees)
                           - [NORTH LAT] = (in degrees, recomputed when header is written)
                           - [EAST LON] = (in degrees, recomputed when header is written)
                   - *************************************************************************
                   - [MIN Y] = in projected coordinates
                   - [MIN X] = in projected coordinates
                   - [MAX Y] = in projected coordinates
                   - [MAX X] = in projected coordinates
                   - [X BIN SIZE] = (in degrees, computed when header is written)
                   - [Y BIN SIZE] = (in degrees, computed when header is written)
                   - [BIN WIDTH] = (in number of cells, computed when header is written)
                   - [BIN HEIGHT] = (in number of cells, computed when header is written)
                   - [MIN FILTERED DEPTH] = minimum filtered depth
                   - [MAX FILTERED DEPTH] = maximum filtered depth
                   - [MIN FILTERED COORD] = bin index of minimum filtered depth
                   - [MAX FILTERED COORD] = bin index of maximum filtered depth
                   - [MIN DEPTH] = minimum unfiltered depth
                   - [MAX DEPTH] = maximum unfiltered depth
                   - [MIN COORD] = bin index of minimum depth
                   - [MAX COORD] = bin index of maximum depth
                   - [MINIMUM BIN COUNT] = minimum bin count
                   - [MAXIMUM BIN COUNT] = maximum bin count
                   - [MIN COUNT COORD] = bin index of minimum count
                   - [MAX COUNT COORD] = bin index of maximum count
                   - [COUNT BITS] = number of bits used for count and numbers
                   - [STD BITS] = number of bits used for standard deviation value
                   - [STD SCALE] = scale factor for standard deviation values
                   - [DEPTH BITS] = number of bits used for depths
                   - [DEPTH SCALE] = scale factor for depths
                   - [DEPTH OFFSET] = positive depth offset to make all depths positive
                   - ************************* Version Dependency ****************************
                       - The following section supports pre-4.0 format files.
                           - [EDITED FLAG BITS] = number of bits used for edited flag
                           - [CHECKED FLAG BITS] = number of bits used for checked flag
                           - [SUSPECT FLAG BITS] = number of bits used for filter suspect flag
                           - [DATA FLAG BITS] = number of bits used for data flag
                           - [SELECTED FLAG BITS] = number of bits used for selected flag
                           - [CLASS 1 FLAG BITS] = number of bits used for class 1 flag
                           - [CLASS 2 FLAG BITS] = number of bits used for class 2 flag
                           - [PROJECTED X BITS] = number of bits used for the projected X value
                           - [PROJECTED X OFFSET] = offset to zeroize the projected x values
                   - *************************************************************************
                   - [RECORD POINTER BITS] = number of bits used for record pointer
                   - [FILE NUMBER BITS] = number of bits used for input file number
                   - [LINE NUMBER BITS] = number of bits used for line number
                   - [PING NUMBER BITS] = number of bits used for ping/record number
                   - [BEAM NUMBER BITS] = number of bits used for beam/subrecord number
                   - [OFFSET BITS] = number of bits used for X/Y offset
                   - [VALIDITY BITS] = number of bits used for data validity flags
                   - [POINT] = SXX.XXXXXXX,SXXX.XXXXXXX (repeating polygon points, up to 200)
                   - [MIN STANDARD DEVIATION] = minimum standard deviation
                   - [MAX STANDARD DEVIATION] = maximum standard deviation
                   - [CHART SCALE] = chart scale (1:CHART SCALE)
                   - [CLASS TYPE] = 0, 1, 2, 3, 4 (bins computed from all data, or bins computed from user flag
                                    1, 2, 3, or 4 data.  This flag will be set by an external program to tell the
                                    library how to do the bin recompute process.
                   - [PROJECTION] = See pfm.h for all projection definitions
                   - [PROJECTION ZONE] = See pfm.h for all projection definitions
                   - [HEMISPHERE] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 0] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 1] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 2] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 3] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 4] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 5] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 6] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 7] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 8] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 9] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 10] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 11] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 12] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 13] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 14] = See pfm.h for all projection definitions
                   - [PROJECTION PARAMETER 15] = See pfm.h for all projection definitions
                   - [AVERAGE FILTERED NAME] = Name of the "average filtered" surface.  This is used for other
                                               surface types (e.g. CUBE).  If this is set to "Average Filtered
                                               Depth" or "Average Edited Depth" the surface is recomputed by
                                               the library, otherwise it can be computed by outside applications
                                               and will not be modified by the library.  The default is "Average
                                               Filtered Depth".
                   - [AVERAGE NAME] = Name of the "average" surface.  This is used for other surface types (e.g.
                                      CUBE).  Whether the surface is recomputed or not depends on the AVERAGE
                                      FILTERED NAME above.  If that is set to "Average Filtered Depth" the value is
                                      recomputed by the library, otherwise it can be computed by outside applications
                                      and will not be modified by the library.  The default for this is "Average Depth".
                   - [DYNAMIC RELOAD] = Boolean flag to indicate that this file will be dynamically unloaded/reloaded
                                        during editing.  By default this is set to NVFalse.
                   - ************************* Version Dependency ****************************
                       - The following section supports pre-5.0 format files.
                           - [ATTRIBUTE 0] = Name of the first extra attribute, 30 characters max
                               - 
                               - 
                               - 
                           - [ATTRIBUTE N] = Name of the Nth extra attribute, 30 characters max
                           - [MINIMUM ATTRIBUTE 0] = Minimum value for attribute 0
                           - [MAXIMUM ATTRIBUTE 0] = Maximum value for attribute 0
                               - 
                               - 
                               - 
                           - [MINIMUM ATTRIBUTE N] = Minimum value for attribute N
                           - [MAXIMUM ATTRIBUTE N] = Maximum value for attribute N
                           - [ATTRIBUTE BITS 0] = Number of bits used to store attribute 0
                               - 
                               - 
                               - 
                           - [ATTRIBUTE BITS N] = Number of bits used to store attribute N
                           - [ATTRIBUTE SCALE 0] = Scale factor for attribute 0 (1.0 for versions before 4.1)
                               - 
                               - 
                               - 
                           - [ATTRIBUTE SCALE N] = Scale factor for attribute N (1.0 for versions before 4.1)
                   - *************************************************************************
                   - [BIN ATTRIBUTE 0] = Name of the first extra bin attribute, 30 characters max
                       - 
                       - 
                       - 
                   - [BIN ATTRIBUTE N] = Name of the Nth extra bin attribute, 30 characters max
                   - [MINIMUM BIN ATTRIBUTE 0] = Minimum value for bin attribute 0
                   - [MAXIMUM BIN ATTRIBUTE 0] = Maximum value for bin attribute 0
                       - 
                       - 
                       - 
                   - [MINIMUM BIN ATTRIBUTE N] = Minimum value for bin attribute N
                   - [MAXIMUM BIN ATTRIBUTE N] = Maximum value for bin attribute N
                   - [BIN ATTRIBUTE BITS 0] = Number of bits used to store bin attribute 0
                       - 
                       - 
                       - 
                   - [BIN ATTRIBUTE BITS N] = Number of bits used to store bin attribute N
                   - [BIN ATTRIBUTE SCALE 0] = Scale factor for bin attribute 0
                       - 
                       - 
                       - 
                   - [BIN ATTRIBUTE SCALE N] = Scale factor for bin attribute N
                   - [NDX ATTRIBUTE 0] = Name of the first extra ndx attribute, 30 characters max
                       - 
                       - 
                       - 
                   - [NDX ATTRIBUTE N] = Name of the Nth extra ndx attribute, 30 characters max
                   - [MINIMUM NDX ATTRIBUTE 0] = Minimum value for ndx attribute 0
                   - [MAXIMUM NDX ATTRIBUTE 0] = Maximum value for ndx attribute 0
                       - 
                       - 
                       - 
                   - [MINIMUM NDX ATTRIBUTE N] = Minimum value for ndx attribute N
                   - [MAXIMUM NDX ATTRIBUTE N] = Maximum value for ndx attribute N
                   - [NDX ATTRIBUTE BITS 0] = Number of bits used to store ndx attribute 0
                       - 
                       - 
                       - 
                   - [NDX ATTRIBUTE BITS N] = Number of bits used to store ndx attribute N
                   - [NDX ATTRIBUTE SCALE 0] = Scale factor for ndx attribute 0
                       - 
                       - 
                       - 
                   - [NDX ATTRIBUTE SCALE N] = Scale factor for ndx attribute N
                   - [USER FLAG 1 NAME] = Name of PFM_USER_01, 30 characters max
                   - [USER FLAG 2 NAME] = Name of PFM_USER_02, 30 characters max
                   - [USER FLAG 3 NAME] = Name of PFM_USER_03, 30 characters max
                   - [USER FLAG 4 NAME] = Name of PFM_USER_04, 30 characters max
                   - [COVERAGE MAP ADDRESS] = address of the coverage map at the end of the bin file.
                   - [NULL DEPTH] = null depth value






           - Subsequent (BIN_WIDTH * BIN_HEIGHT) bin records (address BIN_HEADER_SIZE) :
               - COUNT BITS : count of number of soundings in bin
               - STD BITS : standard deviation (integer * STD SCALE)
               - DEPTH BITS : filtered average depth (integer * DEPTH SCALE)
               - DEPTH BITS : filtered minimum depth (integer * DEPTH SCALE)
               - DEPTH BITS : filtered maximum depth (integer * DEPTH SCALE)
               - DEPTH BITS : average depth (integer * DEPTH SCALE)
               - DEPTH BITS : minimum depth (integer * DEPTH SCALE)
               - DEPTH BITS : maximum depth (integer * DEPTH SCALE)
               - BIN ATTRIBUTE BITS 0 : some value associated with bin attribute 0 (this is directly
                                        settable, not computed)
                   - 
                   - 
                   - 
               - BIN ATTRIBUTE BITS N : some value associated with bin attribute N (this is directly
                                        settable, not computed)
               - ************************* Version Dependency ****************************
                   - The following section supports pre-4.0 format files.
                       - EDITED FLAG BITS : edited flag
                       - CHECKED FLAG BITS : checked flag
                       - SUSPECT FLAG BITS : filter suspect flag
                       - DATA FLAG BITS : data flag
                       - SELECTED FLAG BITS : selected flag
                       - CLASS 1 FLAG BITS : class 1 flag
                       - CLASS 2 FLAG BITS : class 2 flag
                       - PROJECTED X BITS : Difference between computed X offset from western edge and
                                            estimated X offset (meters * 100)
               - *************************************************************************
               - VALIDITY BITS : status information (see pfm.h)
               - RECORD POINTER BITS : pointer to first 'physical' sounding buffer for this bin
               - RECORD POINTER BITS : pointer to last 'physical' sounding buffer for this bin

           - Following all of the bin records is the coverage map (BIN_WIDTH * BIN_HEIGHT) one byte records


       <br><br>\section ndx PFM Index File (.ndx)

       A single, physical index file record contains the following:

           - Sounding information (repeats up to RECORD LENGTH times)
               - FILE NUMBER BITS : input file number (0 indicates end of chain)
               - LINE NUMBER BITS : line number
               - PING NUMBER BITS : ping/record number
               - BEAM NUMBER BITS : beam/subrecord number
               - DEPTH BITS : depth (integer * DEPTH SCALE)
               - OFFSET BITS : X offset from lower left corner of bin
               - OFFSET BITS : Y offset from lower left corner of bin
               - (OPTIONAL) NDX ATTRIBUTE BITS 0 : ndx attribute 0 value (integer * NDX ATTR SCALE 0)
                   - 
                   - 
                   - 
               - (OPTIONAL) NDX ATTRIBUTE BITS N : ndx attribute N value (integer * NDX ATTR SCALE N)
               - VALIDITY BITS : status information (see pfm.h)
               - (OPTIONAL) HORIZONTAL ERROR VALUE : horizontal error (integer * HORIZONTAL ERROR SCALE)
               - (OPTIONAL) VERTICAL ERROR VALUE : vertical (integer * VERTICAL ERROR SCALE)
           - RECORD POINTER BITS : continuation pointer (set to 0 if not needed)


       <br><br>\section notes Programming Notes

       This library uses the NAVO standard data types.  You must set one of the following options on the
       compile command line:
           - NVLinux
           - NVWIN3X


       <br><br>\section search Searching the Doxygen Documentation

       The easiest way to find documentation for a particular C function is by looking for it in the
       <a href="globals.html"><b>Files>Globals>All</b></a> tab.  So, for instance, if you are looking for
       documentation on the <b><i>get_next_list_file_number</i></b> function you would go to
       <a href="globals.html"><b>Files>Globals>All</b></a> and click on the <b><i>g</i></b> tab then look for
       <b><i>get_next_list_file_number</i></b>.  When you find <b><i>get_next_list_file_number</i></b>, just
       click on the .h file associated with it.  If you would like to see the history of changes that have
       been made to the PFM API you can look at pfm_version.h.<br><br>

  */


#ifndef __PFM_INCLUDE__
#define __PFM_INCLUDE__


#ifdef __PFM_IO__
#define __PFM_EXTERN__
#else
#define __PFM_EXTERN__ extern
#endif


#ifdef  __cplusplus
extern "C" {
#endif


#include "pfm_nvtypes.h"


/*!  Debug switch for the PFM library software.  */

#undef              PFM_DEBUG


/*!  Maximum number of polygon points for the defined area.  */

#define             MAX_POLY                200


/*!  Maximum number of PFM files that can be opened at one time.  */

#define             MAX_PFM_FILES           32


/*!  PFM error status            */

__PFM_EXTERN__ NV_INT32             pfm_error;


/*  Layer types (for bin data).  */

#define         NUM_SOUNDINGS               0    /*!<  Number of soundings layer  */ 
#define         STANDARD_DEV                1    /*!<  Standard deviation layer  */
#define         AVERAGE_FILTERED_DEPTH      2    /*!<  Average filtered/edited depth layer  */
#define         MIN_FILTERED_DEPTH          3    /*!<  Minimum filtered/edited depth layer  */
#define         MAX_FILTERED_DEPTH          4    /*!<  Maximum filtered/edited depth layer  */
#define         AVERAGE_DEPTH               5    /*!<  Average unfiltered/unedited depth layer  */
#define         MIN_DEPTH                   6    /*!<  Minimum unfiltered/unedited depth layer  */
#define         MAX_DEPTH                   7    /*!<  Maximum unfiltered/unedited depth layer  */

#define         ATTRIBUTE0                  8    /*!<  Application defined attribute 0 layer  */
#define         ATTRIBUTE1                  9    /*!<  Application defined attribute 1 layer  */
#define         ATTRIBUTE2                  10   /*!<  Application defined attribute 2 layer  */
#define         ATTRIBUTE3                  11   /*!<  Application defined attribute 3 layer  */
#define         ATTRIBUTE4                  12   /*!<  Application defined attribute 4 layer  */
#define         ATTRIBUTE5                  13   /*!<  Application defined attribute 5 layer  */
#define         ATTRIBUTE6                  14   /*!<  Application defined attribute 6 layer  */
#define         ATTRIBUTE7                  15   /*!<  Application defined attribute 7 layer  */
#define         ATTRIBUTE8                  16   /*!<  Application defined attribute 8 layer  */
#define         ATTRIBUTE9                  17   /*!<  Application defined attribute 9 layer  */

#define         NUM_ATTR                    10   /*!<  Maximum number of attribute layers  */

#define         NUM_LAYERS                  8 + NUM_ATTR   /*!<  Maximum number of layers  */


#define         FILE_BITS                   13   /*!<  Default number of bits used for file number  */
#define         PFM_MAX_FILES               8192  /*!< This should always be set to 2 raised to the FILE_BITS power.  If you change
                                                       FILE_BITS you must change this too.  */
#define         LINE_BITS                   15   /*!<  Default number of bits used for line number  */
#define         PING_BITS                   32   /*!<  Careful with this value.  It's stored as NV_U_INT32.  If you need more than
                                                       4,294,967,295 (2 ** 32 - 1) pings per file we'll need to go to NV_INT64 and
                                                       double_bit_(un)pack.  With a rep rate of 20,000 pings per second this
                                                       will store ~60 hours of pings/shots.  I can't see this rep rate for a sonar
                                                       and I can't see 60 hour flights for LIDAR.  Correct me if I'm wrong.  JCD  */
#define         BEAM_BITS                   16   /*!<  Same here, it's stored as NV_U_INT16.  65,535 beams/subrecords seems like 
                                                       it should be enough.  Again, I could be wrong.  JCD  */



  /*!  IMPORTANT NOTE:  As of version 6.0, use of the projection parameters is deprecated.  We now use 
       the Well-Known Text coordinate and datum information that is defined in the Open GIS Coordinate
       Transformations specification.  We will, however, still support old files containing the
       projection parameters so they are left in the header and the header structure.  It's not a
       large price to pay really ;-)  The following describes the old projection parameters:

    <pre>

    February 1996

    Appendix A:



          Projection Transformation Package Projection Parameters

  -----------------------------------------------------------------------------
                          |                     Array Element                 |
   Code & Projection Id   |----------------------------------------------------
                          |   1  |   2  |  3   |  4   |   5   |    6    |7 | 8|
  -----------------------------------------------------------------------------
   0 Geographic           |      |      |      |      |       |         |  |  |
   1 U T M                |Lon/Z |Lat/Z |      |      |       |         |  |  |
   2 State Plane          |      |      |      |      |       |         |  |  |
   3 Albers Equal Area    |SMajor|SMinor|STDPR1|STDPR2|CentMer|OriginLat|FE|FN|
   4 Lambert Conformal C  |SMajor|SMinor|STDPR1|STDPR2|CentMer|OriginLat|FE|FN|
   5 Mercator             |SMajor|SMinor|      |      |CentMer|TrueScale|FE|FN|
   6 Polar Stereographic  |SMajor|SMinor|      |      |LongPol|TrueScale|FE|FN|
   7 Polyconic            |SMajor|SMinor|      |      |CentMer|OriginLat|FE|FN|
   8 Equid. Conic A       |SMajor|SMinor|STDPAR|      |CentMer|OriginLat|FE|FN|
     Equid. Conic B       |SMajor|SMinor|STDPR1|STDPR2|CentMer|OriginLat|FE|FN|
   9 Transverse Mercator  |SMajor|SMinor|Factor|      |CentMer|OriginLat|FE|FN|
  10 Stereographic        |Sphere|      |      |      |CentLon|CenterLat|FE|FN|
  11 Lambert Azimuthal    |Sphere|      |      |      |CentLon|CenterLat|FE|FN|
  12 Azimuthal            |Sphere|      |      |      |CentLon|CenterLat|FE|FN|
  13 Gnomonic             |Sphere|      |      |      |CentLon|CenterLat|FE|FN|
  14 Orthographic         |Sphere|      |      |      |CentLon|CenterLat|FE|FN|
  15 Gen. Vert. Near Per  |Sphere|      |Height|      |CentLon|CenterLat|FE|FN|
  16 Sinusoidal           |Sphere|      |      |      |CentMer|         |FE|FN|
  17 Equirectangular      |Sphere|      |      |      |CentMer|TrueScale|FE|FN|
  18 Miller Cylindrical   |Sphere|      |      |      |CentMer|         |FE|FN|
  19 Van der Grinten      |Sphere|      |      |      |CentMer|OriginLat|FE|FN|
  20 Hotin Oblique Merc A |SMajor|SMinor|Factor|      |       |OriginLat|FE|FN|
     Hotin Oblique Merc B |SMajor|SMinor|Factor|AziAng|AzmthPt|OriginLat|FE|FN|
  21 Robinson             |Sphere|      |      |      |CentMer|         |FE|FN|
  22 Space Oblique Merc A |SMajor|SMinor|      |IncAng|AscLong|         |FE|FN|
     Space Oblique Merc B |SMajor|SMinor|Satnum|Path  |       |         |FE|FN|
  23 Alaska Conformal     |SMajor|SMinor|      |      |       |         |FE|FN|
  24 Interrupted Goode    |Sphere|      |      |      |       |         |  |  |
  25 Mollweide            |Sphere|      |      |      |CentMer|         |FE|FN|
  26 Interrupt Mollweide  |Sphere|      |      |      |       |         |  |  |
  27 Hammer               |Sphere|      |      |      |CentMer|         |FE|FN|
  28 Wagner IV            |Sphere|      |      |      |CentMer|         |FE|FN|
  29 Wagner VII           |Sphere|      |      |      |CentMer|         |FE|FN|
  30 Oblated Equal Area   |Sphere|      |Shapem|Shapen|CentLon|CenterLat|FE|FN|
  -----------------------------------------------------------------------------




         ----------------------------------------------------
                                 |      Array Element       |
          Code & Projection Id   |---------------------------
                                 |  9  | 10 |  11 | 12 | 13 |
         ----------------------------------------------------
          0 Geographic           |     |    |     |    |    |
          1 U T M                |     |    |     |    |    |
          2 State Plane          |     |    |     |    |    |
          3 Albers Equal Area    |     |    |     |    |    |
          4 Lambert Conformal C  |     |    |     |    |    |
          5 Mercator             |     |    |     |    |    |
          6 Polar Stereographic  |     |    |     |    |    |
          7 Polyconic            |     |    |     |    |    |
          8 Equid. Conic A       |zero |    |     |    |    |
            Equid. Conic B       |one  |    |     |    |    |
          9 Transverse Mercator  |     |    |     |    |    |
         10 Stereographic        |     |    |     |    |    |
         11 Lambert Azimuthal    |     |    |     |    |    |
         12 Azimuthal            |     |    |     |    |    |
         13 Gnomonic             |     |    |     |    |    |
         14 Orthographic         |     |    |     |    |    |
         15 Gen. Vert. Near Per  |     |    |     |    |    |
         16 Sinusoidal           |     |    |     |    |    |
         17 Equirectangular      |     |    |     |    |    |
         18 Miller Cylindrical   |     |    |     |    |    |
         19 Van der Grinten      |     |    |     |    |    |
         20 Hotin Oblique Merc A |Long1|Lat1|Long2|Lat2|zero|
            Hotin Oblique Merc B |     |    |     |    |one |
         21 Robinson             |     |    |     |    |    |
         22 Space Oblique Merc A |PSRev|LRat|PFlag|    |zero|
            Space Oblique Merc B |     |    |     |    |one |
         23 Alaska Conformal     |     |    |     |    |    |
         24 Interrupted Goode    |     |    |     |    |    |
         25 Mollweide            |     |    |     |    |    |
         26 Interrupt Mollweide  |     |    |     |    |    |
         27 Hammer               |     |    |     |    |    |
         28 Wagner IV            |     |    |     |    |    |
         29 Wagner VII           |     |    |     |    |    |
         30 Oblated Equal Area   |Angle|    |     |    |    |
         ----------------------------------------------------

    where

    Lon/Z     Longitude of any point in the UTM zone or zero.  If zero,
              a zone code must be specified.
    Lat/Z     Latitude of any point in the UTM zone or zero.  If zero, a
              zone code must be specified.
    SMajor    Semi-major axis of ellipsoid.  If zero, Clarke 1866 in meters
              is assumed.
    SMinor    Eccentricity squared of the ellipsoid if less than zero,
              if zero, a spherical form is assumed, or if greater than
              zero, the semi-minor axis of ellipsoid.
    Sphere    Radius of reference sphere.  If zero, 6370997 meters is used.
    STDPAR    Latitude of the standard parallel
    STDPR1    Latitude of the first standard parallel
    STDPR2    Latitude of the second standard parallel
    CentMer   Longitude of the central meridian
    OriginLat Latitude of the projection origin
    FE        False easting in the same units as the semi-major axis
    FN        False northing in the same units as the semi-major axis
    TrueScale Latitude of true scale
    LongPol   Longitude down below pole of map
    Factor    Scale factor at central meridian (Transverse Mercator) or
              center of projection (Hotine Oblique Mercator)
    CentLon   Longitude of center of projection
    CenterLat Latitude of center of projection
    Height    Height of perspective point
    Long1     Longitude of first point on center line (Hotine Oblique
              Mercator, format A)
    Long2     Longitude of second point on center line (Hotine Oblique
              Mercator, format A)
    Lat1      Latitude of first point on center line (Hotine Oblique
              Mercator, format A)
    Lat2      Latitude of second point on center line (Hotine Oblique
              Mercator, format A)
    AziAng    Azimuth angle east of north of center line (Hotine Oblique
              Mercator, format B)
    AzmthPt   Longitude of point on central meridian where azimuth occurs
              (Hotine Oblique Mercator, format B)
    IncAng    Inclination of orbit at ascending node, counter-clockwise
              from equator (SOM, format A)
    AscLong   Longitude of ascending orbit at equator (SOM, format A)
    PSRev     Period of satellite revolution in minutes (SOM, format A)
    LRat      Landsat ratio to compensate for confusion at northern end
              of orbit (SOM, format A -- use 0.5201613)
    PFlag     End of path flag for Landsat:  0 = start of path,
              1 = end of path (SOM, format A)
    Satnum    Landsat Satellite Number (SOM, format B)
    Path      Landsat Path Number (Use WRS-1 for Landsat 1, 2 and 3 and
              WRS-2 for Landsat 4, 5 and 6.)  (SOM, format B)
    Shapem    Oblated Equal Area oval shape parameter m
    Shapen    Oblated Equal Area oval shape parameter n
    Angle     Oblated Equal Area oval rotation angle


                                   NOTES

    Array elements 14 and 15 are set to zero
    All array elements with blank fields are set to zero
    All angles (latitudes, longitudes, azimuths, etc.) are entered in packed
        degrees/ minutes/ seconds (DDDMMMSSS.SS) format



    The following notes apply to the Space Oblique Mercator A projection.

        A portion of Landsat rows 1 and 2 may also be seen as parts of rows
    246 or 247.  To place these locations at rows 246 or 247, set the end of
    path flag (parameter 11) to 1--end of path.  This flag defaults to zero.

        When Landsat-1,2,3 orbits are being used, use the following values
    for the specified parameters:

        Parameter 4   099005031.2
        Parameter 5   128.87 degrees - (360/251 * path number) in packed
                      DMS format
        Parameter 9   103.2669323
        Parameter 10  0.5201613

        When Landsat-4,5 orbits are being used, use the following values
    for the specified parameters:

        Parameter 4   098012000.0
        Parameter 5   129.30 degrees - (360/233 * path number) in packed
                      DMS format
        Parameter 9   98.884119
        Parameter 10  0.5201613

  </pre>*/

typedef struct
{
  NV_U_BYTE       projection;       /*!<  0 - geographic, 1 - UTM, . . . etc., as defined below (from GCTP
                                          appendix A)  */
  NV_U_INT16      zone;             /*!<  Zone number, used only for UTM or State Plane projection  */
  NV_U_BYTE       hemisphere;       /*!<  Hemisphere, 0 - south, 1 - north  */
  NV_FLOAT64      params[16];       /*!<  Projection parameters as defined above (from GCTP appendix A)  */
  NV_CHAR         wkt[1024];        /*!<  Well-Known Text  */
} PROJECTION_DATA;


  /*!

      - Bin Header record structure:

      - Reserved attribute names:

          - There will occasionally arise situations where we will want to use attributes for
            items that are useful to more than one processing entity.  For instance, CUBE
            processing parameters.  We don't want to make these part of the fixed PFM structure
            because "Son of CUBE" may be coming along soon.  Even so, CUBE will be used for quite
            some time to process sonar data so we want everyone that processes a PFM structure
            to be able to recognize "CUBE number of hypotheses", "CUBE computed vertical uncertainty",
            and other values regardless of which attribute they are placed in.  To this end we
            are implementing reserved attribute names.  Instead of fixing the names in English
            and making it difficult to translate them to other languages we are defining numbers
            and leaving the translation to the application program.  Nonetheless, all reserved
            attribute "names" and definitions MUST be listed here as comments so that others
            can adjust their applications to deal with them.  The definitions here will be in
            American English (since that's all I know ;-)  A reserved attribute name will consist
            of three (3) pound signs (#) followed by a number.  Any addition of reserved attribute
            names should be forwarded to all interested parties (as with data types).

                - ###0             =      CUBE number of hypotheses
                - ###1             =      CUBE computed vertical uncertainty
                - ###2             =      CUBE hypothesis strength
                - ###3             =      CUBE number of contributing soundings
                - ###4             =      Average of vertical TPE for best hypothesis soundings
                - ###5             =      Final Uncertainty (MAX of CUBE StdDev and Average TPE)

            Please note that the min and max values and the scale may change for these reserved
            attributes.  The meaning of the attribute should be universally agreed upon.  If you add 
            to these be sure to modify pfm_standard_attr_name in pfm_extras.c which is also 
            where you would add translations.

  *********************************************************************************************/

typedef struct
{
  NV_CHAR         version[128];               /*!<  Version (ASCII)  */
  NV_CHAR         date[30];                   /*!<  Date (ASCII)  */
  NV_CHAR         classification[40];         /*!<  Classification (ASCII)  */
  NV_CHAR         creation_software[128];     /*!<  Creation software (ASCII)  */
  NV_F64_XYMBR    mbr;                        /*!<  Minimum bounding rectangle  */
  NV_FLOAT64      bin_size_xy;                /*!<  Bin size in projected coordinates  */
  NV_FLOAT32      chart_scale;                /*!<  ISS60 area file chart scale  */
  NV_FLOAT64      x_bin_size_degrees;         /*!<  X bin size in degrees  */
  NV_FLOAT64      y_bin_size_degrees;         /*!<  Y bin size in degrees  */
  NV_INT32        bin_width;                  /*!<  Width of MBR (# x bins)  */
  NV_INT32        bin_height;                 /*!<  Height of MBR (# y bins)  */
  NV_F64_COORD2   polygon[MAX_POLY];          /*!<  Polygon points  */
  NV_INT32        polygon_count;              /*!<  Polygon count  */
  NV_FLOAT32      min_filtered_depth;         /*!<  Min filtered depth for file  */
  NV_FLOAT32      max_filtered_depth;         /*!<  Max filtered depth for file  */
  NV_I32_COORD2   min_filtered_coord;         /*!<  Bin index of min filtered depth  */
  NV_I32_COORD2   max_filtered_coord;         /*!<  Bin index of max filtered depth  */
  NV_FLOAT32      min_depth;                  /*!<  Min depth for file  */
  NV_FLOAT32      max_depth;                  /*!<  Max depth for file  */
  NV_I32_COORD2   min_coord;                  /*!<  Bin index of min depth  */
  NV_I32_COORD2   max_coord;                  /*!<  Bin index of max depth  */
  NV_FLOAT32      null_depth;                 /*!<  Null depth for filtered bins  */
  NV_U_INT32      min_bin_count;              /*!<  Min number of values per bin  */
  NV_U_INT32      max_bin_count;              /*!<  Max number of values per bin  */
  NV_I32_COORD2   min_count_coord;            /*!<  Bin index of min count  */
  NV_I32_COORD2   max_count_coord;            /*!<  Bin index of max count  */
  NV_FLOAT64      min_standard_dev;           /*!<  Minimum standard deviation  */
  NV_FLOAT64      max_standard_dev;           /*!<  Maximum standard deviation  */
  NV_INT32        class_type;                 /*!<  0, 1, 2, 3, 4 - used to flag whether recomputes will use
                                                    only data of a specific user flag type  */
  NV_CHAR         average_filt_name[30];      /*!<  Name of the average filtered data surface.  This may be
                                                    the CUBE surface.  If you want the library to recompute
                                                    the average surface set this to Average Filtered Depth 
                                                    or Average Edited Depth otherwise it will not modify it.
                                                    This is set to Average Filtered Depth by default.  */
  NV_CHAR         average_name[30];           /*!<  Name of the average data surface.  This may be the CUBE
                                                    or other surface.  Whether the surface is recomputed by the
                                                    library depends on the average filtered name above.  This
                                                    is set to Average Depth by default.  */
  NV_BOOL         dynamic_reload;             /*!<  Flag that indicates that the data is to be dynamically
                                                    unloaded/reloaded to the input files during editing.
                                                    Set to NVFalse by default.  */
  PROJECTION_DATA proj_data;                  /*!<  Projection data as described above  */
  NV_CHAR         bin_attr_name[NUM_ATTR][30];/*!<  Names of attributes  */
  NV_FLOAT32      min_bin_attr[NUM_ATTR];     /*!<  Minimum value for each of NUM_ATTR optional attributes  */
  NV_FLOAT32      max_bin_attr[NUM_ATTR];     /*!<  Maximum value for each of NUM_ATTR optional attributes  */
  NV_FLOAT32      bin_attr_scale[NUM_ATTR];   /*!<  Attribute scales  */
  NV_FLOAT32      bin_attr_null[NUM_ATTR];    /*!<  Null value for attributes  */
  NV_U_INT16      num_bin_attr;               /*!<  Actual number of bin attributes.  */
  NV_CHAR         ndx_attr_name[NUM_ATTR][30];/*!<  Names of attributes  */
  NV_FLOAT32      min_ndx_attr[NUM_ATTR];     /*!<  Minimum value for each of NUM_ATTR optional attributes  */
  NV_FLOAT32      max_ndx_attr[NUM_ATTR];     /*!<  Maximum value for each of NUM_ATTR optional attributes  */
  NV_FLOAT32      ndx_attr_scale[NUM_ATTR];   /*!<  Attribute scales  */
  NV_U_INT16      num_ndx_attr;               /*!<  Actual number of ndx attributes  */
  NV_CHAR         user_flag_name[5][30];      /*!<  Names of user flags  */
  NV_FLOAT32      horizontal_error_scale;     /*!<  Scale for horizontal error values.  0.0 if no horizontal
                                                    error values stored.  */
  NV_FLOAT32      max_horizontal_error;       /*!<  Maximum horizontal error.  */
  NV_FLOAT32      vertical_error_scale;       /*!<  Scale for vertical error values.  0.0 if no vertical
                                                    error values stored  */
  NV_FLOAT32      max_vertical_error;         /*!<  Maximum vertical error  */
  NV_U_INT32      max_input_files;            /*!<  Maximum number of input files  */
  NV_U_INT32      max_input_lines;            /*!<  Maximum number of input lines  */
  NV_U_INT32      max_input_pings;            /*!<  Maximum number of records per input file  */
  NV_U_INT32      max_input_beams;            /*!<  Maximum number of subrecords per record  */
  NV_U_BYTE       pad[1024];                  /*!<  Pad area for future expansion.  This allows us to add to the
                                                    header record without creating shared memory incompatibilities.  */
} BIN_HEADER;


  /*!
    Chain pointer structure.  Points to previous and subsequent physical record in the index file
    depth record chain.
  */

typedef struct
{
  NV_INT64        head;                       /*!<  Head pointer for depth chain  */
  NV_INT64        tail;                       /*!<  Tail pointer for depth chain  */
} CHAIN;


  /*!  Bin record structure  */

typedef struct
{
  NV_U_INT32      num_soundings;              /*!<  Number of soundings in this bin  */
  NV_FLOAT32      standard_dev;               /*!<  Standard deviation  */
  NV_FLOAT32      avg_filtered_depth;         /*!<  Avg depth for bin (filtered)  */
  NV_FLOAT32      min_filtered_depth;         /*!<  Min depth for bin (filtered)  */
  NV_FLOAT32      max_filtered_depth;         /*!<  Max depth for bin (filtered)  */
  NV_FLOAT32      avg_depth;                  /*!<  Avg depth for bin  */
  NV_FLOAT32      min_depth;                  /*!<  Min depth for bin  */
  NV_FLOAT32      max_depth;                  /*!<  Max depth for bin  */
  NV_FLOAT32      attr[NUM_ATTR];             /*!<  Some value associated with NUM_ATTR optional attributes
                                                    defined by loader  */
  NV_U_INT32      validity;                   /*!<  Validity bits  */
  NV_I32_COORD2   coord;                      /*!<  X and Y indices for bin  */
  NV_F64_COORD2   xy;                         /*!<  Position of center of bin  */


  /*!
    - WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   WARNING<br><br>
    - The depth_chain pointer values MUST NOT be modified by the calling
      application.  They are used to save a lot of redundant I/O calls.
      Any call to add_depth_record will disable use of these pointers on
      write_bin_record calls.  Closing and reopening the PFM file will
      re-enable their use.  This is useful in a loader after adding all
      of the data prior to recomputing bin values.<br><br>
    - WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   WARNING
  */

  CHAIN           depth_chain;
  void*           temp;                       /*!<  Working pointer, only valid in small scope.
                                                    Expect routine using it to allocate and free memory.  */
  NV_U_CHAR       local_flags;                /*!<  Local flags to be used by the calling routine as
                                                    needed.  */
} BIN_RECORD;


  /*!  Block (physical record) address structure.  */

typedef struct
{
  NV_INT64        block;                      /*!<  Block address of the depth record  */
  NV_U_BYTE       record;                     /*!<  Position within the block of the depth record  */
} BLOCK_ADDRESS;


  /*!
    The depth record structure.  This is a single depth from anywhere within a depth chain or
    physical record.
  */

typedef struct
{
  NV_U_INT16      file_number;                /*!<  File number in file list (.ctl) file  */
  NV_U_INT32      line_number;                /*!<  Line number in line file  */
  NV_U_INT32      ping_number;                /*!<  Ping number in input file  */
  NV_U_INT16      beam_number;                /*!<  Beam (subrecord) number in ping (record)  */
  NV_U_INT32      validity;                   /*!<  Validity bits  */
  NV_I32_COORD2   coord;                      /*!<  X and Y indices for bin  */
  NV_F64_COORD3   xyz;                        /*!<  Position and depth  */
  NV_FLOAT32      horizontal_error;           /*!<  Horizontal error in meters.  Returns -999.0 for max error  */
  NV_FLOAT32      vertical_error;             /*!<  Vertical error in meters.  Returns -999.0 for max error  */
  NV_FLOAT32      attr[NUM_ATTR];             /*!<  NUM_ATTR attributes, defined by loader  */
  BLOCK_ADDRESS   address;                    /*!<  Physical record (block) address information for the depth
                                                    record.  */
  NV_U_BYTE       local_flags;                /*!<  Local flags to be used by the calling routine as needed  */
} DEPTH_RECORD;


  /*!  PFM file open arguments.  */

typedef struct
{
  NV_CHAR         list_path[512];             /*!<  This is the PFM list file for pre 4.6 structures.  In post
                                                    4.6 structures this should be the PFM handle file name.
                                                    The actual PFM  structure names will be derived from the
                                                    handle file name.  */
  NV_CHAR         ctl_path[512];              /*!<  File list (control) file path  */
  NV_CHAR         bin_path[512];              /*!<  Bin file path  */
  NV_CHAR         index_path[512];            /*!<  Index file path  */
  NV_CHAR         image_path[512];            /*!<  GeoTIFF mosaic file path  */
  NV_CHAR         target_path[512];           /*!<  Feature (target) file path  */
  BIN_HEADER      head;                       /*!<  Bin file header structure  */
  NV_FLOAT32      max_depth;                  /*!<  Maximum depth to be stored in the file  */
  NV_FLOAT32      offset;                     /*!<  Offset value so all depths will be positive (inverted minimum
                                                    depth value)  */
  NV_FLOAT32      scale;                      /*!<  Chart scale (not used anymore... I think ;-)  */
  NV_BYTE         checkpoint;                 /*!<
                                                - 0 = check for a checkpoint file
                                                - 1 = create a checkpoint file prior to opening the PFM file
                                                - 2 = recover from a checkpoint if one exists
                                              */
  NV_FLOAT32      bin_attr_offset[NUM_ATTR];  /*!<  Attribute offsets  */
  NV_FLOAT32      bin_attr_max[NUM_ATTR];     /*!<  Attribute max values  */
} PFM_OPEN_ARGS;


  /*!  PFM's shared memory structure (don't change this structure).  */

typedef struct
{
  NV_FLOAT64    cur_x[50];                    /*!<  Polygonal area to be edited (X)  */
  NV_FLOAT64    cur_y[50];                    /*!<  Polygonal area to be edited (Y)  */
  NV_INT32      count;                        /*!<  Count of polygon points, 0 for rectangle  */
  NV_F64_XYMBR  edit_area;                    /*!<  Total rectangular area to be edited  */
  NV_INT32      ppid;                         /*!<  Parent process ID (bin viewer)  */
  NV_FLOAT32    cint;                         /*!<  Current contour interval (0 for user defined)  */
  NV_FLOAT32    contour_levels[200];          /*!<  User defined contour levels  */
  NV_INT32      num_levels;                   /*!<  Number of user defined contour levels  */
  NV_INT32      layer_type;                   /*!<  Type of bin data/contour to display  */
  NV_F64_XYMBR  displayed_area;               /*!<  Displayed area in the editor  */
  PFM_OPEN_ARGS open_args;                    /*!<  Opening arguments for open_pfm_file or open_existing_pfm_file  */
  NV_CHAR       nearest_file[512];            /*!<  Path of file containing nearest point to the cursor in 
                                                    some program (usually some kind of point cloud editor)  */
  NV_CHAR       nearest_line[512];            /*!<  Line name for nearest point to the cursor in some program  */
  NV_INT32      nearest_file_num;             /*!<  File number of nearest point to the cursor in some program  */
  NV_INT32      nearest_line_num;             /*!<  Line number of nearest point to the cursor in some program  */
  NV_INT16      nearest_type;                 /*!<  Data type of nearest point to the cursor in some program  */
  NV_INT32      nearest_record;               /*!<  Record number (in file) of nearest point to the cursor in
                                                    some program  */
  NV_INT32      nearest_subrecord;            /*!<  Subrecord number (in file) of nearest point to the cursor in
                                                    some program  */
  NV_INT32      nearest_point;                /*!<  Point number (in point cloud) of nearest point to the cursor
                                                    in some program  */
  NV_FLOAT64    nearest_value;                /*!<  Z value of nearest point to the cursor in some program  */
  NV_U_INT32    nearest_validity;
  NV_U_INT32    key;                          /*!<  Keycode of last key pressed in some program.  Set to NV_INT32_MAX
                                                    to indicate lock on shared memory.  */
  NV_CHAR       modified_file[512];           /*!<  Modified file path of file containing nearest point to the 
                                                    cursor in some program (possibly modified by an external
                                                    program)  */
  NV_CHAR       modified_line[512];           /*!<  Modified line name  */
  NV_INT32      modified_file_num;            /*!<  Modified file number  */
  NV_INT32      modified_line_num;            /*!<  Modified line number  */
  NV_INT16      modified_type;                /*!<  Modified data type  */
  NV_INT32      modified_record;              /*!<  Modified record number  */
  NV_INT32      modified_subrecord;           /*!<  Modified subrecord number  */
  NV_INT32      modified_point;               /*!<  Modified point number  */
  NV_FLOAT64    modified_value;               /*!<  Modified Z value  */
  NV_U_INT32    modified_validity;            /*!<  Modified validity  */
  NV_U_INT32    modcode;                      /*!<  Set to something other than zero to indicate a change in the
                                                    shared data  */
  NV_U_BYTE     pad[1024];                    /*!<  Pad area for future expansion.  This allows us to add to the
                                                    shared memory area without creating shared memory
                                                    incompatibilities.  */
} EDIT_SHARE;


  /*!  Depth list structure for caching (linked list).  */

typedef struct DEPTH_LIST_STRUCT
{
  NV_U_BYTE                   *depths;
  struct DEPTH_LIST_STRUCT    *next;
} DEPTH_LIST;


  /*!  Summary record for the DEPTH cache.  */

typedef struct
{
    NV_U_INT64    continuation_pointer;
    NV_U_INT32    record_pos;

    DEPTH_LIST    buffers;
    DEPTH_LIST    *last_depth_buffer;

    NV_INT64      previous_chain;

  /*!
    - WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   WARNING<br><br>
    - The depth_chain pointer values MUST NOT be modified by the calling
      application.  They are used to save a lot of redundant I/O calls.
      Any call to add_depth_record will disable use of these pointers on
      write_bin_record calls.  Closing and reopening the PFM file will
      re-enable their use.  This is useful in a loader after adding all
      of the data prior to recomputing bin values.<br><br>
    - WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   WARNING
  */

    CHAIN         chain;                /*!<  Head and tail pointers for the depth chain.  */
} DEPTH_SUMMARY;


/*!  Summary record for the BIN cache. */

typedef struct
{
  NV_BOOL         dirty;                      /*!<  Whether the bin is in use or not.  */
  NV_U_INT32      num_soundings;              /*!<  Number of soundings in this bin  */
  NV_I32_COORD2   coord;                      /*!<  X and Y indices for bin  */
  NV_U_INT32      validity;                   /*!<  Validity bits  */
  DEPTH_SUMMARY   depth;                      /*!<  Depth record buffer for bin  */
  NV_U_INT32      cov_flag;                   /*!<  Flag required SAIC.  */
} BIN_RECORD_SUMMARY;


#define             SUCCESS                                         0
#define             CREATE_LIST_FILE_FILE_EXISTS                    -1
#define             CREATE_LIST_FILE_OPEN_ERROR                     -2
#define             OPEN_LIST_FILE_OPEN_ERROR                       -3
#define             OPEN_LIST_FILE_READ_VERSION_ERROR               -4
#define             OPEN_LIST_FILE_READ_BIN_ERROR                   -5
#define             OPEN_LIST_FILE_READ_INDEX_ERROR                 -6
#define             OPEN_LIST_FILE_READ_IMAGE_ERROR                 -7
#define             OPEN_LIST_FILE_READ_TARGET_ERROR                -8
#define             WRITE_BIN_HEADER_EXCEEDED_MAX_POLY              -9
#define             OPEN_BIN_OPEN_ERROR                             -10
#define             WRITE_DEPTH_BUFFER_WRITE_ERROR                  -11
#define             OPEN_INDEX_OPEN_ERROR                           -12
#define             READ_LIST_FILE_READ_NAME_ERROR                  -13
#define             READ_BIN_RECORD_DATA_READ_ERROR                 -14
#define             READ_DEPTH_RECORD_READ_ERROR                    -15
#define             READ_DEPTH_RECORD_CONTINUATION_ERROR            -16
#define             UPDATE_DEPTH_RECORD_READ_BIN_RECORD_ERROR       -17
#define             UPDATE_DEPTH_RECORD_READ_DEPTH_RECORD_ERROR     -18
#define             ADD_DEPTH_RECORD_READ_BIN_RECORD_ERROR          -19
#define             RECOMPUTE_BIN_VALUES_READ_BIN_RECORD_ERROR      -20
#define             RECOMPUTE_BIN_VALUES_READ_DEPTH_RECORD_ERROR    -21
#define             RECOMPUTE_BIN_VALUES_NO_SOUNDING_DATA_ERROR     -22
#define             RECOMPUTE_BIN_VALUES_WRITE_BIN_RECORD_ERROR     -23
#define             WRITE_BIN_BUFFER_WRITE_ERROR                    -24
#define             SET_OFFSETS_BIN_MALLOC_ERROR                    -25
#define             SET_OFFSETS_DEPTH_MALLOC_ERROR                  -26
#define             WRITE_BIN_RECORD_DATA_READ_ERROR                -27
#define             READ_DEPTH_RECORD_NO_DATA                       -28
#define             WRITE_BIN_RECORD_VALIDITY_INDEX_ERROR           -29
#define             CHECK_INPUT_FILE_OPEN_ERROR                     -30
#define             CHECK_INPUT_FILE_WRITE_ERROR                    -31
#define             CHANGE_DEPTH_RECORD_READ_BIN_RECORD_ERROR       -32
#define             CHANGE_DEPTH_RECORD_READ_DEPTH_RECORD_ERROR     -33
#define             CHANGE_DEPTH_RECORD_OUT_OF_RANGE_ERROR          -34
#define             READ_DEPTH_ARRAY_CALLOC_ERROR                   -35
#define             CREATE_LINE_FILE_FILE_EXISTS                    -36
#define             CREATE_LINE_FILE_OPEN_ERROR                     -37
#define             OPEN_LINE_FILE_OPEN_ERROR                       -38
#define             OPEN_LIST_FILE_CORRUPTED_FILE_ERROR             -39
#define             OPEN_BIN_CORRUPT_HEADER_ERROR                   -40
#define             ADD_DEPTH_RECORD_TOO_MANY_SOUNDINGS_ERROR       -41
#define             FILE_PING_BEAM_MISMATCH_ERROR                   -42
#define             CHANGE_ATTRIBUTE_RECORD_READ_BIN_RECORD_ERROR   -43
#define             CHANGE_ATTRIBUTE_RECORD_READ_DEPTH_RECORD_ERROR -44
#define             CHANGE_ATTRIBUTE_RECORD_OUT_OF_RANGE_ERROR      -45
#define             OPEN_EXISTING_PFM_ENOENT_ERROR                  -46
#define             OPEN_EXISTING_PFM_LIST_FILE_OPEN_ERROR          -47
#define             OPEN_HANDLE_FILE_CREATE_ERROR                   -48
#define             CREATE_PFM_DATA_DIRECTORY_ERROR                 -49
#define             CHECKPOINT_FILE_EXISTS_ERROR                    -50
#define             CHECKPOINT_FILE_UNRECOVERABLE_ERROR             -51
#define             FILE_NUMBER_TOO_LARGE_ERROR                     -52
#define             LINE_NUMBER_TOO_LARGE_ERROR                     -53
#define             PING_NUMBER_TOO_LARGE_ERROR                     -54
#define             BEAM_NUMBER_TOO_LARGE_ERROR                     -55
#define             ADD_DEPTH_RECORD_OUT_OF_LIMITS_ERROR            -56
#define             WRITE_BIN_HEADER_NEGATIVE_BIN_DIMENSION         -57
#define             PFM_GEO_DISTANCE_NOT_GEOGRAPHIC_ERROR           -58
#define             PFM_GEO_DISTANCE_LATLON_PFM_ERROR               -59
#define             PFM_GEO_DISTANCE_ALLOCATE_ERROR                 -60
#define             PFM_GEO_DISTANCE_OUT_OF_BOUNDS                  -61
#define             OPEN_LIST_FILE_NEWER_VERSION_ERROR              -62
#define             OPEN_HANDLE_FILE_OPEN_ERROR                     -63


/*!
  Validity bit meanings, if set...


  - In depth record:
      - PFM_MANUALLY_INVAL    =  Point has been manually marked as invalid.
      - PFM_FILTER_INVAL      =  Point has been automatically marked as invalid.
      - PFM_SELECTED_SOUNDING =  Point is a selected sounding (min, max, avg, depends on application).
      - PFM_SUSPECT           =  Point has been marked as suspect.
      - PFM_SELECTED_FEATURE  =  Point is on or near a selected feature.
      - PFM_MODIFIED          =  Point has been modified since being loaded into the PFM structure.
      - PFM_USER_01           =  User flag 01.
      - PFM_USER_02           =  User flag 02.
      - PFM_USER_03           =  User flag 03.
      - PFM_CHECKED           =  N/A for depth record.
      - PFM_DATA              =  N/A for depth record.
      - PFM_DELETED           =  Point is treated as deleted/non-existent by API.  This can be
                                 cleared if the data was incorrectly marked.
      - PFM_USER_04           =  User flag 04.
      - PFM_INTERPOLATED      =  N/A for depth record.
      - PFM_REFERENCE         =  Used for reference only, not used for computing surfaces.  This is
                                 useful for things like lead line data or UNISIPS depth data.
      - PFM_VERIFIED          =  N/A for depth record.
      - PFM_USER_05           =  User flag 05.  SAIC will use for preserving depth records flagged
                                 when IHO Order for TPE is exceeded.
      - PFM_DESIGNATED_SOUNDING =  Indicates the depth record overrides all hypotheses.  Not all
                                   'designated' soundings will be 'features'.

  - In bin record:
      - PFM_MANUALLY_INVAL    =  Some data in this bin has been manually marked as invalid.
      - PFM_FILTER_INVAL      =  Some data in this bin has been automatically marked as invalid.
      - PFM_SELECTED_SOUNDING =  This bin contains at least one selected sounding.
      - PFM_SUSPECT           =  This bin contains at least one suspect point.
      - PFM_SELECTED_FEATURE  =  This bin contains at least one point on or near a selected feature.
      - PFM_MODIFIED          =  This bin contains at least one point that has been modified since being
                                 loaded into the PFM structure.
      - PFM_USER_01           =  This bin contains at least one point marked PFM_USER_01.
      - PFM_USER_02           =  This bin contains at least one point marked PFM_USER_02.
      - PFM_USER_03           =  This bin contains at least one point marked PFM_USER_03.
      - PFM_CHECKED           =  This bin has been marked as checked.
      - PFM_DATA              =  This bin has at least one valid point.
      - PFM_DELETED           =  This bin contains at least one point marked PFM_DELETED.
      - PFM_USER_04           =  This bin contains at least one point marked PFM_USER_04.
      - PFM_INTERPOLATED      =  The average surface in this bin has been interpolated.
      - PFM_REFERENCE         =  This bin contains reference data.
      - PFM_VERIFIED          =  This bin has been verified.  At present this flag will force
                                 PFM_CHECKED.
      - PFM_USER_05             =  This bin contains at least one point marked PFM_USER_05.
      - PFM_DESIGNATED_SOUNDING =  This bin contains a designated sounding.  The depth and uncertainty
                                   is set to match a specific sounding (Hydrographer override).<br><br>
*/

#define       PFM_MANUALLY_INVAL       1       /*!< 00 0000 0000 0000 0001 */
#define       PFM_FILTER_INVAL         2       /*!< 00 0000 0000 0000 0010 */
#define       PFM_SELECTED_SOUNDING    4       /*!< 00 0000 0000 0000 0100 */
#define       PFM_SUSPECT              8       /*!< 00 0000 0000 0000 1000 */
#define       PFM_SELECTED_FEATURE     16      /*!< 00 0000 0000 0001 0000 */
#define       PFM_MODIFIED             32      /*!< 00 0000 0000 0010 0000 */
#define       PFM_USER_01              64      /*!< 00 0000 0000 0100 0000 */
#define       PFM_USER_02              128     /*!< 00 0000 0000 1000 0000 */
#define       PFM_USER_03              256     /*!< 00 0000 0001 0000 0000 */
#define       PFM_CHECKED              512     /*!< 00 0000 0010 0000 0000 */
#define       PFM_DATA                 1024    /*!< 00 0000 0100 0000 0000 */
#define       PFM_DELETED              2048    /*!< 00 0000 1000 0000 0000 */
#define       PFM_USER_04              4096    /*!< 00 0001 0000 0000 0000 */
#define       PFM_INTERPOLATED         8192    /*!< 00 0010 0000 0000 0000 */
#define       PFM_REFERENCE            16384   /*!< 00 0100 0000 0000 0000 */
#define       PFM_VERIFIED             32768   /*!< 00 1000 0000 0000 0000 */
#define       PFM_USER_05              65536   /*!< 01 0000 0000 0000 0000 */
#define       PFM_DESIGNATED_SOUNDING  131072  /*!< 10 0000 0000 0000 0000 */

#define       PFM_INVAL                3       /*!< 00 0000 0000 0000 0011 */
#define       PFM_SELECTED             20      /*!< 00 0000 0000 0001 0100 */
#define       PFM_USER                 70080   /*!< 01 0001 0001 1100 0000 */

#define       PFM_VAL_MASK             262143  /*!< 11 1111 1111 1111 1111 */


/*!  Coverage map bits.  <br><br>  */

#define       COV_DATA                 128     /*!< 1000 0000 */
#define       COV_CHECKED              64      /*!< 0100 0000 */
#define       COV_VERIFIED             32      /*!< 0010 0000 */
#define       COV_SURVEYED             16      /*!< 0001 0000 ; indicates a bin that should have data */
#define       COV_GAP                  8       /*!< 0000 1000 ; indicates a bin that should but does NOT have data */


/*  Pre-version 4.1  */

/**************************************************************************/

#define             PFM_CLASS_1             PFM_USER_01
#define             PFM_CLASS_2             PFM_USER_02
#define             PFM_NBR                 PFM_USER_03

/**************************************************************************/



/*!  Data types  <br><br>  */

#define PFM_UNDEFINED_DATA      0  /*!<  Undefined */
#define PFM_MRG_DATA            0  /*!<  Retired */
#define PFM_CHRTR_DATA          1  /*!<  CHRTR(2) format (.fin, .ch2), records begin with 0, record is row (south
                                         to north), subrecord is column (west to east)  */
#define PFM_GSF_DATA            2  /*!<  Records begin with 1, subrecords (beams) begin with 1 */
#define PFM_SHOALS_OUT_DATA     3  /*!<  Records begin with 0, subrecords undefined but may be used for anything else */
#define PFM_SHOALS_XY2_DATA     0  /*!<  Retired */
#define PFM_CHARTS_HOF_DATA     4  /*!<  Records begin with 1, subrecord 0 is primary (not to be confused with first) return,
                                         subrecord 1 is secondary (not to be confused with second) return.  This is a replacement for
                                         PFM_SHOALS_1K_DATA that alleviates the need for "swapping" primary and secondary returns.  */
#define PFM_NAVO_ASCII_DATA     5  /*!<  Records begin with 0, no subrecords */
#define PFM_HTF_DATA            6
#define PFM_DEMO_DATA           0  /*!<  Retired */
#define PFM_WLF_DATA            7  /*!<  Waveform LIDAR Format, records begin at 0, no subrecords */
#define PFM_DTM_DATA            8
#define PFM_HDCS_DATA           9
#define PFM_ASCXYZ_DATA         10
#define PFM_CNCBIN_DATA         11
#define PFM_STBBIN_DATA         12
#define PFM_XYZBIN_DATA         13
#define PFM_OMG_DATA            14
#define PFM_CNCTRACE_DATA       15
#define PFM_NEPTUNE_DATA        16
#define PFM_SHOALS_1K_DATA      17 /*!<  Records begin with 1, subrecords undefined but may be used for anything else */
#define PFM_SHOALS_ABH_DATA     18
#define PFM_SURF_DATA           19
#define PFM_SMF_DATA            20
#define PFM_VISE_DATA           21
#define PFM_PFM_DATA            22
#define PFM_MIF_DATA            23
#define PFM_SHOALS_TOF_DATA     24 /*!<  Records begin with 1, subrecord 0 is first return, subrecord 1 is last return
                                         if first return is within 0.05 of last return, first return is not loaded.  On
                                         unload, if first return is within 0.05 of last return and last return is invalid,
                                         first return is marked invalid.  */
#define PFM_UNISIPS_DEPTH_DATA  25 /*!<  Records begin with 0, this is UNISIPS center depth */
#define PFM_HYD93B_DATA         26
#define PFM_LADS_DATA           27
#define PFM_HS2_DATA            28
#define PFM_9COLLIDAR           29
#define PFM_FGE_DATA            30
#define PFM_PIVOT_DATA          31
#define PFM_MBSYSTEM_DATA       32
#define PFM_LAS_DATA            33
#define PFM_BDI_DATA            34
#define PFM_NAVO_LLZ_DATA       35 /*!<  Records begin with 0, no subrecords */
#define PFM_LADSDB_DATA         36
#define PFM_DTED_DATA           37 /*!<  Records begin with 0, record is lon line number, subrecords begin with 0, 
                                         subrecord is lat point number.  */
#define PFM_HAWKEYE_HYDRO_DATA  38 /*!<  Records begin with 0, no subrecords.  */
#define PFM_HAWKEYE_TOPO_DATA   39 /*!<  Records begin with 0, no subrecords.  */
#define PFM_BAG_DATA            40 /*!<  Records begin with 0, record is row number, subrecords begin with 0, 
                                         subrecord is column number.  */
#define PFM_CZMIL_DATA          41 /*!<  Coastal Zone Mapping and Imaging LIDAR place holder  */

#define PFM_DATA_TYPES          42 /*!<  Total number of PFM data types  */


/*!  typedef for progress callback  */

typedef void (*PFM_PROGRESS_CALLBACK) (int state, int percent);



/*!
  The following data type descriptive strings are set in open_pfm_file.
  THIS IS BAD BAD BAD ... and probably should be changed
*/

__PFM_EXTERN__ NV_CHAR               *PFM_data_type[PFM_DATA_TYPES];



/*!
   - PFM Public API functions

   - Internal library functions are found in pfmP.h
*/

NV_INT32 *get_bin_density( void );
NV_FLOAT32 get_avg_per_bin( void );
NV_FLOAT32 get_stddev_per_bin( void );

NV_INT32 get_cache_hits( void );
NV_INT32 get_cache_misses( void );
NV_INT32 get_cache_flushes( void );
NV_INT32 get_cache_size( NV_INT32 hnd );
NV_INT32 get_cache_peak_size( NV_INT32 hnd );
NV_INT32 get_cache_max_size( NV_INT32 hnd );

void set_cache_size (NV_INT32 max_rows, NV_INT32 max_cols, NV_INT32 row, NV_INT32 col);
void set_cache_size_max (NV_INT32 max);
void set_use_cov_flag (NV_INT32 flag);
NV_INT32 set_cached_cov_flag (NV_INT32 hnd, NV_I32_COORD2 coord, NV_U_BYTE flag);
NV_INT32 flush_cov_flag (NV_INT32 hnd);

NV_INT32 open_cached_pfm_file (PFM_OPEN_ARGS *open_args);
void close_cached_pfm_file (NV_INT32 hnd);
NV_INT32 read_cached_bin_record (NV_INT32 hnd, NV_I32_COORD2 coord, BIN_RECORD_SUMMARY **bin_summary);
NV_INT32 write_cached_bin_record (NV_INT32 hnd, BIN_RECORD_SUMMARY *bin_summary );
NV_INT32 add_cached_depth_record (NV_INT32 hnd, DEPTH_RECORD *depth);

NV_INT32 recompute_cached_bin_values (NV_INT32 hnd, BIN_RECORD *bin, NV_U_INT32 mask, DEPTH_RECORD *depth);

NV_INT32 flush_bin_cache (NV_INT32 hnd);
NV_INT32 destroy_bin_cache (NV_INT32 hnd);
NV_INT32 close_bin_cached (NV_INT32 hnd);

NV_INT32 dump_cached_record (NV_INT32 hnd, NV_I32_COORD2 coord);
NV_INT32 dump_cached_records (NV_INT32 hnd);
NV_INT32 get_cached_depth_records (NV_INT32 hnd, NV_I32_COORD2 coord, DEPTH_RECORD **depth, BIN_RECORD_SUMMARY **bin_summary);
NV_INT32 put_cached_depth_records (NV_INT32 hnd, NV_I32_COORD2 coord, DEPTH_RECORD **depth, BIN_RECORD_SUMMARY **bin_summary);
NV_INT32 read_cached_depth_records (NV_INT32 hnd, BIN_RECORD_SUMMARY **bin_summary);


void pfm_substitute (NV_CHAR *path);
void get_version (NV_CHAR *version);
NV_INT32 read_bin_header (NV_INT32 hnd, BIN_HEADER *head);
NV_INT32 write_bin_header (NV_INT32 hnd, BIN_HEADER *head, NV_BOOL init);
void set_pfm_data_types ();
NV_INT32 open_pfm_file (PFM_OPEN_ARGS *open_args);
NV_INT32 open_existing_pfm_file (PFM_OPEN_ARGS *open_args);
void close_pfm_file (NV_INT32 hnd);
NV_INT32 read_list_file (NV_INT32 hnd, NV_INT16 file_number, NV_CHAR *path, NV_INT16 *type);
NV_INT16 write_list_file (NV_INT32 hnd, NV_CHAR *path, NV_INT16 type);
void delete_list_file (NV_INT32 hnd, NV_INT32 file_number);
void restore_list_file (NV_INT32 hnd, NV_INT32 file_number);
void update_target_file (NV_INT32 hnd, NV_CHAR *lst, NV_CHAR *path);
void update_mosaic_file (NV_INT32 hnd, NV_CHAR *lst, NV_CHAR *path);
void get_target_file (NV_INT32 hnd, NV_CHAR *lst, NV_CHAR *path);
void get_mosaic_file (NV_INT32 hnd, NV_CHAR *lst, NV_CHAR *path);
NV_INT32 read_bin_record_index (NV_INT32 hnd, NV_I32_COORD2 coord, BIN_RECORD *bin);
void compute_index (NV_F64_COORD2 xy, NV_I32_COORD2 *coord, BIN_HEADER bin);
void compute_index_ptr (NV_F64_COORD2 xy, NV_I32_COORD2 *coord, BIN_HEADER *bin);
NV_INT32 read_bin_record_xy (NV_INT32 hnd, NV_F64_COORD2 xy, BIN_RECORD *bin);
NV_INT32 read_bin_row (NV_INT32 hnd, NV_INT32 length, NV_INT32 row, NV_INT32 column, BIN_RECORD a[]);
NV_INT32 read_bin_record_validity_index (NV_INT32 hnd, NV_I32_COORD2 coord, NV_U_INT32 *validity);
NV_INT32 read_cov_map_index (NV_INT32 hnd, NV_I32_COORD2 coord, NV_U_BYTE *cov);
NV_INT32 write_cov_map_index (NV_INT32 hnd, NV_I32_COORD2 coord, NV_U_BYTE cov);
NV_INT32 read_coverage_map_index (NV_INT32 hnd, NV_I32_COORD2 coord, NV_BOOL *data, NV_BOOL *checked);
NV_INT32 write_bin_record_index (NV_INT32 hnd, BIN_RECORD *bin);
NV_INT32 write_bin_record_xy (NV_INT32 hnd, BIN_RECORD *bin);
NV_INT32 write_bin_record_validity_index (NV_INT32 hnd, BIN_RECORD *bin, NV_U_INT32 mask);
NV_INT32 read_depth_array_index (NV_INT32 hnd, NV_I32_COORD2 coord, DEPTH_RECORD **depth_array, NV_INT32 *numrecs);
NV_INT32 read_depth_array_xy (NV_INT32 hnd, NV_F64_COORD2 xy, DEPTH_RECORD **depth_array, NV_INT32 *numrecs);
NV_INT32 read_bin_depth_array_index (NV_INT32 hnd, BIN_RECORD *bin, DEPTH_RECORD **depth_array);
NV_INT32 read_bin_depth_array_xy (NV_INT32 hnd, NV_F64_COORD2 xy, BIN_RECORD *bin, DEPTH_RECORD **depth_array);
NV_INT32 update_depth_record_index (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 update_depth_record_xy (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 update_depth_record_index_ext_flags (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 update_depth_record_xy_ext_flags (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 add_depth_record_index (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 add_depth_record_xy (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 recompute_bin_values_index (NV_INT32 hnd, NV_I32_COORD2 coord, BIN_RECORD *bin, NV_U_INT32 mask);
NV_INT32 recompute_bin_values_xy (NV_INT32 hnd, NV_F64_COORD2 xy, BIN_RECORD *bin, NV_U_INT32 mask);
NV_INT32 recompute_bin_values_from_depth_index (NV_INT32 hnd, BIN_RECORD *bin, NV_U_INT32 mask, DEPTH_RECORD *depth_array);
NV_INT32 change_depth_record_index (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 change_depth_record_nomod_index (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 change_bin_attribute_records_index (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_INT32 change_attribute_record_index (NV_INT32 hnd, DEPTH_RECORD *depth);
NV_CHAR *read_line_file (NV_INT32 hnd, NV_INT16 line_number);
NV_INT16 write_line_file (NV_INT32 hnd, NV_CHAR *line);
NV_CHAR *pfm_error_str (NV_INT32 status);
void pfm_error_exit (NV_INT32 status);
NV_INT32 bin_inside (BIN_HEADER bin, NV_F64_COORD2 xy);
NV_INT32 bin_inside_ptr (BIN_HEADER *bin, NV_F64_COORD2 xy);
NV_INT16 get_next_list_file_number (NV_INT32 hnd);
NV_INT16 get_next_line_number (NV_INT32 hnd);
NV_BOOL check_flag (NV_U_BYTE field, NV_U_BYTE flag);
void set_flag (NV_U_BYTE *field, NV_U_BYTE flag);
void clear_flag (NV_U_BYTE *field, NV_U_BYTE flag);
void pfm_register_progress_callback (PFM_PROGRESS_CALLBACK progressCB);
NV_INT32 get_data_extents (NV_INT32 hnd, NV_I32_COORD2 *min_coord, NV_I32_COORD2 *max_coord);
void compute_center_xy (NV_F64_COORD2 *xy, NV_I32_COORD2 coord, BIN_HEADER *bin);
NV_INT32 pfm_geo_distance (NV_INT32 hnd, NV_FLOAT64 lat0, NV_FLOAT64 lon0, NV_FLOAT64 lat1, NV_FLOAT64 lon1, NV_FLOAT64 *distance);
NV_INT32 pfm_get_io_type (NV_INT32 hnd);


#ifdef  __cplusplus
}
#endif


#endif
