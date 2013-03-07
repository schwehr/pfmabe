#ifndef __HOFWAVEFILTERDEF_H__
#define __HOFWAVEFILTERDEF_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <getopt.h>


/* Local Includes. */

#include "nvutility.h"
#include "pfm.h"
#include "FileHydroOutput.h"
#include "FileWave.h"


#include <QtCore>
#include <QSharedMemory>


#define HWF_APD_SIZE  201
#define HWF_PMT_SIZE  501


typedef struct
{
  NV_INT32    pfm_file;
  NV_INT32    orig_rec;
  NV_INT32    rec;
} SORT_REC;


typedef struct
{
  NV_FLOAT64  mx;                        //  X position in meters
  NV_FLOAT64  my;                        //  Y position in meters
  NV_BOOL     check;                     //  Set if we need to check adjacent waveforms (e.g. this is an isolated point)
  NV_INT32    bot_bin_first;
  NV_INT32    bot_bin_second;
  NV_U_BYTE   apd[HWF_APD_SIZE];
  NV_U_BYTE   pmt[HWF_PMT_SIZE];
} WAVE_DATA;


typedef struct
{
  NV_INT32    count;
  NV_INT32    *data;
} BIN_DATA;


// General stuff.

typedef struct
{
  QSharedMemory *abeShare;                //  ABE's shared memory pointer.
  ABE_SHARE   *abe_share;                 //  Pointer to the ABE_SHARE structure in shared memory.
  QSharedMemory *dataShare;               //  Point cloud shared memory.
  POINT_CLOUD *data;                      //  Pointer to POINT_CLOUD structure in point cloud shared memory.  To see what is in the 
                                          //  POINT_CLOUD structure please see the ABE.h file in the nvutility library.
  NV_INT32    *points;                    //  Points within the search radius
  NV_INT32    point_count;                //  Number of points within search radius
  NV_FLOAT64  radius;
  NV_INT32    search_width;
  NV_INT32    rise_threshold;


  //  The following concern PFMs as layers.  There are a few things from ABE_SHARE that also need to be 
  //  addressed when playing with layers - open_args, display_pfm, and pfm_count.

  NV_F64_XYMBR total_mbr;                 //  MBR of all of the displayed PFMs
  NV_INT32    pfm_handle[MAX_ABE_PFMS];   //  PFM file handle
} MISC;


#endif
