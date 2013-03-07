
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



#ifndef __CZMIL_H__
#define __CZMIL_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS


  /*  Preparing for language translation using GNU gettext at some point in the future.  */

#define _(String) (String)
#define N_(String) String

  /*
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
  */



  /*  Windoze insanity.  Note that there may be nothing defined for any of these DLL values.  When building statically on
      Windows I strip out the __declspec (dllexport) and __declspec (dllimport) definitions before I copy the .h file to
      the central include location.  That's so I don't have to modify a bunch of application Makefiles to check for
      static build and set the CZMIL_STATIC option.  Of course, on Linux/UNIX we don't have to do anything to these ;-)  */

#ifdef CZMIL_DLL_EXPORT
#  define CZMIL_DLL __declspec(dllexport)
#else
#  ifdef NVWIN3X
#    ifdef CZMIL_STATIC
#      define CZMIL_DLL
#    else
#      define CZMIL_DLL __declspec(dllimport)
#    endif
#  else
#    define CZMIL_DLL
#  endif
#endif


#endif /*  DOXYGEN_SHOULD_SKIP_THIS  */


#include "czmil_nvtypes.h"
#include "czmil_macros.h"


  /*! \mainpage CZMIL Data Formats

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


       <br><br>\section intro Introduction

       The Coastal Zone Mapping and Imaging LiDAR (CZMIL) system has the ability to collect a very large amount of data
       in a very short time. The system fires the laser 10,000 times per second and generates nine waveforms of up to
       1,500 ten bit samples each for every shot. Each waveform can be processed to generate up to four returns with each
       return having a unique latitude, longitude, elevation, horizontal uncertainty, vertical uncertainty, and status.
       Due to the size of the data sets that are generated by the CZMIL system, the data must be stored in an internally
       compressed format. Since the internally compressed records are variable length there must be an index file
       associated with the data files. Also, because the data is compressed and bit-packed, this document will devote
       less attention to the actual internal format of the files and more attention to the CZMIL Application Programming
       Interface (API) that allows you to read and write the CZMIL data files. Throughout this document we will refer to
       data types of the form NV_name. These data types are defined in czmil_nvtypes.h and should be fairly obvious.
       When working on code in the CZMIL API always use these data types instead of native C data types (like int).
       This helps to keep the API portable to different operating systems.

       At present there are three CZMIL files defined. These are the CZMIL point cloud (CXY) file, the CZMIL waveform
       file (CWF), and the CZMIL index file (CDX).



       <br><br>\section sec1 CZMIL point cloud (CXY) file format description

       The CXY file consists of an ASCII header and variable length, compressed, bit-packed binary records. The header
       will contain tagged ASCII fields. The tags are enclosed in brackets ([ and ]) for single line values and in braces
       ({ and }) for multi-line values. The reason that we don't use XML for the header is that CZMIL files are going to
       be very large while the CZMIL headers are only a few tens of thousands of bytes long. If a user (or an operating
       system) mistakenly thinks that a CZMIL file is an XML file and tries to open the file with an XML reader it will
       crash, but usually not before it hangs the system for a while. The other reason for not using XML is that XML is
       really designed for more complicated data. It also usually requires a schema and/or external information. We
       don't really need all that complexity. Some of the fields that appear in the header, such as bit lengths, are
       used internally in packing and unpacking the records. These fields are not available to the application
       programmer in the CZMIL_CXY_Header structure. The easiest way to understand what is stored in the header is to
       look at the CZMIL_CXY_Header structure.

       The header key fields, in parentheses, defined for each member define how and when each field is populated. The
       (a) key means that the field is populated and/or modified by the API and does not need to be set by the
       application program. A key of (c) means that field needs to be set by the application program prior to creation
       of the file. An (m) key indicates that the field is modifiable at creation time or later using the
       czmil_update_cxy_header function.  The modifiable fields are all optional with the exception of the
       local_vertical_datum field. The local_vertical_datum field will be set to 0 (CZMIL_V_DATUM_UNDEFINED) by default.
       The wkt and the comments field are multi-line text fields. That is, they can contain line-feeds.  Of the
       CZMIL_CXY_Header fields that are needed for creation of the file only the null_z_value and wkt are actually
       required.

       Each record in the CXY file is stored in a bit-packed, variable length record. The values in each record are
       physically stored as unsigned, scaled integers. They are packed and unpacked by the API and the actual format of
       each value is defined by the extra bit field and scale values stored in the header. How they are stored is really
       irrelevant to the application programmer. All that is required is to use the czmil_write_cxy_record and/or
       czmil_read_cxy_record functions to write or read the contents of the CZMIL CXY record. The CXY record is defined
       in the CZMIL_CXY_Data2 structure.  Unfortunately, the CZMIL_CXY_Data2 structure contains a CZMIL_Channel_Data2
       structure, which, in turn, contains a CZMIL_Return_Data structure.  You can look at the czmil.h file to get a
       full picture of the structures.



       <br><br>\section sec2 CZMIL waveform (CWF) file format description

       The CWF file will consist of an ASCII header and variable length, compressed, bit-packed binary records. The
       header will contain tagged ASCII fields in the same format as described for the CXY file header. It will also
       have even more bit size, scaling, and offset fields than the CXY file header.  Note that there are no fields that
       are required to be populated on creation. In fact, the CZMIL_CWF_Header structure is not available to the
       application program.  It is really just used to generate the ASCII header so that the file contents can be
       determined without having to use a special program to open the file.  There are a few modifiable fields that
       will be, by default, copied from the CZMIL_CXY_Header. These may be modified by an application program at a later
       time using the czmil_update_cwf_header function.

       Here's the fun part.  How are we actually compressing the waveform data?  It's not really all that important that
       the application programmer actually understands how the waveform data is compressed or how it is stored on disk
       but, in the interest of completeness, the following is an explanation (of sorts):

       The data will be compressed using first or second difference coding or, in the case of the shallow channels,
       using a difference from the central shallow channel (channel[1]).  There will be a waveform block header containing
       the full block buffer size, the timestamp, and the scan angle.  After that will come the data presence flags and 
       the compressed 64 sample packets.  The data presence flags indicate whether there is data in any given 64 sample
       packet.  This is the structure of a full waveform data block:

       - Full waveform block header:
               - 16 bits        =    Size of the entire compressed waveform data block (all packets of all waveforms)
               - 52 bits        =    Timestamp (microseconds from 01-01-1970)
               - 21 bits        =    Scan angle in degrees

       - CZMIL_MAX_PACKETS bits  =    Deep channel 64 sample packet data presence flags

       - Up to CZMIL_MAX_PACKETS (based on above flags) deep channel compressed 64 sample waveform packets

       - For shallow channels 0 through 6:
               - Shallow channel 64 sample packet data presence flags
               - Up to CZMIL_MAX_PACKETS (based on above flags) shallow channel compressed 64 sample waveform packets

       - CZMIL_MAX_PACKETS bits  =    IR channel 64 sample packet data presence flags

       - Up to CZMIL_MAX_PACKETS (based on above flags) IR channel compressed 64 sample waveform packets


      
       The compressed 64 sample waveform packets will always have a 3 bit header containing the compression type.
       Following that will be either the uncompressed, bit packed 10 bit waveform values (for compression type 0) or more
       header information, the contents of which depend on the type of compression used.  This will be followed by either
       the bit packed first differences, the bit packed second differences, or the bit-packed differences from the central
       shallow channel depending on the compression type.  We know that the count of values in a packet is always 64 and
       the starting value can't exceed 10 bits.  The size of "buffer" should be a static "sizeof (CZMIL_CWF_Data)" which
       will always be more than we can possibly use.  We're making the assumption that 99.9% of the time there are no NULL
       (i.e. 0) values in the waveforms.  This greatly simplifies compressing with first and second differences.  In the
       very few cases where we have NULL values amongst valid data we'll probably waste a little space but it should make
       compressing and decompressing faster since we will be eliminating a bunch of "if" statements.  The following is the
       structure of a single compressed 64 sample waveform packet:

       - 3 bits         =    Compression type
                                 - 0 = bit packed value
                                 - 1 = first difference
                                 - 2 = second difference
                                 - 3 = shallow channel difference (from center channel)
                                 - 4-7 = future possible compression schemes
       - if compression type = 0:
                - 64 * 10 bits   =    Waveform values
       - if compression type = 1:
                - 10 bits        =    Starting value
                - 11 bits        =    Offset value (offset by 2^10 - 1)
                - 4 bits         =    Delta bit size (DBS)
                - 63 * DBS       =    Difference from previous value + offset
       - if compression type = 2:
                - 10 bits        =    Starting value
                - 10 bits        =    Starting first difference value + first difference offset
                - 11 bits        =    First difference offset value (offset by 2^10 - 1)
                - 11 bits        =    Second difference offset value (offset by 2^10 - 1)
                - 4 bits         =    Delta bit size (DBS)
                - 62 * DBS       =    Difference from previous first difference value + second difference offset
       - if compression type = 3:
                - 11 bits        =    Offset value (offset by 2^10 - 1 : based on compression type 1 start bits)
                - 4 bits         =    Delta bit size (DBS)
                - 64 * DBS       =    Difference from the corresponding value in the central shallow
                                      channel (channel[1])


       To put it into words, the first 16 bits will contain the total buffer size in bytes.  Following the buffer size
       there will be 52 bits containing the time in microseconds since January 01, 1970.  After that will be 21 bits 
       containing the LiDAR scan angle in degrees.  Then there will be a block of CZMIL_MAX_PACKETS bits.  Each bit in
       this block corresponds to a 64 sample packet of the corresponding channel.  If the bit is unset then the packet
       contains no data.  If it is set, the packet contains 64 samples of data.  Following the block of bit flags will
       be the compressed, bit-packed sample data in blocks of 64 samples.  Each block will start with 3 bits containing
       the type of compression used.  This will be 0 for no compression (values bit packed into 10 bit fields), 1 for
       first difference compression, 2 for second difference compression, or 3 for differences between the central
       shallow channel and the surrounding channels.

       What follows the compression type is dependent on the compression type.  For type 0 compression it will be
       followed by 64 samples stored in 10 bits each.

       For compression type 1 it will be followed by 10 bits that contain the starting value for the difference values.
       Then 11 bits containing the offset value used to zero base the difference values.  Next is 4 bits containing the
       number of bits (first difference delta bits) used to store each difference value.  Then 63 fields of first
       difference delta bits that contains the difference from the previous value in the packet offset by the offset
       value.

       For compression type 2 it gets even more confusing.  We start with 10 bits that contain the starting value for
       the first difference values.  Then 10 bits that contain the starting value for the second difference values.
       Then 11 bits containing the offset value used to zero base the first difference values.  Next is 11 bits
       containing the offset value used to zero base the second difference values.  That is followed by 4 bits
       containing the number of bits (second difference delta bits) used to store each second difference value.
       Finally, 62 fields of second difference delta bits that contains the difference from the previous first
       difference value in the packet offset by the second difference offset value.

       For compression type 3 it's a bit simpler.  We start with 11 bits that contain the offset value used to zero base
       the difference values.  Following that we have 4 bits containing the number of bits (channel difference delta
       bits) used to store each difference value.  Then 64 fields of channel difference delta bits of differences from 
       the central shallow channel.

       IMPORTANT NOTE:  The number of bits is shown here as hard-wired numbers.  They are defined in the
       czmil_internals.h file but they are subject to change.  They will actually be stored in the file header so
       that we can change them if we need to.

       We assume that the first compression method (bit-packed ten bit values) should not occur since any of the other
       compression schemes will, almost invariably, give us better results.

       The data structure for the CZMIL CWF records, CZMIL_CWF_Data, is very simple (as opposed to how we store it on
       disk).  Note that the 64 sample packet in a channel array will be zero filled if the corresponding channel_ndx
       array is set to zero. That is, if channel_ndx[1][5] is equal to zero then channel[1][320] through channel[1][383]
       will be set to zero.  This is redundant information but it is used internally to avoid storing empty 64 sample
       packets.



       <br><br>\section sec3 CZMIL index (CDX) file format description

       The CDX file, like the CXY and CWF files, will have an ASCII header that contains tagged fields. Those fields are
       read and placed in the CZMIL_CDX_Header structure.  This header is even simpler than the CWF header in that there
       are no modifiable fields. There is really very little reason to be looking at the CDX file header since the file
       itself is created from the CXY and CWF files and, if it is somehow accidentally deleted, it will be automatically
       recreated when needed.

       The CZMIL_CDX_Data records, consisting of just four fields, are really simple.  It's pretty easy to see how the
       API uses the CDX file to find the associated CXY and CWF records. It just multiples the record number requested
       by the size of the CDX record, moves to that location in the CDX file, reads the CDX record, and then moves to
       cxy_address or cwf_address in the pertinent file and reads cxy_buffer_size or cwf_record_size bytes. The bytes
       read are then unpacked and uncompressed and handed back to the application as a simple CXY or CWF structure. The
       overhead of reading the CDX record is the price we must pay to get decent compression of both the waveforms and
       the point cloud. By compressing the data by even a modest two to one ratio we more than make up for the time used
       to read the CDX record and the time used to uncompress the record in less I/O reading the bulk of the data.



       <br><br>\section sec4 CZMIL API I/O function definitions

       The CZMIL API is very simple and consists of only about 20 functions.  The public functions and data structures
       documentation can be accessed from czmil.h in the Doxygen generated HTML documentation.



       <br><br>\section sec5 Example code for CXY and CWF files


       The following C code is an example of how one would create a CXY file, the associated CWF file, and the CDX
       index file:

       <pre>

       CZMIL_CXY_Header cxy_header;
       CZMIL_CXY_Data2 cxy_record;
       CZMIL_CWF_Data cwf_record;
       NV_INT32 i, hnd;
       NV_CHAR path[512];


       /#  This will zero out anything we don't specifically set.  #/


       memset (&cxy_header, 0, sizeof (CZMIL_CXY_Header));


       /#  Set the needed header structure fields.  #/

       strcpy (cxy_header.creation_software "Optech International DPS v1.0");


       strcpy (cxy_header.system_type, "Type 1");
       strcpy (cxy_header.system_number, "System 1");
       strcpy (cxy_header.rep_rate, "10KHz");
       strcpy (cxy_header.project, "NM_Saipan");
       strcpy (cxy_header.mission, "02MD06005_004_060320_2225");
       strcpy (cxy_header.dataset, "02DS06005_004_060320_2225_A");
       strcpy (cxy_header.flight_id, "Line 7-2");

       cxy_header.null_z_value = -998.0;

       strcpy (cxy_header.wkt, "COMPD_CS[\"WGS84 with ellipsoid Z\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],VERT_CS[\"ellipsoid Z in meters\",VERT_DATUM[\"Ellipsoid\",2002],UNIT[\"metre\",1],AXIS[\"Z\",UP]]]");

       cxy_header.local_vertical_datum = CZMIL_V_DATUM_UNDEFINED;

       strcpy (cxy_header.comments, "Blah, blah, blah, yackity smackity");


       /#  Note that setting the third argument to NVTrue indicates that you want to create the associated
           waveform file.  #/

       if ((hnd = czmil_create_cxy_file (path, &cxy_header, NVTrue)) < 0)
         {
           czmil_perror ();
           exit (-1);
         }


       /#  Loop through your source data and place it into the CXY and CWF record structures (cxy_record and
           cwf_record).  #/

       for (i = 0 ; i < a_bunch_of_records ; i++)
         {
           /#  Convert raw waveforms to point cloud... a miracle occurs here ;-) #/


           /#  We may want to set a status bit on some particular return.  #/

           czmil_set_status (cxy_record.data[3].status, CZMIL_SUSPECT, NVTrue);


           /#  Now write the CXY record and the CWF records.  #/

           if (czmil_write_cxy_record (hnd, CZMIL_APPEND_CXY_RECORD, &cxy_record) < 0)
             {
               czmil_perror ();
               exit (-1);
             }

           if (czmil_append_cwf_record (hnd, &cwf_record) < 0)
             {
               czmil_perror ();
               exit (-1);
             }
         }


         /#  Now close the file(s). This will update the headers and create the CDX index file.  #/

         czmil_close_file (hnd);


         </pre><br><br>


         Now that we've created a file we can interact with it by opening the file and calling the read and/or write
         functions. Note that you can't update CWF records. This is because they should only be created from the raw
         waveform data. They should not be modified after creation.


         <br><br><pre>

         CZMIL_CXY_Header cxy_header;
         CZMIL_CXY_Data2 cxy_record;
         CZMIL_CWF_Data cwf_record;
         NV_INT32 i, hnd;
         NV_CHAR path[512];


         if ((hnd = czmil_open_file (path, &cxy_header, CZMIL_UPDATE, NVTrue) < 0)
           {
             czmil_perror ();
             exit (-1);
           }

           for (i = 0 ; i < cxy_header.number_of_records ; i++)
             {
               if (czmil_read_cxy_record (hnd, i, &cxy_record) < 0 )
                 {
                   czmil_perror ();
                   exit (-1);
                 }

               if (czmil_read_cwf_record (hnd, i, &cwf_record) < 0)
                 {
                   czmil_perror ();
                   exit (-1);
                 }


               /#  We may want to check status of a return to see if we're going to use it.  #/

               if (!czmil_get_status (cxy_record.data[3].status, CZMIL_INVAL)
                 {
                   /#  Do something fun here... Maybe add a local_vertical_datum_offset value  #/

                   cxy_record.local_vertical_datum_offset = some_magic_value;


                   /#  If you changed the status you will need to update the CXY record.  #/

                   if (czmil_write_cxy_record (hnd, i, &cxy_record) < 0)
                     {
                       czmil_perror ();
                       exit (-1);
                     }
                 }
             }


           /#  Since we added local_vertical_datum_offset values we need to modify the header to reflect what the
               local_vertical_datum is.  #/

           cxy_header.local_vertical_datum = CZMIL_V_DATUM_MLLW;

           if (czmil_update_cxy_header (hnd, cxy_header) < 0)
             {
               czmil_perror ();
               exit (-1);
             }


           /#  Close the file(s).  #/

           czmil_close_file (hnd);

           </pre>

  */

  /*!

      - CZMIL CXY file header structure.  This doesn't really exist at present.  Header key definitions are as follows:

          - (a) = Set or modified by the API either at creation time or later
          - (c) = Defined by the application program only at creation time
          - (m) = Modifiable by the application program either at creation time or later using the czmil_update_cxy_header
                  function

  */

  typedef struct
  {                                                  /*   Key              Definition  */
    NV_CHAR              version[128];               /*!< (a)       Library version information  */
    NV_CHAR              file_type[128];             /*!< (a)       File type  */
    NV_INT64             creation_timestamp;         /*!< (a)       File creation microseconds from 01-01-1970  */
    NV_CHAR              creation_software[128];     /*!< (c)       File creation software (includes version)  */
    NV_INT64             modification_timestamp;     /*!< (a)       File modification microseconds from 01-01-1970  */
    NV_CHAR              modification_software[128]; /*!< (m)       File modification software (includes version)  */
    NV_CHAR              security_classification[32];/*!< (m)       Security classification  */
    NV_CHAR              distribution[1024];         /*!< (m)       Security distribution statement  */
    NV_CHAR              declassification[256];      /*!< (m)       Security declassification statement  */
    NV_CHAR              class_just[256];            /*!< (m)       Security classification justification  */
    NV_CHAR              downgrade[128];             /*!< (m)       Security downgrade statement  */
    NV_CHAR              cxy_filename[128];          /*!< (a)       CXY file basename (this file)  */
    NV_CHAR              cwf_filename[128];          /*!< (a)       CWF file basename (associated CWF waveform file)  */
    NV_CHAR              cdx_filename[128];          /*!< (a)       CDX file basename (associated CDX index file)  */
    NV_FLOAT64           min_lon;                    /*!< (a)       Minimum longitude value in file (west negative)  */
    NV_FLOAT64           min_lat;                    /*!< (a)       Minimum latitude value in file (south negative)  */
    NV_FLOAT64           max_lon;                    /*!< (a)       Maximum longitude value in file (west negative)  */
    NV_FLOAT64           max_lat;                    /*!< (a)       Maximum latitude value in file (south negative)  */
    NV_FLOAT64           min_platform_lon;           /*!< (a)       Minimum platform longitude value in file (west negative)  */
    NV_FLOAT64           min_platform_lat;           /*!< (a)       Minimum platform latitude value in file (south negative)  */
    NV_FLOAT64           max_platform_lon;           /*!< (a)       Maximum platform longitude value in file (west negative)  */
    NV_FLOAT64           max_platform_lat;           /*!< (a)       Maximum platform latitude value in file (south negative)  */
    NV_U_INT32           number_of_records;          /*!< (a)       Number of records in file  */
    NV_U_INT32           header_size;                /*!< (a)       Size of the ASCII header in bytes  */
    NV_U_INT32           record_size;                /*!< (a)       Size of a binary record in bytes  */
    NV_U_INT16           system_type;                /*!< (c)       Collection system type ID  */
    NV_U_INT16           system_number;              /*!< (c)       Collection system serial number  */
    NV_U_INT32           rep_rate;                   /*!< (c)       System rep rate  */
    NV_CHAR              project[128];               /*!< (c)       Project information  */
    NV_CHAR              mission[128];               /*!< (c)       Mission information  */
    NV_CHAR              dataset[128];               /*!< (c)       Dataset information  */
    NV_CHAR              flight_id[128];             /*!< (c)       Flight information  */
    NV_INT64             flight_start_timestamp;     /*!< (a)       Start of flight in microseconds from 01-01-1970  */
    NV_INT64             flight_end_timestamp;       /*!< (a)       End of flight in microseconds from 01-01-1970  */
    NV_FLOAT32           null_z_value;               /*!< (c)       Null value to be used for elevations  */
    NV_CHAR              wkt[1024];                  /*!< (c)       This is the Well-Known Text coordinate and datum
                                                                    information that is defined in the Open GIS Coordinate
                                                                    Transformations specification.  These are supported by
                                                                    the GDAL/OGR library so you can use the
                                                                    OGRSpatialReference::importFromWkt() method (or the
                                                                    corresponding C function) and then warp from one to
                                                                    another pretty easily).  */
    NV_U_INT16           local_vertical_datum;       /*!< (m)       Local vertical datum (defined in czmil_macros.h).  This
                                                                    is the vertical datum of elevations after correction
                                                                    with local_vertical_datum_offset.  The vertical datum
                                                                    specified in the wkt field will always be meters from
                                                                    some ellipsoid.  */
    NV_CHAR              comments[4096];             /*!< (m)       Comments  */
  } CZMIL_CXY_Header;



  /*!

      - CZMIL CWF file header structure.  This doesn't really exist at present.  Header key definitions are as follows:

          - (a) = Set or modified by the API either at creation time or later
          - (m) = Modifiable by the application program either at creation time or later using the czmil_update_cwf_header
                  function
          - (d) = Duplicated from the CXY header

  */

  typedef struct
  {                                                  /*   Key              Definition  */
    NV_CHAR              version[128];               /*!< (a)       Library version information  */
    NV_CHAR              file_type[128];             /*!< (a)       File type  */
    NV_INT64             creation_timestamp;         /*!< (a)       File creation microseconds from 01-01-1970  */
    NV_CHAR              creation_software[128];     /*!< (d)       File creation software (includes version)  */
    NV_CHAR              security_classification[32];/*!< (m)       Security classification  */
    NV_CHAR              distribution[1024];         /*!< (m)       Security distribution statement  */
    NV_CHAR              declassification[256];      /*!< (m)       Security declassification statement  */
    NV_CHAR              class_just[256];            /*!< (m)       Security classification justification  */
    NV_CHAR              downgrade[128];             /*!< (m)       Security downgrade statement  */
    NV_CHAR              cxy_filename[128];          /*!< (a)       CXY file basename (associated CXY point file)  */
    NV_CHAR              cwf_filename[128];          /*!< (a)       CWF file basename (this file)  */
    NV_CHAR              cdx_filename[128];          /*!< (a)       CDX file basename (associated CDX index file)  */
    NV_U_INT32           number_of_records;          /*!< (d)       Number of records in file  */
    NV_U_INT32           header_size;                /*!< (a)       Size of the ASCII header in bytes  */
    NV_U_INT16           system_type;                /*!< (d)       Collection system type ID  */
    NV_U_INT16           system_number;              /*!< (d)       Collection system serial number  */
    NV_U_INT32           rep_rate;                   /*!< (d)       System rep rate  */
    NV_CHAR              project[128];               /*!< (d)       Project information  */
    NV_CHAR              mission[128];               /*!< (d)       Mission information  */
    NV_CHAR              dataset[128];               /*!< (d)       Dataset information  */
    NV_CHAR              flight_id[128];             /*!< (d)       Flight information  */
    NV_INT64             flight_start_timestamp;     /*!< (d)       Start of flight in microseconds from 01-01-1970  */
    NV_INT64             flight_end_timestamp;       /*!< (d)       End of flight in microseconds from 01-01-1970  */
    NV_CHAR              comments[4096];             /*!< (m)       Comments.  */
  } CZMIL_CWF_Header;


  /*!

      - CZMIL CDX file header structure.  This doesn't really exist at present.  Header key definitions are as follows:

          - (a) = Set or modified by the API either at creation time or later
          - (d) = Duplicated from the CXY header

  */

  typedef struct
  {                                                  /*   Key              Definition  */
    NV_CHAR              version[128];               /*!< (a)       Library version information  */
    NV_CHAR              file_type[128];             /*!< (a)       File type  */
    NV_INT64             creation_timestamp;         /*!< (a)       File creation microseconds from 01-01-1970  */
    NV_CHAR              creation_software[128];     /*!< (d)       File creation software (includes version)  */
    NV_CHAR              cxy_filename[128];          /*!< (a)       CXY file basename (associated CXY point file)  */
    NV_CHAR              cwf_filename[128];          /*!< (a)       CWF file basename (associated CWF waveform file)  */
    NV_CHAR              cdx_filename[128];          /*!< (a)       CDX file basename (this file)  */
    NV_U_INT32           number_of_records;          /*!< (d)       Number of records in file  */
    NV_U_INT32           header_size;                /*!< (a)       Size of the ASCII header in bytes  */
    NV_CHAR              project[128];               /*!< (d)       Project information  */
    NV_CHAR              mission[128];               /*!< (d)       Mission information  */
    NV_CHAR              dataset[128];               /*!< (d)       Dataset information  */
    NV_CHAR              flight_id[128];             /*!< (d)       Flight information  */
    NV_INT64             flight_start_timestamp;     /*!< (d)       Start of flight in microseconds from 01-01-1970  */
    NV_INT64             flight_end_timestamp;       /*!< (d)       End of flight in microseconds from 01-01-1970  */
  } CZMIL_CDX_Header;


  typedef struct
  {
    NV_INT64             timestamp;                  /*!< Microseconds from January 01, 1970  */
    NV_FLOAT64           latitude;                   /*!< Latitude in degrees (south negative)  */
    NV_FLOAT64           longitude;                  /*!< Longitude in degrees (west negative)  */
    NV_FLOAT32           return_elevation[4];        /*!< Elevation above WGS84 ellipsoid  */
    NV_FLOAT32           waveform_class;             /*!< Class?  */
  } CZMIL_Channel_Data;


  typedef struct
  {
    CZMIL_Channel_Data   channel[9];                 /*!< Shot data for channels.  First channel is deep, next
                                                          seven are shallow, last is IR.  */
    NV_U_INT16           status;                     /*!< Per shot status info.  TBD.  */
  } CZMIL_CXY_Data;


  /*!

      CZMIL Waveform Format structure.

      Note that there is a lot of wasted space in this structure.  The reason for this is that there may be up to
      CZMIL_MAX_PACKETS packets of 64 values stored in some of the waveforms (strange atmospheric stuff).  We could get
      real tricky and allocate the memory for each waveform when we read it but this would require some pretty
      bullet-proof garbage collection.  In reality we will probably not have a whole lot of these in memory at one time
      so, in the interest of simplicity, we're going to set the arrays at the maximum size.  The _ndx arrays are simply
      going to contain either a one or a zero to indicate whether the associated packet is populated or not.  The
      information in these _ndx arrays (and the position of the waveform data in the CWF file) will, at some point in the
      future, be read from an index file or from the CXY record itself.  The reason we use an index of 64 value packets
      instead of just having a length is that topo shots that strike the top of a building and then later strike the
      ground (multiple 64 value packets later) will have empty packets in the intervening space.  The information in
      these packets is not useful so we won't store it on the disk in order to save disk storage (which is going to be
      killing us anyway).  From what I understand this will not be the case with hydro data but we'll use the same method
      since any of the channels may be cut off when we get into the noise following the bottom return.

  */

  typedef struct
  {
    NV_INT64             timestamp;                          /*!< Microseconds from January 01, 1970.  */
    NV_FLOAT32           scan_angle;                         /*!< Scan angle.  */
    NV_U_BYTE            channel_ndx[9][CZMIL_MAX_PACKETS];  /*!< Flag to indicate data in the 64 sample packet, zero
                                                                  indicates no data in the packet.  */
    NV_U_INT16           channel[9][CZMIL_MAX_PACKETS * 64]; /*!< Channel data, 9 channels of CZMIL_MAX_PACKETS packets of
                                                                  64 samples.  First channel is deep, next seven are
                                                                  shallow, last is IR.  */
  } CZMIL_CWF_Data;




  /*!

      CXY compression thoughts.

      The compressed record will have a buffer size stored in the first 24 bits of the record (16,777,215 max).  We'll
      follow that with the number of returns per channel stored in 3 bits each for the 9 channels.  After that we can
      pack in the rest of the information.  

      Note that there will be NO NEGATIVE NUMBERS stored in the CXY record.  All values will be offset to make them
      positive so that we don't have to try to sign extend values based on a variable number of bits (the number of bits
      may change with new versions of the software).  Variable bit sign extending can be very slow so we don't want to do
      it every time.  In the case of the full platform latitude/longitude we will offset by +90.0/+180.0 prior to
      multiplying by our scale factor.  All other values will be scaled and then offset by half of 2.0 raised to the
      number of bits power minus 1.  For example, the maximum scaled scan angle will be:

          (NV_INT32) (pow (2.0L, (NV_FLOAT64) CXY_SCAN_ANGLE_BITS)) - 1

      To clarify this, assume that CXY_SCAN_ANGLE_BITS is 21 and the CXY_ANGLE_MULT is 10000.0L.  The largest number that
      can be stored in 21 bits is 2,097,151 - 1 (2^21 = 2,097,152).  Divide that by 2 and you get an offfset of 1,048,575
      (integer truncation).  Since we're  multiplying the scan angle by 10,000 prior to converting it to the nearest
      integer this gives us a range of plus or minus 104.8575 degrees.  If, after adding the offset, the result is less
      than 0 or exceeds the maximum scaled scan angle an erro will occur when you try to write the value.  No error
      checking is done on read since the value can't be written if it falls outside our range.  Simple, no ;-)

      To see what our default bit numbers and scale factors are just look at the top of the czmil.c file.

  */


  typedef struct
  {
    NV_FLOAT64           latitude;                           /*!< Latitude in degrees (south negative)  */
    NV_FLOAT64           longitude;                          /*!< Longitude in degrees (west negative)  */
    NV_FLOAT32           elevation;                          /*!< Elevation above ellipsoid  */
    NV_FLOAT32           reflectance;                        /*!< Reflectance  */
    NV_FLOAT32           horizontal_uncertainty;             /*!< Horizontal uncertainty  */
    NV_FLOAT32           vertical_uncertainty;               /*!< Vertical uncertainty  */
    NV_U_INT16           status;                             /*!< Status information per return (TBD)  */
  } CZMIL_Return_Data;


  typedef struct
  {
    CZMIL_Return_Data    data[4];                            /*!< Information about each of 4 possible returns.  */
    NV_FLOAT32           waveform_class;                     /*!< Class?  */
  } CZMIL_Channel_Data2;


  typedef struct
  {
    NV_INT64             timestamp;                          /*!< Microseconds from January 01, 1970  */
    NV_FLOAT32           scan_angle;                         /*!< Scan angle.  */
    NV_FLOAT64           platform_latitude;                  /*!< Platform latitude in degrees (south negative)  */
    NV_FLOAT64           platform_longitude;                 /*!< Platform longitude in degrees (west negative)  */
    NV_FLOAT32           platform_elevation;                 /*!< Platform elevation above ellipsoid  */
    NV_FLOAT32           platform_roll;                      /*!< Platform roll in degrees  */
    NV_FLOAT32           platform_pitch;                     /*!< Platform pitch in degrees  */
    NV_FLOAT32           platform_heading;                   /*!< Platform heading in degrees  */
    NV_U_BYTE            pdop;                               /*!< GPS positional dilution of precision  */
    NV_U_BYTE            tdop;                               /*!< GPS time dilution of precision  */
    NV_FLOAT64           water_surface_latitude;             /*!< Latitude in degrees (south negative) where LiDAR strikes
                                                                 water surface  */
    NV_FLOAT64           water_surface_longitude;            /*!< Longitude in degrees (west negative) where LiDAR strikes
                                                                 water surface  */
    NV_FLOAT32           water_surface_elevation;            /*!< Elevation above ellipsoid where LiDAR strikes water
                                                                 surface  */
    NV_FLOAT32           water_level;                        /*!< Water level elevation above ellipsoid  */
    NV_FLOAT32           local_vertical_datum_offset;        /*!< Local datum corrector to be added to all
                                                                 return_elevations.  */
    NV_U_INT16           returns[9];                         /*!< Number of returns per channel.  */
    CZMIL_Channel_Data2  channel[9];                         /*!< Deep channel CXY data, 7 shallow channel data, and IR
                                                                 channel data.  */
    NV_U_INT16           status;                             /*!< Per shot status info.  TBD.  */
  } CZMIL_CXY_Data2;


  typedef struct
  {
    NV_INT64             cxy_address;                        /*!< Byte address of the associated CXY record in the CXY
                                                                 file  */
    NV_INT64             cwf_address;                        /*!< Byte address of the associated CWF record in the CWF
                                                                 file  */
    NV_U_INT16           cxy_buffer_size;                    /*!< Size of CXY buffer at cxy_address  */
    NV_U_INT16           cwf_buffer_size;                    /*!< Size of CWF buffer at cwf_address  */
  } CZMIL_CDX_Data;


  /*!  Public function declarations.  */

  CZMIL_DLL NV_INT32 czmil_create_file (const NV_CHAR *path, CZMIL_CXY_Header cxy_header, NV_BOOL cwf);
  CZMIL_DLL NV_INT32 czmil_open_file (const NV_CHAR *path, CZMIL_CXY_Header *cxy_header, NV_INT32 mode, NV_BOOL cwf);
  CZMIL_DLL NV_INT32 czmil_close_file (NV_INT32 hnd);
  CZMIL_DLL NV_INT32 czmil_read_cxy_record (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CXY_Data *record);
  CZMIL_DLL NV_INT32 czmil_read_cxy_record2 (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CXY_Data2 *record);
  CZMIL_DLL NV_INT32 czmil_write_cxy_record (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CXY_Data2 *record);
  CZMIL_DLL NV_INT32 czmil_read_cwf_record (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CWF_Data *record);
  CZMIL_DLL NV_INT32 czmil_append_cwf_record (NV_INT32 hnd, CZMIL_CWF_Data *record);
  CZMIL_DLL void czmil_update_cxy_header (NV_INT32 hnd, CZMIL_CXY_Header cxy_header);
  CZMIL_DLL void czmil_update_cwf_header (NV_INT32 hnd, CZMIL_CXY_Header cxy_header);
  CZMIL_DLL NV_INT32 czmil_get_errno ();
  CZMIL_DLL NV_CHAR *czmil_strerror ();
  CZMIL_DLL void czmil_perror ();
  CZMIL_DLL NV_CHAR *czmil_get_version ();
  CZMIL_DLL void czmil_dump_cxy_record (CZMIL_CXY_Data record);
  CZMIL_DLL void czmil_dump_cwf_record (NV_INT32 hnd, CZMIL_CWF_Data record);
  CZMIL_DLL void czmil_dump_cdx_record (CZMIL_CDX_Data record);
  CZMIL_DLL void czmil_cvtime (NV_INT64 micro_sec, NV_INT32 *year, NV_INT32 *jday, NV_INT32 *hour, NV_INT32 *minute,
                               NV_FLOAT32 *second);
  CZMIL_DLL void czmil_inv_cvtime (NV_INT32 year, NV_INT32 jday, NV_INT32 hour, NV_INT32 min, NV_FLOAT32 sec,
                                   NV_INT64 *timestamp);
  CZMIL_DLL void czmil_set_status (NV_U_INT16 *status, NV_U_INT16 status_flag, NV_BOOL flag);
  CZMIL_DLL NV_U_INT16 czmil_get_status (NV_U_INT16 status, NV_U_INT16 status_flag);
  CZMIL_DLL void czmil_jday2mday (NV_INT32 year, NV_INT32 jday, NV_INT32 *mon, NV_INT32 *mday);


#ifdef  __cplusplus
}
#endif


#endif
