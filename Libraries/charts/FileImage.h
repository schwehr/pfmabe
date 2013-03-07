#include "nvtypes.h"

/*****************************************************************************
 * FileImage.h     Header
 *
 * Purpose:        This header file hold the definition of the image file 
 *                 structure 
 *
 * Revision History:
 * 02/07/15 DR    -wrote it
 *
 * 02/09/17 DR    -added FileImageOpen_PATH for opening with passed filename...
 *
 * 08/21/03 JD    -modified for NAVO data types
 *
 ****************************************************************************/

#ifndef HOF_TOF_Image
#define HOF_TOF_Image

#ifdef  __cplusplus
extern "C" {
#endif


#include "charts.h"


#define IMAGE_HEAD_TEXT_BLK_SIZE    (8 * 1024)
#define IMAGE_HEAD_BIN_BLK_SIZE     (8 * 1024)
#define IMAGE_HEAD_BIN_OFFSET       (IMAGE_HEAD_TEXT_BLK_SIZE)

#define IMAGE_HEAD_SIZE   (IMAGE_HEAD_TEXT_BLK_SIZE + IMAGE_HEAD_BIN_BLK_SIZE)



typedef struct
{
  NV_INT64    timestamp;            /* timestamp                           */
  NV_INT64    byte_offset;          /* offset in data area                 */
  NV_INT32    image_size;           /* size in bytes of jpeg               */
  NV_INT32    image_number;         /* image number in line                */
} OLD_IMAGE_INDEX_T;

typedef struct
{
  NV_INT64    timestamp;            /* timestamp                           */
  NV_INT64    byte_offset;          /* offset in data area                 */
  NV_INT32    image_size;           /* size in bytes of jpeg               */
  NV_INT32    image_number;         /* image number in line                */
  NV_CHAR     fill[160];
} IMAGE_INDEX_T;

typedef struct
{
  NV_CHAR        file_type[128];
  NV_BOOL        endian;

  NV_FLOAT32     software_version;
  NV_FLOAT32     file_version;

  NV_INT32       header_size;
  NV_INT32       text_block_size;
  NV_INT32       bin_block_size;

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
  NV_INT32       number_images;
  NV_CHAR        dataset_create_date[64];
  NV_CHAR        dataset_create_time[64];

  NV_INT32       record_size;      /* index record size  */
  NV_INT32       block_size;       /* index block size  */
  NV_INT32       data_size;        /* size of data block (images) */
} IMAGE_TEXT_T;


typedef struct
{
  NV_INT32    ImageHeaderSize;      /* Includes all text, info and index   */
                                    /* (offset to data)                    */
                                    /* (ImageTextHeaderSize + IndexSize)   */
  NV_INT32    ImageTextHeaderSize;  /* IMAGE_HEADER_SIZE                   */
  NV_INT32    IndexSize;            /* includes index info and index       */
                                    /* (sizeof(IMAGE_INDEX_INFO_T) +       */
                                    /* (sizeof(IMAGE_INDEX_T) *            */
                                    /* image_count)                        */
  NV_INT32    ImageIndexEntrySize;  /* sizeof(IMAGE_INDEX_T)               */
  NV_INT32    number_images;
  NV_INT32    fill_int;

  NV_INT64    start_timestamp;    
  NV_INT64    end_timestamp;        

  NV_U_INT16  flightline;    
  NV_U_INT16  fill_short;
  NV_INT32    fill_int2[15];        /* Future use...                       */
} IMAGE_INFO_T;


typedef struct
{
  /*  8k Text info block */

  IMAGE_TEXT_T text;
  NV_CHAR      text_block[(IMAGE_HEAD_TEXT_BLK_SIZE) - sizeof(IMAGE_TEXT_T)];


  /* 8k Binary block */

  IMAGE_INFO_T info;
  NV_CHAR      bin_block[(IMAGE_HEAD_BIN_BLK_SIZE) - sizeof(IMAGE_INFO_T)];
} IMAGE_HEADER_T;


  NV_INT32 image_read_header (FILE *fp, IMAGE_HEADER_T *head);
  FILE *open_image_file (NV_CHAR *path);
  NV_INT32 image_get_metadata (FILE *fp, NV_INT32 rec_num, IMAGE_INDEX_T *image_index);
  NV_INT32 image_find_record (FILE *fp, NV_INT64 timestamp);
  NV_U_CHAR *image_read_record (FILE *fp, NV_INT64 timestamp, NV_U_INT32 *size, NV_INT64 *image_time);
  NV_U_CHAR *image_read_record_recnum (FILE *fp, NV_INT32 recnum, NV_U_INT32 *size, NV_INT64 *image_time);
  NV_INT64 dump_image (NV_CHAR *file, NV_INT64 timestamp, NV_CHAR *path);


#ifdef  __cplusplus
}
#endif


#endif
