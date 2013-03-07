#ifndef _IO_DATA_H
#define _IO_DATA_H


#include <stdio.h>
#include <cerrno>

#include "geoSwath3DDef.hpp"

#include "charts.h"
#include "normtime.h"
#include "newgp.h"
#include "gsf.h"
#include "gsf_indx.h"
#include "hmpsflag.h"
#include "gsf_flags.hpp"
#include "FileHydroOutput.h"
#include "FileTopoOutput.h"
#include "wlf.h"
#include "hawkeye.h"

NV_INT32 io_data_open (MISC *misc, OPTIONS *options);
void io_data_close ();
void io_data_read (NV_INT32 num_records, POINT_DATA *data, QProgressBar *prog);
void io_data_write (NV_U_INT32 val, NV_INT32 rec, NV_INT32 sub, NV_FLOAT64 time);
NV_INT32 get_record ();
void set_record (NV_INT32 record);
NV_FLOAT64 rec_time (NV_INT32 record);


#endif
