
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



#include "pfmLoadM.hpp"
#include "pfmLoadMHelp.hpp"
#include "version.hpp"


QSemaphore *freeBuffers[CONSUMERS], *usedBuffers[CONSUMERS];


static QProgressBar *prog;


static void initProgress (int state __attribute__ ((unused)), int percent)
{
  static int old_percent = -1;


  //  We don't want to eat up all of the free time on the system with a progress bar.

  if (percent - old_percent >= 20 || old_percent > percent)
    {
      prog->setValue (percent);
      old_percent = percent;
    }

  qApp->processEvents ();
}


#ifdef NVLinux


static NV_INT32 substitute_cnt;
static NV_CHAR substitute_path[10][3][1024];


static void sub_in ()
{
  char        varin[1024], info[1024];
  FILE        *fp;


  substitute_cnt = 0;


  //  If the startup file was found...
    
  if ((fp = find_startup (".pfm_cfg")) != NULL)
    {
      //  Read each entry.
        
      while (ngets (varin, sizeof (varin), fp) != NULL)
        {
	  if (varin[0] != '#')
	    {
	      //  Check input for matching strings and load values if found.
            
	      if (strstr (varin, "[SUBSTITUTE PATH]") != NULL && substitute_cnt < 10)
		{
		  //  Put everything to the right of the equals sign in 'info'.
            
		  get_string (varin, info);


		  /*  Throw out malformed substitute paths.  */

		  if (strchr (info, ','))
		    {
		      /*  Check for more than 1 UNIX substitute path.  */

		      if (strchr (info, ',') == strrchr (info, ','))
			{
			  strcpy (substitute_path[substitute_cnt][0], strtok (info, ","));
			  strcpy (substitute_path[substitute_cnt][1], strtok (NULL, ","));
			  substitute_path[substitute_cnt][2][0] = 0;
			}
		      else
			{
			  strcpy (substitute_path[substitute_cnt][0], strtok (info, ","));
			  strcpy (substitute_path[substitute_cnt][1], strtok (NULL, ","));
			  strcpy (substitute_path[substitute_cnt][2], strtok (NULL, ","));
			}

		      substitute_cnt++;
		    }
		}
	    }
	}

      fclose (fp);
    }
}


static void inv_sub (NV_CHAR *path)
{
  NV_CHAR        string[1024];
  NV_INT16       i;
  NV_BOOL        hit;


  for (i = 0 ; i < substitute_cnt ; i++)
    {
      //  Set the hit flag so we can break out if we do the substitution.

      hit = NVFalse;


      for (NV_INT32 k = 1 ; k < 3 ; k++)
	{
	  //  Make sure that we had two UNIX substitutes, otherwise it will substitute for a blank string
	  //  (this only works when k is 2).

	  if (substitute_path[i][k][0] && strstr (path, substitute_path[i][k]))
	    {
	      strcpy (string, substitute_path[i][0]);

	      NV_INT32 j = strlen (substitute_path[i][k]);
	      strcat (string, &path[j]);
	      strcpy (path, string);

	      for (NV_INT32 j = 0 ; j < (NV_INT32) strlen (path) ; j++)
		{
		  if (path[j] == '/') path[j] = '\\';
		}

	      hit = NVTrue;

	      break;
	    }
	}


      //  Break out if we did a substitution.

      if (hit) break;
    }
}

#endif



pfmLoadM::pfmLoadM (QWidget *parent, QString parm_file)
  : QWizard (parent)
{
  void setAllAttributes (PFM_GLOBAL *global);
  void countAllAttributes (PFM_GLOBAL *global);


  parameter_file = parm_file;
  upr_file = NVFalse;
  pfm_global.input_dirs.clear ();
  total_out_count = 0;
  total_out_of_limits = 0;


  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmLoadMWatermark.png"));


  //  Set the window title (for now).

  setWindowTitle (VERSION);


  /*  Override the HDF5 version check so that we can read BAGs created with an older version of HDF5.  */

  putenv ((NV_CHAR *) "HDF5_DISABLE_VERSION_CHECK=2");


  global_dialog.gsfD = global_dialog.hofD = global_dialog.tofD = global_dialog.wlfD = global_dialog.czmilD = global_dialog.bagD = global_dialog.hawkeyeD = global_dialog.cubeD = NULL;


  //  Dangerous things should always default to the least dangerous mode so we don't save and restore
  //  these through envin/envout

  pfm_global.max_files = NINT (pow (2.0, (NV_FLOAT64) FILE_BITS)) - 1;
  pfm_global.max_lines = NINT (pow (2.0, (NV_FLOAT64) LINE_BITS)) - 1;
  pfm_global.max_pings = NINT (pow (2.0, (NV_FLOAT64) PING_BITS)) - 1;
  pfm_global.max_beams = NINT (pow (2.0, (NV_FLOAT64) BEAM_BITS)) - 1;
  flags.ref = NVFalse;


  //  Set the timezone to GMT, we'll use this later.

#ifdef NVWIN3X
 #ifdef __MINGW64__
  putenv((NV_CHAR *) "TZ=GMT");
  tzset();
 #else
  _putenv((NV_CHAR *) "TZ=GMT");
  _tzset();
#endif

  strcpy (cube_name, "navo_pfm_cube.exe");
#else
  putenv((NV_CHAR *) "TZ=GMT");
  tzset();

  strcpy (cube_name, "navo_pfm_cube");
#endif


  cube_available = NVFalse;
  if (find_startup_name (cube_name) != NULL) cube_available = NVTrue;


  setAllAttributes (&pfm_global);


  envin ();


  countAllAttributes (&pfm_global);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (window_width, window_height);
  this->move (window_x, window_y);


  setPage (0, new startPage (this, parameter_file));

  for (NV_INT32 i = 0 ; i < MAX_LOAD_FILES ; i++)
    {
      pfm_def[i].name = "";
      pfm_def[i].area = "";
      pfm_def[i].polygon_count = 0;
      pfm_def[i].index = i;
      pfm_def[i].mbin_size = ref_def.mbin_size;
      pfm_def[i].gbin_size = ref_def.gbin_size;
      pfm_def[i].min_depth = ref_def.min_depth;
      pfm_def[i].max_depth = ref_def.max_depth;
      pfm_def[i].precision = ref_def.precision;
      pfm_def[i].mosaic = "NONE";
      pfm_def[i].feature = "NONE";
      pfm_def[i].apply_area_filter = ref_def.apply_area_filter;
      pfm_def[i].deep_filter_only = ref_def.deep_filter_only;
      pfm_def[i].cellstd = ref_def.cellstd;
      pfm_def[i].radius = ref_def.radius;
      pfm_def[i].existing = NVFalse;
      memset (&pfm_def[i].open_args, 0, sizeof (PFM_OPEN_ARGS));


      NV_INT32 page_num = i + 1;

      setPage (page_num, new pfmPage (this, &pfm_def[i], &pfm_global, page_num));
    }

  setPage (MAX_LOAD_FILES + 2, globalPg = new globalPage (this, &pfm_global, &flags, &global_dialog, cube_available));
  setPage (MAX_LOAD_FILES + 3, new inputPage (this, &inputFilter, &inputFiles, &pfm_global));
  setPage (MAX_LOAD_FILES + 4, new runPage (this, &progress, &checkList));


  prog = progress.fbar;


  setButtonText (QWizard::CustomButton1, tr("&Load"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start the PFM load process"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  setButtonText (QWizard::CustomButton2, tr("&Save"));
  setOption (QWizard::HaveCustomButton2, TRUE);
  button (QWizard::CustomButton2)->setToolTip (tr ("Save the input parameters to a parameter (.prm or .upr)"));
  button (QWizard::CustomButton2)->setWhatsThis (saveText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



pfmLoadM::~pfmLoadM ()
{
}



void pfmLoadM::initializePage (int id)
{
  NV_BOOL readParameterFile (QString parameter_file, QStringList *input_files, PFM_DEFINITION *pfm_def, 
                             PFM_GLOBAL *pfm_global, FLAGS *flags);

  QString regField, pfmIndex;


  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);
  button (QWizard::CustomButton2)->setEnabled (FALSE);


  //  Start page

  if (!id)
    {
      
    }


  //  PFM pages.

  else if (id >= 1 && id < MAX_LOAD_FILES + 2)
    {
      //  Check for an input parameter file.

      if (id == 1 && !parameter_file.isEmpty ())
        {
          input_files.clear ();

          readParameterFile (parameter_file, &input_files, pfm_def, &pfm_global, &flags);

          for (NV_INT32 i = 0 ; i < input_files.size () ; i++) inputFiles->append (input_files.at (i));
        }


      if (!hasVisitedPage (id))
        {
          NV_INT32 pid = id - 1;
          NV_INT32 prev_id = pid - 1;

          if (pid)
            {
              pfmIndex.sprintf ("%02d", prev_id);
              
              regField = "mBinSize" + pfmIndex;
              pfm_def[prev_id].mbin_size = field (regField).toDouble ();

              regField = "gBinSize" + pfmIndex;
              pfm_def[prev_id].gbin_size = field (regField).toDouble ();

              regField = "minDepth" + pfmIndex;
              pfm_def[prev_id].min_depth = field (regField).toDouble ();

              regField = "maxDepth" + pfmIndex;
              pfm_def[prev_id].max_depth = field (regField).toDouble ();

              regField = "precision" + pfmIndex;
              NV_INT32 j = field (regField).toInt ();

              switch (j)
                {
                case 0:
                  pfm_def[prev_id].precision = 0.01;
                  break;

                case 1:
                  pfm_def[prev_id].precision = 0.10;
                  break;

                case 2:
                  pfm_def[prev_id].precision = 1.00;
                  break;
                }

              regField = "mosaic_edit" + pfmIndex;
              pfm_def[prev_id].mosaic = field (regField).toString ();

              regField = "feature_edit" + pfmIndex;
              pfm_def[prev_id].feature = field (regField).toString ();

              regField = "applyFilter" + pfmIndex;
              pfm_def[prev_id].apply_area_filter = field (regField).toBool ();

              regField = "deepFilter" + pfmIndex;
              pfm_def[prev_id].deep_filter_only = field (regField).toBool ();

              regField = "stdSpin" + pfmIndex;
              pfm_def[prev_id].cellstd = field (regField).toDouble ();

              regField = "featureRadius" + pfmIndex;
              pfm_def[prev_id].radius = field (regField).toDouble ();
            }


          //  Populate the next definition with whatever you put in the previous one unless it's already been 
          //  done (i.e. it has a PFM structure name).

          if (pid && pfm_def[pid].name.isEmpty ()) 
            {
              pfm_def[pid].max_depth = pfm_def[prev_id].max_depth;
              pfm_def[pid].min_depth = pfm_def[prev_id].min_depth;
              pfm_def[pid].precision = pfm_def[prev_id].precision;
              pfm_def[pid].mbin_size = pfm_def[prev_id].mbin_size;
              pfm_def[pid].gbin_size = pfm_def[prev_id].gbin_size;
              pfm_def[pid].apply_area_filter = pfm_def[prev_id].apply_area_filter;
              pfm_def[pid].deep_filter_only = pfm_def[prev_id].deep_filter_only;
              pfm_def[pid].cellstd = pfm_def[prev_id].cellstd;
              pfm_def[pid].radius = pfm_def[prev_id].radius;


              //  Save the latest definitions for writing to the settings file.

              ref_def.mbin_size = pfm_def[pid].mbin_size;
              ref_def.gbin_size = pfm_def[pid].gbin_size;
              ref_def.min_depth = pfm_def[pid].min_depth;
              ref_def.max_depth = pfm_def[pid].max_depth;
              ref_def.precision = pfm_def[pid].precision;
              ref_def.apply_area_filter = pfm_def[pid].apply_area_filter;
              ref_def.deep_filter_only = pfm_def[pid].deep_filter_only;
              ref_def.cellstd = pfm_def[pid].cellstd;
              ref_def.radius = pfm_def[pid].radius;
            }

          pfmPage *nextPage = (pfmPage *) page (id);
          nextPage->setFields (&pfm_def[pid]);
        }

      if (id > 1 && pfm_def[id].name.isEmpty () && pfm_def[id].area.isEmpty ())
        {
          page (id)->setButtonText (QWizard::NextButton, tr ("Finish"));
        }
      else
        {
          page (id)->setButtonText (QWizard::NextButton, tr ("Next"));
        }
    }


  //  Global page

  else if (id == MAX_LOAD_FILES + 2)
    {
      //  Get the file count

      pfm_file_count = 0;
      for (NV_INT32 i = 0 ; i < MAX_LOAD_FILES ; i++) 
        {
          if (pfm_def[i].name == "")
            {
              pfm_file_count = i;
              break;
            }
        }


      //  Check for pre-existing index file attributes.

      for (NV_INT32 i = 0 ; i < pfm_file_count ; i++)
        {
          if (pfm_def[i].existing)
            {
              pfm_global.appending = NVTrue;
              pfm_global.attribute_count = 0;

              NV_INT32 temp_handle;
              PFM_OPEN_ARGS open_args;

              strcpy (open_args.list_path, pfm_def[i].name.toAscii ());

              open_args.checkpoint = 0;

              if ((temp_handle = open_existing_pfm_file (&open_args)) >= 0)
                {
                  for (NV_INT32 j = 0 ; j < pfm_def[i].open_args.head.num_ndx_attr ; j++)
                    {
                      if (QString (pfm_def[i].open_args.head.ndx_attr_name[j]).contains (pfm_global.time_attribute_name))
                        {
                          pfm_global.attribute_count++;
                          pfm_global.time_attribute_num = pfm_global.attribute_count;
                        }

                      for (NV_INT32 k = 0 ; k < GSF_ATTRIBUTES ; k++)
                        {
                          if (QString (pfm_def[i].open_args.head.ndx_attr_name[j]).contains (pfm_global.gsf_attribute_name[k]))
                            {
                              pfm_global.attribute_count++;
                              pfm_global.gsf_attribute_num[k] = pfm_global.attribute_count;
                              break;
                            }
                        }

                      for (NV_INT32 k = 0 ; k < HOF_ATTRIBUTES ; k++)
                        {
                          if (QString (pfm_def[i].open_args.head.ndx_attr_name[j]).contains (pfm_global.hof_attribute_name[k]))
                            {
                              pfm_global.attribute_count++;
                              pfm_global.hof_attribute_num[k] = pfm_global.attribute_count;
                              break;
                            }
                        }

                      for (NV_INT32 k = 0 ; k < TOF_ATTRIBUTES ; k++)
                        {
                          if (QString (pfm_def[i].open_args.head.ndx_attr_name[j]).contains (pfm_global.tof_attribute_name[k]))
                            {
                              pfm_global.attribute_count++;
                              pfm_global.tof_attribute_num[k] = pfm_global.attribute_count;
                              break;
                            }
                        }

                      for (NV_INT32 k = 0 ; k < WLF_ATTRIBUTES ; k++)
                        {
                          if (QString (pfm_def[i].open_args.head.ndx_attr_name[j]).contains (pfm_global.wlf_attribute_name[k]))
                            {
                              pfm_global.attribute_count++;
                              pfm_global.wlf_attribute_num[k] = pfm_global.attribute_count;
                              break;
                            }
                        }

                      for (NV_INT32 k = 0 ; k < CZMIL_ATTRIBUTES ; k++)
                        {
                          if (QString (pfm_def[i].open_args.head.ndx_attr_name[j]).contains (pfm_global.czmil_attribute_name[k]))
                            {
                              pfm_global.attribute_count++;
                              pfm_global.czmil_attribute_num[k] = pfm_global.attribute_count;
                              break;
                            }
                        }

                      for (NV_INT32 k = 0 ; k < BAG_ATTRIBUTES ; k++)
                        {
                          if (QString (pfm_def[i].open_args.head.ndx_attr_name[j]).contains (pfm_global.bag_attribute_name[k]))
                            {
                              pfm_global.attribute_count++;
                              pfm_global.bag_attribute_num[k] = pfm_global.attribute_count;
                              break;
                            }
                        }

                      for (NV_INT32 k = 0 ; k < HAWKEYE_ATTRIBUTES ; k++)
                        {
                          if (QString (pfm_def[i].open_args.head.ndx_attr_name[j]).contains (pfm_global.hawkeye_attribute_name[k]))
                            {
                              pfm_global.attribute_count++;
                              pfm_global.hawkeye_attribute_num[k] = pfm_global.attribute_count;
                              break;
                            }
                        }
                    }
                  close_pfm_file (temp_handle);
                }
            }
        }


      if (flags.old) 
        QMessageBox::warning (this, tr ("pfmLoadM GCS Compatible Mode"),
                              tr ("Do not use GCS compatible mode unless you are going to reprocess the data in the PFM using Optech's GCS program!"));

      globalPg->setFields (&pfm_global, &flags);


      if (check_srtm2_topo () && !flags.srtme && (flags.srtmb || flags.srtm1)) 
        QMessageBox::information (this, tr ("pfmLoadM SRTM2 data"),
                                  tr ("WARNING WARNING WARNING WARNING WARNING WARNING WARNING\n\n") +
                                  tr ("If your PFM file includes any areas outside of the United States it will be limited ") +
                                  tr ("distribution due to the inclusion of NGA SRTM2 topography data.  If you do not want to ") +
                                  tr ("include the SRTM2 data please select 'Exclude SRTM2 data' from the 'Optional flag ") +
                                  tr ("settings' on the next (Global Options) page.\n\n") +
                                  tr ("WARNING WARNING WARNING WARNING WARNING WARNING WARNING"));
    }


  //  Input page.

  else if (id == MAX_LOAD_FILES + 3)
    {
      //  Shut down any of the global page dialogs if they were left open.

      if (global_dialog.gsfD) global_dialog.gsfD->close ();
      if (global_dialog.hofD) global_dialog.hofD->close ();
      if (global_dialog.tofD) global_dialog.tofD->close ();
      if (global_dialog.wlfD) global_dialog.wlfD->close ();
      if (global_dialog.czmilD) global_dialog.czmilD->close ();
      if (global_dialog.bagD) global_dialog.bagD->close ();
      if (global_dialog.hawkeyeD) global_dialog.hawkeyeD->close ();
      if (global_dialog.cubeD) global_dialog.cubeD->close ();

      global_dialog.gsfD = global_dialog.hofD = global_dialog.tofD = global_dialog.wlfD = global_dialog.czmilD = global_dialog.bagD= global_dialog.hawkeyeD  = global_dialog.cubeD = NULL;
    }


  //  Run page.

  else if (id == MAX_LOAD_FILES + 4)
    {
      button (QWizard::CustomButton1)->setEnabled (TRUE);
      button (QWizard::CustomButton2)->setEnabled (TRUE);


      for (NV_INT32 i = 0 ; i < MAX_LOAD_FILES ; i++)
        {
          pfmIndex.sprintf ("%02d", i);

          regField = "pfm_file_edit" + pfmIndex;
          pfm_def[i].name = field (regField).toString ();


          //  Empty PFM name means we're done with defining PFM structures.

          if (pfm_def[i].name.isEmpty ()) break;


          //  We may have to append the .pfm

          if (!pfm_def[i].name.endsWith (".pfm")) pfm_def[i].name.append (".pfm");


          //  Save the output directory.  It might have been input manually instead of browsed.

          pfm_global.output_dir = QFileInfo (pfm_def[i].name).absoluteDir ().absolutePath ();


          regField = "area_edit" + pfmIndex;
          pfm_def[i].area = field (regField).toString ();


          //  Save the area directory.  The area file might have been created using areaCheck instead of browsed.

          pfm_global.area_dir = QFileInfo (pfm_def[i].area).absoluteDir ().absolutePath ();


          regField = "mBinSize" + pfmIndex;
          pfm_def[i].mbin_size = field (regField).toDouble ();

          regField = "gBinSize" + pfmIndex;
          pfm_def[i].gbin_size = field (regField).toDouble ();

          regField = "minDepth" + pfmIndex;
          pfm_def[i].min_depth = field (regField).toDouble ();

          regField = "maxDepth" + pfmIndex;
          pfm_def[i].max_depth = field (regField).toDouble ();

          regField = "precision" + pfmIndex;
          NV_INT32 j = field (regField).toInt ();

          switch (j)
            {
            case 0:
              pfm_def[i].precision = 0.01;
              break;

            case 1:
              pfm_def[i].precision = 0.10;
              break;

            case 2:
              pfm_def[i].precision = 1.00;
              break;
            }

          regField = "mosaic_edit" + pfmIndex;
          pfm_def[i].mosaic = field (regField).toString ();

          regField = "feature_edit" + pfmIndex;
          pfm_def[i].feature = field (regField).toString ();

          regField = "applyFilter" + pfmIndex;
          pfm_def[i].apply_area_filter = field (regField).toBool ();

          regField = "deepFilter" + pfmIndex;
          pfm_def[i].deep_filter_only = field (regField).toBool ();

          regField = "stdSpin" + pfmIndex;
          pfm_def[i].cellstd = field (regField).toDouble ();

          regField = "featureRadius" + pfmIndex;
          pfm_def[i].radius = field (regField).toDouble ();


          flags.old = field ("hofOld").toBool ();
          flags.hof = field ("hofNull").toBool ();
          flags.sub = field ("subPath").toBool ();
          flags.lnd = field ("invLand").toBool ();
          flags.sec = field ("invSec").toBool ();
          flags.ref = field ("refData").toBool ();
          flags.chk = field ("fileCheck").toBool ();
          flags.tof = field ("tofRef").toBool ();
          flags.lid = field ("hofFlag").toBool ();
          flags.nom = field ("gsfNominal").toBool ();
          flags.srtmb = field ("srtmBest").toBool ();
          flags.srtm1 = field ("srtmOne").toBool ();
          flags.srtm3 = field ("srtmThree").toBool ();
          flags.srtm30 = field ("srtmThirty").toBool ();
          flags.srtmr = field ("srtmRef").toBool ();
          flags.srtme = field ("srtm2Ex").toBool ();
          flags.cube = field ("cubeLoad").toBool ();
          flags.attr = field ("cubeAttributes").toBool ();

          pfm_global.horizontal_error = field ("horizontal").toDouble ();
          pfm_global.vertical_error = field ("vertical").toDouble ();
        }


      //  If the CUBE attributes is checked and the default H/V errors are set to 0.0, set them to a reasonable default.

      if (flags.attr)
	{
	  if (pfm_global.horizontal_error == 0.0) pfm_global.horizontal_error = 1.5;
	  if (pfm_global.vertical_error == 0.0) pfm_global.vertical_error = 0.25;
	}


      //  Save the settings.

      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      window_x = tmp.x ();
      window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      window_width = tmp.width ();
      window_height = tmp.height ();


      envout ();
    }
}



void pfmLoadM::cleanupPage (int id)
{
  if (!id)
    {
    }
  else if (id >= 1 && id <= MAX_LOAD_FILES + 1)
    {
    }
  else if (id == MAX_LOAD_FILES + 2)
    {
    }
  else if (id == MAX_LOAD_FILES + 3)
    {
    }
  else if (id == MAX_LOAD_FILES + 4)
    {
    }
}



//  This is where the actual load takes place.

void 
pfmLoadM::slotCustomButtonClicked (int id)
{
  NV_CHAR tmp_char[512], string[1024];
  NV_INT32 total_input_count = 0;
  QString tmp;
  PFM_LOAD_PARAMETERS load_parms[MAX_LOAD_FILES];
  QListWidgetItem *cur;



  NV_BOOL writeParameterFile (QString parameter_file, QStringList input_files, PFM_DEFINITION *pfm_def, 
                              PFM_GLOBAL pfm_global, FLAGS flags);
  NV_INT32 get_file_type (NV_CHAR *);
  NV_BOOL clear_features (NV_INT32, NV_I32_COORD2, NV_CHAR *, NV_FLOAT64);
  void remove_dir (QString dir);


  //  Get the files from the QTextEdit box on the inputPage.

  QTextCursor inputCursor = inputFiles->textCursor ();

  inputCursor.setPosition (0);


  QStringList sort_files;

  sort_files.clear ();


  //  First we need to look for list files (*.lst), remove them from the input file list, and then expand them into the sort file list.

  do
    {
      if (inputCursor.block ().text ().endsWith (".lst"))
        {
          NV_CHAR file[512], string[512];
          FILE *fp;

          strcpy (file, inputCursor.block ().text ().toAscii ());

          if ((fp = fopen (file, "r")) != NULL)
            {
              while (ngets (string, sizeof (string), fp) != NULL) sort_files.append (QString (string));

              fclose (fp);
            }
        }
      else
        {
          sort_files << inputCursor.block ().text ();
        }
    } while (inputCursor.movePosition (QTextCursor::NextBlock));


  //  Sort so we can remove dupes.

  sort_files.sort ();


  //  Remove dupes and place into input_files.

  QString name, prev_name = "";
  input_files.clear ();

  for (NV_INT32 i = 0 ; i < sort_files.size () ; i++)
    {
      name = sort_files.at (i);

      if (name != prev_name)
        {
          input_files.append (name);
          prev_name = name;
        }
    }


  //  If the user selected directories in inputPage we need to sort and de-dupe that list as well just in case 
  //  the user wants to save a pfmLoadMer "update" parameters file.

  if (pfm_global.input_dirs.size ())
    {
      sort_files.clear ();
      sort_files = pfm_global.input_dirs;


      //  Sort so we can remove dupes.

      sort_files.sort ();


      pfm_global.input_dirs.clear ();

      for (NV_INT32 i = 0 ; i < sort_files.size () ; i++)
        {
          name = sort_files.at (i);

          if (name != prev_name)
            {
              pfm_global.input_dirs.append (name);
              prev_name = name;
            }
        }
    }


  //  We need to get the polygon points either from the input area file, the input parameter file or the
  //  existing PFM in case we want to write the parameter file before we load the data.

  for (NV_INT32 i = 0 ; i < pfm_file_count ; i++)
    {
      if (pfm_def[i].existing)
        {
          strcpy (pfm_def[i].open_args.list_path, pfm_def[i].name.toAscii ());
          pfm_def[i].open_args.checkpoint = 0;


          //  Opening the file will load the polygon points in the header.

          if ((pfm_def[i].hnd = open_existing_pfm_file (&pfm_def[i].open_args)) < 0)
            {
              QString tmp = tr ("An error occurred while trying to open file\n") + QString (pfm_def[i].open_args.list_path) +
                tr ("\nThe error message was :\n") + QString (pfm_error_str (pfm_error));
              QMessageBox::warning (this, tr ("pfmLoadM"), tmp);

              pfm_error_exit (pfm_error);
            }

          close_pfm_file (pfm_def[i].hnd);
        }
      else
        {
          //  Load the area file unless we're appending.

          if (!pfm_def[i].area.contains (tr ("Defined in PFM structure")))
            {
              //  If we read in a parameter file with [Polygon Latitude,Longitude] points we're going
              //  to transfer that to the open_args structure, otherwise we'll read in an area file.

              if (pfm_def[i].polygon_count)
                {
                  pfm_def[i].open_args.head.mbr.min_y = pfm_def[i].open_args.head.mbr.min_x = 999.0;
                  pfm_def[i].open_args.head.mbr.max_y = pfm_def[i].open_args.head.mbr.max_x = -999.0;

                  pfm_def[i].open_args.head.polygon_count = pfm_def[i].polygon_count;
                  for (NV_INT32 j = 0 ; j < pfm_def[i].polygon_count ; j++)
                    {
                      pfm_def[i].open_args.head.polygon[j].y = pfm_def[i].polygon[j].y;
                      pfm_def[i].open_args.head.polygon[j].x = pfm_def[i].polygon[j].x;

                      pfm_def[i].open_args.head.mbr.min_y = qMin (pfm_def[i].open_args.head.mbr.min_y, pfm_def[i].open_args.head.polygon[j].y);
                      pfm_def[i].open_args.head.mbr.min_x = qMin (pfm_def[i].open_args.head.mbr.min_x, pfm_def[i].open_args.head.polygon[j].x);
                      pfm_def[i].open_args.head.mbr.max_y = qMax (pfm_def[i].open_args.head.mbr.max_y, pfm_def[i].open_args.head.polygon[j].y);
                      pfm_def[i].open_args.head.mbr.max_x = qMax (pfm_def[i].open_args.head.mbr.max_x, pfm_def[i].open_args.head.polygon[j].x);
                    }
                }
              else
                {
                  strcpy (string, pfm_def[i].area.toAscii ());
                  get_area_mbr2 (string, &pfm_def[i].open_args.head.polygon_count, pfm_def[i].open_args.head.polygon, &pfm_def[i].open_args.head.mbr);
                }
            }
        }
    }


  switch (id)
    {
    case QWizard::CustomButton1:
      button (QWizard::BackButton)->setEnabled (FALSE);
      button (QWizard::CustomButton1)->setEnabled (FALSE);


      //  Set the window title to be the first PFM file name.

      setWindowTitle (pfm_def[0].name);


      checkList->clear ();


      //  Open the errors file.

      sprintf (error_file, "pfmLoadM_error_file_%d.tmp", getpid ());
      if ((errfp = fopen (error_file, "w")) == NULL)
        {
          perror (error_file);
          exit (-1);
        }


      QApplication::setOverrideCursor (Qt::WaitCursor);


      struct stat fstat;


#ifdef NVLinux
      if (flags.sub) sub_in ();
#endif


      input_file_count = input_files.size ();

      input_file_def = new FILE_DEFINITION[input_file_count];


      //  Pre-check the input files.  If we have a problem we want out now.

      for (NV_INT32 j = 0 ; j < input_file_count ; j++)
        {
          input_file_def[j].name = input_files.at (j);

          NV_CHAR name[1024];
          strcpy (name, input_file_def[j].name.toAscii ());

          input_file_def[j].type = get_file_type (name);

          if (!flags.chk)
            {
              input_file_def[j].status = NVTrue;
            }
          else
            {
              progress.fbar->reset ();
              tmp = QString (tr ("Checking file %1 of %2 - %3")
                             .arg(j + 1).arg(input_file_count).arg(QFileInfo (input_file_def[j].name).fileName ()));
              progress.fbox->setTitle (tmp);
              qApp->processEvents ();

              switch (input_file_def[j].type)
                {

		  //  File that has been added from a PFM list and was marked as "deleted".

		case -1:
                  input_file_def[j].status = NVFalse;
		  break;


		  //  Unable to open the file.

		case -2:
                  tmp = tr ("Unable to open file ") + input_file_def[j].name + tr ("\nReason : ") + 
		    QString (strerror (errno));
                  QMessageBox::warning (this, tr ("Preliminary file check"), tmp);
                  input_file_def[j].status = NVFalse;
                  break;

                case PFM_UNDEFINED_DATA: 
                  tmp = tr ("Unable to determine file type for ") + input_file_def[j].name;
                  QMessageBox::warning (this, tr ("Preliminary file check"), tmp);
                  input_file_def[j].status = NVFalse;
                  break;

                case PFM_GSF_DATA:
                  input_file_def[j].status = check_gsf_file (name, progress.fbar, errfp);
                  break;

                case PFM_WLF_DATA:
                  input_file_def[j].status = check_wlf_file (name, progress.fbar, errfp);
                  break;

                case PFM_HAWKEYE_HYDRO_DATA:
                case PFM_HAWKEYE_TOPO_DATA:
                  input_file_def[j].status = check_hawkeye_file (name, progress.fbar, errfp);
                  break;

                case PFM_CHARTS_HOF_DATA:
                  input_file_def[j].status = check_hof_file (name, progress.fbar, errfp);
                  break;

                case PFM_SHOALS_TOF_DATA:
                  input_file_def[j].status = check_tof_file (name, progress.fbar, errfp);
                  break;

                case PFM_UNISIPS_DEPTH_DATA:
                  input_file_def[j].status = check_unisips_depth_file (name, progress.fbar, errfp);
                  break;

                case PFM_NAVO_ASCII_DATA:
                  input_file_def[j].status = check_hypack_xyz_file (name, progress.fbar, errfp);
                  break;

                case PFM_NAVO_LLZ_DATA:
                  input_file_def[j].status = check_llz_file (name, progress.fbar, errfp);
                  break;

                case PFM_CZMIL_DATA:
                  input_file_def[j].status = check_czmil_file (name, progress.fbar, errfp);
                  break;

                case PFM_BAG_DATA:
                  input_file_def[j].status = check_bag_file (name, progress.fbar, errfp);
                  break;

                case PFM_ASCXYZ_DATA:
                  input_file_def[j].status = check_ivs_xyz_file (name, progress.fbar, errfp);
                  break;

                case PFM_DTED_DATA:
                  input_file_def[j].status = check_dted_file (name, progress.fbar, errfp);
                  break;

                case PFM_CHRTR_DATA:
                  input_file_def[j].status = check_chrtr_file (name, progress.fbar, errfp);
                  break;
                }
            }
        }

      if (flags.chk)
        {
          cur = new QListWidgetItem (tr ("Preliminary file check complete"));
          checkList->addItem (cur);
          checkList->setCurrentItem (cur);
          checkList->scrollToItem (cur);
        }


      progress.fbar->setWhatsThis (tr ("Progress of input file loading"));


      //  Check all input files against input files in any preexisting PFM structures so we don't double load.

      for (NV_INT32 i = 0 ; i < pfm_file_count ; i++)
        {
          if (pfm_def[i].existing)
            {
              NV_INT32 temp_handle;
              PFM_OPEN_ARGS open_args;

              strcpy (open_args.list_path, pfm_def[i].name.toAscii ());

              open_args.checkpoint = 0;

              if ((temp_handle = open_existing_pfm_file (&open_args)) >= 0)
                {
                  NV_INT16 k = 0, type;
                  NV_CHAR rfile[1024];
                  while (!read_list_file (temp_handle, k, rfile, &type))
                    {
                      for (NV_INT32 j = 0 ; j < input_file_count ; j++)
                        {
                          if (!strcmp (rfile, input_file_def[j].name.toAscii ())) input_file_def[j].status = NVFalse;
                        }

                      k++;
                    }
                  close_pfm_file (temp_handle);
                }
            }
        }


      //  Put everything into load_parms so we can pass it easily.

      for (NV_INT32 i = 0 ; i < pfm_file_count ; i++) 
        {
          load_parms[i].flags = flags;
          load_parms[i].pfm_global = pfm_global;
        }


      //  Open the PFM files.

      for (NV_INT32 i = 0 ; i < pfm_file_count ; i++)
        {
	  //  If this is 0.0 then we're defining bin sizes in minutes of lat/lon

          if (pfm_def[i].mbin_size == 0.0)
            {
              pfm_def[i].open_args.head.y_bin_size_degrees = pfm_def[i].gbin_size / 60.0;


              //  We're going to use approximately spatially equivalent geographic bin sizes north or 64N and south of 64S.
              //  Otherwise we're going to use equal lat and lon bin sizes.

              if (pfm_def[i].open_args.head.mbr.min_y >= 64.0 || pfm_def[i].open_args.head.mbr.max_y <= -64.0)
                {
                  NV_FLOAT64 dist, az, y, x;
                  if (pfm_def[i].open_args.head.mbr.min_y <= -64.0)
                    {
                      invgp (NV_A0, NV_B0, pfm_def[i].open_args.head.mbr.max_y, pfm_def[i].open_args.head.mbr.min_x, 
                             pfm_def[i].open_args.head.mbr.max_y - (pfm_def[i].gbin_size / 60.0),
                             pfm_def[i].open_args.head.mbr.min_x, &dist, &az);
                    }
                  else
                    {
                      invgp (NV_A0, NV_B0, pfm_def[i].open_args.head.mbr.min_y, pfm_def[i].open_args.head.mbr.min_x, 
                             pfm_def[i].open_args.head.mbr.min_y + (pfm_def[i].gbin_size / 60.0),
                             pfm_def[i].open_args.head.mbr.min_x, &dist, &az);
                    }

                  newgp (pfm_def[i].open_args.head.mbr.min_y, pfm_def[i].open_args.head.mbr.min_x, 90.0, dist, &y, &x);

                  pfm_def[i].open_args.head.x_bin_size_degrees = x - pfm_def[i].open_args.head.mbr.min_x;
                }
              else
                {
                  pfm_def[i].open_args.head.x_bin_size_degrees = pfm_def[i].open_args.head.y_bin_size_degrees;
                }
            }
          else
            {
              pfm_def[i].open_args.head.bin_size_xy = pfm_def[i].mbin_size;
            }


          if (!pfm_def[i].existing)
            {
#warning Add Well-Known Text when we go to version 6.0 of the PFM library!
#warning Uncomment the creation_software setting when we go to version 6.0 of the PFM library!
	      //strcpy (pfm_def[i].open_args.head.creation_software, VERSION);

              tmp = QString (tr ("Initializing PFM structure %1 of %2 - %3")
                             .arg(i + 1).arg(pfm_file_count).arg(QFileInfo (pfm_def[i].name).fileName ()));
              progress.fbox->setTitle (tmp);
              qApp->processEvents ();

              get_version (pfm_def[i].open_args.head.version);
              strcpy (pfm_def[i].open_args.head.classification, "UNCLASSIFIED");

              time_t systemtime = time (&systemtime);
              strcpy (pfm_def[i].open_args.head.date, asctime (localtime (&systemtime)));

              pfm_def[i].open_args.head.date[strlen (pfm_def[i].open_args.head.date) - 1] = 0;


              pfm_def[i].open_args.head.min_filtered_depth = 9999999.0;
              pfm_def[i].open_args.head.min_depth = 9999999.0;
              pfm_def[i].open_args.head.max_filtered_depth = -9999999.0;
              pfm_def[i].open_args.head.max_depth = -9999999.0;
              pfm_def[i].open_args.head.min_bin_count = 9999999;
              pfm_def[i].open_args.head.max_bin_count = 0;
              pfm_def[i].open_args.head.min_standard_dev = 9999999.0;
              pfm_def[i].open_args.head.max_standard_dev = -9999999.0;
            }
          else
            {
              tmp = QString (tr ("Creating PFM checkpoint file %1 of %2 - %3")
                             .arg(i + 1).arg(pfm_file_count).arg(QFileInfo (pfm_def[i].name).fileName ()));
              progress.fbox->setTitle (tmp);
              qApp->processEvents ();
            }


          strcpy (pfm_def[i].open_args.list_path, pfm_def[i].name.toAscii ());
          strcpy (pfm_def[i].open_args.image_path, pfm_def[i].mosaic.toAscii ());
          strcpy (pfm_def[i].open_args.target_path, pfm_def[i].feature.toAscii ());
          pfm_def[i].open_args.max_depth = pfm_def[i].max_depth;
          pfm_def[i].open_args.offset = -pfm_def[i].min_depth;
          pfm_def[i].open_args.scale = 1.0 / pfm_def[i].precision;
          pfm_def[i].open_args.checkpoint = 1;
          pfm_def[i].open_args.head.dynamic_reload = NVTrue;
          pfm_def[i].open_args.head.num_bin_attr = 0;
          pfm_def[i].open_args.head.num_ndx_attr = 0;

          pfm_def[i].open_args.head.max_input_files = pfm_global.max_files;
          pfm_def[i].open_args.head.max_input_lines = pfm_global.max_lines;
          pfm_def[i].open_args.head.max_input_pings = pfm_global.max_pings;
          pfm_def[i].open_args.head.max_input_beams = pfm_global.max_beams;


          //  If we have attributes we need to define them.

          if (pfm_global.attribute_count)
            {
              pfm_def[i].open_args.head.num_ndx_attr = pfm_global.attribute_count;


              NV_INT32 index = pfm_global.time_attribute_num - 1;
              if (pfm_global.time_attribute_num)
                {
                  strcpy (pfm_def[i].open_args.head.ndx_attr_name[index], pfm_global.time_attribute_name.toAscii ());
                  pfm_def[i].open_args.head.min_ndx_attr[index] = pfm_global.time_attribute_def[0];
                  pfm_def[i].open_args.head.max_ndx_attr[index] = pfm_global.time_attribute_def[1];
                  pfm_def[i].open_args.head.ndx_attr_scale[index] = pfm_global.time_attribute_def[2];
                }

              for (NV_INT32 j = 0 ; j < GSF_ATTRIBUTES ; j++)
                {
                  NV_INT32 index = pfm_global.gsf_attribute_num[j] - 1;
                  if (pfm_global.gsf_attribute_num[j])
                    {
                      strcpy (pfm_def[i].open_args.head.ndx_attr_name[index], pfm_global.gsf_attribute_name[j].toAscii ());
                      pfm_def[i].open_args.head.min_ndx_attr[index] = pfm_global.gsf_attribute_def[j][0];
                      pfm_def[i].open_args.head.max_ndx_attr[index] = pfm_global.gsf_attribute_def[j][1];
                      pfm_def[i].open_args.head.ndx_attr_scale[index] = pfm_global.gsf_attribute_def[j][2];
                    }
                }

              for (NV_INT32 j = 0 ; j < HOF_ATTRIBUTES ; j++)
                {
                  NV_INT32 index = pfm_global.hof_attribute_num[j] - 1;
                  if (pfm_global.hof_attribute_num[j])
                    {
                      strcpy (pfm_def[i].open_args.head.ndx_attr_name[index], pfm_global.hof_attribute_name[j].toAscii ());
                      pfm_def[i].open_args.head.min_ndx_attr[index] = pfm_global.hof_attribute_def[j][0];
                      pfm_def[i].open_args.head.max_ndx_attr[index] = pfm_global.hof_attribute_def[j][1];
                      pfm_def[i].open_args.head.ndx_attr_scale[index] = pfm_global.hof_attribute_def[j][2];
                    }
                }

              for (NV_INT32 j = 0 ; j < TOF_ATTRIBUTES ; j++)
                {
                  NV_INT32 index = pfm_global.tof_attribute_num[j] - 1;
                  if (pfm_global.tof_attribute_num[j])
                    {
                      strcpy (pfm_def[i].open_args.head.ndx_attr_name[index], pfm_global.tof_attribute_name[j].toAscii ());
                      pfm_def[i].open_args.head.min_ndx_attr[index] = pfm_global.tof_attribute_def[j][0];
                      pfm_def[i].open_args.head.max_ndx_attr[index] = pfm_global.tof_attribute_def[j][1];
                      pfm_def[i].open_args.head.ndx_attr_scale[index] = pfm_global.tof_attribute_def[j][2];
                    }
                }

              for (NV_INT32 j = 0 ; j < WLF_ATTRIBUTES ; j++)
                {
                  NV_INT32 index = pfm_global.wlf_attribute_num[j] - 1;
                  if (pfm_global.wlf_attribute_num[j])
                    {
                      strcpy (pfm_def[i].open_args.head.ndx_attr_name[index], pfm_global.wlf_attribute_name[j].toAscii ());
                      pfm_def[i].open_args.head.min_ndx_attr[index] = pfm_global.wlf_attribute_def[j][0];
                      pfm_def[i].open_args.head.max_ndx_attr[index] = pfm_global.wlf_attribute_def[j][1];
                      pfm_def[i].open_args.head.ndx_attr_scale[index] = pfm_global.wlf_attribute_def[j][2];
                    }
                }

              for (NV_INT32 j = 0 ; j < CZMIL_ATTRIBUTES ; j++)
                {
                  NV_INT32 index = pfm_global.czmil_attribute_num[j] - 1;
                  if (pfm_global.czmil_attribute_num[j])
                    {
                      strcpy (pfm_def[i].open_args.head.ndx_attr_name[index], pfm_global.czmil_attribute_name[j].toAscii ());
                      pfm_def[i].open_args.head.min_ndx_attr[index] = pfm_global.czmil_attribute_def[j][0];
                      pfm_def[i].open_args.head.max_ndx_attr[index] = pfm_global.czmil_attribute_def[j][1];
                      pfm_def[i].open_args.head.ndx_attr_scale[index] = pfm_global.czmil_attribute_def[j][2];
                    }
                }

              for (NV_INT32 j = 0 ; j < BAG_ATTRIBUTES ; j++)
                {
                  NV_INT32 index = pfm_global.bag_attribute_num[j] - 1;
                  if (pfm_global.bag_attribute_num[j])
                    {
                      strcpy (pfm_def[i].open_args.head.ndx_attr_name[index], pfm_global.bag_attribute_name[j].toAscii ());
                      pfm_def[i].open_args.head.min_ndx_attr[index] = pfm_global.bag_attribute_def[j][0];
                      pfm_def[i].open_args.head.max_ndx_attr[index] = pfm_global.bag_attribute_def[j][1];
                      pfm_def[i].open_args.head.ndx_attr_scale[index] = pfm_global.bag_attribute_def[j][2];
                    }
                }

              for (NV_INT32 j = 0 ; j < HAWKEYE_ATTRIBUTES ; j++)
                {
                  NV_INT32 index = pfm_global.hawkeye_attribute_num[j] - 1;
                  if (pfm_global.hawkeye_attribute_num[j])
                    {
                      strcpy (pfm_def[i].open_args.head.ndx_attr_name[index], pfm_global.hawkeye_attribute_name[j].toAscii ());
                      pfm_def[i].open_args.head.min_ndx_attr[index] = pfm_global.hawkeye_attribute_def[j][0];
                      pfm_def[i].open_args.head.max_ndx_attr[index] = pfm_global.hawkeye_attribute_def[j][1];
                      pfm_def[i].open_args.head.ndx_attr_scale[index] = pfm_global.hawkeye_attribute_def[j][2];
                    }
                }
            }


          if (flags.lid)
            {
              strcpy (pfm_def[i].open_args.head.user_flag_name[0], "Shallow Water Process!");
              strcpy (pfm_def[i].open_args.head.user_flag_name[1], "APD!");
              strcpy (pfm_def[i].open_args.head.user_flag_name[2], "Land!");
              strcpy (pfm_def[i].open_args.head.user_flag_name[3], "Second Depth Present!");
            }


          if (flags.attr)
            {
              NV_INT32 status = stat (pfm_def[i].open_args.list_path, &fstat);

              if (status == -1 && errno == ENOENT)
                {
                  pfm_def[i].open_args.head.num_bin_attr = 6;
                  strcpy (pfm_def[i].open_args.head.bin_attr_name[0], "###0");
                  strcpy (pfm_def[i].open_args.head.bin_attr_name[1], "###1");
                  strcpy (pfm_def[i].open_args.head.bin_attr_name[2], "###2");
                  strcpy (pfm_def[i].open_args.head.bin_attr_name[3], "###3");
                  strcpy (pfm_def[i].open_args.head.bin_attr_name[4], "###4");
                  strcpy (pfm_def[i].open_args.head.bin_attr_name[5], "###5");


                  pfm_def[i].open_args.head.min_bin_attr[0] = 0.0;
                  pfm_def[i].open_args.head.min_bin_attr[1] = 0.0;
                  pfm_def[i].open_args.head.min_bin_attr[2] = 0.0;
                  pfm_def[i].open_args.head.min_bin_attr[3] = 0.0;
                  pfm_def[i].open_args.head.min_bin_attr[4] = 0.0;
                  pfm_def[i].open_args.head.min_bin_attr[5] = 0.0;
                  pfm_def[i].open_args.head.max_bin_attr[0] = 100;
                  pfm_def[i].open_args.head.max_bin_attr[1] = 100.0;
                  pfm_def[i].open_args.head.max_bin_attr[2] = 100.0;
                  pfm_def[i].open_args.head.max_bin_attr[3] = 32000.0;
                  pfm_def[i].open_args.head.max_bin_attr[4] = 100.0;
                  pfm_def[i].open_args.head.max_bin_attr[5] = 100.0;
                  pfm_def[i].open_args.head.bin_attr_scale[0] = 1.0;
                  pfm_def[i].open_args.head.bin_attr_scale[1] = 100.0;
                  pfm_def[i].open_args.head.bin_attr_scale[2] = 100.0;
                  pfm_def[i].open_args.head.bin_attr_scale[3] = 1.0;
                  pfm_def[i].open_args.head.bin_attr_scale[4] = 100.0;
                  pfm_def[i].open_args.head.bin_attr_scale[5] = 100.0;
                }
            }


	  NV_INT32 status = stat (pfm_def[i].open_args.list_path, &fstat);

	  if (status == -1 && errno == ENOENT)
	    {
	      //  Using 10% of max depth as maximum vertical error value except when the max depth is less than 150, then we use
	      //  15.0.

	      if (pfm_def[i].open_args.max_depth < 150.0)
		{
		  pfm_def[i].open_args.head.max_vertical_error = 15.0;
		}
	      else
		{
		  pfm_def[i].open_args.head.max_vertical_error = pfm_def[i].open_args.max_depth * 0.10;
		}


	      //  Setting maximum horizontal error to 10 times the bin size except when we are using bin sizes set in minutes
	      //  (in which case bin_size_xy is set to 0.0).  In that case we must first figure out what the nominal bin size is
	      //  in meters.

	      if (pfm_def[i].open_args.head.bin_size_xy != 0.0)
		{
		  pfm_def[i].open_args.head.max_horizontal_error = pfm_def[i].open_args.head.bin_size_xy * 10.0;
		}
	      else
		{
		  NV_FLOAT64 dist, az;

		  invgp (NV_A0, NV_B0, pfm_def[i].open_args.head.mbr.min_y, pfm_def[i].open_args.head.mbr.min_x, 
			 pfm_def[i].open_args.head.mbr.min_y + (pfm_def[i].gbin_size / 60.0),
			 pfm_def[i].open_args.head.mbr.min_x, &dist, &az);
		  pfm_def[i].open_args.head.max_horizontal_error = dist * 10.0;
		}


	      //  Horizontal and vertical error scales set to centimeters.

	      pfm_def[i].open_args.head.horizontal_error_scale = 100.0;
	      pfm_def[i].open_args.head.vertical_error_scale = 100.0;
            }


          pfm_register_progress_callback (initProgress);
          prog->reset ();


          if ((pfm_def[i].hnd = open_cached_pfm_file (&pfm_def[i].open_args)) < 0) pfm_error_exit (pfm_error);
          //if ((pfm_def[i].hnd = open_pfm_file (&pfm_def[i].open_args)) < 0) pfm_error_exit (pfm_error);


          //  Force LIDAR attribute updates if they were in a pre-existing file.

          for (NV_INT32 j = 0 ; j < pfm_def[i].open_args.head.num_ndx_attr ; j++)
            {
              if (strstr (pfm_def[i].open_args.head.ndx_attr_name[j], "Shallow Water Process"))
                {
                  flags.lid = NVTrue;
                  break;
                }
            }


          //  Force CUBE attribute updates if they were in a pre-existing file.

          for (NV_INT32 j = 0 ; j < pfm_def[i].open_args.head.num_bin_attr ; j++)
            {
              if (!strcmp (pfm_def[i].open_args.head.bin_attr_name[j], "###0"))
                {
                  flags.attr = NVTrue;
                  break;
                }
            }


          //  Check for dateline crossing.

          pfm_def[i].dateline = NVFalse;
          if (!pfm_def[i].open_args.head.proj_data.projection && pfm_def[i].open_args.head.mbr.max_x > 180.0)
            pfm_def[i].dateline = NVTrue;


          //  Allocate and clear memory for the bin add_map.  

          pfm_def[i].add_map = (NV_BYTE *) calloc (pfm_def[i].open_args.head.bin_width * 
                                                   pfm_def[i].open_args.head.bin_height, sizeof (NV_BYTE));

          if (pfm_def[i].add_map == NULL)
            {
              perror ("Unable to allocate memory for bin_map.");
              exit (-1);
            }
        }

      tmp = tr ("PFM structure initialization complete");
      progress.fbar->setValue (100);
      progress.fbox->setTitle (tmp);
      qApp->processEvents ();

      cur = new QListWidgetItem (tmp);
      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);


      //  Zero sounding count 

      NV_INT32 count[MAX_PFM_FILES], out_of_range[MAX_PFM_FILES];

      memset (count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
      memset (out_of_range, 0, MAX_PFM_FILES * sizeof (NV_INT32));


      //  Save the time to the handle file as a comment

      for (NV_INT32 i = 0 ; i < pfm_file_count ; i++)
        {
          FILE *fp = fopen (pfm_def[i].open_args.list_path, "a");

          QDateTime current_time = QDateTime::currentDateTime ();

          strcpy (tmp_char, current_time.toString ().toAscii ());
          fprintf (fp, tr ("#\n#\n# Date : %s  (GMT)\n#\n").toAscii (), tmp_char);
          fclose (fp);
        }


      if (input_file_count >= PRODUCERS && pfm_file_count == 1)
        {
          read_complete = NVFalse;


          QMutex pfmMutex[MAX_PFM_FILES], transMutex[CONSUMERS];


          for (NV_INT32 i = 0 ; i < CONSUMERS ; i++)
            {
              freeBuffers[i] = new QSemaphore (BUFFER_SIZE);
              usedBuffers[i] = new QSemaphore ();
            }


          TRANSFER transfer[CONSUMERS];
          for (NV_INT32 i = 0 ; i < CONSUMERS ; i++) memset (&transfer[i], 0, sizeof (TRANSFER));


          PFM_OPEN_ARGS thread_args[MAX_PFM_FILES][CONSUMERS];
          THREAD_DATA thd[MAX_PFM_FILES];

          for (NV_INT32 i = 0 ; i < pfm_file_count ; i++)
            {
              for (NV_INT32 j = 0 ; j < CONSUMERS ; j++)
                {
                  thread_args[i][j] = pfm_def[i].open_args;
                  thread_args[i][j].checkpoint = 3;

                  sprintf (&thread_args[i][j].list_path[strlen (thread_args[i][j].list_path) - 4], "_part_%01d.pfm", j);
                  if ((thd[i].hnd[j] = open_cached_pfm_file (&thread_args[i][j])) < 0) pfm_error_exit (pfm_error);
                  //if ((thd[i].hnd[j] = open_pfm_file (&thread_args[i][j])) < 0) pfm_error_exit (pfm_error);
 fprintf(stderr,"%s %d %d %d %d %s\n",__FILE__,__LINE__,i,j,thd[i].hnd[j],thread_args[i][j].list_path);
                }

              NV_INT32 mid_x, mid_y;
              mid_x = pfm_def[i].open_args.head.bin_width / 2;
              mid_y = pfm_def[i].open_args.head.bin_height / 2;

              thd[i].mbr[0].min_x = pfm_def[0].open_args.head.mbr.min_x;
              thd[i].mbr[0].min_y = pfm_def[0].open_args.head.mbr.min_y;
              thd[i].mbr[0].max_x = pfm_def[0].open_args.head.mbr.min_x + (NV_FLOAT64) mid_x *
                pfm_def[0].open_args.head.x_bin_size_degrees;
              thd[i].mbr[0].max_y = pfm_def[0].open_args.head.mbr.min_y + (NV_FLOAT64) mid_y *
                pfm_def[0].open_args.head.y_bin_size_degrees;

              /*
              NV_F64_COORD2 nxy;
              nxy.x = thd[i].mbr[0].min_x;
              nxy.y = thd[i].mbr[0].min_y;
              compute_index_ptr (nxy, &thd[i].corners[0][0], &pfm_def[i].open_args.head);
              */

              thd[i].mbr[1].min_x = thd[i].mbr[0].max_x;
              thd[i].mbr[1].min_y = thd[i].mbr[0].min_y;
              thd[i].mbr[1].max_x = pfm_def[0].open_args.head.mbr.max_x;
              thd[i].mbr[1].max_y = thd[i].mbr[0].max_y;

              thd[i].mbr[2].min_x = thd[i].mbr[0].min_x;
              thd[i].mbr[2].min_y = thd[i].mbr[0].max_y;
              thd[i].mbr[2].max_x = thd[i].mbr[0].max_x;
              thd[i].mbr[2].max_y = pfm_def[0].open_args.head.mbr.max_y;

              thd[i].mbr[3].min_x = thd[i].mbr[1].min_x;
              thd[i].mbr[3].min_y = thd[i].mbr[2].min_y;
              thd[i].mbr[3].max_x = thd[i].mbr[1].max_x;
              thd[i].mbr[3].max_y = thd[i].mbr[2].max_y;
            }


          readThread read_thread[PRODUCERS];
          writeThread write_thread[CONSUMERS];
          NV_INT32 num_files = input_file_count / PRODUCERS;
          NV_INT32 shared_file_start[PRODUCERS], shared_file_end[PRODUCERS];

          for (NV_INT32 i = 0 ; i < PRODUCERS ; i++)
            {
              read_done[i] = NVFalse;

              shared_file_start[i] = i * num_files;
              shared_file_end[i] = qMin ((i + 1) * num_files, input_file_count - 1);


              //  We're starting all readThreads concurrently.  Note that we're using the Qt::DirectConnection type
              //  for the signal/slot connections.  This causes all of the signals emitted from the threads to be
              //  serviced immediately.

              qRegisterMetaType<NV_INT32> ("NV_INT32");

              connect (&read_thread[i], SIGNAL (percentValue (NV_INT32, NV_INT32, NV_INT32)), this,
                       SLOT (slotReadPercentValue (NV_INT32, NV_INT32, NV_INT32)), Qt::DirectConnection);

              read_thread[i].read (i, thd, shared_file_start[i], shared_file_end[i], input_file_def,
                                   load_parms, pfm_def, freeBuffers, usedBuffers, transfer, errfp,
                                   out_of_range, count, pfmMutex, transMutex, read_done);
            }


          for (NV_INT32 i = 0 ; i < CONSUMERS ; i++)
            {
              //  We're starting all writeThreads concurrently.  Note that we're using the Qt::DirectConnection type
              //  for the signal/slot connections.  This causes all of the signals emitted from the threads to be
              //  serviced immediately.

              qRegisterMetaType<NV_INT32> ("NV_INT32");

              connect (&write_thread[i], SIGNAL (complete (NV_INT32, NV_INT32, NV_INT32)), this,
                       SLOT (slotWriteComplete (NV_INT32, NV_INT32, NV_INT32)), Qt::DirectConnection);

              write_thread[i].write (i, load_parms, pfm_def, pfmMutex, freeBuffers, usedBuffers, transfer, errfp, read_done,
                                     thd);
            }


          //  We can't move on until the threads are complete but we want to keep our progress bar updated.  This is a bit tricky 
          //  because you can't update the progress bar from within slots connected to thread signals.  Those slots are considered part
          //  of the read and write threads and not part of the GUI thread.  When the threads are finished we move on.

          NV_INT32 pass_count = 0;
          while (pass_count < CONSUMERS)
            {
#ifdef NVWIN3X
              Sleep (2000);
#else
              usleep (2000000);
#endif

              if (read_complete)
                {
                  pass_count = 0;
                  for (NV_INT32 i = 0 ; i < CONSUMERS ; i++)
                    {
                      if (!usedBuffers[i]->available ()) pass_count++;
                      fprintf (stderr, "%s %d %d %d\n",__FILE__,__LINE__,i, usedBuffers[i]->available ());
                    }


                  if (pass_count >= CONSUMERS)
                    {
                      //  Just trying to make sure we exit from our consumer (write) threads nicely.  If we didn't 
                      //  put any data into a consumer thread it will be hanging on a semaphore acquire.  We'll set
                      //  the "pos" to a negative number (now that all of the read threads are done) to indicate to
                      //  the consumer thread that everything is done.

                      for (NV_INT32 i = 0 ; i < CONSUMERS ; i++)
                        {
                          transfer[i].pos = -1;
                          usedBuffers[i]->release ();
                        }
                    }
                }

              qApp->processEvents ();
            }


          for (NV_INT32 k = 0 ; k < pfm_file_count ; k++)
            {
              for (NV_INT32 m = 0 ; m < CONSUMERS ; m++)
                {
                  close_cached_pfm_file (thd[k].hnd[m]);

                  if ((thd[k].hnd[m] = open_pfm_file (&thread_args[k][m])) < 0) pfm_error_exit (pfm_error);

                  for (NV_INT32 i = 0 ; i < pfm_def[k].open_args.head.bin_height ; i++)
                    {
                      for (NV_INT32 j = 0 ; j < pfm_def[k].open_args.head.bin_width ; j++)
                        {
                          NV_I32_COORD2 coord;
                          coord.y = i;
                          coord.x = j;
                          DEPTH_RECORD *depth;
                          NV_INT32 numrecs;

                          if (!read_depth_array_index (thd[k].hnd[m], coord, &depth, &numrecs))
                            {
                              for (NV_INT32 n = 0 ; n < numrecs ; n++)
                                {
                                  NV_INT32 status;
                                  if ((status = add_cached_depth_record (pfm_def[k].hnd, &depth[n])))
                                    {
                                      if (status == WRITE_BIN_RECORD_DATA_READ_ERROR)
                                        {
                                          fprintf (stderr, "%s\n", pfm_error_str (status));
                                          fprintf (stderr, "%d %d %f %f %f %f\n\n", depth[n].coord.y, depth[n].coord.x,
                                                   pfm_def[k].open_args.head.mbr.min_y, pfm_def[k].open_args.head.mbr.max_y,
                                                   pfm_def[k].open_args.head.mbr.min_x, pfm_def[k].open_args.head.mbr.max_x);
                                          fflush (stderr);
                                        }
                                      else
                                        {
                                          pfm_error_exit (status);
                                        }
                                    }
                                }

                              free (depth);
                            }
                        }
                    }

                  close_pfm_file (thd[k].hnd[m]);

                  remove (thread_args[k][m].list_path);
                  destroy_dir (QString (thread_args[k][m].list_path).append (".data"));
                }
            }
          fprintf (stderr, "%s %d %d %d\n",__FILE__,__LINE__,total_out_count, total_out_of_limits);
        }
      else
        {
          total_input_count = 0;
          for (NV_INT32 i = 0 ; i < input_file_count ; i++)
            {
              //  Make sure that the file status was good.

              if (input_file_def[i].status)
                {
                  progress.fbar->reset ();

                  tmp = QString (tr ("Reading file %1 of %2 - %3")
                                 .arg(i + 1).arg(input_file_count).arg(QFileInfo (input_file_def[i].name).fileName ()));

                  progress.fbox->setTitle (tmp);


                  //  Note: When adding other file types to process, add them here, the processing within the call is 
                  //  responsible for opening the file, parsing it, navigating it and calling Do_PFM_Processing with the 
                  //  appropriate arguments.  We can use keywords within the filename string to determine the file type
                  //  or open and look for a header. 

                  switch (input_file_def[i].type)
                    {
                    case PFM_GSF_DATA:
                      out_of_range[i] += load_gsf_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                        load_parms, pfm_def);
                      break;

                    case PFM_WLF_DATA:
                      out_of_range[i] += load_wlf_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                        load_parms, pfm_def);
                      break;

                    case PFM_HAWKEYE_HYDRO_DATA:
                    case PFM_HAWKEYE_TOPO_DATA:
                      out_of_range[i] += load_hawkeye_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                            load_parms, pfm_def);
                      break;

                    case PFM_CHARTS_HOF_DATA:
                      out_of_range[i] += load_hof_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                        load_parms, pfm_def);
                      break;

                    case PFM_SHOALS_TOF_DATA:
                      out_of_range[i] += load_tof_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                        load_parms, pfm_def);
                      break;

                    case PFM_UNISIPS_DEPTH_DATA:
                      out_of_range[i] += load_unisips_depth_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                                  load_parms, pfm_def);
                      break;

                    case PFM_NAVO_ASCII_DATA:
                      out_of_range[i] += load_hypack_xyz_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                               load_parms, pfm_def);
                      break;

                    case PFM_NAVO_LLZ_DATA:
                      out_of_range[i] += load_llz_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                        load_parms, pfm_def);
                      break;

                    case PFM_CZMIL_DATA:
                      out_of_range[i] += load_czmil_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                          load_parms, pfm_def);
                      break;

                    case PFM_BAG_DATA:
                      out_of_range[i] += load_bag_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                        load_parms, pfm_def);
                      break;

                    case PFM_ASCXYZ_DATA:
                      out_of_range[i] += load_ivs_xyz_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                            load_parms, pfm_def);
                      break;

                    case PFM_DTED_DATA:
                      out_of_range[i] += load_dted_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                         load_parms, pfm_def);
                      break;

                    case PFM_CHRTR_DATA:
                      out_of_range[i] += load_chrtr_file (pfm_file_count, count, input_file_def[i].name, progress.fbar, errfp,
                                                          load_parms, pfm_def);
                      break;
                    }

                  total_input_count = 0;
                  for (NV_INT32 j = 0 ; j < pfm_file_count ; j++) 
                    {
                      FILE *fp = fopen (pfm_def[j].open_args.list_path, "a");

                      strcpy (tmp_char, input_file_def[i].name.toAscii ());
                      fprintf (fp, tr ("# File : %s\n").toAscii (), tmp_char);
                      fprintf (fp, tr ("# Total data points read = %d\n").toAscii (), count[j]);
                      fclose (fp);

                      total_input_count += count[j];
                    }

                  tmp = tr ("File : ") + input_file_def[i].name;
                  checkList->addItem (tmp);
                  tmp = QString (tr ("Total data points read = %1")).arg(total_input_count);

                  cur = new QListWidgetItem (tmp);
                  checkList->addItem (cur);
                  checkList->setCurrentItem (cur);
                  checkList->scrollToItem (cur);
                }
            }
        }


      //  Just in case we only loaded SRTM data.

      if (input_file_count)
        {
          tmp = tr ("Reading input files complete (%1 files read)").arg (input_file_count);
          progress.fbox->setTitle (tmp);
          qApp->processEvents ();
        }
      else
        {
          tmp = tr ("No input data read");
          progress.fbox->setTitle (tmp);
          qApp->processEvents ();
        }


      cur = new QListWidgetItem (tmp);
      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);


      //  If we are loading SRTM data, call the load function.

      if (load_parms[0].flags.srtmb || load_parms[0].flags.srtm1 || load_parms[0].flags.srtm3 || load_parms[0].flags.srtm30)
        {
          tmp = tr ("Reading SRTM data");
          progress.fbox->setTitle (tmp);
          qApp->processEvents ();


          load_srtm_file (pfm_file_count, count, progress.fbar, load_parms, pfm_def);


          for (NV_INT32 j = 0 ; j < pfm_file_count ; j++) 
            {
              FILE *fp = fopen (pfm_def[j].open_args.list_path, "a");

              if (load_parms[0].flags.srtmb)
                {
                  fprintf (fp, tr ("# Best available resolution SRTM data\n").toAscii ());
                }
              else if (load_parms[0].flags.srtm1)
                {
                  fprintf (fp, tr ("# 1 second resolution SRTM data\n").toAscii ());
                }
              else if (load_parms[0].flags.srtm3)
                {
                  fprintf (fp, tr ("# 3 second resolution SRTM data\n").toAscii ());
                }
              else if (load_parms[0].flags.srtm30)
                {
                  fprintf (fp, tr ("# 30 second resolution SRTM data\n").toAscii ());
                }

              fprintf (fp, tr ("# Total data points read = %d\n").toAscii (), count[j]);
              fclose (fp);

              total_input_count += count[j];
            }


          if (load_parms[0].flags.srtmb)
            {
              tmp.sprintf (tr ("\n\nBest available resolution SRTM data").toAscii ());
            }
          else if (load_parms[0].flags.srtm1)
            {
              tmp.sprintf (tr ("\n\n1 second resolution SRTM data").toAscii ());
            }
          else if (load_parms[0].flags.srtm3)
            {
              tmp.sprintf (tr ("\n\n3 second resolution SRTM data").toAscii ());
            }
          else if (load_parms[0].flags.srtm30)
            {
              tmp.sprintf (tr ("\n\n30 second resolution SRTM data").toAscii ());
            }
          checkList->addItem (tmp);
          tmp.sprintf (tr ("Total data points read = %d").toAscii (), total_input_count);

          cur = new QListWidgetItem (tmp);
          checkList->addItem (cur);
          checkList->setCurrentItem (cur);
          checkList->scrollToItem (cur);

          tmp = tr ("SRTM data input complete");
          progress.fbox->setTitle (tmp);
          qApp->processEvents ();
        }
      else
        {
          tmp = tr ("No SRTM data loaded");
        }
DPRINT

      cur = new QListWidgetItem (tmp);
      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);


      //  Write that we've finished loading into each PFM handle file.

      for (NV_INT32 k = 0 ; k < pfm_file_count ; k++)
        {
          //  Post 4.6 structure (handle file)

          FILE *fp = fopen (pfm_def[k].open_args.list_path, "a");

          fprintf (fp, "# --------------------------------------\n");
          fprintf (fp, tr ("# Finished with data load\n").toAscii ());
          fprintf (fp, "# --------------------------------------\n");
          fclose (fp);
        }

DPRINT

      for (NV_INT32 k = 0 ; k < pfm_file_count ; k++)
        {
          NV_INT32 old_percent = -1;


          //  Close and reopen the PFM files so that we can take advantage of the head and tail pointers in the bin records.

DPRINT
          close_cached_pfm_file (pfm_def[k].hnd);
DPRINT
          //close_pfm_file (pfm_def[k].hnd);
          pfm_def[k].open_args.checkpoint = 0;

          if ((pfm_def[k].hnd = open_existing_pfm_file (&pfm_def[k].open_args)) < 0)
            {
              QString tmp = tr ("An error occurred while trying to recompute the bin surfaces for file\n") + QString (pfm_def[k].open_args.list_path) +
                tr ("\nThe error message was :\n") + QString (pfm_error_str (pfm_error)) +
                tr ("\n\nPlease try running pfm_recompute on the file to correct the problem.");
              QMessageBox::warning (this, tr ("pfmLoadM recompute"), tmp);

              pfm_error_exit (pfm_error);
            }

DPRINT

          //  Compute diagonal for area filter.  First we have to check to see if we're using geographic or meter bin sizes.


          NV_FLOAT64 bin_diagonal;
          if (fabs (pfm_def[k].open_args.head.x_bin_size_degrees - pfm_def[k].open_args.head.y_bin_size_degrees) < 0.000001)
            {
              NV_FLOAT64 az;
              if (pfm_def[k].open_args.head.mbr.min_y <= 0.0)
                {
                  invgp (NV_A0, NV_B0, pfm_def[k].open_args.head.mbr.max_y, pfm_def[k].open_args.head.mbr.min_x, 
                         pfm_def[k].open_args.head.mbr.max_y - (pfm_def[k].open_args.head.y_bin_size_degrees), 
                         pfm_def[k].open_args.head.mbr.min_x + (pfm_def[k].open_args.head.x_bin_size_degrees), 
                         &bin_diagonal, &az);
                }
              else
                {
                  invgp (NV_A0, NV_B0, pfm_def[k].open_args.head.mbr.min_y, pfm_def[k].open_args.head.mbr.min_x, 
                         pfm_def[k].open_args.head.mbr.min_y + (pfm_def[k].open_args.head.y_bin_size_degrees), 
                         pfm_def[k].open_args.head.mbr.min_x + (pfm_def[k].open_args.head.x_bin_size_degrees), 
                         &bin_diagonal, &az);
                }
            }
          else
            {
              bin_diagonal = 2.0 * sqrt (pfm_def[k].open_args.head.bin_size_xy);
            }


          NV_INT32 total_bins = pfm_def[k].open_args.head.bin_height * pfm_def[k].open_args.head.bin_width;


          //  If we are going to use the area filter we need to recompute all those cells that were modified.

          if (pfm_def[k].apply_area_filter)
            {
              InitializeAreaFilter (&pfm_def[k]);

              tmp = QString (tr ("Filtering bins in PFM %1 of %2 - %3").arg(k + 1).arg(pfm_file_count).arg(QFileInfo (pfm_def[k].name).fileName ()));
            }
          else
            {
              tmp = QString (tr ("Recomputing bins in PFM %1 of %2 - %3").arg(k + 1).arg(pfm_file_count).arg(QFileInfo (pfm_def[k].name).fileName ()));
            }
          progress.rbox->setTitle (tmp);
          progress.rbar->reset ();


          NV_I32_COORD2 coord;
          BIN_RECORD bin_record;

DPRINT

          //  Recompute all of the bins and min/max values.

          for (NV_INT32 i = 0; i < pfm_def[k].open_args.head.bin_height; i++)
            {
              coord.y = i;
              for (NV_INT32 j = 0; j < pfm_def[k].open_args.head.bin_width; j++)
                {
                  coord.x = j;
                  if (pfm_def[k].add_map[i * pfm_def[k].open_args.head.bin_width + j])
                    {

                      //  Clear filter edits around features.

                      NV_BOOL trg = clear_features (pfm_def[k].hnd, coord, pfm_def[k].open_args.list_path, pfm_def[k].radius);


                      //  Don't filter if there was a feature within "radius" of the center of the bin.

                      if (!trg && pfm_def[k].apply_area_filter)
                        {
                          AreaFilter (&coord, &bin_record, &pfm_def[k], bin_diagonal, errfp);
                        }
                      else
                        {
                          recompute_bin_values_index (pfm_def[k].hnd, coord, &bin_record, 0);
                        }


                      //  Reset the PFM_CHECKED and PFM_VERIFIED flags, even around features.

                      bin_record.validity &= (~PFM_CHECKED & ~PFM_VERIFIED);
                      write_bin_record_validity_index (pfm_def[k].hnd, &bin_record, PFM_CHECKED);


                      if (bin_record.num_soundings)
                        {
                          if (bin_record.num_soundings < pfm_def[k].open_args.head.min_bin_count)
                            {
                              pfm_def[k].open_args.head.min_bin_count = bin_record.num_soundings;
                              pfm_def[k].open_args.head.min_count_coord = bin_record.coord;
                            }

                          if (bin_record.num_soundings > pfm_def[k].open_args.head.max_bin_count)
                            {
                              pfm_def[k].open_args.head.max_bin_count = bin_record.num_soundings;
                              pfm_def[k].open_args.head.max_count_coord = bin_record.coord;
                            }


                          if (bin_record.max_depth < pfm_def[k].max_depth + 1.0)
                            {
                              if (bin_record.validity & PFM_DATA)
                                {
                                  if (bin_record.min_filtered_depth < pfm_def[k].open_args.head.min_filtered_depth)
                                    {
                                      pfm_def[k].open_args.head.min_filtered_depth = bin_record.min_filtered_depth;
                                      pfm_def[k].open_args.head.min_filtered_coord = bin_record.coord;
                                    }

                                  if (bin_record.max_filtered_depth > pfm_def[k].open_args.head.max_filtered_depth)
                                    {
                                      pfm_def[k].open_args.head.max_filtered_depth = bin_record.max_filtered_depth;
                                      pfm_def[k].open_args.head.max_filtered_coord = bin_record.coord;
                                    }

                                  if (bin_record.standard_dev < pfm_def[k].open_args.head.min_standard_dev)
                                    pfm_def[k].open_args.head.min_standard_dev = bin_record.standard_dev;

                                  if (bin_record.standard_dev > pfm_def[k].open_args.head.max_standard_dev)
                                    pfm_def[k].open_args.head.max_standard_dev = bin_record.standard_dev;
                                }

                              if (bin_record.min_depth < pfm_def[k].open_args.head.min_depth)
                                {
                                  pfm_def[k].open_args.head.min_depth = bin_record.min_depth;
                                  pfm_def[k].open_args.head.min_coord = bin_record.coord;
                                }

                              if (bin_record.max_depth > pfm_def[k].open_args.head.max_depth)
                                {
                                  pfm_def[k].open_args.head.max_depth = bin_record.max_depth;
                                  pfm_def[k].open_args.head.max_coord = bin_record.coord;
                                }
                            }
                        }
                    }


                  NV_INT32 percent = (NV_INT32) (((NV_FLOAT32) (i * pfm_def[k].open_args.head.bin_width + j) / (NV_FLOAT32) total_bins) * 100.0);


                  //  We don't want to eat up all of the free time on the system with a progress bar.

                  if (percent - old_percent >= 5 || old_percent > percent)
                    {
                      progress.rbar->setValue (percent);
                      old_percent = percent;
                    }

                  qApp->processEvents ();
                }
            }


          if (pfm_def[k].apply_area_filter)
            {
              tmp = tr ("Filtering bins - complete");
            }
          else
            {
              tmp = tr ("Recomputing bins - complete");
            }
          progress.rbox->setTitle (tmp);
          progress.rbar->setValue (100);
          qApp->processEvents ();


          cur = new QListWidgetItem (tmp);
          checkList->addItem (cur);
          checkList->setCurrentItem (cur);
          checkList->scrollToItem (cur);


          write_bin_header (pfm_def[k].hnd, &pfm_def[k].open_args.head, NVFalse);


          free (pfm_def[k].add_map);


          close_pfm_file (pfm_def[k].hnd);


          //  Post 4.6 structure (handle file)

          FILE *fp = fopen (pfm_def[k].open_args.list_path, "r+");
          fgets (string, sizeof (string), fp);

          NV_CHAR ctl_file[512];
          strcpy (ctl_file, pfm_def[k].open_args.list_path);

          if (strstr (string, "PFM Handle File"))
            {
              sprintf (ctl_file, "%s.data%1c%s.ctl", pfm_def[k].open_args.list_path, (NV_CHAR) SEPARATOR,
                       pfm_basename (pfm_def[k].open_args.list_path));
            }

          sprintf (string, "%s.bak", ctl_file);


          FILE *fp1 = fopen (ctl_file, "r");
          FILE *fp2 = fopen (string, "w+");

          if (fp1 != NULL && fp2 != NULL)
            {
              while (fgets (string, sizeof (string), fp1) != NULL) fprintf (fp2, "%s", string);
              fclose (fp1);


#ifdef NVLinux

              //  Substitute paths if flag is set.

              if (load_parms[0].flags.sub)
                {
                  remove (ctl_file);
                  fseek (fp2, 0, SEEK_SET);
                  fp1 = fopen (ctl_file, "w");

                  ngets (string, sizeof (string), fp2);
                  fprintf (fp1, "%s\n", string);

                  ngets (string, sizeof (string), fp2);
                  inv_sub (string);
                  fprintf (fp1, "%s\n", string);

                  ngets (string, sizeof (string), fp2);
                  inv_sub (string);
                  fprintf (fp1, "%s\n", string);

                  ngets (string, sizeof (string), fp2);
                  if (strcmp (string, "NONE")) inv_sub (string);
                  fprintf (fp1, "%s\n", string);

                  ngets (string, sizeof (string), fp2);
                  if (strcmp (string, "NONE")) inv_sub (string);
                  fprintf (fp1, "%s\n", string);

                  NV_CHAR pre[20], post[512];
                  while (ngets (string, sizeof (string), fp2) != NULL)
                    {
                      strncpy (pre, string, 11);
                      pre[11] = 0;
                      strcpy (post, &string[11]);
                      inv_sub (post);
                      fprintf (fp1, "%s%s\n", pre, post);
                    }

                  fclose (fp1);
                }

#endif

              fclose (fp2);
            }
          else
            {
              fprintf (errfp, "\n\nUnable to create backup list file %s.\n\n", string);
            }


          if (pfm_def[k].apply_area_filter) 
            {
              FILTER_SUMMARY summary;
              FinalizeAreaFilter (&summary);


              NV_FLOAT64 good_ratio, bad_ratio, number = (NV_FLOAT64) summary.TotalSoundings;

              if (number > 0.0)
                {
                  good_ratio = (NV_FLOAT64) summary.GoodSoundings / number * 100.0;
                  bad_ratio = (NV_FLOAT64) summary.BadSoundings / number * 100.0;


                  fseek (fp, 0, SEEK_END);
                  fprintf (fp, "# --------------------------------------\n");
                  fprintf (fp, tr ("# PFM Filter Statistics\n").toAscii ());
                  fprintf (fp, tr ("# Total number of points processed %d\n").toAscii (), summary.TotalSoundings);
                  fprintf (fp, tr ("# Number of soundings Good     : %d\n").toAscii (), summary.GoodSoundings);
                  fprintf (fp, tr ("# Percent of soundings Good     : %f\n").toAscii (), good_ratio);
                  fprintf (fp, tr ("# Number of soundings Filtered : %d\n").toAscii (), summary.BadSoundings);
                  fprintf (fp, tr ("# Percent of soundings Filtered : %f\n").toAscii (), bad_ratio);
                  fprintf (fp, "# --------------------------------------\n");


                  checkList->addItem ("--------------------------------------");
                  checkList->addItem (tr ("PFM Filter Statistics"));
                  tmp = QString (tr ("File: %1").arg(QString (pfm_def[k].open_args.list_path)));
                  checkList->addItem (tmp);
                  tmp = QString (tr ("Total number of points processed %1").arg(summary.TotalSoundings));
                  checkList->addItem (tmp);


                  tmp = QString (tr ("Number of soundings Good     : %1").arg (summary.GoodSoundings));
                  checkList->addItem (tmp);
                  tmp = QString (tr ("Percent of soundings Good     : %1").arg (good_ratio, 0, 'g', 3));
                  checkList->addItem (tmp);

                  tmp = QString (tr ("Number of soundings Filtered : %1").arg (summary.BadSoundings));
                  checkList->addItem (tmp);
                  tmp = QString (tr ("Percent of soundings Filtered : %1").arg (bad_ratio, 0, 'g', 3));
                  checkList->addItem (tmp);
                  tmp = "--------------------------------------";
                  cur = new QListWidgetItem (tmp);
                  checkList->addItem (cur);
                  checkList->setCurrentItem (cur);
                  checkList->scrollToItem (cur);
                }
            }
          else
            {
              fseek (fp, 0, SEEK_END);
              fprintf (fp, "# --------------------------------------\n");
              fprintf (fp, tr ("# Finished with recompute section\n").toAscii ());
              fprintf (fp, "# --------------------------------------\n");
            }


          if (out_of_range[k] > 5000) 
            {
              fprintf (errfp, tr ("\n**** WARNING ****\n").toAscii ());
              fprintf (errfp, tr ("File %s :\n").toAscii (), pfm_def[k].open_args.list_path);
              fprintf (errfp, tr ("%d soundings out of min/max range!\n").toAscii (), out_of_range[k]);
              fprintf (errfp, tr ("These will be marked invalid in the input files\n").toAscii ());
              fprintf (errfp, tr ("if you unload this file!\n").toAscii ());
              fprintf (errfp, "*****************\n");

              fprintf (fp, tr ("\n**** WARNING ****\n").toAscii ());
              fprintf (fp, tr ("File %s :\n").toAscii (), pfm_def[k].open_args.list_path);
              fprintf (fp, tr ("%d soundings out of min/max range!\n").toAscii (), out_of_range[k]);
              fprintf (fp, tr ("These will be marked invalid in the input files\n").toAscii ());
              fprintf (fp, tr ("if you unload this file!\n").toAscii ());
              fprintf (fp, "*****************\n");

              tmp.sprintf (tr ("File %s :\n%d soundings out of min/max range!  These will be marked invalid in the input files if you unload this file!").toAscii (), 
                           pfm_def[k].open_args.list_path, out_of_range[k]);
              QMessageBox::warning (this, tr ("pfmLoadM"), tmp);
            }
          fclose (fp);
        }


      //  If we elected to run the CUBE processing.

      if (load_parms[0].flags.cube && cube_available)
	{
	  for (NV_INT32 k = 0 ; k < pfm_file_count ; k++)
	    {
	      QStringList arguments;

	      arguments.clear ();

	      cubeProc = new QProcess (this);


	      arguments += QString (pfm_def[k].open_args.list_path);

	      QString arg;

	      arguments += "0";
	      arguments += "0.0";
	      arguments += "0.0";
	      arguments += "0.0";
	      arguments += "0.0";
	      arguments += arg.setNum (pfm_global.iho);
	      arguments += arg.setNum (pfm_global.capture);
	      arguments += arg.setNum (pfm_global.queue);
	      arguments += arg.setNum (pfm_global.horiz);
	      arguments += arg.setNum (pfm_global.distance);
	      arguments += arg.setNum (pfm_global.min_context);
	      arguments += arg.setNum (pfm_global.max_context);
	      arguments += arg.setNum (pfm_global.std2conf + 1);
	      arguments += arg.setNum (pfm_global.disambiguation);


	      connect (cubeProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
		       SLOT (slotCubeDone (int, QProcess::ExitStatus)));
	      connect (cubeProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotCubeReadyReadStandardError ()));
	      connect (cubeProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotCubeReadyReadStandardOutput ()));
	      connect (cubeProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotCubeError (QProcess::ProcessError)));

	      cubeProc->start ("navo_pfm_cube", arguments);
	    }
	}
      else
	{
	  error_and_summary ();
	}

      break;

    case QWizard::CustomButton2:
      QFileDialog *fd = new QFileDialog (this, tr ("pfmLoadM save parameter file"));
      fd->setViewMode (QFileDialog::List);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.

      QList<QUrl> urls;
      urls = fd->sidebarUrls ();
      urls += QUrl::fromLocalFile (QDir (pfm_global.output_dir).absolutePath ());
      urls += QUrl::fromLocalFile (QDir (".").absolutePath ());
      QSet<QUrl> set = urls.toSet ();
      urls = set.toList ();
      qSort (urls);
      fd->setSidebarUrls (urls);

      fd->setDirectory (QDir (pfm_global.output_dir).absolutePath ());
      connect (fd, SIGNAL (filterSelected (const QString)), this, SLOT (slotParameterFilterSelected (const QString)));

      QStringList filters;
      filters << tr ("pfmLoadM parameter file (*.prm)");

      if (pfm_global.input_dirs.size ()) filters << tr ("pfmLoadMer \"update\" parameter file (*.upr)");

      fd->setFilters (filters);
      fd->setFileMode (QFileDialog::AnyFile);
      fd->selectFilter (tr ("pfmLoadM parameter file (*.prm)"));


      if (fd->exec () == QDialog::Accepted)
        {
          QStringList files = fd->selectedFiles ();

          QString file = files.at (0);

          if (!file.isEmpty())
            {
              //  Add .prm or .upr to filename if not there.

              if (upr_file)
                {
                  if (!file.endsWith (".upr")) file.append (".upr");
                }
              else
                {
                  if (!file.endsWith (".prm")) file.append (".prm");
                }

              if (!writeParameterFile (file, input_files, pfm_def, pfm_global, flags))
                {
                  QString tmp;
                  tmp.sprintf (tr ("Error opening parameters file : %s").toAscii (), strerror (errno));
                  QMessageBox::warning (this, tr ("pfmLoadM Save load parameters"), tmp);
                }
            }
        }
      break;
    }
}



void 
pfmLoadM::slotReadPercentValue (NV_INT32 percent, NV_INT32 file_num, NV_INT32 pass)
{
  NV_INT32 pass_count = 0;

  for (NV_INT32 i = 0 ; i < PRODUCERS ; i++)
    {
      if (read_done[i]) pass_count++;
    }

  if (pass_count >= PRODUCERS) read_complete = NVTrue;

  fprintf (stderr,"%s %d %d %d %d %d %d\n",__FILE__,__LINE__,pass, file_num, percent, pass_count, read_complete);
  fprintf (stderr,"%s %d %d %d %d %d\n",__FILE__,__LINE__,usedBuffers[0]->available (),usedBuffers[1]->available (),usedBuffers[2]->available (),usedBuffers[3]->available ());
}



void 
pfmLoadM::slotWriteComplete (NV_INT32 out_count, NV_INT32 out_of_limits, NV_INT32 pass)
{
  fprintf (stderr,"%s %d %d %d %d\n",__FILE__,__LINE__,pass, out_count, out_of_limits);

  total_out_count += out_count;
  total_out_of_limits += out_of_limits;
}



void 
pfmLoadM::slotParameterFilterSelected (const QString &filter)
{
  if (filter == tr ("pfmLoadM parameter file (*.prm)"))
    {
      upr_file = NVFalse;
    }
  else
    {
      upr_file = NVTrue;
    }
}



void 
pfmLoadM::error_and_summary ()
{
  //  Check for empty error file.

  if (!(ftell (errfp)))
    {
      fclose (errfp);
      remove (error_file);
    }
  else
    {
      fclose (errfp);

      errfp = fopen (error_file, "r");

      NV_CHAR string[1024];

      while (fgets (string, sizeof (string), errfp) != NULL)
        {
          QListWidgetItem *cur = new QListWidgetItem (string);
          checkList->addItem (cur);
          checkList->setCurrentItem (cur);
          checkList->scrollToItem (cur);
        }

      fclose (errfp);
    }


  checkList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("\nLoading complete, press Finish to exit.\n"));

  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);


  QApplication::restoreOverrideCursor ();

  button (QWizard::FinishButton)->setEnabled (TRUE);
}



void 
pfmLoadM::slotCubeReadyReadStandardError ()
{
  static QString resp_string = "";


  QByteArray response = cubeProc->readAllStandardError ();


  //  Parse the return response for carriage returns and line feeds

  for (NV_INT32 i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n' || response.at (i) == '\r')
        {
          QListWidgetItem *cur = new QListWidgetItem (resp_string);

          checkList->addItem (cur);
          checkList->setCurrentItem (cur);
          checkList->scrollToItem (cur);


	  NV_CHAR resp[512];
	  strcpy (resp, resp_string.toAscii ());
	  fprintf (errfp, "%s", resp);

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



void 
pfmLoadM::slotCubeReadyReadStandardOutput ()
{
  static QString resp_string = "";


  QByteArray response = cubeProc->readAllStandardOutput ();


  //  Parse the return response for carriage returns and line feeds

  for (NV_INT32 i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n' || response.at (i) == '\r')
        {
	  if (resp_string.contains ("%"))
	    {
	      NV_CHAR resp[512];
	      strcpy (resp, resp_string.toAscii ());

	      NV_INT32 value;
	      sscanf (resp, "%d", &value);
	      progress.rbar->setValue (value);
	    }
          else if (resp_string.contains ("Processing Entire PFM"))
	    {
	      progress.rbox->setTitle (tr ("PFM CUBE processing (pass 1 of 2)"));
	      
	      progress.rbar->reset ();
	    }
          else if (resp_string.contains ("Extract Best Hypothesis"))
	    {
	      progress.rbox->setTitle (tr ("Extracting best CUBE hypotheses (pass 2 of 2)"));
	      
	      progress.rbar->reset ();
	    }
          else
            {
              if (resp_string.length () >= 3)
                {
                  QListWidgetItem *cur = new QListWidgetItem (resp_string);

                  checkList->addItem (cur);
                  checkList->setCurrentItem (cur);
                  checkList->scrollToItem (cur);
                }
            }

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



void 
pfmLoadM::slotCubeError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmLoadM CUBE"), tr ("Unable to start the CUBE process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmLoadM CUBE"), tr ("The CUBE process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmLoadM CUBE"), tr ("The CUBE process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmLoadM CUBE"), tr ("There was a write error from the CUBE process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmLoadM CUBE"), tr ("There was a read error from the CUBE process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmLoadM CUBE"), tr ("The CUBE process died with an unknown error!"));
      break;
    }
}



void 
pfmLoadM::slotCubeDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  QString tmp = tr ("PFM CUBE processing complete");

  progress.rbar->setValue (100);
  progress.rbox->setTitle (tmp);

  qApp->processEvents ();

  QListWidgetItem *cur = new QListWidgetItem (tmp);
  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);


  error_and_summary ();
}



void 
pfmLoadM::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}




void 
pfmLoadM::destroy_dir (QString dir)
{
  QDir dirs;
  dirs.cd (dir);

  dirs.setFilter (QDir::Dirs | QDir::Hidden);


  //  Get all files in this directory.

  QDir files;
  files.setFilter (QDir::Files | QDir::Hidden);

  if (files.cd (dir))
    {
      QFileInfoList flist = files.entryInfoList ();
      for (NV_INT32 i = 0 ; i < flist.size () ; i++) remove (flist.at (i).absoluteFilePath ().toAscii ());
    }


  //  Get all directories in this directory.

  QFileInfoList dlist = dirs.entryInfoList ();
  QStringList dirList;
  for (NV_INT32 i = 0 ; i < dlist.size () ; i++)
    {
      if (dlist.at (i).fileName () != "." && dlist.at (i).fileName () != "..") dirList.append (dlist.at (i).absoluteFilePath ());
    }


  //  Get all subordinate directories.

  for (NV_INT32 i = 0 ; i < dirList.size () ; i++)
    {
      QString dirName = dirList.at (i);

      if (dirs.cd (dirName))
        {
          QFileInfoList nlist = dirs.entryInfoList ();
          for (NV_INT32 i = 0 ; i < nlist.size () ; i++)
            {
              if (nlist.at (i).fileName () != "." && nlist.at (i).fileName () != "..")
                dirList.append (nlist.at (i).absoluteFilePath ());
            }
        }
    }


  //  Get all files in all subordinate directories (reverse order since we're removing directories).

  for (NV_INT32 j = dirList.size () - 1 ; j >= 0 ; j--)
    {
      files.setFilter (QDir::Files | QDir::Hidden);

      if (files.cd (dirList.at (j)))
        {
          QFileInfoList flist2 = files.entryInfoList ();

          for (NV_INT32 i = 0 ; i < flist2.size () ; i++) remove (flist2.at (i).absoluteFilePath ().toAscii ());
        }


      //  Remove each directory

      dirs.rmpath (dirList.at (j));
    }


  //  Remove the top level directory

  dirs.rmpath (dir);
}
