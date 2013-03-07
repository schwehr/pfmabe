#include "nvtypes.h"

/*****************************************************************************
 * FileWave.h      Header
 *
 * Purpose:        This header file hold the definition of the .inh file 
 *                 structure 
 *
 * Revision History:
 *
 ****************************************************************************/

#ifndef __WAVE_H__
#define __WAVE_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "charts.h"


#define    WAVE_NEXT_RECORD           (-1)


typedef struct
{
  NV_CHAR        file_type[128];
  NV_BOOL        endian;

  NV_FLOAT32     software_version;
  NV_FLOAT32     file_version;

  NV_INT32       header_size;
  NV_INT32       text_block_size;
  NV_INT32       bin_block_size;
  NV_INT32       hardware_block_size;
  NV_INT32       haps_block_size;
  NV_INT32       other_block_size;

  NV_INT16       record_size;
  NV_INT16       shot_data_size;
  NV_INT16       wave_form_size;
  NV_INT16       pmt_size;
  NV_INT16       apd_size;
  NV_INT16       ir_size;
  NV_INT16       raman_size;

  NV_BYTE        ab_system_type;
  NV_BYTE        ab_system_number;
  NV_INT16       system_rep_rate;

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
  NV_U_INT16     ac_zero_offset[4];  /*  These aren't stored in the ASCII portion of the header.
                                         They are stored in the hardware portion of the header
                                         at offset 572 from the beginning of the hardware
                                         header block.  */
} WAVE_HEADER_T;

typedef struct
{
  NV_INT64       timestamp;        /* time in microseconds from 01/01/1970 */
  NV_U_BYTE      *shot_data;       /* undefined shot data */
  NV_U_BYTE      *pmt;
  NV_U_BYTE      *apd;
  NV_U_BYTE      *ir;
  NV_U_BYTE      *raman;
} WAVE_DATA_T;


  NV_INT32 wave_read_header (FILE *fp, WAVE_HEADER_T *head);
  FILE *open_wave_file (NV_CHAR *path);
  NV_INT32 wave_read_record (FILE *fp, NV_INT32 num, WAVE_DATA_T *record);
  void wave_dump_record (WAVE_DATA_T record);


#ifdef  __cplusplus
}
#endif


#endif
