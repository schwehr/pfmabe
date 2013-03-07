#include "hofWaveFilter.hpp"

NV_BOOL waveform_check (MISC *misc, WAVE_DATA *wave_data, NV_INT32 recnum)
{
  NV_INT32 bin = wave_data[recnum].bot_bin_first;

  if (misc->data[recnum].sub) bin = wave_data[recnum].bot_bin_second;


  //  I'm not looking at surface data.

  if (bin < 20) return (NVFalse);


  for (NV_INT32 i = 0 ; i < misc->point_count ; i++)
    {
      NV_INT32 start_apd_search = qMax (20, bin - misc->abe_share->filterShare.search_width);
      NV_INT32 start_pmt_search = qMax (20, bin - misc->abe_share->filterShare.search_width);
      NV_INT32 end_apd_search = qMin (HWF_APD_SIZE - 1, bin + misc->abe_share->filterShare.search_width);
      NV_INT32 end_pmt_search = qMin (HWF_PMT_SIZE - 1, bin + misc->abe_share->filterShare.search_width);
      NV_INT32 rise_count = 0;
      NV_INT32 ndx = misc->points[i];

      if (start_apd_search < HWF_APD_SIZE - 20)
        {
          for (NV_INT32 j = start_apd_search ; j < end_apd_search ; j++)
            {
              if (wave_data[ndx].apd[j] - wave_data[ndx].apd[j - 1] > 0)
                {
                  rise_count++;
                }
              else if (wave_data[ndx].apd[j] - wave_data[ndx].apd[j - 1] < 0)
                {
                  rise_count = 0;
                }

              if (rise_count >= misc->abe_share->filterShare.rise_threshold) return (NVFalse);
            }
        }


      rise_count = 0;

      for (NV_INT32 j = start_pmt_search ; j < end_pmt_search ; j++)
        {
          if (wave_data[ndx].pmt[j] - wave_data[ndx].pmt[j - 1] > 0)
            {
              rise_count++;
            }
          else if (wave_data[ndx].pmt[j] - wave_data[ndx].pmt[j - 1] < 0)
            {
              rise_count = 0;
            }

          if (rise_count >= misc->abe_share->filterShare.rise_threshold) return (NVFalse);
        }
    }


  return (NVTrue);
}
