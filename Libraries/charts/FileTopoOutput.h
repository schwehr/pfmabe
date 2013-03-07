#include "nvtypes.h"

/*****************************************************************************
 * FileTopoOutput.h         Header
 *
 * Purpose:    This header file hold the definition of the topographic inputs 
 *             structure for use with the the inputs file.
 *              
 *
 * Revision History:
 *
 * 02/07/29 DR  -cleanup from SHOALSNT
 *
 * 02/08/15 DR  -continued implimentation as per DS/AP DD 
 *
 * 02/09/05 DR  -added HAPS back to header...
 *
 * 02/10/31 DR  -added new define for status as a byte of bits (for 
 *               deleted/kept,swapped) instead of a short.  
 *              -killed flag was removed (use bit in status)
 *              -defines for bits added 
 * 
 * 03/06/09 DR  -added username to header...
 *
 * 03/06/13 DR  -redefined TOPO_OUTPUT structure.. Removed Flightline, 
 *               Taps version, reduced intensity to char (now 64 bytes/record)
 *
 * 03/06/27 DR  -changed 1 to first and 2 to second in some names...
 *
 * 08/02/03 JD  -modified for NAVO data types
 *
 * 04/08/04 JD  -added text header info as per GCS_file_formats.pdf
 *
 ****************************************************************************/

#ifndef TOFHead
#define TOFHead


#ifdef  __cplusplus
extern "C" {
#endif


#include "charts.h"


/* Full header will be a 16k block. */


/*

Full Output header is 256k:

Header Text        8k -has WAY more than I can use but keep extra for future
Binary             8k -Binary - date, start timestamp, stop timestamp, planning info (points).... will also contain most 
                   of the items in the "text" block in BIN form

*/


#define  TOF_HEAD_TEXT_BLK_SIZE        (8 * 1024)
#define  TOF_HEAD_BIN_BLK_SIZE         (8 * 1024)
#define  TOF_HEAD_BIN_OFFSET           (TOF_HEAD_TEXT_BLK_SIZE)


#define  TOF_HEAD_SIZE                 (TOF_HEAD_TEXT_BLK_SIZE +  TOF_HEAD_BIN_BLK_SIZE)


#define  TOF_NEXT_RECORD               (-1)


typedef struct
{
  NV_INT64       start_time;       /* time in microseconds from 01/01/1970 */
  NV_INT64       stop_time;        /* time in microseconds from 01/01/1970 */
  NV_INT64       file_create_time; /* timestamp when file was created...   */
  NV_INT64       file_mod_time;    /* timestamp when file was last         */
                                   /* modified **TBR**                     */

  NV_FLOAT64     min_lat;
  NV_FLOAT64     min_long;
  NV_FLOAT64     max_lat;
  NV_FLOAT64     max_long;

  NV_INT64       fill_i64[6];

  NV_INT32       num_shots;
  NV_FLOAT32     gcs_version;
  NV_FLOAT32     gcs_file_version;

  NV_INT32       header_size;
  NV_INT32       text_block_size;
  NV_INT32       binary_block_size;
  NV_INT32       record_size;

  NV_INT32       ab_system_type;
  NV_INT32       ab_system_number;
  NV_INT32       rep_rate;

  NV_INT16       data_type;        /* 0 = normal (DGPS), 1 = kinematic GPS */
  NV_INT16       fill_short;

  NV_INT32       fill_i32[11];

  NV_U_INT16     flightline;       /* Line number is flightline/10 and     */
                                   /* version is remainder (ie. 13 is      */
                                   /* 1-3)                                 */
  NV_INT16       fill_i16[7];
    
  NV_CHAR        UserName[64];     /* Who processed this (whole) line last */

  NV_CHAR        TBD[960];

  NV_CHAR        project[128];
  NV_CHAR        mission[128];
  NV_CHAR        dataset[128];
} TOF_INFO_T;


typedef struct
{
  NV_CHAR        file_type[128];
  NV_BOOL        endian;

  NV_FLOAT32     software_version;
  NV_FLOAT32     file_version;

  NV_INT32       header_size;
  NV_INT32       text_block_size;
  NV_INT32       bin_block_size;
  NV_INT32       record_size;

  NV_BYTE        ab_system_type;
  NV_BYTE        ab_system_number;
  NV_INT16       system_rep_rate;

  NV_CHAR        UserName[64];     /* Who processed this (whole) line last */

  NV_CHAR        project[64];
  NV_CHAR        mission[64];
  NV_CHAR        dataset[64];
  NV_CHAR        flightline_number[64];
  NV_INT16       coded_fl_number;
  NV_CHAR        flight_date[64];
  NV_CHAR        start_time[64];
  NV_CHAR        end_time[64];
  NV_INT64       start_timestamp;  /* time in microseconds from 01/01/1970 */
  NV_INT64       end_timestamp;    /* time in microseconds from 01/01/1970 */
  NV_INT32       number_shots;
  NV_CHAR        dataset_create_date[64];
  NV_CHAR        dataset_create_time[64];
  NV_FLOAT64     line_min_lat;
  NV_FLOAT64     line_max_lat;
  NV_FLOAT64     line_min_lon;
  NV_FLOAT64     line_max_lon;
} TOF_TEXT_T;


typedef struct
{
  /*  8k Text info block */

  TOF_TEXT_T     text;
  NV_CHAR        text_block[(TOF_HEAD_TEXT_BLK_SIZE) - sizeof(TOF_TEXT_T)];


  /* 8k Flightline block */

  TOF_INFO_T     info;
  NV_CHAR        bin_block[(TOF_HEAD_BIN_BLK_SIZE) - sizeof(TOF_INFO_T)];
} TOF_HEADER_T;


typedef struct
{
  NV_INT64       timestamp;

  NV_FLOAT64     latitude_first;
  NV_FLOAT64     longitude_first;
  NV_FLOAT64     latitude_last;
  NV_FLOAT64     longitude_last;

  NV_FLOAT32     elevation_first;        
  NV_FLOAT32     elevation_last;

  NV_FLOAT32     scanner_azimuth;
  NV_FLOAT32     nadir_angle;

  NV_CHAR        conf_first;  /*  Below 50 is bad  */
  NV_CHAR        conf_last;   /*  Below 50 is bad  */

  NV_U_CHAR      intensity_first;        
  NV_U_CHAR      intensity_last;        


  /*   Classification status byte (bits defined in GCSCnsts.h)

       bit 0 = classified        (1=classified; 0=not classified)

       bit 1 = non-bare_earth1   (0x02);

       bit 2 = non-bare_earth2   (0x04);

       bit 3 = water             (0x08);   

       rest TBD.

       If only the first bit is set it is BARE EARTH.

       If value is 0, it has not been classified.

       Yes, this is a weird definition. 

       In future the two non bare earth bits will be split into

       buildings/vegetation.  */

  NV_U_CHAR      classification_status;

  NV_U_CHAR      tbd_1;

  NV_CHAR        pos_conf;
  NV_CHAR        tbd_2;

  NV_FLOAT32     result_elevation_first;
  NV_FLOAT32     result_elevation_last;
  NV_FLOAT32     altitude;
  NV_FLOAT32     tbdfloat;
} TOPO_OUTPUT_T;


  FILE *open_tof_file (NV_CHAR *path);
  NV_INT32 tof_read_header (FILE *fp, TOF_HEADER_T *head);
  NV_INT32 tof_read_record (FILE *fp, NV_INT32 num, TOPO_OUTPUT_T *record);
  NV_INT32 tof_write_header (FILE *fp, TOF_HEADER_T head);
  NV_INT32 tof_write_record (FILE *fp, NV_INT32 num, TOPO_OUTPUT_T record);
  void tof_dump_record (TOPO_OUTPUT_T record);


#ifdef  __cplusplus
}
#endif

#endif
