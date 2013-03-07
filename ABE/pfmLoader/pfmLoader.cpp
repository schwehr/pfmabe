
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



#include "pfmLoader.hpp"
#include "version.hpp"


NV_FLOAT64 settings_version = 1.1;


/***************************************************************************\
*                                                                           *
*   Module Name:        pfmLoader                                           *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       September 21, 2010                                  *
*                                                                           *
*   Purpose:            Command line version of pfmLoad Qt Wizard.  This    *
*                       allows you to read and process a pfmLoad parameter  *
*                       file (*.prm or .upr) without having to use the      *
*                       pfmLoad GUI.                                        *
*                                                                           *
\***************************************************************************/

NV_INT32 main (NV_INT32 argc, NV_CHAR **argv)
{
  pfmLoader *pf;

  pf = new pfmLoader (argc, argv);
}


void pfmLoader::usage ()
{
  fprintf (stderr, "\nUsage: pfmLoader PFM_LOAD_PARAMETERS_FILE\n\n");
  fflush (stderr);
  exit (-1);
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



pfmLoader::pfmLoader (NV_INT32 argc, NV_CHAR **argv)
{
  NV_INT32 percent = 0, old_percent = -1;


  void setAllAttributes (PFM_GLOBAL *global);
  void countAllAttributes (PFM_GLOBAL *global);
  NV_BOOL readParameterFile (QString parameter_file, QStringList *input_files, PFM_DEFINITION *pfm_def, PFM_GLOBAL *pfm_global, FLAGS *flags);
  NV_INT32 get_file_type (NV_CHAR *);
  NV_BOOL update_upr_file (QString parameter_file, FILE_DEFINITION *input_file_def, NV_INT32 input_file_count);


  //  Check for command line parameter file name argument.

  if (argc < 2) usage ();


  /*  Override the HDF5 version check so that we can read BAGs created with an older version of HDF5.  */

  putenv ((NV_CHAR *) "HDF5_DISABLE_VERSION_CHECK=2");


  // Set defaults so that if keys don't exist the parameters are defined

  pfm_global.horizontal_error = 1.5;
  pfm_global.vertical_error = 0.25;


  //  We're not saving these since we don't really know what the hell they do ;-)

  pfm_global.iho = 0;
  pfm_global.capture = 5.0;
  pfm_global.queue = 11;
  pfm_global.horiz = 0.0;
  pfm_global.distance = 2.0;
  pfm_global.min_context = 0.0;
  pfm_global.max_context = 5.0;
  pfm_global.std2conf = 1;
  pfm_global.disambiguation = 0;
  pfm_global.appending = NVFalse;
  pfm_global.attribute_count = 0;
  pfm_global.max_files = NINT (pow (2.0, (NV_FLOAT64) FILE_BITS)) - 1;
  pfm_global.max_lines = NINT (pow (2.0, (NV_FLOAT64) LINE_BITS)) - 1;
  pfm_global.max_pings = NINT (pow (2.0, (NV_FLOAT64) PING_BITS)) - 1;
  pfm_global.max_beams = NINT (pow (2.0, (NV_FLOAT64) BEAM_BITS)) - 1;
  pfm_global.cache_mem = 400000000;

  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++) pfm_global.gsf_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++) pfm_global.hof_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++) pfm_global.tof_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++) pfm_global.wlf_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++) pfm_global.czmil_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++) pfm_global.bag_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++) pfm_global.hawkeye_attribute_num[i] = 0;

  flags.chk = NVTrue;
  flags.old = NVFalse;
  flags.hof = NVFalse;
  flags.sub = NVFalse;
  flags.lnd = NVFalse;
  flags.tof = NVFalse;
  flags.lid = NVTrue;
  flags.nom = NVFalse;
  flags.sec = NVFalse;
  flags.srtmb = NVFalse;
  flags.srtm1 = NVFalse;
  flags.srtm3 = NVFalse;
  flags.srtm30 = NVFalse;
  flags.srtmr = NVFalse;
  flags.srtme = NVTrue;
  flags.attr = NVTrue;
  flags.cube = NVFalse;
  flags.ref = NVFalse;

  ref_def.mbin_size = 2.0;
  ref_def.gbin_size = 0.0;
  ref_def.min_depth = -500.0;
  ref_def.max_depth = 1000.0;
  ref_def.precision = 0.01;
  ref_def.apply_area_filter = NVFalse;
  ref_def.deep_filter_only = NVTrue;
  ref_def.cellstd = 2.4;
  ref_def.radius = 20.0;


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

  countAllAttributes (&pfm_global);


  for (NV_INT32 i = 0 ; i < MAX_LOAD_FILES ; i++)
    {
      pfm_def[i].name = "";
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
      pfm_def[i].rebuild_flag = NVFalse;
      memset (&pfm_def[i].open_args, 0, sizeof (PFM_OPEN_ARGS));
    }


  input_files.clear ();

  pfm_file_count = 0;
  QString parameter_file = QString (argv[1]);
  if (!readParameterFile (parameter_file, &input_files, pfm_def, &pfm_global, &flags)) usage ();


  set_cache_size_max (pfm_global.cache_mem);


  QStringList sort_files;

  for (NV_INT32 i = 0 ; i < input_files.size () ; i++) sort_files += input_files.at (i);


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


  NV_CHAR string[1024];
  struct stat fstat;


  /*  Code to check your input file list.  Only for debugging.

  for (NV_INT32 i = 0 ; i < input_files.size () ; i++)
    {
      strcpy (string, input_files.at (i).toAscii ());
      fprintf (stderr, "%s\n", string);
    }
  exit (-1);

  */


#ifdef NVLinux
  if (flags.sub) sub_in ();
#endif


  input_file_count = input_files.size ();

  input_file_def = new FILE_DEFINITION[input_file_count];


  //  Pre-check the input files.  If we have a problem we want out now.

  for (NV_INT32 j = 0 ; j < input_file_count ; j++)
    {
      input_file_def[j].name = input_files.at (j);

      NV_CHAR name[1024], short_name[512];
      strcpy (name, input_file_def[j].name.toAscii ());
      strcpy (short_name, QFileInfo (input_file_def[j].name).fileName ().toAscii ());


      input_file_def[j].type = get_file_type (name);

      input_file_def[j].status = NVTrue;


      //  If the input parameter file was a .upr file we want to automatically exclude any files in the 
      //  **  Update Parameter File Input Files  ** section.  There is no point in checking them since
      //  they have already been beaten against the PFM files.

      if (pfm_global.upr_flag)
        {
          for (NV_INT32 k = 0 ; k < pfm_global.upr_input_files.size () ; k++)
            {
              if (input_file_def[j].name == pfm_global.upr_input_files.at (k)) input_file_def[j].status = NVFalse;
            }
        }

      if (flags.chk)
        {
          switch (input_file_def[j].type)
            {

              //  File that has been added from a PFM list and was marked as "deleted".

            case -1:
              input_file_def[j].status = NVFalse;
              break;


              //  Unable to open the file.

            case -2:
              fprintf (stderr, tr ("Unable to open file %s\nReason %s\n").toAscii (), name, strerror (errno));
              fflush (stderr);
              input_file_def[j].status = NVFalse;
              break;

            case PFM_UNDEFINED_DATA: 
              fprintf (stderr, tr ("Unable to determine file type for %s\n").toAscii (), name);
              fflush (stderr);
              input_file_def[j].status = NVFalse;
              break;

            case PFM_GSF_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_gsf_file (name);
              break;

            case PFM_WLF_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_wlf_file (name);
              break;

            case PFM_HAWKEYE_HYDRO_DATA:
            case PFM_HAWKEYE_TOPO_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_hawkeye_file (name);
              break;

            case PFM_CHARTS_HOF_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_hof_file (name);
              break;

            case PFM_SHOALS_TOF_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_tof_file (name);
              break;

            case PFM_UNISIPS_DEPTH_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_unisips_depth_file (name);
              break;

            case PFM_NAVO_ASCII_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_hypack_xyz_file (name);
              break;

            case PFM_NAVO_LLZ_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_llz_file (name);
              break;

            case PFM_CZMIL_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_czmil_file (name);
              break;

            case PFM_BAG_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_bag_file (name);
              break;

            case PFM_ASCXYZ_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_ivs_xyz_file (name);
              break;

            case PFM_DTED_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_dted_file (name);
              break;

            case PFM_CHRTR_DATA:
              if (input_file_def[j].status) input_file_def[j].status = check_chrtr_file (name);
              break;
            }

          fprintf (stderr, "Checking file %d of %d - %s                                   \n", j + 1, input_file_count, short_name);
          fflush (stderr);
        }
    }

  if (flags.chk)
    {
      fprintf (stderr, "Preliminary file check complete                                                                                                   \n\n");
      fflush (stderr);
    }


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


  /*
      Check all input files against input files in any preexisting PFM structures so we don't double load.

      IMPORTANT NOTE:  If we're using a .upr (update parameter file) then any file found in ANY PFM has already
      been beaten against ALL of the PFMs in the parameter file.  We don't want to keep checking and re-checking
      all of the files in a repeating load from one or more directories.  The files that have been loaded are 
      saved to the .upr file and checked against the new input files in the prliminary check section above.
  */

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
                      if (!input_file_def[j].status && !strcmp (rfile, input_file_def[j].name.toAscii ())) input_file_def[j].status = NVFalse;
                    }

                  k++;
                }
              close_pfm_file (temp_handle);
            }
        }
    }


  //  Check to see if we have any files left after all of the file checks above.

  NV_BOOL hit = NVFalse;

  for (NV_INT32 i = 0 ; i < input_file_count ; i++)
    {
      if (input_file_def[i].status)
        {
          hit = NVTrue;
          break;
        }
    }

  if (!hit)
    {
      fprintf (stderr,"\n\nNo input files need to be loaded.  Terminating.\n\n");
      fflush (stderr);
      exit (0);
    }


  //  If we are using a .upr parameter file, update the .upr file with the final file list.

  if (pfm_global.upr_flag)
    {
      if (!update_upr_file (parameter_file, input_file_def, input_file_count))
        {
          fprintf (stderr, "\nUnable to update the parameter file!\n\n");
          fflush (stderr);
          exit (-1);
        }
    }


  PFM_LOAD_PARAMETERS load_parms[MAX_LOAD_FILES];


  //  Put everything into load_parms so we can pass it easily.

  for (NV_INT32 i = 0 ; i < pfm_file_count ; i++) 
    {
      load_parms[i].flags = flags;
      load_parms[i].pfm_global = pfm_global;
    }


  //  Open the PFM files.

  for (NV_INT32 i = 0 ; i < pfm_file_count ; i++)
    {
      NV_CHAR name[1024], short_name[512];
      strcpy (name, pfm_def[i].name.toAscii ());
      strcpy (short_name, QFileInfo (pfm_def[i].name).fileName ().toAscii ());


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

          fprintf (stderr, "Initializing PFM structure %d of %d - %s                                                   \n", i + 1, pfm_file_count, short_name);
          fflush (stderr);

          get_version (pfm_def[i].open_args.head.version);
          strcpy (pfm_def[i].open_args.head.classification, "UNCLASSIFIED");

          time_t systemtime = time (&systemtime);
          strcpy (pfm_def[i].open_args.head.date, asctime (localtime (&systemtime)));

          pfm_def[i].open_args.head.date[strlen (pfm_def[i].open_args.head.date) - 1] = 0;


          //  Make the MBR from the input polygon points.

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
          fprintf (stderr, "Creating PFM checkpoint file %d of %d - %s", i + 1, pfm_file_count, short_name);
          fflush (stderr);
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
          //  Using 10% of max depth as maximum vertical error value except when the max depth is less than 150, then we use 15.0.

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

              invgp (NV_A0, NV_B0, pfm_def[i].open_args.head.mbr.min_y, pfm_def[i].open_args.head.mbr.min_x, pfm_def[i].open_args.head.mbr.min_y +
                     (pfm_def[i].gbin_size / 60.0), pfm_def[i].open_args.head.mbr.min_x, &dist, &az);
              pfm_def[i].open_args.head.max_horizontal_error = dist * 10.0;
            }


          //  Horizontal and vertical error scales set to centimeters.

          pfm_def[i].open_args.head.horizontal_error_scale = 100.0;
          pfm_def[i].open_args.head.vertical_error_scale = 100.0;
        }


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
      if (!pfm_def[i].open_args.head.proj_data.projection && pfm_def[i].open_args.head.mbr.max_x > 180.0) pfm_def[i].dateline = NVTrue;


      //  Allocate and clear memory for the bin add_map.

      pfm_def[i].add_map = (NV_BYTE *) calloc (pfm_def[i].open_args.head.bin_width * pfm_def[i].open_args.head.bin_height, sizeof (NV_BYTE));

      if (pfm_def[i].add_map == NULL)
        {
          perror ("Unable to allocate memory for bin_map.");
          exit (-1);
        }
    }

  fprintf (stderr, "PFM structure initialization complete\n\n");
  fflush (stderr);


  //  Zero sounding count 

  NV_INT32 count[MAX_PFM_FILES], out_of_range[MAX_PFM_FILES];

  memset (count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  memset (out_of_range, 0, MAX_PFM_FILES * sizeof (NV_INT32));


  NV_CHAR tmp_char[512];
  NV_INT32 total_input_count = 0;
  QString tmp;


  //  Save the time to the handle file as a comment

  for (NV_INT32 i = 0 ; i < pfm_file_count ; i++)
    {
      FILE *fp = fopen (pfm_def[i].open_args.list_path, "a");

      QDateTime current_time = QDateTime::currentDateTime ();

      strcpy (tmp_char, current_time.toString ().toAscii ());
      fprintf (fp, tr ("#\n#\n# Date : %s  (GMT)\n#\n").toAscii (), tmp_char);
      fclose (fp);
    }


  total_input_count = 0;
  for (NV_INT32 i = 0 ; i < input_file_count ; i++)
    {
      //  Make sure that the file status was good.

      if (input_file_def[i].status)
        {
          NV_CHAR name[1024], short_name[512];
          strcpy (name, input_file_def[i].name.toAscii ());
          strcpy (short_name, QFileInfo (input_file_def[i].name).fileName ().toAscii ());

          fprintf (stderr, "Reading file %d of %d - %s                                         \n", i + 1, input_file_count, short_name);
          fflush (stderr);


          //  Note: When adding other file types to process, add them here, the processing within the call is 
          //  resposible for opening the file, parsing it, navigating it and calling Do_PFM_Processing with the 
          //  appropriate arguments.  We can use keywords within the filename string to determine the file type
          //  or open and look for a header. 

          switch (input_file_def[i].type)
            {
            case PFM_GSF_DATA:
              out_of_range[i] += load_gsf_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_WLF_DATA:
              out_of_range[i] += load_wlf_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_HAWKEYE_HYDRO_DATA:
            case PFM_HAWKEYE_TOPO_DATA:
              out_of_range[i] += load_hawkeye_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_CHARTS_HOF_DATA:
              out_of_range[i] += load_hof_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_SHOALS_TOF_DATA:
              out_of_range[i] += load_tof_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_UNISIPS_DEPTH_DATA:
              out_of_range[i] += load_unisips_depth_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_NAVO_ASCII_DATA:
              out_of_range[i] += load_hypack_xyz_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_NAVO_LLZ_DATA:
              out_of_range[i] += load_llz_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_CZMIL_DATA:
              out_of_range[i] += load_czmil_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_BAG_DATA:
              out_of_range[i] += load_bag_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_ASCXYZ_DATA:
              out_of_range[i] += load_ivs_xyz_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_DTED_DATA:
              out_of_range[i] += load_dted_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
              break;

            case PFM_CHRTR_DATA:
              out_of_range[i] += load_chrtr_file (pfm_file_count, count, input_file_def[i].name, load_parms, pfm_def);
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

          fprintf (stderr, "Total data points read = %d for file %s                                         \n", total_input_count, short_name);
          fflush (stderr);
        }
    }


  //  Just in case we only loaded SRTM data.

  if (input_file_count)
    {
      fprintf (stderr, "Reading input files complete\n\n");
      fflush (stderr);
    }


  //  If we are loading SRTM data, call the load function.

  if (load_parms[0].flags.srtmb || load_parms[0].flags.srtm1 || load_parms[0].flags.srtm3 || load_parms[0].flags.srtm30)
    {
      fprintf (stderr, "Reading SRTM data\n\n");
      fflush (stderr);


      load_srtm_file (pfm_file_count, count, load_parms, pfm_def);


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
          fprintf (stderr, "\n\nBest available resolution SRTM data\n");
        }
      else if (load_parms[0].flags.srtm1)
        {
          fprintf (stderr, "\n\n1 second resolution SRTM data\n");
        }
      else if (load_parms[0].flags.srtm3)
        {
          fprintf (stderr, "\n\n3 second resolution SRTM data\n");
        }
      else if (load_parms[0].flags.srtm30)
        {
          fprintf (stderr, "\n\n30 second resolution SRTM data\n");
        }

      fprintf (stderr, "Total data points read = %d\n", total_input_count);
      fflush (stderr);
    }
  else
    {
      fprintf (stderr, "No SRTM data loaded\n");
      fflush (stderr);
    }


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


  for (NV_INT32 k = 0 ; k < pfm_file_count ; k++)
    {
      NV_CHAR name[1024], short_name[512];
      strcpy (name, pfm_def[k].name.toAscii ());
      strcpy (short_name, QFileInfo (pfm_def[k].name).fileName ().toAscii ());


      //  Close and reopen the PFM files so that we can take advantage of the head and tail pointers in the bin records.

      close_cached_pfm_file (pfm_def[k].hnd);
      //close_pfm_file (pfm_def[k].hnd);
      pfm_def[k].open_args.checkpoint = 0;

      if ((pfm_def[k].hnd = open_existing_pfm_file (&pfm_def[k].open_args)) < 0)
        {
          fprintf (stderr, "An error occurred while trying to recompute the bin surfaces for file %s\n", pfm_def[k].open_args.list_path);
          fprintf (stderr, "The error message was : %s\n", pfm_error_str (pfm_error));
          fprintf (stderr, "Please try running pfm_recompute on the file to correct the problem.\n");
          fflush (stderr);

          pfm_error_exit (pfm_error);
        }


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

          fprintf (stderr, "Filtering bins in PFM %d of %d - %s\n", k + 1, pfm_file_count, short_name);
          fflush (stderr);
        }
      else
        {
          fprintf (stderr, "Recomputing bins in PFM %d of %d - %s\n", k + 1, pfm_file_count, short_name);
          fflush (stderr);
        }


      NV_I32_COORD2 coord;
      BIN_RECORD bin_record;
      NV_BOOL clear_features (NV_INT32 pfm_handle, NV_I32_COORD2 coord, NV_CHAR *lst, NV_FLOAT64 feature_radius);


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
                      AreaFilter (&coord, &bin_record, &pfm_def[k], bin_diagonal);
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
            }


          percent = (NV_INT32) (round (((NV_FLOAT32) (i * pfm_def[k].open_args.head.bin_width) / (NV_FLOAT32) total_bins) * 100.0));

          if (percent != old_percent)
            {
              fprintf (stderr, "%03d%% processed\r", percent);
              fflush (stderr);
              old_percent = percent;
            }
        }


      if (pfm_def[k].apply_area_filter)
        {
          fprintf (stderr, "Filtering bins - complete\n\n");
        }
      else
        {
          fprintf (stderr, "Recomputing bins - complete\n\n");
        }


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
          fprintf (stderr, "\n\nUnable to create backup list file %s.\n\n", string);
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

              fprintf (stderr, "# --------------------------------------\n");
              fprintf (stderr, tr ("# PFM Filter Statistics\n").toAscii ());
              fprintf (stderr, tr ("# Total number of points processed %d\n").toAscii (), summary.TotalSoundings);
              fprintf (stderr, tr ("# Number of soundings Good     : %d\n").toAscii (), summary.GoodSoundings);
              fprintf (stderr, tr ("# Percent of soundings Good     : %f\n").toAscii (), good_ratio);
              fprintf (stderr, tr ("# Number of soundings Filtered : %d\n").toAscii (), summary.BadSoundings);
              fprintf (stderr, tr ("# Percent of soundings Filtered : %f\n").toAscii (), bad_ratio);
              fprintf (stderr, "# --------------------------------------\n");
            }
        }
      else
        {
          fseek (fp, 0, SEEK_END);
          fprintf (fp, "# --------------------------------------\n");
          fprintf (fp, tr ("# Finished with recompute section\n").toAscii ());
          fprintf (fp, "# --------------------------------------\n");

          fprintf (stderr, "# --------------------------------------\n");
          fprintf (stderr, tr ("# Finished with recompute section\n").toAscii ());
          fprintf (stderr, "# --------------------------------------\n");
        }


      if (out_of_range[k] > 5000) 
        {
          fprintf (stderr, tr ("\n**** WARNING ****\n").toAscii ());
          fprintf (stderr, tr ("File %s :\n").toAscii (), pfm_def[k].open_args.list_path);
          fprintf (stderr, tr ("%d soundings out of min/max range!\n").toAscii (), out_of_range[k]);
          fprintf (stderr, tr ("These will be marked invalid in the input files\n").toAscii ());
          fprintf (stderr, tr ("if you unload this file!\n").toAscii ());
          fprintf (stderr, "*****************\n");

          fprintf (fp, tr ("\n**** WARNING ****\n").toAscii ());
          fprintf (fp, tr ("File %s :\n").toAscii (), pfm_def[k].open_args.list_path);
          fprintf (fp, tr ("%d soundings out of min/max range!\n").toAscii (), out_of_range[k]);
          fprintf (fp, tr ("These will be marked invalid in the input files\n").toAscii ());
          fprintf (fp, tr ("if you unload this file!\n").toAscii ());
          fprintf (fp, "*****************\n");
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


          connect (cubeProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotCubeDone (int, QProcess::ExitStatus)));
          connect (cubeProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotCubeReadyReadStandardError ()));
          connect (cubeProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotCubeReadyReadStandardOutput ()));
          connect (cubeProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotCubeError (QProcess::ProcessError)));

          cubeProc->start ("navo_pfm_cube", arguments);
        }
    }
}



pfmLoader::~pfmLoader ()
{
}



void 
pfmLoader::slotCubeReadyReadStandardError ()
{
  static QString resp_string = "";


  QByteArray response = cubeProc->readAllStandardError ();


  //  Parse the return response for carriage returns and line feeds

  for (NV_INT32 i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n' || response.at (i) == '\r')
        {
	  NV_CHAR resp[512];
	  strcpy (resp, resp_string.toAscii ());
	  fprintf (stderr, "%s", resp);

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



void 
pfmLoader::slotCubeReadyReadStandardOutput ()
{
  static QString resp_string = "";
  static NV_INT32 percent = 0, old_percent = -1;


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

	      sscanf (resp, "%d", &percent);
              if (percent != old_percent)
                {
                  fprintf (stderr, "%03d%% processed\r", percent);
                  fflush (stderr);

                  old_percent = percent;
                }
	    }
          else if (resp_string.contains ("Processing Entire PFM"))
	    {
              fprintf (stderr, "PFM CUBE processing (pass 1 of 2)\n");
	      fflush (stderr);
	    }
          else if (resp_string.contains ("Extract Best Hypothesis"))
	    {
	      fprintf (stderr, "Extracting best CUBE hypotheses (pass 2 of 2)\n");
	      fflush (stderr);
	    }
          else
            {
              if (resp_string.length () >= 3)
                {
                  NV_CHAR resp[512];
                  strcpy (resp, resp_string.toAscii ());

                  fprintf (stderr, "%s\n", resp);
                  fflush (stderr);
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
pfmLoader::slotCubeError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      fprintf (stderr, "Unable to start the CUBE process!\n");
      break;

    case QProcess::Crashed:
      fprintf (stderr, "The CUBE process crashed!\n");
      break;

    case QProcess::Timedout:
      fprintf (stderr, "The CUBE process timed out!\n");
      break;

    case QProcess::WriteError:
      fprintf (stderr, "There was a write error from the CUBE process!\n");
      break;

    case QProcess::ReadError:
      fprintf (stderr, "There was a read error from the CUBE process!\n");
      break;

    case QProcess::UnknownError:
      fprintf (stderr, "The CUBE process died with an unknown error!\n");
      break;
    }
  fflush (stderr);
}



void 
pfmLoader::slotCubeDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  fprintf (stderr, "PFM CUBE processing complete\n\n");
  fflush (stderr);
}
