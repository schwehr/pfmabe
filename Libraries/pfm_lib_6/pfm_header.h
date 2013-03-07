
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



#ifndef __PFM_HEADER__
#define __PFM_HEADER__

#include "pfm_nvtypes.h"
#include "pfm.h"

/***************************************************************************/
/*!

   - Module Name:        Pure File Magic (PFM) option header file

   - Programmer(s):      Jan C. Depner

   - Date Written:       May 1999

   - Purpose:            This header file defines all of the format options
                         that are stored in the PFM file header.  To add a new
                         option you simply add it to the header_data structure,
                         then put a definition in the keys structure.  The keys
                         structure contains a keyphrase such as "[DATE]", a 
                         format like "%s", a datatype (see the union in the
                         BIN_HEADER_DATA structure, a pointer to the location at
                         which to store the value(s), and a count if it is a
                         repeating field (array).

   - ************************************************************************

   - Structure:          KEY

****************************************************************************/

typedef struct
{
  NV_CHAR                  *keyphrase;          /*!<  Key phrase to be matched  */
  NV_CHAR                  *datatype;           /*!<  Data type (.e.g ui32)  */
  union
  {
    NV_U_CHAR              *uc;                 /*!<  Unsigned character data type  */
    NV_CHAR                *c;                  /*!<  Signed character data type  */
    NV_U_BYTE              *ub;                 /*!<  Unsigned byte integer data type  */
    NV_BYTE                *by;                 /*!<  Signed byte integer data type  */
    NV_BOOL                *b;                  /*!<  Boolean data type  */
    NV_INT16               *i16;                /*!<  Signed 16 bit integer data type  */
    NV_INT32               *i32;                /*!<  Signed 32 bit integer data type  */
    NV_INT64               *i64;                /*!<  Signed 64 bit integer data type  */
    NV_U_INT16             *ui16;               /*!<  Unsigned 16 bit integer data type  */
    NV_U_INT32             *ui32;               /*!<  Unsigned 32 bit integer data type  */
    NV_U_INT64             *ui64;               /*!<  Unsigned 64 bit integer data type  */
    NV_FLOAT32             *f32;                /*!<  32 bit floating point data type  */
    NV_FLOAT64             *f64;                /*!<  64 bit floadting point data type  */
    NV_C_HSV               *hsv;                /*!<  Hue, Saturation, and Value color structure data type  */
    NV_C_RGB               *rgb;                /*!<  Red, Green, and Blue color structure data type  */
    NV_F64_POS             *f64p;               /*!<  64 bit floating point lat,lon position structure data type  */
    NV_F64_POSDEP          *f64pd;              /*!<  64 bit floating point lat,lon,dep position structure data type  */
    NV_F64_XYMBR           *mbr;                /*!<  64 bit floating point Minimum Bounding Rectangle structure data type  */
    NV_I32_COORD2          *i32c2;              /*!<  32 bit signed integer coordinate (x,y) structure data type  */
    NV_I32_COORD3          *i32c3;              /*!<  32 bit signed integer coordinate (x,y,z) structure data type  */
    NV_F64_COORD2          *f64c2;              /*!<  64 bit floating point coordinate (x,y) structure data type  */
    NV_F64_COORD3          *f64c3;              /*!<  64 bit floating point coordinate (x,y,z) structure data type  */
  } address;
  NV_INT32                 *count;              /*!<  Count of values for repeating fields  */
  NV_INT16                 version_dependency;  /*!<  Version times 10  */
} KEY;


/***************************************************************************/
/*!

   - Module Name:        Pure File Magic (PFM) option header file

   - Programmer(s):      Jan C. Depner

   - Date Written:       May 1999

   - Purpose:            This header file defines all of the format options
                         that are stored in the PFM file header.  To add a new
                         option you simply add it to the header_data structure,
                         then put a definition in the keys structure.  The keys
                         structure contains a keyphrase such as "[DATE]", a 
                         format like "%s", a datatype (see the union in the
                         BIN_HEADER_DATA structure, a pointer to the location at
                         which to store the value(s), and a count if it is a
                         repeating field (array).

   - ************************************************************************

   - Structure:          BIN_HEADER_DATA
*/

typedef struct
{
  BIN_HEADER                 head;                       /*!<  Bin header structure  */
  NV_U_INT32                 record_length;              /*!<  Length of record in bytes  */
  NV_FLOAT32                 depth_offset;               /*!<  Offset added to depths to make them positive (inverted minimum depth)  */
  NV_U_INT32                 depth_bits;                 /*!<  Number of bits used to store a depth record  */
  NV_FLOAT32                 depth_scale;                /*!<  Scale depths are multiplied by (i.e. resolution)  */
  NV_U_INT32                 count_bits;                 /*!<  Number of bits used to store the number of points in a bin  */
  NV_U_INT32                 std_bits;                   /*!<  Number of bits used to store a standard deviation value  */
  NV_FLOAT32                 std_scale;                  /*!<  Scale standard deviations are multiplied by  */
  NV_U_INT32                 edited_flag_bits;           /*!<  Bits used to store the PFM_EDITED flag (pre 4.0)  */
  NV_U_INT32                 checked_flag_bits;          /*!<  Bits used to store the PFM_CHECKED flag (pre 4.0)  */
  NV_U_INT32                 suspect_flag_bits;          /*!<  Bits used to store the PFM_SUSPECT flag (pre 4.0)  */
  NV_U_INT32                 data_flag_bits;             /*!<  Bits used to store the PFM_DATA flag (pre 4.0)  */
  NV_U_INT32                 selected_flag_bits;         /*!<  Bits used to store the PFM_SELECTED flag (pre 4.0)  */
  NV_U_INT32                 class1_flag_bits;           /*!<  Bits used to store the PFM_CLASS1 flag (pre 4.0)  */
  NV_U_INT32                 class2_flag_bits;           /*!<  Bits used to store the PFM_CLASS2 flag (pre 4.0)  */
  NV_U_INT32                 record_pointer_bits;        /*!<  Number of bits used to store a record pointer  */
  NV_U_INT32                 file_number_bits;           /*!<  Number of bits used to store a file number  */
  NV_U_INT32                 ping_number_bits;           /*!<  Number of bits used to store a ping/record number  */
  NV_U_INT32                 line_number_bits;           /*!<  Number of bits used to store a line number  */
  NV_U_INT32                 beam_number_bits;           /*!<  Number of bits used to store a beam/subrecord number  */
  NV_U_INT32                 offset_bits;                /*!<  Number of bits used to store offsets  */
  NV_U_INT32                 validity_bits;              /*!<  Number of bits used to store validity  */
  NV_U_INT32                 projected_x_bits;           /*!<  Bits used to store the projected X value (pre 4.0)  */
  NV_U_INT32                 projected_x_offset;         /*!<  Offset value used to make the projected X value positive (pre 4.0)  */
  NV_U_INT32                 bin_attr_bits[NUM_ATTR];    /*!<  Number of bits used to store BIN attributes  */
  NV_U_INT32                 ndx_attr_bits[NUM_ATTR];    /*!<  Number of bits used to store NDX attributes  */
  NV_U_INT32                 horizontal_error_bits;      /*!<  Number of bits used to store horizontal uncertainty  */
  NV_U_INT32                 vertical_error_bits;        /*!<  Number of bits used to store vertical uncertainty  */
  NV_FLOAT32                 horizontal_error_null;      /*!<  Null value for horizontal uncertainty  */
  NV_FLOAT32                 vertical_error_null;        /*!<  Null value for vertical uncertainty  */
  NV_INT64                   coverage_map_address;       /*!<  Address in bytes of the beginning of the coverage map (at the end of the BIN file)  */
  NV_FLOAT32                 bin_attr_offset[NUM_ATTR];  /*!<  Offset value added to BIN attributes to make them positive  */
  NV_FLOAT32                 bin_attr_max[NUM_ATTR];     /*!<  Maximum BIN attribute values  */
} BIN_HEADER_DATA;


static BIN_HEADER_DATA         header_data;

static KEY                     keys[] =
{
    {
        "[VERSION]",
        "string",
        {(void *) header_data.head.version},
        NULL,
        0
    },


    {
        "[RECORD LENGTH]",
        "ui32",
        {(void *) &header_data.record_length},
        NULL,
        0
    },


    {
        "[DATE]",
        "string",
        {(void *) header_data.head.date},
        NULL,
        0
    },


    {
        "[CLASSIFICATION]",
        "string",
        {(void *) header_data.head.classification},
        NULL,
        0
    },

    {
        "[CREATION SOFTWARE]",
        "string",
        {(void *) header_data.head.creation_software},
        NULL,
        0
    },

    /************************* Version Dependency ****************************/

    /*  The following section supports pre-4.0 format files.  */

    {
        "[SOUTH LAT]",
        "f64p9",
        {(void *) &header_data.head.mbr.min_y},
        NULL,
        30
    },


    {
        "[WEST LON]",
        "f64p9",
        {(void *) &header_data.head.mbr.min_x},
        NULL,
        30
    },


    {
        "[NORTH LAT]",
        "f64p9",
        {(void *) &header_data.head.mbr.max_y},
        NULL,
        30
    },


    {
        "[EAST LON]",
        "f64p9",
        {(void *) &header_data.head.mbr.max_x},
        NULL,
        30
    },

    /************************************************************************/


    {
        "[MIN Y]",
        "f64p9",
        {(void *) &header_data.head.mbr.min_y},
        NULL,
        0
    },


    {
        "[MIN X]",
        "f64p9",
        {(void *) &header_data.head.mbr.min_x},
        NULL,
        0
    },


    {
        "[MAX Y]",
        "f64p9",
        {(void *) &header_data.head.mbr.max_y},
        NULL,
        0
    },


    {
        "[MAX X]",
        "f64p9",
        {(void *) &header_data.head.mbr.max_x},
        NULL,
        0
    },


    {
        "[BIN SIZE XY]",
        "f64p15",
        {(void *) &header_data.head.bin_size_xy},
        NULL,
        0
    },


    {
        "[X BIN SIZE]",
        "f64p15",
        {(void *) &header_data.head.x_bin_size_degrees},
        NULL,
        0
    },


    {
        "[Y BIN SIZE]",
        "f64p15",
        {(void *) &header_data.head.y_bin_size_degrees},
        NULL,
        0
    },


    {
        "[BIN WIDTH]",
        "i32",
        {(void *) &header_data.head.bin_width},
        NULL,
        0
    },


    {
        "[BIN HEIGHT]",
        "i32",
        {(void *) &header_data.head.bin_height},
        NULL,
        0
    },


    {
        "[MIN FILTERED DEPTH]",
        "f32",
        {(void *) &header_data.head.min_filtered_depth},
        NULL,
        0
    },


    {
        "[MAX FILTERED DEPTH]",
        "f32",
        {(void *) &header_data.head.max_filtered_depth},
        NULL,
        0
    },


    {
        "[MIN FILTERED COORD]",
        "i32c2",
        {(void *) &header_data.head.min_filtered_coord},
        NULL,
        0
    },


    {
        "[MAX FILTERED COORD]",
        "i32c2",
        {(void *) &header_data.head.max_filtered_coord},
        NULL,
        0
    },


    {
        "[MIN DEPTH]",
        "f32",
        {(void *) &header_data.head.min_depth},
        NULL,
        0
    },


    {
        "[MAX DEPTH]",
        "f32",
        {(void *) &header_data.head.max_depth},
        NULL,
        0
    },


    {
        "[MIN COORD]",
        "i32c2",
        {(void *) &header_data.head.min_coord},
        NULL,
        0
    },


    {
        "[MAX COORD]",
        "i32c2",
        {(void *) &header_data.head.max_coord},
        NULL,
        0
    },


    {
        "[COUNT BITS]",
        "ui32",
        {(void *) &header_data.count_bits},
        NULL,
        0
    },


    {
        "[STD BITS]",
        "ui32",
        {(void *) &header_data.std_bits},
        NULL,
        0
    },


    {
        "[STD SCALE]",
        "f32",
        {(void *) &header_data.std_scale},
        NULL,
        0
    },


    {
        "[DEPTH BITS]",
        "ui32",
        {(void *) &header_data.depth_bits},
        NULL,
        0
    },


    {
        "[DEPTH SCALE]",
        "f32",
        {(void *) &header_data.depth_scale},
        NULL,
        0
    },


    {
        "[DEPTH OFFSET]",
        "f32",
        {(void *) &header_data.depth_offset},
        NULL,
        0
    },


    /************************* Version Dependency ****************************/

    /*  The following section supports pre-4.0 format files.  */

    {
        "[EDITED FLAG BITS]",
        "ui32",
        {(void *) &header_data.edited_flag_bits},
        NULL,
        30
    },


    {
        "[CHECKED FLAG BITS]",
        "ui32",
        {(void *) &header_data.checked_flag_bits},
        NULL,
        30
    },


    {
        "[SUSPECT FLAG BITS]",
        "ui32",
        {(void *) &header_data.suspect_flag_bits},
        NULL,
        30
    },


    {
        "[SELECTED FLAG BITS]",
        "ui32",
        {(void *) &header_data.selected_flag_bits},
        NULL,
        30
    },


    {
        "[CLASS 1 FLAG BITS]",
        "ui32",
        {(void *) &header_data.class1_flag_bits},
        NULL,
        30
    },


    {
        "[CLASS 2 FLAG BITS]",
        "ui32",
        {(void *) &header_data.class2_flag_bits},
        NULL,
        30
    },


    {
        "[PROJECTED X BITS]",
        "ui32",
        {(void *) &header_data.projected_x_bits},
        NULL,
        30
    },


    {
        "[PROJECTED X OFFSET]",
        "ui32",
        {(void *) &header_data.projected_x_offset},
        NULL,
        30
    },


    {
        "[DATA FLAG BITS]",
        "ui32",
        {(void *) &header_data.data_flag_bits},
        NULL,
        30
    },

    /*************************************************************************/


    {
        "[RECORD POINTER BITS]",
        "ui32",
        {(void *) &header_data.record_pointer_bits},
        NULL,
        0
    },


    {
        "[FILE NUMBER BITS]",
        "ui32",
        {(void *) &header_data.file_number_bits},
        NULL,
        0
    },


    {
        "[LINE NUMBER BITS]",
        "ui32",
        {(void *) &header_data.line_number_bits},
        NULL,
        0
    },


    {
        "[PING NUMBER BITS]",
        "ui32",
        {(void *) &header_data.ping_number_bits},
        NULL,
        0
    },


    {
        "[BEAM NUMBER BITS]",
        "ui32",
        {(void *) &header_data.beam_number_bits},
        NULL,
        0
    },


    {
        "[OFFSET BITS]",
        "ui32",
        {(void *) &header_data.offset_bits},
        NULL,
        0
    },


    {
        "[VALIDITY BITS]",
        "ui32",
        {(void *) &header_data.validity_bits},
        NULL,
        0
    },


    {
        "[POINT]",
        "f64c2",
        {(void *) &header_data.head.polygon},
        &header_data.head.polygon_count,
        0
    },


    {
        "[MINIMUM BIN COUNT]",
        "i32",
        {(void *) &header_data.head.min_bin_count},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN COUNT]",
        "i32",
        {(void *) &header_data.head.max_bin_count},
        NULL,
        0
    },


    {
        "[MIN COUNT COORD]",
        "i32c2",
        {(void *) &header_data.head.min_count_coord},
        NULL,
        0
    },


    {
        "[MAX COUNT COORD]",
        "i32c2",
        {(void *) &header_data.head.max_count_coord},
        NULL,
        0
    },


    {
        "[MIN STANDARD DEVIATION]",
        "f64",
        {(void *) &header_data.head.min_standard_dev},
        NULL,
        0
    },


    {
        "[MAX STANDARD DEVIATION]",
        "f64",
        {(void *) &header_data.head.max_standard_dev},
        NULL,
        0
    },


    {
        "[CHART SCALE]",
        "f32",
        {(void *) &header_data.head.chart_scale},
        NULL,
        0
    },


    {
        "[CLASS TYPE]",
        "i32",
        {(void *) &header_data.head.class_type},
        NULL,
        0
    },


    {
        "[PROJECTION]",
        "uc",
        {(void *) &header_data.head.proj_data.projection},
        NULL,
        0
    },


    {
        "[PROJECTION ZONE]",
        "ui16",
        {(void *) &header_data.head.proj_data.zone},
        NULL,
        0
    },


    {
        "[HEMISPHERE]",
        "uc",
        {(void *) &header_data.head.proj_data.hemisphere},
        NULL,
        0
    },


    {
        "[WELL-KNOWN TEXT]",
        "string",
        {(void *) header_data.head.proj_data.wkt},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 0]",
        "f64p3",
        {(void *) &header_data.head.proj_data.params[0]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 1]",
        "f64p3",
        {(void *) &header_data.head.proj_data.params[1]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 2]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[2]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 3]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[3]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 4]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[4]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 5]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[5]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 6]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[6]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 7]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[7]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 8]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[8]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 9]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[9]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 10]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[10]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 11]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[11]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 12]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[12]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 13]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[13]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 14]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[14]},
        NULL,
        0
    },


    {
        "[PROJECTION PARAMETER 15]",
        "f64p12",
        {(void *) &header_data.head.proj_data.params[15]},
        NULL,
        0
    },


    {
        "[AVERAGE FILTERED NAME]",
        "string",
        {(void *) header_data.head.average_filt_name},
        NULL,
        0
    },


    {
        "[AVERAGE NAME]",
        "string",
        {(void *) header_data.head.average_name},
        NULL,
        0
    },


    {
        "[DYNAMIC RELOAD]",
        "b",
        {(void *) &header_data.head.dynamic_reload},
        NULL,
        0
    },


    /************************* Version Dependency ****************************/

    /*  The following section supports pre-5.0 format files.  */

    {
        "[NUMBER OF ATTRIBUTES]",
        "ui16",
        {(void *) &header_data.head.num_bin_attr},
        NULL,
        47
    },


    {
        "[ATTRIBUTE 0]",
        "string",
        {(void *) header_data.head.bin_attr_name[0]},
        NULL,
        47
    },


    {
        "[ATTRIBUTE 1]",
        "string",
        {(void *) header_data.head.bin_attr_name[1]},
        NULL,
        47
    },


    {
        "[ATTRIBUTE 2]",
        "string",
        {(void *) header_data.head.bin_attr_name[2]},
        NULL,
        47
    },


    {
        "[MINIMUM ATTRIBUTE 0]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[0]},
        NULL,
        47
    },


    {
        "[MAXIMUM ATTRIBUTE 0]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[0]},
        NULL,
        47
    },


    {
        "[MINIMUM ATTRIBUTE 1]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[1]},
        NULL,
        47
    },


    {
        "[MAXIMUM ATTRIBUTE 1]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[1]},
        NULL,
        47
    },


    {
        "[MINIMUM ATTRIBUTE 2]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[2]},
        NULL,
        47
    },


    {
        "[MAXIMUM ATTRIBUTE 2]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[2]},
        NULL,
        47
    },


    {
        "[ATTRIBUTE BITS 0]",
        "i32",
        {(void *) &header_data.bin_attr_bits[0]},
        NULL,
        47
    },


    {
        "[ATTRIBUTE BITS 1]",
        "i32",
        {(void *) &header_data.bin_attr_bits[1]},
        NULL,
        47
    },


    {
        "[ATTRIBUTE BITS 2]",
        "i32",
        {(void *) &header_data.bin_attr_bits[2]},
        NULL,
        47
    },


    {
        "[ATTRIBUTE SCALE 0]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[0]},
        NULL,
        47
    },


    {
        "[ATTRIBUTE SCALE 1]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[1]},
        NULL,
        47
    },


    {
        "[ATTRIBUTE SCALE 2]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[2]},
        NULL,
        47
    },


    /*************************************************************************/


    {
        "[NUMBER OF BIN ATTRIBUTES]",
        "ui16",
        {(void *) &header_data.head.num_bin_attr},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 0]",
        "string",
        {(void *) header_data.head.bin_attr_name[0]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 1]",
        "string",
        {(void *) header_data.head.bin_attr_name[1]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE 2]",
        "string",
        {(void *) header_data.head.bin_attr_name[2]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 3]",
        "string",
        {(void *) header_data.head.bin_attr_name[3]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 4]",
        "string",
        {(void *) header_data.head.bin_attr_name[4]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 5]",
        "string",
        {(void *) header_data.head.bin_attr_name[5]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 6]",
        "string",
        {(void *) header_data.head.bin_attr_name[6]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 7]",
        "string",
        {(void *) header_data.head.bin_attr_name[7]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 8]",
        "string",
        {(void *) header_data.head.bin_attr_name[8]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE 9]",
        "string",
        {(void *) header_data.head.bin_attr_name[9]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE OFFSET 0]",
        "f32",
        {(void *) &header_data.bin_attr_offset[0]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 1]",
        "f32",
        {(void *) &header_data.bin_attr_offset[1]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 2]",
        "f32",
        {(void *) &header_data.bin_attr_offset[2]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 3]",
        "f32",
        {(void *) &header_data.bin_attr_offset[3]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 4]",
        "f32",
        {(void *) &header_data.bin_attr_offset[4]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 5]",
        "f32",
        {(void *) &header_data.bin_attr_offset[5]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 6]",
        "f32",
        {(void *) &header_data.bin_attr_offset[6]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 7]",
        "f32",
        {(void *) &header_data.bin_attr_offset[7]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 8]",
        "f32",
        {(void *) &header_data.bin_attr_offset[8]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE OFFSET 9]",
        "f32",
        {(void *) &header_data.bin_attr_offset[9]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 0]",
        "f32",
        {(void *) &header_data.bin_attr_max[0]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE MAX 1]",
        "f32",
        {(void *) &header_data.bin_attr_max[1]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 2]",
        "f32",
        {(void *) &header_data.bin_attr_max[2]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 3]",
        "f32",
        {(void *) &header_data.bin_attr_max[3]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 4]",
        "f32",
        {(void *) &header_data.bin_attr_max[4]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 5]",
        "f32",
        {(void *) &header_data.bin_attr_max[5]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 6]",
        "f32",
        {(void *) &header_data.bin_attr_max[6]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 7]",
        "f32",
        {(void *) &header_data.bin_attr_max[7]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 8]",
        "f32",
        {(void *) &header_data.bin_attr_max[8]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE MAX 9]",
        "f32",
        {(void *) &header_data.bin_attr_max[9]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE NULL 0]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[0]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 1]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[1]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 2]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[2]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 3]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[3]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 4]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[4]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 5]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[5]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 6]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[6]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 7]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[7]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 8]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[8]},
        NULL,
        0
    },

    {
        "[BIN ATTRIBUTE NULL 9]",
        "f32",
        {(void *) &header_data.head.bin_attr_null[9]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 0]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[0]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 0]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[0]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 1]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[1]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 1]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[1]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 2]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[2]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 2]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[2]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 3]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[3]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 3]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[3]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 4]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[4]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 4]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[4]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 5]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[5]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 5]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[5]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 6]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[6]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 6]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[6]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 7]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[7]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 7]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[7]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 8]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[8]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 8]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[8]},
        NULL,
        0
    },


    {
        "[MINIMUM BIN ATTRIBUTE 9]",
        "f32",
        {(void *) &header_data.head.min_bin_attr[9]},
        NULL,
        0
    },


    {
        "[MAXIMUM BIN ATTRIBUTE 9]",
        "f32",
        {(void *) &header_data.head.max_bin_attr[9]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 0]",
        "i32",
        {(void *) &header_data.bin_attr_bits[0]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 1]",
        "i32",
        {(void *) &header_data.bin_attr_bits[1]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 2]",
        "i32",
        {(void *) &header_data.bin_attr_bits[2]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 3]",
        "i32",
        {(void *) &header_data.bin_attr_bits[3]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 4]",
        "i32",
        {(void *) &header_data.bin_attr_bits[4]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 5]",
        "i32",
        {(void *) &header_data.bin_attr_bits[5]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 6]",
        "i32",
        {(void *) &header_data.bin_attr_bits[6]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 7]",
        "i32",
        {(void *) &header_data.bin_attr_bits[7]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 8]",
        "i32",
        {(void *) &header_data.bin_attr_bits[8]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE BITS 9]",
        "i32",
        {(void *) &header_data.bin_attr_bits[9]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 0]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[0]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 1]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[1]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 2]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[2]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 3]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[3]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 4]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[4]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 5]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[5]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 6]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[6]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 7]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[7]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 8]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[8]},
        NULL,
        0
    },


    {
        "[BIN ATTRIBUTE SCALE 9]",
        "f32",
        {(void *) &header_data.head.bin_attr_scale[9]},
        NULL,
        0
    },


    {
        "[NUMBER OF NDX ATTRIBUTES]",
        "ui16",
        {(void *) &header_data.head.num_ndx_attr},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 0]",
        "string",
        {(void *) header_data.head.ndx_attr_name[0]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 1]",
        "string",
        {(void *) header_data.head.ndx_attr_name[1]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 2]",
        "string",
        {(void *) header_data.head.ndx_attr_name[2]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 3]",
        "string",
        {(void *) header_data.head.ndx_attr_name[3]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 4]",
        "string",
        {(void *) header_data.head.ndx_attr_name[4]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 5]",
        "string",
        {(void *) header_data.head.ndx_attr_name[5]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 6]",
        "string",
        {(void *) header_data.head.ndx_attr_name[6]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 7]",
        "string",
        {(void *) header_data.head.ndx_attr_name[7]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 8]",
        "string",
        {(void *) header_data.head.ndx_attr_name[8]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE 9]",
        "string",
        {(void *) header_data.head.ndx_attr_name[9]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 0]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[0]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 0]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[0]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 1]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[1]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 1]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[1]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 2]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[2]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 2]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[2]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 3]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[3]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 3]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[3]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 4]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[4]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 4]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[4]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 5]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[5]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 5]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[5]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 6]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[6]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 6]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[6]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 7]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[7]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 7]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[7]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 8]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[8]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 8]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[8]},
        NULL,
        0
    },


    {
        "[MINIMUM NDX ATTRIBUTE 9]",
        "f32",
        {(void *) &header_data.head.min_ndx_attr[9]},
        NULL,
        0
    },


    {
        "[MAXIMUM NDX ATTRIBUTE 9]",
        "f32",
        {(void *) &header_data.head.max_ndx_attr[9]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 0]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[0]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 1]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[1]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 2]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[2]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 3]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[3]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 4]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[4]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 5]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[5]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 6]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[6]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 7]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[7]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 8]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[8]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE BITS 9]",
        "i32",
        {(void *) &header_data.ndx_attr_bits[9]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 0]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[0]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 1]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[1]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 2]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[2]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 3]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[3]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 4]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[4]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 5]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[5]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 6]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[6]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 7]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[7]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 8]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[8]},
        NULL,
        0
    },


    {
        "[NDX ATTRIBUTE SCALE 9]",
        "f32",
        {(void *) &header_data.head.ndx_attr_scale[9]},
        NULL,
        0
    },


    {
        "[USER FLAG 1 NAME]",
        "string",
        {(void *) header_data.head.user_flag_name[0]},
        NULL,
        0
    },


    {
        "[USER FLAG 2 NAME]",
        "string",
        {(void *) header_data.head.user_flag_name[1]},
        NULL,
        0
    },


    {
        "[USER FLAG 3 NAME]",
        "string",
        {(void *) header_data.head.user_flag_name[2]},
        NULL,
        0
    },


    {
        "[USER FLAG 4 NAME]",
        "string",
        {(void *) header_data.head.user_flag_name[3]},
        NULL,
        0
    },


    {
        "[USER FLAG 5 NAME]",
        "string",
        {(void *) header_data.head.user_flag_name[4]},
        NULL,
        0
    },


    {
        "[COVERAGE MAP ADDRESS]",
        "i64",
        {(void *) &header_data.coverage_map_address},
        NULL,
        0
    },


    {
        "[HORIZONTAL ERROR BITS]",
        "i32",
        {(void *) &header_data.horizontal_error_bits},
        NULL,
        0
    },


    {
        "[HORIZONTAL ERROR SCALE]",
        "f32",
        {(void *) &header_data.head.horizontal_error_scale},
        NULL,
        0
    },


    {
        "[MAXIMUM HORIZONTAL ERROR]",
        "f32",
        {(void *) &header_data.head.max_horizontal_error},
        NULL,
        0
    },


    {
        "[VERTICAL ERROR BITS]",
        "i32",
        {(void *) &header_data.vertical_error_bits},
        NULL,
        0
    },


    {
        "[VERTICAL ERROR SCALE]",
        "f32",
        {(void *) &header_data.head.vertical_error_scale},
        NULL,
        0
    },


    {
        "[MAXIMUM VERTICAL ERROR]",
        "f32",
        {(void *) &header_data.head.max_vertical_error},
        NULL,
        0
    },


    {
        "[NULL DEPTH]",
        "f32",
        {(void *) &header_data.head.null_depth},
        NULL,
        0
    }
};


#endif
