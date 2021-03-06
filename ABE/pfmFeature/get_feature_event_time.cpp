#include "pfmFeatureDef.hpp"

#include "gsf.h"
#include "wlf.h"
#include "FileHydroOutput.h"
#include "FileTopoOutput.h"


//  This is a data type specific method of getting the POSIX time of a depth record from the original input file.

void get_feature_event_time (NV_INT32 pfm_handle, DEPTH_RECORD depth, time_t *tv_sec, long *tv_nsec)
{
  NV_INT16         type;
  NV_CHAR          filename[512];
  gsfDataID        gsf_data_id;
  gsfRecords       gsf_record;
  extern int       gsfError;
  HYDRO_OUTPUT_T   hof_record;
  TOPO_OUTPUT_T    tof_record;
  WLF_HEADER       wlf_header;
  WLF_RECORD       wlf_record;
  FILE             *hof_fp, *tof_fp;
  NV_INT32         gsf_handle, wlf_handle;



  read_list_file (pfm_handle, depth.file_number, filename, &type);


  //  Just in case it's not one of our supported formats, get the current time (that's about the best we can do).

  QDateTime current_time = QDateTime::currentDateTime ();

  NV_INT32 year = current_time.date ().year ();
  NV_INT32 day = current_time.date ().dayOfYear ();
  NV_INT32 hour = current_time.time ().hour ();
  NV_INT32 minute = current_time.time ().minute ();
  NV_FLOAT32 second = current_time.time ().second ();

  inv_cvtime (year - 1900, day, hour, minute, second, tv_sec, tv_nsec);



  switch (type)
    {
    case PFM_GSF_DATA:
      if (gsfOpen (filename, GSF_READONLY_INDEX, &gsf_handle))
        {
          if (gsfError != GSF_FOPEN_ERROR) gsfClose (gsf_handle);
          return;
        }

      gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
      gsf_data_id.record_number = depth.ping_number;
      if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_record, NULL, 0) < 0) return;

      *tv_sec = gsf_record.mb_ping.ping_time.tv_sec;
      *tv_nsec = gsf_record.mb_ping.ping_time.tv_nsec;

      gsfClose (gsf_handle);
      break;

    case PFM_SHOALS_1K_DATA:
    case PFM_CHARTS_HOF_DATA:
      if ((hof_fp = open_hof_file (filename)) == NULL) return;
      hof_read_record (hof_fp, depth.ping_number, &hof_record);

      charts_cvtime (hof_record.timestamp, &year, &day, &hour, &minute, &second);
      inv_cvtime (year, day, hour, minute, second, tv_sec, tv_nsec);

      fclose (hof_fp);
      break;

    case PFM_SHOALS_TOF_DATA:
      if ((tof_fp = open_tof_file (filename)) == NULL) return;
      tof_read_record (tof_fp, depth.ping_number, &tof_record);

      charts_cvtime (tof_record.timestamp, &year, &day, &hour, &minute, &second);
      inv_cvtime (year, day, hour, minute, second, tv_sec, tv_nsec);

      fclose (tof_fp);
      break;

    case PFM_WLF_DATA:
      if ((wlf_handle = wlf_open_file (filename, &wlf_header, WLF_READONLY)) < 0) return;
      if (wlf_read_record (wlf_handle, depth.ping_number, &wlf_record, NVFalse, NULL) < 0)
        {
          wlf_close_file (wlf_handle);
          return;
        }

      *tv_sec = wlf_record.tv_sec;
      *tv_nsec = wlf_record.tv_nsec;

      wlf_close_file (wlf_handle);
      break;
    }
}
