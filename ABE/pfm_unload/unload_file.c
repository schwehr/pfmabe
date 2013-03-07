#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "FileHydroOutput.h"
#include "FileTopoOutput.h"

#include "nvutility.h"

#include "pfm.h"

#include "gsf.h"
#include "hmpsflag.h"

#include "wlf.h"
#include "hawkeye.h"

#include "llz.h"

#include "dted.h"

#include "output.h"
#include "waveforms.h"
#include "file_globals.h"

#include "unisips_io.h"


static NV_BOOL     sun_data;

static NV_INT32    last_handle = -1;
static FILE        *last_fp = NULL;
static NV_INT32    last_type = -1;



void close_last_file ()
{
  switch (last_type)
    {
    case PFM_GSF_DATA:
      /*  Don't close GSF here.  We need to flush the last buffer.  */
      break;

    case PFM_SHOALS_OUT_DATA:
    case PFM_SHOALS_1K_DATA:
    case PFM_CHARTS_HOF_DATA:
    case PFM_SHOALS_TOF_DATA:
    case PFM_UNISIPS_DEPTH_DATA:
    case PFM_DTED_DATA:
      fclose (last_fp);
      break;

    case PFM_NAVO_LLZ_DATA:
      close_llz (last_handle);
      break;

    case PFM_WLF_DATA:
      wlf_close_file (last_handle);
      break;

    case PFM_HAWKEYE_HYDRO_DATA:
    case PFM_HAWKEYE_TOPO_DATA:
      hawkeye_close_file (last_handle);
      break;
    }
}


void check_gsf_file (NV_CHAR *path)
{
    NV_INT32          stat, gsf_handle;
    NV_CHAR           ndx_file[256];


    /*  Remove the GSF index file.  We have been having problems with
        corrupted GSF index files (cause unknown) so we want to get rid of
        it and let the GSF library gen us up a new one.  */

    strcpy (ndx_file, path);
    ndx_file[strlen (ndx_file) - 3] = 'n';
    stat = remove (ndx_file);

    if (stat && errno != ENOENT)
    {
        fprintf (stderr,"\n\nUnable to remove the GSF index file.\n");
        perror (ndx_file);
        exit (-1);
    }


    /*  Open the GSF file indexed so the library will generate another index 
        file.  */

    if (gsfOpen (path, GSF_UPDATE_INDEX, &gsf_handle)) 
    {
        fprintf (stderr, "\n\nUnable to open file %s\n", path);
        gsfPrintError (stderr);
        exit (-1);
    }

    gsfClose (gsf_handle);
}



NV_INT32 unload_gsf_file (NV_INT32 pfm_handle, NV_INT16 file_number, NV_INT32 ping_number, NV_INT16 beam_number, NV_U_INT32 validity, 
                          BIN_HEADER bin_header __attribute__ ((unused)), NV_INT32 argc __attribute__ ((unused)), NV_CHAR **argv __attribute__ ((unused)),
                          NV_CHAR *file)
{
    static gsfDataID        gsf_data_id;
    static gsfRecords       gsf_records;
    static NV_INT32         gsf_handle, prev_ping_number = -1, 
                            numrecs = 0, numfiles = 0;
    static NV_INT16         prev_file_number = -1;
    static NV_CHAR          prev_file[512];


    void pfm_to_gsf_flags (NV_U_INT32 pfm_flags, NV_U_CHAR *gsf_flags);



    /*  If we've changed files, write out the last record, close the old file,
        and open the new one.  */

    if (file_number != prev_file_number)
    {
        numfiles = get_next_list_file_number (pfm_handle);


        /*  Write last record and close old file.  */

        if (prev_file_number != -1)
        {
            if (gsfWrite (gsf_handle, &gsf_data_id, &gsf_records) == -1)
            {
                fprintf (stderr,"\nFile : %s\n", prev_file);
                gsfPrintError (stderr);
                fprintf(stderr,"Record number = %d\n", gsf_data_id.record_number);
                gsfClose (gsf_handle);
                fflush (stderr);
                return (-1);
            }

            gsfClose (gsf_handle);


            /*  Bail out if no more files.  */

            if (file_number < 0 && file == NULL) return (0);


            prev_ping_number = -1;
        }


        /*  If we have no more files, bail out.  */

        if (file_number < 0 || file_number > numfiles)
        {
            fprintf (stderr, "\nFile number %d out of range: 0 - %d\n", file_number, numfiles);
            fflush (stderr);
            return (-1);
        }


        prev_file_number = file_number;
        strcpy (prev_file, file);


        /*  Open the next file.  */

        if (gsfOpen (file, GSF_UPDATE_INDEX, &gsf_handle))
        {
            fprintf (stderr,"\nFile : %s\n", file);
            gsfPrintError (stderr);
            if (gsf_handle) gsfClose (gsf_handle);
            fflush (stderr);
            return (-1);
        }


        numrecs = gsfGetNumberRecords (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING);


        last_handle = gsf_handle;
        last_type = PFM_GSF_DATA;
    }


    /*  If we changed pings, write the old ping and read the new one.  */

    if (ping_number != prev_ping_number)
    {
        /*  Write the old ping.  */

        if (prev_ping_number != -1)
        {
            if (gsfWrite (gsf_handle, &gsf_data_id, &gsf_records) == -1)
            {
                fprintf (stderr,"\nFile : %s\n", prev_file);
                gsfPrintError (stderr);
                fprintf(stderr,"Record number = %d\n", gsf_data_id.record_number);
                prev_ping_number = -1;
                fflush (stderr);
                return (-1);
            }
        }


        if (ping_number < 0 || ping_number > numrecs) 
        {
            fprintf (stderr,"\nFile : %s\n", file);
            fprintf (stderr,"Ping %d out of range: 0 - %d\n", ping_number, numrecs);
            fflush (stderr);
            return (-1);
        }


        /*  Read the new one.  */

        gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
        gsf_data_id.record_number = ping_number;

        if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, 
            &gsf_data_id, &gsf_records, NULL, 0) == -1)
        {
            fprintf (stderr,"\nFile : %s\n", file);
            gsfPrintError (stderr);
            prev_ping_number = ping_number;
            fflush (stderr);
            return (-1);
        }

        prev_ping_number = ping_number;
    }


    if (beam_number < 1 || beam_number > gsf_records.mb_ping.number_beams)
    {
        fprintf (stderr,"\nFile : %s\n", file);
        fprintf (stderr,"\nPing : %d\n", ping_number);
        fprintf (stderr,"Beam %d out of range: 0 - %d\n", beam_number, gsf_records.mb_ping.number_beams);
        fflush (stderr);
        return (-1);
    }


    /*  Set the beam flags.  */

    pfm_to_gsf_flags (validity, &gsf_records.mb_ping.beam_flags[beam_number - 1]);


    return (0);
}



void check_shoals_file (NV_CHAR *path)
{
    FILE           *out_fp;


    NV_INT32 open_out_file (NV_CHAR *, FILE **);


    /*  Open the SHOALS .out file.  */

    if (open_out_file (path, &out_fp))
    {
        fprintf (stderr, "\n\nUnable to open the SHOALS .out file.\n");
        perror (path);
        exit (-1);
    }

    fclose (out_fp);
}



NV_INT32 unload_shoals_file (NV_INT32 pfm_handle __attribute__ ((unused)), NV_INT16 file_number, NV_INT32 ping_number,
                             NV_INT16 beam_number __attribute__ ((unused)), NV_U_INT32 validity, BIN_HEADER bin_header __attribute__ ((unused)),
                             NV_INT32 argc __attribute__ ((unused)), NV_CHAR **argv __attribute__ ((unused)), NV_CHAR *file)
{
    static NV_INT16         prev_file_number = -1;
    static FILE             *out_fp;
    OUTPUT_FILE_T           out;
    NV_INT32                position;
    NV_INT16                shoals_flag;


    NV_INT32 open_out_file (NV_CHAR *, FILE **);
    NV_INT32 big_endian ();
    void swap_output (OUTPUT_FILE_T *);


    /*  If we've changed files close the old file and open the new one.  */

    if (file_number != prev_file_number)
    {

        /*  Close old file.  */

        if (prev_file_number != -1) fclose (out_fp);


        /*  Open the SHOALS file.  */

        if (open_out_file (file, &out_fp))
        {
            fprintf (stderr, "\nError opening SHOALS out file : %s\n\n", file);
            prev_file_number = -1;
            fflush (stderr);
            return (-1);
        }


        last_fp = out_fp;
        last_type = PFM_SHOALS_OUT_DATA;


        /*  Read the first record to check for file source.  If the flightline
            is big_endian (less than 0 or greater than 255) this is a SUN
            data file.  */

        file_read_output (out_fp, &out);

        sun_data = NVFalse;
        if ((!big_endian () && (out.au.flightline < 0 || 
            out.au.flightline > 255)) || (big_endian () &&
            out.au.flightline > 0 && out.au.flightline < 255))
            sun_data = NVTrue;
        fseek (out_fp, 0, SEEK_SET);

        prev_file_number = file_number;
    }


    if (sun_data)
    {
        position = ping_number * (sizeof (OUTPUT_FILE_T) + 8);
    }
    else
    {
        position = ping_number * sizeof (OUTPUT_FILE_T);
    }
    fseek (out_fp, position, SEEK_SET);

    file_read_output (out_fp, &out);


    /*  Turn off the PFM_MODIFIED bit and save the rest of the status.

        SHOALS uses the lower two bits of the status field for
        status thusly :

        1 - STATUS_KEPT_MASK
        2 - STATUS_KILLED_MASK
        3 - STATUS_SWAPPED_MASK

        We're going to use the next 12 bits for PFM style status.
        Check pfm.h for a definition.  */

    shoals_flag = out.au.status & 3;
    if ((validity & PFM_INVAL) && shoals_flag != 3) shoals_flag = 2;
    out.au.status = ((validity & ~PFM_MODIFIED) << 2) | shoals_flag;

    if (validity & PFM_INVAL) 
        out.au.ab_dep_conf = -(ABS (out.au.ab_dep_conf));


    /*  Swap the bytes if needed.  */

    if ((!big_endian () && sun_data) || (big_endian () && !sun_data))
        swap_output (&out);


    fseek (out_fp, position, SEEK_SET);


    file_write_output (out_fp, &out);

    return (0);
}



void check_hof_file (NV_CHAR *path)
{
    FILE           *hof_fp;


    /*  Open the SHOALS .hof file.  */

    if ((hof_fp = open_hof_file (path)) == NULL)
    {
        fprintf (stderr, "\n\nUnable to open the SHOALS .hof file.\n");
        perror (path);
        exit (-1);
    }

    fclose (hof_fp);
}



NV_INT32 unload_hof_file (NV_INT32 pfm_handle __attribute__ ((unused)), NV_INT16 file_number, NV_INT32 ping_number, NV_INT16 beam_number, NV_U_INT32 validity, 
                          BIN_HEADER bin_header __attribute__ ((unused)), NV_INT32 argc __attribute__ ((unused)), NV_CHAR **argv __attribute__ ((unused)),
                          NV_CHAR *file, NV_BOOL old_lidar __attribute__ ((unused)), NV_INT16 type)
{
    static NV_INT16         prev_file_number = -1;
    static FILE             *hof_fp;
    HOF_HEADER_T            head;
    HYDRO_OUTPUT_T          record;


    /*  If we've changed files close the old file and open the new one.  */

    if (file_number != prev_file_number)
    {
        /*  Close old file.  */

        if (prev_file_number != -1) fclose (hof_fp);


        /*  Open the SHOALS file.  */

        if ((hof_fp = open_hof_file (file)) == NULL)
        {
            fprintf (stderr, "\nError opening SHOALS .hof file : %s\n\n", file);
            prev_file_number = -1;
            fflush (stderr);
            return (-1);
        }


        last_fp = hof_fp;
        last_type = type;


        hof_read_header (hof_fp, &head);


        prev_file_number = file_number;
    }


    hof_read_record (hof_fp, ping_number, &record);


    /*  If we used PFM_CHARTS_HOF_DATA we don't honor primary and secondary depths.  They are both valid unless
        marked otherwise.  */

    if (type == PFM_CHARTS_HOF_DATA)
      {
        if (validity & PFM_SELECTED_FEATURE) record.suspect_status |= SUSPECT_STATUS_FEATURE_BIT;


        if (beam_number)
          {
            if (validity & PFM_INVAL)
              {
                if (record.sec_abdc > 0) record.sec_abdc = -record.sec_abdc;


                /*  Now we check to see if the user has invalidated a point that was originally invalidated by GCS (abdc < 70)
                    but was manually overridden (+100).  */

                if (record.sec_abdc < -100) record.sec_abdc += 100;
              }
            else
              {
                if (record.sec_abdc < 0) record.sec_abdc = -record.sec_abdc;


                /*  Now we check to see if the user has manually overridden a point marked invalid by GCS (abdc < 70).  As long as 
                    the depth is valid and the abdc isn't 13 or 10, we add 100 to the value to force it to be valid.  */

                if (record.sec_abdc < 70 && record.correct_sec_depth > -998.0 && record.sec_abdc != 13 && record.sec_abdc != 10) record.sec_abdc += 100;


                /*  We need to see if GCS has set the AU_STATUS_DELETED bit in the status field.  If it has and we want this record
                    to be valid we have to override that bit.  */

                if (record.status & AU_STATUS_DELETED_BIT) record.status &= 0xfe;
              }
          }
        else
          {
            if (validity & PFM_INVAL)
              {
                if (record.abdc > 0) record.abdc = -record.abdc;


                /*  Now we check to see if the user has invalidated a point that was originally invalidate by GCS (abdc < 70)
                    but was manually overridden (+100).  */

                if (record.abdc < -100) record.abdc += 100;
              }
            else
              {
                if (record.abdc < 0) record.abdc = -record.abdc;


                /*  Now we check to see if the user has manually overridden a point marked invalid by GCS (abdc < 70).  As long as 
                    the depth is valid and the abdc isn't 13 or 10, we add 100 to the value to force it to be valid.  */

                if (record.abdc < 70 && record.correct_depth > -998.0 && record.abdc != 13 && record.abdc != 10) record.abdc += 100;


                /*  We need to see if GCS has set the AU_STATUS_DELETED bit in the status field.  If it has and we want this record
                    to be valid we have to override that bit.  */

                if (record.status & AU_STATUS_DELETED_BIT) record.status &= 0xfe;
              }
          }
      }
    else
      {
        if (validity & PFM_SELECTED_FEATURE) record.suspect_status |= SUSPECT_STATUS_FEATURE_BIT;


        /*  Another thing with the hof data - if the deleted bit is set we also set the abdc to a negative (or vice-versa).  */

        if (validity & PFM_INVAL)
          {
            if (record.abdc > 0) record.abdc = -record.abdc;


            /*  Now we check to see if the user has invalidated a point that was originally invalidate by GCS (abdc < 70)
                but was manually overridden (+100).  */

            if (record.abdc < -100) record.abdc += 100;
          }
        else
          {
            if (record.abdc < 0) record.abdc = -record.abdc;


            /*  Now we check to see if the user has manually overridden a point marked invalid by GCS (abdc < 70).  As long as 
                the depth is valid and the abdc isn't 13 or 10, we add 100 to the value to force it to be valid.  */

            if (record.abdc < 70 && record.correct_depth > -998.0 && record.abdc != 13 && record.abdc != 10) record.abdc += 100;


            /*  We need to see if GCS has set the AU_STATUS_DELETED bit in the status field.  If it has and we want this record
                to be valid we have to override that bit.  */

            if (record.status & AU_STATUS_DELETED_BIT) record.status &= 0xfe;
          }
      }


    if (!hof_write_record (hof_fp, ping_number, record))
      {
        fprintf (stderr, "\n\nUnable to write to the SHOALS .hof file.\n");
        perror (file);
        exit (-1);
      }

    return (0);
}



void check_tof_file (NV_CHAR *path)
{
    FILE           *tof_fp;


    /*  Open the SHOALS .tof file.  */

    if ((tof_fp = open_tof_file (path)) == NULL)
    {
        fprintf (stderr, "\n\nUnable to open the SHOALS .tof file.\n");
        perror (path);
        exit (-1);
    }

    fclose (tof_fp);
}



NV_INT32 unload_tof_file (NV_INT32 pfm_handle __attribute__ ((unused)), NV_INT16 file_number, NV_INT32 ping_number, NV_INT16 beam_number, NV_U_INT32 validity, 
                          BIN_HEADER bin_header __attribute__ ((unused)), NV_INT32 argc __attribute__ ((unused)), NV_CHAR **argv __attribute__ ((unused)),
                          NV_CHAR *file, NV_BOOL old_lidar __attribute__ ((unused)))
{
    static NV_INT16         prev_file_number = -1;
    static FILE             *tof_fp;
    TOF_HEADER_T            head;
    TOPO_OUTPUT_T           record;
    static NV_INT32         prev_ping = -1;


    /*  If we've changed files close the old file and open the new one.  */

    if (file_number != prev_file_number)
    {

        /*  Close old file.  */

        if (prev_file_number != -1) fclose (tof_fp);


        /*  Open the SHOALS file.  */

        if ((tof_fp = open_tof_file (file)) == NULL)
        {
            fprintf (stderr, "\nError opening SHOALS .tof file : %s\n\n", file);
            prev_file_number = -1;
            fflush (stderr);
            return (-1);
        }


        last_fp = tof_fp;
        last_type = PFM_SHOALS_TOF_DATA;


        tof_read_header (tof_fp, &head);


        prev_file_number = file_number;
        prev_ping = -1;
    }


    if (ping_number != prev_ping) tof_read_record (tof_fp, ping_number, &record);
    prev_ping = ping_number;


    if (beam_number)
      {
        if (validity & PFM_INVAL)
          {
            if (record.conf_last > 0) record.conf_last = -record.conf_last;


            /*  If the difference between the first and last return is less than 5 cm we didn't load the first return
                so we want to make sure that the first return gets set with the same validity as the last return if,
                and only if, the last return is set to invalid.  That is, we don't want to validate a first return
                that is within 5 cm of the last return if the last return is valid, but we do want to invalidate the
                first return if the last return is invalid and they are within 5 cm of each other.  */

            if (record.conf_last < 0 && fabs ((NV_FLOAT64) (record.elevation_last - record.elevation_first)) < 0.05)
              {
                if (record.conf_first > 0) record.conf_first = -record.conf_first;
              }
          }
        else
          {
            if (record.conf_last < 0) record.conf_last = -record.conf_last;
          }
      }
    else
      {
        if (validity & PFM_INVAL)
          {
            if (record.conf_first > 0) record.conf_first = -record.conf_first;
          }
        else
          {
            if (record.conf_first < 0) record.conf_first = -record.conf_first;
          }
      }


    if (!tof_write_record (tof_fp, ping_number, record))
      {
        fprintf (stderr, "\n\nUnable to write to the SHOALS .tof file.\n");
        perror (file);
        exit (-1);
      }

    return (0);
}



void check_wlf_file (NV_CHAR *path)
{
  NV_INT32           wlf_handle;
  WLF_HEADER         wlf_header;


  /*  Open the WLF file.  */

  if ((wlf_handle = wlf_open_file (path, &wlf_header, WLF_UPDATE)) < 0)
    {
      fprintf (stderr, wlf_strerror ());
      return;
    }


  wlf_close_file (wlf_handle);
}



NV_INT32 unload_wlf_file (NV_INT32 pfm_handle __attribute__ ((unused)), NV_INT16 file_number, NV_INT32 ping_number, NV_INT16 beam_number __attribute__ ((unused)),
                          NV_U_INT32 validity, BIN_HEADER bin_header __attribute__ ((unused)), NV_INT32 argc __attribute__ ((unused)),
                          NV_CHAR **argv __attribute__ ((unused)), NV_CHAR *file, NV_BOOL old_lidar __attribute__ ((unused)))
{
  static NV_INT16         prev_file_number = -1;
  static NV_INT32         wlf_handle;
  WLF_HEADER              head;
  WLF_RECORD              record;


  /*  If we've changed files close the old file and open the new one.  */

  if (file_number != prev_file_number)
    {

      /*  Close old file.  */

      if (prev_file_number != -1) wlf_close_file (wlf_handle);


      /*  Open the WLF file.  */

      if ((wlf_handle = wlf_open_file (file, &head, WLF_UPDATE)) < 0)
        {
          fprintf (stderr, wlf_strerror ());
          prev_file_number = -1;
          return (-1);
        }


      last_handle = wlf_handle;
      last_type = PFM_WLF_DATA;


      prev_file_number = file_number;
    }


  if (wlf_read_record (wlf_handle, ping_number, &record, NVFalse, NULL) >= 0)
    {
      record.status = 0;
      if (validity & PFM_MANUALLY_INVAL) record.status = WLF_MANUALLY_INVAL;
      if (validity & PFM_FILTER_INVAL) record.status = WLF_FILTER_INVAL;
      if (validity & PFM_SUSPECT) record.status |= WLF_SUSPECT;
      if (validity & PFM_SELECTED_SOUNDING) record.status |= WLF_SELECTED_SOUNDING;
      if (validity & PFM_SELECTED_FEATURE) record.status |= WLF_SELECTED_FEATURE;
      if (validity & PFM_DESIGNATED_SOUNDING) record.status |= WLF_DESIGNATED_SOUNDING;

      if (wlf_update_record (wlf_handle, ping_number, record) < 0)
        {
          fprintf (stderr, "\n\nUnable to write to the WLF file.\n");
          wlf_perror ();
          exit (-1);
        }
    }

    return (0);
}



void check_hawkeye_file (NV_CHAR *path)
{
  NV_INT32                hawkeye_handle;
  HAWKEYE_META_HEADER     *hawkeye_meta_header;
  HAWKEYE_CONTENTS_HEADER *hawkeye_contents_header;


  /*  Open the HAWKEYE file.  */

  if ((hawkeye_handle = hawkeye_open_file (path, &hawkeye_meta_header, &hawkeye_contents_header, HAWKEYE_READONLY)) < 0)
    {
      fprintf (stderr, hawkeye_strerror ());
      return;
    }


  hawkeye_close_file (hawkeye_handle);
}



NV_INT32 unload_hawkeye_file (NV_INT32 pfm_handle __attribute__ ((unused)), NV_INT16 file_number, NV_INT32 ping_number,
                              NV_INT16 beam_number __attribute__ ((unused)), NV_U_INT32 validity, BIN_HEADER bin_header __attribute__ ((unused)),
                              NV_INT32 argc __attribute__ ((unused)), NV_CHAR **argv __attribute__ ((unused)), NV_CHAR *file,
                              NV_BOOL old_lidar __attribute__ ((unused)))
{
  static NV_INT16         prev_file_number = -1;
  static NV_INT32         hawkeye_handle;
  HAWKEYE_META_HEADER     *meta_header;
  HAWKEYE_CONTENTS_HEADER *contents_header;
  HAWKEYE_RECORD          record;


  /*  If we've changed files close the old file and open the new one.  */

  if (file_number != prev_file_number)
    {

      /*  Close old file.  */

      if (prev_file_number != -1) hawkeye_close_file (hawkeye_handle);


      /*  Open the HAWKEYE file.  */

      if ((hawkeye_handle = hawkeye_open_file (file, &meta_header, &contents_header, HAWKEYE_UPDATE)) < 0)
        {
          fprintf (stderr, hawkeye_strerror ());
          prev_file_number = -1;
          return (-1);
        }


      last_handle = hawkeye_handle;


      /*  We don't have to differentiate between hydro and topo data since they use the same file format.  */

      last_type = PFM_HAWKEYE_HYDRO_DATA;


      prev_file_number = file_number;
    }


  if (!hawkeye_read_record (hawkeye_handle, ping_number, &record))
    {
      record.Manual_Output_Screening_Flags = 0;
      if (validity & PFM_INVAL)
        {
          record.Manual_Output_Screening_Flags = HAWKEYE_INVALID;
        }
      else
        {
          record.Manual_Output_Screening_Flags = HAWKEYE_VALID;
        }

      if (hawkeye_update_record (hawkeye_handle, ping_number, record) < 0)
        {
          fprintf (stderr, "\n\nUnable to write to the Hawkeye file.\n");
          hawkeye_perror ();
          exit (-1);
        }
    }

    return (0);
}



NV_BOOL check_unisips_file (NV_CHAR *path)
{
    FILE               *uni_fp;
    NV_INT32           swap;
    UNISIPS_HEADER     unisips_header;



    /*  Open the UNISIPS file.  */

    uni_fp = fopen (path, "rb+");


    /* Read UNISIPS header information          */

    if (!read_unisips_header (uni_fp, &unisips_header, &swap))
    {
        fprintf (stderr, "\n\nUnable to open the UNISIPS file.\n");
        fprintf (stderr, "%s : %s\n", path, strerror (errno));
        fflush (stderr);
        return (NVFalse);
    }

    fclose (uni_fp);

    return (NVTrue);
}



NV_INT32 unload_unisips_file (NV_INT32 pfm_handle __attribute__ ((unused)), NV_INT16 file_number, NV_INT32 ping_number,
                              NV_INT16 beam_number __attribute__ ((unused)), NV_U_INT32 validity, BIN_HEADER bin_header __attribute__ ((unused)),
                              NV_INT32 argc __attribute__ ((unused)), NV_CHAR **argv __attribute__ ((unused)), NV_CHAR *file)
{
    static NV_INT16         prev_file_number = -1;
    static FILE             *uni_fp;
    UNISIPS_HEADER          unisips_header;
    UNISIPS_REC_HDR         unisips_rec_hdr;
    UNISIPS_REC_SUBHDR      unisips_rec_subhdr;
    NV_INT32                swap, height, width;



    /*  If we've changed files close the old file and open the new one.  */

    if (file_number != prev_file_number)
    {

        /*  Close old file.  */

        if (prev_file_number != -1) fclose (uni_fp);


        /*  Open the UNISIPS file.  */

        if ((uni_fp = fopen (file, "rb+")) == NULL)
        {
            fprintf (stderr, "\nError opening UNISIPS file : %s\n\n", file);
            prev_file_number = -1;
            fflush (stderr);
            return (-1);
        }


        last_fp = uni_fp;
        last_type = PFM_UNISIPS_DEPTH_DATA;


        /* Read UNISIPS header information          */

        if (!(read_unisips_header (uni_fp, &unisips_header, &swap)))
        {
            fprintf (stderr, "\nError opening UNISIPS file : %s\n\n", file);
            prev_file_number = -1;
            fflush (stderr);
            return (-1);
        }

        height = unisips_header.rows;
        width = unisips_header.cols;

        prev_file_number = file_number;
    }


    read_unisips_rec_hdr (uni_fp, &unisips_header, &unisips_rec_hdr, 
                          &unisips_rec_subhdr, ping_number, swap);

    /*  At present we're only using the lower two bits which correspond to
        the PFM validity lower two bits.  */

    unisips_rec_hdr.tide_v |= (validity & 0x0003);


    write_unisips_rec_hdr (uni_fp, &unisips_header, &unisips_rec_hdr, 
                           &unisips_rec_subhdr, ping_number, swap);


    return (0);
}



NV_BOOL check_llz_file (NV_CHAR *path)
{
  NV_INT32                llz_hnd;
  LLZ_HEADER              llz_header;


  /*  Open the llz file.  */

  if ((llz_hnd = open_llz (path, &llz_header)) < 0)
    {
      fprintf (stderr, "\n\nUnable to open the LLZ file.\n");
      fprintf (stderr, "%s : %s\n", path, strerror (errno));
      fflush (stderr);
      return (NVFalse);
    }

  if (!strstr (llz_header.version, "llz library"))
    {
      fprintf (stderr, "\n\nLLZ file version corrupt or %s is not an LLZ file.\n", path);
      fflush (stderr);
      return (NVFalse);
    }

  close_llz (llz_hnd);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : unload_llz_file
 *
 * Description : Writes modified flags to an llz file.  See llz.h for format.
 *
 ********************************************************************/

NV_INT32 unload_llz_file (NV_INT32 pfm_handle __attribute__ ((unused)), NV_INT16 file_number, NV_INT32 ping_number, NV_INT16 beam_number __attribute__ ((unused)),
                          NV_U_INT32 validity, BIN_HEADER bin_header __attribute__ ((unused)), NV_INT32 argc __attribute__ ((unused)),
                          NV_CHAR **argv __attribute__ ((unused)), NV_CHAR *file)
{
  static NV_INT32         llz_hnd = -1;
  static LLZ_HEADER       llz_header;
  LLZ_REC                 llz;
  static NV_INT16         prev_file_number = -1;


  /*  If we've changed files close the old file and open the new one.  */

  if (file_number != prev_file_number)
    {

      /*  Close old file.  */

      if (prev_file_number != -1) close_llz (llz_hnd);


      /*  Open the llz file.  */

      if ((llz_hnd = open_llz (file, &llz_header)) < 0)
        {
          fprintf (stderr, "\nError opening llz file %s.\n\n", file);
          prev_file_number = -1;
          fflush (stderr);
          return (-1);
        }


        last_handle = llz_hnd;
        last_type = PFM_NAVO_LLZ_DATA;


        prev_file_number = file_number;
    }


    read_llz (llz_hnd, ping_number, &llz);

    llz.status = validity & 0x00000003;


    update_llz (llz_hnd, ping_number, llz);

    return (0);
}



NV_BOOL check_dted_file (NV_CHAR *path)
{
  FILE           *fp;
  UHL            uhl;


  /*  Open the DTED file.  */

  if ((fp = fopen (path, "rb")) == NULL)
    {
      fprintf (stderr, "\n\nUnable to open the DTED file.\n");
      fprintf (stderr, "%s : %s\n", path, strerror (errno));
      return (NVFalse);
    }


  if (read_uhl (fp, &uhl) < 0)
    {
      fprintf (stderr, "\n\n%s is not a DTED file.\n", path);
      return (NVFalse);
    }


  fclose (fp);


  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : unload_dted_file
 *
 * Description : This is a special unloader that writes a zero value
 *               back to the file if the point is set to invalid.  
 *               This is used to correct bad -32767 values only!
 *
 ********************************************************************/

NV_INT32 unload_dted_file (NV_INT32 pfm_handle __attribute__ ((unused)), NV_INT16 file_number, NV_INT32 ping_number, NV_INT16 beam_number, NV_U_INT32 validity, 
                           BIN_HEADER bin_header __attribute__ ((unused)), NV_INT32 argc __attribute__ ((unused)), NV_CHAR **argv __attribute__ ((unused)),
                           NV_CHAR *file)
{
  static FILE             *fp = NULL;
  NV_INT32                status;
  DTED_DATA               dted_data;
  static UHL              uhl;
  static NV_INT16         prev_file_number = -1;


  /*  If we've changed files close the old file and open the new one.  */

  if (file_number != prev_file_number)
    {

      /*  Close old file.  */

      if (prev_file_number != -1) fclose (fp);


      /*  Open the dted file.  */

      if ((fp = fopen (file, "rb+")) == NULL)
        {
          fprintf (stderr, "\nError opening dted file %s.\n\n", file);
          prev_file_number = -1;
          fflush (stderr);
          return (-1);
        }

      read_uhl (fp, &uhl);


      last_fp = fp;
      last_type = PFM_DTED_DATA;


      prev_file_number = file_number;
    }


  status = read_dted_data (fp, uhl.num_lat_points, ping_number, &dted_data);

  if (status < 0)
    {
      fprintf (stderr, "Unable to read dted record at %d\n", ping_number);
      return (-1);
    }

  if (validity & PFM_MANUALLY_INVAL) dted_data.elev[beam_number] = 0.0;

  status = write_dted_data (fp, uhl.num_lat_points, ping_number, dted_data);

  if (status < 0)
    {
      fprintf (stderr, "Unable to write dted record at %d\n", ping_number);
      return (-1);
    }


  return (0);
}
