#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "nvutility.h"

#include "output.h"
#include "waveforms.h"
#include "file_globals.h"

#include "hmpsflag.h"


void swap_int (int *);
void swap_float (float *);
void swap_double (double *);
void swap_short (short *);


/***************************************************************************\
*                                                                           *
*   Module Name:        open_files                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Open the SHOALS out and wave files.                 *
*                                                                           *
*   Arguments:          filename   -   SHOALS .out file name                *
*                       ofp        -   .out file pointer                    *
*                       wfp        -   .wave file pointer                   *
*                       nwfp       -   new .wave file pointer               *
*                                                                           *
*   Return Value:       0 - success, else failure                           *
*                                                                           *
\***************************************************************************/

NV_INT32 open_files (NV_CHAR *filename, FILE **ofp, FILE **wfp, FILE ** nwfp)
{
    NV_CHAR               *cutoff, path[512], string[1024], db_name[40];
    NV_INT32              flightline;
    

    /*  Dealing with the possibility of mixed separators in MSYS on Windoze.  */

    strcpy (string, filename);
    if (strrchr (string, '/'))
      {
        cutoff = strrchr (string, '/');
      }
    else if (strrchr (string, '\\'))
      {
        cutoff = strrchr (string, '\\');
      }
    else
      {
        cutoff = NULL;
      }

    if (cutoff != NULL)
    {
        strcpy (path, string);
        path[(cutoff - string)] = 0;      
        strcpy (db_name, (cutoff + 1));
    }
    else
    {
        strcpy (path, ".");
        strcpy (db_name, string);
    }


    if (strchr (db_name, '_') == NULL)
    {
        fprintf (stderr, "No underscore (_) in SHOALS .out filename\n");
        fflush (stderr);
        return (1);
    }


    cutoff = strchr (db_name, '_');

    sscanf (cutoff, "_%d", &flightline);

    strcpy (string, db_name);
    string[cutoff - db_name] = 0;
    strcpy (db_name, string);


    sprintf (string, "GB_DATA_DIR=%s", path);
    putenv (string);


    /*  Open the files.  */

    if ((*ofp = file_open_output (db_name, flightline)) == NULL) return (2);

    if ((*wfp = file_open_wave (db_name, flightline)) == NULL) return (3);



    sprintf (string, "%s%1c%s_%d.wavc", path, (NV_CHAR) SEPARATOR, db_name, flightline);

    if ((*nwfp = fopen (string, "w")) == NULL) return (4);

      
    return (0);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        open_out_file                                       *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Open the SHOALS out file.                           *
*                                                                           *
*   Arguments:          filename   -   SHOALS .out file name                *
*                       ofp        -   .out file pointer                    *
*                                                                           *
*   Return Value:       0 - success, else failure                           *
*                                                                           *
\***************************************************************************/

NV_INT32 open_out_file (NV_CHAR *filename, FILE **ofp)
{
    NV_CHAR               *cutoff, path[512], string[1024], db_name[40];
    NV_INT32              flightline;
    

    /*  Dealing with the possibility of mixed separators in MSYS on Windoze.  */

    strcpy (string, filename);
    if (strrchr (string, '/'))
      {
        cutoff = strrchr (string, '/');
      }
    else if (strrchr (string, '\\'))
      {
        cutoff = strrchr (string, '\\');
      }
    else
      {
        cutoff = NULL;
      }

    if (cutoff != NULL)
    {
        strcpy (path, string);
        path[(cutoff - string)] = 0;
        strcpy (db_name, (cutoff + 1));
    }
    else
    {
        strcpy (path, ".");
        strcpy (db_name, string);
    }


    if (strchr (db_name, '_') == NULL)
    {
        fprintf (stderr, "No underscore (_) in SHOALS .out filename\n");
        fflush (stderr);
        return (1);
    }


    cutoff = strchr (db_name, '_');

    sscanf (cutoff, "_%d", &flightline);

    strcpy (string, db_name);
    string[cutoff - db_name] = 0;
    strcpy (db_name, string);


    sprintf (string, "GB_DATA_DIR=%s", path);
    putenv (string);


    /*  Open the files.  */

    if ((*ofp = file_open_output (db_name, flightline)) == NULL) return (2);


    return (0);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        swap_output                                         *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Byte swap the SHOALS OUTPUT_FILE structure.         *
*                                                                           *
*   Arguments:          output_file   -   address of OUTPUT_FILE structure  *
*                                                                           *
\***************************************************************************/

void swap_output (OUTPUT_FILE_T *output_file)
{
    swap_int (&output_file->au.timestamp);
    swap_short (&output_file->au.flightline);
    swap_short (&output_file->au.haps_version);
    swap_double (&output_file->au.pulse_time);
    swap_double (&output_file->au.latitude);
    swap_double (&output_file->au.longitude);
    swap_double (&output_file->au.sec_latitude);
    swap_double (&output_file->au.sec_longitude);
    swap_float (&output_file->au.tide_cor_depth);
    swap_float (&output_file->au.reported_depth);
    swap_float (&output_file->au.altitude);
    swap_float (&output_file->au.elevation);
    swap_float (&output_file->au.otf_datum);
    swap_float (&output_file->au.otf_elevation);
    swap_float (&output_file->au.otf_res_elev);
    swap_float (&output_file->au.otf_sec_elev);
    swap_float (&output_file->au.otf_topo);
    swap_float (&output_file->au.otf_water_level);
    swap_float (&output_file->au.result_depth);
    swap_float (&output_file->au.sec_depth);
    swap_float (&output_file->au.topo);
    swap_float (&output_file->au.wave_height);
    swap_int (&output_file->au.depth_conf);
    swap_int (&output_file->au.sec_depth_conf);
    swap_short (&output_file->au.ab_dep_conf);
    swap_short (&output_file->au.sec_ab_dep_conf);
    swap_short (&output_file->au.otf_abd_conf);
    swap_short (&output_file->au.otf_sec_abd_conf);
    swap_short (&output_file->au.position_conf);
    swap_short (&output_file->au.status);

    swap_float (&output_file->early.bot_conf);
    swap_float (&output_file->early.sec_bot_conf);	 
    swap_float (&output_file->early.nadir_angle);		
    swap_float (&output_file->early.scanner_azimuth);
    swap_float (&output_file->early.sfc_fom_apd);	 
    swap_float (&output_file->early.sfc_fom_ir);		 
    swap_float (&output_file->early.sfc_fom_ram);	 
    swap_int (&output_file->early.warnings);	
    swap_int (&output_file->early.warnings2);
    swap_short (&output_file->early.bot_bin_first);	 
    swap_short (&output_file->early.bot_bin_second);	 
    swap_short (&output_file->early.bot_bin_used_pmt);	 
    swap_short (&output_file->early.sec_bot_bin_used_pmt);	 
    swap_short (&output_file->early.bot_bin_used_apd);	 
    swap_short (&output_file->early.sec_bot_bin_used_apd);
    swap_short (&output_file->early.bot_channel);		 
    swap_short (&output_file->early.sec_bot_chan);	 
    swap_short (&output_file->early.sfc_bin_apd);	 	
    swap_short (&output_file->early.sfc_bin_ir);		 	
    swap_short (&output_file->early.sfc_bin_ram);	 	
    swap_short (&output_file->early.sfc_channel_used);	 
/*    swap_int (&output_file->early.compressed_wave_address);*/	
}



/***************************************************************************\
*                                                                           *
*   Module Name:        swap_wave                                           *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Byte swap the SHOALS WAVEFORMS structure.           *
*                                                                           *
*   Arguments:          waveforms   -   address of WAVEFORMS structure      *
*                                                                           *
\***************************************************************************/

void swap_wave (WAVEFORMS_T *waveforms)
{
    NV_INT32    i;


    swap_int (&waveforms->time_stamp);

    for (i = 0 ; i < PMT_LEN + 1; i++)
    {
        swap_short (&waveforms->pmt_raw_waveform_ra[i]);
    }

    for (i = 0 ; i < APD_LEN + 1 ; i++)
    {
        swap_short (&waveforms->apd_raw_waveform_ra[i]);
    }

    for (i = 0 ; i < IR_LEN + 1 ; i++)
    {
        swap_short (&waveforms->ir_raw_waveform_ra[i]);
    }

    for (i = 0 ; i < RAMAN_LEN + 1 ; i++)
    {
        swap_short (&waveforms->raman_raw_waveform_ra[i]);
    }
}
