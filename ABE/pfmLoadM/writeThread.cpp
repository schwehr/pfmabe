#include "writeThread.hpp"

writeThread::writeThread (QObject *parent)
  : QThread(parent)
{
}



writeThread::~writeThread ()
{
}



void writeThread::write (NV_INT32 a_pass, PFM_LOAD_PARAMETERS *a_load_parms, PFM_DEFINITION *a_pfm_def, QMutex *a_pfmMutex,
                         QSemaphore **a_freeBuffers, QSemaphore **a_usedBuffers, TRANSFER *a_transfer, FILE *a_errfp,
                         NV_BOOL *a_read_done, THREAD_DATA *a_thd)
{
  QMutexLocker locker (&mutex);


  l_pass = a_pass;
  l_load_parms = a_load_parms;
  l_pfm_def = a_pfm_def;
  l_pfmMutex = a_pfmMutex;
  l_freeBuffers = a_freeBuffers;
  l_usedBuffers = a_usedBuffers;
  l_transfer = a_transfer;
  l_errfp = a_errfp;
  l_read_done = a_read_done;
  l_thd = a_thd;


  if (!isRunning ()) start ();
}



void writeThread::run ()
{
  mutex.lock ();


  pass = l_pass;
  load_parms = l_load_parms;
  pfm_def = l_pfm_def;
  pfmMutex = l_pfmMutex;
  freeBuffers = l_freeBuffers;
  usedBuffers = l_usedBuffers;
  transfer = l_transfer;
  errfp = l_errfp;
  read_done = l_read_done;
  thd = l_thd;


  mutex.unlock ();


  for (NV_INT32 i = 0 ; i < MAX_PFM_FILES ; i++)
    {
      index[i] = 0;
      out_count[i] = 0;
      out_of_limits[i] = 0;
    }


  //  Hardwired for now (we only want to do one output PFM at the moment).

  NV_INT32 pfm_file_count = 1;


  NV_INT32 pos = 0;
  NV_INT32 status = 0;
  NV_INT32 done = 0;
      
  do
    {
      usedBuffers[pass]->acquire ();


      if (transfer[pass].pos < 0) break;


      NV_INT32 index = pos % BUFFER_SIZE;

      DEPTH_RECORD depth_record = transfer[pass].rec[index];

      pos = index + 1;

      freeBuffers[pass]->release ();


      NV_F64_COORD2 nxy;

      nxy.x = depth_record.xyz.x;
      nxy.y = depth_record.xyz.y;


      //  Loop over PFM files we have to fill. 

      for (NV_INT32 j = 0 ; j < pfm_file_count ; j++) 
        {
          //if (bin_inside_ptr (&pfm_def[j].open_args.head, nxy))
          if (inside_polygon (pfm_def[j].open_args.head.polygon, pfm_def[j].open_args.head.polygon_count, nxy.x, nxy.y))
            {
              //compute_index_ptr (nxy, &depth_record.coord, &pfm_def[j].open_args.head);
              if (pfm_def[j].open_args.head.proj_data.projection)
                {
                  depth_record.coord.x = (nxy.x - pfm_def[j].open_args.head.mbr.min_x) / pfm_def[j].open_args.head.bin_size_xy;
                  depth_record.coord.y = (nxy.y - pfm_def[j].open_args.head.mbr.min_y) / pfm_def[j].open_args.head.bin_size_xy;
                }
              else
                {
                  depth_record.coord.x = (NV_INT32) ((NV_FLOAT64) (nxy.x - pfm_def[j].open_args.head.mbr.min_x) /
                                                     (NV_FLOAT64) pfm_def[j].open_args.head.x_bin_size_degrees);
                  depth_record.coord.y = (NV_INT32) ((NV_FLOAT64) (nxy.y - pfm_def[j].open_args.head.mbr.min_y) /
                                                     (NV_FLOAT64) pfm_def[j].open_args.head.y_bin_size_degrees);
                }


              //  If the depth is outside of the min and max depths, set it to the null depth (defined in the PFM API as
              //  max_depth + 1.0) and set the PFM_FILTER_INVAL and the PFM_MODIFIED bit that is used by the PFM API to signify that 
              //  the data must be saved to the input file after editing.  Note that these points will not be visible in the PFM editor.

              if (depth_record.xyz.z > pfm_def[j].max_depth || depth_record.xyz.z <= pfm_def[j].min_depth)
                {
                  depth_record.xyz.z = pfm_def[j].max_depth + 1.0;


                  //  Caveat - If we're loading HOF LIDAR null data and this is a null point we don't want to set this to invalid
                  //  or modified so that GCS can do reprocessing.

                  if (!depth_record.local_flags) depth_record.validity |= (PFM_FILTER_INVAL | PFM_MODIFIED);

                  out_of_limits[j]++;
                }


              //  Turn off selected soundings in input data.

              depth_record.validity &= ~PFM_SELECTED_SOUNDING;


              //  Load the point.

              //pfmMutex[j].lock ();

              if ((status = add_cached_depth_record (thd[j].hnd[pass], &depth_record)))
                //if ((status = add_depth_record_index (thd[j].hnd[pass], &depth_record)))
                {
                  if (status == WRITE_BIN_RECORD_DATA_READ_ERROR)
                    {
                      fprintf (stderr, "%s\n", pfm_error_str (status));
                      fprintf (stderr, "%d %d %f %f %f %f\n\n", depth_record.coord.y, depth_record.coord.x,
                               pfm_def[j].open_args.head.mbr.min_y, pfm_def[j].open_args.head.mbr.max_y,
                               pfm_def[j].open_args.head.mbr.min_x, pfm_def[j].open_args.head.mbr.max_x);
                      fflush (stderr);
                    }
                  else
                    {
                      pfm_error_exit (status);
                    }
                }

              //pfmMutex[j].unlock ();


              //  Set the add_map value to show that we have new data in this bin.

              pfm_def[j].add_map[depth_record.coord.y * pfm_def[j].open_args.head.bin_width + depth_record.coord.x] = 1;


              out_count[j]++;
            }
        }


      done = 0;
      for (NV_INT32 i = 0 ; i < PRODUCERS ; i++)
        {
          if (read_done[i]) done++;
        }


      qApp->processEvents ();


    } while (done < PRODUCERS || usedBuffers[pass]->available ());


  emit complete (out_count[0], out_of_limits[0], pass);
  qApp->processEvents ();
}
