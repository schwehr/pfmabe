#ifndef __TOD2_DHT_H__
#define __TOD2_DHT_H__


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32             id;
  NV_CHAR              vpf_version[11];
  NV_CHAR              database_name[9];
  NV_CHAR              database_description[101];
  NV_CHAR              media_standard[21];
  NV_CHAR              originator[51];
  NV_CHAR              addressee[101];
  NV_CHAR              media_volumes[5];
  NV_CHAR              *seq_numbers;
  NV_CHAR              num_data_sets[5];
  NV_CHAR              security_class;
  NV_CHAR              downgrading[4];
  NV_CHAR              downgrade_date[21];
  NV_CHAR              releasability[21];
  NV_CHAR              transmittal_id;
  NV_CHAR              edition_number[11];
  NV_CHAR              edition_date[21];
  NV_CHAR              *declassification;
  NV_CHAR              *class_just;
} TOD2_DHT;


  TOD2_DHT read_tod2_dht (NV_CHAR *path);
  void dump_tod2_dht (TOD2_DHT data);


#ifdef  __cplusplus
}
#endif

#endif
