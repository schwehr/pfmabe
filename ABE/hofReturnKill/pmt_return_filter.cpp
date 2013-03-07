#include "hofReturnKill.hpp"


/***************************************************************************\
*                                                                           *
*   Module Name:        pmt_return_filter                                   *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2009                                        *
*                                                                           *
*   Purpose:            If the return is "insignificant" based on the       *
*                       leading slope, the trailing slope, and the run      *
*                       count then it will be marked as invalid.  Default   *
*                       value for the slopes was empirically derived by     *
*                       examining shitloads (that's a technical term) of    *
*                       data.                                               *
*                                                                           *
*   Arguments:          rec            - record number (used for debugging) *
*                       sub_rec        - sub_record number (0 is primary,   *
*                                        1 is secondary).                   *
*                       hof_record     - the HOF record                     *
*                       run_req        - run length required (default is 6) *
*                       slope_req      - required significant slope,        *
*                                        if the slope or backslope is less  *
*                                        than this the hit is insignificant *
*                                        (default is 0.75)                  *
*                                                                           *
*   Return Value:       NV_BOOL        - NVTrue if we need to kill the      *
*                                        return                             *
*                                                                           *
\***************************************************************************/

NV_BOOL pmt_return_filter (NV_INT32 rec, NV_INT32 sub_rec, HYDRO_OUTPUT_T hof_record, WAVE_HEADER_T wave_header, NV_INT32 pmt_run_req,
                           NV_FLOAT32 slope_req, WAVE_DATA_T wave_data);
NV_BOOL pmt_return_filter (NV_INT32 rec __attribute__ ((unused)), NV_INT32 sub_rec, HYDRO_OUTPUT_T *hof_record, NV_INT32 pmt_run_req,
                           NV_FLOAT32 slope_req, WAVE_DATA_T *wave_rec)
{
  //  Make sure the return we're looking for is not shallow water algorithm or shoreline depth swapped.

  if ((sub_rec == 0 && (hof_record->abdc == 72 || hof_record->abdc == 74)) ||
      (sub_rec == 1 && (hof_record->sec_abdc == 72 || hof_record->sec_abdc == 74))) return (NVFalse);


  //  Shorthand ;^)

  NV_INT32 channel = hof_record->bot_channel;
  NV_INT32 bin = hof_record->bot_bin_first;
  if (sub_rec == 0)
    {
      channel = hof_record->bot_channel;
      bin = hof_record->bot_bin_first;
    }
  else
    {
      channel = hof_record->sec_bot_chan;
      bin = hof_record->bot_bin_second;
    }


  //  Initialize the run and slope variables for the PMT data.

  NV_INT32 rise = 0;
  NV_INT32 drop = 0;
  NV_INT32 start_data = 0;
  NV_INT32 end_data = 0;
  NV_INT32 peak = 0;
  NV_INT32 run = 0;
  NV_INT32 back_run = 0;
  NV_FLOAT32 slope = 0.0;
  NV_FLOAT32 backslope = 0.0;


  //  Initialize the first_drop index.  We don't want to start searching for runs until we've cleared the surface
  //  return.  This is not how Optech does it but I'm not really interested in very shallow water for this filter.

  NV_INT32 first_drop = 0;


  //  Loop through the PMT data looking for the first drop (from the surface).  Skip the first 20
  //  bins so that we don't start looking in the noisy section prior to the surface return.

  for (NV_INT32 i = 20 ; i < HWF_PMT_SIZE ; i++)
    {
      //  If we get three zeros in a row we want to reset the drop counter.

      NV_INT32 change = (wave_rec->pmt[i] - wave_rec->pmt[i - 1]) + (wave_rec->pmt[i - 1] - wave_rec->pmt[i - 2]) +
        (wave_rec->pmt[i - 2] - wave_rec->pmt[i - 3]);

      if (!change) drop = 0;


      if (wave_rec->pmt[i] - wave_rec->pmt[i - 1] <= 0)
        {
          //  Increment the drop counter.

          drop++;


          if (drop >= 5)
            {
              first_drop = i;
              break;
            }
        }
      else
        {
          drop = 0;
        }
    }
              

  //  If the return bin is prior to the first drop (ie surface return) we want to go ahead and kill it.

  if (bin < first_drop) return (NVTrue);


  rise = 0;
  start_data = 0;


  //  Find the start of the run.

  for (NV_INT32 i = bin ; i >= 20 ; i--)
    {
      if (wave_rec->pmt[i] - wave_rec->pmt[i - 1] <= 0)
        {
          if (!start_data) start_data = i;

          rise++;

          if (rise >= 2) break;
        }
      else
        {
          rise = 0;
          start_data = 0;
        }
    }


  drop = 0;
  peak = 0;


  //  Find the peak.

  NV_INT32 length = qMin (bin + 50, HWF_PMT_SIZE - 1);

  for (NV_INT32 i = bin ; i < length ; i++)
    {
      if (wave_rec->pmt[i] - wave_rec->pmt[i - 1] < 0)
        {
          if (!peak) peak = i;

          drop++;

          if (drop >= 2) break;
        }
      else
        {
          drop = 0;
          peak = 0;
        }
    }


  //  Compute the slope.

  run = peak - start_data;
  slope = (NV_FLOAT32) (wave_rec->pmt[peak] - wave_rec->pmt[start_data]) / (NV_FLOAT32) run;


  length = qMin (peak + 50, HWF_PMT_SIZE - 1);


  //  Find the end of the backslope.

  for (NV_INT32 i = peak ; i < length ; i++)
    {
      if (wave_rec->pmt[i] - wave_rec->pmt[i - 1] > 1)
        {
          end_data = i;
          break;
        }
    }


  //  Force the backslope to be ignored if we don't have enough points to compute a decent one.
  //  The assumption being that if you got X rise points you want X fall points.

  back_run = end_data - peak;

  if (back_run < run)
    {
      backslope = 999.0;
    }
  else
    {
      backslope = (NV_FLOAT32) (wave_rec->pmt[peak] - wave_rec->pmt[end_data]) / (NV_FLOAT32) back_run;
    }


  //  If the slope is less than "slope_req" or the backslope is less than "slope_req" or the run length
  //  is less than "pmt_run_req" we want to kill the return.

  if (run < pmt_run_req || slope < slope_req)// || backslope < slope_req)
    {
      //fprintf(stderr,"%s %d %d %d %d %d\n",__FILE__,__LINE__,bin,start_data, peak, end_data);
      //fprintf(stderr,"%s %d %d %d %f %f %f\n",__FILE__,__LINE__,run,pmt_run_req,slope, backslope,slope_req);
      return (NVTrue);
    }


  //  If we got to here we didn't find anything.

  return (NVFalse);
}
