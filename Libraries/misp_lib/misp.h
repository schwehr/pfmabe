#ifndef __MISP_H__
#define __MISP_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "nvutility.h"

#ifdef  __cplusplus
extern "C" {
#endif


#define MULT        3
#define FILELENGTH  132
#define MISPNULL    10000000000000000.0
#define OFFSET      0x10000000
#define SPLINE_COL  53
#define SPLINE_ROW  3

typedef struct
{
    NV_INT32       grid_cols;
    NV_INT32       grid_rows;
    NV_FLOAT32     x_min;
    NV_FLOAT32     y_min;
    NV_FLOAT32     x_max;
    NV_FLOAT32     y_max;
    NV_INT32       width;
    NV_INT32       height;
} MISP_HEADER;

  NV_INT32 misp_init (NV_FLOAT64 x_interval, NV_FLOAT64 y_interval, NV_FLOAT32 dlta, NV_INT32 reg_mfact, 
                      NV_FLOAT32 srch_rad, NV_INT32 err_cont, NV_FLOAT32 maxz, NV_FLOAT32 minz, 
                      NV_INT32 weight, NV_F64_XYMBR mbr);
  NV_INT32 misp_load (NV_F64_COORD3 xyz);
  NV_BOOL misp_proc ();
  NV_INT32 misp_rtrv (NV_FLOAT32 *array);
  NV_BOOL misp_progress_callback_registered ();
  void misp_progress (NV_CHAR *info);


typedef void (*MISP_PROGRESS_CALLBACK) (NV_CHAR *info);

void misp_register_progress_callback (MISP_PROGRESS_CALLBACK progressCB);


#ifdef  __cplusplus
}
#endif

#endif
