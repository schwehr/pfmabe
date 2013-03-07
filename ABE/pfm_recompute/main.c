#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "nvutility.h"

#include "pfm.h"

#include "version.h"


/*****************************************************************************

    Program:    pfm_recompute

    Purpose:    Recomputes all of the bin layer values using the PFM_USER flags

    Programmer: Jan C. Depner

    Date:       02/13/00

*****************************************************************************/


void usage ()
{
    fprintf (stderr, "\nUsage: pfm_recompute <PFM_HANDLE_FILE or PFM_LIST_FILE> [-1|-2|-3|-4|-c|-f -z]\n");
    fprintf (stderr, "\nWhere:\n\n");
    fprintf (stderr, "\t-1, -2, -3, -4, -c, and -f are mutually exclusive:\n\n");
    fprintf (stderr, "\t-1  =  recompute surfaces using only PFM_USER_01 data\n");
    fprintf (stderr, "\t-2  =  recompute surfaces using only PFM_USER_02 data\n");
    fprintf (stderr, "\t-3  =  recompute surfaces using only PFM_USER_03 data\n");
    fprintf (stderr, "\t-4  =  recompute surfaces using only PFM_USER_04 data\n");
    fprintf (stderr, "\t-c  =  clear all edited and selected flags\n");
    fprintf (stderr, "\t-f  =  clear all filter edited and selected flags\n\n\n");
    fprintf (stderr, "\t-z requires a following value:\n\n");
    fprintf (stderr, "\t-z  =  limit the Z range of the data to be cleared\n");
    fprintf (stderr, "\t\tThis option is meaningless without -c or -f\n");
    fprintf (stderr, "\t\tIf this is not specified no limit is placed on -c or -f\n");
    fprintf (stderr, "\t\tExample: pfm_recompute fred.pfm -c -z 150.0,2000.0\n\n");
    fflush (stderr);
}



NV_INT32 main (NV_INT32 argc, char **argv)
{
    NV_INT32            i, j, m, percent = 0, old_percent = -1, pfm_handle, 
                        flag, recnum;
    PFM_OPEN_ARGS       open_args;
    NV_FLOAT32          total, null_depth, minz, maxz;
    BIN_RECORD          bin_record;
    DEPTH_RECORD        *depth;
    NV_I32_COORD2       coord;
    NV_BOOL             clear, filter, limit;
    NV_CHAR             c;
    extern char         *optarg;
    extern int          optind;



    fprintf (stderr, "\n\n%s\n\n", VERSION);
    fflush (stderr);


    minz = -999999.0;
    maxz = 999999.0;
    flag = 0;
    clear = NVFalse;
    filter = NVFalse;
    limit = NVFalse;


    while ((c = getopt (argc, argv, "1234cfz:")) != EOF)
      {
	switch (c)
          {
          case '1':
          case '2':
          case '3':
          case '4':
            clear = NVFalse;
            sscanf (optarg, "%d", &flag);
            break;

          case 'c':
            flag = 0;
            clear = NVTrue;
            break;

          case 'f':
            flag = 0;
            clear = NVTrue;
            filter = NVTrue;
            break;

          case 'z':
            limit = NVTrue;
            sscanf (optarg, "%f,%f", &minz, &maxz);
            break;

          default:
            usage ();
            exit (-1);
            break;
          }
      }


    /* Make sure we got the mandatory file name argument.  */

    if (optind >= argc)
      {
        usage ();
        exit (-1);
      }


    strcpy (open_args.list_path, argv[optind]);

    open_args.checkpoint = 2;

    pfm_handle = open_existing_pfm_file (&open_args);


    /*  Set the flag type so that recomputes will be done with only the
        required user flag.  */

    open_args.head.class_type = flag;
    write_bin_header (pfm_handle, &open_args.head, NVFalse);


    null_depth = open_args.max_depth + open_args.offset + 1.0;


    if (pfm_handle < 0) pfm_error_exit (pfm_error);


    total = open_args.head.bin_height;

    fprintf (stderr, "File : %s\n\n", open_args.list_path);
    fflush (stderr);


    for (i = 0; i < open_args.head.bin_height; i++)
    {
        coord.y = i;

        for (j = 0; j < open_args.head.bin_width; j++)
        {
            coord.x = j;

            if (clear)
            {
                if (!read_depth_array_index (pfm_handle, coord, &depth, 
                    &recnum))
                {
                    for (m = 0 ; m < recnum ; m++)
                    {
                        if (depth[m].validity & (PFM_INVAL | PFM_SELECTED))
                        {
                            if (!limit || (depth[m].xyz.z >= minz &&
                                           depth[m].xyz.z <= maxz))
                            {
                                if (filter)
                                {
                                    depth[m].validity &= 
                                      ~(PFM_FILTER_INVAL | PFM_SELECTED);
                                }
                                else
                                {
                                    depth[m].validity &= 
                                      ~(PFM_INVAL | PFM_SELECTED);
                                }
                                depth[m].validity |= PFM_MODIFIED;

                                update_depth_record_index (pfm_handle, 
                                                           &depth[m]);
                            }
                        }
                    }

                    free (depth);

                    bin_record.validity &= ~PFM_CHECKED;
                    recompute_bin_values_index (pfm_handle, coord, &bin_record,
                                                PFM_CHECKED);
                }
            }
            else
            {
                recompute_bin_values_index (pfm_handle, coord, &bin_record, 0);
            }


            if (bin_record.num_soundings)
            {
                if (bin_record.num_soundings < open_args.head.min_bin_count)
                {
                    open_args.head.min_bin_count = bin_record.num_soundings;
                    open_args.head.min_count_coord = bin_record.coord;
                }

                if (bin_record.num_soundings > open_args.head.max_bin_count)
                {
                    open_args.head.max_bin_count = bin_record.num_soundings;
                    open_args.head.max_count_coord = bin_record.coord;
                }


                if (bin_record.max_depth < null_depth)
                {
                    if (bin_record.validity & PFM_DATA)
                    {
                        if (bin_record.min_filtered_depth <
                            open_args.head.min_filtered_depth)
                        {
                            open_args.head.min_filtered_depth =
                                bin_record.min_filtered_depth;
                            open_args.head.min_filtered_coord =
                                bin_record.coord;
                        }
                    
                        if (bin_record.max_filtered_depth >
                            open_args.head.max_filtered_depth)
                        {
                            open_args.head.max_filtered_depth =
                                bin_record.max_filtered_depth;
                            open_args.head.max_filtered_coord =
                                bin_record.coord;
                        }

                        if (bin_record.standard_dev <
                            open_args.head.min_standard_dev)
                        {
                            open_args.head.min_standard_dev =
                                bin_record.standard_dev;
                        }

                        if (bin_record.standard_dev >
                            open_args.head.max_standard_dev)
                        {
                            open_args.head.max_standard_dev =
                                bin_record.standard_dev;
                        }
                    }

                    if (bin_record.min_depth < open_args.head.min_depth)
                    {
                        open_args.head.min_depth = bin_record.min_depth;
                        open_args.head.min_coord = bin_record.coord;
                    }

                    if (bin_record.max_depth > open_args.head.max_depth)
                    {
                        open_args.head.max_depth = bin_record.max_depth;
                        open_args.head.max_coord = bin_record.coord;
                    }
                }
            }
        }

        percent = (i / total) * 100.0;
        if (percent != old_percent)
        {
            old_percent = percent;
            fprintf (stderr, "Recomputing bins : %03d%%          \r", percent);
            fflush (stderr);
        }
    }

    fprintf (stderr, "100%% bins computed                       \n\n");
    fflush (stderr);


    write_bin_header (pfm_handle, &open_args.head, NVFalse);


    close_pfm_file (pfm_handle);


    return (0);
}
