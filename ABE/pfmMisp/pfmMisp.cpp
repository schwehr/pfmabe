
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
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "pfmMisp.hpp"
#include "pfmMispHelp.hpp"


NV_FLOAT64 settings_version = 1.0;


QListWidget      *checkList;
RUN_PROGRESS     *surfProg;


static void misp_progress_callback (NV_CHAR *info)
{
  if (strlen (info) >= 2)
    {
      QListWidgetItem *cur = new QListWidgetItem (QString (info));
      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);
    }

  qApp->processEvents ();
}



pfmMisp::pfmMisp (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmMispWatermark.png"));


  //  Get the user's defaults if available

  envin (&options);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, this, &options));

  setPage (1, new surfacePage (this, &options));

  setPage (2, new runPage (this, &progress, &checkList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start generating the surface"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}


pfmMisp::~pfmMisp ()
{
}



void pfmMisp::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      button (QWizard::CustomButton1)->setEnabled (TRUE);

      pfm_file_name = field ("pfm_file_edit").toString ();

      options.nibble = field ("nibble").toInt ();
      options.clear_land = field ("clearLand").toBool ();
      options.replace_all = field ("replaceAll").toBool ();
      options.weight = field ("factor").toInt ();
      options.force_original_value = field ("force").toBool ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      //  Save the options.

      envout (&options);


      QString string;

      checkList->clear ();

      string = tr ("PFM file : ") + pfm_file_name;
      checkList->addItem (string);

      switch (options.surface)
        {
        case 0:
          string = tr ("Grid Minimum Filtered Surface");
          checkList->addItem (string);
          break;

        case 1:
          string = tr ("Grid Maximum Filtered Surface");
          checkList->addItem (string);
          break;

        case 2:
          string = tr ("Grid all depth values");
          checkList->addItem (string);
          break;
        }


      switch (options.replace_all)
        {
        case NVFalse:
          string = tr ("Replace only empty bins with interpolated value");
          checkList->addItem (string);
          break;

        case NVTrue:
          string = tr ("Replace all bins with interpolated value");
          checkList->addItem (string);
          break;
        }

      switch (options.clear_land)
        {
        case NVFalse:
          string = tr ("Don't clear SRTM masked land");
          checkList->addItem (string);
          break;

        case NVTrue:
          string = tr ("Clear SRTM masked land");
          checkList->addItem (string);
          break;
        }


      switch (options.force_original_value)
        {
        case NVFalse:
          string = tr ("Don't force bins to original input value");
          checkList->addItem (string);
          break;

        case NVTrue:
          string = tr ("Force bins to original input value");
          checkList->addItem (string);
          break;
        }

      string = QString (tr ("MISP weight factor : %1")).arg (options.weight);
      checkList->addItem (string);


      if (options.nibble >= 0)
        {
          string = QString (tr ("Nibbler value (bins) : %1")).arg (options.nibble);
          checkList->addItem (string);
        }

      break;
    }
}



void pfmMisp::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;
    }
}



void pfmMisp::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
pfmMisp::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  NV_INT32            recnum, pfm_handle, out_count = 0, dn, up, bw, fw;
  NV_FLOAT32          *array;
  NV_FLOAT64          lat, lon;
  NV_F64_XYMBR        mbr;
  NV_F64_COORD3       xyz;
  BIN_RECORD          bin;
  DEPTH_RECORD        *depth;
  NV_I32_COORD2       coord;
  PFM_OPEN_ARGS       open_args;
  NV_BOOL             found = NVFalse, land_mask_flag = NVFalse;
  NV_U_INT32          **val_array = NULL;


  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (FALSE);
  button (QWizard::BackButton)->setEnabled (FALSE);
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  strcpy (open_args.list_path, pfm_file_name.toAscii ());

  open_args.checkpoint = 0;
  pfm_handle = open_existing_pfm_file (&open_args);

  if (pfm_handle < 0) pfm_error_exit (pfm_error);


  //  Check for the land mask flag in PFM_USER_05 in any of the PFM layers.

  land_mask_flag = NVFalse;
  if (!strcmp (open_args.head.user_flag_name[4], "Land masked cell")) land_mask_flag = NVTrue;


  /*  We're going to let MISP handle everything in zero based units of the bin size.  That is, we subtract off the
      west lon from longitudes then divide by the grid size in the X direction.  We do the same with the latitude using
      the south latitude.  This will give us values that range from 0.0 to gridcols in longitude and 0.0 to gridrows
      in latitude.  The assumption here is that the bins are essentially squares (spatially).  */

  mbr.min_x = 0.0;
  mbr.min_y = 0.0;
  if (open_args.head.proj_data.projection)
    {
      mbr.max_x = (NV_FLOAT64) NINT ((open_args.head.mbr.max_x - open_args.head.mbr.min_x) / open_args.head.bin_size_xy);
      mbr.max_y = (NV_FLOAT64) NINT ((open_args.head.mbr.max_y - open_args.head.mbr.min_y) / open_args.head.bin_size_xy);
    }
  else
    {
      mbr.max_x = (NV_FLOAT64) NINT ((open_args.head.mbr.max_x - open_args.head.mbr.min_x) /
                                     open_args.head.x_bin_size_degrees);
      mbr.max_y = (NV_FLOAT64) NINT ((open_args.head.mbr.max_y - open_args.head.mbr.min_y) /
                                     open_args.head.y_bin_size_degrees);
    }


  //  Register the progress callback with MISP.

  misp_register_progress_callback (misp_progress_callback);

  misp_init (1.0, 1.0, 0.05, 4, 20.0, 20, 999999.0, -999999.0, options.weight, mbr);


  progress.gbox->setTitle (tr ("Reading data for surface"));

  progress.gbar->setRange (0, open_args.head.bin_height);


  for (NV_INT32 i = 0 ; i < open_args.head.bin_height ; i++)
    {
      coord.y = i;

      for (NV_INT32 j = 0 ; j < open_args.head.bin_width ; j++)
        {
          coord.x = j;

          read_bin_record_index (pfm_handle, coord, &bin);

          if (bin.num_soundings)
            {
              if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                {
                  found = NVFalse;
                  for (NV_INT32 k = 0 ; k < recnum ; k++)
                    {
                      switch (options.surface)
                        {
                        case 2:
                          if (!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                            {
                              out_count++;
    
                              if (open_args.head.proj_data.projection)
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.bin_size_xy;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.bin_size_xy;
                                }
                              else
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees;
                                }

                              xyz.z = depth[k].xyz.z;

                              misp_load (xyz);
                            }
                          break;
    
    
                        case 0:
                          if ((!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) && 
                              fabs (depth[k].xyz.z - bin.min_filtered_depth) < EPS)
                            {
                              out_count++;

                              if (open_args.head.proj_data.projection)
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.bin_size_xy;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.bin_size_xy;
                                }
                              else
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees;
                                }
    
                              xyz.z = depth[k].xyz.z;

                              misp_load (xyz);

                              found = NVTrue;
                            }
                          break;
    
    
                        case 1:
                          if ((!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) &&
                              fabs (depth[k].xyz.z - bin.max_filtered_depth) < EPS)
                            {
                              out_count++;
    
                              if (open_args.head.proj_data.projection)
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.bin_size_xy;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.bin_size_xy;
                                }
                              else
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees;
                                }
    
                              xyz.z = depth[k].xyz.z;

                              misp_load (xyz);

                              found = NVTrue;
                            }
                          break;
                        }
                      if (found) break;
                    }
                  free (depth);
                }
            }
        }


      progress.gbar->setValue (i);

      qApp->processEvents ();
    }
  progress.gbar->reset ();


  progress.gbox->setTitle (tr ("Generating grid surface"));


  //  Setting range to 0, 0 makes the bar just show movement.

  progress.gbar->setRange (0, 0);

  qApp->processEvents ();


  if (misp_proc ()) exit (-1);


  if (options.replace_all)
    {
      switch (options.surface)
        {
        case 2:
          strcpy (open_args.head.average_filt_name, "AVERAGE MISP SURFACE");
          write_bin_header (pfm_handle, &open_args.head, 0);
          break;

        case 0:
          strcpy (open_args.head.average_filt_name, "MINIMUM MISP SURFACE");
          write_bin_header (pfm_handle, &open_args.head, 0);
          break;

        case 1:
          strcpy (open_args.head.average_filt_name, "MAXIMUM MISP SURFACE");
          write_bin_header (pfm_handle, &open_args.head, 0);
          break;
        }
    }


  progress.gbar->setRange (0, open_args.head.bin_height);

  progress.gbox->setTitle (tr ("Retrieving gridded surface data"));


  /*  Allocating one more column than we need due to chrtr specific changes in misp_rtrv (see misp_funcs.c).  */

  array = (NV_FLOAT32 *) malloc ((open_args.head.bin_width + 1) * sizeof (NV_FLOAT32));

  if (array == NULL)
    {
      perror ("Allocating array");
      exit (-1);
    }


  for (NV_INT32 i = 0 ; i < open_args.head.bin_height ; i++)
    {
      //  Compute the latitude of the bin.

      NV_F64_COORD2 xy;
      xy.y = open_args.head.mbr.min_y + i * open_args.head.y_bin_size_degrees;


      coord.y = i;

      if (!misp_rtrv (array)) break;

      for (NV_INT32 j = 0 ; j < open_args.head.bin_width ; j++)
        {
          //  Compute the longitude of the bin.

          xy.x = open_args.head.mbr.min_x + j * open_args.head.x_bin_size_degrees;


          //  Don't try to write points that fall outside of the PFM polygon (it might not be a rectangle).

          if (bin_inside_ptr (&open_args.head, xy))
            {
              coord.x = j;


              read_bin_record_index (pfm_handle, coord, &bin);


              //  This is a special case.  We don't want to replace land masked bins unless the land mask point has been
              //  deleted.

              if (!land_mask_flag || !(bin.validity & PFM_DATA) || !(bin.validity & PFM_USER_05))
                {
                  if (options.replace_all || !(bin.validity & PFM_DATA)) 
                    {
                      bin.validity |= PFM_INTERPOLATED;
                      if (array[j] <= open_args.max_depth && array[j] > -open_args.offset)
                        {
                          bin.avg_filtered_depth = array[j];
                        }
                      else
                        {
                          bin.avg_filtered_depth = open_args.head.null_depth;
                        }


                      //  If there was a land mask point in this bin and it has been deleted, unset the 
                      //  PFM_USER_05 flag.

                      if (bin.validity & PFM_USER_05) bin.validity &= ~PFM_USER_05;


                      //  If we set the nibble argument to 0 we don't want to put interpolated values into
                      //  empty bins.

                      if ((bin.validity & PFM_DATA) || options.nibble) write_bin_record_index (pfm_handle, &bin);
                    }
                }
            }
        }

      progress.gbar->setValue (i);

      qApp->processEvents ();
    }

  free (array);

  progress.gbar->setValue (open_args.head.bin_height);


  qApp->processEvents ();


  /*  Nibble out the cells that aren't within our optional nibbling distance from a cell with valid data.  */

  if (options.nibble > 0)
    {
      /*  This can get into a fair amount of memory but it is the most
          efficient way to do this since we're doing, basically, sequential
          I/O on both ends.  If you have a 2000 by 2000 cell bin this will
          allocate 16MB of memory.  Hopefully your machine can handle that.
          If not, buy a new machine.  */

      val_array = (NV_U_INT32 **) calloc (open_args.head.bin_height, sizeof (NV_U_INT32 *));

      if (val_array == NULL)
        {
          perror ("Allocating val_array");
          exit (-1);
        }



      for (NV_INT32 i = 0 ; i < open_args.head.bin_height ; i++)
        {
          val_array[i] = (NV_U_INT32 *) calloc (open_args.head.bin_width, sizeof (NV_U_INT32));

          if (val_array[i] == NULL)
            {
              perror ("Allocating memory for nibbler");
              exit (-1);
            }
        }


      progress.gbar->reset ();

      progress.gbox->setTitle (tr ("Clearing interpolated data (reading validity)"));

      progress.gbar->setRange (0, open_args.head.bin_height);


      for (NV_INT32 i = 0 ; i < open_args.head.bin_height ; i++)
        {
          coord.y = i;

          for (NV_INT32 j = 0 ; j < open_args.head.bin_width ; j++)
            {
              coord.x = j;

              read_bin_record_validity_index (pfm_handle, coord, &val_array[i][j]);
            }

          progress.gbar->setValue (i);

          qApp->processEvents ();
        }

      progress.gbar->setValue (open_args.head.bin_height);

      qApp->processEvents ();


      progress.gbar->reset ();

      progress.gbox->setTitle (tr ("Clearing interpolated data (nibbling)"));

      progress.gbar->setRange (0, open_args.head.bin_height);


      for (NV_INT32 i = 0 ; i < open_args.head.bin_height ; i++)
        {
          coord.y = i;

          dn = MAX (i - options.nibble, 0);
          up = MIN (i + options.nibble, open_args.head.bin_height - 1);

          for (NV_INT32 j = 0 ; j < open_args.head.bin_width ; j++)
            {
              coord.x = j;

              if (!(val_array[i][j] & PFM_DATA))
                {
                  bw = MAX (j - options.nibble, 0);
                  fw = MIN (j + options.nibble, open_args.head.bin_width - 1);

                  found = NVFalse;
                  for (NV_INT32 k = dn ; k <= up ; k++)
                    {
                      for (NV_INT32 m = bw ; m <= fw ; m++)
                        {
                          if (val_array[k][m] & PFM_DATA)
                            {
                              found = NVTrue;
                              break;
                            }
                        }
                      if (found) break;
                    }

                  if (!found)
                    {
                      bin.coord = coord;
                      bin.validity = val_array[i][j] & (~PFM_INTERPOLATED);
                      write_bin_record_validity_index (pfm_handle, &bin, PFM_INTERPOLATED);
                    }
                }
            }

          progress.gbar->setValue (i);

          qApp->processEvents ();
        }

      progress.gbar->setValue (open_args.head.bin_height);

      qApp->processEvents ();
    }


  /*  Clear landmasked data if requested.  */

  if (options.clear_land)
    {
      progress.gbar->reset ();

      progress.gbox->setTitle (tr ("Clearing SRTM land data"));

      progress.gbar->setRange (0, open_args.head.bin_height);


      for (NV_INT32 i = 0 ; i < open_args.head.bin_height ; i++)
        {
          coord.y = i;

          lat = open_args.head.mbr.min_y + ((NV_FLOAT64) i + 0.5) * open_args.head.y_bin_size_degrees;

          for (NV_INT32 j = 0 ; j < open_args.head.bin_width ; j++)
            {
              coord.x = j;

              lon = open_args.head.mbr.min_x + ((NV_FLOAT64) j + 0.5) * open_args.head.x_bin_size_degrees;

              read_bin_record_index (pfm_handle, coord, &bin);


              /*  If there's real data in the cell don't believe the mask.  */

              if (!(bin.num_soundings))
                {
                  if (read_srtm_mask (lat, lon) == 1)
                    {
                      bin.validity &= (~PFM_INTERPOLATED);
                      write_bin_record_index (pfm_handle, &bin);
                    }
                }
            }

          progress.gbar->setValue (i);

          qApp->processEvents ();
        }

      progress.gbar->setValue (open_args.head.bin_height);

      qApp->processEvents ();
    }

  close_pfm_file (pfm_handle);


  button (QWizard::FinishButton)->setEnabled (TRUE);
  button (QWizard::CancelButton)->setEnabled (FALSE);


  QApplication::restoreOverrideCursor ();
  qApp->processEvents ();


  checkList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Gridding complete, press Finish to exit."));

  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);
}



//  Get the users defaults.

void pfmMisp::envin (OPTIONS *options)
{
  NV_FLOAT64 saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  options->clear_land = NVFalse;
  options->replace_all = NVFalse;
  options->force_original_value = NVFalse;
  options->surface = 2;
  options->nibble = -1;
  options->weight = 2;
  options->input_dir = ".";
  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 800;
  options->window_height = 400;


  QSettings settings (tr ("navo.navy.mil"), tr ("pfmMisp"));

  settings.beginGroup (tr ("pfmMisp"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->clear_land = settings.value (tr ("clear land"), options->clear_land).toBool ();

  options->replace_all = settings.value (tr ("replace all"), options->replace_all).toBool ();

  options->force_original_value = settings.value (tr ("force original value"), options->force_original_value).toBool ();

  options->surface = settings.value (tr ("surface"), options->surface).toInt ();

  options->nibble = settings.value (tr ("nibble value"), options->nibble).toInt ();

  options->weight = settings.value (tr ("weight"), options->weight).toInt ();

  options->input_dir = settings.value (tr ("input directory"), options->input_dir).toString ();

  options->window_width = settings.value (tr ("width"), options->window_width).toInt ();
  options->window_height = settings.value (tr ("height"), options->window_height).toInt ();
  options->window_x = settings.value (tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (tr ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void pfmMisp::envout (OPTIONS *options)
{
  QSettings settings (tr ("navo.navy.mil"), tr ("pfmMisp"));

  settings.beginGroup (tr ("pfmMisp"));

  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("clear land"), options->clear_land);

  settings.setValue (tr ("replace all"), options->replace_all);

  settings.setValue (tr ("force original value"), options->force_original_value);

  settings.setValue (tr ("surface"), options->surface);

  settings.setValue (tr ("nibble value"), options->nibble);

  settings.setValue (tr ("weight"), options->weight);

  settings.setValue (tr ("input directory"), options->input_dir);

  settings.setValue (tr ("width"), options->window_width);
  settings.setValue (tr ("height"), options->window_height);
  settings.setValue (tr ("x position"), options->window_x);
  settings.setValue (tr ("y position"), options->window_y);

  settings.endGroup ();
}
