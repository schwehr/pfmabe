
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#include <errno.h>
#include <signal.h>

#include "wlf.h"
#include "wlf_version.h"


#define NINT(a)                 ((a)<0.0 ? (NV_INT32) ((a) - 0.5) : (NV_INT32) ((a) + 0.5))
#define NINT64(a)               ((a)<0.0 ? (NV_INT64) ((a) - 0.5) : (NV_INT64) ((a) + 0.5))

#ifndef MAX
  #define       MAX(x,y)        (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
  #define       MIN(x,y)        (((x) < (y)) ? (x) : (y))
#endif

#define         DPRINT          fprintf (stderr, "%s %d\n", __FILE__, __LINE__);fflush (stderr);


#define         WAVE_CAT_BLOCK  1048576


#undef WLF_DEBUG


/*  This is the structure we use to keep track of important formatting data for an open WLF file.  */

typedef struct
{
  FILE          *fp;                        /*  WLF file pointer.  */
  FILE          *wfp;                       /*  File pointer for temporary waveform file when creating the WLF file.  */
  NV_CHAR       path[1024];                 /*  File name.  */
  NV_CHAR       wave_path[1024];            /*  Temporary waveform file name.  */
  NV_BOOL       at_end;                     /*  Set if the file position is at the end of the file.  */
  NV_BOOL       modified;                   /*  Set if the file has been modified.  */
  NV_BOOL       created;                    /*  Set if we created the file.  */
  NV_BOOL       write;                      /*  Set if the last action to the file was a write.  */
  NV_INT32      prev_shot_num;              /*  Shot number of the last shot stored for this file (in wlf_append_record).  */
  NV_INT64      prev_waveform_address;      /*  Waveform address stored with the previously appended record.  */
  NV_INT16      major_version;              /*  Major version number for backward compatibility.  */
  NV_INT64      waveform_block_offset;      /*  Address of the beginning of the waveform block.  */
  NV_INT64      waveform_address;           /*  Waveform address associated with the current record.  */
  NV_INT16      waveform_start_value_size;  /*  8, 16, or 32 depending upon the data range.  */
  NV_FLOAT64    waveform_total_bytes;       /*  Total number of bytes used to store the waveforms (on creation of file).  */
  NV_INT32      waveform_max_size;          /*  Size of waveform arrays without compression.  */
  NV_U_INT32    header_size;                /*  Header size in bytes.  */
  NV_U_INT16    record_size;                /*  Record size in bytes.  */
  NV_U_BYTE     *wave_buffer;               /*  Buffer for reading waveform data.  This is allocated on open/create and
                                                freed on close.  This saves us from allocating and freeing on each
                                                waveform read.  */
  NV_INT32      **wave;                     /*  Waveform array for storing waveform data when it is read.  This is allocated
                                                on open/create and freed on close.  This saves us from allocating and
                                                freeing on each waveform read.  It also saves the application the hassle
                                                of trying to determine if it needs to free the waveform data.  */


  /*  Number of bits needed for each field of the record.  */

  NV_U_BYTE     tv_sec_bits;
  NV_U_BYTE     tv_nsec_bits;
  NV_U_BYTE     h_uncert_bits;
  NV_U_BYTE     v_uncert_bits;
  NV_U_BYTE     x_bits;
  NV_U_BYTE     y_bits;
  NV_U_BYTE     z_bits;
  NV_U_BYTE     waveform_bits;
  NV_U_BYTE     waveform_point_bits;
  NV_U_BYTE     sensor_x_bits;
  NV_U_BYTE     sensor_y_bits;
  NV_U_BYTE     sensor_z_bits;
  NV_U_BYTE     sensor_roll_bits;
  NV_U_BYTE     sensor_pitch_bits;
  NV_U_BYTE     sensor_heading_bits;
  NV_U_BYTE     scan_angle_bits;
  NV_U_BYTE     nadir_angle_bits;
  NV_U_BYTE     water_surface_bits;
  NV_U_BYTE     z_offset_bits;
  NV_U_BYTE     edge_bits;
  NV_U_BYTE     return_bits;
  NV_U_BYTE     point_source_bits;
  NV_U_BYTE     intensity_bits;
  NV_U_BYTE     attr_bits[WLF_MAX_ATTR];
  NV_U_BYTE     rgb_bits;
  NV_U_BYTE     reflectance_bits;
  NV_U_BYTE     class_bits;
  NV_U_BYTE     status_bits;
  NV_U_BYTE     address_bits;
  NV_U_BYTE     waveform_blocksize_bits;
  NV_U_BYTE     waveform_blocksize_bytes;


  /*  Max and min values for the hardwired fields (only populated when creating file).  */

  NV_INT64      min_tv_sec;
  NV_INT64      max_tv_sec;
  NV_INT32      min_tv_nsec;
  NV_INT32      max_tv_nsec;
  NV_INT32      max_class;
  NV_INT32      max_status;
  NV_INT64      max_address;


  /*  Bit positions within the record of each field.  */

  NV_U_INT16    tv_sec_pos;
  NV_U_INT16    tv_nsec_pos;
  NV_U_INT16    h_uncert_pos;
  NV_U_INT16    v_uncert_pos;
  NV_U_INT16    x_pos;
  NV_U_INT16    y_pos;
  NV_U_INT16    z_pos;
  NV_U_INT16    waveform_pos;
  NV_U_INT16    waveform_point_pos;
  NV_U_INT16    sensor_x_pos;
  NV_U_INT16    sensor_y_pos;
  NV_U_INT16    sensor_z_pos;
  NV_U_INT16    sensor_roll_pos;
  NV_U_INT16    sensor_pitch_pos;
  NV_U_INT16    sensor_heading_pos;
  NV_U_INT16    scan_angle_pos;
  NV_U_INT16    nadir_angle_pos;
  NV_U_INT16    water_surface_pos;
  NV_U_INT16    z_offset_pos;
  NV_U_INT16    edge_pos;
  NV_U_INT16    number_of_returns_pos;
  NV_U_INT16    return_number_pos;
  NV_U_INT16    point_source_pos;
  NV_U_INT16    intensity_pos;
  NV_U_INT16    attr_pos[WLF_MAX_ATTR];
  NV_U_INT16    rgb_pos;
  NV_U_INT16    reflectance_pos;
  NV_U_INT16    class_pos;
  NV_U_INT16    status_pos;
  NV_U_INT16    address_pos;


  /*  WLF header.  */

  WLF_HEADER    header;                     /*  WLF file header.  */
} INTERNAL_WLF_STRUCT;


/*  This is where we'll store the headers of all open WLF files (in addition to a bunch of other things, see above).  */

static INTERNAL_WLF_STRUCT wlfh[WLF_MAX_FILES];


/*  Startup flag used by either wlf_create_file or wlf_open_file to initialize the internal struct array and
    set the SIGINT handler.  */

static NV_BOOL first = NVTrue;


/*  WLF error handling variables.  */

typedef struct 
{
  NV_INT32      system;            /*  Last system error condition encountered.  */
  NV_INT32      wlf;               /*  Last WLF error condition encountered.  */
  NV_CHAR       file[512];         /*  Name of file being accessed when last error encountered (if applicable).  */
  NV_INT32      recnum;            /*  Record number being accessed when last error encountered (if applicable).  */
  NV_CHAR       info[128];
} WLF_ERROR_STRUCT;

static WLF_ERROR_STRUCT wlf_error;


/*  So we don't have to keep computing this number over and over, it is defined on open or create.  */

static NV_FLOAT64    log_of_two;


/*  Initialize the progress callback to NULL.  */

static  WLF_PROGRESS_CALLBACK  wlf_progress_callback = NULL;



/*********************************************************************************************

    Module Name:        wlf_register_progress_callback

    Programmer(s):      Jan C. Depner

    Date Written:       04/14/09

    Purpose:            Allows the calling application to register a callback procedure that
                        will receive a state and percent complete value from the wlf_close_file
                        waveform concatenation process.  This allows a caller to keep the user 
                        informed about the progress of the last part of creating a new
                        WLF file.  For an example of use see section 4 (SAMPLE CREATION CODE)
                        in the wlf.h file.

    Arguments:          progressCB - callback function

    Return Value:       None

    Caveats:            At present the state variable is not used and is set to 0.  If we add 
                        other callbacks in the future the state will be set to 1, 2, 3, etc
                        for each type needed.

*********************************************************************************************/

WLF_DLL void wlf_register_progress_callback (WLF_PROGRESS_CALLBACK progressCB)
{
    wlf_progress_callback = progressCB;
}



/*********************************************************************************************

    Module Name:        get_string

    Programmer(s):      Jan C. Depner

    Date Written:       December 1994

    Purpose:            Parses the input string for the equals sign and returns everything to
                        the right between the first and last non-blank character (inclusive).

    Arguments:          *in     -   Input string
                        *out    -   Output string

    Return Value:       None

*********************************************************************************************/

static void get_string (const NV_CHAR *in, NV_CHAR *out)
{
    NV_INT32            i, start, length;
    NV_CHAR             *ptr;


    start = 0;
    length = 0;


    ptr = strchr (in, '=') + 1;


    /*  Search for first non-blank character.   */

    for (i = 0 ; i < strlen (ptr) ; i++)
    {
        if (ptr[i] != ' ')
        {
            start = i;
            break;
        }
    }


    /*  Search for last non-blank character.    */

    for (i = strlen (ptr) ; i >= 0 ; i--)
    {
        if (ptr[i] != ' ' && ptr[i] != 0)
        {
            length = (i + 1) - start;
            break;
        }
    }

    strncpy (out, &ptr[start], length);
    out[length] = 0;
}



/*********************************************************************************************

  Function:    clean_exit

  Purpose:     Exit from the application after first cleaning up memory and orphaned files.
               This will only be called in the case of an abnormal exit (like a failed malloc).

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        03/18/09

  Arguments:   ret            -    Value to be passed to exit ();  If set to -999 we were
                                   called from the SIGINT signal handler so we must return
                                   to allow it to SIGINT itself.

  Returns:     N/A

*********************************************************************************************/

static void clean_exit (NV_INT32 ret)
{
  NV_INT32 i;

  for (i = 0 ; i < WLF_MAX_FILES ; i++)
    {
      if (wlfh[i].fp != NULL)
        {
          if (wlfh[i].created)
            {
              /*  If we were in the process of creating a file we need to remove it since it isn't finished.  */

              fclose (wlfh[i].fp);
              remove (wlfh[i].path);


              /*  If we had waveforms we need to get rid of the temporary waveform file since we aborted somehow.  */

              if (wlfh[i].header.number_of_waveforms)
                {
                  fclose (wlfh[i].wfp);
                  remove (wlfh[i].wave_path);
                }
            }
          else
            {
              /*  Free the waveform buffer and arrays if needed.  These only get created if you called wlf_open_file
                  not if you called wlf_create_file.  */

              if (wlfh[i].header.number_of_waveforms)
                {
                  free (wlfh[i].wave_buffer);

                  for (i = 0 ; i < wlfh[i].header.number_of_waveforms ; i++) free (wlfh[i].wave[i]);

                  free (wlfh[i].wave);
                }
            }
        }
    }


  /*  Return to the SIGINT handler.  */

  if (ret == -999 && getpid () > 1) return;


  exit (ret);
}



/*********************************************************************************************

  Function:    sigint_handler

  Purpose:     Simple little SIGINT handler.  Allows us to clean up the files if we were 
               creating a new WLF file and someone does a CTRL-C.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   sig            -    The signal

  Returns:     N/A

  Caveats:     The way to do this was borrowed from "Proper handling of SIGINT/SIGQUIT",
               http://www.cons.org/cracauer/sigint.html

*********************************************************************************************/

static void sigint_handler (int sig)
{
  clean_exit (-999);

  signal (SIGINT, SIG_DFL);

#ifdef NVWIN3X
  raise (sig);
#else
  kill (getpid (), SIGINT);
#endif
}



/*********************************************************************************************

  Function:    define_record_fields

  Purpose:     Computes the size and position, in bits, of each point record based on the
               information in the header of the WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The WLF file handle
               creating       -    NVTrue if we are creating a new file

  Returns:     N/A

  Caveats:     !!!!!!!!!!!!!!!!!!!!!!! VERY IMPORTANT NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

               In the future, when adding fields to the point record structure, add new
               fields AFTER the waveform address.  As long as we don't change the order 
               of the fields we should be able to provide "forward compatibility" since
               the record size is stored in the file header.  That is, suppose we added
               two new fields after the waveform address field and changed the version to
               2.0.  A 1.0 implementation of the library would be able to read the file
               because the point record size it would read is based on the [RECORD SIZE]
               field of the header.  Of course, the old library would not unpack the two
               new fields but it should still function normally in realation to the rest
               of the (1.0 defined) fields.

               !!!!!!!!!!!!!!!!!!!!!!! VERY IMPORTANT NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

*********************************************************************************************/

static void define_record_fields (NV_INT32 hnd, NV_BOOL creating)
{
  NV_FLOAT64 range;
  NV_INT32 i, total_bits, waveform_count;


  /*  [TV SEC BITS] (wlfh[hnd].tv_sec_bits) is either read in wlf_open_file or hardwired in wlf_create_file.  */

  wlfh[hnd].tv_sec_pos = 0;


  /*  [TV NSEC BITS] (wlfh[hnd].tv_nsec_bits) is either read in wlf_open_file or hardwired in wlf_create_file.  */

  wlfh[hnd].tv_nsec_pos = wlfh[hnd].tv_sec_pos + wlfh[hnd].tv_sec_bits;


  /*  Horizontal uncertainty  */

  wlfh[hnd].h_uncert_pos = wlfh[hnd].tv_nsec_pos + wlfh[hnd].tv_nsec_bits;
  if (wlfh[hnd].header.horizontal_uncertainty_scale > 0.0)
    {
      range = (NV_FLOAT64) ((wlfh[hnd].header.max_horizontal_uncertainty - wlfh[hnd].header.min_horizontal_uncertainty) *
                            wlfh[hnd].header.horizontal_uncertainty_scale);
      wlfh[hnd].h_uncert_bits = NINT (log10 (range) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].h_uncert_bits = 0;
    }


  /*  Vertical uncertainty  */

  wlfh[hnd].v_uncert_pos = wlfh[hnd].h_uncert_pos + wlfh[hnd].h_uncert_bits;
  if (wlfh[hnd].header.vertical_uncertainty_scale > 0.0)
    {
      range = (NV_FLOAT64) ((wlfh[hnd].header.max_vertical_uncertainty - wlfh[hnd].header.min_vertical_uncertainty) *
                            wlfh[hnd].header.vertical_uncertainty_scale);
      wlfh[hnd].v_uncert_bits = NINT (log10 (range) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].v_uncert_bits = 0;
    }


  /*  X  */

  wlfh[hnd].x_pos = wlfh[hnd].v_uncert_pos + wlfh[hnd].v_uncert_bits;
  range = (wlfh[hnd].header.max_x - wlfh[hnd].header.min_x) * wlfh[hnd].header.x_scale;
  wlfh[hnd].x_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Y  */

  wlfh[hnd].y_pos = wlfh[hnd].x_pos + wlfh[hnd].x_bits;
  range = (wlfh[hnd].header.max_y - wlfh[hnd].header.min_y) * wlfh[hnd].header.y_scale;
  wlfh[hnd].y_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Z  -  Note that we are adding 1 to the range in order to store our NULL value as one more than max.  */

  wlfh[hnd].z_pos = wlfh[hnd].y_pos + wlfh[hnd].y_bits;
  range = (NV_FLOAT64) (((wlfh[hnd].header.max_z - wlfh[hnd].header.min_z) + 1.0) * wlfh[hnd].header.z_scale);
  wlfh[hnd].z_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Waveform  */

  wlfh[hnd].waveform_pos = wlfh[hnd].z_pos + wlfh[hnd].z_bits;
  if (wlfh[hnd].header.number_of_waveforms)
    {
      wlfh[hnd].waveform_bits = NINT (log10 ((NV_FLOAT64) wlfh[hnd].header.number_of_waveforms) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].waveform_bits = 0;
    }


  /*  Waveform Point */

  wlfh[hnd].waveform_point_pos = wlfh[hnd].waveform_pos + wlfh[hnd].waveform_bits;
  if (wlfh[hnd].header.number_of_waveforms)
    {
      /*  Find the max range of all of the waveforms from which to compute the number of bits for this field.  */

      waveform_count = 0;
      for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++)
        {
          if (wlfh[hnd].header.waveform_count[i] > waveform_count) waveform_count = wlfh[hnd].header.waveform_count[i];
        }

      wlfh[hnd].waveform_point_bits = NINT (log10 ((NV_FLOAT64) waveform_count) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].waveform_point_bits = 0;
    }


  /*  Sensor X  */

  wlfh[hnd].sensor_x_pos = wlfh[hnd].waveform_point_pos + wlfh[hnd].waveform_point_bits;
  range = (wlfh[hnd].header.sensor_max_x - wlfh[hnd].header.sensor_min_x) * wlfh[hnd].header.sensor_x_scale;
  wlfh[hnd].sensor_x_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Sensor Y  */

  wlfh[hnd].sensor_y_pos = wlfh[hnd].sensor_x_pos + wlfh[hnd].sensor_x_bits;
  range = (wlfh[hnd].header.sensor_max_y - wlfh[hnd].header.sensor_min_y) * wlfh[hnd].header.sensor_y_scale;
  wlfh[hnd].sensor_y_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Sensor Z  */

  wlfh[hnd].sensor_z_pos = wlfh[hnd].sensor_y_pos + wlfh[hnd].sensor_y_bits;
  range = (NV_FLOAT64) ((wlfh[hnd].header.sensor_max_z - wlfh[hnd].header.sensor_min_z) * wlfh[hnd].header.sensor_z_scale);
  wlfh[hnd].sensor_z_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Sensor Roll  */

  wlfh[hnd].sensor_roll_pos = wlfh[hnd].sensor_z_pos + wlfh[hnd].sensor_z_bits;
  range = (NV_FLOAT64) ((wlfh[hnd].header.sensor_max_roll - wlfh[hnd].header.sensor_min_roll) * wlfh[hnd].header.sensor_roll_scale);
  wlfh[hnd].sensor_roll_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Sensor Pitch  */

  wlfh[hnd].sensor_pitch_pos = wlfh[hnd].sensor_roll_pos + wlfh[hnd].sensor_roll_bits;
  range = (NV_FLOAT64) ((wlfh[hnd].header.sensor_max_pitch - wlfh[hnd].header.sensor_min_pitch) * wlfh[hnd].header.sensor_pitch_scale);
  wlfh[hnd].sensor_pitch_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Sensor Heading  */

  wlfh[hnd].sensor_heading_pos = wlfh[hnd].sensor_pitch_pos + wlfh[hnd].sensor_pitch_bits;
  range = (NV_FLOAT64) ((wlfh[hnd].header.sensor_max_heading - wlfh[hnd].header.sensor_min_heading) * wlfh[hnd].header.sensor_heading_scale);
  wlfh[hnd].sensor_heading_bits = NINT (log10 (range) / log_of_two + 0.5);


  /*  Scan angle  */

  wlfh[hnd].scan_angle_pos = wlfh[hnd].sensor_heading_pos + wlfh[hnd].sensor_heading_bits;
  if (wlfh[hnd].header.scan_angle_scale > 0.0)
    {
      range = (NV_FLOAT64) ((wlfh[hnd].header.max_scan_angle - wlfh[hnd].header.min_scan_angle) *
                            wlfh[hnd].header.scan_angle_scale);
      wlfh[hnd].scan_angle_bits = NINT (log10 (range) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].scan_angle_bits = 0;
    }


  /*  Nadir angle  */

  wlfh[hnd].nadir_angle_pos = wlfh[hnd].scan_angle_pos + wlfh[hnd].scan_angle_bits;
  if (wlfh[hnd].header.nadir_angle_scale > 0.0)
    {
      range = (NV_FLOAT64) ((wlfh[hnd].header.max_nadir_angle - wlfh[hnd].header.min_nadir_angle) *
                            wlfh[hnd].header.nadir_angle_scale);
      wlfh[hnd].nadir_angle_bits = NINT (log10 (range) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].nadir_angle_bits = 0;
    }


  /*  Water surface  */

  wlfh[hnd].water_surface_pos = wlfh[hnd].nadir_angle_pos + wlfh[hnd].nadir_angle_bits;
  if (wlfh[hnd].header.water_surface_scale > 0.0)
    {
      range = (NV_FLOAT64) ((wlfh[hnd].header.max_water_surface - wlfh[hnd].header.min_water_surface) *
                            wlfh[hnd].header.water_surface_scale);
      wlfh[hnd].water_surface_bits = NINT (log10 (range) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].water_surface_bits = 0;
    }


  /*  Z offset  */

  wlfh[hnd].z_offset_pos = wlfh[hnd].water_surface_pos + wlfh[hnd].water_surface_bits;
  if (wlfh[hnd].header.z_offset_scale > 0.0)
    {
      range = (NV_FLOAT64) ((wlfh[hnd].header.max_z_offset - wlfh[hnd].header.min_z_offset) *
                            wlfh[hnd].header.z_offset_scale);
      wlfh[hnd].z_offset_bits = NINT (log10 (range) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].z_offset_bits = 0;
    }


  /*  Point source  */

  wlfh[hnd].point_source_pos = wlfh[hnd].z_offset_pos + wlfh[hnd].z_offset_bits;
  if (wlfh[hnd].header.max_point_source_id)
    {
      wlfh[hnd].point_source_bits = NINT (log10 ((NV_FLOAT64) wlfh[hnd].header.max_point_source_id) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].point_source_bits = 0;
    }


  /*  Edge flag  */

  wlfh[hnd].edge_pos = wlfh[hnd].point_source_pos + wlfh[hnd].point_source_bits;
  if (wlfh[hnd].header.edge_flag_present)
    {
      wlfh[hnd].edge_bits = 2;
    }
  else
    {
      wlfh[hnd].edge_bits = 0;
    }


  /*  Number of returns and return number  */

  wlfh[hnd].number_of_returns_pos = wlfh[hnd].edge_pos + wlfh[hnd].edge_bits;
  if (wlfh[hnd].header.max_number_of_returns > 1)
    {
      wlfh[hnd].return_bits = NINT (log10 ((NV_FLOAT64) wlfh[hnd].header.max_number_of_returns) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].return_bits = 0;
    }

  wlfh[hnd].return_number_pos = wlfh[hnd].number_of_returns_pos + wlfh[hnd].return_bits;


  /*  Intensity  */

  wlfh[hnd].intensity_pos = wlfh[hnd].return_number_pos + wlfh[hnd].return_bits;
  if (wlfh[hnd].header.intensity_scale > 0.0)
    {
      range = (NV_FLOAT64) ((wlfh[hnd].header.max_intensity - wlfh[hnd].header.min_intensity) *
                            wlfh[hnd].header.intensity_scale);
      wlfh[hnd].intensity_bits = NINT (log10 (range) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].intensity_bits = 0;
    }


  /*  Optional attributes  */

  wlfh[hnd].attr_pos[0] = wlfh[hnd].intensity_pos + wlfh[hnd].intensity_bits;

  for (i = 0 ; i < WLF_MAX_ATTR ; i++)
    {
      if (i) wlfh[hnd].attr_pos[i] = wlfh[hnd].attr_pos[i - 1] + wlfh[hnd].attr_bits[i - 1];

      if (wlfh[hnd].header.attr_scale[i] > 0.0)
        {
          wlfh[hnd].attr_bits[i] = NINT (log10 ((NV_FLOAT64) wlfh[hnd].header.max_attr[i]) / log_of_two + 0.5);
        }
      else
        {
          wlfh[hnd].attr_bits[i] = 0;
        }
    }


  /*  RGB  */

  wlfh[hnd].rgb_pos = wlfh[hnd].attr_pos[WLF_MAX_ATTR - 1] + wlfh[hnd].attr_bits[WLF_MAX_ATTR - 1];
  if (wlfh[hnd].header.max_rgb)
    {
      wlfh[hnd].rgb_bits = NINT (log10 ((NV_FLOAT64) wlfh[hnd].header.max_rgb) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].rgb_bits = 0;
    }


  /*  Reflectance - Note that we're multiplying rgb_bits * 3 for R, G, and B.  */

  wlfh[hnd].reflectance_pos = wlfh[hnd].rgb_pos + wlfh[hnd].rgb_bits * 3;
  if (wlfh[hnd].header.reflectance_scale > 0.0)
    {
      range = (NV_FLOAT64) ((wlfh[hnd].header.max_reflectance - wlfh[hnd].header.min_reflectance) *
                            wlfh[hnd].header.reflectance_scale);
      wlfh[hnd].reflectance_bits = NINT (log10 (range) / log_of_two + 0.5);
    }
  else
    {
      wlfh[hnd].reflectance_bits = 0;
    }


  /*  [CLASSIFICATION BITS] (wlfh[hnd].class_bits) is either read in wlf_open_file or hardwired in wlf_create_file.  */

  wlfh[hnd].class_pos = wlfh[hnd].reflectance_pos + wlfh[hnd].reflectance_bits;


  /*  [STATUS BITS] (wlfh[hnd].status_bits) is either read in wlf_open_file or hardwired in wlf_create_file.  */

  wlfh[hnd].status_pos = wlfh[hnd].class_pos + wlfh[hnd].class_bits;


  /*  [WAVEFORM ADDRESS BITS] (wlfh[hnd].address_bits) is either read in wlf_open_file or hardwired in wlf_create_file.  */

  wlfh[hnd].address_pos = wlfh[hnd].status_pos + wlfh[hnd].status_bits;

 
  /*  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  ADD NEW FIELDS AFTER THIS POINT  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  */




  /*  If we are creating a new file we need to compute the record size in bytes, otherwise it will
      be read from the header.  This allows us to be sort of "forward compatible".  */

  if (creating)
    {
      total_bits = wlfh[hnd].address_pos + wlfh[hnd].address_bits;

      wlfh[hnd].record_size = (NV_FLOAT64) total_bits / 8.0;
      if (total_bits % 8) wlfh[hnd].record_size++;
    }


#ifdef WLF_DEBUG
  fprintf (stderr,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", wlfh[hnd].tv_sec_bits,
           wlfh[hnd].tv_nsec_bits, wlfh[hnd].h_uncert_bits, wlfh[hnd].v_uncert_bits, wlfh[hnd].x_bits, wlfh[hnd].y_bits, wlfh[hnd].z_bits,
           wlfh[hnd].waveform_bits, wlfh[hnd].waveform_point_bits, wlfh[hnd].sensor_x_bits, wlfh[hnd].sensor_y_bits, wlfh[hnd].sensor_z_bits,
           wlfh[hnd].sensor_roll_bits, wlfh[hnd].sensor_pitch_bits, wlfh[hnd].sensor_heading_bits, wlfh[hnd].scan_angle_bits,
           wlfh[hnd].nadir_angle_bits, wlfh[hnd].water_surface_bits, wlfh[hnd].z_offset_bits, wlfh[hnd].point_source_bits, wlfh[hnd].edge_bits,
           wlfh[hnd].return_bits, wlfh[hnd].intensity_bits, wlfh[hnd].rgb_bits, wlfh[hnd].reflectance_bits,
           wlfh[hnd].class_bits, wlfh[hnd].status_bits, wlfh[hnd].address_bits, wlfh[hnd].waveform_blocksize_bits);

  fprintf (stderr,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", wlfh[hnd].tv_sec_pos,
           wlfh[hnd].tv_nsec_pos, wlfh[hnd].h_uncert_pos, wlfh[hnd].v_uncert_pos, wlfh[hnd].x_pos, wlfh[hnd].y_pos, wlfh[hnd].z_pos,
           wlfh[hnd].waveform_pos, wlfh[hnd].waveform_point_pos, wlfh[hnd].sensor_x_pos, wlfh[hnd].sensor_y_pos, wlfh[hnd].sensor_z_pos,
           wlfh[hnd].sensor_roll_pos, wlfh[hnd].sensor_pitch_pos, wlfh[hnd].sensor_heading_pos, wlfh[hnd].scan_angle_pos,
           wlfh[hnd].nadir_angle_pos, wlfh[hnd].water_surface_pos, wlfh[hnd].point_source_pos, wlfh[hnd].edge_pos,
           wlfh[hnd].number_of_returns_pos, wlfh[hnd].return_number_pos, wlfh[hnd].intensity_pos, wlfh[hnd].rgb_pos, wlfh[hnd].reflectance_pos,
           wlfh[hnd].class_pos, wlfh[hnd].status_pos, wlfh[hnd].address_pos, wlfh[hnd].record_size);
#endif
}



/*********************************************************************************************


  Function:    unpack_point_record

  Purpose:     Unpack the bit packed WLF record from the byte buffer.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The WLF file handle
               buffer         -    Unsigned character buffer
               wlf_record     -    The WLF point record to receive the data

  Returns:     N/A

*********************************************************************************************/

static void unpack_point_record (NV_INT32 hnd, NV_U_BYTE *buffer, WLF_RECORD *wlf_record)
{
  NV_INT64 value64;
  NV_INT32 i, value32;


  memset (wlf_record, 0, sizeof (WLF_RECORD));


  /*  Time  */

  wlf_record->tv_sec = wlf_double_bit_unpack (buffer, wlfh[hnd].tv_sec_pos, wlfh[hnd].tv_sec_bits);
  wlf_record->tv_nsec = wlf_bit_unpack (buffer, wlfh[hnd].tv_nsec_pos, wlfh[hnd].tv_nsec_bits);


  /*  Horizontal uncertainty  */

  if (wlfh[hnd].h_uncert_bits)
    {
      value32 = wlf_bit_unpack (buffer, wlfh[hnd].h_uncert_pos, wlfh[hnd].h_uncert_bits);
      wlf_record->horizontal_uncertainty = ((NV_FLOAT32) value32 / wlfh[hnd].header.horizontal_uncertainty_scale) +
        wlfh[hnd].header.min_horizontal_uncertainty;
    }


  /*  Vertical uncertainty  */

  if (wlfh[hnd].v_uncert_bits)
    {
      value32 = wlf_bit_unpack (buffer, wlfh[hnd].v_uncert_pos, wlfh[hnd].v_uncert_bits);
      wlf_record->vertical_uncertainty = ((NV_FLOAT32) value32 / wlfh[hnd].header.vertical_uncertainty_scale) +
        wlfh[hnd].header.min_vertical_uncertainty;
    }


  /*  X  */

  value64 = wlf_double_bit_unpack (buffer, wlfh[hnd].x_pos, wlfh[hnd].x_bits);
  wlf_record->x = ((NV_FLOAT64) value64 / wlfh[hnd].header.x_scale) + wlfh[hnd].header.min_x;


  /*  Y  */

  value64 = wlf_double_bit_unpack (buffer, wlfh[hnd].y_pos, wlfh[hnd].y_bits);
  wlf_record->y = ((NV_FLOAT64) value64 / wlfh[hnd].header.y_scale) + wlfh[hnd].header.min_y;


  /*  Z  */

  value32 = wlf_bit_unpack (buffer, wlfh[hnd].z_pos, wlfh[hnd].z_bits);
  wlf_record->z = ((NV_FLOAT32) value32 / wlfh[hnd].header.z_scale) + wlfh[hnd].header.min_z;
  if (wlf_record->z > wlfh[hnd].header.max_z) wlf_record->z = wlfh[hnd].header.null_z_value;


  /*  Waveform and Waveform Point */

  if (wlfh[hnd].waveform_bits)
    {
      wlf_record->waveform = wlf_bit_unpack (buffer, wlfh[hnd].waveform_pos, wlfh[hnd].waveform_bits);
      wlf_record->waveform_point = wlf_bit_unpack (buffer, wlfh[hnd].waveform_point_pos, wlfh[hnd].waveform_point_bits);
    }


  /*  Sensor Position  */

 if (wlfh[hnd].sensor_x_bits)
   {
     value64 = wlf_double_bit_unpack (buffer, wlfh[hnd].sensor_x_pos, wlfh[hnd].sensor_x_bits);
     wlf_record->sensor_x = ((NV_FLOAT64) value64 / wlfh[hnd].header.sensor_x_scale) + wlfh[hnd].header.sensor_min_x;

     value64 = wlf_double_bit_unpack (buffer, wlfh[hnd].sensor_y_pos, wlfh[hnd].sensor_y_bits);
     wlf_record->sensor_y = ((NV_FLOAT64) value64 / wlfh[hnd].header.sensor_y_scale) + wlfh[hnd].header.sensor_min_y;

     value64 = wlf_bit_unpack (buffer, wlfh[hnd].sensor_z_pos, wlfh[hnd].sensor_z_bits);
     wlf_record->sensor_z = ((NV_FLOAT64) value64 / wlfh[hnd].header.sensor_z_scale) + wlfh[hnd].header.sensor_min_z;
   }


  /*  Sensor Attitude  */

 if (wlfh[hnd].sensor_roll_bits)
   {
     value64 = wlf_bit_unpack (buffer, wlfh[hnd].sensor_roll_pos, wlfh[hnd].sensor_roll_bits);
     wlf_record->sensor_roll = ((NV_FLOAT64) value64 / wlfh[hnd].header.sensor_roll_scale) + wlfh[hnd].header.sensor_min_roll;

     value64 = wlf_bit_unpack (buffer, wlfh[hnd].sensor_pitch_pos, wlfh[hnd].sensor_pitch_bits);
     wlf_record->sensor_pitch = ((NV_FLOAT64) value64 / wlfh[hnd].header.sensor_pitch_scale) + wlfh[hnd].header.sensor_min_pitch;

     value64 = wlf_bit_unpack (buffer, wlfh[hnd].sensor_heading_pos, wlfh[hnd].sensor_heading_bits);
     wlf_record->sensor_heading = ((NV_FLOAT64) value64 / wlfh[hnd].header.sensor_heading_scale) + wlfh[hnd].header.sensor_min_heading;
   }


  /*  Scan angle  */

  if (wlfh[hnd].scan_angle_bits)
    {
      value32 = wlf_bit_unpack (buffer, wlfh[hnd].scan_angle_pos, wlfh[hnd].scan_angle_bits);
      wlf_record->scan_angle = ((NV_FLOAT32) value32 / wlfh[hnd].header.scan_angle_scale) + wlfh[hnd].header.min_scan_angle;
    }


  /*  Nadir angle  */

  if (wlfh[hnd].nadir_angle_bits)
    {
      value32 = wlf_bit_unpack (buffer, wlfh[hnd].nadir_angle_pos, wlfh[hnd].nadir_angle_bits);
      wlf_record->nadir_angle = ((NV_FLOAT32) value32 / wlfh[hnd].header.nadir_angle_scale) + wlfh[hnd].header.min_nadir_angle;
    }


  /*  Water surface  */

  if (wlfh[hnd].water_surface_bits)
    {
      value32 = wlf_bit_unpack (buffer, wlfh[hnd].water_surface_pos, wlfh[hnd].water_surface_bits);
      wlf_record->water_surface = ((NV_FLOAT32) value32 / wlfh[hnd].header.water_surface_scale) + wlfh[hnd].header.min_water_surface;
    }


  /*  Z offset  */

  if (wlfh[hnd].z_offset_bits)
    {
      value32 = wlf_bit_unpack (buffer, wlfh[hnd].z_offset_pos, wlfh[hnd].z_offset_bits);
      wlf_record->z_offset = ((NV_FLOAT32) value32 / wlfh[hnd].header.z_offset_scale) + wlfh[hnd].header.min_z_offset;
    }


  /*  Number of returns and return number (note that we add 1 to the return number since it is stored -1.  */

  if (wlfh[hnd].header.max_number_of_returns > 1)
    {
      wlf_record->number_of_returns = wlf_bit_unpack (buffer, wlfh[hnd].number_of_returns_pos, wlfh[hnd].return_bits);
      wlf_record->return_number = wlf_bit_unpack (buffer, wlfh[hnd].return_number_pos, wlfh[hnd].return_bits) + 1;
    }
  else
    {
      wlf_record->number_of_returns = 1;
      wlf_record->return_number = 1;
    }


  /*  Point source  */

  if (wlfh[hnd].point_source_bits)
    {
      wlf_record->point_source = wlf_bit_unpack (buffer, wlfh[hnd].point_source_pos, wlfh[hnd].point_source_bits);
    }


  /*  Edge of flight line (-1, 0, or 1 but stored as 0, 1, or 2 respectively).  */

  if (wlfh[hnd].edge_bits)
    {
      wlf_record->edge_of_flight_line = (NV_U_BYTE) wlf_bit_unpack (buffer, wlfh[hnd].edge_pos, wlfh[hnd].edge_bits);
      wlf_record->edge_of_flight_line--;
    }


  /*  Intensity  */

  if (wlfh[hnd].intensity_bits)
    {
      value32 = wlf_bit_unpack (buffer, wlfh[hnd].intensity_pos, wlfh[hnd].intensity_bits);
      wlf_record->intensity = ((NV_FLOAT32) value32 / wlfh[hnd].header.intensity_scale) + wlfh[hnd].header.min_intensity;
    }


  /*  Optional attributes  */

  for (i = 0 ; i < WLF_MAX_ATTR ; i++)
    {
      if (wlfh[hnd].attr_bits[i])
        {
          value32 = wlf_bit_unpack (buffer, wlfh[hnd].attr_pos[i], wlfh[hnd].attr_bits[i]);
          wlf_record->attribute[i] = ((NV_FLOAT32) value32 / wlfh[hnd].header.attr_scale[i]) + wlfh[hnd].header.min_attr[i];
        }
    }


  /*  RGB  */

  if (wlfh[hnd].rgb_bits)
    {
      wlf_record->red = wlf_bit_unpack (buffer, wlfh[hnd].rgb_pos, wlfh[hnd].rgb_bits);
      wlf_record->green = wlf_bit_unpack (buffer, wlfh[hnd].rgb_pos + wlfh[hnd].rgb_bits, wlfh[hnd].rgb_bits);
      wlf_record->blue = wlf_bit_unpack (buffer, wlfh[hnd].rgb_pos + wlfh[hnd].rgb_bits * 2, wlfh[hnd].rgb_bits);
    }


  /*  Reflectance  */

  if (wlfh[hnd].reflectance_bits)
    {
      value32 = wlf_bit_unpack (buffer, wlfh[hnd].reflectance_pos, wlfh[hnd].reflectance_bits);
      wlf_record->reflectance = ((NV_FLOAT32) value32 / wlfh[hnd].header.reflectance_scale) + wlfh[hnd].header.min_reflectance;
    }


  /*  Classification  */

  wlf_record->classification = wlf_bit_unpack (buffer, wlfh[hnd].class_pos, wlfh[hnd].class_bits);


  /*  Status  */

  wlf_record->status = wlf_bit_unpack (buffer, wlfh[hnd].status_pos, wlfh[hnd].status_bits);


  /*  Waveform address  */

  if (wlfh[hnd].header.number_of_waveforms)
    wlfh[hnd].waveform_address = wlf_double_bit_unpack (buffer, wlfh[hnd].address_pos, wlfh[hnd].address_bits);


  /*  It doesn't really matter, since the wlf_bit_unpack functions will jump around in the buffer, but
      it would be nice to add new fields after this point.  The important part is in the
      define_record_fields function.  */
}



/*********************************************************************************************

  Function:    write_point_waveform_record

  Purpose:     Bit pack the WLF record and the (optional) waveforms
               into the byte buffer.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The WLF file handle
               wlf_record     -    The WLF point record to receive the data
               wave           -    The waveform structure
               shot_num       -    The shot number

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_WAVE_WRITE_FSEEK_ERROR
                                   WLF_WAVE_WRITE_ERROR
                                   WLF_RECORD_WRITE_FSEEK_ERROR
                                   WLF_RECORD_WRITE_ERROR
                                   WLF_VALUE_OUT_OF_RANGE_ERROR

  Caveats:     The shot number is used to prevent storing redundant waveform data when you
               have multiple returns per shot.  If the shot number doesn't change between
               calls to wlf_append_record then the waveform_address that is stored in the
               first retrun for this shot is stored in all subsequent returns.

*********************************************************************************************/

static NV_INT32 write_point_waveform_record (NV_INT32 hnd, WLF_RECORD wlf_record, NV_INT32 **wave, NV_INT32 shot_num)
{
  NV_INT64 value64;
  NV_INT32 i, j, value32, bit_pos, diff, min_diff, max_diff, num_bits, bytes, range;
  NV_U_BYTE buffer[1024];


  /*  Time  */

  if (wlf_record.tv_sec < wlfh[hnd].min_tv_sec || wlf_record.tv_sec > wlfh[hnd].max_tv_sec)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
#ifdef NVWIN3X
      sprintf (wlf_error.info, _("TV_SEC value %d out of range %I64d to %I64d"), (NV_INT32) wlf_record.tv_sec, wlfh[hnd].min_tv_sec,
               wlfh[hnd].max_tv_sec);
#else
      sprintf (wlf_error.info, _("TV_SEC value %d out of range %lld to %lld"), (NV_INT32) wlf_record.tv_sec, wlfh[hnd].min_tv_sec,
               wlfh[hnd].max_tv_sec);
#endif
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }
  wlf_double_bit_pack (buffer, wlfh[hnd].tv_sec_pos, wlfh[hnd].tv_sec_bits, wlf_record.tv_sec);


  if (wlf_record.tv_nsec < wlfh[hnd].min_tv_nsec || wlf_record.tv_nsec > wlfh[hnd].max_tv_nsec)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("TV_NSEC value %d out of range %d to %d"), (NV_INT32) wlf_record.tv_nsec, wlfh[hnd].min_tv_nsec,
               wlfh[hnd].max_tv_nsec);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }
  wlf_bit_pack (buffer, wlfh[hnd].tv_nsec_pos, wlfh[hnd].tv_nsec_bits, wlf_record.tv_nsec);


  /*  Horizontal uncertainty  */

  if (wlfh[hnd].h_uncert_bits)
    {
      if (wlf_record.horizontal_uncertainty < wlfh[hnd].header.min_horizontal_uncertainty ||
          wlf_record.horizontal_uncertainty > wlfh[hnd].header.max_horizontal_uncertainty)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Horizontal uncertainty value %f out of range %f to %f"), wlf_record.horizontal_uncertainty,
                   wlfh[hnd].header.min_horizontal_uncertainty, wlfh[hnd].header.max_horizontal_uncertainty);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.horizontal_uncertainty - wlfh[hnd].header.min_horizontal_uncertainty) *
                      wlfh[hnd].header.horizontal_uncertainty_scale);
      wlf_bit_pack (buffer, wlfh[hnd].h_uncert_pos, wlfh[hnd].h_uncert_bits, value32);
    }


  /*  Vertical uncertainty  */

  if (wlfh[hnd].v_uncert_bits)
    {
      if (wlf_record.vertical_uncertainty < wlfh[hnd].header.min_vertical_uncertainty ||
          wlf_record.vertical_uncertainty > wlfh[hnd].header.max_vertical_uncertainty)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Vertical uncertainty value %f out of range %f to %f"), wlf_record.vertical_uncertainty,
                   wlfh[hnd].header.min_vertical_uncertainty, wlfh[hnd].header.max_vertical_uncertainty);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.vertical_uncertainty - wlfh[hnd].header.min_vertical_uncertainty) *
                      wlfh[hnd].header.vertical_uncertainty_scale);
      wlf_bit_pack (buffer, wlfh[hnd].v_uncert_pos, wlfh[hnd].v_uncert_bits, value32);
    }


  /*  X  */

  if (wlf_record.x < wlfh[hnd].header.min_x || wlf_record.x > wlfh[hnd].header.max_x)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("X value %f out of range %f to %f"), wlf_record.x, wlfh[hnd].header.min_x, wlfh[hnd].header.max_x);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }

  value64 = NINT64 ((wlf_record.x - wlfh[hnd].header.min_x) * wlfh[hnd].header.x_scale);
  wlf_double_bit_pack (buffer, wlfh[hnd].x_pos, wlfh[hnd].x_bits, value64);


  /*  Y  */

  if (wlf_record.y < wlfh[hnd].header.min_y || wlf_record.y > wlfh[hnd].header.max_y)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("Y value %f out of range %f to %f"), wlf_record.y, wlfh[hnd].header.min_y, wlfh[hnd].header.max_y);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }

  value64 = NINT64 ((wlf_record.y - wlfh[hnd].header.min_y) * wlfh[hnd].header.y_scale);
  wlf_double_bit_pack (buffer, wlfh[hnd].y_pos, wlfh[hnd].y_bits, value64);


  /*  Check for null value.  */

  if (wlf_record.z == wlfh[hnd].header.null_z_value)
    {
      wlf_record.z = wlfh[hnd].header.max_z + 1.0;
      wlf_record.z_offset = 0.0;
    }


  /*  Z offset  */

  if (wlfh[hnd].z_offset_bits)
    {
      if (wlf_record.z_offset < wlfh[hnd].header.min_z_offset || wlf_record.z_offset > wlfh[hnd].header.max_z_offset)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Z offset value %f out of range %f to %f"), wlf_record.z_offset,
                   wlfh[hnd].header.min_z_offset, wlfh[hnd].header.max_z_offset);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.z_offset - wlfh[hnd].header.min_z_offset) * wlfh[hnd].header.z_offset_scale);
      wlf_bit_pack (buffer, wlfh[hnd].z_offset_pos, wlfh[hnd].z_offset_bits, value32);
    }


  /*  Z, first check for Z offset.  */

  if (wlfh[hnd].z_offset_bits && wlf_record.z != wlfh[hnd].header.null_z_value) wlf_record.z += wlf_record.z_offset;

  if (wlf_record.z < wlfh[hnd].header.min_z || wlf_record.z > wlfh[hnd].header.max_z + 1.0)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("Z value %f out of range %f to %f"), wlf_record.z, wlfh[hnd].header.min_z, wlfh[hnd].header.max_z);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }

  value32 = NINT ((wlf_record.z - wlfh[hnd].header.min_z) * wlfh[hnd].header.z_scale);
  wlf_bit_pack (buffer, wlfh[hnd].z_pos, wlfh[hnd].z_bits, value32);


  if (wlf_record.x < wlfh[hnd].header.obs_min_x) wlfh[hnd].header.obs_min_x = wlf_record.x;
  if (wlf_record.x > wlfh[hnd].header.obs_max_x) wlfh[hnd].header.obs_max_x = wlf_record.x;
  if (wlf_record.y < wlfh[hnd].header.obs_min_y) wlfh[hnd].header.obs_min_y = wlf_record.y;
  if (wlf_record.y > wlfh[hnd].header.obs_max_y) wlfh[hnd].header.obs_max_y = wlf_record.y;
  if (wlf_record.z < wlfh[hnd].header.obs_min_z && wlf_record.z != wlfh[hnd].header.null_z_value)
    wlfh[hnd].header.obs_min_z = wlf_record.z;
  if (wlf_record.z > wlfh[hnd].header.obs_max_z && wlf_record.z != wlfh[hnd].header.null_z_value)
    wlfh[hnd].header.obs_max_z = wlf_record.z;
  if (!(wlf_record.status & WLF_INVAL))
    {
      if (wlf_record.x < wlfh[hnd].header.obs_valid_min_x) wlfh[hnd].header.obs_valid_min_x = wlf_record.x;
      if (wlf_record.x > wlfh[hnd].header.obs_valid_max_x) wlfh[hnd].header.obs_valid_max_x = wlf_record.x;
      if (wlf_record.y < wlfh[hnd].header.obs_valid_min_y) wlfh[hnd].header.obs_valid_min_y = wlf_record.y;
      if (wlf_record.y > wlfh[hnd].header.obs_valid_max_y) wlfh[hnd].header.obs_valid_max_y = wlf_record.y;
      if (wlf_record.z < wlfh[hnd].header.obs_valid_min_z && wlf_record.z != wlfh[hnd].header.null_z_value) 
        wlfh[hnd].header.obs_valid_min_z = wlf_record.z;
      if (wlf_record.z > wlfh[hnd].header.obs_valid_max_z && wlf_record.z != wlfh[hnd].header.null_z_value)
        wlfh[hnd].header.obs_valid_max_z = wlf_record.z;
    }


  /*  Waveform and Waveform Point  */

  if (wlfh[hnd].waveform_bits)
    {
      if (wlf_record.waveform > wlfh[hnd].header.number_of_waveforms)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Waveform number %d out of range 0 to %d"), wlf_record.waveform,
                   wlfh[hnd].header.number_of_waveforms);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      wlf_bit_pack (buffer, wlfh[hnd].waveform_pos, wlfh[hnd].waveform_bits, wlf_record.waveform);


      if (wlf_record.waveform_point > wlfh[hnd].header.waveform_count[wlf_record.waveform])
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Waveform point %d in waveform %s out of range 0 to %d"), wlf_record.waveform_point,
                   wlfh[hnd].header.waveform_name[wlf_record.waveform], wlfh[hnd].header.waveform_count[wlf_record.waveform]);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      wlf_bit_pack (buffer, wlfh[hnd].waveform_point_pos, wlfh[hnd].waveform_point_bits, wlf_record.waveform_point);
    }


  /*  Sensor X  */

  if (wlf_record.sensor_x < wlfh[hnd].header.sensor_min_x || wlf_record.sensor_x > wlfh[hnd].header.sensor_max_x)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("Sensor X value %f out of range %f to %f"), wlf_record.sensor_x, wlfh[hnd].header.sensor_min_x,
               wlfh[hnd].header.sensor_max_x);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }

  value64 = NINT64 ((wlf_record.sensor_x - wlfh[hnd].header.sensor_min_x) * wlfh[hnd].header.sensor_x_scale);
  wlf_double_bit_pack (buffer, wlfh[hnd].sensor_x_pos, wlfh[hnd].sensor_x_bits, value64);


  /*  Sensor Y  */

  if (wlf_record.sensor_y < wlfh[hnd].header.sensor_min_y || wlf_record.sensor_y > wlfh[hnd].header.sensor_max_y)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("Sensor Y value %f out of range %f to %f"), wlf_record.sensor_y, wlfh[hnd].header.sensor_min_y,
               wlfh[hnd].header.sensor_max_y);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }

  value64 = NINT64 ((wlf_record.sensor_y - wlfh[hnd].header.sensor_min_y) * wlfh[hnd].header.sensor_y_scale);
  wlf_double_bit_pack (buffer, wlfh[hnd].sensor_y_pos, wlfh[hnd].sensor_y_bits, value64);


  /*  Sensor Z  */

  if (wlfh[hnd].sensor_z_bits)
    {
      if (wlf_record.sensor_z < wlfh[hnd].header.sensor_min_z || wlf_record.sensor_z > wlfh[hnd].header.sensor_max_z)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Sensor Z value %f out of range %f to %f"), wlf_record.sensor_z,
                   wlfh[hnd].header.sensor_min_z, wlfh[hnd].header.sensor_max_z);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.sensor_z - wlfh[hnd].header.sensor_min_z) * wlfh[hnd].header.sensor_z_scale);
      wlf_bit_pack (buffer, wlfh[hnd].sensor_z_pos, wlfh[hnd].sensor_z_bits, value32);
    }


  /*  Sensor Roll  */

  if (wlfh[hnd].sensor_roll_bits)
    {
      if (wlf_record.sensor_roll < wlfh[hnd].header.sensor_min_roll || wlf_record.sensor_roll > wlfh[hnd].header.sensor_max_roll)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Sensor roll value %f out of range %f to %f"), wlf_record.sensor_roll,
                   wlfh[hnd].header.sensor_min_roll, wlfh[hnd].header.sensor_max_roll);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.sensor_roll - wlfh[hnd].header.sensor_min_roll) * wlfh[hnd].header.sensor_roll_scale);
      wlf_bit_pack (buffer, wlfh[hnd].sensor_roll_pos, wlfh[hnd].sensor_roll_bits, value32);
    }


  /*  Sensor Pitch  */

  if (wlfh[hnd].sensor_pitch_bits)
    {
      if (wlf_record.sensor_pitch < wlfh[hnd].header.sensor_min_pitch || wlf_record.sensor_pitch > wlfh[hnd].header.sensor_max_pitch)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Sensor pitch value %f out of range %f to %f"), wlf_record.sensor_pitch,
                   wlfh[hnd].header.sensor_min_pitch, wlfh[hnd].header.sensor_max_pitch);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.sensor_pitch - wlfh[hnd].header.sensor_min_pitch) * wlfh[hnd].header.sensor_pitch_scale);
      wlf_bit_pack (buffer, wlfh[hnd].sensor_pitch_pos, wlfh[hnd].sensor_pitch_bits, value32);
    }


  /*  Sensor Heading  */

  if (wlfh[hnd].sensor_heading_bits)
    {
      if (wlf_record.sensor_heading < wlfh[hnd].header.sensor_min_heading || wlf_record.sensor_heading > wlfh[hnd].header.sensor_max_heading)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Sensor heading value %f out of range %f to %f"), wlf_record.sensor_heading,
                   wlfh[hnd].header.sensor_min_heading, wlfh[hnd].header.sensor_max_heading);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.sensor_heading - wlfh[hnd].header.sensor_min_heading) * wlfh[hnd].header.sensor_heading_scale);
      wlf_bit_pack (buffer, wlfh[hnd].sensor_heading_pos, wlfh[hnd].sensor_heading_bits, value32);
    }


  /*  Scan angle  */

  if (wlfh[hnd].scan_angle_bits)
    {
      if (wlf_record.scan_angle < wlfh[hnd].header.min_scan_angle ||
          wlf_record.scan_angle > wlfh[hnd].header.max_scan_angle)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Scan angle value %f out of range %f to %f"), wlf_record.scan_angle,
                   wlfh[hnd].header.min_scan_angle, wlfh[hnd].header.max_scan_angle);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.scan_angle - wlfh[hnd].header.min_scan_angle) * wlfh[hnd].header.scan_angle_scale);
      wlf_bit_pack (buffer, wlfh[hnd].scan_angle_pos, wlfh[hnd].scan_angle_bits, value32);
    }


  /*  Nadir angle  */

  if (wlfh[hnd].nadir_angle_bits)
    {
      if (wlf_record.nadir_angle < wlfh[hnd].header.min_nadir_angle ||
          wlf_record.nadir_angle > wlfh[hnd].header.max_nadir_angle)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Nadir angle value %f out of range %f to %f"), wlf_record.nadir_angle,
                   wlfh[hnd].header.min_nadir_angle, wlfh[hnd].header.max_nadir_angle);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.nadir_angle - wlfh[hnd].header.min_nadir_angle) * wlfh[hnd].header.nadir_angle_scale);
      wlf_bit_pack (buffer, wlfh[hnd].nadir_angle_pos, wlfh[hnd].nadir_angle_bits, value32);
    }


  /*  Water surface  */

  if (wlfh[hnd].water_surface_bits)
    {
      /*  Check for Z offset.  */

      if (wlfh[hnd].z_offset_bits) wlf_record.water_surface += wlf_record.z_offset;

      if (wlf_record.water_surface < wlfh[hnd].header.min_water_surface ||
          wlf_record.water_surface > wlfh[hnd].header.max_water_surface)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Water surface value %f out of range %f to %f"), wlf_record.water_surface,
                   wlfh[hnd].header.min_water_surface, wlfh[hnd].header.max_water_surface);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.water_surface - wlfh[hnd].header.min_water_surface) * wlfh[hnd].header.water_surface_scale);
      wlf_bit_pack (buffer, wlfh[hnd].water_surface_pos, wlfh[hnd].water_surface_bits, value32);
    }


  /*  Number of returns and return number  */

  if (wlfh[hnd].header.max_number_of_returns > 1)
    {
      if (wlf_record.number_of_returns < 1 || wlf_record.number_of_returns > wlfh[hnd].header.max_number_of_returns)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Number of returns %d out of range 1 to %d"), wlf_record.number_of_returns,
                   wlfh[hnd].header.max_number_of_returns);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      if (wlf_record.return_number < 1 || wlf_record.return_number > wlfh[hnd].header.max_number_of_returns)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Return number %d out of range 1 to %d"), wlf_record.return_number,
                   wlfh[hnd].header.max_number_of_returns);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      wlf_bit_pack (buffer, wlfh[hnd].number_of_returns_pos, wlfh[hnd].return_bits, wlf_record.number_of_returns);
      wlf_bit_pack (buffer, wlfh[hnd].return_number_pos, wlfh[hnd].return_bits, (wlf_record.return_number - 1));
    }


  /*  Point source  */

  if (wlfh[hnd].point_source_bits)
    {
      if (wlf_record.point_source > wlfh[hnd].header.max_point_source_id)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Point source value %d out of range 0 to %d"), wlf_record.point_source,
                   wlfh[hnd].header.max_point_source_id);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      wlf_bit_pack (buffer, wlfh[hnd].point_source_pos, wlfh[hnd].point_source_bits, wlf_record.point_source);
    }


  /*  Edge of flight line (-1, 0, or 1 but stored as 0, 1, or 2 respectively)  */

  if (wlfh[hnd].edge_bits)
    {
      if (wlf_record.edge_of_flight_line < -1 || wlf_record.edge_of_flight_line > 1)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Edge of flight line flag %hd is out of range -1 to 1"), (NV_INT16) wlf_record.edge_of_flight_line);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      wlf_record.edge_of_flight_line++;
      wlf_bit_pack (buffer, wlfh[hnd].edge_pos, wlfh[hnd].edge_bits, wlf_record.edge_of_flight_line);
    }


  /*  Intensity  */

  if (wlfh[hnd].intensity_bits)
    {
      if (wlf_record.intensity < wlfh[hnd].header.min_intensity ||
          wlf_record.intensity > wlfh[hnd].header.max_intensity)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Intensity value %f out of range %f to %f"), wlf_record.intensity,
                   wlfh[hnd].header.min_intensity, wlfh[hnd].header.max_intensity);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.intensity - wlfh[hnd].header.min_intensity) * wlfh[hnd].header.intensity_scale);
      wlf_bit_pack (buffer, wlfh[hnd].intensity_pos, wlfh[hnd].intensity_bits, value32);
    }


  /*  Optional attributes  */

  for (i = 0 ; i < WLF_MAX_ATTR ; i++)
    {
      if (wlfh[hnd].attr_bits[i])
        {
          if (wlf_record.attribute[i] < wlfh[hnd].header.min_attr[i] ||
              wlf_record.attribute[i] > wlfh[hnd].header.max_attr[i])
            {
              wlf_error.system = 0;
              strcpy (wlf_error.file, wlfh[hnd].path);
              sprintf (wlf_error.info, _("Attribute %02d value %f out of range %f to %f"), i + 1, wlf_record.attribute[i],
                       wlfh[hnd].header.min_attr[i], wlfh[hnd].header.max_attr[i]);
              return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
            }

          value32 = NINT ((wlf_record.attribute[i] - wlfh[hnd].header.min_attr[i]) * wlfh[hnd].header.attr_scale[i]);
          wlf_bit_pack (buffer, wlfh[hnd].attr_pos[i], wlfh[hnd].attr_bits[i], value32);
        }
    }


  /*  RGB  */

  if (wlfh[hnd].rgb_bits)
    {
      if (wlf_record.red > wlfh[hnd].header.max_rgb || wlf_record.green > wlfh[hnd].header.max_rgb ||
          wlf_record.blue > wlfh[hnd].header.max_rgb)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("RGB value(s) %d, %d, %d out of range 0 to %d"), wlf_record.red, wlf_record.green,
                   wlf_record.blue, wlfh[hnd].header.max_rgb);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      wlf_bit_pack (buffer, wlfh[hnd].rgb_pos, wlfh[hnd].rgb_bits, wlf_record.red);
      wlf_bit_pack (buffer, wlfh[hnd].rgb_pos + wlfh[hnd].rgb_bits, wlfh[hnd].rgb_bits, wlf_record.green);
      wlf_bit_pack (buffer, wlfh[hnd].rgb_pos + wlfh[hnd].rgb_bits * 2, wlfh[hnd].rgb_bits, wlf_record.blue);
    }


  /*  Reflectance  */

  if (wlfh[hnd].reflectance_bits)
    {
      if (wlf_record.reflectance < wlfh[hnd].header.min_reflectance || wlf_record.reflectance > wlfh[hnd].header.max_reflectance)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Reflectance value %f out of range %f to %f"), wlf_record.reflectance,
                   wlfh[hnd].header.min_reflectance, wlfh[hnd].header.max_reflectance);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((wlf_record.reflectance - wlfh[hnd].header.min_reflectance) * wlfh[hnd].header.reflectance_scale);
      wlf_bit_pack (buffer, wlfh[hnd].reflectance_pos, wlfh[hnd].reflectance_bits, value32);
    }


  /*  Classification  */

  if (wlf_record.classification > wlfh[hnd].max_class)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("Classification value %d out of range 0 to %d"), wlf_record.classification, wlfh[hnd].max_class);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }
  wlf_bit_pack (buffer, wlfh[hnd].class_pos, wlfh[hnd].class_bits, wlf_record.classification);


  /*  Status  */

  if (wlf_record.status > wlfh[hnd].max_status)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("Status value %d out of range 0 to %d"), wlf_record.status, wlfh[hnd].max_status);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }
  wlf_bit_pack (buffer, wlfh[hnd].status_pos, wlfh[hnd].status_bits, wlf_record.status);


  /*  It doesn't really matter, since the wlf_bit_pack functions will jump around in the buffer, but
      it would be nice to add new fields after this point and before the waveform packing.  The
      important part is in the define_record_fields function.  */



  /*  Waveform address and waveforms  */

  if (wlfh[hnd].header.number_of_waveforms)
    {
      /*  Only store the waveforms if we're on a new shot.  */

      if (wlfh[hnd].prev_shot_num != shot_num)
        {
          if (fseeko64 (wlfh[hnd].wfp, 0LL, SEEK_END) < 0)
            {
              wlf_error.system = errno;
              strcpy (wlf_error.file, wlfh[hnd].wave_path);
              return (wlf_error.wlf = WLF_WAVE_WRITE_FSEEK_ERROR);
            }

          wlfh[hnd].waveform_address = ftello64 (wlfh[hnd].wfp);

          if (wlfh[hnd].waveform_address > wlfh[hnd].max_address)
            {
              wlf_error.system = 0;
              strcpy (wlf_error.file, wlfh[hnd].wave_path);

#ifdef NVWIN3X
              sprintf (wlf_error.info, _("Waveform address value %I64d greater than max address size %I64d"),
                       wlfh[hnd].waveform_address, wlfh[hnd].max_address);
#else
              sprintf (wlf_error.info, _("Waveform address value %lld greater than max address size %lld"),
                       wlfh[hnd].waveform_address, wlfh[hnd].max_address);
#endif

              return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
            }

          wlf_double_bit_pack (buffer, wlfh[hnd].address_pos, wlfh[hnd].address_bits, wlfh[hnd].waveform_address);


          bit_pos = wlfh[hnd].waveform_blocksize_bits;
          for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++)
            {
              wlfh[hnd].waveform_start_value_size = 8;
              range = wlfh[hnd].header.max_waveform[i] - wlfh[hnd].header.min_waveform[i];
              if (range > 255) wlfh[hnd].waveform_start_value_size = 16;
              if (range > 65535) wlfh[hnd].waveform_start_value_size = 32;


              /*  Range check the first value.  */

              if (wave[i][0] < wlfh[hnd].header.min_waveform[i] || wave[i][0] > wlfh[hnd].header.max_waveform[i])
                {
                  wlf_error.system = 0;
                  strcpy (wlf_error.file, wlfh[hnd].path);
                  sprintf (wlf_error.info, _("%s waveform value %d out of range %d to %d"), wlfh[hnd].header.waveform_name[i], 
                           wave[i][0], wlfh[hnd].header.min_waveform[i], wlfh[hnd].header.max_waveform[i]);
                  return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
                }


              min_diff = 999999;
              max_diff = -999999;
              for (j = 1 ; j < wlfh[hnd].header.waveform_count[i] ; j++)
                {
                  /*  Range check.  */

                  if (wave[i][j] < wlfh[hnd].header.min_waveform[i] || wave[i][j] > wlfh[hnd].header.max_waveform[i])
                    {
                      wlf_error.system = 0;
                      strcpy (wlf_error.file, wlfh[hnd].path);
                      sprintf (wlf_error.info, _("%s waveform value %d out of range %d to %d"), wlfh[hnd].header.waveform_name[i], 
                               wave[i][j], wlfh[hnd].header.min_waveform[i], wlfh[hnd].header.max_waveform[i]);
                      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
                    }

                  diff = wave[i][j] - wave[i][j - 1];
                  if (diff < min_diff) min_diff = diff;
                  if (diff > max_diff) max_diff = diff;
                }


              /*  Just in case someone sends us a totally flat array of points.  */

              if (!(max_diff - min_diff))
                {
                  num_bits = 0;
                }
              else
                {
                  num_bits = NINT (log10 ((NV_FLOAT64) (max_diff - min_diff)) / log_of_two + 0.5);
                }


              wlf_bit_pack (wlfh[hnd].wave_buffer, bit_pos, wlfh[hnd].waveform_start_value_size, wave[i][0] - wlfh[hnd].header.min_waveform[i]);
              bit_pos += wlfh[hnd].waveform_start_value_size;
              wlf_bit_pack (wlfh[hnd].wave_buffer, bit_pos, wlfh[hnd].waveform_start_value_size, min_diff);
              bit_pos += wlfh[hnd].waveform_start_value_size;
              wlf_bit_pack (wlfh[hnd].wave_buffer, bit_pos, 5, num_bits);
              bit_pos += 5;


              if (num_bits)
                {
                  for (j = 1 ; j < wlfh[hnd].header.waveform_count[i] ; j++)
                    {
                      diff = (wave[i][j] - wave[i][j - 1]) - min_diff;

                      wlf_bit_pack (wlfh[hnd].wave_buffer, bit_pos, num_bits, diff); bit_pos += num_bits;
                    }
                }
            }

          bytes = (NV_FLOAT64) bit_pos / 8.0;
          if (bit_pos % 8) bytes++;


          /*  Compute the total bytes used for the waveforms so far.  */

          wlfh[hnd].waveform_total_bytes += bytes;


          wlf_bit_pack (wlfh[hnd].wave_buffer, 0, wlfh[hnd].waveform_blocksize_bits, bytes);


          if (!fwrite (wlfh[hnd].wave_buffer, bytes, 1, wlfh[hnd].wfp))
            {
              wlf_error.system = errno;
              strcpy (wlf_error.file, wlfh[hnd].wave_path);
              return (wlf_error.wlf = WLF_WAVE_WRITE_ERROR);
            }
        }
      else
        {
          wlfh[hnd].waveform_address = wlfh[hnd].prev_waveform_address;
          wlf_double_bit_pack (buffer, wlfh[hnd].address_pos, wlfh[hnd].address_bits, wlfh[hnd].waveform_address);
        }


      /*  Save the previously stored shot number and waveform address just in case we have multiple returns per shot.  */

      wlfh[hnd].prev_shot_num = shot_num;
      wlfh[hnd].prev_waveform_address = wlfh[hnd].waveform_address;
    }


  if (fseeko64 (wlfh[hnd].fp, 0LL, SEEK_END) < 0)
    {
      wlf_error.system = errno;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_RECORD_WRITE_FSEEK_ERROR);
    }

  if (!fwrite (buffer, wlfh[hnd].record_size, 1, wlfh[hnd].fp))
    {
      wlf_error.system = errno;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_RECORD_WRITE_ERROR);
    }


  wlf_error.system = 0;
  return (wlf_error.wlf = WLF_SUCCESS);
}



/*********************************************************************************************

  Function:    wlf_write_header

  Purpose:     Write the wlf_header to the WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The WLF file handle

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_HEADER_WRITE_FSEEK_ERROR
                                   WLF_HEADER_WRITE_ERROR

*********************************************************************************************/

static NV_INT32 wlf_write_header (NV_INT32 hnd)
{
  NV_CHAR space = ' ';
  NV_INT32 i, size, year, jday, hour, minute, month, day;
  NV_FLOAT32 second;


  if (fseeko64 (wlfh[hnd].fp, 0LL, SEEK_SET) < 0)
    {
      wlf_error.system = errno;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_HEADER_WRITE_FSEEK_ERROR);
    }



  fprintf (wlfh[hnd].fp, N_("[VERSION] = %s\n"), WLF_VERSION);

  fprintf (wlfh[hnd].fp, N_("[FILE SOURCE ID] = %d\n"), wlfh[hnd].header.file_source_id);

  wlf_cvtime (wlfh[hnd].header.creation_tv_sec, wlfh[hnd].header.creation_tv_nsec, &year, &jday, &hour, &minute, &second);
  wlf_jday2mday (year, jday, &month, &day);
  month++;

  fprintf (wlfh[hnd].fp, N_("[CREATION YEAR] = %d\n"), year + 1900);
  fprintf (wlfh[hnd].fp, N_("[CREATION MONTH] = %02d\n"), month);
  fprintf (wlfh[hnd].fp, N_("[CREATION DAY] = %02d\n"), day);
  fprintf (wlfh[hnd].fp, N_("[CREATION DAY OF YEAR] = %03d\n"), jday);
  fprintf (wlfh[hnd].fp, N_("[CREATION HOUR] = %02d\n"), hour);
  fprintf (wlfh[hnd].fp, N_("[CREATION MINUTE] = %02d\n"), minute);
  fprintf (wlfh[hnd].fp, N_("[CREATION SECOND] = %5.2f\n"), second);
  if (strlen (wlfh[hnd].header.creation_software) > 2) fprintf (wlfh[hnd].fp, N_("[CREATION SOFTWARE] = %s\n"),
                                                                wlfh[hnd].header.creation_software);


  wlf_cvtime (wlfh[hnd].header.modification_tv_sec, wlfh[hnd].header.modification_tv_nsec, &year, &jday, &hour, &minute, &second);
  wlf_jday2mday (year, jday, &month, &day);
  month++;

  fprintf (wlfh[hnd].fp, N_("[MODIFICATION YEAR] = %d\n"), year + 1900);
  fprintf (wlfh[hnd].fp, N_("[MODIFICATION MONTH] = %02d\n"), month);
  fprintf (wlfh[hnd].fp, N_("[MODIFICATION DAY] = %02d\n"), day);
  fprintf (wlfh[hnd].fp, N_("[MODIFICATION DAY OF YEAR] = %03d\n"), jday);
  fprintf (wlfh[hnd].fp, N_("[MODIFICATION HOUR] = %02d\n"), hour);
  fprintf (wlfh[hnd].fp, N_("[MODIFICATION MINUTE] = %02d\n"), minute);
  fprintf (wlfh[hnd].fp, N_("[MODIFICATION SECOND] = %5.2f\n"), second);
  if (strlen (wlfh[hnd].header.modification_software) > 2) fprintf (wlfh[hnd].fp, N_("[MODIFICATION SOFTWARE] = %s\n"),
                                                                    wlfh[hnd].header.modification_software);

  if (strlen (wlfh[hnd].header.security_classification) > 2) fprintf (wlfh[hnd].fp, N_("[SECURITY CLASSIFICATION] = %s\n"),
                                                             wlfh[hnd].header.security_classification);
  if (strlen (wlfh[hnd].header.distribution) > 2) fprintf (wlfh[hnd].fp, N_("{DISTRIBUTION = \n%s\n}\n"),
                                                           wlfh[hnd].header.distribution);
  if (strlen (wlfh[hnd].header.declassification) > 2) fprintf (wlfh[hnd].fp, N_("{DECLASSIFICATION = \n%s\n}\n"),
                                                               wlfh[hnd].header.declassification);
  if (strlen (wlfh[hnd].header.class_just) > 2) fprintf (wlfh[hnd].fp, N_("{SECURITY CLASSIFICATION JUSTIFICATION = \n%s\n}\n"),
                                                         wlfh[hnd].header.class_just);
  if (strlen (wlfh[hnd].header.downgrade) > 2) fprintf (wlfh[hnd].fp, N_("{DOWNGRADE = \n%s\n}\n"),
                                                        wlfh[hnd].header.downgrade);

  if (strlen (wlfh[hnd].header.source) > 2) fprintf (wlfh[hnd].fp, N_("[SOURCE] = %s\n"), wlfh[hnd].header.source);
  if (strlen (wlfh[hnd].header.system) > 2) fprintf (wlfh[hnd].fp, N_("[SYSTEM] = %s\n"), wlfh[hnd].header.system);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED MIN X] = %.11f\n"), wlfh[hnd].header.obs_min_x);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED MIN Y] = %.11f\n"), wlfh[hnd].header.obs_min_y);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED MIN Z] = %.9f\n"), wlfh[hnd].header.obs_min_z);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED MAX X] = %.11f\n"), wlfh[hnd].header.obs_max_x);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED MAX Y] = %.11f\n"), wlfh[hnd].header.obs_max_y);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED MAX Z] = %.9f\n"), wlfh[hnd].header.obs_max_z);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED VALID MIN X] = %.11f\n"), wlfh[hnd].header.obs_valid_min_x);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED VALID MIN Y] = %.11f\n"), wlfh[hnd].header.obs_valid_min_y);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED VALID MIN Z] = %.9f\n"), wlfh[hnd].header.obs_valid_min_z);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED VALID MAX X] = %.11f\n"), wlfh[hnd].header.obs_valid_max_x);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED VALID MAX Y] = %.11f\n"), wlfh[hnd].header.obs_valid_max_y);
  fprintf (wlfh[hnd].fp, N_("[OBSERVED VALID MAX Z] = %.9f\n"), wlfh[hnd].header.obs_valid_max_z);

  fprintf (wlfh[hnd].fp, N_("[NUMBER OF RECORDS] = %d\n"), wlfh[hnd].header.number_of_records);
  if (strlen (wlfh[hnd].header.project) > 2) fprintf (wlfh[hnd].fp, N_("[PROJECT] = %s\n"), wlfh[hnd].header.project);
  if (strlen (wlfh[hnd].header.mission) > 2) fprintf (wlfh[hnd].fp, N_("[MISSION] = %s\n"), wlfh[hnd].header.mission);
  if (strlen (wlfh[hnd].header.dataset) > 2) fprintf (wlfh[hnd].fp, N_("[DATASET] = %s\n"), wlfh[hnd].header.dataset);
  if (strlen (wlfh[hnd].header.flight_id) > 2) fprintf (wlfh[hnd].fp, N_("[FLIGHT ID] = %s\n"),
                                                        wlfh[hnd].header.flight_id);


  if (wlfh[hnd].header.flight_start_tv_nsec)
    {
      wlf_cvtime (wlfh[hnd].header.flight_start_tv_sec, wlfh[hnd].header.flight_start_tv_nsec, &year, &jday, &hour, 
              &minute, &second);
      wlf_jday2mday (year, jday, &month, &day);
      month++;

      fprintf (wlfh[hnd].fp, N_("[FLIGHT START YEAR] = %d\n"), year + 1900);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT START MONTH] = %02d\n"), month);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT START DAY] = %02d\n"), day);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT START DAY OF YEAR] = %03d\n"), jday);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT START HOUR] = %02d\n"), hour);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT START MINUTE] = %02d\n"), minute);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT START SECOND] = %5.2f\n"), second);
    }


  if (wlfh[hnd].header.flight_end_tv_nsec)
    {
      wlf_cvtime (wlfh[hnd].header.flight_end_tv_sec, wlfh[hnd].header.flight_end_tv_nsec, &year, &jday, &hour, &minute,
              &second);
      wlf_jday2mday (year, jday, &month, &day);
      month++;

      fprintf (wlfh[hnd].fp, N_("[FLIGHT END YEAR] = %d\n"), year + 1900);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT END MONTH] = %02d\n"), month);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT END DAY] = %02d\n"), day);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT END DAY OF YEAR] = %03d\n"), jday);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT END HOUR] = %02d\n"), hour);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT END MINUTE] = %02d\n"), minute);
      fprintf (wlfh[hnd].fp, N_("[FLIGHT END SECOND] = %5.2f\n"), second);
    }


  for (i = 0 ; i < 5 ; i++)
    {
      if (strlen (wlfh[hnd].header.wlf_user_flag_name[i]) > 2) fprintf (wlfh[hnd].fp, N_("[WLF USER FLAG %02d NAME] = %s\n"),
                                                                        i + 1, wlfh[hnd].header.wlf_user_flag_name[i]);
    }

  if (strlen (wlfh[hnd].header.comments) > 2) fprintf (wlfh[hnd].fp, N_("{COMMENTS = \n%s\n}\n"), wlfh[hnd].header.comments);

  switch (wlfh[hnd].header.z_units)
    {
    case WLF_METERS:
    default:
      fprintf (wlfh[hnd].fp, N_("[Z UNITS] = METERS\n"));
      break;

    case WLF_FEET:
      fprintf (wlfh[hnd].fp, N_("[Z UNITS] = FEET\n"));
      break;

    case WLF_FATHOMS:
      fprintf (wlfh[hnd].fp, N_("[Z UNITS] = FATHOMS\n"));
      break;
    }

  fprintf (wlfh[hnd].fp, N_("[NULL Z VALUE] = %.5f\n"), wlfh[hnd].header.null_z_value);

  fprintf (wlfh[hnd].fp, N_("[MAX NUMBER OF RETURNS PER RECORD] = %d\n"), wlfh[hnd].header.max_number_of_returns);

  fprintf (wlfh[hnd].fp, N_("{WELL-KNOWN TEXT = \n%s\n}\n"), wlfh[hnd].header.wkt);

  fprintf (wlfh[hnd].fp, N_("[MIN X] = %.11f\n"), wlfh[hnd].header.min_x);
  fprintf (wlfh[hnd].fp, N_("[MAX X] = %.11f\n"), wlfh[hnd].header.max_x);
  fprintf (wlfh[hnd].fp, N_("[X SCALE] = %.11f\n"), wlfh[hnd].header.x_scale);
  fprintf (wlfh[hnd].fp, N_("[MIN Y] = %.11f\n"), wlfh[hnd].header.min_y);
  fprintf (wlfh[hnd].fp, N_("[MAX Y] = %.11f\n"), wlfh[hnd].header.max_y);
  fprintf (wlfh[hnd].fp, N_("[Y SCALE] = %.11f\n"), wlfh[hnd].header.y_scale);
  fprintf (wlfh[hnd].fp, N_("[MIN Z] = %.9f\n"), wlfh[hnd].header.min_z);
  fprintf (wlfh[hnd].fp, N_("[MAX Z] = %.9f\n"), wlfh[hnd].header.max_z);
  fprintf (wlfh[hnd].fp, N_("[Z SCALE] = %.9f\n"), wlfh[hnd].header.z_scale);


  /*  Only need to check one of these since sensor position is an all or nothing situation.  */

  if (wlfh[hnd].sensor_x_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[SENSOR MIN X] = %.11f\n"), wlfh[hnd].header.sensor_min_x);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MAX X] = %.11f\n"), wlfh[hnd].header.sensor_max_x);
      fprintf (wlfh[hnd].fp, N_("[SENSOR X SCALE] = %.11f\n"), wlfh[hnd].header.sensor_x_scale);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MIN Y] = %.11f\n"), wlfh[hnd].header.sensor_min_y);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MAX Y] = %.11f\n"), wlfh[hnd].header.sensor_max_y);
      fprintf (wlfh[hnd].fp, N_("[SENSOR Y SCALE] = %.11f\n"), wlfh[hnd].header.sensor_y_scale);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MIN Z] = %.9f\n"), wlfh[hnd].header.sensor_min_z);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MAX Z] = %.9f\n"), wlfh[hnd].header.sensor_max_z);
      fprintf (wlfh[hnd].fp, N_("[SENSOR Z SCALE] = %.9f\n"), wlfh[hnd].header.sensor_z_scale);
    }


  /*  Only need to check one of these since sensor attitude is an all or nothing situation.  */

  if (wlfh[hnd].sensor_roll_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[SENSOR MIN ROLL] = %.9f\n"), wlfh[hnd].header.sensor_min_roll);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MAX ROLL] = %.9f\n"), wlfh[hnd].header.sensor_max_roll);
      fprintf (wlfh[hnd].fp, N_("[SENSOR ROLL SCALE] = %.9f\n"), wlfh[hnd].header.sensor_roll_scale);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MIN PITCH] = %.9f\n"), wlfh[hnd].header.sensor_min_pitch);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MAX PITCH] = %.9f\n"), wlfh[hnd].header.sensor_max_pitch);
      fprintf (wlfh[hnd].fp, N_("[SENSOR PITCH SCALE] = %.9f\n"), wlfh[hnd].header.sensor_pitch_scale);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MIN HEADING] = %.9f\n"), wlfh[hnd].header.sensor_min_heading);
      fprintf (wlfh[hnd].fp, N_("[SENSOR MAX HEADING] = %.9f\n"), wlfh[hnd].header.sensor_max_heading);
      fprintf (wlfh[hnd].fp, N_("[SENSOR HEADING SCALE] = %.9f\n"), wlfh[hnd].header.sensor_heading_scale);
    }

  if (wlfh[hnd].scan_angle_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[MIN SCAN ANGLE] = %.9f\n"), wlfh[hnd].header.min_scan_angle);
      fprintf (wlfh[hnd].fp, N_("[MAX SCAN ANGLE] = %.9f\n"), wlfh[hnd].header.max_scan_angle);
      fprintf (wlfh[hnd].fp, N_("[SCAN ANGLE SCALE] = %.9f\n"), wlfh[hnd].header.scan_angle_scale);
    }

  if (wlfh[hnd].nadir_angle_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[MIN NADIR ANGLE] = %.9f\n"), wlfh[hnd].header.min_nadir_angle);
      fprintf (wlfh[hnd].fp, N_("[MAX NADIR ANGLE] = %.9f\n"), wlfh[hnd].header.max_nadir_angle);
      fprintf (wlfh[hnd].fp, N_("[NADIR ANGLE SCALE] = %.9f\n"), wlfh[hnd].header.nadir_angle_scale);
    }

  if (wlfh[hnd].water_surface_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[MIN WATER SURFACE] = %.9f\n"), wlfh[hnd].header.min_water_surface);
      fprintf (wlfh[hnd].fp, N_("[MAX WATER SURFACE] = %.9f\n"), wlfh[hnd].header.max_water_surface);
      fprintf (wlfh[hnd].fp, N_("[WATER SURFACE SCALE] = %.9f\n"), wlfh[hnd].header.water_surface_scale);
    }

  if (wlfh[hnd].z_offset_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[MIN Z OFFSET] = %.9f\n"), wlfh[hnd].header.min_z_offset);
      fprintf (wlfh[hnd].fp, N_("[MAX Z OFFSET] = %.9f\n"), wlfh[hnd].header.max_z_offset);
      fprintf (wlfh[hnd].fp, N_("[Z OFFSET SCALE] = %.9f\n"), wlfh[hnd].header.z_offset_scale);
      fprintf (wlfh[hnd].fp, N_("[Z OFFSET DATUM] = %s\n"), wlfh[hnd].header.z_offset_datum);
    }

  if (wlfh[hnd].point_source_bits) fprintf (wlfh[hnd].fp, N_("[MAX POINT SOURCE ID] = %d\n"), wlfh[hnd].header.max_point_source_id);

  if (wlfh[hnd].edge_bits) fprintf (wlfh[hnd].fp, N_("[EDGE FLAG PRESENT] = %hd\n"), (NV_INT16) wlfh[hnd].header.edge_flag_present);

  if (wlfh[hnd].intensity_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[MIN INTENSITY] = %.9f\n"), wlfh[hnd].header.min_intensity);
      fprintf (wlfh[hnd].fp, N_("[MAX INTENSITY] = %.9f\n"), wlfh[hnd].header.max_intensity);
      fprintf (wlfh[hnd].fp, N_("[INTENSITY SCALE] = %.9f\n"), wlfh[hnd].header.intensity_scale);
    }

  for (i = 0 ; i < WLF_MAX_ATTR ; i++)
    {
      if (wlfh[hnd].attr_bits[i])
        {
          fprintf (wlfh[hnd].fp, N_("[ATTRIBUTE NAME %02d] = %s\n"), i + 1, wlfh[hnd].header.attr_name[i]);
          fprintf (wlfh[hnd].fp, N_("[MIN ATTRIBUTE %02d] = %.9f\n"), i + 1, wlfh[hnd].header.min_attr[i]);
          fprintf (wlfh[hnd].fp, N_("[MAX ATTRIBUTE %02d] = %.9f\n"), i + 1, wlfh[hnd].header.max_attr[i]);
          fprintf (wlfh[hnd].fp, N_("[ATTRIBUTE SCALE %02d] = %.9f\n"), i + 1, wlfh[hnd].header.attr_scale[i]);
        }
    }

  if (wlfh[hnd].rgb_bits) fprintf (wlfh[hnd].fp, N_("[MAX RGB] = %d\n"), wlfh[hnd].header.max_rgb);

  if (wlfh[hnd].reflectance_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[MIN REFLECTANCE] = %.9f\n"), wlfh[hnd].header.min_reflectance);
      fprintf (wlfh[hnd].fp, N_("[MAX REFLECTANCE] = %.9f\n"), wlfh[hnd].header.max_reflectance);
      fprintf (wlfh[hnd].fp, N_("[REFLECTANCE SCALE] = %.9f\n"), wlfh[hnd].header.reflectance_scale);
    }

  if (wlfh[hnd].h_uncert_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[MIN HORIZONTAL UNCERTAINTY] = %.9f\n"), wlfh[hnd].header.min_horizontal_uncertainty);
      fprintf (wlfh[hnd].fp, N_("[MAX HORIZONTAL UNCERTAINTY] = %.9f\n"), wlfh[hnd].header.max_horizontal_uncertainty);
      fprintf (wlfh[hnd].fp, N_("[HORIZONTAL UNCERTAINTY SCALE] = %.9f\n"), wlfh[hnd].header.horizontal_uncertainty_scale);
    }

  if (wlfh[hnd].v_uncert_bits)
    {
      fprintf (wlfh[hnd].fp, N_("[MIN VERTICAL UNCERTAINTY] = %.9f\n"), wlfh[hnd].header.min_vertical_uncertainty);
      fprintf (wlfh[hnd].fp, N_("[MAX VERTICAL UNCERTAINTY] = %.9f\n"), wlfh[hnd].header.max_vertical_uncertainty);
      fprintf (wlfh[hnd].fp, N_("[VERTICAL UNCERTAINTY SCALE] = %.9f\n"), wlfh[hnd].header.vertical_uncertainty_scale);
    }

  if (wlfh[hnd].header.number_of_waveforms)
    {
      fprintf (wlfh[hnd].fp, N_("[NUMBER OF WAVEFORMS] = %hd\n"), (NV_INT16) wlfh[hnd].header.number_of_waveforms);

      for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++)
        {
          fprintf (wlfh[hnd].fp, N_("[WAVEFORM %02d NAME] = %s\n"), i + 1, wlfh[hnd].header.waveform_name[i]);
          fprintf (wlfh[hnd].fp, N_("[MIN WAVEFORM %02d] = %d\n"), i + 1, wlfh[hnd].header.min_waveform[i]);
          fprintf (wlfh[hnd].fp, N_("[MAX WAVEFORM %02d] = %d\n"), i + 1, wlfh[hnd].header.max_waveform[i]);
          fprintf (wlfh[hnd].fp, N_("[WAVEFORM %02d COUNT] = %d\n"), i + 1, wlfh[hnd].header.waveform_count[i]);
        }
    }


  fprintf (wlfh[hnd].fp, N_("[POSIX TIME SECOND BITS] = %d\n"), wlfh[hnd].tv_sec_bits);
  fprintf (wlfh[hnd].fp, N_("[POSIX TIME NANOSECOND BITS] = %d\n"), wlfh[hnd].tv_nsec_bits);
  fprintf (wlfh[hnd].fp, N_("[CLASSIFICATION BITS] = %d\n"), wlfh[hnd].class_bits);
  fprintf (wlfh[hnd].fp, N_("[STATUS BITS] = %d\n"), wlfh[hnd].status_bits);
  fprintf (wlfh[hnd].fp, N_("[WAVEFORM ADDRESS BITS] = %d\n"), wlfh[hnd].address_bits);
  fprintf (wlfh[hnd].fp, N_("[WAVEFORM BLOCKSIZE BITS] = %d\n"), wlfh[hnd].waveform_blocksize_bits);
#ifdef NVWIN3X
  fprintf (wlfh[hnd].fp, N_("[WAVEFORM BLOCK OFFSET] = %I64d\n"), wlfh[hnd].waveform_block_offset);
#else
  fprintf (wlfh[hnd].fp, N_("[WAVEFORM BLOCK OFFSET] = %lld\n"), wlfh[hnd].waveform_block_offset);
#endif
  wlfh[hnd].header_size = WLF_HEADER_SIZE;
  fprintf (wlfh[hnd].fp, N_("[HEADER SIZE] = %d\n"), wlfh[hnd].header_size);
  fprintf (wlfh[hnd].fp, N_("[RECORD SIZE] = %d\n"), wlfh[hnd].record_size);

  fprintf (wlfh[hnd].fp, N_("[END OF HEADER]\n"));


  /*  If the user flags weren't defined, set the default names.  */

  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[0]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[0], "WLF_USER_01");
  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[1]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[1], "WLF_USER_02");
  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[2]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[2], "WLF_USER_03");
  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[3]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[3], "WLF_USER_04");
  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[4]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[4], "WLF_USER_05");


  /*  Space fill the rest.  */

  size = wlfh[hnd].header_size - ftell (wlfh[hnd].fp);


  for (i = 0 ; i < size ; i++)
    {
      if (!fwrite (&space, 1, 1, wlfh[hnd].fp))
        {
          wlf_error.system = errno;
          strcpy (wlf_error.file, wlfh[hnd].path);
          return (wlf_error.wlf = WLF_HEADER_WRITE_ERROR);
        }
    }

  wlf_error.system = 0;
  return (wlf_error.wlf = WLF_SUCCESS);
}



/*********************************************************************************************

  Function:    wlf_create_file

  Purpose:     Create a WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   path           -    The WLF file path
               wlf_header     -    WLF_HEADER structure to
                                   be written to the file

  Returns:     NV_INT32       -    The file handle (0 or positive) or
                                   WLF_TOO_MANY_OPEN_FILES
                                   WLF_CREATE_ERROR
                                   WLF_CREATE_WAVE_ERROR
                                   WLF_HEADER_WRITE_FSEEK_ERROR
                                   WLF_HEADER_WRITE_ERROR
                                   WLF_SENSOR_POSITION_RANGE_ERROR
                                   WLF_SENSOR_ATTITUDE_RANGE_ERROR

*********************************************************************************************/

WLF_DLL NV_INT32 wlf_create_file (const NV_CHAR *path, WLF_HEADER wlf_header)
{
  NV_INT32 i, hnd, bytes;


  /*  The first time through we want to initialize (zero) the WLF handle array.  */

  if (first)
    {
      for (i = 0 ; i < WLF_MAX_FILES ; i++) 
        {
          memset (&wlfh[i], 0, sizeof (INTERNAL_WLF_STRUCT));
          wlfh[i].fp = NULL;
          wlfh[i].prev_shot_num = -1;
        }

      log_of_two = log10 (2.0L);


      /*  Set up the SIGINT handler.  */

      signal (SIGINT, sigint_handler);


      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = WLF_MAX_FILES;
  for (i = 0 ; i < WLF_MAX_FILES ; i++)
    {
      if (wlfh[i].fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == WLF_MAX_FILES) return (wlf_error.wlf = WLF_TOO_MANY_OPEN_FILES);


  /*  Hardwire the bit sizes for fixed fields.  */

  wlfh[hnd].tv_sec_bits = WLF_TV_SEC_BITS;
  wlfh[hnd].tv_nsec_bits = WLF_TV_NSEC_BITS;
  wlfh[hnd].class_bits = WLF_CLASS_BITS;
  wlfh[hnd].status_bits = WLF_STATUS_BITS;
  wlfh[hnd].waveform_blocksize_bits = WLF_WAVEFORM_BLOCKSIZE_BITS;
  if (wlf_header.number_of_waveforms)
    {
      wlfh[hnd].address_bits = WLF_ADDRESS_BITS;
    }
  else
    {
      wlfh[hnd].address_bits = 0;
    }


  /*  Compute min and max sizes for hardwired fields.  */

  wlfh[hnd].min_tv_sec = -NINT64 (pow (2.0, (NV_FLOAT64) (wlfh[hnd].tv_sec_bits - 1)));
  wlfh[hnd].max_tv_sec = NINT64 (pow (2.0, (NV_FLOAT64) (wlfh[hnd].tv_sec_bits - 1))) - 1;
  wlfh[hnd].min_tv_nsec = -NINT (pow (2.0, (NV_FLOAT64) (wlfh[hnd].tv_nsec_bits - 1)));
  wlfh[hnd].max_tv_nsec = NINT (pow (2.0, (NV_FLOAT64) (wlfh[hnd].tv_nsec_bits - 1))) - 1;
  wlfh[hnd].max_class = NINT (pow (2.0, (NV_FLOAT64) wlfh[hnd].class_bits)) - 1;
  wlfh[hnd].max_status = NINT (pow (2.0, (NV_FLOAT64) wlfh[hnd].status_bits)) - 1;
  wlfh[hnd].max_address = NINT64 (pow (2.0, (NV_FLOAT64) wlfh[hnd].address_bits)) - 1;


  /*  Open the file and write the header.  */

  if ((wlfh[hnd].fp = fopen64 (path, "wb+")) == NULL)
    {
      wlf_error.system = errno;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_CREATE_ERROR);
    }


  /*  Open the associated waveform file if we need it.  */

  if (hnd >= 0 && wlf_header.number_of_waveforms)
    {
      sprintf (wlfh[hnd].wave_path, "%s.tmpwave", path);

      if ((wlfh[hnd].wfp = fopen64 (wlfh[hnd].wave_path, "wb+")) == NULL)
        {
          wlf_error.system = errno;
          strcpy (wlf_error.file, wlfh[hnd].wave_path);
          return (wlf_error.wlf = WLF_CREATE_WAVE_ERROR);
        }
    }


  /*  Save the file name for error messages.  */

  strcpy (wlfh[hnd].path, path);


  wlfh[hnd].header = wlf_header;


  /*  Set the observed min and max values to some ridiculous number so they'll get replaced immediately.  */

  wlfh[hnd].header.obs_min_x = wlfh[hnd].header.obs_min_y = wlfh[hnd].header.obs_valid_min_x =
    wlfh[hnd].header.obs_valid_min_y = 1000000000000000.0;
  wlfh[hnd].header.obs_max_x = wlfh[hnd].header.obs_max_y = wlfh[hnd].header.obs_valid_max_x =
    wlfh[hnd].header.obs_valid_max_y = -1000000000000000.0;
  wlfh[hnd].header.obs_min_z = wlfh[hnd].header.obs_valid_min_z = 1000000000.0;
  wlfh[hnd].header.obs_max_z = wlfh[hnd].header.obs_valid_max_z = -1000000000.0;


  /*  Set to the default.  */

  wlfh[hnd].tv_sec_bits = WLF_TV_SEC_BITS;
  wlfh[hnd].tv_nsec_bits = WLF_TV_NSEC_BITS;
  wlfh[hnd].class_bits = WLF_CLASS_BITS;
  wlfh[hnd].status_bits = WLF_STATUS_BITS;
  wlfh[hnd].address_bits = WLF_ADDRESS_BITS;


  /*  Define the bit fields and record size.  */

  define_record_fields (hnd, NVTrue);


  /*  Check for all sensor position min/max/scale values set.  */

  if ((wlfh[hnd].sensor_x_bits + wlfh[hnd].sensor_y_bits + wlfh[hnd].sensor_z_bits) &&
      (!wlfh[hnd].sensor_x_bits || !wlfh[hnd].sensor_y_bits || !wlfh[hnd].sensor_z_bits))
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("One or more of the sensor position data scales/mins/maxs not set"));
      return (wlf_error.wlf = WLF_SENSOR_POSITION_RANGE_ERROR);
    }


  /*  Check for all sensor attitude min/max/scale values set.  */

  if ((wlfh[hnd].sensor_roll_bits + wlfh[hnd].sensor_pitch_bits + wlfh[hnd].sensor_heading_bits) &&
      (!wlfh[hnd].sensor_roll_bits || !wlfh[hnd].sensor_pitch_bits || !wlfh[hnd].sensor_heading_bits))
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("One or more of the sensor position data scales/mins/maxs not set"));
      return (wlf_error.wlf = WLF_SENSOR_ATTITUDE_RANGE_ERROR);
    }


  /*  Write the header.  */

  if (wlf_write_header (hnd) < 0) return (wlf_error.wlf);


  /*  Allocate waveform buffer if needed.  */

  if (wlfh[hnd].header.number_of_waveforms)
    {
      /*  Allocating the absolute maximum number of bytes that we could ever use (i.e. no compression).  */

      wlfh[hnd].waveform_max_size = 0;
      for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++)
        {
          bytes = NINT (log10 ((NV_FLOAT64) (wlfh[hnd].header.max_waveform[i] - wlfh[hnd].header.min_waveform[i])) / log_of_two);
          bytes /= 8;
          if (bytes % 8) bytes++;
          wlfh[hnd].waveform_max_size += wlfh[hnd].header.waveform_count[i] * bytes;
        }

      wlfh[hnd].wave_buffer = (NV_U_BYTE *) calloc (wlfh[hnd].waveform_max_size, sizeof (NV_U_BYTE));
      if (wlfh[hnd].wave_buffer == NULL)
        {
          perror ("Allocating waveform buffer in wlf_open_file");
          clean_exit (-1);
        }
    }


  /*  Set the optional record field presence flags.  Since the header was zeroed above, these will all default to NVFalse.  */

  if (wlfh[hnd].header.horizontal_uncertainty_scale != 0.0) wlfh[hnd].header.opt.horizontal_uncertainty_present = NVTrue;
  if (wlfh[hnd].header.vertical_uncertainty_scale != 0.0) wlfh[hnd].header.opt.vertical_uncertainty_present = NVTrue;
  if (wlfh[hnd].header.sensor_x_scale != 0.0) wlfh[hnd].header.opt.sensor_position_present = NVTrue;
  if (wlfh[hnd].header.sensor_roll_scale != 0.0) wlfh[hnd].header.opt.sensor_attitude_present = NVTrue;
  if (wlfh[hnd].header.scan_angle_scale != 0.0) wlfh[hnd].header.opt.scan_angle_present = NVTrue;
  if (wlfh[hnd].header.nadir_angle_scale != 0.0) wlfh[hnd].header.opt.nadir_angle_present = NVTrue;
  if (wlfh[hnd].header.water_surface_scale != 0.0) wlfh[hnd].header.opt.water_surface_present = NVTrue;
  if (wlfh[hnd].header.z_offset_scale != 0.0) wlfh[hnd].header.opt.z_offset_present = NVTrue;
  if (wlfh[hnd].header.max_point_source_id) wlfh[hnd].header.opt.point_source_present = NVTrue;
  if (wlfh[hnd].header.edge_flag_present) wlfh[hnd].header.opt.edge_of_flight_line_present = NVTrue;
  if (wlfh[hnd].header.intensity_scale != 0.0) wlfh[hnd].header.opt.intensity_present = NVTrue;
  if (wlfh[hnd].header.max_rgb) wlfh[hnd].header.opt.rgb_present = NVTrue;
  if (wlfh[hnd].header.reflectance_scale != 0.0) wlfh[hnd].header.opt.reflectance_present = NVTrue;
  for (i = 0 ; i < WLF_MAX_ATTR ; i++) if (wlfh[hnd].header.attr_scale[i] != 0.0) wlfh[hnd].header.opt.attr_present[i] = NVTrue;


  wlfh[hnd].at_end = NVTrue;
  wlfh[hnd].modified = NVTrue;
  wlfh[hnd].created = NVTrue;
  wlfh[hnd].write = NVTrue;
  wlfh[hnd].header.number_of_records = 0;


  wlf_error.system = 0;
  return (hnd);
}


/*********************************************************************************************

  Function:    wlf_open_file

  Purpose:     Open a WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        09/25/09

  Arguments:   path           -    The WLF file path
               wlf_header     -    WLF_HEADER structure to be populated
               mode           -    WLF_UPDATE or WLF_READ_ONLY

  Returns:     NV_INT32       -    The file handle (0 or positive) or
                                   WLF_TOO_MANY_OPEN_FILES
                                   WLF_OPEN_UPDATE_ERROR
                                   WLF_OPEN_READONLY_ERROR
                                   WLF_NOT_WLF_FILE_ERROR

  Caveats:     wlf_error.wlf may be set to WLF_NEWER_FILE_VERSION_WARNING if the file version
               is newer than the library version.  This shouldn't cause a problem but
               you may want to check it with wlf_get_errno () when you open a file so
               you can issue a warning.

*********************************************************************************************/

WLF_DLL NV_INT32 wlf_open_file (const NV_CHAR *path, WLF_HEADER *wlf_header, NV_INT32 mode)
{
  NV_INT32 i, hnd, year[4], jday[4], hour[4], minute[4], bytes;
  NV_FLOAT32 second[4], tmpf;
  NV_CHAR varin[8192], info[8192], tmp_char[128];


  /*  Due to the stupidity that is Microsoft I can't use %hhd in scanf statements so I have to play games
      to read a value in to an unsigned char.  */

  NV_INT16 tmpi16;


  /*  The first time through we want to initialize (zero) the wlf handle array.  */

  if (first)
    {
      for (i = 0 ; i < WLF_MAX_FILES ; i++) 
        {
          memset (&wlfh[i], 0, sizeof (INTERNAL_WLF_STRUCT));
          wlfh[i].fp = NULL;
          wlfh[i].prev_shot_num = -1;
        }

      log_of_two = log10 (2.0L);


      /*  Set up the SIGINT handler.  */

      signal (SIGINT, sigint_handler);


      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = WLF_MAX_FILES;
  for (i = 0 ; i < WLF_MAX_FILES ; i++)
    {
      if (wlfh[i].fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == WLF_MAX_FILES) return (wlf_error.wlf = WLF_TOO_MANY_OPEN_FILES);


  /*  Internal structs are zeroed above and on close of file so we don't have to do it here.  */


  /*  Open the file and read the header.  */

  switch (mode)
    {
    case WLF_UPDATE:
      if ((wlfh[hnd].fp = fopen64 (path, "rb+")) == NULL)
        {
          wlf_error.system = errno;
          strcpy (wlf_error.file, wlfh[hnd].path);
          return (wlf_error.wlf = WLF_OPEN_UPDATE_ERROR);
        }
      break;

    case WLF_READONLY:
      if ((wlfh[hnd].fp = fopen64 (path, "rb")) == NULL)
        {
          wlf_error.system = errno;
          strcpy (wlf_error.file, wlfh[hnd].path);
          return (wlf_error.wlf = WLF_OPEN_READONLY_ERROR);
        }
      break;
    }


  wlfh[hnd].header.z_units = WLF_METERS;
  for (i = 0 ; i < 4 ; i++) year[i] = 0;


  /*  Save the file name for error messages.  */

  strcpy (wlfh[hnd].path, path);


  /*  We want to try to read the first line (version info) with an fread in case we mistakenly asked to
      load a binary file.  If we try to use wlf_ngets to read a binary file and there are no line feeds in 
      the first sizeof (varin) characters we would segfault since fgets doesn't check for overrun.  */

  if (!fread (varin, 128, 1, wlfh[hnd].fp))
    {
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_NOT_WLF_FILE_ERROR);
    }


  /*  Check for the WLF library string at the beginning of the file.  */

  if (!strstr (varin, N_("WLF library")))
    {
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_NOT_WLF_FILE_ERROR);
    }


  /*  Rewind to the beginning of the file.  Yes, we'll read the version again but we need to check the version number anyway.  */

  fseek (wlfh[hnd].fp, 0, SEEK_SET);


  /*  Note, we're using wlf_ngets instead of fgets since we really don't want the CR/LF in the strings.  */

  while (wlf_ngets (varin, sizeof (varin), wlfh[hnd].fp))
    {
      if (strstr (varin, N_("[END OF HEADER]"))) break;


      /*  Put everything to the right of the equals sign into 'info'.   */

      get_string (varin, info);


      /*  Read the version string and check the major version number against the library major version.  */

      if (strstr (varin, "[VERSION]"))
        {
          strcpy (wlfh[hnd].header.version, info);
          strcpy (info, strstr (varin, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          wlfh[hnd].major_version = (NV_INT16) tmpf;

          strcpy (info, strstr (WLF_VERSION, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          if (wlfh[hnd].major_version > (NV_INT16) tmpf)
            {
              strcpy (wlf_error.file, wlfh[hnd].path);
              wlf_error.wlf = WLF_NEWER_FILE_VERSION_WARNING;
            }
        }


      if (strstr (varin, N_("[FILE SOURCE ID]"))) sscanf (info, "%d", &wlfh[hnd].header.file_source_id);

      if (strstr (varin, N_("[CREATION YEAR]"))) sscanf (info, "%d", &year[0]);
      if (strstr (varin, N_("[CREATION DAY OF YEAR]"))) sscanf (info, "%d", &jday[0]);
      if (strstr (varin, N_("[CREATION HOUR]"))) sscanf (info, "%d", &hour[0]);
      if (strstr (varin, N_("[CREATION MINUTE]"))) sscanf (info, "%d", &minute[0]);
      if (strstr (varin, N_("[CREATION SECOND]"))) sscanf (info, "%f", &second[0]);
      if (strstr (varin, N_("[CREATION SOFTWARE]"))) strcpy (wlfh[hnd].header.creation_software, info);

      if (strstr (varin, N_("[MODIFICATION YEAR]"))) sscanf (info, "%d", &year[1]);
      if (strstr (varin, N_("[MODIFICATION DAY OF YEAR]"))) sscanf (info, "%d", &jday[1]);
      if (strstr (varin, N_("[MODIFICATION HOUR]"))) sscanf (info, "%d", &hour[1]);
      if (strstr (varin, N_("[MODIFICATION MINUTE]"))) sscanf (info, "%d", &minute[1]);
      if (strstr (varin, N_("[MODIFICATION SECOND]"))) sscanf (info, "%f", &second[1]);
      if (strstr (varin, N_("[MODIFICATION SOFTWARE]"))) strcpy (wlfh[hnd].header.modification_software, info);

      if (strstr (varin, N_("[SECURITY CLASSIFICATION]"))) strcpy (wlfh[hnd].header.security_classification, info);

      if (strstr (varin, N_("{DISTRIBUTION =")))
        {
          strcpy (wlfh[hnd].header.distribution, "");
          while (fgets (varin, sizeof (varin), wlfh[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (wlfh[hnd].header.distribution, varin);
            }
        }

      if (strstr (varin, N_("{DECLASSIFICATION =")))
        {
          strcpy (wlfh[hnd].header.declassification, "");
          while (fgets (varin, sizeof (varin), wlfh[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (wlfh[hnd].header.declassification, varin);
            }
        }

      if (strstr (varin, N_("{SECURITY CLASSIFICATION JUSTIFICATION =")))
        {
          strcpy (wlfh[hnd].header.class_just, "");
          while (fgets (varin, sizeof (varin), wlfh[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (wlfh[hnd].header.class_just, varin);
            }
        }


      if (strstr (varin, N_("{DOWNGRADE =")))
        {
          strcpy (wlfh[hnd].header.downgrade, "");
          while (fgets (varin, sizeof (varin), wlfh[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (wlfh[hnd].header.downgrade, varin);
            }
        }


      if (strstr (varin, N_("[SOURCE]"))) strcpy (wlfh[hnd].header.source, info);
      if (strstr (varin, N_("[SYSTEM]"))) strcpy (wlfh[hnd].header.system, info);

      if (strstr (varin, N_("[OBSERVED MIN X]"))) sscanf (info, "%lf", &wlfh[hnd].header.obs_min_x);
      if (strstr (varin, N_("[OBSERVED MIN Y]"))) sscanf (info, "%lf", &wlfh[hnd].header.obs_min_y);
      if (strstr (varin, N_("[OBSERVED MIN Z]"))) sscanf (info, "%f", &wlfh[hnd].header.obs_min_z);
      if (strstr (varin, N_("[OBSERVED MAX X]"))) sscanf (info, "%lf", &wlfh[hnd].header.obs_max_x);
      if (strstr (varin, N_("[OBSERVED MAX Y]"))) sscanf (info, "%lf", &wlfh[hnd].header.obs_max_y);
      if (strstr (varin, N_("[OBSERVED MAX Z]"))) sscanf (info, "%f", &wlfh[hnd].header.obs_max_z);

      if (strstr (varin, N_("[OBSERVED VALID MIN X]"))) sscanf (info, "%lf", &wlfh[hnd].header.obs_valid_min_x);
      if (strstr (varin, N_("[OBSERVED VALID MIN Y]"))) sscanf (info, "%lf", &wlfh[hnd].header.obs_valid_min_y);
      if (strstr (varin, N_("[OBSERVED VALID MIN Z]"))) sscanf (info, "%f", &wlfh[hnd].header.obs_valid_min_z);
      if (strstr (varin, N_("[OBSERVED VALID MAX X]"))) sscanf (info, "%lf", &wlfh[hnd].header.obs_valid_max_x);
      if (strstr (varin, N_("[OBSERVED VALID MAX Y]"))) sscanf (info, "%lf", &wlfh[hnd].header.obs_valid_max_y);
      if (strstr (varin, N_("[OBSERVED VALID MAX Z]"))) sscanf (info, "%f", &wlfh[hnd].header.obs_valid_max_z);

      if (strstr (varin, N_("[NUMBER OF RECORDS]"))) sscanf (info, "%d", &wlfh[hnd].header.number_of_records);
      if (strstr (varin, N_("[PROJECT]"))) strcpy (wlfh[hnd].header.project, info);
      if (strstr (varin, N_("[MISSION]"))) strcpy (wlfh[hnd].header.mission, info);
      if (strstr (varin, N_("[DATASET]"))) strcpy (wlfh[hnd].header.dataset, info);
      if (strstr (varin, N_("[FLIGHT ID]"))) strcpy (wlfh[hnd].header.flight_id, info);

      if (strstr (varin, N_("[FLIGHT START YEAR]"))) sscanf (info, "%d", &year[2]);
      if (strstr (varin, N_("[FLIGHT START DAY OF YEAR]"))) sscanf (info, "%d", &jday[2]);
      if (strstr (varin, N_("[FLIGHT START HOUR]"))) sscanf (info, "%d", &hour[2]);
      if (strstr (varin, N_("[FLIGHT START MINUTE]"))) sscanf (info, "%d", &minute[2]);
      if (strstr (varin, N_("[FLIGHT START SECOND]"))) sscanf (info, "%f", &second[2]);

      if (strstr (varin, N_("[FLIGHT END YEAR]"))) sscanf (info, "%d", &year[3]);
      if (strstr (varin, N_("[FLIGHT END DAY OF YEAR]"))) sscanf (info, "%d", &jday[3]);
      if (strstr (varin, N_("[FLIGHT END HOUR]"))) sscanf (info, "%d", &hour[3]);
      if (strstr (varin, N_("[FLIGHT END MINUTE]"))) sscanf (info, "%d", &minute[3]);
      if (strstr (varin, N_("[FLIGHT END SECOND]"))) sscanf (info, "%f", &second[3]);


      if (strstr (varin, N_("[WLF USER FLAG 01 NAME]"))) strcpy (wlfh[hnd].header.wlf_user_flag_name[0], info);
      if (strstr (varin, N_("[WLF USER FLAG 02 NAME]"))) strcpy (wlfh[hnd].header.wlf_user_flag_name[1], info);
      if (strstr (varin, N_("[WLF USER FLAG 03 NAME]"))) strcpy (wlfh[hnd].header.wlf_user_flag_name[2], info);
      if (strstr (varin, N_("[WLF USER FLAG 04 NAME]"))) strcpy (wlfh[hnd].header.wlf_user_flag_name[3], info);
      if (strstr (varin, N_("[WLF USER FLAG 05 NAME]"))) strcpy (wlfh[hnd].header.wlf_user_flag_name[4], info);

      if (strstr (varin, N_("{COMMENTS =")))
        {
          strcpy (wlfh[hnd].header.comments, "");
          while (fgets (varin, sizeof (varin), wlfh[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (wlfh[hnd].header.comments, varin);
            }
        }

      if (strstr (varin, N_("[Z UNITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].header.z_units = (NV_U_BYTE) tmpi16;
        }

      if (strstr (varin, N_("[NULL Z VALUE]"))) sscanf (info, "%f", &wlfh[hnd].header.null_z_value);

      if (strstr (varin, N_("[MAX NUMBER OF RETURNS PER RECORD]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].header.max_number_of_returns = (NV_U_BYTE) tmpi16;
        }

      if (strstr (varin, N_("{WELL-KNOWN TEXT =")))
        {
          strcpy (wlfh[hnd].header.wkt, "");
          while (fgets (varin, sizeof (varin), wlfh[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (wlfh[hnd].header.wkt, varin);
            }
        }

      if (strstr (varin, N_("[MIN X]"))) sscanf (info, "%lf", &wlfh[hnd].header.min_x);
      if (strstr (varin, N_("[MAX X]"))) sscanf (info, "%lf", &wlfh[hnd].header.max_x);
      if (strstr (varin, N_("[X SCALE]"))) sscanf (info, "%lf", &wlfh[hnd].header.x_scale);
      if (strstr (varin, N_("[MIN Y]"))) sscanf (info, "%lf", &wlfh[hnd].header.min_y);
      if (strstr (varin, N_("[MAX Y]"))) sscanf (info, "%lf", &wlfh[hnd].header.max_y);
      if (strstr (varin, N_("[Y SCALE]"))) sscanf (info, "%lf", &wlfh[hnd].header.y_scale);
      if (strstr (varin, N_("[MIN Z]"))) sscanf (info, "%f", &wlfh[hnd].header.min_z);
      if (strstr (varin, N_("[MAX Z]"))) sscanf (info, "%f", &wlfh[hnd].header.max_z);
      if (strstr (varin, N_("[Z SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.z_scale);

      if (strstr (varin, N_("[SENSOR MIN X]"))) sscanf (info, "%lf", &wlfh[hnd].header.sensor_min_x);
      if (strstr (varin, N_("[SENSOR MAX X]"))) sscanf (info, "%lf", &wlfh[hnd].header.sensor_max_x);
      if (strstr (varin, N_("[SENSOR X SCALE]"))) sscanf (info, "%lf", &wlfh[hnd].header.sensor_x_scale);
      if (strstr (varin, N_("[SENSOR MIN Y]"))) sscanf (info, "%lf", &wlfh[hnd].header.sensor_min_y);
      if (strstr (varin, N_("[SENSOR MAX Y]"))) sscanf (info, "%lf", &wlfh[hnd].header.sensor_max_y);
      if (strstr (varin, N_("[SENSOR Y SCALE]"))) sscanf (info, "%lf", &wlfh[hnd].header.sensor_y_scale);
      if (strstr (varin, N_("[SENSOR MIN Z]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_min_z);
      if (strstr (varin, N_("[SENSOR MAX Z]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_max_z);
      if (strstr (varin, N_("[SENSOR Z SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_z_scale);

      if (strstr (varin, N_("[SENSOR MIN ROLL]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_min_roll);
      if (strstr (varin, N_("[SENSOR MAX ROLL]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_max_roll);
      if (strstr (varin, N_("[SENSOR ROLL SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_roll_scale);
      if (strstr (varin, N_("[SENSOR MIN PITCH]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_min_pitch);
      if (strstr (varin, N_("[SENSOR MAX PITCH]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_max_pitch);
      if (strstr (varin, N_("[SENSOR PITCH SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_pitch_scale);
      if (strstr (varin, N_("[SENSOR MIN HEADING]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_min_heading);
      if (strstr (varin, N_("[SENSOR MAX HEADING]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_max_heading);
      if (strstr (varin, N_("[SENSOR HEADING SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.sensor_heading_scale);

      if (strstr (varin, N_("[MIN SCAN ANGLE]"))) sscanf (info, "%f", &wlfh[hnd].header.min_scan_angle);
      if (strstr (varin, N_("[MAX SCAN ANGLE]"))) sscanf (info, "%f", &wlfh[hnd].header.max_scan_angle);
      if (strstr (varin, N_("[SCAN ANGLE SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.scan_angle_scale);

      if (strstr (varin, N_("[MIN NADIR ANGLE]"))) sscanf (info, "%f", &wlfh[hnd].header.min_nadir_angle);
      if (strstr (varin, N_("[MAX NADIR ANGLE]"))) sscanf (info, "%f", &wlfh[hnd].header.max_nadir_angle);
      if (strstr (varin, N_("[NADIR ANGLE SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.nadir_angle_scale);

      if (strstr (varin, N_("[MIN WATER SURFACE]"))) sscanf (info, "%f", &wlfh[hnd].header.min_water_surface);
      if (strstr (varin, N_("[MAX WATER SURFACE]"))) sscanf (info, "%f", &wlfh[hnd].header.max_water_surface);
      if (strstr (varin, N_("[WATER SURFACE SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.water_surface_scale);

      if (strstr (varin, N_("[MIN Z OFFSET]"))) sscanf (info, "%f", &wlfh[hnd].header.min_z_offset);
      if (strstr (varin, N_("[MAX Z OFFSET]"))) sscanf (info, "%f", &wlfh[hnd].header.max_z_offset);
      if (strstr (varin, N_("[Z OFFSET SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.z_offset_scale);
      if (strstr (varin, N_("[Z OFFSET DATUM]"))) strcpy (wlfh[hnd].header.z_offset_datum, info);

      if (strstr (varin, N_("[MAX POINT SOURCE ID]"))) sscanf (info, "%d", &wlfh[hnd].header.max_point_source_id);

      if (strstr (varin, N_("[EDGE FLAG PRESENT]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].header.edge_flag_present = (NV_U_BYTE) tmpi16;
        }

      if (strstr (varin, N_("[MIN INTENSITY]"))) sscanf (info, "%f", &wlfh[hnd].header.min_intensity);
      if (strstr (varin, N_("[MAX INTENSITY]"))) sscanf (info, "%f", &wlfh[hnd].header.max_intensity);
      if (strstr (varin, N_("[INTENSITY SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.intensity_scale);

      for (i = 0 ; i < WLF_MAX_ATTR ; i++)
        {
          sprintf (tmp_char, N_("[ATTRIBUTE %02d NAME]"), i + 1);
          if (strstr (varin, tmp_char)) strcpy (wlfh[hnd].header.attr_name[i], info);
          sprintf (tmp_char, N_("[MIN ATTRIBUTE %02d]"), i + 1);
          if (strstr (varin, tmp_char)) sscanf (info, "%f", &wlfh[hnd].header.min_attr[i]);
          sprintf (tmp_char, N_("[MAX ATTRIBUTE %02d]"), i + 1);
          if (strstr (varin, tmp_char)) sscanf (info, "%f", &wlfh[hnd].header.max_attr[i]);
          sprintf (tmp_char, N_("[ATTRIBUTE %02d SCALE]"), i + 1);
          if (strstr (varin, tmp_char)) sscanf (info, "%f", &wlfh[hnd].header.attr_scale[i]);
        }

      if (strstr (varin, N_("[MAX RGB]"))) sscanf (info, "%d", &wlfh[hnd].header.max_rgb);

      if (strstr (varin, N_("[MIN REFLECTANCE]"))) sscanf (info, "%f", &wlfh[hnd].header.min_reflectance);
      if (strstr (varin, N_("[MAX REFLECTANCE]"))) sscanf (info, "%f", &wlfh[hnd].header.max_reflectance);
      if (strstr (varin, N_("[REFLECTANCE SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.reflectance_scale);

      if (strstr (varin, N_("[MIN HORIZONTAL UNCERTAINTY]"))) sscanf (info, "%f", &wlfh[hnd].header.min_horizontal_uncertainty);
      if (strstr (varin, N_("[MAX HORIZONTAL UNCERTAINTY]"))) sscanf (info, "%f", &wlfh[hnd].header.max_horizontal_uncertainty);
      if (strstr (varin, N_("[HORIZONTAL UNCERTAINTY SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.horizontal_uncertainty_scale);

      if (strstr (varin, N_("[MIN VERTICAL UNCERTAINTY]"))) sscanf (info, "%f", &wlfh[hnd].header.min_vertical_uncertainty);
      if (strstr (varin, N_("[MAX VERTICAL UNCERTAINTY]"))) sscanf (info, "%f", &wlfh[hnd].header.max_vertical_uncertainty);
      if (strstr (varin, N_("[VERTICAL UNCERTAINTY SCALE]"))) sscanf (info, "%f", &wlfh[hnd].header.vertical_uncertainty_scale);

      if (strstr (varin, N_("[NUMBER OF WAVEFORMS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].header.number_of_waveforms = (NV_U_BYTE) tmpi16;
        }

      for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++)
        {
          sprintf (tmp_char, N_("[WAVEFORM %02d NAME]"), i + 1);
          if (strstr (varin, tmp_char)) strcpy (wlfh[hnd].header.waveform_name[i], info);
          sprintf (tmp_char, N_("[MIN WAVEFORM %02d]"), i + 1);
          if (strstr (varin, tmp_char)) sscanf (info, "%d", &wlfh[hnd].header.min_waveform[i]);
          sprintf (tmp_char, N_("[MAX WAVEFORM %02d]"), i + 1);
          if (strstr (varin, tmp_char)) sscanf (info, "%d", &wlfh[hnd].header.max_waveform[i]);
          sprintf (tmp_char, N_("[WAVEFORM %02d COUNT]"), i + 1);
          if (strstr (varin, tmp_char)) sscanf (info, "%d", &wlfh[hnd].header.waveform_count[i]);
        }

      if (strstr (varin, N_("[POSIX TIME SECOND BITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].tv_sec_bits = (NV_U_BYTE) tmpi16;
        }
      if (strstr (varin, N_("[POSIX TIME NANOSECOND BITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].tv_nsec_bits = (NV_U_BYTE) tmpi16;
        }
      if (strstr (varin, N_("[CLASSIFICATION BITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].class_bits = (NV_U_BYTE) tmpi16;
        }
      if (strstr (varin, N_("[STATUS BITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].status_bits = (NV_U_BYTE) tmpi16;
        }
      if (strstr (varin, N_("[WAVEFORM ADDRESS BITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].address_bits = (NV_U_BYTE) tmpi16;
        }
      if (strstr (varin, N_("[WAVEFORM BLOCKSIZE BITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          wlfh[hnd].waveform_blocksize_bits = (NV_U_BYTE) tmpi16;
        }
      if (strstr (varin, N_("[WAVEFORM BLOCK OFFSET]"))) sscanf (info, NV_INT64_SPECIFIER, &wlfh[hnd].waveform_block_offset);
      if (strstr (varin, N_("[HEADER SIZE]"))) sscanf (info, "%d", &wlfh[hnd].header_size);
      if (strstr (varin, N_("[RECORD SIZE]"))) sscanf (info, "%hd", &wlfh[hnd].record_size);
    }


  /*  If the user flags weren't defined, set the default names.  */

  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[0]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[0], "WLF_USER_01");
  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[1]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[1], "WLF_USER_02");
  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[2]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[2], "WLF_USER_03");
  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[3]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[3], "WLF_USER_04");
  if (!(strlen (wlfh[hnd].header.wlf_user_flag_name[4]))) strcpy (wlfh[hnd].header.wlf_user_flag_name[4], "WLF_USER_05");


  /*  Compute min and max sizes for modifiable hardwired fields so we can check the range in wlf_update_record.  */

  wlfh[hnd].max_class = NINT (pow (2.0, (NV_FLOAT64) wlfh[hnd].class_bits)) - 1;
  wlfh[hnd].max_status = NINT (pow (2.0, (NV_FLOAT64) wlfh[hnd].status_bits)) - 1;


  /*  Compute the number of bytes needed for the waveform blocksize.  */
 
  wlfh[hnd].waveform_blocksize_bytes = wlfh[hnd].waveform_blocksize_bits / 8;


  /*  Allocate waveform buffer if needed.  */

  if (wlfh[hnd].header.number_of_waveforms)
    {
      /*  Allocating the waveform array.  */

      wlfh[hnd].wave = (NV_INT32 **) malloc (wlfh[hnd].header.number_of_waveforms * sizeof (NV_INT32 *));
      if (wlfh[hnd].wave == NULL)
        {
          perror (_("Allocating wave array in wlf_open_file"));
          clean_exit (-1);
        }


      /*  Allocating the absolute maximum number of bytes that we could ever use (i.e. no compression).  */

      wlfh[hnd].waveform_max_size = 0;
      for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++)
        {
          /*  Compute the number of bytes needed for the wave_buffer.  */

          bytes = NINT (log10 ((NV_FLOAT64) (wlfh[hnd].header.max_waveform[i] - wlfh[hnd].header.min_waveform[i])) / log_of_two);
          bytes /= 8;
          if (bytes % 8) bytes++;
          wlfh[hnd].waveform_max_size += wlfh[hnd].header.waveform_count[i] * bytes;


          /*  Allocate wave array for each waveform.  */

          wlfh[hnd].wave[i] = (NV_INT32 *) malloc (wlfh[hnd].header.waveform_count[i] * sizeof (NV_INT32));
          if (wlfh[hnd].wave[i] == NULL)
            {
              perror (_("Allocating wave[i] array in wlf_open_file"));
              clean_exit (-1);
            }
        }

      wlfh[hnd].wave_buffer = (NV_U_BYTE *) calloc (wlfh[hnd].waveform_max_size, sizeof (NV_U_BYTE));
      if (wlfh[hnd].wave_buffer == NULL)
        {
          perror (_("Allocating waveform buffer in wlf_open_file"));
          clean_exit (-1);
        }
    }


  /*  Set the optional record field presence flags.  Since the header was zeroed above, these will all default to NVFalse.  */

  if (wlfh[hnd].header.horizontal_uncertainty_scale != 0.0) wlfh[hnd].header.opt.horizontal_uncertainty_present = NVTrue;
  if (wlfh[hnd].header.vertical_uncertainty_scale != 0.0) wlfh[hnd].header.opt.vertical_uncertainty_present = NVTrue;
  if (wlfh[hnd].header.sensor_x_scale != 0.0) wlfh[hnd].header.opt.sensor_position_present = NVTrue;
  if (wlfh[hnd].header.sensor_roll_scale != 0.0) wlfh[hnd].header.opt.sensor_attitude_present = NVTrue;
  if (wlfh[hnd].header.scan_angle_scale != 0.0) wlfh[hnd].header.opt.scan_angle_present = NVTrue;
  if (wlfh[hnd].header.nadir_angle_scale != 0.0) wlfh[hnd].header.opt.nadir_angle_present = NVTrue;
  if (wlfh[hnd].header.water_surface_scale != 0.0) wlfh[hnd].header.opt.water_surface_present = NVTrue;
  if (wlfh[hnd].header.z_offset_scale != 0.0) wlfh[hnd].header.opt.z_offset_present = NVTrue;
  if (wlfh[hnd].header.max_point_source_id) wlfh[hnd].header.opt.point_source_present = NVTrue;
  if (wlfh[hnd].header.edge_flag_present) wlfh[hnd].header.opt.edge_of_flight_line_present = NVTrue;
  if (wlfh[hnd].header.intensity_scale != 0.0) wlfh[hnd].header.opt.intensity_present = NVTrue;
  if (wlfh[hnd].header.max_rgb) wlfh[hnd].header.opt.rgb_present = NVTrue;
  if (wlfh[hnd].header.reflectance_scale != 0.0) wlfh[hnd].header.opt.reflectance_present = NVTrue;
  for (i = 0 ; i < WLF_MAX_ATTR ; i++) if (wlfh[hnd].header.attr_scale[i] != 0.0) wlfh[hnd].header.opt.attr_present[i] = NVTrue;


  wlf_inv_cvtime (year[0] - 1900, jday[0], hour[0], minute[0], second[0], &wlfh[hnd].header.creation_tv_sec,
                  &wlfh[hnd].header.creation_tv_nsec);

  wlf_inv_cvtime (year[1] - 1900, jday[1], hour[1], minute[1], second[1], &wlfh[hnd].header.modification_tv_sec,
                  &wlfh[hnd].header.modification_tv_nsec);

  if (year[2]) wlf_inv_cvtime (year[2] - 1900, jday[2], hour[2], minute[2], second[2], &wlfh[hnd].header.flight_start_tv_sec,
                               &wlfh[hnd].header.flight_start_tv_nsec);

  if (year[3]) wlf_inv_cvtime (year[3] - 1900, jday[3], hour[3], minute[3], second[3], &wlfh[hnd].header.flight_end_tv_sec,
                               &wlfh[hnd].header.flight_end_tv_nsec);


  define_record_fields (hnd, NVFalse);

  *wlf_header = wlfh[hnd].header;


  wlfh[hnd].at_end = NVFalse;
  wlfh[hnd].modified = NVFalse;
  wlfh[hnd].created = NVFalse;
  wlfh[hnd].write = NVFalse;


  wlf_error.system = 0;
  return (hnd);
}


/*********************************************************************************************

  Function:    wlf_close_file

  Purpose:     Close a WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_CLOSE_FSEEK_ERROR
                                   WLF_CLOSE_WAVE_FSEEK_ERROR
                                   WLF_CLOSE_WAVE_READ_ERROR
                                   WLF_CLOSE_WAVE_WRITE_ERROR
                                   WLF_HEADER_WRITE_FSEEK_ERROR
                                   WLF_HEADER_WRITE_ERROR
                                   WLF_CLOSE_ERROR

*********************************************************************************************/

WLF_DLL NV_INT32 wlf_close_file (NV_INT32 hnd)
{
  time_t t;
  struct tm *cur_tm;
  NV_U_BYTE *cat_buffer;
  NV_INT32 i, bytes_read, percent = 0, old_percent = -1;
  NV_INT64 eof = 0LL;


  /*  Just in case someone tries to close a file more than once... */

  if (wlfh[hnd].fp == NULL) return (wlf_error.wlf = WLF_SUCCESS);


  /*  If the file was created or modified we need to save the time.  */

  if (wlfh[hnd].created || wlfh[hnd].modified)
    {
      t = time (&t);
      cur_tm = gmtime (&t);
      wlf_inv_cvtime (cur_tm->tm_year, cur_tm->tm_yday + 1, cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec, 
                      &wlfh[hnd].header.modification_tv_sec, &wlfh[hnd].header.modification_tv_nsec);
    }


  /*  If the file was created and we had waveforms...  */

  if (wlfh[hnd].created)
    {
      /*  If we had waveforms we need to concatenate the temporary waveform file to the point file, save the waveform
          block offset value, and remove the temporary waveform file.  */

      if (wlfh[hnd].header.number_of_waveforms)
        {
          /*  Go to the end of the point file.  */

          if (fseeko64 (wlfh[hnd].fp, 0LL, SEEK_END) < 0)
            {
              wlf_error.system = errno;
              strcpy (wlf_error.file, wlfh[hnd].path);
              return (wlf_error.wlf = WLF_CLOSE_FSEEK_ERROR);
            }


          /*  Save the waveform block offset value for the file header.  */

          wlfh[hnd].waveform_block_offset = ftello64 (wlfh[hnd].fp);


          /*  Get the size of the waveform file so we can inform the caller of progress.  */

          if (wlf_progress_callback)
            {
              if (fseeko64 (wlfh[hnd].wfp, 0LL, SEEK_END) < 0)
                {
                  wlf_error.system = errno;
                  strcpy (wlf_error.file, wlfh[hnd].wave_path);
                  return (wlf_error.wlf = WLF_CLOSE_WAVE_FSEEK_ERROR);
                }
              eof = ftello64 (wlfh[hnd].wfp);
            }


          /*  Go to the beginning of the temporary waveform file.  */

          if (fseeko64 (wlfh[hnd].wfp, 0LL, SEEK_SET) < 0)
            {
              wlf_error.system = errno;
              strcpy (wlf_error.file, wlfh[hnd].wave_path);
              return (wlf_error.wlf = WLF_CLOSE_WAVE_FSEEK_ERROR);
            }


          cat_buffer = (NV_U_BYTE *) malloc (WAVE_CAT_BLOCK);
          if (cat_buffer == NULL)
            {
              perror (_("Allocating cat_buffer in wlf_close_file"));
              clean_exit (-1);
            }


          /*  Read the temporary waveform file and write it to the end of the point file.  */

          while (NVTrue)
            {
              bytes_read = fread (cat_buffer, 1, WAVE_CAT_BLOCK, wlfh[hnd].wfp);


              if (!bytes_read) break;


              if (bytes_read < 0)
                {
                  wlf_error.system = errno;
                  strcpy (wlf_error.file, wlfh[hnd].wave_path);
                  return (wlf_error.wlf = WLF_CLOSE_WAVE_READ_ERROR);
                }

              if (!fwrite (cat_buffer, 1, bytes_read, wlfh[hnd].fp))
                {
                  wlf_error.system = errno;
                  strcpy (wlf_error.file, wlfh[hnd].path);
                  return (wlf_error.wlf = WLF_CLOSE_WAVE_WRITE_ERROR);
                }


              /*  If the caller has registered a progress callback, call it when the percentage complete has changed.  */

              if (wlf_progress_callback)
                {
                  percent = NINT (((NV_FLOAT64) ftello64 (wlfh[hnd].wfp) / (NV_FLOAT64) eof) * 100.0L);
                  if (percent != old_percent)
                    {
                      (*wlf_progress_callback) (0, percent);

                      old_percent = percent;
                    }
                }
            }


          if (wlf_progress_callback) (*wlf_progress_callback) (1, 100);


          free (cat_buffer);


          /*  Get rid of the temporary waveform file.  */

          fclose (wlfh[hnd].wfp);
          remove (wlfh[hnd].wave_path);
        }


      /*  Get the creation time.  */

      t = time (&t);
      cur_tm = gmtime (&t);
      wlf_inv_cvtime (cur_tm->tm_year, cur_tm->tm_yday + 1, cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec, 
                      &wlfh[hnd].header.creation_tv_sec, &wlfh[hnd].header.creation_tv_nsec);
    }
  else
    {
      /*  Free the waveform buffer and arrays if needed.  These only get created if you called wlf_open_file
          not if you called wlf_create_file.  */

      if (wlfh[hnd].header.number_of_waveforms)
        {
          free (wlfh[hnd].wave_buffer);

          for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++) free (wlfh[hnd].wave[i]);

          free (wlfh[hnd].wave);
        }
    }


  /*  If we created or modified the file, update the header.  */

  if (wlfh[hnd].created || wlfh[hnd].modified)
    {
      if (wlf_write_header (hnd) < 0) return (wlf_error.wlf);
    }


  if (fclose (wlfh[hnd].fp))
    {
      wlf_error.system = errno;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_CLOSE_ERROR);
    }


  /*  Clear the internal structure.  */

  memset (&wlfh[hnd], 0, sizeof (INTERNAL_WLF_STRUCT));
  wlfh[hnd].fp = NULL;


  wlf_error.system = 0;
  return (wlf_error.wlf = WLF_SUCCESS);
}


/*********************************************************************************************

  Function:    wlf_close_file_ratio

  Purpose:     Special version of wlf_close file that returns the final waveform compression
               ratio.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               ratio          -    If the file was just created and had waveforms, this value
                                   will be set to the final compression ratio.  Otherwise, it 
                                   is set to 0.0.
               total_bytes    -    Total number of bytes used for waveforms.

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_CLOSE_FSEEK_ERROR
                                   WLF_CLOSE_WAVE_FSEEK_ERROR
                                   WLF_CLOSE_WAVE_READ_ERROR
                                   WLF_CLOSE_WAVE_WRITE_ERROR
                                   WLF_HEADER_WRITE_FSEEK_ERROR
                                   WLF_HEADER_WRITE_ERROR
                                   WLF_CLOSE_ERROR

  Caveats:     The ratio returned is based on the min and max values for each waveform type not
               on the original storage used.  In other words, if you had a min and max of 0 and
               1023 (i.e. 10 bits), the compression ration would be based on that, not on the
               fact that you probably had it stored in a 2 byte (i.e. 16 bits) record.  If you
               would like to know that ratio you can use the returned total_bytes field to
               compute it.

*********************************************************************************************/

WLF_DLL NV_INT32 wlf_close_file_ratio (NV_INT32 hnd, NV_FLOAT32 *ratio, NV_INT32 *total_bytes)
{
  NV_INT32 i, num_waveforms;


  if (wlfh[hnd].created && wlfh[hnd].header.number_of_waveforms)
    {
      *total_bytes = wlfh[hnd].waveform_total_bytes;


      /*  Compute the final waveform compression ratio.  */

      num_waveforms = 0;
      for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++) num_waveforms += wlfh[hnd].header.waveform_count[i];

      *ratio = (NV_FLOAT32) wlfh[hnd].waveform_total_bytes / ((NV_FLOAT32) wlfh[hnd].header.number_of_records *
                                                              (NV_FLOAT32) num_waveforms);
    }
  else
    {
      *total_bytes = 0;
      *ratio = 0.0;
    }


  return (wlf_close_file (hnd));
}



/*********************************************************************************************

  Function:    wlf_read_record

  Purpose:     Retrieve a WLF point and (optionally) waveforms from a WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        09/25/09

  Arguments:   hnd            -    The file handle
               recnum         -    The record number of the WLF record to be retrieved
               wlf_record     -    The returned WLF record
               wave_flag      -    Set to NVTrue if you want waveforms
               wave           -    Pointer to the waveform structure

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_INVALID_RECORD_NUMBER
                                   WLF_READ_FSEEK_ERROR
                                   WLF_READ_ERROR
                                   WLF_WAVE_READ_FSEEK_ERROR
                                   WLF_WAVE_READ_ERROR

  Caveats:     The ***wave structure will be allocated by the software.  If you don't have or
               want waveforms, set wave_flag to NVFalse and pass a NULL pointer in the
               ***wave argument (see example in wlf.h).

*********************************************************************************************/

WLF_DLL NV_INT32 wlf_read_record (NV_INT32 hnd, NV_INT32 recnum, WLF_RECORD *wlf_record, NV_BOOL wave_flag, NV_INT32 ***wave)
{
  NV_INT64 pos;
  NV_U_BYTE buffer[1024];
  NV_INT32 i, j, bit_pos, diff, min_diff = 0, num_bits, bytes, range, start32, bias32;
  NV_INT16 start16, bias16;
  NV_BYTE start8, bias8;


  /*  Check for record out of bounds.  */

  if (recnum >= wlfh[hnd].header.number_of_records || recnum < 0)
    {
      wlf_error.recnum = recnum;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_INVALID_RECORD_NUMBER);
    }


  pos = (NV_INT64) recnum * (NV_INT64) wlfh[hnd].record_size + (NV_INT64) wlfh[hnd].header_size;

  if (fseeko64 (wlfh[hnd].fp, pos, SEEK_SET) < 0)
    {
      wlf_error.system = errno;
      wlf_error.recnum = recnum;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_READ_FSEEK_ERROR);
    }

  if (!fread (buffer, wlfh[hnd].record_size, 1, wlfh[hnd].fp))
    {
      wlf_error.system = errno;
      wlf_error.recnum = recnum;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_READ_ERROR);
    }


  /*  Unpack the record.  */

  unpack_point_record (hnd, buffer, wlf_record);


  /*  Read and unpack the waveform address and waveforms if requested and present  */

  if (wave_flag && wlfh[hnd].header.number_of_waveforms)
    {
      if (fseeko64 (wlfh[hnd].fp, wlfh[hnd].waveform_address + wlfh[hnd].waveform_block_offset, SEEK_SET) < 0)
        {
          wlf_error.system = errno;
          wlf_error.recnum = recnum;
          strcpy (wlf_error.file, wlfh[hnd].path);
          return (wlf_error.wlf = WLF_WAVE_READ_FSEEK_ERROR);
        }

      if (!fread (wlfh[hnd].wave_buffer, wlfh[hnd].waveform_blocksize_bytes, 1, wlfh[hnd].fp))
        {
          wlf_error.system = errno;
          wlf_error.recnum = recnum;
          strcpy (wlf_error.file, wlfh[hnd].path);
          return (wlf_error.wlf = WLF_WAVE_READ_ERROR);
        }

      bytes = wlf_bit_unpack (wlfh[hnd].wave_buffer, 0, wlfh[hnd].waveform_blocksize_bits);

      bytes -= wlfh[hnd].waveform_blocksize_bytes;

      if (!fread (wlfh[hnd].wave_buffer, bytes, 1, wlfh[hnd].fp))
        {
          wlf_error.system = errno;
          wlf_error.recnum = recnum;
          strcpy (wlf_error.file, wlfh[hnd].path);
          return (wlf_error.wlf = WLF_WAVE_READ_ERROR);
        }

      bit_pos = 0;
      for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++)
        {
          wlfh[hnd].waveform_start_value_size = 8;
          range = wlfh[hnd].header.max_waveform[i] - wlfh[hnd].header.min_waveform[i];
          if (range > 255) wlfh[hnd].waveform_start_value_size = 16;
          if (range > 65535) wlfh[hnd].waveform_start_value_size = 32;


          /*  Defacto sign extension of the start and bias values (wlf_bit_unpack doesn't sign extend and these
              are supposed to be signed integers).  */

          switch (wlfh[hnd].waveform_start_value_size)
            {
            case 8:
              start8 = wlf_bit_unpack (wlfh[hnd].wave_buffer, bit_pos, 8);
              wlfh[hnd].wave[i][0] = start8 + wlfh[hnd].header.min_waveform[i];
              bit_pos += 8;
              bias8 = wlf_bit_unpack (wlfh[hnd].wave_buffer, bit_pos, 8);
              min_diff = bias8;
              break;

            case 16:
              start16 = wlf_bit_unpack (wlfh[hnd].wave_buffer, bit_pos, 16);
              wlfh[hnd].wave[i][0] = start16 + wlfh[hnd].header.min_waveform[i];
              bit_pos += 16;
              bias16 = wlf_bit_unpack (wlfh[hnd].wave_buffer, bit_pos, 16);
              min_diff = bias16;
              break;

            case 32:
              start32 = wlf_bit_unpack (wlfh[hnd].wave_buffer, bit_pos, 32);
              wlfh[hnd].wave[i][0] = start32 + wlfh[hnd].header.min_waveform[i];
              bit_pos += 32;
              bias32 = wlf_bit_unpack (wlfh[hnd].wave_buffer, bit_pos, 32);
              min_diff = bias32;
              break;
            }
          bit_pos += wlfh[hnd].waveform_start_value_size;


          /*  Get the number of bits needed to store each offset.  */

          num_bits = wlf_bit_unpack (wlfh[hnd].wave_buffer, bit_pos, 5);
          bit_pos += 5;

          for (j = 1 ; j < wlfh[hnd].header.waveform_count[i] ; j++)
            {
              /*  Just in case we stored a completely flat waveform array.  */

              if (num_bits)
                {
                  diff = wlf_bit_unpack (wlfh[hnd].wave_buffer, bit_pos, num_bits);
                  bit_pos += num_bits;
                }
              else
                {
                  diff = 0;
                }

              wlfh[hnd].wave[i][j] = wlfh[hnd].wave[i][j - 1] + diff + min_diff;
            }
        }

      *wave = wlfh[hnd].wave;
    }


  wlfh[hnd].at_end = NVFalse;
  wlfh[hnd].write = NVFalse;


  wlf_error.system = 0;
  return (wlf_error.wlf = WLF_SUCCESS);
}



/*********************************************************************************************

  Function:    wlf_append_record

  Purpose:     Store a WLF record and associated waveforms (if present) at the end of a WLF
               file and update the number_of_records.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               wlf_record     -    The WLF record
               wave           -    The waveforms
               shot_num       -    The shot number

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_APPEND_NOT_CREATING_ERROR
                                   WLF_APPEND_FSEEK_ERROR
                                   WLF_WAVE_WRITE_FSEEK_ERROR
                                   WLF_WAVE_WRITE_ERROR
                                   WLF_RECORD_WRITE_FSEEK_ERROR
                                   WLF_RECORD_WRITE_ERROR
                                   WLF_VALUE_OUT_OF_RANGE_ERROR

  Caveats:     The shot number is used to prevent storing redundant waveform data when you
               have multiple returns per shot.  If the shot number doesn't change between
               calls to wlf_append_record then the waveform_address that is stored in the
               first retrun for this shot is stored with all subsequent returns.

*********************************************************************************************/

WLF_DLL NV_INT32 wlf_append_record (NV_INT32 hnd, WLF_RECORD wlf_record, NV_INT32 **wave, NV_INT32 shot_num)
{
  if (!wlfh[hnd].created)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_APPEND_NOT_CREATING_ERROR);
    }


  if (!wlfh[hnd].at_end)
    {
      if (fseeko64 (wlfh[hnd].fp, 0LL, SEEK_END) < 0)
        {
          wlf_error.system = errno;
          strcpy (wlf_error.file, wlfh[hnd].path);
          return (wlf_error.wlf = WLF_APPEND_FSEEK_ERROR);
        }
    }
  else
    {
      /*  Flush the buffer if the last thing we did was a read operation.  */

      if (!wlfh[hnd].write) fflush (wlfh[hnd].fp);
    }


  if (write_point_waveform_record (hnd, wlf_record, wave, shot_num) < 0) return (wlf_error.wlf);


  wlfh[hnd].header.number_of_records++;
  wlfh[hnd].modified = NVTrue;
  wlfh[hnd].write = NVTrue;
  wlfh[hnd].at_end = NVTrue;

  wlf_error.system = 0;
  return (wlf_error.wlf = WLF_SUCCESS);
}


/*********************************************************************************************

  Function:    wlf_update_record

  Purpose:     Update the modifiable fields of a record.  See the WLF_RECORD structure
               definition in wlf.h to see which fields are modifiable after file creation.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               recnum         -    The record number
               wlf_record     -    The WLF_RECORD structure to save.

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_INVALID_RECORD_NUMBER
                                   WLF_UPDATE_RECORD_FSEEK_ERROR
                                   WLF_UPDATE_RECORD_READ_ERROR
                                   WLF_UPDATE_RECORD_WRITE_ERROR

*********************************************************************************************/

WLF_DLL NV_INT32 wlf_update_record (NV_INT32 hnd, NV_INT32 recnum, WLF_RECORD wlf_record)
{
  NV_INT64 pos;
  NV_U_BYTE buffer[1024];
  NV_INT32 i, value;


  if (recnum >= wlfh[hnd].header.number_of_records || recnum < 0)
    {
      wlf_error.recnum = recnum;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_INVALID_RECORD_NUMBER);
    }


  pos = (NV_INT64) recnum * (NV_INT64) wlfh[hnd].record_size + (NV_INT64) wlfh[hnd].header_size;

  if (fseeko64 (wlfh[hnd].fp, pos, SEEK_SET) < 0)
    {
      wlf_error.system = errno;
      wlf_error.recnum = recnum;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_UPDATE_RECORD_FSEEK_ERROR);
    }


  if (!fread (buffer, wlfh[hnd].record_size, 1, wlfh[hnd].fp))
    {
      wlf_error.system = errno;
      wlf_error.recnum = recnum;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_UPDATE_RECORD_READ_ERROR);
    }


  /*  Optional attributes  */

  for (i = 0 ; i < WLF_MAX_ATTR ; i++)
    {
      if (wlfh[hnd].attr_bits > 0)
        {
          if (wlf_record.attribute[i] < wlfh[hnd].header.min_attr[i] || wlf_record.attribute[i] > wlfh[hnd].header.max_attr[i])
            {
              wlf_error.system = 0;
              strcpy (wlf_error.file, wlfh[hnd].path);
              sprintf (wlf_error.info, _("Attribute %02d value %f out of range %f to %f"), i + 1, wlf_record.attribute[i],
                       wlfh[hnd].header.min_attr[i], wlfh[hnd].header.max_attr[i]);
              return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
            }

          value = NINT ((wlf_record.attribute[i] - wlfh[hnd].header.min_attr[i]) * wlfh[hnd].header.attr_scale[i]);
          wlf_bit_pack (buffer, wlfh[hnd].attr_pos[i], wlfh[hnd].attr_bits[i], value);
        }
    }


  /*  RGB  */

  if (wlfh[hnd].rgb_bits > 0)
    {
      if (wlf_record.red > wlfh[hnd].header.max_rgb || wlf_record.green > wlfh[hnd].header.max_rgb ||
          wlf_record.blue > wlfh[hnd].header.max_rgb)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("RGB value(s) %d, %d, %d out of range 0 to %d"), wlf_record.red, wlf_record.green,
                   wlf_record.blue, wlfh[hnd].header.max_rgb);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      wlf_bit_pack (buffer, wlfh[hnd].rgb_pos, wlfh[hnd].rgb_bits, wlf_record.red);
      wlf_bit_pack (buffer, wlfh[hnd].rgb_pos + wlfh[hnd].rgb_bits, wlfh[hnd].rgb_bits, wlf_record.green);
      wlf_bit_pack (buffer, wlfh[hnd].rgb_pos + wlfh[hnd].rgb_bits * 2, wlfh[hnd].rgb_bits, wlf_record.blue);
    }


  /*  Reflectance  */

  if (wlfh[hnd].reflectance_bits > 0)
    {
      if (wlf_record.reflectance < wlfh[hnd].header.min_reflectance || wlf_record.reflectance > wlfh[hnd].header.max_reflectance)
        {
          wlf_error.system = 0;
          strcpy (wlf_error.file, wlfh[hnd].path);
          sprintf (wlf_error.info, _("Reflectance value %f out of range %f to %f"), wlf_record.reflectance,
                   wlfh[hnd].header.min_reflectance, wlfh[hnd].header.max_reflectance);
          return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
        }

      value = NINT ((wlf_record.reflectance - wlfh[hnd].header.min_reflectance) * wlfh[hnd].header.reflectance_scale);
      wlf_bit_pack (buffer, wlfh[hnd].reflectance_pos, wlfh[hnd].reflectance_bits, value);
    }


  /*  Classification  */

  if (wlf_record.classification > wlfh[hnd].max_class)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("Classification value %d out of range 0 to %d"), wlf_record.classification, wlfh[hnd].max_class);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }
  wlf_bit_pack (buffer, wlfh[hnd].class_pos, wlfh[hnd].class_bits, wlf_record.classification);


  /*  Status  */

  if (wlf_record.status > wlfh[hnd].max_status)
    {
      wlf_error.system = 0;
      strcpy (wlf_error.file, wlfh[hnd].path);
      sprintf (wlf_error.info, _("Status value %d out of range 0 to %d"), wlf_record.status, wlfh[hnd].max_status);
      return (wlf_error.wlf = WLF_VALUE_OUT_OF_RANGE_ERROR);
    }
  wlf_bit_pack (buffer, wlfh[hnd].status_pos, wlfh[hnd].status_bits, wlf_record.status);


  if (fseeko64 (wlfh[hnd].fp, pos, SEEK_SET) < 0)
    {
      wlf_error.system = errno;
      wlf_error.recnum = recnum;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_UPDATE_RECORD_FSEEK_ERROR);
    }

  if (!fwrite (buffer, wlfh[hnd].record_size, 1, wlfh[hnd].fp))
    {
      wlf_error.system = errno;
      wlf_error.recnum = recnum;
      strcpy (wlf_error.file, wlfh[hnd].path);
      return (wlf_error.wlf = WLF_UPDATE_RECORD_WRITE_ERROR);
    }


  wlfh[hnd].modified = NVTrue;
  wlfh[hnd].write = NVTrue;


  wlf_error.system = 0;
  return (wlf_error.wlf = WLF_SUCCESS);
}



/*********************************************************************************************

  Function:    wlf_update_header

  Purpose:     Update the modifiable fields of the header record.  See WLF_HEADER in wlf.h to
               determine which fields are modifiable after file creation.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               wlf_header     -    The WLF_HEADER structure.

  Returns:     N/A

*********************************************************************************************/

WLF_DLL void wlf_update_header (NV_INT32 hnd, WLF_HEADER wlf_header)
{
  strcpy (wlfh[hnd].header.modification_software, wlf_header.modification_software);
  strcpy (wlfh[hnd].header.security_classification, wlf_header.security_classification);
  strcpy (wlfh[hnd].header.distribution, wlf_header.distribution);
  strcpy (wlfh[hnd].header.declassification, wlf_header.declassification);
  strcpy (wlfh[hnd].header.class_just, wlf_header.class_just);
  strcpy (wlfh[hnd].header.downgrade, wlf_header.downgrade);
  strcpy (wlfh[hnd].header.comments, wlf_header.comments);


  /*  Force a header write when we close the file.  */

  wlfh[hnd].modified = NVTrue;
  wlfh[hnd].write = NVTrue;
}



/*********************************************************************************************

  Function:    wlf_get_errno

  Purpose:     Returns the latest WLF error condition code

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     NV_INT32       -    error condition code

  Caveats:     The only thing this is good for at present is to determine if, when you opened
               the file, the library version was older than the file.  That is, if
               WLF_NEWER_FILE_VERSION_WARNING has been set when you called wlf_open_file.
               Otherwise, you can just use wlf_perror or wlf_strerror to get the last
               error information.

*********************************************************************************************/

WLF_DLL NV_INT32 wlf_get_errno ()
{
  return (wlf_error.wlf);
}



/*********************************************************************************************

  Function:    wlf_strerror

  Purpose:     Returns the error string related to the latest error.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     NV_CHAR       -    Error message

*********************************************************************************************/

WLF_DLL NV_CHAR *wlf_strerror ()
{
  static NV_CHAR message[1024];

  switch (wlf_error.wlf)
    {
    case WLF_SUCCESS:
      sprintf (message, _("SUCCESS!\n"));
      break;

    case WLF_WAVE_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing waveform data :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_WAVE_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing waveform data :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_RECORD_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing record data :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_RECORD_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing record data :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_HEADER_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing header :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_HEADER_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing header :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_TOO_MANY_OPEN_FILES:
      sprintf (message, _("Too many WLF files are already open.\n"));
      break;

    case WLF_CREATE_ERROR:
      sprintf (message, _("File : %s\nError creating WLF file :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_CREATE_WAVE_ERROR:
      sprintf (message, _("File : %s\nError creating temporary waveform file :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_OPEN_UPDATE_ERROR:
      sprintf (message, _("File : %s\nError opening WLF file for update :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_OPEN_READONLY_ERROR:
      sprintf (message, _("File : %s\nError opening WLF file read-only :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_NOT_WLF_FILE_ERROR:
      sprintf (message, _("File : %s\nThe file version string is corrupt or indicates that this is not a WLF file.\n"),
               wlf_error.file);
      break;

    case WLF_NEWER_FILE_VERSION_WARNING:
      sprintf (message, _("File : %s\nThe file version is newer than the WLF library version.\nThis may cause problems.\n"),
               wlf_error.file);
      break;

    case WLF_CLOSE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek to end of record file to append waveforms :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_CLOSE_WAVE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek to beginning of temporary waveform file to append waveforms :\n%s\n"), 
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_CLOSE_WAVE_READ_ERROR:
      sprintf (message, _("File : %s\nError reading from temporary waveform file while appending to the end of the WLF file :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_CLOSE_WAVE_WRITE_ERROR:
      sprintf (message, _("File : %s\nError appending waveforms to the end of the WLF file :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_CLOSE_ERROR:
      sprintf (message, _("File : %s\nError closing WLF file :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

    case WLF_INVALID_RECORD_NUMBER:
      sprintf (message, _("File : %s\nRecord : %d\nInvalid record number.\n"),
               wlf_error.file, wlf_error.recnum);
      break;

    case WLF_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError during fseek prior reading a record :\n%s\n"),
               wlf_error.file, wlf_error.recnum, strerror (wlf_error.system));
      break;

    case WLF_READ_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError reading record :\n%s\n"),
               wlf_error.file, wlf_error.recnum, strerror (wlf_error.system));
      break;

    case WLF_APPEND_NOT_CREATING_ERROR:
      sprintf (message, _("File : %s\nAppending to previously created files is not allowed.\n"),
               wlf_error.file);
      break;

    case WLF_APPEND_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to appending a record :\n%s\n"),
               wlf_error.file, strerror (wlf_error.system));
      break;

   case WLF_WAVE_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError during fseek prior to reading waveform data :\n%s\n"),
               wlf_error.file, wlf_error.recnum, strerror (wlf_error.system));
      break;

    case WLF_WAVE_READ_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError reading waveform data :\n%s\n"),
               wlf_error.file, wlf_error.recnum, strerror (wlf_error.system));
      break;

    case WLF_UPDATE_RECORD_FSEEK_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError during fseek while updating record data :\n%s\n"),
               wlf_error.file, wlf_error.recnum, strerror (wlf_error.system));
      break;

    case WLF_UPDATE_RECORD_READ_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError reading record prior to updating record data :\n%s\n"),
               wlf_error.file, wlf_error.recnum, strerror (wlf_error.system));
      break;

    case WLF_UPDATE_RECORD_WRITE_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError updating record value :\n%s\n"),
               wlf_error.file, wlf_error.recnum, strerror (wlf_error.system));
      break;

    case WLF_VALUE_OUT_OF_RANGE_ERROR:
    case WLF_SENSOR_POSITION_RANGE_ERROR:
    case WLF_SENSOR_ATTITUDE_RANGE_ERROR:
      sprintf (message, _("File : %s\n%s\n"), wlf_error.file, wlf_error.info);
      break;
    }

  return (message);
}



/*********************************************************************************************

  Function:    wlf_perror

  Purpose:     Prints (to stderr) the latest error messages.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     N/A

*********************************************************************************************/

WLF_DLL void wlf_perror ()
{
  fprintf (stderr, wlf_strerror ());
  fflush (stderr);
}



/*********************************************************************************************

  Function:    wlf_get_version

  Purpose:     Returns the WLF library version string

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     NV_CHAR       -    version string

*********************************************************************************************/

WLF_DLL NV_CHAR *wlf_get_version ()
{
  return (WLF_VERSION);
}



/*********************************************************************************************

  Function:    wlf_dump_record

  Purpose:     Print the WLF record to stdout.  If you don't want to dump the waveforms just
               send NULL as the **wave address.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               wlf_record     -    The WLF record
               wave           -    Pointer to waveforms or NULL

  Returns:     N/A

*********************************************************************************************/

WLF_DLL void wlf_dump_record (NV_INT32 hnd, WLF_RECORD wlf_record, NV_INT32 **wave)
{
  NV_INT32        i, j, k, start, end, year, day, hour, minute, month, mday;
  NV_FLOAT32      second;


  wlf_cvtime (wlf_record.tv_sec, wlf_record.tv_nsec, &year, &day, &hour, &minute, &second);
  wlf_jday2mday (year, day, &month, &mday);
  month++;

  printf (N_("******************************************************************\n"));
  printf (_("Date/Time : %d-%02d-%02d (%d) %02d:%02d:%05.9f\n"), year + 1900, month, mday, day, hour, minute, second);

  if (wlfh[hnd].h_uncert_bits) printf (_("Horizontal uncertainty : %f\n"), wlf_record.horizontal_uncertainty);
  if (wlfh[hnd].v_uncert_bits) printf (_("Vertical uncertainty : %f\n"), wlf_record.vertical_uncertainty);

  printf (_("X : %.11lf\n"), wlf_record.x);
  printf (_("Y : %.11lf\n"), wlf_record.y);
  printf (_("Z : %f\n"), wlf_record.z);

  if (wlfh[hnd].waveform_bits)
    {
      printf (_("Waveform : %d\n"), wlf_record.waveform);
      printf (_("Waveform Point : %d\n"), wlf_record.waveform_point);
    }

  if (wlfh[hnd].sensor_x_bits)
    {
      printf (_("Sensor X : %.11lf\n"), wlf_record.sensor_x);
      printf (_("Sensor Y : %.11lf\n"), wlf_record.sensor_y);
      printf (_("Sensor Z : %f\n"), wlf_record.sensor_z);
    }

  if (wlfh[hnd].sensor_roll_bits)
    {
      printf (_("Sensor Roll : %f\n"), wlf_record.sensor_roll);
      printf (_("Sensor Pitch : %f\n"), wlf_record.sensor_pitch);
      printf (_("Sensor Heading : %f\n"), wlf_record.sensor_heading);
    }

  if (wlfh[hnd].scan_angle_bits) printf (_("Scan angle : %f\n"), wlf_record.scan_angle);

  if (wlfh[hnd].nadir_angle_bits) printf (_("Nadir angle : %f\n"), wlf_record.nadir_angle);

  if (wlfh[hnd].water_surface_bits) printf (_("Water surface : %f\n"), wlf_record.water_surface);

  if (wlfh[hnd].z_offset_bits) printf (_("Z offset : %f\n"), wlf_record.z_offset);

  printf (_("Number of returns : %d\n"), wlf_record.number_of_returns);
  printf (_("Return number : %d\n"), wlf_record.return_number);

  if (wlfh[hnd].point_source_bits) printf (_("Point source ID : %d\n"), wlf_record.point_source);

  if (wlfh[hnd].edge_bits) printf (_("Edge flag : %hd\n"), (NV_INT16) wlf_record.edge_of_flight_line);

  printf (_("Classification : %d\n"), wlf_record.classification);

  if (wlfh[hnd].intensity_bits) printf (_("Intensity : %f\n"), wlf_record.intensity);

  if (wlfh[hnd].rgb_bits)
    {
      printf (_("Red : %d\n"), wlf_record.red);
      printf (_("Green : %d\n"), wlf_record.green);
      printf (_("Blue : %d\n"), wlf_record.blue);
    }

  if (wlfh[hnd].reflectance_bits) printf (_("Reflectance : %f\n"), wlf_record.reflectance);


  printf (_("Status : %x\n"), wlf_record.status);


  /*  Waveforms (if present).  */

  if (wave != NULL)
    {
      for (i = 0 ; i < wlfh[hnd].header.number_of_waveforms ; i++)
        {
          printf (_("\n*****************  %s waveform values  *****************\n"), wlfh[hnd].header.waveform_name[i]);

          for (j = 0 ; j < wlfh[hnd].header.waveform_count[i] ; j += 10)
            {
              if (j >= wlfh[hnd].header.waveform_count[i]) break;

              start = j;
              end = MIN (j + 10, wlfh[hnd].header.waveform_count[i]);

              printf ("%04d-%04d : ", start, end - 1);

              for (k = start ; k < end ; k++) printf ("%05d ", wave[i][k]);

              printf ("\n");
            }
        }
      printf ("\n");
    }


  fflush (stdout);
}
