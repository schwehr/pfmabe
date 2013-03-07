
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "pfmViewDef.hpp"

#include "gsf.h"
#include "wlf.h"
#include "czmil.h"
#include "FileHydroOutput.h"
#include "FileTopoOutput.h"


//!  This is a data type specific method of getting the POSIX time of a depth record from the original input file.

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
  CZMIL_CXY_Header czmil_header;
  CZMIL_CXY_Data   czmil_record;
  FILE             *hof_fp, *tof_fp;
  NV_INT32         gsf_handle, wlf_handle, czmil_handle;



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
      inv_cvtime (year - 1900, day, hour, minute, second, tv_sec, tv_nsec);

      fclose (hof_fp);
      break;

    case PFM_SHOALS_TOF_DATA:
      if ((tof_fp = open_tof_file (filename)) == NULL) return;
      tof_read_record (tof_fp, depth.ping_number, &tof_record);

      charts_cvtime (tof_record.timestamp, &year, &day, &hour, &minute, &second);
      inv_cvtime (year - 1900, day, hour, minute, second, tv_sec, tv_nsec);

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

    case PFM_CZMIL_DATA:
      czmil_handle = czmil_open_file (filename, &czmil_header, CZMIL_READONLY, NVTrue);
      if (czmil_handle != CZMIL_SUCCESS) return;

      czmil_read_cxy_record (czmil_handle, depth.ping_number, &czmil_record);

      czmil_cvtime (czmil_record.channel[CZMIL_DEEP_CHANNEL].timestamp, &year, &day, &hour, &minute, &second);
      inv_cvtime (year - 1900, day, hour, minute, second, tv_sec, tv_nsec);

      czmil_close_file (czmil_handle);
      break;
    }
}
