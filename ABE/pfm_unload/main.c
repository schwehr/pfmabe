#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "nvutility.h"

#include "pfm.h"
#include "hmpsflag.h"

#include "llz.h"

#include "dted.h"

#include "version.h"


typedef struct
{
  NV_INT32    file;
  NV_INT32    rec;
  NV_INT32    sub;
  NV_U_INT32  val;
} SORT_REC;


void usage ()
{
  fprintf (stderr, "\nUsage: pfm_unload <PFM_HANDLE_FILE or PFM_LIST_FILE> [-c -u]\n");
  fprintf (stderr, "\nWhere:\n\n");
  fprintf (stderr, "\t-c  =  force preliminary file check\n");
  fprintf (stderr, "\t-u  =  unload ALL data that has EVER been modified (warning - VERY slow)\n\n");
  fflush (stderr);


  /*  The following are still active but undocumented.  */

  /*fprintf (stderr, "\t-a  =  area of file to dump, N,S,E,W (normally only used from pfmView)\n");*/
  /*fprintf (stderr, "\t\tEx. -a 29.0,28.0,-80.0,-79.0\n");*/
  /*fprintf (stderr, "\t-s  =  start file, requires argument (ex. -s 10)\n");*/
}



/*  This is the file/rec sort function for qsort.  */

static NV_INT32 compare_file_and_rec_numbers (const void *a, const void *b)
{
    SORT_REC *sa = (SORT_REC *) (a);
    SORT_REC *sb = (SORT_REC *) (b);


    /*  If the files aren't equal we sort on file.  */

    if (sa->file != sb->file) return (sa->file < sb->file ? 0 : 1);


    /*  Otherwise we sort on the record number.  */

    return (sa->rec < sb->rec ? 0 : 1);
}



NV_INT32 main (NV_INT32 argc, char **argv)
{
  NV_INT32                i, j, k, status = 0, percent = 0, gsf_handle, old_percent = -1, recnum, total_bins, ret, rec,
                          in_count = 0, year, mon, mday, hour, min, sec, filter = 0, manual = 0, width, height,
                          selected = 0;
  NV_INT16                type;
  PFM_OPEN_ARGS           open_args;
  NV_CHAR                 file[512], string[512], comment[512], c, cday[10], cmon[10];
  NV_I32_COORD2           ll, ur, coord;
  NV_F64_COORD2           xy;
  NV_F64_XYMBR            mbr;              
  BIN_RECORD              bin_record;
  DEPTH_RECORD            *depth;
  NV_INT32                pfm_handle, start_file = 0, prev_file;
  NV_BOOL                 gsf = NVFalse, error_on_update = NVFalse, dump_all = NVFalse, check = NVFalse, 
                          partial = NVFalse, old_lidar = NVFalse, Qt = NVFalse;
  SORT_REC                *sort_rec = NULL;
  NV_INT32                sort_count = 0;
  extern char             *optarg;
  extern int              optind;
  NV_CHAR                 month[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
					  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


  NV_INT32 unload_gsf_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *);
  NV_INT32 unload_shoals_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *);
  NV_INT32 unload_wlf_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *, NV_BOOL);
  NV_INT32 unload_hawkeye_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *, NV_BOOL);
  NV_INT32 unload_hof_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *, NV_BOOL, NV_INT16);
  NV_INT32 unload_tof_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *, NV_BOOL);
  NV_INT32 unload_unisips_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *);
  NV_INT32 unload_llz_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *);
  NV_INT32 unload_dted_file (NV_INT32, NV_INT16, NV_INT32, NV_INT16, NV_U_INT32, BIN_HEADER, NV_INT32, char **, NV_CHAR *);
  void check_gsf_file (NV_CHAR *);
  void check_shoals_file (NV_CHAR *);
  void check_wlf_file (NV_CHAR *);
  void check_hawkeye_file (NV_CHAR *);
  void check_hof_file (NV_CHAR *);
  void check_tof_file (NV_CHAR *);
  NV_BOOL check_unisips_file (NV_CHAR *);
  NV_BOOL check_llz_file (NV_CHAR *);
  NV_BOOL check_dted_file (NV_CHAR *);
  void close_last_file ();
  NV_INT32 write_history (NV_INT32, char **, NV_CHAR *, NV_CHAR *, NV_INT32);



  printf ("\n\n %s \n\n\n", VERSION);



  while ((c = getopt (argc, argv, "Qcus:a:")) != EOF)
    {
      switch (c)
        {
	case 'Q':
	  Qt = NVTrue;
	  break;

	case 'c':
	  check = NVTrue;
	  break;

	case 'u':
	  dump_all = NVTrue;
	  break;

	case 's':
	  sscanf (optarg, "%d", &start_file);
	  break;

	case 'a':
	  sscanf (optarg, "%lf,%lf,%lf,%lf", &mbr.max_y, &mbr.min_y, &mbr.max_x, &mbr.min_x);
	  partial = NVTrue;
	  check = NVFalse;
	  break;

	default:
	  usage ();
	  exit (-1);
	  break;
        }
    }


  /*  Make sure we got the mandatory file name argument.  */

  if (optind >= argc)
    {
      usage ();
      exit (-1);
    }


  strcpy (open_args.list_path, argv[optind]);


  printf ("\n\nPFM file : %s\n\n\n",open_args.list_path);


  /*  Open the PFM files.  */

  open_args.checkpoint = 0;
  if ((pfm_handle = open_existing_pfm_file (&open_args)) < 0) pfm_error_exit (pfm_error);


  /*  Check for old lidar data - record number starts at 0 instead of 1.   This
      is to fix a screwup between IVS, Optech, and us.  For some unknown reason
      IVS chose to start numbering at 1 instead of 0.  Go figure.  */

  sscanf (open_args.head.date, "%s %s %d %d:%d:%d %d", cday, cmon, &mday, &hour, &min, &sec, &year);

  mon = 0;
  for (i = 0 ; i < 12 ; i++)
    {
      if (!strcmp (cmon, month[i])) 
	{
	  mon = i;
	  break;
	}
    }

  if (year < 2004 || (year == 2004 && (mon < 9 || (mon == 9 && mday < 30)))) old_lidar = NVTrue;


  if (check)
    {
      /*  Pre-certify all of the files for writing.  */

      k = get_next_list_file_number (pfm_handle);

      for (j = 0 ; j < k ; j++)
        {
	  fprintf(stderr, "Checking input file %4d of %4d\r",j + 1, k);
	  fflush (stderr);

	  if (read_list_file (pfm_handle, j, file, &type)) break;


	  /*  Bypass files marked as PFM_DELETED.  */

	  if (file[0] != '*')
            {
              /*  If the file has /PFMWDB:: as the beginning of the file then we are tying to unload from a PFM World Data Base
                  (PFMWDB) file and we need to strip the /PFMWDB:: off of the file name and hope that it has been placed in the 
                  current directory.  */

              if (!strncmp (file, "/PFMWDB::", 9))
                {
                  strcpy (string, &file[9]);
                  strcpy (file, string);
                }


	      switch (type)
                {
		case PFM_GSF_DATA:
		  check_gsf_file (file);
		  break;

		case PFM_SHOALS_OUT_DATA:
		  check_shoals_file (file);
		  break;

		case PFM_WLF_DATA:
		  check_wlf_file (file);
		  break;

		case PFM_HAWKEYE_HYDRO_DATA:
		case PFM_HAWKEYE_TOPO_DATA:
		  check_hawkeye_file (file);
		  break;

		case PFM_SHOALS_1K_DATA:
		case PFM_CHARTS_HOF_DATA:
		  check_hof_file (file);
		  break;

		case PFM_SHOALS_TOF_DATA:
		  check_tof_file (file);
		  break;

		case PFM_UNISIPS_DEPTH_DATA:
		  check_unisips_file (file);
		  break;

		case PFM_NAVO_LLZ_DATA:
		  check_llz_file (file);
		  break;

		case PFM_DTED_DATA:
		  check_dted_file (file);
		  break;
                }
            }
        }
      fprintf(stderr, "                                        \r");
      fflush (stderr);
    }

  if (partial)
    {
      xy.y = mbr.min_y;
      xy.x = mbr.min_x;

      compute_index_ptr (xy, &ll, &open_args.head);

      if (ll.x < 0) ll.x = 0;
      if (ll.y < 0) ll.y = 0;


      xy.y = mbr.max_y;
      xy.x = mbr.max_x;
        
      compute_index_ptr (xy, &ur, &open_args.head);

      if (ur.x > open_args.head.bin_width) ur.x = open_args.head.bin_width;
      if (ur.y > open_args.head.bin_height) ur.y = open_args.head.bin_height;
    }
  else
    {
      ll.x = 0;
      ll.y = 0;
      ur.x = open_args.head.bin_width;
      ur.y = open_args.head.bin_height;
    }


  width = ur.x - ll.x;
  height = ur.y - ll.y;
  total_bins = width * height;


  fprintf(stderr, "Reading PFM bin and index files.\n\n");
  fflush (stderr);


  /*  First we're going to sort all the modified records by file and line so that we don't thrash the disk when we unload.
      Please note that we don't unset the PFM_MODIFIED flag in the depth records for two reasons.  One, because if something screws up we can
      always use the brute force dump_all method even if someone has reset the bin PFM_MODIFIED flag.  Two, it is nice to keep track of just how
      many records have actually been modified.  The drawback to this is that if you modify a record in a bin that has been previously unloaded you
      will unload all of the PFM_MODIFIED records in the bin again.  I can live with this.  IVS and SAIC may do it differently.  */

  for (i = ll.y ; i < ur.y ; i++)
    {
      coord.y = i;

      for (j = ll.x ; j < ur.x ; j++)
        {
	  coord.x = j;

	  if (read_bin_record_index (pfm_handle, coord, &bin_record))
            {
	      fprintf(stderr,"%d %d %d %d\n",open_args.head.bin_width, open_args.head.bin_height, i, j);
	      fflush (stderr);
            }
	
          if (dump_all || (bin_record.validity & PFM_MODIFIED))
            {
	      if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                {
                  for (k = 0 ; k < recnum ; k++)
                    {
		      /*  Don't unload any data marked as PFM_DELETED or PFM_REFERENCE.  */

		      if (!(depth[k].validity & (PFM_DELETED | PFM_REFERENCE)))
                        {
			  /*  Unload data marked in the depth record as PFM_MODIFIED.  */

			  if (depth[k].validity & PFM_MODIFIED)
                            {
			      /*  Only unload those files whose file numbers are greater than start_file.  */

			      if (!start_file || (depth[k].file_number > start_file))
                                {
				  read_list_file (pfm_handle, depth[k].file_number, file, &type);

				  if (file[0] != '*')
                                    {
                                      if (depth[k].validity & PFM_FILTER_INVAL) filter++;
                                      if (depth[k].validity & PFM_MANUALLY_INVAL) manual++;
                                      if (depth[k].validity & PFM_SELECTED_SOUNDING) selected++;

                                      sort_rec = (SORT_REC *) realloc (sort_rec, (sort_count + 1) * sizeof (SORT_REC));

                                      if (sort_rec == NULL)
                                        {
                                          perror ("Allocating sort_rec in main.c");
                                          exit (-1);
                                        }
                                      sort_rec[sort_count].file = depth[k].file_number;
                                      sort_rec[sort_count].rec = depth[k].ping_number;
                                      sort_rec[sort_count].sub = depth[k].beam_number;
                                      sort_rec[sort_count].val = depth[k].validity;
                                      sort_count++;
                                    }
                                }
                            }
                        }
                    }
		  free (depth);
                }
            }

	  percent = ((NV_FLOAT32) ((i - ll.y) * width + (j - ll.x)) / (NV_FLOAT32) total_bins) * 100.0;
	  if (old_percent != percent)
            {
	      if (Qt)
		{
		  fprintf (stderr, "%d%%\r", percent);
		}
	      else
		{
		  fprintf (stderr, "%03d%% read    \r", percent);
		}
	      fflush (stderr);
	      old_percent = percent;
            }
        }
    }


  /*  Sort the records so we can write to each file in order.  */

  qsort (sort_rec, sort_count, sizeof (SORT_REC), compare_file_and_rec_numbers);


  if (Qt)
    {
      fprintf (stderr, "100%%\r");
    }
  else
    {
      fprintf (stderr, "100%% read         \n");
    }


  fprintf(stderr, "Updating input files.\n\n");
  fflush (stderr);


  prev_file = -999;

  for (i = 0 ; i < sort_count ; i++)
    {
      if (sort_rec[i].file != prev_file)
        {
          read_list_file (pfm_handle, sort_rec[i].file, file, &type);

          prev_file = sort_rec[i].file;


          /*  If the file has /PFMWDB:: as the beginning of the file then we are tying to unload from a PFM World Data Base
              (PFMWDB) file and we need to strip the /PFMWDB:: off of the file name and hope that it has been placed in the 
              current directory.  */

          if (!strncmp (file, "/PFMWDB::", 9))
            {
              strcpy (string, &file[9]);
              strcpy (file, string);
            }
        }

      switch (type)
        {
        case PFM_GSF_DATA:
          status = unload_gsf_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file);
          if (!status) gsf = NVTrue;
          break;

        case PFM_SHOALS_OUT_DATA:
          status = unload_shoals_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file);
          break;

        case PFM_WLF_DATA:
          status = unload_wlf_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file,
                                    old_lidar);
          break;

        case PFM_HAWKEYE_HYDRO_DATA:
        case PFM_HAWKEYE_TOPO_DATA:
          status = unload_hawkeye_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file,
                                        old_lidar);
          break;

        case PFM_SHOALS_1K_DATA:
        case PFM_CHARTS_HOF_DATA:

          /*  New lidar loads/unloads done starting at record 1.  */

          if (old_lidar)
            {
              rec = sort_rec[i].rec + 1;
            }
          else
            {
              rec = sort_rec[i].rec;
            }

          status = unload_hof_file (pfm_handle, sort_rec[i].file, rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file, old_lidar, type);
          break;

        case PFM_SHOALS_TOF_DATA:

          /*  New lidar loads/unloads done starting at record 1.  */

          if (old_lidar)
            {
              rec = sort_rec[i].rec + 1;
            }
          else
            {
              rec = sort_rec[i].rec;
            }

          status = unload_tof_file (pfm_handle, sort_rec[i].file, rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file, old_lidar);
          break;

        case PFM_UNISIPS_DEPTH_DATA:
          status = unload_unisips_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file);
          break;

        case PFM_NAVO_LLZ_DATA:
          status = unload_llz_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file);
          break;

        case PFM_DTED_DATA:
          status = unload_dted_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, open_args.head, argc, argv, file);
          break;
        }

      if (status) error_on_update = NVTrue;

      in_count++;


      percent = ((NV_FLOAT32) i / (NV_FLOAT32) sort_count) * 100.0;
      if (old_percent != percent)
        {
          if (Qt)
            {
              fprintf (stderr, "%d%%\r", percent);
            }
          else
            {
              fprintf (stderr, "%03d%% written    \r", percent);
            }
          fflush (stderr);
          old_percent = percent;
        }
    }


  free (sort_rec);


  close_last_file ();


  if (Qt)
    {
      fprintf (stderr, "100%%\r");
    }
  else
    {
      fprintf (stderr, "100%% written         \n");
    }
  fprintf (stderr, "\n\n%d edited points read\n", in_count);
  fprintf (stderr, "%d filter edited\n", filter);
  fprintf (stderr, "%d manually edited\n", manual);
  fprintf (stderr, "%d selected soundings\n\n\n", selected);
  fflush (stderr);


  /*  If we had any GSF files, flush the last one and close it.  */

  if (gsf)
    {
      if (unload_gsf_file (pfm_handle, -1, -1, -1, 0, open_args.head, argc, argv, NULL) == -1) error_on_update = NVTrue;


      k = get_next_list_file_number (pfm_handle);


      /*  Write history records to GSF files.  */

      if (!partial)
        {
	  fprintf (stderr, "Writing history records to GSF files\n\n");
	  fflush (stderr);

	  for (j = 0 ; j < k ; j++)
            {
	      if (read_list_file (pfm_handle, j, file, &type)) break;


              /*  If the file has /PFMWDB:: as the beginning of the file then we are tying to unload from a PFM World Data Base
                  (PFMWDB) file and we need to strip the /PFMWDB:: off of the file name and hope that it has been placed in the 
                  current directory.  */

              if (!strncmp (file, "/PFMWDB::", 9))
                {
                  strcpy (string, &file[9]);
                  strcpy (file, string);
                }


	      if (type == PFM_GSF_DATA)
                {
		  fprintf (stderr, "%s\n", file);
		  fflush (stderr);

		  /*  Open the file non-indexed so that we can write a history record.  */

		  if (gsfOpen (file, GSF_UPDATE, &gsf_handle))
                    {
		      gsfPrintError (stderr);
		      fflush (stderr);
		      gsfClose (gsf_handle);
                    }
		  else
                    {
		      /*  Write a history record describing the polygon and bin size.  */

		      sprintf (comment, "PFM edited:\nBin size - %fm\nPolygon points - %d\n", open_args.head.bin_size_xy, open_args.head.polygon_count);

		      for (i = 0 ; i < open_args.head.polygon_count ; i++)
                        {
			  sprintf (string, "Point %03d - %f , %f\n", i, open_args.head.polygon[i].y, open_args.head.polygon[i].x);
			  strcat (comment, string);
                        }

		      ret = write_history (argc, argv, comment, file, gsf_handle);
		      if (ret)
                        {
			  fprintf(stderr, "Error: %d - writing gsf history record\n", ret);
			  fflush (stderr);
                        }

		      gsfClose (gsf_handle);
                    }
                }
            }
        }
    }



  if (error_on_update)
    {
      fprintf(stderr, "\n\nError unloading one or more points!\n");
      if (!Qt)
	{
	  fprintf(stderr, "Do you want to reset modified flags anyway [y/n] ? ");
	  ngets (string, sizeof (string), stdin);
	  fprintf(stderr, "\n\n");
	  if (string[0] == 'y' || string[0] == 'Y') error_on_update = NVFalse;
	}
      fflush (stderr);
    }



  if (!error_on_update)
    {
      fprintf(stderr, "Resetting modified flags in PFM bin file.\n\n");
      fflush (stderr);


      /*  Go back through the bin file and unset the modified flag.  This 
	  is not done as it is read in because the write to the input file 
	  might fail.  */

      for (i = ll.y ; i < ur.y ; i++)
        {
	  coord.y = i;

	  for (j = ll.x ; j < ur.x ; j++)
            {
	      coord.x = j;

	      if ((status = read_bin_record_index (pfm_handle, coord, &bin_record))) break;

	      if (bin_record.validity & PFM_MODIFIED)
                {
		  bin_record.validity &= ~PFM_MODIFIED;
		  write_bin_record_index (pfm_handle, &bin_record);
                }

	      percent = ((NV_FLOAT32) ((i - ll.y) * width + (j - ll.x)) / (NV_FLOAT32) total_bins) * 100.0;
	      if (old_percent != percent)
		{
		  if (Qt)
		    {
		      fprintf (stderr, "%d%%\r", percent);
		    }
		  else
		    {
		      fprintf (stderr, "%03d%% processed    \r", percent);
		    }
		  fflush (stderr);
		  old_percent = percent;
		}
            }
        }

      if (Qt)
	{
	  fprintf (stderr,"100%%\r");
	}
      else
	{
	  fprintf (stderr,"100%% processed         \n\n\n");
	}
      fflush (stderr);
    }


  /*  Close the PFM files.  */

  close_pfm_file (pfm_handle);


  return (0);
}
