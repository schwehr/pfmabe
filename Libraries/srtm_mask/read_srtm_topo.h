/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#ifndef _READ_SRTM_TOPO_H_
#define _READ_SRTM_TOPO_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include "read_srtm1_topo.h"
#include "read_srtm2_topo.h"
#include "read_srtm3_topo.h"
#include "read_srtm30_topo.h"


  void set_exclude_srtm2_data (NV_BOOL flag);
  NV_INT32 read_srtm_topo_one_degree (NV_INT32 lat, NV_INT32 lon, NV_INT16 **array);
  NV_INT16 read_srtm_topo (NV_FLOAT64 lat, NV_FLOAT64 lon);
  void cleanup_srtm_topo ();


#ifdef  __cplusplus
}
#endif

#endif
