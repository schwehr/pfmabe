
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


  /*! \mainpage Pure File Magic (PFM) World Data Base (WDB) API

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


       <br><br>\section author Author
       
       - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)
       - Date:        02/11/10


       <br><br>\section date Introduction

       The PFM World Data Base (PFMWDB) is an attempt to keep point cloud data readily accessible
       without using up too much disk storage.  The bulk of the PFMWDB consists of one-degree PFM
       structures (a .pfm file and .pfm.data directory).  The naming convention for the files is
       HDDhddd.pfm where H is the latitude hemisphere (N or S), DD is the latitude degrees of the
       southwest corner of the one-degree cell (note that there will be no S00 files), h is the
       longitude hemisphere (E or W), and ddd is the longitude degrees of the southwest corner of
       the one-degree cell (there will be no W000 files).  Each of these files uses a .1 degree
       bin size, a minimum Z value of -9000 meters, a maximum Z value of 11500 meters, and a Z
       scale of one centimeter.  Due to the bin size the best resolution in lat and lon is 185.2
       meters divided by 4095.0 (0.0452 meters).  For each point stored in a PFM there are five
       attributes.  These are, the NAVO security key (from security.cfg see explanation below),
       the time in minutes from January 01 1970, the vertical datum key (see datums below), the
       WGS84 ellipsoid offset, and either a nominal depth or a LIDAR intensity value (in depths
       less than 200 meters the nominal depth is the same as the true depth so we re-used this
       slot for intensity for LIDAR data since it will never be deeper than 200 meters).  There
       is one attribute attached to each bin in the PFMs.  This is a combination of all possible
       classifications (not security keys) of data within the PFM.  Due to the bit-packed,
       compressed form in which data is stored within a PFM structure, each single data point
       stored in the PFMWDB uses up, on average, approximately 32.5 bytes.  For example, with
       overhead, 100 billion data points would require about 3.3TB of storage space.  More
       information about the PFM data structure is available in the
       <a href="../../PFM_API_Documentation/html/index.html"><b>PFM API documentation</b></a>.


       <br><br>\section map PFMWDB.map File Structure

       The PFMWDB.map file keeps track of highest classification, checked, verified, deleted, and
       number of records for each of 64,800 possible one-degree PFM files in the PFM World Data
       Base.  In addition, a list (ASCII, line-feed terminated) of ALL files used in ANY of the
       one-degree PFMs is appended after the 64,800th record.  This allows us to pre-scan for
       already loaded data files in the pfmWDBLoad application.  The PFMWDB.map one-degree records
       are bit packed.  Each file in the file list is in ASCII with a trailing line-feed.  The
       header of the file is PFMWDBMAP_HEADER_SIZE bytes of ASCII and looks something like this:

       <br>

       [VERSION] = PFM Software - pfmWDB library V1.00 - 02/11/10                  <br>
       [HEADER SIZE] = 65536                                                       <br>
       [CREATION DATE/TIME] = Tue Mar 23 14:17:46 2010                             <br>
       [MODIFICATION DATE/TIME] = Tue Mar 23 15:55:22 2010                         <br>
       [SECURITY CLASSIFICATION] = *** UNCLASSIFIED PUBLIC RELEASE ***             <br>
       [NUMBER OF RECORDS] = 2398535                                               <br>
       [NUMBER OF FILES] = 7                                                       <br>
       [STATUS BITS] = 24                                                          <br>
       [RECORDS BITS] = 40                                                         <br>
       [END OF HEADER]                                                             <br>

       <br>

       The remainder of the header (after [END OF HEADER]) is zero filled.

       <br>

       Each of the 64,800 one-degree cell records is either zero filled, if there is no data in
       the cell, or contains the information described in the PFMWDBMAP_RECORD definition.
       The data is bit packed in PFMWDBMAP_HEADER.status_bits for the status information and
       PFMWDBMAP_HEADER.records_bits for the record count.  The way that the classification and
       other status information is bit packed into the PFMWDBMAP_HEADER.status_bits field of the
       buffer is described in the "Status information definition" section below.  The buffer is
       written as a (PFMWDBMAP_HEADER.status_bits + PFMWDBMAP_HEADER.records_bits) / 8 byte
       buffer.  If the result of the above computation is not exactly divisible by eight we
       add one to the result.  This gives us enough bytes to handle any overflow of bits.  When
       you read a record it is unpacked into the PFMWDBMAP_RECORD structure.


       <br><br>\section security Security Key File (security.cfg)

       If you want to use the PFMWDB suite of programs to build and manipulate a PFMWDB you will
       have to have a file called security.cfg located in the directory (or folder for the Windows
       inclined) that is pointed to by the $PFMWDB environment variable.  If all of the data you
       want to store is unclassified public domain you can just make a file called security.cfg
       that contains the following two records:

       <br>

       SECURITY_KEY,HANDLING_INSTRUCTIONS,DOWNGRADE_AUTHORITY,DISTRIBUTION_LIMITATIONS,DISTRIBUTION_LIMIT_REASON,CLASSIFICATION_REF_ENCLOSURE,EXEMPTION_CATEGORY,CLASSIFICATION,FULL_DISTRIBUTION_CODE,DISTRIBUTION_STATEMENT_PARTA,DISTRIBUTION_STATEMENT_PARTB 100,,,PUBLIC RELEASE,,,,P,A, *** UNCLASSIFIED PUBLIC DOMAIN **** DISTRIBUTION STATEMENT A: APPROVED FOR PUBLIC RELEASE;, DISTRIBUTION IS UNLIMITED

       <br>

       You can always make up your own records with handling instructions, downgrade authority, and
       the rest if you like ;-)


       <br><br>\section search Searching the Doxygen Documentation

       The easiest way to find documentation for a particular C function is by looking for it in the
       <a href="globals.html"><b>Files>Globals>All</b></a> tab.  So, for instance, if you are looking for
       documentation on the <b><i>PFMWDBMAP_read_file</i></b> function you would go to
       <a href="globals.html"><b>Files>Globals>All</b></a> and click on the <b><i>P</i></b> tab then look for
       <b><i>PFMWDBMAP_read_file</i></b>.  When you find <b><i>PFMWDBMAP_read_file</i></b>, just
       click on the .h file associated with it.  If you would like to see the history of changes that have
       been made to the PFMWDB API you can look at pfmWDB_version.h.<br><br>

  */




#ifndef __PFMWDB_H__
#define __PFMWDB_H__

#ifdef  __cplusplus
extern "C" {
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "nvtypes.h"
#include "get_string.h"
#include "ngets.h"
#include "bit_pack.h"


#define     PFMWDBMAP_HEADER_SIZE                 65536        /*!<  PFMWDB.map ASCII header size in bytes  */
#define     PFMWDB_MAX_SECURITY_KEY               1023         /*!<  Maximum security key value  */
#define     PFMWDB_NULL_TIME                      -64000000.0  /*!<  Null time value (no time available for record)  */
#define     PFMWDB_NULL_Z_OFFSET                  327.00       /*!<  Null Z offset value  */
#define     PFMWDB_STATUS_BITS                    24           /*!<  Number of bits used to store status information  */
#define     PFMWDB_RECORDS_BITS                   40           /*!<  Number of bits used to store a record number  */


  /*!  Vertical datum definitions.  These are direct copies of the Generic Sensor Format (GSF) vertical datum definitions
       you can use either the GSF definitions (e.g. GSF_V_DATUM_MLLW) or these to define the vertical datum.<br>  */

#define     PFMWDB_V_DATUM_UNKNOWN                1     /*!< Unknown vertical datum */
#define     PFMWDB_V_DATUM_MLLW                   2     /*!< Mean lower low water */
#define     PFMWDB_V_DATUM_MLW                    3     /*!< Mean low water */
#define     PFMWDB_V_DATUM_ALAT                   4     /*!< Aprox Lowest Astronomical Tide */
#define     PFMWDB_V_DATUM_ESLW                   5     /*!< Equatorial Springs Low Water */
#define     PFMWDB_V_DATUM_ISLW                   6     /*!< Indian Springs Low Water */
#define     PFMWDB_V_DATUM_LAT                    7     /*!< Lowest Astronomical Tide */
#define     PFMWDB_V_DATUM_LLW                    8     /*!< Lowest Low Water */
#define     PFMWDB_V_DATUM_LNLW                   9     /*!< Lowest Normal Low Water */
#define     PFMWDB_V_DATUM_LWD                    10    /*!< Low Water Datum */
#define     PFMWDB_V_DATUM_MLHW                   11    /*!< Mean Lower High Water */
#define     PFMWDB_V_DATUM_MLLWS                  12    /*!< Mean Lower Low Water Springs */
#define     PFMWDB_V_DATUM_MLWN                   13    /*!< Mean Low Water Neap */
#define     PFMWDB_V_DATUM_MSL                    14    /*!< Mean Sea Level */


  /*!  Status information definition.  The default for number of status bits is 24.  We're only using 7 for
       now but this way we don't have to rebuild if we add a couple.<br>  */

#define     PFMWDB_P_MASK                         0x000001   /*!<  Bin contains unclassified public domain data  */
#define     PFMWDB_U_MASK                         0x000002   /*!<  Bin contains unclassified restricted data  */
#define     PFMWDB_C_MASK                         0x000004   /*!<  Bin contains confidential data  */
#define     PFMWDB_S_MASK                         0x000008   /*!<  Bin contains secret data  */
#define     PFMWDB_CHECKED_MASK                   0x000010   /*!<  Bin is set as checked  */
#define     PFMWDB_VERIFIED_MASK                  0x000020   /*!<  Bin is set as verified  */
#define     PFMWDB_DELETED_MASK                   0x000040   /*!<  Bin contains data marked for deletion  */


  /*!  Shift/position values for the status bits.  These are used to move the requisite bit into the low order position.  */

#define     PFMWDB_P_SHIFT                        0
#define     PFMWDB_U_SHIFT                        1
#define     PFMWDB_C_SHIFT                        2
#define     PFMWDB_S_SHIFT                        3
#define     PFMWDB_CHECKED_SHIFT                  4
#define     PFMWDB_VERIFIED_SHIFT                 5
#define     PFMWDB_DELETED_SHIFT                  6


  /*!  PFMWDB.map header structure.  */

  typedef struct
  {
    NV_CHAR         version[128];                 /*!<  pfmWDB version information  */
    NV_INT32        header_size;                  /*!<  Header size in bytes  */
    NV_CHAR         creation_date_time[50];       /*!<  File creation date and time  */
    NV_CHAR         modification_date_time[50];   /*!<  File modification date and time  */
    NV_CHAR         classification[50];           /*!<  Highest security classification of data in PFMWDB  */
    NV_INT64        records;                      /*!<  Number of records in PFMWDB  */
    NV_INT32        files;                        /*!<  Number of unique files in PFMWDB  */
    NV_INT32        status_bits;                  /*!<  Number of bits used to store classification and other flags (TBD)  */
    NV_INT32        records_bits;                 /*!<  Number of bits used to store the record count  */
  } PFMWDBMAP_HEADER;


  /*!  PFMWDB.map record structure.  */

  typedef struct
  {
    NV_BOOL         P;                            /*!<  Set if the associated PFM contains PUBLIC DOMAIN data  */
    NV_BOOL         U;                            /*!<  Set if the associated PFM contains UNCLASSIFIED data  */
    NV_BOOL         C;                            /*!<  Set if the associated PFM contains CONFIDENTIAL data  */
    NV_BOOL         S;                            /*!<  Set if the associated PFM contains SECRET data  */
    NV_BOOL         checked;                      /*!<  Set if the associated PFM has been marked as checked  */
    NV_BOOL         verified;                     /*!<  Set if the associated PFM has been marked as verified  */
    NV_BOOL         deleted;                      /*!<  Set if the associated PFM contains PFM_DELETED files  */
    NV_INT64        records;                      /*!<  Number of records in the associated PFM  */
  } PFMWDBMAP_RECORD;



  /*  Public API function calls.  */

  NV_BOOL PFMWDBMAP_open (PFMWDBMAP_HEADER *header);
  void PFMWDBMAP_close ();
  NV_BOOL PFMWDBMAP_write_header (PFMWDBMAP_HEADER *header);
  PFMWDBMAP_RECORD PFMWDBMAP_read_record (NV_INT32 lat, NV_INT32 lon);
  void PFMWDBMAP_write_record (NV_INT32 lat, NV_INT32 lon, PFMWDBMAP_RECORD record);
  void PFMWDBMAP_append_file (const NV_CHAR *filename, NV_INT16 type, NV_INT16 security, NV_INT16 datum);
  NV_BOOL PFMWDBMAP_read_file (NV_CHAR *filename, NV_BOOL *deleted, NV_INT16 *type, NV_INT16 *security, NV_INT16 *datum);
  NV_BOOL PFMWDBMAP_mark_file (const NV_CHAR *filename, NV_INT16 idelete, NV_INT16 itype __attribute__ ((unused)), NV_INT16 isecurity, NV_INT16 idatum);
  NV_BOOL PFMWDBMAP_delete_file (const NV_CHAR *filename);
  NV_BOOL PFMWDBMAP_restore_file (const NV_CHAR *filename);
  NV_BOOL PFMWDBMAP_flag_file (const NV_CHAR *filename, NV_INT16 security, NV_INT16 datum);
  NV_BOOL PFMWDBMAP_file_cleanup ();
  void PFMWDBMAP_recompute ();
  NV_BOOL PFMWDB_is_read_only ();
  FILE *PFMWDB_open_lock_file ();
  void PFMWDB_close_lock_file ();
  NV_BOOL PFMWDB_is_locked ();



#ifdef  __cplusplus
}
#endif

#endif
