/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/

#ifndef _READ_SRTM3_TOPO_H_
#define _READ_SRTM3_TOPO_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include "nvtypes.h"
#include "nvdef.h"


  NV_BOOL check_srtm3_topo ();
  NV_INT32 read_srtm3_topo_one_degree (NV_INT32 lat, NV_INT32 lon, NV_INT16 **array);
  NV_INT16 read_srtm3_topo (NV_FLOAT64 lat, NV_FLOAT64 lon);
  void cleanup_srtm3_topo ();


#ifdef  __cplusplus
}
#endif

#endif
