
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



#ifndef __CZMIL_INTERNALS_H__
#define __CZMIL_INTERNALS_H__

#ifdef  __cplusplus
extern "C" {
#endif


#undef CZMIL_DEBUG


#ifndef DPRINT
  #define DPRINT      fprintf (stderr, "%s %d\n", __FILE__, __LINE__);fflush (stderr);
#endif

#ifndef NINT
  #define NINT(a)     ((a)<0.0 ? (NV_INT32) ((a) - 0.5) : (NV_INT32) ((a) + 0.5))
#endif

#ifndef NINT64
  #define NINT64(a)   ((a)<0.0 ? (NV_INT64) ((a) - 0.5) : (NV_INT64) ((a) + 0.5))
#endif

#ifndef MAX
#define MAX(x,y)      (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y)      (((x) < (y)) ? (x) : (y))
#endif

#ifndef LOG2
#define LOG2          0.30102999566398L
#endif


#define       CZMIL_CXY_HEADER_SIZE              0           /*!<  CXY header size, for now.  */
#define       CZMIL_CWF_HEADER_SIZE              0           /*!<  CWF header size, for now.  */
#define       CZMIL_CDX_HEADER_SIZE              65536       /*!<  CDX header size.  */


/*  These are default bit/byte field sizes and scale factors used for CXY point cloud data compression/decompression.  */

#define CXY_BUFFER_SIZE_BYTES     2             /*!<  Number of bytes used to store the CXY bit-packed,
                                                      compressed buffer.  DO NOT CHANGE!  */
#define CXY_RETURN_BITS           3             /*!<  Number of bits used to store the number of returns per channel.  */
#define CXY_TIME_BITS             52            /*!<  Number of bits used to store a bit packed timestamp.  */
#define CXY_POS_SCALE             111120000.0L  /*!<  Position scale factor.  This is the number of millimeters per degree
                                                      at the equator.  It is used to determine how many bits we need to
                                                      store the full platform latitude and longitude.  */
#define CXY_ANGLE_SCALE           10000.0L      /*!<  Ten thousandths of a degree.  */
#define CXY_SCAN_ANGLE_BITS       21            /*!<  0 to 2,097,151 ( / CXY_ANGLE_SCALE = 209.7152104.8575,
                                                      / 2.0L = +/-104.8575 degrees.  */
#define CXY_ROLL_PITCH_BITS       20            /*!<  0 to 1,048,575 ( / CXY_ANGLE_SCALE = 104.8575,
                                                      / 2.0L = +/- 52.42875 degrees.  */
#define CXY_HEADING_BITS          22            /*!<  0 to 4,194,303 ( / CXY_ANGLE_SCALE = 0.0 to 419.4303 degrees (no
                                                      negatives).  */
#define CXY_DOP_BITS              5             /*!<  Should never exceed 31.  */
#define CXY_LAT_DIFF_BITS         23            /*!<  0 to 8,388,607 (+/- 4,194,303 millimeters in units of CXY_POS_SCALE).
                                                      Longitude difference bits will be based on latitude difference bits.
                                                      It will increase by one bit times 1 over the cosine of the latitude
                                                      degrees up to 84 degrees latitude then it will be stored as a full
                                                      resolution longitude.  */
#define CXY_ELEV_BITS             22            /*!<  0 to 4,194,303 ( / CXY_ELEV_SCALE = 4194.303, / 2.0L = +/- 2097.1515
                                                      meters.  */
#define CXY_ELEV_SCALE            1000.0L       /*!<  Elevation scale factor (millimeters).  */
#define CXY_UNCERT_BITS           14            /*!<  0 to 16,383 ( / CXY_UNCERT_SCALE = 0.0 to 16.383 meters).  */
#define CXY_UNCERT_SCALE          1000.0L       /*!<  Uncertainty scale factor (millimeters).  */
#define CXY_REFLECTANCE_BITS      14            /*!<  0 to 16,383 ( / CXY_REFLECTANCE_SCALE = 0.0 to 1.6383, reflectance
                                                      range is 0.0 to 1.0)  */
#define CXY_REFLECTANCE_SCALE     10000.0L      /*!<  Ten thousandths of whatever units reflectance is in ;-)  */
#define CXY_STATUS_BITS           0             /*!<  ??????  */
#define CXY_WAVEFORM_CLASS_BITS   0             /*!<  ??????  */



/*  These are default constant values used for CWF waveform compression/decompression.  */

#define CWF_BUFFER_SIZE_BYTES     2             /*!<  Number of bytes used to store the CWF bit-packed, compressed buffer.
                                                      DO NOT CHANGE!  */
#define CWF_TYPE_BITS             3             /*!<  Number of bits used to store compressed CWF compression type.  See
                                                      czmil_compress_cwf_record for a description of how we're compressing
                                                      the different types.  */
#define CWF_TYPE_1_START_BITS     10            /*!<  Number of bits used to store first difference compressed CWF start
                                                      value.  */
#define CWF_TYPE_2_START_BITS     10            /*!<  Number of bits used to store second difference compressed CWF start
                                                      value.  */
#define CWF_TYPE_2_OFFSET_BITS    11            /*!<  Number of bits used to store second difference compressed CWF offset
                                                      value.  We're adding one bit for the worst case scenario of a large
                                                      difference in both directions.  */
#define CWF_TYPE_3_OFFSET_BITS    11            /*!<  Number of bits used to store shallow channel minus central shallow
                                                      channel difference compressed CWF offset value.  We're adding one
                                                      bit for the worst case scenario of a large difference in both
                                                      directions.  */
#define CWF_DELTA_BITS            4             /*!<  Number of bits used to store the number of bits we're using to store
                                                      CWF compressed delta values.  */
#define CWF_TIME_BITS             52            /*!<  Number of bits used to store a bit packed timestamp.  */
#define CWF_ANGLE_SCALE           10000.0L      /*!<  Ten thousandths of a degree.  */
#define CWF_SCAN_ANGLE_BITS       21            /*!<  +/- 90 degrees.  */


/*  These are default constant values used for CDX index file packing/unpacking.  */

#define CDX_CXY_ADDRESS_BITS      48            /*!<  This MUST be a multiple of 8.  */
#define CDX_CWF_ADDRESS_BITS      48            /*!<  This MUST be a multiple of 8.  */
#define CDX_CXY_BUFFER_SIZE_BITS  CXY_BUFFER_SIZE_BYTES * 8
#define CDX_CWF_BUFFER_SIZE_BITS  CWF_BUFFER_SIZE_BYTES * 8


/*!  This is a flag that is used to signify that we are packing/unpacking full longitude instead of longitude difference
     from a reference point.  */

#define FULL_LON_FLAG             999


/*  This is the structure we use to keep track of important formatting data for an open CZMIL file.  */

typedef struct
{
  NV_CHAR           cxy_path[1024];             /*!<  Fully qualified CXY file name (basenames are stored in each file
                                                      header).  */
  NV_CHAR           cwf_path[1024];             /*!<  Fully qualified CWF file name (basenames are stored in each file
                                                      header).  */
  NV_CHAR           cdx_path[1024];             /*!<  Fully qualified CDX file name (basenames are stored in each file
                                                      header).  */
  FILE              *cxy_fp;                    /*!<  CXY file pointer.  */
  FILE              *cwf_fp;                    /*!<  CWF file pointer.  */
  FILE              *cdx_fp;                    /*!<  CDX file pointer.  */
  CZMIL_CXY_Header  cxy_header;                 /*!<  CXY file header.  */
  CZMIL_CWF_Header  cwf_header;                 /*!<  CWF file header.  */
  CZMIL_CDX_Header  cdx_header;                 /*!<  CDX file header.  */
  NV_BOOL           cxy_at_end;                 /*!<  Set if the CXY file position is at the end of the file.  */
  NV_BOOL           cxy_modified;               /*!<  Set if the CXY file has been modified.  */
  NV_BOOL           cxy_created;                /*!<  Set if we created the CXY file.  */
  NV_BOOL           cxy_write;                  /*!<  Set if the last action to the CXY file was a write.  */
  NV_INT64          cxy_pos;                    /*!<  Position of the CXY file pointer after last I/O operation.  */
  NV_BOOL           cwf_at_end;                 /*!<  Set if the CWF file position is at the end of the file.  */
  NV_BOOL           cwf_created;                /*!<  Set if we created the CWF file.  */
  NV_BOOL           cwf_modified;               /*!<  Set if the CWF file header has been modified.  */
  NV_BOOL           cwf_write;                  /*!<  Set if the last action to the CWF file was a write.  */
  NV_INT64          cwf_pos;                    /*!<  Position of the CWF file pointer after last I/O operation.  */
  NV_BOOL           cdx_at_end;                 /*!<  Set if the CDX file position is at the end of the file.  */
  NV_BOOL           cdx_created;                /*!<  Set if we created the CDX file.  */
  NV_BOOL           cdx_write;                  /*!<  Set if the last action to the CDX file was a write.  */
  NV_INT64          cdx_pos;                    /*!<  Position of the CDX file pointer after last I/O operation.  */
  CZMIL_CDX_Data    cdx_record;                 /*!<  Last CDX record read.  This may save us from re-reading the CDX
                                                      record for the CWF waveforms if we just read the CXY point.  */


  /*  The following information is read from the CXY file ASCII header or computed from information read from the CXY
      file ASCII header but is not placed in the header since we have no need of this information outside of the API.  */

  NV_U_INT16        cxy_buffer_size_bytes;      /*!<  Number of bytes used to store the CXY bit-packed, compressed
                                                      buffer.  */
  NV_U_INT16        cxy_return_bits;            /*!<  Number of bits used to store the number of returns per channel.  */
  NV_U_INT16        cxy_time_bits;              /*!<  Number of bits used to store a bit packed timestamp.  */
  NV_FLOAT32        cxy_angle_scale;            /*!<  Scale Factor for all angles (scan, roll, pitch, heading).  */
  NV_U_INT16        cxy_scan_angle_bits;        /*!<  Number of bits used to store the scan angle.  */
  NV_U_INT16        cxy_heading_bits;           /*!<  Number of bits used to store heading.  */
  NV_U_INT16        cxy_roll_pitch_bits;        /*!<  Number of bits used to store roll and pitch.  */
  NV_U_INT16        cxy_dop_bits;               /*!<  Number of bits used to dilution of precision values.  */
  NV_FLOAT32        cxy_pos_scale;              /*!<  Position scale factor.  Used to determine how many bits we need to
                                                      store the full platform latitude and longitude.  */
  NV_U_INT16        cxy_lat_diff_bits;          /*!<  Number of bits used to store latitude differences.  */
  NV_U_INT16        cxy_lon_diff_bits[180];     /*!<  This is based on the latitude band (every degree) and
                                                      cxy_lat_diff_bits so we can deal with longitude convergence as we
                                                      move toward the poles.  */
  NV_U_INT16        cxy_elev_bits;              /*!<  Number of bits used to store elevations.  */
  NV_FLOAT32        cxy_elev_scale;             /*!<  Elevation scale factor.  */
  NV_U_INT16        cxy_uncert_bits;            /*!<  Number of bits used to store horizontal and vertical
                                                      uncertainties.  */
  NV_FLOAT32        cxy_uncert_scale;           /*!<  Uncertainty scale factor.  */
  NV_U_INT16        cxy_reflectance_bits;       /*!<  Number of bits used to store reflectance.  */
  NV_FLOAT32        cxy_reflectance_scale;      /*!<  Reflectance scale factor.  */
  NV_U_INT16        cxy_status_bits;            /*!<  Number of bits used to store status flags.  */
  NV_U_INT16        cxy_waveform_class_bits;    /*!<  Number of bits used to store waveform class.  */
  NV_U_INT16        cxy_lat_bits;               /*!<  Number of bits used to store the full platform latitude in the CXY
                                                      record.  */
  NV_U_INT16        cxy_lon_bits;               /*!<  Number of bits used to store the full platform longitude in the CXY
                                                      record.  */
  NV_U_INT32        cxy_lat_diff_max;           /*!<  Maximum allowed latitude difference from the platform latitude after
                                                      adding the offset.  */
  NV_U_INT32        cxy_lat_diff_offset;        /*!<  Latitude difference offset (half of cxy_lat_diff_max).  */
  NV_U_INT32        cxy_lon_diff_max[180];      /*!<  Maximum allowed longitude difference (per lat band) from the platform
                                                      longitude after adding the offset.  */
  NV_U_INT32        cxy_lon_diff_offset[180];   /*!<  Longitude difference offset (per lat band, half of
                                                      cxy_lon_diff_max).  */
  NV_U_INT32        cxy_elev_max;               /*!<  Maximum allowed elevation difference value after adding the
                                                      offset.  */
  NV_U_INT32        cxy_elev_offset;            /*!<  Elevation difference offset (half of cxy_elev_max).  */
  NV_U_INT32        cxy_uncert_max;             /*!<  Maximum uncertainty value that will be stored in a CXY record.  */
  NV_U_INT32        cxy_roll_pitch_max;         /*!<  Maximum roll and pitch value after adding the offset.  */
  NV_U_INT32        cxy_roll_pitch_offset;      /*!<  Roll and pitch offset (half of cxy_roll_pitch_max).  */
  NV_U_INT32        cxy_reflectance_max;        /*!<  Maximum reflectance value after adding the offset.  */
  NV_U_INT32        cxy_reflectance_offset;     /*!<  Reflectance offset (half of cxy_reflectance_max).  */
  NV_U_INT32        cxy_waveform_class_max;     /*!<  Maximum waveform_class value after adding the offset.  */
  NV_U_INT32        cxy_waveform_class_offset;  /*!<  Waveform_class offset (half of cxy_waveform_class_max).  */
  NV_U_INT32        cxy_scan_angle_max;         /*!<  Maximum scan angle value after adding the offset.  */
  NV_U_INT32        cxy_scan_angle_offset;      /*!<  Scan angle offset (half of cxy_scan_angle_max).  */


  /*  The following information is read from the CWF file ASCII header or computed from information read from the CWF
      file ASCII header but is not placed in the header since we have no need of this information outside of the API.  */

  NV_U_INT16        cwf_buffer_size_bytes;      /*!<  Number of bytes used to store compressed CWF buffer size.  */
  NV_U_INT16        cwf_type_bits;              /*!<  Number of bits used to store compressed CWF compression type.  See
                                                      czmil_compress_cwf_packet for a description of how we're compressing
                                                      the different types.  */
  NV_U_INT16        cwf_type_0_bytes;           /*!<  Number of bytes used to store bit packed 10 bit values.  */
  NV_U_INT16        cwf_type_1_start_bits;      /*!<  Number of bits used to store first difference compressed CWF start
                                                      value.  */
  NV_U_INT16        cwf_type_1_offset;          /*!<  This is 2 raised to the cwf_type_1_start_bits + 1 power.  This is the
                                                      offset value added to the offset value so that the first offset will
                                                      be a positive value.  This way we don't have to sign extend the 
                                                      offset.  This is also used for the type 3 compression offset.  */
  NV_U_INT16        cwf_type_1_offset_bits;     /*!<  Number of bits used to store first difference compressed CWF offset
                                                      value.  */
  NV_U_INT16        cwf_type_2_start_bits;      /*!<  Number of bits used to store second difference compressed CWF start
                                                      value.  */
  NV_U_INT16        cwf_type_2_offset;          /*!<  This is 2 raised to the cwf_type_2_start_bits + 1 power.  This is the
                                                      offset value added to the offset value so that the first offset will
                                                      be a positive value.  This way we don't have to sign extend the 
                                                      offset.  */
  NV_U_INT16        cwf_type_2_offset_bits;     /*!<  Number of bits used to store second difference compressed CWF offset
                                                      value.  */
  NV_U_INT16        cwf_type_3_offset_bits;     /*!<  Number of bits used to store shallow channel difference compressed
                                                      CWF offset value.  */
  NV_U_INT16        cwf_delta_bits;             /*!<  Number of bits used to store the number of bits we're using to store
                                                      CWF compressed delta values.  */
  NV_U_INT16        cwf_czmil_max_packets;      /*!<  Maximum number of 64 sample packets allowed.  This is also the number
                                                      of bits used to store the _ndx bits.  */
  NV_U_INT16        cwf_time_bits;              /*!<  Number of bits used to store a bit packed timestamp.  */
  NV_FLOAT32        cwf_angle_scale;            /*!<  Scale factor for scan angle.  */
  NV_U_INT16        cwf_scan_angle_bits;        /*!<  Number of bits used to store scan angle.  */
  NV_U_INT32        cwf_scan_angle_max;         /*!<  Maximum scan angle value after adding the offset.  */
  NV_U_INT32        cwf_scan_angle_offset;      /*!<  Scan angle offset (half of cxy_scan_angle_max).  */
  NV_U_INT16        cwf_type_1_header_bits;     /*!<  Bits used for the CWF type 1 compressed CWF packet header.  */
  NV_U_INT16        cwf_type_2_header_bits;     /*!<  Bits used for the CWF type 2 compressed CWF packet header.  */
  NV_U_INT16        cwf_type_3_header_bits;     /*!<  Bits used for the CWF type 3 compressed CWF packet header.  */


  /*  The following information is read from the CDX file ASCII header or computed from information read from the CDX
      file ASCII header but is not placed in the header since we have no need of this information outside of the API.  */

  NV_U_INT16        cdx_record_size_bytes;      /*!<  Size in bytes of each CDX record.  */
  NV_U_INT16        cdx_cxy_address_bits;       /*!<  Number of bits used to store the CXY record byte address.  */
  NV_U_INT16        cdx_cwf_address_bits;       /*!<  Number of bits used to store the CWF record byte address.  */
  NV_U_INT16        cdx_cxy_buffer_size_bits;   /*!<  Number of bits used to store the size of the CXY buffer.  This is 
                                                      triply redundant but it will save one fread call (we won't have to
                                                      read the buffer size from the CXY file before we read the rest of the
                                                      buffer) per shot read.  The added bytes in the CDX file are pretty
                                                      much under the radar at 10,000 shots per second.  */
  NV_U_INT16        cdx_cwf_buffer_size_bits;   /*!<  Number of bits used to store the size of the CWF buffer.  Same logic
                                                      as above.  */
} INTERNAL_CZMIL_STRUCT;


/*  CZMIL error handling variables.  */

typedef struct 
{
  NV_INT32          system;                     /*!<  Last system error condition encountered.  */
  NV_INT32          czmil;                      /*!<  Last CZMIL error condition encountered.  */
  NV_CHAR           file[512];                  /*!<  Name of file being accessed when last error encountered (if
                                                      applicable).  */
  NV_INT32          recnum;                     /*!<  Record number being accessed when last error encountered (if
                                                      applicable).  */
  NV_CHAR           info[128];                  /*!<  Text information to be printed out.  */
} CZMIL_ERROR_STRUCT;


#ifdef  __cplusplus
}
#endif


#endif
