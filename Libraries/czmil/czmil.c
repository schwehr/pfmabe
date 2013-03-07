
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

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "czmil.h"
#include "czmil_internals.h"
#include "czmil_version.h"


/*  This is where we'll store the headers and formatting/usgae information of all open CZMIL files
    (see czmil_internals.h).  */

static INTERNAL_CZMIL_STRUCT czmilh[CZMIL_MAX_FILES];


/*  This is where we'll store error information in the event of some kind of screwup (see czmil_internals.h).  */

static CZMIL_ERROR_STRUCT czmil_error;


/*  Startup flag used by either czmil_create_file or czmil_open_file to initialize the internal struct array and
    set the SIGINT handler.  */

static NV_BOOL first = NVTrue;


/*  These should never be called by an application program so we're defining them here.  */

static NV_INT32 czmil_read_cdx_record (NV_INT32 hnd, NV_INT32 recnum);
static NV_INT32 czmil_append_cdx_record (NV_INT32 hnd, CZMIL_CDX_Data *record);


/*  Insert a bunch of static utility functions that really don't need to live in this file.  */

#include "czmil_functions.h"



/********************************************************************************************/
/*!

 - Function:    czmil_write_cxy_header

 - Purpose:     Write the CXY ASCII file header to the CZMIL CXY file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The CZMIL file handle

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_CXY_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CXY_HEADER_WRITE_ERROR

*********************************************************************************************/

static NV_INT32 czmil_write_cxy_header (NV_INT32 hnd)
{
  NV_CHAR space = ' ';
  NV_INT32 i, size, year, jday, hour, minute, month, day;
  NV_FLOAT32 second;


  /*  Position to the beginning of the file.  */

  if (fseeko64 (czmilh[hnd].cxy_fp, 0LL, SEEK_SET) < 0)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_HEADER_WRITE_FSEEK_ERROR);
    }


  /*  Write the tagged ASCII fields to the ASCII header.  Why not use XML?  Because these files are going to be HUGE!
      The header is only a few tens of thousands of bytes.  If you mistakenly think this is XML and you try to open
      the file with an XML reader it will not be pretty.  The other reason is that XML is designed for MUCH more
      complicated data.  It also usually requires a schema and/or external stuff.  We don't need all that complexity.
      Remember the KISS principle - Keep It Simple Stupid.  */

  fprintf (czmilh[hnd].cxy_fp, N_("[VERSION] = %s\n"), CZMIL_VERSION);
  fprintf (czmilh[hnd].cxy_fp,
           N_("[FILE TYPE] = Optech International Coastal Zone Mapping and Imaging LiDAR (CZMIL) XYZ File\n"));

  czmil_cvtime (czmilh[hnd].cxy_header.creation_timestamp, &year, &jday, &hour, &minute, &second);
  czmil_jday2mday (year, jday, &month, &day);
  month++;

  fprintf (czmilh[hnd].cxy_fp, N_("[CREATION YEAR] = %d\n"), year + 1900);
  fprintf (czmilh[hnd].cxy_fp, N_("[CREATION MONTH] = %02d\n"), month);
  fprintf (czmilh[hnd].cxy_fp, N_("[CREATION DAY] = %02d\n"), day);
  fprintf (czmilh[hnd].cxy_fp, N_("[CREATION DAY OF YEAR] = %03d\n"), jday);
  fprintf (czmilh[hnd].cxy_fp, N_("[CREATION HOUR] = %02d\n"), hour);
  fprintf (czmilh[hnd].cxy_fp, N_("[CREATION MINUTE] = %02d\n"), minute);
  fprintf (czmilh[hnd].cxy_fp, N_("[CREATION SECOND] = %5.2f\n"), second);
  if (strlen (czmilh[hnd].cxy_header.creation_software) > 2) fprintf (czmilh[hnd].cxy_fp,
                                                                      N_("[CREATION SOFTWARE] = %s\n"),
                                                                      czmilh[hnd].cxy_header.creation_software);

  czmil_cvtime (czmilh[hnd].cxy_header.modification_timestamp, &year, &jday, &hour, &minute, &second);
  czmil_jday2mday (year, jday, &month, &day);
  month++;

  fprintf (czmilh[hnd].cxy_fp, N_("[MODIFICATION YEAR] = %d\n"), year + 1900);
  fprintf (czmilh[hnd].cxy_fp, N_("[MODIFICATION MONTH] = %02d\n"), month);
  fprintf (czmilh[hnd].cxy_fp, N_("[MODIFICATION DAY] = %02d\n"), day);
  fprintf (czmilh[hnd].cxy_fp, N_("[MODIFICATION DAY OF YEAR] = %03d\n"), jday);
  fprintf (czmilh[hnd].cxy_fp, N_("[MODIFICATION HOUR] = %02d\n"), hour);
  fprintf (czmilh[hnd].cxy_fp, N_("[MODIFICATION MINUTE] = %02d\n"), minute);
  fprintf (czmilh[hnd].cxy_fp, N_("[MODIFICATION SECOND] = %5.2f\n"), second);
  if (strlen (czmilh[hnd].cxy_header.modification_software) > 2) fprintf (czmilh[hnd].cxy_fp,
                                                                          N_("[MODIFICATION SOFTWARE] = %s\n"),
                                                                          czmilh[hnd].cxy_header.modification_software);

  if (strlen (czmilh[hnd].cxy_header.security_classification) > 2)
    fprintf (czmilh[hnd].cxy_fp, N_("[SECURITY CLASSIFICATION] = %s\n"), czmilh[hnd].cxy_header.security_classification);
  if (strlen (czmilh[hnd].cxy_header.distribution) > 2)
    fprintf (czmilh[hnd].cxy_fp, N_("{DISTRIBUTION = \n%s\n}\n"), czmilh[hnd].cxy_header.distribution);
  if (strlen (czmilh[hnd].cxy_header.declassification) > 2)
    fprintf (czmilh[hnd].cxy_fp, N_("{DECLASSIFICATION = \n%s\n}\n"), czmilh[hnd].cxy_header.declassification);
  if (strlen (czmilh[hnd].cxy_header.class_just) > 2)
    fprintf (czmilh[hnd].cxy_fp, N_("{SECURITY CLASSIFICATION JUSTIFICATION = \n%s\n}\n"), czmilh[hnd].cxy_header.class_just);
  if (strlen (czmilh[hnd].cxy_header.downgrade) > 2)
    fprintf (czmilh[hnd].cxy_fp, N_("{DOWNGRADE = \n%s\n}\n"), czmilh[hnd].cxy_header.downgrade);

  fprintf (czmilh[hnd].cxy_fp, N_("[MIN LONGITUDE] = %.11f\n"), czmilh[hnd].cxy_header.min_lon);
  fprintf (czmilh[hnd].cxy_fp, N_("[MIN LATITUDE] = %.11f\n"), czmilh[hnd].cxy_header.min_lat);
  fprintf (czmilh[hnd].cxy_fp, N_("[MAX LONGITUDE] = %.11f\n"), czmilh[hnd].cxy_header.max_lon);
  fprintf (czmilh[hnd].cxy_fp, N_("[MAX LATITUDE] = %.11f\n"), czmilh[hnd].cxy_header.max_lat);

  fprintf (czmilh[hnd].cxy_fp, N_("[MIN PLATFORM LONGITUDE] = %.11f\n"), czmilh[hnd].cxy_header.min_platform_lon);
  fprintf (czmilh[hnd].cxy_fp, N_("[MIN PLATFORM LATITUDE] = %.11f\n"), czmilh[hnd].cxy_header.min_platform_lat);
  fprintf (czmilh[hnd].cxy_fp, N_("[MAX PLATFORM LONGITUDE] = %.11f\n"), czmilh[hnd].cxy_header.max_platform_lon);
  fprintf (czmilh[hnd].cxy_fp, N_("[MAX PLATFORM LATITUDE] = %.11f\n"), czmilh[hnd].cxy_header.max_platform_lat);

  fprintf (czmilh[hnd].cxy_fp, N_("[NUMBER OF RECORDS] = %d\n"), czmilh[hnd].cxy_header.number_of_records);

  czmilh[hnd].cxy_header.header_size = CZMIL_CXY_HEADER_SIZE;
  fprintf (czmilh[hnd].cxy_fp, N_("[HEADER SIZE] = %d\n"), czmilh[hnd].cxy_header.header_size);

  fprintf (czmilh[hnd].cxy_fp, N_("[SYSTEM TYPE] = %d\n"), czmilh[hnd].cxy_header.system_type);
  fprintf (czmilh[hnd].cxy_fp, N_("[SYSTEM NUMBER] = %d\n"), czmilh[hnd].cxy_header.system_number);
  fprintf (czmilh[hnd].cxy_fp, N_("[SYSTEM REP RATE] = %d\n"), czmilh[hnd].cxy_header.rep_rate);

  if (strlen (czmilh[hnd].cxy_header.project) > 2) fprintf (czmilh[hnd].cxy_fp, N_("[PROJECT] = %s\n"),
                                                            czmilh[hnd].cxy_header.project);
  if (strlen (czmilh[hnd].cxy_header.mission) > 2) fprintf (czmilh[hnd].cxy_fp, N_("[MISSION] = %s\n"),
                                                            czmilh[hnd].cxy_header.mission);
  if (strlen (czmilh[hnd].cxy_header.dataset) > 2) fprintf (czmilh[hnd].cxy_fp, N_("[DATASET] = %s\n"),
                                                            czmilh[hnd].cxy_header.dataset);
  if (strlen (czmilh[hnd].cxy_header.flight_id) > 2) fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT ID] = %s\n"),
                                                              czmilh[hnd].cxy_header.flight_id);

  if (czmilh[hnd].cxy_header.flight_start_timestamp)
    {
      czmil_cvtime (czmilh[hnd].cxy_header.flight_start_timestamp, &year, &jday, &hour, &minute, &second);
      czmil_jday2mday (year, jday, &month, &day);
      month++;

      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT START YEAR] = %d\n"), year + 1900);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT START MONTH] = %02d\n"), month);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT START DAY] = %02d\n"), day);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT START DAY OF YEAR] = %03d\n"), jday);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT START HOUR] = %02d\n"), hour);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT START MINUTE] = %02d\n"), minute);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT START SECOND] = %5.2f\n"), second);
    }


  if (czmilh[hnd].cxy_header.flight_end_timestamp)
    {
      czmil_cvtime (czmilh[hnd].cxy_header.flight_end_timestamp, &year, &jday, &hour, &minute, &second);
      czmil_jday2mday (year, jday, &month, &day);
      month++;

      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT END YEAR] = %d\n"), year + 1900);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT END MONTH] = %02d\n"), month);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT END DAY] = %02d\n"), day);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT END DAY OF YEAR] = %03d\n"), jday);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT END HOUR] = %02d\n"), hour);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT END MINUTE] = %02d\n"), minute);
      fprintf (czmilh[hnd].cxy_fp, N_("[FLIGHT END SECOND] = %5.2f\n"), second);
    }


  fprintf (czmilh[hnd].cxy_fp, N_("[NULL Z VALUE] = %.5f\n"), czmilh[hnd].cxy_header.null_z_value);

  fprintf (czmilh[hnd].cxy_fp, N_("{WELL-KNOWN TEXT = \n%s\n}\n"), czmilh[hnd].cxy_header.wkt);
  fprintf (czmilh[hnd].cxy_fp, N_("[LOCAL VERTICAL DATUM] = %02d\n"), czmilh[hnd].cxy_header.local_vertical_datum);

  if (strlen (czmilh[hnd].cxy_header.comments) > 2) fprintf (czmilh[hnd].cxy_fp, N_("{COMMENTS = \n%s\n}\n"), czmilh[hnd].cxy_header.comments);

  fprintf (czmilh[hnd].cxy_fp, N_("[BUFFER SIZE BYTES] = %d\n"), czmilh[hnd].cxy_buffer_size_bytes);
  fprintf (czmilh[hnd].cxy_fp, N_("[RETURN BITS] = %d\n"), czmilh[hnd].cxy_return_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[TIME BITS] = %d\n"), czmilh[hnd].cxy_time_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[ANGLE SCALE] = %f\n"), czmilh[hnd].cxy_angle_scale);
  fprintf (czmilh[hnd].cxy_fp, N_("[SCAN ANGLE BITS] = %d\n"), czmilh[hnd].cxy_scan_angle_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[ROLL AND PITCH BITS] = %d\n"), czmilh[hnd].cxy_roll_pitch_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[HEADING BITS] = %d\n"), czmilh[hnd].cxy_heading_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[DOP BITS] = %d\n"), czmilh[hnd].cxy_dop_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[POS SCALE] = %f\n"), czmilh[hnd].cxy_pos_scale);
  fprintf (czmilh[hnd].cxy_fp, N_("[LAT DIFF BITS] = %d\n"), czmilh[hnd].cxy_lat_diff_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[ELEV BITS] = %d\n"), czmilh[hnd].cxy_elev_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[ELEV SCALE] = %f\n"), czmilh[hnd].cxy_elev_scale);
  fprintf (czmilh[hnd].cxy_fp, N_("[UNCERT BITS] = %d\n"), czmilh[hnd].cxy_uncert_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[UNCERT SCALE] = %f\n"), czmilh[hnd].cxy_uncert_scale);
  fprintf (czmilh[hnd].cxy_fp, N_("[REFLECTANCE BITS] = %d\n"), czmilh[hnd].cxy_reflectance_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[REFLECTANCE SCALE] = %f\n"), czmilh[hnd].cxy_reflectance_scale);
  fprintf (czmilh[hnd].cxy_fp, N_("[STATUS BITS] = %d\n"), czmilh[hnd].cxy_status_bits);
  fprintf (czmilh[hnd].cxy_fp, N_("[WAVEFORM CLASS BITS] = %d\n"), czmilh[hnd].cxy_waveform_class_bits);

  fprintf (czmilh[hnd].cxy_fp, N_("[END OF HEADER]\n"));


  /*  Space fill the rest.  */

  size = czmilh[hnd].cxy_header.header_size - ftell (czmilh[hnd].cxy_fp);


  for (i = 0 ; i < size ; i++)
    {
      if (!fwrite (&space, 1, 1, czmilh[hnd].cxy_fp))
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cxy_path);
          return (czmil_error.czmil = CZMIL_CXY_HEADER_WRITE_ERROR);
        }
    }


  czmilh[hnd].cxy_pos = czmilh[hnd].cxy_header.header_size;
  czmilh[hnd].cxy_write = NVTrue;


  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_read_cxy_header

 - Purpose:     Read the CXY ASCII file header from the CZMIL CXY file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The CZMIL file handle

 - Returns:     - CZMIL_SUCCESS
                - CZMIL_CXY_HEADER_READ_FSEEK_ERROR
                - CZMIL_NOT_CZMIL_FILE_ERROR
                - CZMIL_NEWER_FILE_VERSION_WARNING

*********************************************************************************************/

static NV_INT32 czmil_read_cxy_header (NV_INT32 hnd)
{
  NV_INT32 i, j, year[4], jday[4], hour[4], minute[4];
  NV_INT16 major_version;
  NV_FLOAT32 second[4], tmpf;
  NV_CHAR varin[8192], info[8192];


  /*  Position to the beginning of the file.  */

  if (fseeko64 (czmilh[hnd].cxy_fp, 0LL, SEEK_SET) < 0)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_HEADER_READ_FSEEK_ERROR);
    }


  /*  We want to try to read the first line (version info) with an fread in case we mistakenly asked to
      load a binary file.  If we try to use czmil_ngets to read a binary file and there are no line feeds in 
      the first sizeof (varin) characters we would segfault since fgets doesn't check for overrun.  */

  if (!fread (varin, 128, 1, czmilh[hnd].cxy_fp))
    {
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_NOT_CZMIL_FILE_ERROR);
    }


  /*  Check for the CZMIL library string at the beginning of the file.  */

  if (!strstr (varin, N_("CZMIL library")))
    {
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_NOT_CZMIL_FILE_ERROR);
    }


  /*  Rewind to the beginning of the file.  Yes, we'll read the version again but we need to check the version number
      anyway.  */

  fseeko64 (czmilh[hnd].cxy_fp, 0LL, SEEK_SET);


  /*  Read the tagged ASCII header data.  Note, we're using czmil_ngets instead of fgets since we really don't want the
      CR/LF in the strings.  */

  while (czmil_ngets (varin, sizeof (varin), czmilh[hnd].cxy_fp))
    {
      if (strstr (varin, N_("[END OF HEADER]"))) break;


      /*  Put everything to the right of the equals sign into 'info'.   */

      czmil_get_string (varin, info);


      /*  Read the version string and check the major version number against the library major version.  */

      if (strstr (varin, N_("[VERSION]")))
        {
          strcpy (czmilh[hnd].cxy_header.version, info);
          strcpy (info, strstr (varin, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          major_version = (NV_INT16) tmpf;

          strcpy (info, strstr (CZMIL_VERSION, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          if (major_version > (NV_INT16) tmpf)
            {
              strcpy (czmil_error.file, czmilh[hnd].cxy_path);
              czmil_error.czmil = CZMIL_NEWER_FILE_VERSION_WARNING;
            }
        }

      if (strstr (varin, N_("[FILE TYPE]"))) strcpy (czmilh[hnd].cxy_header.file_type, info);

      if (strstr (varin, N_("[CREATION YEAR]"))) sscanf (info, "%d", &year[0]);
      if (strstr (varin, N_("[CREATION DAY OF YEAR]"))) sscanf (info, "%d", &jday[0]);
      if (strstr (varin, N_("[CREATION HOUR]"))) sscanf (info, "%d", &hour[0]);
      if (strstr (varin, N_("[CREATION MINUTE]"))) sscanf (info, "%d", &minute[0]);
      if (strstr (varin, N_("[CREATION SECOND]"))) sscanf (info, "%f", &second[0]);
      if (strstr (varin, N_("[CREATION SOFTWARE]"))) strcpy (czmilh[hnd].cxy_header.creation_software, info);

      if (strstr (varin, N_("[MODIFICATION YEAR]"))) sscanf (info, "%d", &year[1]);
      if (strstr (varin, N_("[MODIFICATION DAY OF YEAR]"))) sscanf (info, "%d", &jday[1]);
      if (strstr (varin, N_("[MODIFICATION HOUR]"))) sscanf (info, "%d", &hour[1]);
      if (strstr (varin, N_("[MODIFICATION MINUTE]"))) sscanf (info, "%d", &minute[1]);
      if (strstr (varin, N_("[MODIFICATION SECOND]"))) sscanf (info, "%f", &second[1]);
      if (strstr (varin, N_("[MODIFICATION SOFTWARE]"))) strcpy (czmilh[hnd].cxy_header.modification_software, info);

      if (strstr (varin, N_("[SECURITY CLASSIFICATION]"))) strcpy (czmilh[hnd].cxy_header.security_classification, info);

      if (strstr (varin, N_("{DISTRIBUTION =")))
        {
          strcpy (czmilh[hnd].cxy_header.distribution, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cxy_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cxy_header.distribution, varin);
            }
        }

      if (strstr (varin, N_("{DECLASSIFICATION =")))
        {
          strcpy (czmilh[hnd].cxy_header.declassification, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cxy_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cxy_header.declassification, varin);
            }
        }

      if (strstr (varin, N_("{SECURITY CLASSIFICATION JUSTIFICATION =")))
        {
          strcpy (czmilh[hnd].cxy_header.class_just, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cxy_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cxy_header.class_just, varin);
            }
        }


      if (strstr (varin, N_("{DOWNGRADE =")))
        {
          strcpy (czmilh[hnd].cxy_header.downgrade, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cxy_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cxy_header.downgrade, varin);
            }
        }


      if (strstr (varin, N_("[MIN LONGITUDE]"))) sscanf (info, "%lf", &czmilh[hnd].cxy_header.min_lon);
      if (strstr (varin, N_("[MIN LATITUDE]"))) sscanf (info, "%lf", &czmilh[hnd].cxy_header.min_lat);
      if (strstr (varin, N_("[MAX LONGITUDE]"))) sscanf (info, "%lf", &czmilh[hnd].cxy_header.max_lon);
      if (strstr (varin, N_("[MAX LATITUDE]"))) sscanf (info, "%lf", &czmilh[hnd].cxy_header.max_lat);

      if (strstr (varin, N_("[MIN PLATFORM LONGITUDE]"))) sscanf (info, "%lf", &czmilh[hnd].cxy_header.min_platform_lon);
      if (strstr (varin, N_("[MIN PLATFORM LATITUDE]"))) sscanf (info, "%lf", &czmilh[hnd].cxy_header.min_platform_lat);
      if (strstr (varin, N_("[MAX PLATFORM LONGITUDE]"))) sscanf (info, "%lf", &czmilh[hnd].cxy_header.max_platform_lon);
      if (strstr (varin, N_("[MAX PLATFORM LATITUDE]"))) sscanf (info, "%lf", &czmilh[hnd].cxy_header.max_platform_lat);

      if (strstr (varin, N_("[NUMBER OF RECORDS]"))) sscanf (info, "%d", &czmilh[hnd].cxy_header.number_of_records);

      if (strstr (varin, N_("[HEADER SIZE]"))) sscanf (info, "%d", &czmilh[hnd].cxy_header.header_size);

      if (strstr (varin, N_("[SYSTEM TYPE]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_header.system_type);
      if (strstr (varin, N_("[SYSTEM NUMBER]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_header.system_number);
      if (strstr (varin, N_("[SYSTEM REP RATE]"))) sscanf (info, "%d", &czmilh[hnd].cxy_header.rep_rate);

      if (strstr (varin, N_("[PROJECT]"))) strcpy (czmilh[hnd].cxy_header.project, info);
      if (strstr (varin, N_("[MISSION]"))) strcpy (czmilh[hnd].cxy_header.mission, info);
      if (strstr (varin, N_("[DATASET]"))) strcpy (czmilh[hnd].cxy_header.dataset, info);
      if (strstr (varin, N_("[FLIGHT ID]"))) strcpy (czmilh[hnd].cxy_header.flight_id, info);

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

      if (strstr (varin, N_("[NULL Z VALUE]"))) sscanf (info, "%f", &czmilh[hnd].cxy_header.null_z_value);

      if (strstr (varin, N_("{WELL-KNOWN TEXT =")))
        {
          strcpy (czmilh[hnd].cxy_header.wkt, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cxy_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cxy_header.wkt, varin);
            }
        }

      if (strstr (varin, N_("[LOCAL VERTICAL DATUM]"))) sscanf (info, "%hd",
                                                                &czmilh[hnd].cxy_header.local_vertical_datum);

      if (strstr (varin, N_("{COMMENTS =")))
        {
          strcpy (czmilh[hnd].cxy_header.comments, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cxy_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cxy_header.comments, varin);
            }
        }

      if (strstr (varin, N_("[BUFFER SIZE BYTES]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_buffer_size_bytes);
      if (strstr (varin, N_("[RETURN BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_return_bits);
      if (strstr (varin, N_("[TIME BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_time_bits);
      if (strstr (varin, N_("[ANGLE SCALE]"))) sscanf (info, "%f", &czmilh[hnd].cxy_angle_scale);
      if (strstr (varin, N_("[SCAN ANGLE BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_scan_angle_bits);
      if (strstr (varin, N_("[ROLL AND PITCH BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_roll_pitch_bits);
      if (strstr (varin, N_("[HEADING BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_heading_bits);
      if (strstr (varin, N_("[DOP BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_dop_bits);
      if (strstr (varin, N_("[POS SCALE]"))) sscanf (info, "%f", &czmilh[hnd].cxy_pos_scale);
      if (strstr (varin, N_("[LAT DIFF BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_lat_diff_bits);
      if (strstr (varin, N_("[ELEV BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_elev_bits);
      if (strstr (varin, N_("[ELEV SCALE]"))) sscanf (info, "%f", &czmilh[hnd].cxy_elev_scale);
      if (strstr (varin, N_("[UNCERT BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_uncert_bits);
      if (strstr (varin, N_("[UNCERT SCALE]"))) sscanf (info, "%f", &czmilh[hnd].cxy_uncert_scale);
      if (strstr (varin, N_("[REFLECTANCE BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_reflectance_bits);
      if (strstr (varin, N_("[REFLECTANCE SCALE]"))) sscanf (info, "%f", &czmilh[hnd].cxy_reflectance_scale);
      if (strstr (varin, N_("[STATUS BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_status_bits);
      if (strstr (varin, N_("[WAVEFORM CLASS BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cxy_waveform_class_bits);
    }


  /*  Compute the remaining field definitions from the input header data.  */

  czmilh[hnd].cxy_lat_bits = (NV_INT32) (log10 ((NV_FLOAT64) czmilh[hnd].cxy_pos_scale * 180.0L) / LOG2) + 1;
  czmilh[hnd].cxy_lon_bits = (NV_INT32) (log10 ((NV_FLOAT64) czmilh[hnd].cxy_pos_scale * 360.0L) / LOG2) + 1;
  czmilh[hnd].cxy_lat_diff_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_lat_diff_bits)) - 1;
  czmilh[hnd].cxy_lat_diff_offset = czmilh[hnd].cxy_lat_diff_max / 2;
  czmilh[hnd].cxy_elev_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_elev_bits)) - 1;
  czmilh[hnd].cxy_elev_offset = czmilh[hnd].cxy_elev_max / 2;
  czmilh[hnd].cxy_roll_pitch_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_roll_pitch_bits)) - 1;
  czmilh[hnd].cxy_roll_pitch_offset = czmilh[hnd].cxy_roll_pitch_max / 2;
  czmilh[hnd].cxy_reflectance_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_reflectance_bits)) - 1;
  czmilh[hnd].cxy_reflectance_offset = czmilh[hnd].cxy_reflectance_max / 2;
  czmilh[hnd].cxy_waveform_class_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_waveform_class_bits)) - 1;
  czmilh[hnd].cxy_waveform_class_offset = czmilh[hnd].cxy_waveform_class_max / 2;
  czmilh[hnd].cxy_uncert_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_uncert_bits)) - 1;
  czmilh[hnd].cxy_scan_angle_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_scan_angle_bits)) - 1;
  czmilh[hnd].cxy_scan_angle_offset = czmilh[hnd].cxy_scan_angle_max / 2;


  /*  Create the array of longitude difference bits and values.  The array subscript used will be based on the
      latitude degrees.  This allows us to increase the number of bits used to store longitude differences from
      the platform longitude as we move towards the poles.  We start with one more bit than cxy_lat_diff_bits at
      0-60 degrees latitude and add bits as 1 over the cosine of the latitude increases by a power of two.  The
      size of a degree of longitude decreases approximately as the cosine of the latitude so there is a halving of
      longitude distance at 60.0, 75.52, 82.82, and 86.42 degrees.  At approximately these intervals we need to add 
      a bit in order to double the stored values.  This will maintain better than 1 millimeter resolution up to 86
      degrees (N or S).  After 86 degrees we'll be storing the full resolution longitude.  Even though there will
      only be 5 discrete values in the array we're using an array of 180 values to allow us to index into the array
      each time we pack or unpack a record instead of using multiple if statements to determine the number of bits
      used.  This way we only need one if statement for the greater than 86 cases.  */

  for (i = 0, j = 89 ; i < 90 ; i++, j--)
    {
      if (i >= 86)
        {
          czmilh[hnd].cxy_lon_diff_bits[i + 90] = FULL_LON_FLAG;
          czmilh[hnd].cxy_lon_diff_max[i + 90] = 0;
          czmilh[hnd].cxy_lon_diff_offset[i + 90] = 0;
          if (j >= 0)
            {
              czmilh[hnd].cxy_lon_diff_bits[j] = FULL_LON_FLAG;
              czmilh[hnd].cxy_lon_diff_max[j] = 0;
              czmilh[hnd].cxy_lon_diff_offset[j] = 0;
            }
        }
      else
        {
          if (i > 81)
            {
              czmilh[hnd].cxy_lon_diff_bits[i + 90] = czmilh[hnd].cxy_lon_diff_bits[j] = czmilh[hnd].cxy_lat_diff_bits + 4;
            }
          else if (i > 74)
            {
              czmilh[hnd].cxy_lon_diff_bits[i + 90] = czmilh[hnd].cxy_lon_diff_bits[j] = czmilh[hnd].cxy_lat_diff_bits + 3;
            }
          else if (i > 60)
            {
              czmilh[hnd].cxy_lon_diff_bits[i + 90] = czmilh[hnd].cxy_lon_diff_bits[j] = czmilh[hnd].cxy_lat_diff_bits + 2;
            }
          else
            {
              czmilh[hnd].cxy_lon_diff_bits[i + 90] = czmilh[hnd].cxy_lon_diff_bits[j] = czmilh[hnd].cxy_lat_diff_bits + 1;
            }

          czmilh[hnd].cxy_lon_diff_max[i + 90] = czmilh[hnd].cxy_lon_diff_max[j] =
            (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_lon_diff_bits[i])) - 1;
          czmilh[hnd].cxy_lon_diff_offset[i + 90] = czmilh[hnd].cxy_lon_diff_offset[j] =
            czmilh[hnd].cxy_lon_diff_max[i] / 2;
        }
    }


  /*  Convert the date and time strings read from the header to timestamps.  */

  czmil_inv_cvtime (year[0], jday[0], hour[0], minute[0], second[0], &czmilh[hnd].cxy_header.creation_timestamp);
  czmil_inv_cvtime (year[1], jday[1], hour[1], minute[1], second[1], &czmilh[hnd].cxy_header.modification_timestamp);
  czmil_inv_cvtime (year[2], jday[2], hour[2], minute[2], second[2], &czmilh[hnd].cxy_header.flight_start_timestamp);
  czmil_inv_cvtime (year[3], jday[3], hour[3], minute[3], second[3], &czmilh[hnd].cxy_header.flight_end_timestamp);


  /*  Seek to the end of the header.  */

  fseeko64 (czmilh[hnd].cxy_fp, czmilh[hnd].cxy_header.header_size, SEEK_SET);
  czmilh[hnd].cxy_pos = czmilh[hnd].cxy_header.header_size;
  czmilh[hnd].cxy_write = NVFalse;


  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_write_cwf_header

 - Purpose:     Write the CWF ASCII file header to the CZMIL CWF file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The CZMIL file handle

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_CWF_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CWF_HEADER_WRITE_ERROR

*********************************************************************************************/

static NV_INT32 czmil_write_cwf_header (NV_INT32 hnd)
{
  NV_CHAR space = ' ';
  NV_INT32 i, size, year, jday, hour, minute, month, day;
  NV_FLOAT32 second;


  /*  Position to the beginning of the file.  */

  if (fseeko64 (czmilh[hnd].cwf_fp, 0LL, SEEK_SET) < 0)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_CWF_HEADER_WRITE_FSEEK_ERROR);
    }


  /*  Write the tagged ASCII fields to the ASCII header.  Why not use XML?  Because these files are going to be HUGE!
      The header is only a few tens of thousands of bytes.  If you mistakenly think this is XML and you try to open
      the file with an XML reader it will not be pretty.  The other reason is that XML is designed for MUCH more
      complicated data.  It also usually requires a schema and/or external stuff.  We don't need all that complexity.
      Remember the KISS principle - Keep It Simple Stupid.  */

  fprintf (czmilh[hnd].cwf_fp, N_("[VERSION] = %s\n"), CZMIL_VERSION);
  fprintf (czmilh[hnd].cwf_fp, N_("[FILE TYPE] = Optech International Coastal Zone Mapping and Imaging LiDAR (CZMIL) Waveform File\n"));

  czmil_cvtime (czmilh[hnd].cwf_header.creation_timestamp, &year, &jday, &hour, &minute, &second);
  czmil_jday2mday (year, jday, &month, &day);
  month++;

  fprintf (czmilh[hnd].cwf_fp, N_("[CREATION YEAR] = %d\n"), year + 1900);
  fprintf (czmilh[hnd].cwf_fp, N_("[CREATION MONTH] = %02d\n"), month);
  fprintf (czmilh[hnd].cwf_fp, N_("[CREATION DAY] = %02d\n"), day);
  fprintf (czmilh[hnd].cwf_fp, N_("[CREATION DAY OF YEAR] = %03d\n"), jday);
  fprintf (czmilh[hnd].cwf_fp, N_("[CREATION HOUR] = %02d\n"), hour);
  fprintf (czmilh[hnd].cwf_fp, N_("[CREATION MINUTE] = %02d\n"), minute);
  fprintf (czmilh[hnd].cwf_fp, N_("[CREATION SECOND] = %5.2f\n"), second);
  if (strlen (czmilh[hnd].cwf_header.creation_software) > 2) fprintf (czmilh[hnd].cwf_fp,
                                                                      N_("[CREATION SOFTWARE] = %s\n"),
                                                                      czmilh[hnd].cwf_header.creation_software);

  if (strlen (czmilh[hnd].cwf_header.security_classification) > 2)
    fprintf (czmilh[hnd].cwf_fp, N_("[SECURITY CLASSIFICATION] = %s\n"), czmilh[hnd].cwf_header.security_classification);
  if (strlen (czmilh[hnd].cwf_header.distribution) > 2)
    fprintf (czmilh[hnd].cwf_fp, N_("{DISTRIBUTION = \n%s\n}\n"), czmilh[hnd].cwf_header.distribution);
  if (strlen (czmilh[hnd].cwf_header.declassification) > 2)
    fprintf (czmilh[hnd].cwf_fp, N_("{DECLASSIFICATION = \n%s\n}\n"), czmilh[hnd].cwf_header.declassification);
  if (strlen (czmilh[hnd].cwf_header.class_just) > 2)
    fprintf (czmilh[hnd].cwf_fp, N_("{SECURITY CLASSIFICATION JUSTIFICATION = \n%s\n}\n"), czmilh[hnd].cwf_header.class_just);
  if (strlen (czmilh[hnd].cwf_header.downgrade) > 2)
    fprintf (czmilh[hnd].cwf_fp, N_("{DOWNGRADE = \n%s\n}\n"), czmilh[hnd].cwf_header.downgrade);

  fprintf (czmilh[hnd].cwf_fp, N_("[NUMBER OF RECORDS] = %d\n"), czmilh[hnd].cwf_header.number_of_records);

  czmilh[hnd].cwf_header.header_size = CZMIL_CWF_HEADER_SIZE;
  fprintf (czmilh[hnd].cwf_fp, N_("[HEADER SIZE] = %d\n"), czmilh[hnd].cwf_header.header_size);

  fprintf (czmilh[hnd].cwf_fp, N_("[SYSTEM TYPE] = %d\n"), czmilh[hnd].cwf_header.system_type);
  fprintf (czmilh[hnd].cwf_fp, N_("[SYSTEM NUMBER] = %d\n"), czmilh[hnd].cwf_header.system_number);
  fprintf (czmilh[hnd].cwf_fp, N_("[SYSTEM REP RATE] = %d\n"), czmilh[hnd].cwf_header.rep_rate);

  if (strlen (czmilh[hnd].cwf_header.project) > 2) fprintf (czmilh[hnd].cwf_fp, N_("[PROJECT] = %s\n"),
                                                            czmilh[hnd].cwf_header.project);
  if (strlen (czmilh[hnd].cwf_header.mission) > 2) fprintf (czmilh[hnd].cwf_fp, N_("[MISSION] = %s\n"),
                                                            czmilh[hnd].cwf_header.mission);
  if (strlen (czmilh[hnd].cwf_header.dataset) > 2) fprintf (czmilh[hnd].cwf_fp, N_("[DATASET] = %s\n"),
                                                            czmilh[hnd].cwf_header.dataset);
  if (strlen (czmilh[hnd].cwf_header.flight_id) > 2) fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT ID] = %s\n"),
                                                              czmilh[hnd].cwf_header.flight_id);

  if (czmilh[hnd].cwf_header.flight_start_timestamp)
    {
      czmil_cvtime (czmilh[hnd].cwf_header.flight_start_timestamp, &year, &jday, &hour, &minute, &second);
      czmil_jday2mday (year, jday, &month, &day);
      month++;

      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT START YEAR] = %d\n"), year + 1900);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT START MONTH] = %02d\n"), month);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT START DAY] = %02d\n"), day);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT START DAY OF YEAR] = %03d\n"), jday);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT START HOUR] = %02d\n"), hour);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT START MINUTE] = %02d\n"), minute);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT START SECOND] = %5.2f\n"), second);
    }


  if (czmilh[hnd].cwf_header.flight_end_timestamp)
    {
      czmil_cvtime (czmilh[hnd].cwf_header.flight_end_timestamp, &year, &jday, &hour, &minute, &second);
      czmil_jday2mday (year, jday, &month, &day);
      month++;

      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT END YEAR] = %d\n"), year + 1900);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT END MONTH] = %02d\n"), month);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT END DAY] = %02d\n"), day);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT END DAY OF YEAR] = %03d\n"), jday);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT END HOUR] = %02d\n"), hour);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT END MINUTE] = %02d\n"), minute);
      fprintf (czmilh[hnd].cwf_fp, N_("[FLIGHT END SECOND] = %5.2f\n"), second);
    }

  if (strlen (czmilh[hnd].cwf_header.comments) > 2) fprintf (czmilh[hnd].cwf_fp, N_("{COMMENTS = \n%s\n}\n"), czmilh[hnd].cwf_header.comments);

  fprintf (czmilh[hnd].cwf_fp, N_("[BUFFER SIZE BYTES] = %d\n"), czmilh[hnd].cwf_buffer_size_bytes);
  fprintf (czmilh[hnd].cwf_fp, N_("[TYPE BITS] = %hd\n"), czmilh[hnd].cwf_type_bits);
  fprintf (czmilh[hnd].cwf_fp, N_("[TYPE 1 START BITS] = %hd\n"), czmilh[hnd].cwf_type_1_start_bits);
  fprintf (czmilh[hnd].cwf_fp, N_("[TYPE 2 START BITS] = %hd\n"), czmilh[hnd].cwf_type_2_start_bits);
  fprintf (czmilh[hnd].cwf_fp, N_("[DELTA BITS] = %hd\n"), czmilh[hnd].cwf_delta_bits);
  fprintf (czmilh[hnd].cwf_fp, N_("[CZMIL MAX PACKETS] = %hd\n"), czmilh[hnd].cwf_czmil_max_packets);
  fprintf (czmilh[hnd].cwf_fp, N_("[TIME BITS] = %d\n"), czmilh[hnd].cwf_time_bits);
  fprintf (czmilh[hnd].cwf_fp, N_("[ANGLE SCALE] = %f\n"), czmilh[hnd].cwf_angle_scale);
  fprintf (czmilh[hnd].cwf_fp, N_("[SCAN ANGLE BITS] = %d\n"), czmilh[hnd].cwf_scan_angle_bits);

  fprintf (czmilh[hnd].cwf_fp, N_("[END OF HEADER]\n"));


  /*  Space fill the rest.  */

  size = czmilh[hnd].cwf_header.header_size - ftell (czmilh[hnd].cwf_fp);


  for (i = 0 ; i < size ; i++)
    {
      if (!fwrite (&space, 1, 1, czmilh[hnd].cwf_fp))
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cwf_path);
          return (czmil_error.czmil = CZMIL_CWF_HEADER_WRITE_ERROR);
        }
    }


  czmilh[hnd].cwf_pos = czmilh[hnd].cwf_header.header_size;
  czmilh[hnd].cwf_write = NVTrue;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_read_cwf_header

 - Purpose:     Read the CWF ASCII file header from the CZMIL CWF file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The CZMIL file handle

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_CWF_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CWF_HEADER_WRITE_ERROR

*********************************************************************************************/

static NV_INT32 czmil_read_cwf_header (NV_INT32 hnd)
{
  NV_INT32 year[3], jday[3], hour[3], minute[3];
  NV_INT16 major_version;
  NV_FLOAT32 second[4], tmpf;
  NV_CHAR varin[8192], info[8192];


  /*  Position to the beginning of the file.  */

  if (fseeko64 (czmilh[hnd].cwf_fp, 0LL, SEEK_SET) < 0)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_CWF_HEADER_READ_FSEEK_ERROR);
    }


  /*  We want to try to read the first line (version info) with an fread in case we mistakenly asked to
      load a binary file.  If we try to use czmil_ngets to read a binary file and there are no line feeds in 
      the first sizeof (varin) characters we would segfault since fgets doesn't check for overrun.  */

  if (!fread (varin, 128, 1, czmilh[hnd].cwf_fp))
    {
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_NOT_CZMIL_FILE_ERROR);
    }


  /*  Check for the CZMIL library string at the beginning of the file.  */

  if (!strstr (varin, N_("CZMIL library")))
    {
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_NOT_CZMIL_FILE_ERROR);
    }


  /*  Rewind to the beginning of the file.  Yes, we'll read the version again but we need to check the version number
      anyway.  */

  fseeko64 (czmilh[hnd].cwf_fp, 0LL, SEEK_SET);


  /*  Read the tagged ASCII header data.  Note, we're using czmil_ngets instead of fgets since we really don't want the
      CR/LF in the strings.  */

  while (czmil_ngets (varin, sizeof (varin), czmilh[hnd].cwf_fp))
    {
      if (strstr (varin, N_("[END OF HEADER]"))) break;


      /*  Put everything to the right of the equals sign into 'info'.   */

      czmil_get_string (varin, info);


      /*  Read the version string and check the major version number against the library major version.  */

      if (strstr (varin, N_("[VERSION]")))
        {
          strcpy (czmilh[hnd].cwf_header.version, info);
          strcpy (info, strstr (varin, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          major_version = (NV_INT16) tmpf;

          strcpy (info, strstr (CZMIL_VERSION, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          if (major_version > (NV_INT16) tmpf)
            {
              strcpy (czmil_error.file, czmilh[hnd].cxy_path);
              czmil_error.czmil = CZMIL_NEWER_FILE_VERSION_WARNING;
            }
        }

      if (strstr (varin, N_("[FILE TYPE]"))) strcpy (czmilh[hnd].cwf_header.file_type, info);

      if (strstr (varin, N_("[CREATION YEAR]"))) sscanf (info, "%d", &year[0]);
      if (strstr (varin, N_("[CREATION DAY OF YEAR]"))) sscanf (info, "%d", &jday[0]);
      if (strstr (varin, N_("[CREATION HOUR]"))) sscanf (info, "%d", &hour[0]);
      if (strstr (varin, N_("[CREATION MINUTE]"))) sscanf (info, "%d", &minute[0]);
      if (strstr (varin, N_("[CREATION SECOND]"))) sscanf (info, "%f", &second[0]);
      if (strstr (varin, N_("[CREATION SOFTWARE]"))) strcpy (czmilh[hnd].cwf_header.creation_software, info);

      if (strstr (varin, N_("[SECURITY CLASSIFICATION]"))) strcpy (czmilh[hnd].cwf_header.security_classification, info);

      if (strstr (varin, N_("{DISTRIBUTION =")))
        {
          strcpy (czmilh[hnd].cwf_header.distribution, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cwf_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cwf_header.distribution, varin);
            }
        }

      if (strstr (varin, N_("{DECLASSIFICATION =")))
        {
          strcpy (czmilh[hnd].cwf_header.declassification, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cwf_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cwf_header.declassification, varin);
            }
        }

      if (strstr (varin, N_("{SECURITY CLASSIFICATION JUSTIFICATION =")))
        {
          strcpy (czmilh[hnd].cwf_header.class_just, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cwf_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cwf_header.class_just, varin);
            }
        }


      if (strstr (varin, N_("{DOWNGRADE =")))
        {
          strcpy (czmilh[hnd].cwf_header.downgrade, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cwf_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cwf_header.downgrade, varin);
            }
        }


      if (strstr (varin, N_("[NUMBER OF RECORDS]"))) sscanf (info, "%d", &czmilh[hnd].cwf_header.number_of_records);

      if (strstr (varin, N_("[HEADER SIZE]"))) sscanf (info, "%d", &czmilh[hnd].cwf_header.header_size);

      if (strstr (varin, N_("[SYSTEM TYPE]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_header.system_type);
      if (strstr (varin, N_("[SYSTEM NUMBER]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_header.system_number);
      if (strstr (varin, N_("[SYSTEM REP RATE]"))) sscanf (info, "%d", &czmilh[hnd].cwf_header.rep_rate);

      if (strstr (varin, N_("[PROJECT]"))) strcpy (czmilh[hnd].cwf_header.project, info);
      if (strstr (varin, N_("[MISSION]"))) strcpy (czmilh[hnd].cwf_header.mission, info);
      if (strstr (varin, N_("[DATASET]"))) strcpy (czmilh[hnd].cwf_header.dataset, info);
      if (strstr (varin, N_("[FLIGHT ID]"))) strcpy (czmilh[hnd].cwf_header.flight_id, info);

      if (strstr (varin, N_("[FLIGHT START YEAR]"))) sscanf (info, "%d", &year[1]);
      if (strstr (varin, N_("[FLIGHT START DAY OF YEAR]"))) sscanf (info, "%d", &jday[1]);
      if (strstr (varin, N_("[FLIGHT START HOUR]"))) sscanf (info, "%d", &hour[1]);
      if (strstr (varin, N_("[FLIGHT START MINUTE]"))) sscanf (info, "%d", &minute[1]);
      if (strstr (varin, N_("[FLIGHT START SECOND]"))) sscanf (info, "%f", &second[1]);

      if (strstr (varin, N_("[FLIGHT END YEAR]"))) sscanf (info, "%d", &year[2]);
      if (strstr (varin, N_("[FLIGHT END DAY OF YEAR]"))) sscanf (info, "%d", &jday[2]);
      if (strstr (varin, N_("[FLIGHT END HOUR]"))) sscanf (info, "%d", &hour[2]);
      if (strstr (varin, N_("[FLIGHT END MINUTE]"))) sscanf (info, "%d", &minute[2]);
      if (strstr (varin, N_("[FLIGHT END SECOND]"))) sscanf (info, "%f", &second[2]);

      if (strstr (varin, N_("{COMMENTS =")))
        {
          strcpy (czmilh[hnd].cwf_header.comments, "");
          while (fgets (varin, sizeof (varin), czmilh[hnd].cwf_fp))
            {
              if (varin[0] == '}') break;
              strcat (czmilh[hnd].cwf_header.comments, varin);
            }
        }

      if (strstr (varin, N_("[BUFFER SIZE BYTES]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_buffer_size_bytes);
      if (strstr (varin, N_("[TYPE BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_type_bits);
      if (strstr (varin, N_("[TYPE 1 START BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_type_1_start_bits);
      if (strstr (varin, N_("[TYPE 2 START BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_type_2_start_bits);
      if (strstr (varin, N_("[DELTA BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_delta_bits);
      if (strstr (varin, N_("[CZMIL MAX PACKETS]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_czmil_max_packets);
      if (strstr (varin, N_("[TIME BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_time_bits);
      if (strstr (varin, N_("[ANGLE SCALE]"))) sscanf (info, "%f", &czmilh[hnd].cwf_angle_scale);
      if (strstr (varin, N_("[SCAN ANGLE BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cwf_scan_angle_bits);
    }


  /*  Compute the remaining field definitions from the input header data.  */

  czmilh[hnd].cwf_type_0_bytes = (czmilh[hnd].cwf_type_bits + 64 * 10) / 8;
  if ((czmilh[hnd].cwf_type_bits + 64 * 10) % 8) czmilh[hnd].cwf_type_0_bytes++;
  czmilh[hnd].cwf_type_1_offset_bits = czmilh[hnd].cwf_type_1_start_bits + 1;
  czmilh[hnd].cwf_type_1_offset = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_type_1_start_bits)) - 1;
  czmilh[hnd].cwf_type_2_offset_bits = czmilh[hnd].cwf_type_2_start_bits + 1;
  czmilh[hnd].cwf_type_2_offset = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_type_2_start_bits)) - 1;
  czmilh[hnd].cwf_type_3_offset_bits = czmilh[hnd].cwf_type_1_start_bits + 1;
  czmilh[hnd].cwf_scan_angle_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_scan_angle_bits)) - 1;
  czmilh[hnd].cwf_scan_angle_offset = czmilh[hnd].cwf_scan_angle_max / 2;
  czmilh[hnd].cwf_type_1_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_1_start_bits +
    czmilh[hnd].cwf_type_1_offset_bits + czmilh[hnd].cwf_delta_bits;
  czmilh[hnd].cwf_type_2_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_1_start_bits +
    czmilh[hnd].cwf_type_1_offset_bits + czmilh[hnd].cwf_type_2_start_bits + czmilh[hnd].cwf_type_2_offset_bits +
    czmilh[hnd].cwf_delta_bits;
  czmilh[hnd].cwf_type_3_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_3_offset_bits +
    czmilh[hnd].cwf_delta_bits;


  /*  Convert the date and time strings read from the header to timestamps.  */

  czmil_inv_cvtime (year[0], jday[0], hour[0], minute[0], second[0], &czmilh[hnd].cwf_header.creation_timestamp);
  czmil_inv_cvtime (year[1], jday[1], hour[1], minute[1], second[1], &czmilh[hnd].cwf_header.flight_start_timestamp);
  czmil_inv_cvtime (year[2], jday[2], hour[2], minute[2], second[2], &czmilh[hnd].cwf_header.flight_end_timestamp);


  /*  Seek to the end of the header.  */

  fseeko64 (czmilh[hnd].cwf_fp, czmilh[hnd].cwf_header.header_size, SEEK_SET);
  czmilh[hnd].cwf_pos = czmilh[hnd].cwf_header.header_size;
  czmilh[hnd].cwf_write = NVFalse;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_write_cdx_header

 - Purpose:     Write the CDX ASCII file header to the CZMIL CDX file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The CZMIL file handle

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_CDX_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CDX_HEADER_WRITE_ERROR

*********************************************************************************************/

static NV_INT32 czmil_write_cdx_header (NV_INT32 hnd)
{
  NV_CHAR space = ' ';
  NV_INT32 i, size, year, jday, hour, minute, month, day;
  NV_FLOAT32 second;


  /*  Position to the beginning of the file.  */

  if (fseeko64 (czmilh[hnd].cdx_fp, 0LL, SEEK_SET) < 0)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_CDX_HEADER_WRITE_FSEEK_ERROR);
    }


  /*  Write the tagged ASCII fields to the ASCII header.  Why not use XML?  Because these files are going to be HUGE!
      The header is only a few tens of thousands of bytes.  If you mistakenly think this is XML and you try to open
      the file with an XML reader it will not be pretty.  The other reason is that XML is designed for MUCH more
      complicated data.  It also usually requires a schema and/or external stuff.  We don't need all that complexity.
      Remember the KISS principle - Keep It Simple Stupid.  */

  fprintf (czmilh[hnd].cdx_fp, N_("[VERSION] = %s\n"), CZMIL_VERSION);
  fprintf (czmilh[hnd].cdx_fp, N_("[FILE TYPE] = Optech International Coastal Zone Mapping and Imaging LiDAR (CZMIL) Index File\n"));

  czmil_cvtime (czmilh[hnd].cdx_header.creation_timestamp, &year, &jday, &hour, &minute, &second);
  czmil_jday2mday (year, jday, &month, &day);
  month++;

  fprintf (czmilh[hnd].cdx_fp, N_("[CREATION YEAR] = %d\n"), year + 1900);
  fprintf (czmilh[hnd].cdx_fp, N_("[CREATION MONTH] = %02d\n"), month);
  fprintf (czmilh[hnd].cdx_fp, N_("[CREATION DAY] = %02d\n"), day);
  fprintf (czmilh[hnd].cdx_fp, N_("[CREATION DAY OF YEAR] = %03d\n"), jday);
  fprintf (czmilh[hnd].cdx_fp, N_("[CREATION HOUR] = %02d\n"), hour);
  fprintf (czmilh[hnd].cdx_fp, N_("[CREATION MINUTE] = %02d\n"), minute);
  fprintf (czmilh[hnd].cdx_fp, N_("[CREATION SECOND] = %5.2f\n"), second);
  if (strlen (czmilh[hnd].cdx_header.creation_software) > 2) fprintf (czmilh[hnd].cdx_fp,
                                                                      N_("[CREATION SOFTWARE] = %s\n"),
                                                                      czmilh[hnd].cdx_header.creation_software);

  fprintf (czmilh[hnd].cdx_fp, N_("[NUMBER OF RECORDS] = %d\n"), czmilh[hnd].cdx_header.number_of_records);

  fprintf (czmilh[hnd].cdx_fp, N_("[HEADER SIZE] = %d\n"), czmilh[hnd].cdx_header.header_size);

  if (strlen (czmilh[hnd].cdx_header.project) > 2) fprintf (czmilh[hnd].cdx_fp, N_("[PROJECT] = %s\n"),
                                                            czmilh[hnd].cdx_header.project);
  if (strlen (czmilh[hnd].cdx_header.mission) > 2) fprintf (czmilh[hnd].cdx_fp, N_("[MISSION] = %s\n"),
                                                            czmilh[hnd].cdx_header.mission);
  if (strlen (czmilh[hnd].cdx_header.dataset) > 2) fprintf (czmilh[hnd].cdx_fp, N_("[DATASET] = %s\n"),
                                                            czmilh[hnd].cdx_header.dataset);
  if (strlen (czmilh[hnd].cdx_header.flight_id) > 2) fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT ID] = %s\n"),
                                                              czmilh[hnd].cdx_header.flight_id);

  if (czmilh[hnd].cdx_header.flight_start_timestamp)
    {
      czmil_cvtime (czmilh[hnd].cdx_header.flight_start_timestamp, &year, &jday, &hour, &minute, &second);
      czmil_jday2mday (year, jday, &month, &day);
      month++;

      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT START YEAR] = %d\n"), year + 1900);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT START MONTH] = %02d\n"), month);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT START DAY] = %02d\n"), day);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT START DAY OF YEAR] = %03d\n"), jday);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT START HOUR] = %02d\n"), hour);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT START MINUTE] = %02d\n"), minute);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT START SECOND] = %5.2f\n"), second);
    }


  if (czmilh[hnd].cdx_header.flight_end_timestamp)
    {
      czmil_cvtime (czmilh[hnd].cdx_header.flight_end_timestamp, &year, &jday, &hour, &minute, &second);
      czmil_jday2mday (year, jday, &month, &day);
      month++;

      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT END YEAR] = %d\n"), year + 1900);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT END MONTH] = %02d\n"), month);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT END DAY] = %02d\n"), day);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT END DAY OF YEAR] = %03d\n"), jday);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT END HOUR] = %02d\n"), hour);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT END MINUTE] = %02d\n"), minute);
      fprintf (czmilh[hnd].cdx_fp, N_("[FLIGHT END SECOND] = %5.2f\n"), second);

      fprintf (czmilh[hnd].cdx_fp, N_("[CXY ADDRESS BITS] = %d\n"), czmilh[hnd].cdx_cxy_address_bits);
      fprintf (czmilh[hnd].cdx_fp, N_("[CWF ADDRESS BITS] = %d\n"), czmilh[hnd].cdx_cwf_address_bits);
      fprintf (czmilh[hnd].cdx_fp, N_("[CXY BUFFER SIZE BITS] = %d\n"), czmilh[hnd].cdx_cxy_buffer_size_bits);
      fprintf (czmilh[hnd].cdx_fp, N_("[CWF BUFFER SIZE BITS] = %d\n"), czmilh[hnd].cdx_cwf_buffer_size_bits);
    }

  fprintf (czmilh[hnd].cdx_fp, N_("[END OF HEADER]\n"));


  /*  Space fill the rest.  */

  size = czmilh[hnd].cdx_header.header_size - ftell (czmilh[hnd].cdx_fp);


  for (i = 0 ; i < size ; i++)
    {
      if (!fwrite (&space, 1, 1, czmilh[hnd].cdx_fp))
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cdx_path);
          return (czmil_error.czmil = CZMIL_CDX_HEADER_WRITE_ERROR);
        }
    }

  czmilh[hnd].cdx_pos = czmilh[hnd].cdx_header.header_size;
  czmilh[hnd].cdx_write = NVTrue;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_read_cdx_header

 - Purpose:     Read the CDX ASCII file header from the CZMIL CDX file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The CZMIL file handle

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_CDX_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CDX_HEADER_WRITE_ERROR

*********************************************************************************************/

static NV_INT32 czmil_read_cdx_header (NV_INT32 hnd)
{
  NV_INT32 year[3], jday[3], hour[3], minute[3];
  NV_INT16 major_version;
  NV_FLOAT32 second[4], tmpf;
  NV_CHAR varin[8192], info[8192];


  /*  Position to the beginning of the file.  */

  if (fseeko64 (czmilh[hnd].cdx_fp, 0LL, SEEK_SET) < 0)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_CDX_HEADER_READ_FSEEK_ERROR);
    }


  /*  We want to try to read the first line (version info) with an fread in case we mistakenly asked to
      load a binary file.  If we try to use czmil_ngets to read a binary file and there are no line feeds in 
      the first sizeof (varin) characters we would segfault since fgets doesn't check for overrun.  */

  if (!fread (varin, 128, 1, czmilh[hnd].cdx_fp))
    {
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_NOT_CZMIL_FILE_ERROR);
    }


  /*  Check for the CZMIL library string at the beginning of the file.  */

  if (!strstr (varin, N_("CZMIL library")))
    {
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_NOT_CZMIL_FILE_ERROR);
    }


  /*  Rewind to the beginning of the file.  Yes, we'll read the version again but we need to check the version number
      anyway.  */

  fseeko64 (czmilh[hnd].cdx_fp, 0LL, SEEK_SET);


  /*  Read the tagged ASCII header data.  Note, we're using czmil_ngets instead of fgets since we really don't want the
      CR/LF in the strings.  */

  while (czmil_ngets (varin, sizeof (varin), czmilh[hnd].cdx_fp))
    {
      if (strstr (varin, N_("[END OF HEADER]"))) break;


      /*  Put everything to the right of the equals sign into 'info'.   */

      czmil_get_string (varin, info);


      /*  Read the version string and check the major version number against the library major version.  */

      if (strstr (varin, N_("[VERSION]")))
        {
          strcpy (czmilh[hnd].cdx_header.version, info);
          strcpy (info, strstr (varin, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          major_version = (NV_INT16) tmpf;

          strcpy (info, strstr (CZMIL_VERSION, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          if (major_version > (NV_INT16) tmpf)
            {
              strcpy (czmil_error.file, czmilh[hnd].cxy_path);
              czmil_error.czmil = CZMIL_NEWER_FILE_VERSION_WARNING;
            }
        }

      if (strstr (varin, N_("[FILE TYPE]"))) strcpy (czmilh[hnd].cdx_header.file_type, info);

      if (strstr (varin, N_("[CREATION YEAR]"))) sscanf (info, "%d", &year[0]);
      if (strstr (varin, N_("[CREATION DAY OF YEAR]"))) sscanf (info, "%d", &jday[0]);
      if (strstr (varin, N_("[CREATION HOUR]"))) sscanf (info, "%d", &hour[0]);
      if (strstr (varin, N_("[CREATION MINUTE]"))) sscanf (info, "%d", &minute[0]);
      if (strstr (varin, N_("[CREATION SECOND]"))) sscanf (info, "%f", &second[0]);
      if (strstr (varin, N_("[CREATION SOFTWARE]"))) strcpy (czmilh[hnd].cdx_header.creation_software, info);

      if (strstr (varin, N_("[NUMBER OF RECORDS]"))) sscanf (info, "%d", &czmilh[hnd].cdx_header.number_of_records);

      if (strstr (varin, N_("[HEADER SIZE]"))) sscanf (info, "%d", &czmilh[hnd].cdx_header.header_size);

      if (strstr (varin, N_("[PROJECT]"))) strcpy (czmilh[hnd].cdx_header.project, info);
      if (strstr (varin, N_("[MISSION]"))) strcpy (czmilh[hnd].cdx_header.mission, info);
      if (strstr (varin, N_("[DATASET]"))) strcpy (czmilh[hnd].cdx_header.dataset, info);
      if (strstr (varin, N_("[FLIGHT ID]"))) strcpy (czmilh[hnd].cdx_header.flight_id, info);

      if (strstr (varin, N_("[FLIGHT START YEAR]"))) sscanf (info, "%d", &year[1]);
      if (strstr (varin, N_("[FLIGHT START DAY OF YEAR]"))) sscanf (info, "%d", &jday[1]);
      if (strstr (varin, N_("[FLIGHT START HOUR]"))) sscanf (info, "%d", &hour[1]);
      if (strstr (varin, N_("[FLIGHT START MINUTE]"))) sscanf (info, "%d", &minute[1]);
      if (strstr (varin, N_("[FLIGHT START SECOND]"))) sscanf (info, "%f", &second[1]);


      if (strstr (varin, N_("[FLIGHT END YEAR]"))) sscanf (info, "%d", &year[2]);
      if (strstr (varin, N_("[FLIGHT END DAY OF YEAR]"))) sscanf (info, "%d", &jday[2]);
      if (strstr (varin, N_("[FLIGHT END HOUR]"))) sscanf (info, "%d", &hour[2]);
      if (strstr (varin, N_("[FLIGHT END MINUTE]"))) sscanf (info, "%d", &minute[2]);
      if (strstr (varin, N_("[FLIGHT END SECOND]"))) sscanf (info, "%f", &second[2]);

      if (strstr (varin, N_("[CXY ADDRESS BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cdx_cxy_address_bits);
      if (strstr (varin, N_("[CWF ADDRESS BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cdx_cwf_address_bits);
      if (strstr (varin, N_("[CXY BUFFER SIZE BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cdx_cxy_buffer_size_bits);
      if (strstr (varin, N_("[CWF BUFFER SIZE BITS]"))) sscanf (info, "%hd", &czmilh[hnd].cdx_cwf_buffer_size_bits);
    }


  /*  Compute the remaining field definitions from the input header data.  */

  czmilh[hnd].cdx_record_size_bytes= (czmilh[hnd].cdx_cxy_address_bits + czmilh[hnd].cdx_cwf_address_bits +
                                      czmilh[hnd].cdx_cxy_buffer_size_bits + czmilh[hnd].cdx_cwf_buffer_size_bits) / 8;


  /*  Convert the date and time strings read from the header to timestamps.  */

  czmil_inv_cvtime (year[0], jday[0], hour[0], minute[0], second[0], &czmilh[hnd].cdx_header.creation_timestamp);
  czmil_inv_cvtime (year[1], jday[1], hour[1], minute[1], second[1], &czmilh[hnd].cdx_header.flight_start_timestamp);
  czmil_inv_cvtime (year[2], jday[2], hour[2], minute[2], second[2], &czmilh[hnd].cdx_header.flight_end_timestamp);


  /*  Seek to the end of the header.  */

  fseeko64 (czmilh[hnd].cdx_fp, czmilh[hnd].cdx_header.header_size, SEEK_SET);
  czmilh[hnd].cdx_pos = czmilh[hnd].cdx_header.header_size;
  czmilh[hnd].cdx_write = NVFalse;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_regen_cdx_file

 - Purpose:     Regenerate a missing CZMIL CDX index file from the associated CXY and CWF files.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The CZMIL file handle

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_CXY_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CXY_HEADER_WRITE_ERROR

 - Caveats:     This function is mostly just a placeholder for now.  We're regenerating the
                CDX file by reading through the CXY and CWF files and using the buffer sizes
                to find the location of the next record.

*********************************************************************************************/

static NV_INT32 czmil_regen_cdx_file (NV_INT32 hnd)
{
  NV_INT32            i;
  NV_U_INT16          buffer_size;


  /*  Position the cxy file to the start of the data.  */

  fseeko64 (czmilh[hnd].cxy_fp, czmilh[hnd].cxy_header.header_size, SEEK_SET);


  /*  If the associated CWF file was not opened we're going to force an open anyway (unless it doesn't exist).  */

  if (czmilh[hnd].cwf_fp == NULL)
    {
      /*  Check to see if there is an associated CWF file.  */

      strcpy (czmilh[hnd].cwf_path, czmilh[hnd].cxy_path);
      sprintf (&czmilh[hnd].cwf_path[strlen (czmilh[hnd].cwf_path) - 4], ".cwf");

      if ((czmilh[hnd].cwf_fp = fopen64 (czmilh[hnd].cwf_path, "rb")) != NULL)
        {
          fseeko64 (czmilh[hnd].cwf_fp, 0, SEEK_SET);


          /*  Read the header.  */

          czmil_read_cwf_header (hnd);
        }
    }


  /*  Try to open the CDX file.  The name was created when we first tried to open it.  */

  if ((czmilh[hnd].cdx_fp = fopen64 (czmilh[hnd].cdx_path, "wb+")) == NULL)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_CREATE_CDX_ERROR);
    }


  /*  Set the record field sizes.  */

  czmilh[hnd].cdx_cxy_address_bits = CDX_CXY_ADDRESS_BITS;
  czmilh[hnd].cdx_cwf_address_bits = CDX_CWF_ADDRESS_BITS;
  czmilh[hnd].cdx_cxy_buffer_size_bits = CDX_CXY_BUFFER_SIZE_BITS;
  czmilh[hnd].cdx_cwf_buffer_size_bits = CDX_CWF_BUFFER_SIZE_BITS;
  czmilh[hnd].cdx_record_size_bytes= (czmilh[hnd].cdx_cxy_address_bits + czmilh[hnd].cdx_cwf_address_bits +
                                      czmilh[hnd].cdx_cxy_buffer_size_bits + czmilh[hnd].cdx_cwf_buffer_size_bits) / 8;


  /*  Save the unqualified filenames in the headers.  */

  strcpy (czmilh[hnd].cxy_header.cdx_filename, czmil_gen_basename (czmilh[hnd].cdx_path));
  strcpy (czmilh[hnd].cwf_header.cdx_filename, czmilh[hnd].cxy_header.cdx_filename);
  strcpy (czmilh[hnd].cdx_header.cdx_filename, czmilh[hnd].cxy_header.cdx_filename);
  strcpy (czmilh[hnd].cdx_header.cwf_filename, czmilh[hnd].cxy_header.cwf_filename);
  strcpy (czmilh[hnd].cdx_header.cxy_filename, czmilh[hnd].cxy_header.cxy_filename);


  /*  These are identical fields from the CXY header.  */

  strcpy (czmilh[hnd].cdx_header.creation_software, czmilh[hnd].cxy_header.creation_software);
  czmilh[hnd].cdx_header.number_of_records = czmilh[hnd].cxy_header.number_of_records;
  czmilh[hnd].cdx_header.header_size = CZMIL_CDX_HEADER_SIZE;
  strcpy (czmilh[hnd].cdx_header.project, czmilh[hnd].cxy_header.project);
  strcpy (czmilh[hnd].cdx_header.mission, czmilh[hnd].cxy_header.mission);
  strcpy (czmilh[hnd].cdx_header.dataset, czmilh[hnd].cxy_header.dataset);
  strcpy (czmilh[hnd].cdx_header.flight_id, czmilh[hnd].cxy_header.flight_id);
  czmilh[hnd].cdx_header.flight_start_timestamp = czmilh[hnd].cxy_header.flight_start_timestamp;
  czmilh[hnd].cdx_header.flight_end_timestamp = czmilh[hnd].cxy_header.flight_end_timestamp;


  /*  Write the CDX header.  */

  if (czmil_write_cdx_header (hnd) < 0) return (czmil_error.czmil);


  /*  Loop through the CXY file and the CWF file (if available) and regenerate the CDX file.  */

  for (i = 0 ; i < czmilh[hnd].cxy_header.number_of_records ; i++)
    {
      czmilh[hnd].cdx_record.cxy_address = ftello64 (czmilh[hnd].cxy_fp);
      fread (&buffer_size, czmilh[hnd].cxy_buffer_size_bytes, 1, czmilh[hnd].cxy_fp);
      fseeko64 (czmilh[hnd].cxy_fp, czmilh[hnd].cdx_record.cxy_address + (NV_INT64) buffer_size, SEEK_SET);

      czmilh[hnd].cdx_record.cxy_buffer_size = buffer_size;

      czmilh[hnd].cdx_record.cwf_address = czmilh[hnd].cdx_record.cwf_buffer_size = 0;
      if (czmilh[hnd].cwf_fp != NULL)
        {
          czmilh[hnd].cdx_record.cwf_address = ftello64 (czmilh[hnd].cwf_fp);
          fread (&buffer_size, czmilh[hnd].cwf_buffer_size_bytes, 1, czmilh[hnd].cwf_fp);
          fseeko64 (czmilh[hnd].cwf_fp, czmilh[hnd].cdx_record.cwf_address + (NV_INT64) buffer_size, SEEK_SET);
          czmilh[hnd].cdx_record.cwf_buffer_size = buffer_size;
        }

      czmil_append_cdx_record (hnd, &czmilh[hnd].cdx_record);
    }


  /*  Now update the header since we have to get the correct record count in there.  */

  czmil_write_cdx_header (hnd);


  /*  Seek to the end of the header.  */

  fseeko64 (czmilh[hnd].cxy_fp, czmilh[hnd].cxy_header.header_size, SEEK_SET);
  if (czmilh[hnd].cwf_fp != NULL) fseeko64 (czmilh[hnd].cwf_fp, czmilh[hnd].cwf_header.header_size, SEEK_SET);
  fseeko64 (czmilh[hnd].cdx_fp, czmilh[hnd].cdx_header.header_size, SEEK_SET);


  czmilh[hnd].cxy_pos = czmilh[hnd].cxy_header.header_size;
  czmilh[hnd].cxy_write = NVFalse;
  if (czmilh[hnd].cwf_fp != NULL) 
    {
      czmilh[hnd].cwf_pos = czmilh[hnd].cwf_header.header_size;
      czmilh[hnd].cwf_write = NVFalse;
    }
  czmilh[hnd].cdx_pos = czmilh[hnd].cdx_header.header_size;
  czmilh[hnd].cdx_write = NVFalse;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_create_file

 - Purpose:     Create CZMIL CXY file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - path           =    The CZMIL file path
                - cxy_header     =    CZMIL_CXY_Header structure to be written to the file
                - cwf            =    If NVTrue, create CWF file as well

 - Returns:     
                - The file handle (0 or positive)
                - CZMIL_TOO_MANY_OPEN_FILES
                - CZMIL_CREATE_ERROR
                - CZMIL_CREATE_CWF_ERROR
                - CZMIL_CXY_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CXY_HEADER_WRITE_ERROR
                - CZMIL_CWF_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CWF_HEADER_WRITE_ERROR
                - CZMIL_CDX_HEADER_WRITE_FSEEK_ERROR
                - CZMIL_CDX_HEADER_WRITE_ERROR

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_create_file (const NV_CHAR *path, CZMIL_CXY_Header cxy_header, NV_BOOL cwf)
{
  NV_INT32 i, j, hnd;


  /*  The first time through we want to initialize (zero) the CZMIL handle array.  */

  if (first)
    {
      for (i = 0 ; i < CZMIL_MAX_FILES ; i++) 
        {
          memset (&czmilh[i], 0, sizeof (INTERNAL_CZMIL_STRUCT));


          /*  We do this just on the off chance that someday NULL won't be 0.  */

          czmilh[i].cxy_fp = NULL;
          czmilh[i].cwf_fp = NULL;
          czmilh[i].cdx_fp = NULL;
        }


      /*  Set up the SIGINT handler.  */

      signal (SIGINT, sigint_handler);


      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = CZMIL_MAX_FILES;
  for (i = 0 ; i < CZMIL_MAX_FILES ; i++)
    {
      if (czmilh[i].cxy_fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == CZMIL_MAX_FILES) return (czmil_error.czmil = CZMIL_TOO_MANY_OPEN_FILES);


  /*  Save the file name for building the other file names and for error messages.  */

  strcpy (czmilh[hnd].cxy_path, path);


  /*  Open the file and write the header.  */

  if ((czmilh[hnd].cxy_fp = fopen64 (path, "wb+")) == NULL)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CREATE_CXY_ERROR);
    }


  czmilh[hnd].cxy_header = cxy_header;


  /*  Set the default packed record bit field sizes and scale factors.  */

  czmilh[hnd].cxy_buffer_size_bytes = CXY_BUFFER_SIZE_BYTES;
  czmilh[hnd].cxy_return_bits = CXY_RETURN_BITS;
  czmilh[hnd].cxy_time_bits = CXY_TIME_BITS;
  czmilh[hnd].cxy_angle_scale = CXY_ANGLE_SCALE;
  czmilh[hnd].cxy_scan_angle_bits = CXY_SCAN_ANGLE_BITS;
  czmilh[hnd].cxy_roll_pitch_bits = CXY_ROLL_PITCH_BITS;
  czmilh[hnd].cxy_heading_bits = CXY_HEADING_BITS;
  czmilh[hnd].cxy_dop_bits = CXY_DOP_BITS;
  czmilh[hnd].cxy_pos_scale = CXY_POS_SCALE;
  czmilh[hnd].cxy_lat_diff_bits = CXY_LAT_DIFF_BITS;
  czmilh[hnd].cxy_elev_bits = CXY_ELEV_BITS;
  czmilh[hnd].cxy_elev_scale = CXY_ELEV_SCALE;
  czmilh[hnd].cxy_uncert_bits = CXY_UNCERT_BITS;
  czmilh[hnd].cxy_uncert_scale = CXY_UNCERT_SCALE;
  czmilh[hnd].cxy_reflectance_bits = CXY_REFLECTANCE_BITS;
  czmilh[hnd].cxy_reflectance_scale = CXY_REFLECTANCE_SCALE;
  czmilh[hnd].cxy_status_bits = CXY_STATUS_BITS;
  czmilh[hnd].cxy_waveform_class_bits = CXY_WAVEFORM_CLASS_BITS;


  /*  Compute the rest of the field size information from the defaults.  */

  czmilh[hnd].cxy_lat_bits = (NV_INT32) (log10 ((NV_FLOAT64) czmilh[hnd].cxy_pos_scale * 180.0L) / LOG2) + 1;
  czmilh[hnd].cxy_lon_bits = (NV_INT32) (log10 ((NV_FLOAT64) czmilh[hnd].cxy_pos_scale * 360.0L) / LOG2) + 1;
  czmilh[hnd].cxy_lat_diff_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_lat_diff_bits)) - 1;
  czmilh[hnd].cxy_lat_diff_offset = czmilh[hnd].cxy_lat_diff_max / 2;
  czmilh[hnd].cxy_elev_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_elev_bits)) - 1;
  czmilh[hnd].cxy_elev_offset = czmilh[hnd].cxy_elev_max / 2;
  czmilh[hnd].cxy_roll_pitch_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_roll_pitch_bits)) - 1;
  czmilh[hnd].cxy_roll_pitch_offset = czmilh[hnd].cxy_roll_pitch_max / 2;
  czmilh[hnd].cxy_reflectance_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_reflectance_bits)) - 1;
  czmilh[hnd].cxy_reflectance_offset = czmilh[hnd].cxy_reflectance_max / 2;
  czmilh[hnd].cxy_waveform_class_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_waveform_class_bits)) - 1;
  czmilh[hnd].cxy_waveform_class_offset = czmilh[hnd].cxy_waveform_class_max / 2;
  czmilh[hnd].cxy_uncert_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_uncert_bits)) - 1;
  czmilh[hnd].cxy_scan_angle_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_scan_angle_bits)) - 1;
  czmilh[hnd].cxy_scan_angle_offset = czmilh[hnd].cxy_scan_angle_max / 2;


  /*  Create the array of longitude difference bits and values.  The array subscript used will be based on the
      latitude degrees.  This allows us to increase the number of bits used to store longitude differences from
      the platform longitude as we move towards the poles.  We start with one more bit than cxy_lat_diff_bits at
      0-60 degrees latitude and add bits as 1 over the cosine of the latitude increases by a power of two.  The
      size of a degree of longitude decreases approximately as the cosine of the latitude so there is a halving of
      longitude distance at 60.0, 75.52, 82.82, and 86.42 degrees.  At approximately these intervals we need to add 
      a bit in order to double the stored values.  This will maintain better than 1 millimeter resolution up to 86
      degrees (N or S).  After 86 degrees we'll be storing the full resolution longitude.  Even though there will
      only be 5 discrete values in the array we're using an array of 180 values to allow us to index into the array
      each time we pack or unpack a record instead of using multiple if statements to determine the number of bits
      used.  This way we only need one if statement for the greater than 86 cases.  */

  for (i = 0, j = 89 ; i < 90 ; i++, j--)
    {
      if (i >= 86)
        {
          czmilh[hnd].cxy_lon_diff_bits[i + 90] = FULL_LON_FLAG;
          czmilh[hnd].cxy_lon_diff_max[i + 90] = 0;
          czmilh[hnd].cxy_lon_diff_offset[i + 90] = 0;
          if (j >= 0)
            {
              czmilh[hnd].cxy_lon_diff_bits[j] = FULL_LON_FLAG;
              czmilh[hnd].cxy_lon_diff_max[j] = 0;
              czmilh[hnd].cxy_lon_diff_offset[j] = 0;
            }
        }
      else
        {
          if (i > 81)
            {
              czmilh[hnd].cxy_lon_diff_bits[i + 90] = czmilh[hnd].cxy_lon_diff_bits[j] = czmilh[hnd].cxy_lat_diff_bits + 4;
            }
          else if (i > 74)
            {
              czmilh[hnd].cxy_lon_diff_bits[i + 90] = czmilh[hnd].cxy_lon_diff_bits[j] = czmilh[hnd].cxy_lat_diff_bits + 3;
            }
          else if (i > 60)
            {
              czmilh[hnd].cxy_lon_diff_bits[i + 90] = czmilh[hnd].cxy_lon_diff_bits[j] = czmilh[hnd].cxy_lat_diff_bits + 2;
            }
          else
            {
              czmilh[hnd].cxy_lon_diff_bits[i + 90] = czmilh[hnd].cxy_lon_diff_bits[j] = czmilh[hnd].cxy_lat_diff_bits + 1;
            }

          czmilh[hnd].cxy_lon_diff_max[i + 90] = czmilh[hnd].cxy_lon_diff_max[j] =
            (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cxy_lon_diff_bits[i])) - 1;
          czmilh[hnd].cxy_lon_diff_offset[i + 90] = czmilh[hnd].cxy_lon_diff_offset[j] =
            czmilh[hnd].cxy_lon_diff_max[i] / 2;
        }
    }


  /*  Hardwired for now.  */

  czmilh[hnd].cxy_header.record_size = 396;
  czmilh[hnd].cxy_header.null_z_value = -998.0;
  czmilh[hnd].cxy_header.number_of_records = NV_U_INT32_MAX;


  /*  Set the header sizes and the max packets from the defaults.  */

  czmilh[hnd].cxy_header.header_size = CZMIL_CXY_HEADER_SIZE;
  czmilh[hnd].cwf_header.header_size = CZMIL_CWF_HEADER_SIZE;
  czmilh[hnd].cdx_header.header_size = CZMIL_CDX_HEADER_SIZE;
  czmilh[hnd].cwf_czmil_max_packets = CZMIL_MAX_PACKETS;

  strcpy (czmilh[hnd].cxy_header.cxy_filename, czmil_gen_basename (czmilh[hnd].cxy_path));


  /*  Generate the CDX filename and try to create it.  */

  strcpy (czmilh[hnd].cdx_path, czmilh[hnd].cxy_path);
  sprintf (&czmilh[hnd].cdx_path[strlen (czmilh[hnd].cdx_path) - 4], ".cdx");

  if ((czmilh[hnd].cdx_fp = fopen64 (czmilh[hnd].cdx_path, "wb+")) == NULL)
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_CREATE_CDX_ERROR);
    }


  /*  Put identical fields from the CXY header into the CDX header.  */

  strcpy (czmilh[hnd].cdx_header.creation_software, czmilh[hnd].cxy_header.creation_software);
  czmilh[hnd].cdx_header.number_of_records = czmilh[hnd].cxy_header.number_of_records;
  strcpy (czmilh[hnd].cdx_header.project, czmilh[hnd].cxy_header.project);
  strcpy (czmilh[hnd].cdx_header.mission, czmilh[hnd].cxy_header.mission);
  strcpy (czmilh[hnd].cdx_header.dataset, czmilh[hnd].cxy_header.dataset);
  strcpy (czmilh[hnd].cdx_header.flight_id, czmilh[hnd].cxy_header.flight_id);
  czmilh[hnd].cdx_header.flight_start_timestamp = czmilh[hnd].cxy_header.flight_start_timestamp;
  czmilh[hnd].cdx_header.flight_end_timestamp = czmilh[hnd].cxy_header.flight_end_timestamp;


  strcpy (czmilh[hnd].cwf_path, "");


  /*  Create the associated waveform file if requested.  */

  if (hnd >= 0 && cwf)
    {
      /*  Hardwire the CWF compression bit sizes.  */

      czmilh[hnd].cwf_buffer_size_bytes = CWF_BUFFER_SIZE_BYTES;
      czmilh[hnd].cwf_type_bits = CWF_TYPE_BITS;
      czmilh[hnd].cwf_type_0_bytes = (czmilh[hnd].cwf_type_bits + 64 * 10) / 8;
      if ((czmilh[hnd].cwf_type_bits + 64 * 10) % 8) czmilh[hnd].cwf_type_0_bytes++;
      czmilh[hnd].cwf_type_1_start_bits = CWF_TYPE_1_START_BITS;
      czmilh[hnd].cwf_type_1_offset_bits = czmilh[hnd].cwf_type_1_start_bits + 1;
      czmilh[hnd].cwf_type_1_offset = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_type_1_start_bits)) - 1;
      czmilh[hnd].cwf_type_2_start_bits = CWF_TYPE_2_START_BITS;
      czmilh[hnd].cwf_type_2_offset_bits = czmilh[hnd].cwf_type_2_start_bits + 1;
      czmilh[hnd].cwf_type_2_offset = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_type_2_start_bits)) - 1;
      czmilh[hnd].cwf_type_3_offset_bits = czmilh[hnd].cwf_type_1_start_bits + 1;
      czmilh[hnd].cwf_delta_bits = CWF_DELTA_BITS;
      czmilh[hnd].cwf_time_bits = CWF_TIME_BITS;
      czmilh[hnd].cwf_angle_scale = CWF_ANGLE_SCALE;
      czmilh[hnd].cwf_scan_angle_bits = CWF_SCAN_ANGLE_BITS;
      czmilh[hnd].cwf_scan_angle_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_scan_angle_bits)) - 1;
      czmilh[hnd].cwf_scan_angle_offset = czmilh[hnd].cwf_scan_angle_max / 2;
      czmilh[hnd].cwf_type_1_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_1_start_bits +
        czmilh[hnd].cwf_type_1_offset_bits + czmilh[hnd].cwf_delta_bits;
      czmilh[hnd].cwf_type_2_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_1_start_bits +
        czmilh[hnd].cwf_type_1_offset_bits + czmilh[hnd].cwf_type_2_start_bits + czmilh[hnd].cwf_type_2_offset_bits +
        czmilh[hnd].cwf_delta_bits;
      czmilh[hnd].cwf_type_3_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_3_offset_bits +
        czmilh[hnd].cwf_delta_bits;


      /*  Generate the CWF filename and try to create it.  */

      strcpy (czmilh[hnd].cwf_path, czmilh[hnd].cxy_path);
      sprintf (&czmilh[hnd].cwf_path[strlen (czmilh[hnd].cwf_path) - 4], ".cwf");

      if ((czmilh[hnd].cwf_fp = fopen64 (czmilh[hnd].cwf_path, "wb+")) == NULL)
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cwf_path);
          return (czmil_error.czmil = CZMIL_CREATE_CWF_ERROR);
        }


      /*  Put identical fields from the CXY header into the CWF header.  */

      strcpy (czmilh[hnd].cwf_header.creation_software, czmilh[hnd].cxy_header.creation_software);
      strcpy (czmilh[hnd].cwf_header.security_classification, czmilh[hnd].cxy_header.security_classification);
      strcpy (czmilh[hnd].cwf_header.distribution, czmilh[hnd].cxy_header.distribution);
      strcpy (czmilh[hnd].cwf_header.declassification, czmilh[hnd].cxy_header.declassification);
      strcpy (czmilh[hnd].cwf_header.class_just, czmilh[hnd].cxy_header.class_just);
      strcpy (czmilh[hnd].cwf_header.downgrade, czmilh[hnd].cxy_header.downgrade);
      czmilh[hnd].cwf_header.number_of_records = czmilh[hnd].cxy_header.number_of_records;
      strcpy (czmilh[hnd].cwf_header.project, czmilh[hnd].cxy_header.project);
      strcpy (czmilh[hnd].cwf_header.mission, czmilh[hnd].cxy_header.mission);
      strcpy (czmilh[hnd].cwf_header.dataset, czmilh[hnd].cxy_header.dataset);
      strcpy (czmilh[hnd].cwf_header.flight_id, czmilh[hnd].cxy_header.flight_id);
      czmilh[hnd].cwf_header.flight_start_timestamp = czmilh[hnd].cxy_header.flight_start_timestamp;
      czmilh[hnd].cwf_header.flight_end_timestamp = czmilh[hnd].cxy_header.flight_end_timestamp;
    }


  /*  Save the unqualified filenames in the headers of all three files.  */

  strcpy (czmilh[hnd].cxy_header.cwf_filename, czmil_gen_basename (czmilh[hnd].cwf_path));
  strcpy (czmilh[hnd].cxy_header.cdx_filename, czmil_gen_basename (czmilh[hnd].cdx_path));
  strcpy (czmilh[hnd].cdx_header.cdx_filename, czmilh[hnd].cxy_header.cdx_filename);
  strcpy (czmilh[hnd].cdx_header.cwf_filename, czmilh[hnd].cxy_header.cwf_filename);
  strcpy (czmilh[hnd].cdx_header.cxy_filename, czmilh[hnd].cxy_header.cxy_filename);
  strcpy (czmilh[hnd].cwf_header.cdx_filename, czmilh[hnd].cxy_header.cdx_filename);
  strcpy (czmilh[hnd].cwf_header.cwf_filename, czmilh[hnd].cxy_header.cwf_filename);
  strcpy (czmilh[hnd].cwf_header.cxy_filename, czmilh[hnd].cxy_header.cxy_filename);


  /*  Write the CWF and CDX headers.  */

  if (cwf)
    {
      if (czmil_write_cwf_header (hnd) < 0) return (czmil_error.czmil);
    }
  if (czmil_write_cdx_header (hnd) < 0) return (czmil_error.czmil);


  /*  Set the CXY observed min and max values to some ridiculous number so they'll get replaced immediately.  */

  czmilh[hnd].cxy_header.min_lon = czmilh[hnd].cxy_header.min_lat = 999.0;
  czmilh[hnd].cxy_header.max_lon = czmilh[hnd].cxy_header.max_lat = -999.0;


  /*  Write the CXY header.  */

  if (czmil_write_cxy_header (hnd) < 0) return (czmil_error.czmil);


  czmilh[hnd].cxy_header.number_of_records = czmilh[hnd].cwf_header.number_of_records =
    czmilh[hnd].cdx_header.number_of_records = 0;

  czmilh[hnd].cxy_at_end = czmilh[hnd].cwf_at_end = czmilh[hnd].cdx_at_end = NVTrue;
  czmilh[hnd].cxy_modified = NVTrue;
  czmilh[hnd].cxy_created = czmilh[hnd].cwf_created = czmilh[hnd].cdx_created = NVTrue;
  czmilh[hnd].cxy_write = czmilh[hnd].cwf_write = czmilh[hnd].cdx_write = NVTrue;

  czmil_error.system = 0;
  return (hnd);
}


/********************************************************************************************/
/*!

 - Function:    czmil_open_file

 - Purpose:     Open a CZMIL file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - path           =    The CZMIL file path
                - cxy_header     =    CZMIL_CXY_HEADER structure to be populated
                - mode           =    CZMIL_UPDATE or CZMIL_READ_ONLY
                - cwf            =    If NVTrue, open CWF file as well

 - Returns:
                - The file handle (0 or positive)
                - CZMIL_TOO_MANY_OPEN_FILES
                - CZMIL_OPEN_UPDATE_ERROR
                - CZMIL_OPEN_READONLY_ERROR
                - CZMIL_NOT_CZMIL_FILE_ERROR

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_open_file (const NV_CHAR *path, CZMIL_CXY_Header *cxy_header, NV_INT32 mode, NV_BOOL cwf)
{
  NV_INT32 i, hnd;


  /*  The first time through we want to initialize (zero) the czmil handle array.  */

  if (first)
    {
      for (i = 0 ; i < CZMIL_MAX_FILES ; i++) 
        {
          memset (&czmilh[i], 0, sizeof (INTERNAL_CZMIL_STRUCT));


          /*  We do this just on the off chance that someday NULL won't be 0.  */

          czmilh[i].cxy_fp = NULL;
          czmilh[i].cwf_fp = NULL;
          czmilh[i].cdx_fp = NULL;
        }


      /*  Set up the SIGINT handler.  */

      signal (SIGINT, sigint_handler);


      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = CZMIL_MAX_FILES;
  for (i = 0 ; i < CZMIL_MAX_FILES ; i++)
    {
      if (czmilh[i].cxy_fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == CZMIL_MAX_FILES) return (czmil_error.czmil = CZMIL_TOO_MANY_OPEN_FILES);


  /*  Internal structs are zeroed above and on close of file so we don't have to do it here.  */


  /*  Save the file name for error messages.  */

  strcpy (czmilh[hnd].cxy_path, path);


  /*  Open the file and read the header.  */

  switch (mode)
    {
    case CZMIL_UPDATE:
      if ((czmilh[hnd].cxy_fp = fopen64 (path, "rb+")) == NULL)
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cxy_path);
          return (czmil_error.czmil = CZMIL_OPEN_CXY_UPDATE_ERROR);
        }


      /*  Open the associated CWF file.  */

      if (cwf)
        {
          strcpy (czmilh[hnd].cwf_path, czmilh[hnd].cxy_path);
          sprintf (&czmilh[hnd].cwf_path[strlen (czmilh[hnd].cwf_path) - 4], ".cwf");

          if ((czmilh[hnd].cwf_fp = fopen64 (czmilh[hnd].cwf_path, "rb+")) == NULL)
            {
              czmil_error.system = errno;
              strcpy (czmil_error.file, czmilh[hnd].cwf_path);
              return (czmil_error.czmil = CZMIL_OPEN_CWF_UPDATE_ERROR);
            }


          /*  Read the header.  Not doing this for now.  */

          /*czmil_read_cwf_header (hnd);*/
        }
      break;

    case CZMIL_READONLY:
      if ((czmilh[hnd].cxy_fp = fopen64 (path, "rb")) == NULL)
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cxy_path);
          return (czmil_error.czmil = CZMIL_OPEN_CXY_READONLY_ERROR);
        }


      /*  Open the associated CWF file.  */

      if (cwf)
        {
          strcpy (czmilh[hnd].cwf_path, czmilh[hnd].cxy_path);
          sprintf (&czmilh[hnd].cwf_path[strlen (czmilh[hnd].cwf_path) - 4], ".cwf");
          if ((czmilh[hnd].cwf_fp = fopen64 (czmilh[hnd].cwf_path, "rb")) == NULL)
            {
              czmil_error.system = errno;
              strcpy (czmil_error.file, czmilh[hnd].cwf_path);
              return (czmil_error.czmil = CZMIL_OPEN_CWF_READONLY_ERROR);
            }


          /*  Read the header.  Not doing this for now.  */

          /*czmil_read_cwf_header (hnd);*/
        }
      break;
    }


  /*  Hardwire the CWF compression bit sizes for now (until we actually have a header).  */

  czmilh[hnd].cwf_buffer_size_bytes = CWF_BUFFER_SIZE_BYTES;
  czmilh[hnd].cwf_type_bits = CWF_TYPE_BITS;
  czmilh[hnd].cwf_type_0_bytes = (czmilh[hnd].cwf_type_bits + 64 * 10) / 8;
  if ((czmilh[hnd].cwf_type_bits + 64 * 10) % 8) czmilh[hnd].cwf_type_0_bytes++;
  czmilh[hnd].cwf_type_1_start_bits = CWF_TYPE_1_START_BITS;
  czmilh[hnd].cwf_type_1_offset_bits = czmilh[hnd].cwf_type_1_start_bits + 1;
  czmilh[hnd].cwf_type_1_offset = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_type_1_start_bits)) - 1;
  czmilh[hnd].cwf_type_2_start_bits = CWF_TYPE_2_START_BITS;
  czmilh[hnd].cwf_type_2_offset_bits = czmilh[hnd].cwf_type_2_start_bits + 1;
  czmilh[hnd].cwf_type_2_offset = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_type_2_start_bits)) - 1;
  czmilh[hnd].cwf_type_3_offset_bits = czmilh[hnd].cwf_type_1_start_bits + 1;
  czmilh[hnd].cwf_delta_bits = CWF_DELTA_BITS;
  czmilh[hnd].cwf_time_bits = CWF_TIME_BITS;
  czmilh[hnd].cwf_angle_scale = CWF_ANGLE_SCALE;
  czmilh[hnd].cwf_scan_angle_bits = CWF_SCAN_ANGLE_BITS;
  czmilh[hnd].cwf_scan_angle_max = (NV_INT32) (pow (2.0L, (NV_FLOAT64) czmilh[hnd].cwf_scan_angle_bits)) - 1;
  czmilh[hnd].cwf_scan_angle_offset = czmilh[hnd].cwf_scan_angle_max / 2;
  czmilh[hnd].cwf_type_1_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_1_start_bits +
    czmilh[hnd].cwf_type_1_offset_bits + czmilh[hnd].cwf_delta_bits;
  czmilh[hnd].cwf_type_2_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_1_start_bits +
    czmilh[hnd].cwf_type_1_offset_bits + czmilh[hnd].cwf_type_2_start_bits + czmilh[hnd].cwf_type_2_offset_bits +
    czmilh[hnd].cwf_delta_bits;
  czmilh[hnd].cwf_type_3_header_bits = czmilh[hnd].cwf_type_bits + czmilh[hnd].cwf_type_3_offset_bits +
    czmilh[hnd].cwf_delta_bits;


  /*  Open the associated CDX file.  Not doing this for now.

  strcpy (czmilh[hnd].cdx_path, czmilh[hnd].cxy_path);
  sprintf (&czmilh[hnd].cdx_path[strlen (czmilh[hnd].cdx_path) - 4], ".cdx");

  if ((czmilh[hnd].cdx_fp = fopen64 (czmilh[hnd].cdx_path, "rb+")) == NULL)
    {
      //  If the file is missing, try to generate the associated CDX file using the CXY file  //

      if (errno == ENOENT)
        {
          if (czmil_regen_cdx_file (hnd))
            {
              czmil_error.system = errno;
              strcpy (czmil_error.file, czmilh[hnd].cdx_path);
              return (czmil_error.czmil = CZMIL_REGEN_CDX_ERROR);
            }
        }
      else
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cdx_path);
          return (czmil_error.czmil = CZMIL_OPEN_CDX_UPDATE_ERROR);
        }
    }


  //  Read the header.  //

  czmil_read_cdx_header (hnd);
  */


  /*  Read the header.  Not doing this for now.  */

  /*czmil_read_cxy_header (hnd);*/


  /*  Hardwired for now.  */

  /*  Let this be a lesson - never read or write structures.  sizeof (CZMIL_CXY_Data) != 396 because sizeof
      (CZMIL_Channel_Data) = 48 not 44.  Go figure!  */

  /*czmilh[hnd].cxy_header.record_size = sizeof (CZMIL_CXY_Data);*/
  czmilh[hnd].cxy_header.record_size = 396;
  czmilh[hnd].cxy_header.header_size = CZMIL_CXY_HEADER_SIZE;
  czmilh[hnd].cwf_header.header_size = CZMIL_CWF_HEADER_SIZE;
  czmilh[hnd].cdx_header.header_size = CZMIL_CDX_HEADER_SIZE;
  czmilh[hnd].cxy_header.null_z_value = -998.0;
  czmilh[hnd].cxy_header.number_of_records = NV_U_INT32_MAX;
  czmilh[hnd].cwf_czmil_max_packets = CZMIL_MAX_PACKETS;


  *cxy_header = czmilh[hnd].cxy_header;


  czmilh[hnd].cxy_at_end = czmilh[hnd].cwf_at_end = czmilh[hnd].cdx_at_end = NVFalse;
  czmilh[hnd].cxy_modified = NVFalse;
  czmilh[hnd].cxy_created = czmilh[hnd].cwf_created = czmilh[hnd].cdx_created = NVFalse;
  czmilh[hnd].cxy_write = czmilh[hnd].cwf_write = czmilh[hnd].cdx_write = NVFalse;


  czmil_error.system = 0;
  return (hnd);
}


/********************************************************************************************/
/*!

 - Function:    czmil_close_file

 - Purpose:     Close a CZMIL file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_CLOSE_WAVE_READ_ERROR
                - CZMIL_CLOSE_WAVE_WRITE_ERROR
                - CZMIL_CXY_HEADER_WRITE_ERROR
                - CZMIL_CWF_HEADER_WRITE_ERROR
                - CZMIL_CLOSE_ERROR

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_close_file (NV_INT32 hnd)
{
  time_t t;
  struct tm *cur_tm;
  CZMIL_CXY_Data cxy_record;


  t = time (&t);
  cur_tm = gmtime (&t);


  /*  Just in case someone tries to close a file more than once... */

  if (czmilh[hnd].cxy_fp == NULL) return (czmil_error.czmil = CZMIL_SUCCESS);


  /*  If we created a CXY file we have to generate the CDX index file, set the creation time, and set the start and end
      timestamps.  */

  if (czmilh[hnd].cxy_created)
    {
      if (czmil_regen_cdx_file (hnd))
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cdx_path);
          return (czmil_error.czmil = CZMIL_REGEN_CDX_ERROR);
        }


      /*  Save the creation time to the header.  */

      czmil_inv_cvtime (cur_tm->tm_year, cur_tm->tm_yday + 1, cur_tm->tm_hour,
                        cur_tm->tm_min, cur_tm->tm_sec,
                        &czmilh[hnd].cxy_header.creation_timestamp);


      /*  Get the start and end timestamps and put them in the header.  We don't have a unified timestamp for now.  */

      czmil_read_cxy_record (hnd, 0, &cxy_record);
      /*czmilh[hnd].cxy_header.flight_start_timestamp = cxy_record.timestamp;*/

      czmil_read_cxy_record (hnd, czmilh[hnd].cxy_header.number_of_records - 1, &cxy_record);
      /*czmilh[hnd].cxy_header.flight_end_timestamp = cxy_record.timestamp;*/
    }


  /*  If the CXY file was created or modified we need to write the header to the file.  */

  if (czmilh[hnd].cxy_created || czmilh[hnd].cxy_modified)
    {
      czmil_inv_cvtime (cur_tm->tm_year, cur_tm->tm_yday + 1, cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec,
                        &czmilh[hnd].cxy_header.modification_timestamp);

      if (czmil_write_cxy_header (hnd) < 0) return (czmil_error.czmil);
    }


  /*  If the CWF file was created we need to save the time and update the header.  If the header was modified we just
      need to update the header.  */

  if (czmilh[hnd].cwf_created || czmilh[hnd].cwf_modified)
    {
      czmilh[hnd].cwf_header.number_of_records = czmilh[hnd].cxy_header.number_of_records;
      czmilh[hnd].cwf_header.flight_start_timestamp = czmilh[hnd].cxy_header.flight_start_timestamp;
      czmilh[hnd].cwf_header.flight_end_timestamp = czmilh[hnd].cxy_header.flight_end_timestamp;

      if (czmilh[hnd].cwf_created) czmil_inv_cvtime (cur_tm->tm_year, cur_tm->tm_yday + 1, cur_tm->tm_hour,
                                                     cur_tm->tm_min, cur_tm->tm_sec,
                                                     &czmilh[hnd].cwf_header.creation_timestamp);

      if (czmil_write_cwf_header (hnd) < 0) return (czmil_error.czmil);
    }


  /*  If the CDX file was created we need to save the time and update the header.  */

  if (czmilh[hnd].cdx_created)
    {
      czmilh[hnd].cdx_header.number_of_records = czmilh[hnd].cxy_header.number_of_records;
      czmilh[hnd].cdx_header.flight_start_timestamp = czmilh[hnd].cxy_header.flight_start_timestamp;
      czmilh[hnd].cdx_header.flight_end_timestamp = czmilh[hnd].cxy_header.flight_end_timestamp;

      czmil_inv_cvtime (cur_tm->tm_year, cur_tm->tm_yday + 1, cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec,
                        &czmilh[hnd].cdx_header.creation_timestamp);

      if (czmil_write_cdx_header (hnd) < 0) return (czmil_error.czmil);
    }


  if (fclose (czmilh[hnd].cxy_fp))
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CLOSE_ERROR);
    }

  if (czmilh[hnd].cwf_fp != NULL)
    {
      if (fclose (czmilh[hnd].cwf_fp))
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cwf_path);
          return (czmil_error.czmil = CZMIL_CLOSE_ERROR);
        }
    }

  if (czmilh[hnd].cdx_fp != NULL)
    {
      if (fclose (czmilh[hnd].cdx_fp))
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cdx_path);
          return (czmil_error.czmil = CZMIL_CLOSE_ERROR);
        }
    }


  /*  Clear the internal structure.  */

  memset (&czmilh[hnd], 0, sizeof (INTERNAL_CZMIL_STRUCT));


  /*  We do this just on the off chance that someday NULL won't be 0.  */

  czmilh[hnd].cxy_fp = NULL;
  czmilh[hnd].cwf_fp = NULL;
  czmilh[hnd].cdx_fp = NULL;


  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_read_cxy_record

 - Purpose:     Retrieve a CZMIL CXY record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - recnum         =    The record number of the CZMIL record to be retrieved
                - record         =    The returned CZMIL CXY record

 - Returns:     - CZMIL_SUCCESS
                - CZMIL_INVALID_RECORD_NUMBER
                - CZMIL_CXY_READ_FSEEK_ERROR
                - CZMIL_CXY_READ_ERROR

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_read_cxy_record (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CXY_Data *record)
{
  NV_INT64 pos;
  NV_INT32 i, j, bpos;
  NV_U_BYTE buffer[396];


  /*  Check for record out of bounds.  */

  if (recnum >= czmilh[hnd].cxy_header.number_of_records || recnum < 0)
    {
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_INVALID_RECORD_NUMBER);
    }


  pos = (NV_INT64) recnum * (NV_INT64) czmilh[hnd].cxy_header.record_size +
    (NV_INT64) czmilh[hnd].cxy_header.header_size;


  /*  We only want to do the fseek (which flushes the buffer) if our last operation was a write or if we aren't in the
      correct position.  */

  if (czmilh[hnd].cxy_write || pos != czmilh[hnd].cxy_pos)
    {
      if (fseeko64 (czmilh[hnd].cxy_fp, pos, SEEK_SET) < 0)
        {
          czmil_error.system = errno;
          czmil_error.recnum = recnum;
          strcpy (czmil_error.file, czmilh[hnd].cxy_path);
          return (czmil_error.czmil = CZMIL_CXY_READ_FSEEK_ERROR);
        }
    }

  if (!fread (buffer, czmilh[hnd].cxy_header.record_size, 1, czmilh[hnd].cxy_fp))
    {
      czmil_error.system = errno;
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_READ_ERROR);
    }


  bpos = 0;
  for (i = 0 ; i < 9 ; i++)
    {
      memcpy (&record->channel[i].timestamp, &buffer[bpos], 8); bpos += 8;
      memcpy (&record->channel[i].latitude, &buffer[bpos], 8); bpos += 8;
      memcpy (&record->channel[i].longitude, &buffer[bpos], 8); bpos += 8;
      for (j = 0 ; j < 4 ; j++) 
        {
          memcpy (&record->channel[i].return_elevation[j], &buffer[bpos], 4); bpos += 4;
        }
      memcpy (&record->channel[i].waveform_class, &buffer[bpos], 4); bpos += 4;
    }


  czmilh[hnd].cxy_pos += czmilh[hnd].cxy_header.record_size;
  czmilh[hnd].cxy_at_end = NVFalse;
  czmilh[hnd].cxy_write = NVFalse;


  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_unpack_full_lon

 - Purpose:     Function to bit unpack longitude as a full resolution value.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - buffer         =    The buffer from which we're unpacking the value
                - bpos           =    The position within the buffer at which to retrieve the value
                - ref_lon        =    The reference longitude (not used)
                - lat_band       =    The degrees of latitude of the full position (not used)
                - value          =    The unpacked value

 - Returns:
                - CZMIL_SUCCESS

 - Caveats:     This function is used to speed up unpacking of longitudes.  The longitudes may
                be stored as a full longitude or as a difference from a reference longitude
                (to save space).  If the longitude is stored as a difference from a reference
                longitude it may be stored in a varying number of bits depending on latitude.
                We're trying to avoid "if" statements when we unpack the longitude values.
                For more information look for lon_func below.

*********************************************************************************************/

static NV_INT32 czmil_unpack_full_lon (NV_INT32 hnd, NV_U_BYTE *buffer, NV_INT32 *bpos,
                                       NV_INT64 ref_lon __attribute__ ((unused)), 
                                       NV_INT32 lat_band __attribute__ ((unused)),
                                       NV_FLOAT64 *value)
{
  NV_INT64 lon;

  lon = czmil_double_bit_unpack (buffer, *bpos, czmilh[hnd].cxy_lon_bits);
  *bpos += czmilh[hnd].cxy_lon_bits;
  *value = (NV_FLOAT64) lon / czmilh[hnd].cxy_pos_scale - 180.0;

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_unpack_lon_diff

 - Purpose:     Function to bit unpack longitude as a difference from a reference longitude.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - buffer         =    The buffer from which we're unpacking the value
                - bpos           =    The position within the buffer at which to retrieve the value
                - ref_lon        =    The reference longitude
                - lat_band       =    The degrees of latitude of the full position (not used)
                - value          =    The unpacked value

 - Returns:
                - CZMIL_SUCCESS

 - Caveats:     This function is used to speed up unpacking of longitudes.  The longitudes may
                be stored as a full longitude or as a difference from a reference longitude
                (to save space).  If the longitude is stored as a difference from a reference
                longitude it may be stored in a varying number of bits depending on latitude.
                We're trying to avoid "if" statements when we unpack the longitude values.
                For more information look for lon_func below.

*********************************************************************************************/

static NV_INT32 czmil_unpack_lon_diff (NV_INT32 hnd, NV_U_BYTE *buffer, NV_INT32 *bpos, NV_INT64 ref_lon,
                                       NV_INT32 lat_band, NV_FLOAT64 *value)
{
  NV_INT32 i32value, lon_diff;
  NV_INT64 lon;

  i32value = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cxy_lon_diff_bits[lat_band]);
  *bpos += czmilh[hnd].cxy_lon_diff_bits[lat_band];
  lon_diff = i32value - czmilh[hnd].cxy_lon_diff_offset[lat_band];
  lon = ref_lon + (NV_INT64) lon_diff;
  *value = (NV_FLOAT64) lon / czmilh[hnd].cxy_pos_scale - 180.0;

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_read_cxy_record2

 - Purpose:     Retrieve a CZMIL CXY record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - recnum         =    The record number of the CZMIL record to be retrieved
                - record         =    The returned CZMIL CXY record

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_INVALID_RECORD_NUMBER
                - CZMIL_CXY_READ_FSEEK_ERROR
                - CZMIL_CXY_READ_ERROR

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_read_cxy_record2 (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CXY_Data2 *record)
{
  NV_INT64 ref_lat, ref_lon, lat;
  NV_INT32 i, j, bpos, lat_diff, size, i32value, lat_band, lon_func;


  /*  The actual buffer will never be sizeof (CZMIL_CXY_Data2) in size since we are unpacking it but this way
      we don't have to worry about blowing this up.  */

  NV_U_BYTE buffer[sizeof (CZMIL_CXY_Data2)];


  /*  A nifty little array of functions so we don't have to use any "if" statements to determine whether to unpack
      a full longitude (above latitude 86) or a difference from the reference longitude.  */

  static NV_INT32 (*czmil_unpack_lon[2]) (NV_INT32, NV_U_BYTE *, NV_INT32 *, NV_INT64, NV_INT32, NV_FLOAT64 *) =
    {czmil_unpack_lon_diff, czmil_unpack_full_lon};


  /*  Check for record out of bounds.  */

  if (recnum >= czmilh[hnd].cxy_header.number_of_records || recnum < 0)
    {
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_INVALID_RECORD_NUMBER);
    }


  /*  Get the CXY record byte address and buffer size from the CDX index file.  */

  czmil_read_cdx_record (hnd, recnum);


  /*  We only want to do the fseek (which flushes the buffer) if our last operation was a write or if we aren't in the
      correct position.  */

  if (czmilh[hnd].cxy_write || czmilh[hnd].cdx_record.cxy_address != czmilh[hnd].cxy_pos)
    {
      if (fseeko64 (czmilh[hnd].cxy_fp, czmilh[hnd].cdx_record.cxy_address, SEEK_SET) < 0)
        {
          czmil_error.system = errno;
          czmil_error.recnum = recnum;
          strcpy (czmil_error.file, czmilh[hnd].cxy_path);
          return (czmil_error.czmil = CZMIL_CXY_READ_FSEEK_ERROR);
        }
    }


  czmilh[hnd].cxy_at_end = NVFalse;

  if (!fread (buffer, czmilh[hnd].cdx_record.cxy_buffer_size, 1, czmilh[hnd].cxy_fp))
    {
      czmil_error.system = errno;
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_READ_ERROR);
    }


  /*  Make sure the buffer size read from the CXY file matches the buffer size read from the CDX file.  */

  bpos = 0;
  size = czmil_bit_unpack (buffer, 0, czmilh[hnd].cxy_buffer_size_bytes * 8);
  bpos += czmilh[hnd].cxy_buffer_size_bytes * 8;

  if (size != czmilh[hnd].cdx_record.cxy_buffer_size)
    {
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_CDX_BUFFER_SIZE_ERROR);
    }


  /*  Return presence flags.  */

  for (i = 0 ; i < 9 ; i++)
    {
      record->returns[i] = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_return_bits);
      bpos += czmilh[hnd].cxy_return_bits;
    }


  /*  Timestamp.  */
  
  record->timestamp = czmil_double_bit_unpack (buffer, bpos, czmilh[hnd].cxy_time_bits);
  bpos += czmilh[hnd].cxy_time_bits;


  /*  Scan angle.  */

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_scan_angle_bits);
  bpos += czmilh[hnd].cxy_scan_angle_bits;
  record->scan_angle = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_scan_angle_offset) / czmilh[hnd].cxy_angle_scale;


  /*  Platform latitude, longitude, and elevation.  */

  ref_lat = czmil_double_bit_unpack (buffer, bpos, czmilh[hnd].cxy_lat_bits);
  bpos += czmilh[hnd].cxy_lat_bits;
  record->platform_latitude = (NV_FLOAT64) ref_lat / czmilh[hnd].cxy_pos_scale - 90.0;

  ref_lon = czmil_double_bit_unpack (buffer, bpos, czmilh[hnd].cxy_lon_bits);
  bpos += czmilh[hnd].cxy_lon_bits;
  record->platform_longitude = (NV_FLOAT64) ref_lon / czmilh[hnd].cxy_pos_scale - 180.0;

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_elev_bits);
  bpos += czmilh[hnd].cxy_elev_bits;
  record->platform_elevation = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_elev_offset) / czmilh[hnd].cxy_elev_scale;


  /*  Compute the latitude band to index into the lon_diff arrays.  Also, determine which function to use to unpack the
      longitude.  If the number of bits used to store the longitude difference is equal to FULL_LON_FLAG then this value
      will be 1, in which case we'll store the full longitude.  The number of bits used to store longitude differences
      is stored in an array (180 values corresponding to latitude degrees computed on file open or creation).  This value
      will always be MUCH smaller than FULL_LON_FLAG except when the latitude is north of 86N or south of 86S.  In those
      cases the number of bits stored in the array is set to FULL_LON_FLAG.  The reason we're going through all of these
      gyrations is that "if" statements are notoriously slow.  Since we might have up to 38 longitudes stored per shot
      this could cause us to perform 38 "if" statements for each shot.  At 10,000 shots per second this could become
      annoying.  The way we implement the test here, using an array of functions, allows us to perform one computation
      (compute lon_func) instead of doing ANY "if" statements.  Try not to get confused by this, it's just PFM  ;-)  */

  lat_band = (NV_INT32) (record->platform_latitude + 90.0);
  lon_func = czmilh[hnd].cxy_lon_diff_bits[lat_band] / FULL_LON_FLAG;


  /*  Platform roll, pitch, and heading.  */

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_roll_pitch_bits);
  bpos += czmilh[hnd].cxy_roll_pitch_bits;
  record->platform_roll = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_roll_pitch_offset) / czmilh[hnd].cxy_angle_scale;

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_roll_pitch_bits);
  bpos += czmilh[hnd].cxy_roll_pitch_bits;
  record->platform_pitch = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_roll_pitch_offset) / czmilh[hnd].cxy_angle_scale;

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_heading_bits);
  bpos += czmilh[hnd].cxy_heading_bits;
  record->platform_heading = (NV_FLOAT32) i32value / czmilh[hnd].cxy_angle_scale;


  /*  Position and time dilution of precision.  */

  record->pdop = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_dop_bits);
  bpos += czmilh[hnd].cxy_dop_bits;
  record->tdop = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_dop_bits);
  bpos += czmilh[hnd].cxy_dop_bits;


  /*  Water surface latitude, longitude, and elevation.  */

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_lat_diff_bits);
  bpos += czmilh[hnd].cxy_lat_diff_bits;
  lat_diff = i32value - czmilh[hnd].cxy_lat_diff_offset;
  lat = ref_lat + (NV_INT64) lat_diff;
  record->water_surface_latitude =  (NV_FLOAT64) lat / czmilh[hnd].cxy_pos_scale - 90.0;


  /*  Unpack the longitude using the proper function based on the latitude band (north of 86N or south of 86S is full longitude).  */

  (*czmil_unpack_lon[lon_func]) (hnd, buffer, &bpos, ref_lon, lat_band, &record->water_surface_longitude);


  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_elev_bits);
  bpos += czmilh[hnd].cxy_elev_bits;
  record->water_surface_elevation = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_elev_offset) / czmilh[hnd].cxy_elev_scale;


  /*  Water level elevation.  */

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_elev_bits);
  bpos += czmilh[hnd].cxy_elev_bits;
  record->water_level = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_elev_offset) / czmilh[hnd].cxy_elev_scale;


  /*  Local vertical datum offset (elevation).  */

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_elev_bits);
  bpos += czmilh[hnd].cxy_elev_bits;
  record->local_vertical_datum_offset = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_elev_offset) /
    czmilh[hnd].cxy_elev_scale;


  /*  Loop through all nine channels.  */

  for (i = 0 ; i < 9 ; i++)
    {

      /*  Four possible returns per channel.  */

      for (j = 0 ; j < 4 ; j++)
        {
          /*  If the return is present, process it.  */

          if (j <= record->returns[i])
            {
              /*  Return latitude, longitude, and elevation.  */

              i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_lat_diff_bits);
              bpos += czmilh[hnd].cxy_lat_diff_bits;
              lat_diff = i32value - czmilh[hnd].cxy_lat_diff_offset;
              lat = ref_lat + (NV_INT64) lat_diff;
              record->channel[i].data[j].latitude =  (NV_FLOAT64) lat / czmilh[hnd].cxy_pos_scale - 90.0;


              /*  Unpack the longitude using the proper function based on the latitude band (north of 86N or south of 86S
                  is full longitude).  */

              (*czmil_unpack_lon[lon_func]) (hnd, buffer, &bpos, ref_lon, lat_band,
                                             &record->channel[i].data[j].longitude);


              i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_elev_bits);
              bpos += czmilh[hnd].cxy_elev_bits;
              record->channel[i].data[j].elevation = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_elev_offset) /
                czmilh[hnd].cxy_elev_scale;


              /*  Reflectance.  */

              i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_reflectance_bits);
              bpos += czmilh[hnd].cxy_reflectance_bits;
              record->channel[i].data[j].reflectance = (NV_FLOAT32) (i32value - czmilh[hnd].cxy_reflectance_offset) /
                czmilh[hnd].cxy_reflectance_scale;


              /*  Horizontal and vertical uncertainty.  */

              i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_uncert_bits);
              bpos += czmilh[hnd].cxy_uncert_bits;
              record->channel[i].data[j].horizontal_uncertainty =
                (NV_FLOAT32) (i32value - czmilh[hnd].cxy_reflectance_offset) / czmilh[hnd].cxy_reflectance_scale;

              i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cxy_uncert_bits);
              bpos += czmilh[hnd].cxy_uncert_bits;
              record->channel[i].data[j].vertical_uncertainty =
                (NV_FLOAT32) (i32value - czmilh[hnd].cxy_reflectance_offset) / czmilh[hnd].cxy_reflectance_scale;

              /*  Status ?????  */
            }
        }


      /*  Waveform class ?????  */
    }


  czmilh[hnd].cxy_pos += size;
  czmilh[hnd].cxy_modified = NVFalse;
  czmilh[hnd].cxy_write = NVFalse;


  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_pack_full_lon

 - Purpose:     Function to bit pack longitude as a full resolution value.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - buffer         =    The buffer into which we're packing the value
                - bpos           =    The position within the buffer at which to place the value
                - ref_lon        =    The reference longitude (not used)
                - lat_band       =    The degrees of latitude of the full position (not used)
                - value          =    The value to be packed

 - Returns:
                - CZMIL_SUCCESS

 - Caveats:     This function is used to speed up packing of longitudes.  The longitudes may
                be stored as a full longitude or as a difference from a reference longitude
                (to save space).  If the longitude is stored as a difference from a reference
                longitude it may be stored in a varying number of bits depending on latitude.
                We're trying to avoid "if" statements when we pack the longitude values.  For
                more information look for lon_func below.

*********************************************************************************************/

static NV_INT32 czmil_pack_full_lon (NV_INT32 hnd, NV_U_BYTE *buffer, NV_INT32 *bpos, 
                                     NV_INT64 ref_lon __attribute__ ((unused)), 
                                     NV_INT32 lat_band __attribute__ ((unused)), 
                                     NV_FLOAT64 value)
{
  NV_INT64 lon;

  lon = NINT64 ((value + 180.0) * czmilh[hnd].cxy_pos_scale);

  czmil_double_bit_pack (buffer, *bpos, czmilh[hnd].cxy_lon_bits, lon);
  *bpos += czmilh[hnd].cxy_lon_bits;

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_pack_lon_diff

 - Purpose:     Function to bit pack longitude as a difference from a reference longitude.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - buffer         =    The buffer into which we're packing the value
                - bpos           =    The position within the buffer at which to place the value
                - ref_lon        =    The reference longitude
                - lat_band       =    The degrees of latitude of the full position
                - value          =    The value to be packed

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_CXY_LON_OUT_OF_RANGE_ERROR

 - Caveats:     This function is used to speed up packing of longitudes.  The longitudes may
                be stored as a full longitude or as a difference from a reference longitude
                (to save space).  If the longitude is stored as a difference from a reference
                longitude it may be stored in a varying number of bits depending on latitude.
                We're trying to avoid "if" statements when we pack the longitude values.  For
                more information look for lon_func below.

*********************************************************************************************/

static NV_INT32 czmil_pack_lon_diff (NV_INT32 hnd, NV_U_BYTE *buffer, NV_INT32 *bpos, NV_INT64 ref_lon,
                                     NV_INT32 lat_band, NV_FLOAT64 value)
{
  NV_INT64 lon;
  NV_INT32 lon_diff, i32value;

  lon = NINT64 ((value + 180.0) * czmilh[hnd].cxy_pos_scale);
  lon_diff = (NV_INT32) (ref_lon - lon);

  i32value = lon_diff + czmilh[hnd].cxy_lon_diff_offset[lat_band];
  if (i32value < 0 || i32value > czmilh[hnd].cxy_lon_diff_max[lat_band])
    return (czmil_error.czmil = CZMIL_CXY_LON_OUT_OF_RANGE_ERROR);

  czmil_bit_pack (buffer, *bpos, czmilh[hnd].cxy_lon_diff_bits[lat_band], i32value);
  *bpos += czmilh[hnd].cxy_lon_diff_bits[lat_band];

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_write_cxy_record

 - Purpose:     Append or update a CZMIL CXY record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - recnum         =    The record number of the CZMIL record to be written
                - record         =    The CZMIL CXY record

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_INVALID_RECORD_NUMBER
                - CZMIL_CXY_WRITE_FSEEK_ERROR
                - CZMIL_CXY_WRITE_ERROR

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_write_cxy_record (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CXY_Data2 *record)
{
  NV_INT64 ref_lat, ref_lon, lat, lon;
  NV_INT32 i, j, bpos, lat_diff, lon_diff, size, i32value, lat_band, lon_func;


  /*  The actual buffer will never be sizeof (CZMIL_CXY_Data2) in size since we are bit packing it but this way
      we don't have to worry about blowing this up.  */

  NV_U_BYTE buffer[sizeof (CZMIL_CXY_Data2)];


  /*  A nifty little array of functions so we don't have to use any "if" statements to determine whether to pack
      a full longitude (above latitude 86) or a difference from the reference longitude.  */

  static NV_INT32 (*czmil_pack_lon[2]) (NV_INT32, NV_U_BYTE *, NV_INT32 *, NV_INT64, NV_INT32, NV_FLOAT64) =
    {czmil_pack_lon_diff, czmil_pack_full_lon};


  /*  This section is for updating a record.  */

  if (recnum != CZMIL_APPEND_CXY_RECORD)
    {
      /*  Check for record out of bounds.  */

      if (recnum >= czmilh[hnd].cxy_header.number_of_records || recnum < 0)
        {
          czmil_error.recnum = recnum;
          strcpy (czmil_error.file, czmilh[hnd].cxy_path);
          return (czmil_error.czmil = CZMIL_INVALID_RECORD_NUMBER);
        }


      /*  Get the CXY record byte address from the CDX index file.  */

      czmil_read_cdx_record (hnd, recnum);


      /*  We only want to do the fseek (which flushes the buffer) if our last operation was a read or if we aren't in the
          correct position.  */

      if (!czmilh[hnd].cxy_write || czmilh[hnd].cdx_record.cxy_address != czmilh[hnd].cxy_pos)
        {
          if (fseeko64 (czmilh[hnd].cxy_fp, czmilh[hnd].cdx_record.cxy_address, SEEK_SET) < 0)
            {
              czmil_error.system = errno;
              czmil_error.recnum = recnum;
              strcpy (czmil_error.file, czmilh[hnd].cxy_path);
              return (czmil_error.czmil = CZMIL_CXY_WRITE_FSEEK_ERROR);
            }
        }

      czmilh[hnd].cxy_at_end = NVFalse;
    }


  /*  This is for appending a record to a newly created file.  */

  else
    {
      /*  Appending a record is only allowed if you are creating a new file.  */

      if (!czmilh[hnd].cxy_created)
        {
          czmil_error.system = 0;
          strcpy (czmil_error.file, czmilh[hnd].cxy_path);
          return (czmil_error.czmil = CZMIL_CXY_APPEND_ERROR);
        }
          
      if (!czmilh[hnd].cxy_at_end)
        {
          if (fseeko64 (czmilh[hnd].cxy_fp, 0, SEEK_END) < 0)
            {
              czmil_error.system = errno;
              czmil_error.recnum = recnum;
              strcpy (czmil_error.file, czmilh[hnd].cxy_path);
              return (czmil_error.czmil = CZMIL_CXY_WRITE_FSEEK_ERROR);
            }
        }

      czmilh[hnd].cxy_at_end = NVTrue;


      /*  Check for min and max since we're appending a record.  */

      for (i = 0 ; i < 9 ; i++)
        {
          for (j = 0 ; j < 4 ; j++)
            {
              czmilh[hnd].cxy_header.min_lon = MIN (czmilh[hnd].cxy_header.min_lon, record->channel[i].data[j].longitude);
              czmilh[hnd].cxy_header.max_lon = MAX (czmilh[hnd].cxy_header.max_lon, record->channel[i].data[j].longitude);
              czmilh[hnd].cxy_header.min_lat = MIN (czmilh[hnd].cxy_header.min_lat, record->channel[i].data[j].latitude);
              czmilh[hnd].cxy_header.max_lat = MAX (czmilh[hnd].cxy_header.max_lat, record->channel[i].data[j].latitude);
            }
        }


      /*  Increment the record count.  */

      czmilh[hnd].cxy_header.number_of_records++;
    }


  /*  Save the reference lat and lon.  */

  ref_lat = NINT64 ((record->platform_latitude + 90.0) * czmilh[hnd].cxy_pos_scale);
  ref_lon = NINT64 ((record->platform_longitude + 180.0) * czmilh[hnd].cxy_pos_scale);


  /*  Compute the latitude band to index into the lon_diff arrays.  Also, determine which function to use to pack the
      longitude.  If the number of bits used to store the longitude difference is equal to FULL_LON_FLAG then this value
      will be 1, in which case we'll store the full longitude.  The number of bits used to store longitude differences
      is stored in an array (180 values corresponding to latitude degrees computed on file open or creation).  This value
      will always be MUCH smaller than FULL_LON_FLAG except when the latitude is north of 86N or south of 86S.  In those
      cases the number of bits stored in the array is set to FULL_LON_FLAG.  The reason we're going through all of these
      gyrations is that "if" statements are notoriously slow.  Since we might have up to 38 longitudes stored per shot
      this could cause us to perform 38 "if" statements for each shot.  At 10,000 shots per second this could become
      annoying.  The way we implement the test here, using an array of functions, allows us to perform one computation
      (compute lon_func) instead of doing ANY "if" statements.  Try not to get confused by this, it's just PFM  ;-)  */

  lat_band = (NV_INT32) (record->platform_latitude + 90.0);
  lon_func = czmilh[hnd].cxy_lon_diff_bits[lat_band] / FULL_LON_FLAG;


  /*  Pack the record.  czmilh[hnd].cxy_buffer_size_bytes is the offset for the buffer size which will be stored first in
      the buffer.  */

  bpos = czmilh[hnd].cxy_buffer_size_bytes * 8;


  /*  Return presence flags.  */

  for (i = 0 ; i < 9 ; i++)
    {
      czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_return_bits, record->returns[i]);
      bpos += czmilh[hnd].cxy_return_bits;
    }


  /*  Timestamp.  */
  
  czmil_double_bit_pack (buffer, bpos, czmilh[hnd].cxy_time_bits, record->timestamp);
  bpos += czmilh[hnd].cxy_time_bits;


  /*  Scan angle.  */

  i32value = NINT (record->scan_angle * czmilh[hnd].cxy_angle_scale) + czmilh[hnd].cxy_scan_angle_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cxy_scan_angle_max)
    {
      sprintf (czmil_error.info, _("Platform scan angle %f out of range."), record->scan_angle);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_scan_angle_bits, i32value);
  bpos += czmilh[hnd].cxy_scan_angle_bits;


  /*  Platform latitude, longitude, and elevation.  */

  czmil_double_bit_pack (buffer, bpos, czmilh[hnd].cxy_lat_bits, ref_lat);
  bpos += czmilh[hnd].cxy_lat_bits;
  czmil_double_bit_pack (buffer, bpos, czmilh[hnd].cxy_lon_bits, ref_lon);
  bpos += czmilh[hnd].cxy_lon_bits;

  i32value = NINT (record->platform_elevation * czmilh[hnd].cxy_elev_scale) + czmilh[hnd].cxy_elev_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cxy_elev_max)
    {
      sprintf (czmil_error.info, _("Platform elevation %f out of range."), record->platform_elevation);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_elev_bits, i32value);
  bpos += czmilh[hnd].cxy_elev_bits;


  /*  Platform roll, pitch, and heading.  */

  i32value = NINT (record->platform_roll * czmilh[hnd].cxy_angle_scale) + czmilh[hnd].cxy_roll_pitch_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cxy_roll_pitch_max)
    {
      sprintf (czmil_error.info, _("Platform roll %f out of range."), record->platform_roll);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_roll_pitch_bits, i32value);
  bpos += czmilh[hnd].cxy_roll_pitch_bits;

  i32value = NINT (record->platform_pitch * czmilh[hnd].cxy_angle_scale) + czmilh[hnd].cxy_roll_pitch_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cxy_roll_pitch_max)
    {
      sprintf (czmil_error.info, _("Platform pitch %f out of range."), record->platform_pitch);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_roll_pitch_bits, i32value);
  bpos += czmilh[hnd].cxy_roll_pitch_bits;


  /*  Adjust the heading for negatives and greater than 360.0 values.  */

  if (record->platform_heading < 0.0) record->platform_heading += 360.0;
  if (record->platform_heading > 360.0) record->platform_heading -= 360.0;

  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_heading_bits, NINT (record->platform_heading *
                                                                    czmilh[hnd].cxy_angle_scale));
  bpos += czmilh[hnd].cxy_heading_bits;


  /*  Position and time dilution of precision.  */

  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_dop_bits, MAX (record->pdop, 31));
  bpos += czmilh[hnd].cxy_dop_bits;
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_dop_bits, MAX (record->tdop, 31));
  bpos += czmilh[hnd].cxy_dop_bits;


  /*  Water surface latitude, longitude, and elevation.  */

  lat = NINT64 ((record->water_surface_latitude + 90.0) * czmilh[hnd].cxy_pos_scale);
  lat_diff = (NV_INT32) (ref_lat - lat);

  i32value = lat_diff + czmilh[hnd].cxy_lat_diff_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cxy_lat_diff_max)
    {
      sprintf (czmil_error.info, _("Water surface latitude %f difference from platform latitude %f out of range."),
               record->water_surface_latitude, record->platform_latitude);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_lat_diff_bits, i32value);
  bpos += czmilh[hnd].cxy_lat_diff_bits;


  /*  Pack the longitude using the proper function based on the latitude band (north of 86N or south of 86S is full longitude).  */

  if ((*czmil_pack_lon[lon_func]) (hnd, buffer, &bpos, ref_lon, lat_band, record->water_surface_longitude) < 0)
    {
      sprintf (czmil_error.info, _("Water surface longitude %f difference from platform longitude %f out of range."),
               record->water_surface_longitude, record->platform_longitude);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }


  i32value = NINT (record->water_surface_elevation * czmilh[hnd].cxy_elev_scale) + czmilh[hnd].cxy_elev_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cxy_elev_max)
    {
      sprintf (czmil_error.info, _("Water surface elevation %f out of range."), record->water_surface_elevation);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_elev_bits, i32value);
  bpos += czmilh[hnd].cxy_elev_bits;


  /*  Water level elevation.  */

  i32value = NINT (record->water_level * czmilh[hnd].cxy_elev_scale) + czmilh[hnd].cxy_elev_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cxy_elev_max)
    {
      sprintf (czmil_error.info, _("Water level elevation %f out of range."), record->water_level);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_elev_bits, i32value);
  bpos += czmilh[hnd].cxy_elev_bits;


  /*  Local vertical datum offset (elevation).  */

  i32value = NINT (record->local_vertical_datum_offset * czmilh[hnd].cxy_elev_scale) + czmilh[hnd].cxy_elev_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cxy_elev_max)
    {
      sprintf (czmil_error.info, _("Local vertical datum offset %f out of range."), record->local_vertical_datum_offset);
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_elev_bits, i32value);
  bpos += czmilh[hnd].cxy_elev_bits;


  /*  Loop through all nine channels.  */

  for (i = 0 ; i < 9 ; i++)
    {

      /*  Four possible returns per channel.  */

      for (j = 0 ; j < 4 ; j++)
        {
          /*  If the return is present, process it.  */

          if (j <= record->returns[i])
            {
              /*  Return latitude, longitude, and elevation.  */

              lat = NINT64 ((record->channel[i].data[j].latitude + 90.0) * czmilh[hnd].cxy_pos_scale);
              lon = NINT64 ((record->channel[i].data[j].longitude + 180.0) * czmilh[hnd].cxy_pos_scale);
              lat_diff = (NV_INT32) (ref_lat - lat);
              lon_diff = (NV_INT32) (ref_lon - lon);

              i32value = lat_diff + czmilh[hnd].cxy_lat_diff_offset;
              if (i32value < 0 || i32value > czmilh[hnd].cxy_lat_diff_max)
                {
                  sprintf (czmil_error.info,
                           _("Channel %d return %d latitude %f difference from platform latitude %f out of range."), i, j,
                           record->channel[i].data[j].latitude, record->platform_latitude);
                  strcpy (czmil_error.file, czmilh[hnd].cxy_path);
                  return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
                }
              czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_lat_diff_bits, i32value);
              bpos += czmilh[hnd].cxy_lat_diff_bits;


              /*  Pack the longitude using the proper function based on the latitude band (north of 86N or south of 86S
                  is full longitude).  */

              if ((*czmil_pack_lon[lon_func]) (hnd, buffer, &bpos, ref_lon, lat_band,
                                               record->channel[i].data[j].longitude) < 0)
                {
                  sprintf (czmil_error.info,
                           _("Channel %d return %d longitude %f difference from platform longitude %f out of range."),
                           i, j, record->channel[i].data[j].longitude, record->platform_longitude);
                  strcpy (czmil_error.file, czmilh[hnd].cxy_path);
                  return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
                }

              i32value = NINT (record->channel[i].data[j].elevation * czmilh[hnd].cxy_elev_scale) +
                czmilh[hnd].cxy_elev_offset;
              if (i32value < 0 || i32value > czmilh[hnd].cxy_elev_max)
                {
                  sprintf (czmil_error.info, _("Channel %d return %d elevation %f out of range."), i, j,
                           record->channel[i].data[j].elevation);
                  strcpy (czmil_error.file, czmilh[hnd].cxy_path);
                  return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
                }
              czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_elev_bits, i32value);
              bpos += czmilh[hnd].cxy_elev_bits;


              /*  Reflectance.  */

              i32value = NINT (record->channel[i].data[j].reflectance * czmilh[hnd].cxy_reflectance_scale) +
                czmilh[hnd].cxy_reflectance_offset;
              if (i32value < 0 || i32value > czmilh[hnd].cxy_reflectance_max)
                {
                  sprintf (czmil_error.info, _("Channel %d return %d reflectance %f out of range."), i, j,
                           record->channel[i].data[j].reflectance);
                  strcpy (czmil_error.file, czmilh[hnd].cxy_path);
                  return (czmil_error.czmil = CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR);
                }
              czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_reflectance_bits, i32value);
              bpos += czmilh[hnd].cxy_reflectance_bits;


              /*  Horizontal and vertical uncertainty.  */

              i32value = NINT (record->channel[i].data[j].horizontal_uncertainty * czmilh[hnd].cxy_uncert_scale);
              if (i32value > czmilh[hnd].cxy_uncert_max) i32value = czmilh[hnd].cxy_uncert_max;
              czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_uncert_bits, i32value);
              bpos += czmilh[hnd].cxy_uncert_bits;

              i32value = NINT (record->channel[i].data[j].vertical_uncertainty * czmilh[hnd].cxy_uncert_scale);
              if (i32value > czmilh[hnd].cxy_uncert_max) i32value = czmilh[hnd].cxy_uncert_max;
              czmil_bit_pack (buffer, bpos, czmilh[hnd].cxy_uncert_bits, i32value);
              bpos += czmilh[hnd].cxy_uncert_bits;


              /*  Status ?????  */
            }
        }


      /*  Waveform class ?????  */
    }


  /*  Pack in the buffer size.  */

  size = bpos / 8;
  if (bpos % 8) size++;

  czmil_bit_pack (buffer, 0, czmilh[hnd].cxy_buffer_size_bytes * 8, size);


  /*  Now write the buffer to the file.  */

  if (!fwrite (buffer, size, 1, czmilh[hnd].cxy_fp))
    {
      czmil_error.system = errno;
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cxy_path);
      return (czmil_error.czmil = CZMIL_CXY_WRITE_ERROR);
    }


  czmilh[hnd].cxy_pos += size;
  czmilh[hnd].cxy_modified = NVTrue;
  czmilh[hnd].cxy_write = NVTrue;


  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_compress_cwf_record

 - Purpose:     Compress and bit pack a CZMIL CWF waveform record into an unsigned byte buffer.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/02/11

 - Arguments:
                - hnd            =    The file handle
                - record         =    CZMIL CWF record
                - buffer         =    Unsigned byte buffer in which to store the compressed/packed data

 - Returns:
                - Size of packed buffer in bytes


*********************************************************************************************/

static NV_INT32 czmil_compress_cwf_record (NV_INT32 hnd, CZMIL_CWF_Data *record, NV_U_BYTE *buffer)
{
  NV_U_INT16 start[4], offset[4], delta_bits[4], size[4], buffer_size = 0;
  NV_INT16 i, j, k, m, min_delta[4], max_value[4], previous, delta[63], delta2[62], delta3[64], type;
  NV_U_INT16 *packet = NULL, *shallow_central = NULL;
  NV_INT32 bpos, num_bits, i32value;


  /*  We need to skip the buffer size at the beginning of the buffer.  The buffer size will be stored in 
      czmilh[hnd].cwf_buffer_size_bytes bytes.  We multiply by 8 to get the number of bits to skip.  */

  bpos = czmilh[hnd].cwf_buffer_size_bytes * 8;


  /*  Loop through each of the 9 channels and compress all of the available 64 sample packets.  */

  for (i = 0 ; i < 9 ; i++)
    {
      /*  First pack the channel packet "data" flags (0 or 1) into the buffer.  */

      for (j = 0 ; j < czmilh[hnd].cwf_czmil_max_packets ; j++)
        {
          czmil_bit_pack (buffer, bpos, 1, record->channel_ndx[i][j]);
          bpos += 1;
        }


      /*  Now loop through all of the possible packets and compress them.  */

      for (j = 0 ; j < czmilh[hnd].cwf_czmil_max_packets ; j++)
        {

          /*  If the "data" flag is 1, compress the data.  */

          if (record->channel_ndx[i][j])
            {
              /*  Set the packet pointer to point to the correct channel and packet.  */

              packet = &record->channel[i][j * 64];


              /*  This is the uncompressed (i.e. bit packed values) buffer size for comparison.  */

              size[0] = czmilh[hnd].cwf_type_0_bytes;


              /************** First difference computations ******************/


              /*  Compute the first differences and figure out how big the buffer will be for compression type 1.  
                  The start value is the first value in the packet.  */

              start[1] = previous = packet[0];
              min_delta[1] = 32767;


              /*  Compute the 63 possible difference values and determine the minimum value.  */

              for (k = 1 ; k < 64 ; k++)
                {
                  m = k - 1;
                  delta[m] = packet[k] - previous;
                  min_delta[1] = MIN (delta[m], min_delta[1]);
                  previous = packet[k];
                }


              /*  The offset to be applied to the differences is the minimum difference. */

              offset[1] = -min_delta[1];


              /*  Compute the maximum value to be stored while computing the values.  */

              max_value[1] = 0;
              for (k = 0 ; k < 63 ; k++) max_value[1] = MAX (max_value[1], delta[k] + offset[1]);


              /*  Compute the number of bits needed to store values based on the maximum value.  */

              delta_bits[1] = NINT (log10 ((NV_FLOAT64) max_value[1]) / LOG2 + 0.5);
              if (!max_value[1]) delta_bits[1] = 0;


              /*  Compute the first difference packed size.  */

              num_bits = czmilh[hnd].cwf_type_1_header_bits + 63 * delta_bits[1];
              size[1] = num_bits / 8;
              if (num_bits % 8) size[1]++;


              /*  Add the first difference offset to the first difference delta values.  This either has to be done here
                  or on output to the buffer.  We do it here so that the second difference computation gets to work
                  against all positive values.  */

              for (k = 0 ; k < 63 ; k++) delta[k] += offset[1];


              /************** Second difference computations *****************/


              /*  Compute the second differences and figure out how big the buffer will be for compression type 2.
                  The start value is the first of the first difference values.  */

              start[2] = previous = delta[0];
              min_delta[2] = 32767;


              /*  Compute the 62 possible difference values and determine the minimum value.  */

              for (k = 0 ; k < 63 ; k++)
                {
                  m = k - 1;
                  delta2[m] = delta[k] - previous;
                  min_delta[2] = MIN (delta2[m], min_delta[2]);
                  previous = delta[k];
                }


              /*  The offset is the min difference. */

              offset[2] = -min_delta[2];


              /*  Compute the maximum value to be stored while computing the values.  */

              max_value[2] = 0;
              for (k = 0 ; k < 62 ; k++) max_value[2] = MAX (max_value[2], delta2[k] + offset[2]);


              /*  Compute the number of bits needed to store values based on the maximum value.  */

              delta_bits[2] = NINT (log10 ((NV_FLOAT64) max_value[2]) / LOG2 + 0.5);
              if (!max_value[2]) delta_bits[2] = 0;


              /*  Compute the second difference packed size.  */

              num_bits = czmilh[hnd].cwf_type_2_header_bits + 62 * delta_bits[2];
              size[2] = num_bits / 8;
              if (num_bits % 8) size[2]++;


              /*  Figure out which one saves the most space.  */

              if (size[0] <= size[1] && size[0] <= size[2])
                {
                  type = 0;
                }
              else if (size[1] < size[0] && size[1] <= size[2])
                {
                  type = 1;
                }
              else
                {
                  type = 2;
                }


              /********** Shallow channel difference computations ************/


              /*  If we're checking shallow channels other than the central channel (in other words, channel[2] through
                  channel[7]) we need to see if we can save space by storing these as differences from the central
                  shallow channel instead of "along the waveform" differences.  */

              size[3] = 999;
              if (i > 1 && i < 8)
                {
                  /*  Set the pointer to the correct packet in the shallow central channel (channel[1]).  */

                  shallow_central = &record->channel[1][j * 64];


                  /*  Compute the 64 possible difference values and the minimum difference.  */

                  min_delta[3] = 32767;
                  for (k = 0 ; k < 64 ; k++)
                    {
                      delta3[k] = packet[k] - shallow_central[k];
                      min_delta[3] = MIN (delta3[k], min_delta[3]);
                    }


                  /*  The offset is the min difference. */

                  offset[3] = -min_delta[3];


                  max_value[3] = 0;
                  for (k = 0 ; k < 64 ; k++) max_value[3] = MAX (max_value[3], delta3[k] + offset[3]);


                  /*  Compute the number of bits needed to store values based on the maximum value.  */

                  delta_bits[3] = NINT (log10 ((NV_FLOAT64) max_value[3]) / LOG2 + 0.5);
                  if (!max_value[3]) delta_bits[3] = 0;


                  /*  Compute the shallow channel difference packed size.  */

                  num_bits = czmilh[hnd].cwf_type_3_header_bits + 64 * delta_bits[3];
                  size[3] = num_bits / 8;
                  if (num_bits % 8) size[3]++;


                  /*  Check to see if this will make a smaller buffer than the previously chosen compression type.  */

                  if (size[3] < size[type]) type = 3;
                }


              /*  Pack the buffer.  This is the compression type.  */
              /*fprintf(stderr,"%s %d %d %d %d %d %d %d %d\n",__FILE__,__LINE__,i,j,type,size[0],size[1],size[2],size[3]);*/

              czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_type_bits, type);
              bpos += czmilh[hnd].cwf_type_bits;


              switch (type)
                {
                  /*  Compression type 0.  Just bit packed.  */

                case 0:
                  for (k = 0 ; k < 64 ; k++)
                    {
                      czmil_bit_pack (buffer, bpos, packet[k], 10);
                      bpos += 10;
                    }
                  break;


                  /*  Compression type 1.  Bit packed first differences.  */

                case 1:
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_type_1_start_bits, start[type]);
                  bpos += czmilh[hnd].cwf_type_1_start_bits;
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_type_1_offset_bits, offset[type] +
                                  czmilh[hnd].cwf_type_1_offset);
                  bpos += czmilh[hnd].cwf_type_1_offset_bits;
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_delta_bits, delta_bits[type]);
                  bpos += czmilh[hnd].cwf_delta_bits;

                  for (k = 0 ; k < 63 ; k++)
                    {
                      czmil_bit_pack (buffer, bpos, delta_bits[type], delta[k]);
                      bpos += delta_bits[type];
                    }
                  break;


                  /*  Compression type 2.  Bit packed second differences.  */

                case 2:
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_type_1_start_bits, start[1]);
                  bpos += czmilh[hnd].cwf_type_1_start_bits;
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_type_2_start_bits, start[2]);
                  bpos += czmilh[hnd].cwf_type_2_start_bits;
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_type_1_offset_bits, offset[1] +
                                  czmilh[hnd].cwf_type_1_offset);
                  bpos += czmilh[hnd].cwf_type_1_offset_bits;
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_type_2_offset_bits, offset[2] + 
                                  czmilh[hnd].cwf_type_2_offset);
                  bpos += czmilh[hnd].cwf_type_2_offset_bits;
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_delta_bits, delta_bits[type]);
                  bpos += czmilh[hnd].cwf_delta_bits;

                  for (k = 0 ; k < 62 ; k++)
                    {
                      czmil_bit_pack (buffer, bpos, delta_bits[type], delta2[k] + offset[type]);
                      bpos += delta_bits[type];
                    }
                  break;


                  /*  Compression type 3.  Bit packed differences between the central shallow channel and one of the
                      surrounding shallow channels.  */

                case 3:
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_type_3_offset_bits, offset[3] +
                                  czmilh[hnd].cwf_type_1_offset);
                  bpos += czmilh[hnd].cwf_type_3_offset_bits;
                  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_delta_bits, delta_bits[type]);
                  bpos += czmilh[hnd].cwf_delta_bits;

                  for (k = 0 ; k < 64 ; k++)
                    {
                      czmil_bit_pack (buffer, bpos, delta_bits[type], delta3[k] + offset[type]);
                      bpos += delta_bits[type];
                    }
                  break;
                }
            }
        }
    }


  /*  Add the timestamp and the scan_angle.  */

  czmil_double_bit_pack (buffer, bpos, czmilh[hnd].cwf_time_bits, record->timestamp); bpos += czmilh[hnd].cwf_time_bits;

  i32value = NINT (record->scan_angle * czmilh[hnd].cwf_angle_scale) + czmilh[hnd].cwf_scan_angle_offset;
  if (i32value < 0 || i32value > czmilh[hnd].cwf_scan_angle_max)
    {
      sprintf (czmil_error.info, _("Platform scan angle %f out of range."), record->scan_angle);
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_CWF_VALUE_OUT_OF_RANGE_ERROR);
    }
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cwf_scan_angle_bits, i32value);
  bpos += czmilh[hnd].cwf_scan_angle_bits;


  /*  Compute the buffer size and pack it in.  */

  buffer_size = bpos / 8;
  if (bpos % 8) buffer_size++;

  czmil_bit_pack (buffer, 0, czmilh[hnd].cwf_buffer_size_bytes * 8, buffer_size);

  return (buffer_size);
}



/********************************************************************************************/
/*!

 - Function:    czmil_uncompress_cwf_packet_type_0

 - Purpose:     Uncompress and bit unpack a 64 sample unsigned byte buffer that was compressed
                with compression type 0 (no compression) into a CZMIL CWF record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/02/11

 - Arguments:
                - hnd            -    The file handle
                - buffer         -    Unsigned byte buffer from which to retrieve the data
                - packet         -    Unsigned short integer array to hold 64 waveform values
                - dummy          -    Dummy pointer so we can handle the 3 arguments to
                                      czmil_uncompress_cwf_packet_type_3 without an if statement
                - bpos           -    Current bit position within the buffer

 - Returns:
                - CZMIL_SUCCESS

*********************************************************************************************/

static NV_INT32 czmil_uncompress_cwf_packet_type_0 (NV_INT32 hnd, NV_U_BYTE *buffer, NV_U_INT16 *packet,
                                                    NV_U_INT16 *dummy __attribute__ ((unused)), NV_INT32 *bpos)
{
  NV_INT16 i;


  /*  Just unpack the values.  */

  for (i = 0 ; i < 64 ; i++)
    {
      packet[i] = czmil_bit_unpack (buffer, *bpos, 10);
      *bpos += 10;
    }

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_uncompress_cwf_packet_type_1

 - Purpose:     Uncompress and bit unpack a 64 sample unsigned byte buffer that was compressed
                with compression type 1 (first difference compression) into a CZMIL CWF record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/02/11

 - Arguments:
                - hnd            =    The file handle
                - buffer         =    Unsigned byte buffer from which to retrieve the data
                - packet         =    Unsigned short integer array to hold 64 waveform values
                - dummy          =    Dummy pointer so we can handle the 3 arguments to
                                      czmil_uncompress_cwf_packet_type_3 without an if statement
                - bpos           =    Current bit position within the buffer

 - Returns:
                - CZMIL_SUCCESS

*********************************************************************************************/

static NV_INT32 czmil_uncompress_cwf_packet_type_1 (NV_INT32 hnd, NV_U_BYTE *buffer, NV_U_INT16 *packet,
                                                    NV_U_INT16 *dummy __attribute__ ((unused)), NV_INT32 *bpos)
{
  NV_INT16 i, start, offset, delta_bits, previous, delta[63];


  /*  Unpack the start value.  */

  start = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_type_1_start_bits);
  *bpos += czmilh[hnd].cwf_type_1_start_bits;


  /*  Unpack the offset value.  */

  offset = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_type_1_offset_bits) - czmilh[hnd].cwf_type_1_offset;
  *bpos += czmilh[hnd].cwf_type_1_offset_bits;


  /*  Unpack the delta bits value.  */

  delta_bits = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_delta_bits);
  *bpos += czmilh[hnd].cwf_delta_bits;


  /*  Unpack the first differences and remove the offset.  */

  for (i = 0 ; i < 63 ; i++)
    {
      delta[i] = czmil_bit_unpack (buffer, *bpos, delta_bits) - offset;
      *bpos += delta_bits;
    }


  /*  Convert the first differences to waveform values using the start value from the header.  */

  packet[0] = previous = start;

  for (i = 1 ; i < 64 ; i++)
    {
      packet[i] = delta[i - 1] + previous;
      previous = packet[i];
    }

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_uncompress_cwf_packet_type_2

 - Purpose:     Uncompress and bit unpack a 64 sample unsigned byte buffer that was compressed
                with compression type 2 (second difference compression) into a CZMIL CWF record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/02/11

 - Arguments:
                - hnd            =    The file handle
                - buffer         =    Unsigned byte buffer from which to retrieve the data
                - packet         =    Unsigned short integer array to hold 64 waveform values
                - dummy          =    Dummy pointer so we can handle the 3 arguments to
                                      czmil_uncompress_cwf_packet_type_3 without an if statement
                - bpos           =    Current bit position within the buffer

 - Returns:
                - CZMIL_SUCCESS

*********************************************************************************************/

static NV_INT32 czmil_uncompress_cwf_packet_type_2 (NV_INT32 hnd, NV_U_BYTE *buffer, NV_U_INT16 *packet,
                                                    NV_U_INT16 *dummy __attribute__ ((unused)), NV_INT32 *bpos)
{
  NV_U_INT16 start[2], offset[2], delta_bits;
  NV_INT16 i, previous, delta[63], delta2[62];


  /*  Unpack the type 1 and type 2 start bits values.  */

  start[0] = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_type_1_start_bits);
  *bpos += czmilh[hnd].cwf_type_1_start_bits;
  start[1] = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_type_2_start_bits);
  *bpos += czmilh[hnd].cwf_type_2_start_bits;


  /*  Unpack the type 1 and type 2 offset values.  */

  offset[0] = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_type_1_offset_bits) - czmilh[hnd].cwf_type_1_offset;
  *bpos += czmilh[hnd].cwf_type_1_offset_bits;
  offset[1] = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_type_2_offset_bits) - czmilh[hnd].cwf_type_2_offset;
  *bpos += czmilh[hnd].cwf_type_2_offset_bits;


  /*  Unpack the delta bits value.  */

  delta_bits = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_delta_bits);
  *bpos += czmilh[hnd].cwf_delta_bits;


  /*  Unpack the second differences and remove the second offset.  */

  for (i = 0 ; i < 62 ; i++)
    {
      delta2[i] = czmil_bit_unpack (buffer, *bpos, delta_bits) - offset[1];
      *bpos += delta_bits;
    }


  /*  Convert the second differences to first differences using the second start value with the first difference offset
      removed.  */

  delta[0] = previous = start[1] - offset[0];

  for (i = 1 ; i < 63 ; i++)
    {
      delta[i] = delta2[i - 1] + previous;
      previous = delta[i];
    }


  /*  Convert the first differences to waveform values using the first start value.  */

  packet[0] = previous = start[0];

  for (i = 1 ; i < 64 ; i++)
    {
      packet[i] = delta[i - 1] + previous;
      previous = packet[i];
    }

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_uncompress_cwf_packet_type_3

 - Purpose:     Uncompress and bit unpack a 64 sample unsigned byte buffer that was compressed
                with compression type 3 (shallow channel difference from central shallow channel)
                into a CZMIL CWF record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/02/11

 - Arguments:
                - hnd            =    The file handle
                - buffer         =    Unsigned byte buffer from which to retrieve the data
                - packet         =    Unsigned short integer array to hold 64 waveform values
                - shallow_central=    Unsigned short integer array from central shallow channel (channel[1])
                - bpos           =    Current bit position within the buffer

 - Returns:
                - CZMIL_SUCCESS

*********************************************************************************************/

static NV_INT32 czmil_uncompress_cwf_packet_type_3 (NV_INT32 hnd, NV_U_BYTE *buffer, NV_U_INT16 *packet,
                                                    NV_U_INT16 *shallow_central, NV_INT32 *bpos)
{
  NV_INT16 i, offset, delta_bits, delta[64];


  /*  Unpack the type 3 offset value.  */

  offset = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_type_3_offset_bits) - czmilh[hnd].cwf_type_1_offset;
  *bpos += czmilh[hnd].cwf_type_3_offset_bits;


  /*  Unpack the delta bits value.  */

  delta_bits = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_delta_bits);
  *bpos += czmilh[hnd].cwf_delta_bits;


  /*  Unpack the channel differences and remove the offset.  */

  for (i = 0 ; i < 64 ; i++)
    {
      delta[i] = czmil_bit_unpack (buffer, *bpos, delta_bits) - offset;
      *bpos += delta_bits;
    }


  /*  Convert the channel differences to waveform values using the corresponding shallow central channel value.  */

  for (i = 0 ; i < 64 ; i++) packet[i] = delta[i] + shallow_central[i];

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_uncompress_cwf_packet

 - Purpose:     Uncompress and bit unpack a 64 value unsigned byte buffer into a CZMIL CWF
                record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/02/11

 - Arguments:
                - hnd            =    The file handle
                - buffer         =    Unsigned byte buffer from which to retrieve the data
                - packet         =    Unsigned short integer array to hold 64 waveform values
                - shallow_central=    Unsigned short integer array from central shallow channel (channel[1])
                - bpos           =    Current bit position within the buffer

 - Returns:
                - CZMIL_SUCCESS

*********************************************************************************************/

static NV_U_INT16 czmil_uncompress_cwf_packet (NV_INT32 hnd, NV_U_BYTE *buffer, NV_U_INT16 *packet,
                                               NV_U_INT16 *shallow_central, NV_INT32 *bpos)
{
  NV_U_INT16 type;


  /*  A nifty little array of functions so we don't have to use "if" statements.  */

  static NV_INT32 (*czmil_uncomp[4]) (NV_INT32, NV_U_BYTE *, NV_U_INT16 *, NV_U_INT16 *, NV_INT32 *) =
    {czmil_uncompress_cwf_packet_type_0, czmil_uncompress_cwf_packet_type_1, 
     czmil_uncompress_cwf_packet_type_2, czmil_uncompress_cwf_packet_type_3};


  /*  Unpack the compression type.  */

  type = czmil_bit_unpack (buffer, *bpos, czmilh[hnd].cwf_type_bits);
  *bpos += czmilh[hnd].cwf_type_bits;


  /*  Using a static array of functions allows us to uncompress the waveform data without having to use any
      "if" statements in the uncompressing code. We just call (*czmil_uncomp[type]) where "type" is the
      compression type read from the common header.  */

  (*czmil_uncomp[type]) (hnd, buffer, packet, shallow_central, bpos);


  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_uncompress_cwf_record

 - Purpose:     Uncompress and bit unpack a CZMIL CWF waveform record from an unsigned byte buffer.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/02/11

 - Arguments:
                - hnd            =    The file handle
                - record         =    CZMIL CWF record
                - buffer         =    Unsigned byte buffer from which to uncompress/unpack data

 - Returns:
                - CZMIL_SUCCESS

*********************************************************************************************/

static NV_INT32 czmil_uncompress_cwf_record (NV_INT32 hnd, CZMIL_CWF_Data *record, NV_U_BYTE *buffer)
{
  NV_INT16 i, j;
  NV_INT32 bpos, i32value;


  /*  Zero the entire record so that empty packets will be initialized.  */

  memset (record, 0, sizeof (CZMIL_CWF_Data));


  /*  We need to skip the buffer size in the beginning of the buffer.  The buffer size will be stored in 
      czmilh[hnd].cwf_buffer_size_bytes bytes.  We multiply by 8 to get the number of bits to skip.  */

  bpos = czmilh[hnd].cwf_buffer_size_bytes * 8;


  /*  Loop through each of the nine channels.  */

  for (i = 0 ; i < 9 ; i++)
    {
      /*  First unpack the channel packet "data" flags (0 or 1) from the buffer.  */

      for (j = 0 ; j < czmilh[hnd].cwf_czmil_max_packets ; j++)
        {
          record->channel_ndx[i][j] = czmil_bit_unpack (buffer, bpos, 1);
          bpos += 1;
        }


      /*  Now unpack the packet.  */

      for (j = 0 ; j < czmilh[hnd].cwf_czmil_max_packets ; j++)
        {
          /*  If the "data" flag is 1 we have data in the packet.  */

          if (record->channel_ndx[i][j])
            {
              czmil_uncompress_cwf_packet (hnd, buffer, &record->channel[i][j * 64], &record->channel[1][j * 64], &bpos);
            }
        }
    }


  /*  Unpack the timestamp and the scan_angle.  */

  record->timestamp = czmil_double_bit_unpack (buffer, bpos, czmilh[hnd].cwf_time_bits);
  bpos += czmilh[hnd].cwf_time_bits;

  i32value = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cwf_scan_angle_bits);
  bpos += czmilh[hnd].cwf_scan_angle_bits;
  record->scan_angle = (NV_FLOAT32) (i32value - czmilh[hnd].cwf_scan_angle_offset) / czmilh[hnd].cwf_angle_scale;

  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_read_cwf_record

 - Purpose:     Retrieve a CZMIL CWF record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - recnum         =    The record number of the CZMIL record to be retrieved
                - record         =    The returned CZMIL CWF record

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_INVALID_RECORD_NUMBER
                - CZMIL_CWF_READ_FSEEK_ERROR
                - CZMIL_CWF_READ_ERROR

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_read_cwf_record (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CWF_Data *record)
{
  NV_INT32 i, j, bpos;
  NV_INT64 size, pos;
  NV_U_INT16 buffer[CZMIL_MAX_PACKETS * 64 * 9 + sizeof (NV_INT64) / 2 + sizeof (NV_FLOAT32) / 2];


  /*  This is hardwired for now.  We'll be getting the position from an index file.  */

  memset (record, 0, sizeof (CZMIL_CWF_Data));
  for (i = 0 ; i < 9 ; i++)
    {
      for (j = 0 ; j < 10 ; j++)
        {
          record->channel_ndx[i][j] = 1;
        }
    }


  /*  Compute the size of the record to be read and then place the information in the proper places in the record.  */

  size = 0;
  for (i = 0 ; i < 9 ; i++)
    {
      for (j = 0 ; j < czmilh[hnd].cwf_czmil_max_packets ; j++)
        {
          if (record->channel_ndx[i][j]) size += 128;
        }
    }


  /*  Add in the timestamp and scan_angle sizes.  */

  size += sizeof (NV_INT64) + sizeof (NV_FLOAT32);


  /*  Check for record out of bounds.  */

  if (recnum >= czmilh[hnd].cxy_header.number_of_records || recnum < 0)
    {
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_INVALID_RECORD_NUMBER);
    }


  pos = (NV_INT64) recnum * size;


  /*  We only want to do the fseek (which flushes the buffer) if our last operation was a write or if we aren't in the
      correct position.  */

  if (czmilh[hnd].cwf_write || pos != czmilh[hnd].cwf_pos)
    {
      if (fseeko64 (czmilh[hnd].cwf_fp, pos, SEEK_SET) < 0)
        {
          czmil_error.system = errno;
          czmil_error.recnum = recnum;
          strcpy (czmil_error.file, czmilh[hnd].cwf_path);
          return (czmil_error.czmil = CZMIL_CWF_READ_FSEEK_ERROR);
        }
    }


  /*  Read the waveform data.  */

  if (!fread (buffer, size, 1, czmilh[hnd].cwf_fp))
    {
      czmil_error.system = errno;
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_CWF_READ_ERROR);
    }


  /*  This whole section will eventually be replaced by a single czmil_uncompress_cwf_record call but, for now...  */


  /*  Place the waveform information in the proper location in the record.  */

  bpos = 0;
  for (i = 0 ; i < 9 ; i++)
    {
      for (j = 0 ; j < czmilh[hnd].cwf_czmil_max_packets ; j++)
        {
          if (record->channel_ndx[i][j])
            {
              memcpy (&record->channel[i][j * 64], &buffer[bpos], 128);
              bpos += 64;
            }
        }
    }


  memcpy (&record->timestamp, &buffer[bpos], sizeof (NV_INT64));
  bpos += 4;
  memcpy (&record->scan_angle, &buffer[bpos], sizeof (NV_FLOAT32));


  /*  Testing

  CZMIL_CWF_Data record2;
  NV_U_BYTE comp_buffer[sizeof (CZMIL_CWF_Data)];
  NV_INT32 k;
  static NV_INT64 sum = 0;
  static NV_INT32 count = 0;
  memset (&record2, 0, sizeof (CZMIL_CWF_Data));

  memset (comp_buffer, 0, sizeof (CZMIL_CWF_Data));
  i = czmil_compress_cwf_record (hnd, &record, comp_buffer);
  czmil_uncompress_cwf_record (hnd, &record2, comp_buffer);
  sum += i;
  count++;

  fprintf(stderr,"%s %d %d %d %d\n",__FILE__,__LINE__,i,sum,count);

  for (i = 0 ; i < 9 ; i++)
    {
      for (j = 0 ; j < czmilh[hnd].cwf_czmil_max_packets ; j++)
        {
          if (record.channel_ndx[i][j] != record2.channel_ndx[i][j])
            {
              DPRINT
                exit (-1);
            }

          for (k = 0 ; k < 64 ; k++)
            {
              if (record.channel[i][j * 64 + k] != record2.channel[i][j * 64 + k])
                {
                  fprintf (stderr,"%s %d %d %d %d %d %d %d\n",__FILE__,__LINE__,i,j,k,record.channel[i][j * 64 + k],record2.channel[i][j * 64 + k],record2.channel[0][0]);
                  exit (-1);
                }
            }
        }
    }


      End testing  */


  czmilh[hnd].cwf_pos += size;
  czmilh[hnd].cwf_at_end = NVFalse;
  czmilh[hnd].cwf_write = NVFalse;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/*********************************************************************************************/
/*!

 - Function:    czmil_read_cwf_record2

 - Purpose:     Retrieve a CZMIL CWF record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - recnum         =    The record number of the CZMIL record to be retrieved
                - record         =    The returned CZMIL CWF record

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_INVALID_RECORD_NUMBER
                - CZMIL_CWF_READ_FSEEK_ERROR
                - CZMIL_CWF_READ_ERROR

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_read_cwf_record2 (NV_INT32 hnd, NV_INT32 recnum, CZMIL_CWF_Data *record)
{
  NV_INT64 size;


  /*  The actual buffer will never be sizeof (CZMIL_CWF_Data) in size since we are unpacking it but this way
      we don't have to worry about an SOD error.  */

  NV_U_BYTE buffer[sizeof (CZMIL_CWF_Data)];


  /*  Check for record out of bounds.  */

  if (recnum >= czmilh[hnd].cwf_header.number_of_records || recnum < 0)
    {
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_INVALID_RECORD_NUMBER);
    }


  /*  Get the CWF record byte address and buffer size from the CDX index file.  */

  czmil_read_cdx_record (hnd, recnum);


  /*  We only want to do the fseek (which flushes the buffer) if our last operation was a write or if we aren't in the
      correct position.  */

  if (czmilh[hnd].cwf_write || czmilh[hnd].cdx_record.cwf_address != czmilh[hnd].cwf_pos)
    {
      if (fseeko64 (czmilh[hnd].cwf_fp, czmilh[hnd].cdx_record.cwf_address, SEEK_SET) < 0)
        {
          czmil_error.system = errno;
          czmil_error.recnum = recnum;
          strcpy (czmil_error.file, czmilh[hnd].cwf_path);
          return (czmil_error.czmil = CZMIL_CWF_READ_FSEEK_ERROR);
        }
    }


  czmilh[hnd].cwf_at_end = NVFalse;


  /*  Read the buffer.  */

  if (!fread (buffer, czmilh[hnd].cdx_record.cwf_buffer_size, 1, czmilh[hnd].cwf_fp))
    {
      czmil_error.system = errno;
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_CWF_READ_ERROR);
    }


  /*  Make sure the buffer size read from the CWF file matches the buffer size read from the CDX file.  */

  size = czmil_bit_unpack (buffer, 0, czmilh[hnd].cwf_buffer_size_bytes * 8);

  if (size != czmilh[hnd].cdx_record.cwf_buffer_size)
    {
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_CWF_CDX_BUFFER_SIZE_ERROR);
    }


  /*  Unpack the record.  */

  czmil_uncompress_cwf_record (hnd, record, buffer);


  czmilh[hnd].cwf_pos += size;
  czmilh[hnd].cwf_at_end = NVFalse;
  czmilh[hnd].cwf_write = NVFalse;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_append_cwf_record

 - Purpose:     Append a CZMIL CWF record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - record         =    The CZMIL CWF record

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_INVALID_RECORD_NUMBER
                - CZMIL_WRITE_FSEEK_ERROR
                - CZMIL_WRITE_ERROR

 - Caveats:     This function is ONLY used to append a new record to a newly created file.
                There should be no reason to actually change waveform data after the file
                has been created.

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_append_cwf_record (NV_INT32 hnd, CZMIL_CWF_Data *record)
{
  NV_INT64 size;
  NV_U_BYTE buffer[sizeof (CZMIL_CWF_Data)];


  /*  Appending a record is only allowed if you are creating a new file.  */

  if (!czmilh[hnd].cwf_created)
    {
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_CWF_APPEND_ERROR);
    }


  /*  If we're not already at the end of the file...  */

  if (!czmilh[hnd].cwf_at_end)
    {
      /*  We're appending so we need to seek to the end of the file.  */

      if (fseeko64 (czmilh[hnd].cwf_fp, 0, SEEK_END) < 0)
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cwf_path);
          return (czmil_error.czmil = CZMIL_CWF_WRITE_FSEEK_ERROR);
        }
    }

  czmilh[hnd].cwf_at_end = NVTrue;


  /*  Pack the record.  */

  size = czmil_compress_cwf_record (hnd, record, buffer);


  /*  Write the buffer.  */

  if (!fwrite (buffer, size, 1, czmilh[hnd].cxy_fp))
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cwf_path);
      return (czmil_error.czmil = CZMIL_CWF_WRITE_ERROR);
    }


  czmilh[hnd].cwf_pos += size;
  czmilh[hnd].cwf_write = NVTrue;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_read_cdx_record

 - Purpose:     Retrieve a CZMIL CDX record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - recnum         =    The record number of the CZMIL record to be retrieved
                - record         =    The returned CZMIL CDX record

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_INVALID_RECORD_NUMBER
                - CZMIL_CDX_READ_FSEEK_ERROR
                - CZMIL_CDX_READ_ERROR

 - Caveats:     Note that we don't pass in a record structure to fill.  This is because this
                function is only used internally and we want to put the record into
                czmilh[hnd].cdx_record so that we can re-use it for CXY and CWF I/O.

*********************************************************************************************/

static NV_INT32 czmil_read_cdx_record (NV_INT32 hnd, NV_INT32 recnum)
{
  NV_INT64 pos;
  NV_INT32 bpos;
  NV_U_BYTE buffer[sizeof (CZMIL_CDX_Data) + 16];


  /*  Check for record out of bounds.  */

  if (recnum >= czmilh[hnd].cdx_header.number_of_records || recnum < 0)
    {
      czmil_error.recnum = recnum;
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_INVALID_RECORD_NUMBER);
    }


  /*  Compute the record position based on the record size computed from the packed bit sizes of the CDX record.  */

  pos = (NV_INT64) recnum * (NV_INT64) czmilh[hnd].cdx_record_size_bytes +
    (NV_INT64) czmilh[hnd].cdx_header.header_size;


  /*  We only want to do the fseek (which flushes the buffer) if our last operation was a write or if we aren't in the
      correct position.  We also don't need to read the record since it is in czmilh[hnd].cdx_record so, at this
      point we'll just return.  */

  if (czmilh[hnd].cdx_write || pos != czmilh[hnd].cdx_pos)
    {
      if (fseeko64 (czmilh[hnd].cdx_fp, pos, SEEK_SET) < 0)
        {
          czmil_error.system = errno;
          czmil_error.recnum = recnum;
          strcpy (czmil_error.file, czmilh[hnd].cdx_path);
          return (czmil_error.czmil = CZMIL_CDX_READ_FSEEK_ERROR);
        }


      /*  Read the record.  */

      if (!fread (buffer, czmilh[hnd].cdx_record_size_bytes, 1, czmilh[hnd].cdx_fp))
        {
          czmil_error.system = errno;
          czmil_error.recnum = recnum;
          strcpy (czmil_error.file, czmilh[hnd].cdx_path);
          return (czmil_error.czmil = CZMIL_CDX_READ_ERROR);
        }


      /*  Unpack the CXY address, the CWF address, the CXY buffer size, and the CWF buffer size.  */

      bpos = 0;
      czmilh[hnd].cdx_record.cxy_address = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cdx_cxy_address_bits);
      bpos += czmilh[hnd].cdx_cxy_address_bits;
      czmilh[hnd].cdx_record.cwf_address = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cdx_cwf_address_bits);
      bpos += czmilh[hnd].cdx_cwf_address_bits;
      czmilh[hnd].cdx_record.cxy_buffer_size = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cdx_cxy_buffer_size_bits) / 8;
      bpos += czmilh[hnd].cdx_cxy_buffer_size_bits;
      czmilh[hnd].cdx_record.cwf_buffer_size = czmil_bit_unpack (buffer, bpos, czmilh[hnd].cdx_cwf_buffer_size_bits) / 8;
      bpos += czmilh[hnd].cdx_cwf_buffer_size_bits;

  
      czmilh[hnd].cdx_pos += czmilh[hnd].cdx_record_size_bytes;
      czmilh[hnd].cdx_at_end = NVFalse;
      czmilh[hnd].cdx_write = NVFalse;
    }

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_append_cdx_record

 - Purpose:     Append or update a CZMIL CDX record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - record         =    The CZMIL CDX record

 - Returns:
                - CZMIL_SUCCESS
                - CZMIL_INVALID_RECORD_NUMBER
                - CZMIL_CDX_WRITE_FSEEK_ERROR
                - CZMIL_CDX_WRITE_ERROR

 - Caveats:     This function is ONLY used to append a new record to a newly created file.
                There is no reason to change CXY or CWF addresses after the file has been
                created.

*********************************************************************************************/

static NV_INT32 czmil_append_cdx_record (NV_INT32 hnd, CZMIL_CDX_Data *record)
{
  NV_INT32 bpos;
  NV_U_BYTE buffer[sizeof (CZMIL_CDX_Data) + 16];


  /*  Appending a record is only allowed if you are creating a new file.  */

  if (!czmilh[hnd].cdx_created)
    {
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_CDX_APPEND_ERROR);
    }


  /*  No need to seek if we're already at the end.  */

  if (!czmilh[hnd].cdx_at_end)
    {
      if (fseeko64 (czmilh[hnd].cdx_fp, 0, SEEK_END) < 0)
        {
          czmil_error.system = errno;
          strcpy (czmil_error.file, czmilh[hnd].cdx_path);
          return (czmil_error.czmil = CZMIL_CDX_WRITE_FSEEK_ERROR);
        }
    }

  czmilh[hnd].cdx_at_end = NVTrue;


  /*  Pack the CXY address, the CWF address, the CXY buffer size, and the CWF record size into the buffer.  */

  bpos = 0;
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cdx_cxy_address_bits, record->cxy_address);
  bpos += czmilh[hnd].cdx_cxy_address_bits;
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cdx_cwf_address_bits, record->cwf_address);
  bpos += czmilh[hnd].cdx_cwf_address_bits;
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cdx_cxy_buffer_size_bits, record->cxy_buffer_size);
  bpos += czmilh[hnd].cdx_cxy_buffer_size_bits;
  czmil_bit_pack (buffer, bpos, czmilh[hnd].cdx_cwf_buffer_size_bits, record->cwf_buffer_size);
  bpos += czmilh[hnd].cdx_cwf_buffer_size_bits;


  /*  Write the record.  */

  if (!fwrite (buffer, czmilh[hnd].cdx_record_size_bytes, 1, czmilh[hnd].cdx_fp))
    {
      czmil_error.system = errno;
      strcpy (czmil_error.file, czmilh[hnd].cdx_path);
      return (czmil_error.czmil = CZMIL_CDX_WRITE_ERROR);
    }


  czmilh[hnd].cdx_pos += czmilh[hnd].cdx_record_size_bytes;
  czmilh[hnd].cdx_write = NVTrue;

  czmil_error.system = 0;
  return (czmil_error.czmil = CZMIL_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    czmil_update_cxy_header

 - Purpose:     Update the modifiable fields of the CXY header record.  See CZMIL_CXY_Header in
                czmil.h to determine which fields are modifiable after file creation.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - cxy_header     =    The CZMIL_CXY_Header structure.

 - Returns:
                - void

 - Modifiable fields: 

                - modification_software
                - local_vertical_datum
                - security_classification
                - distribution
                - declassification
                - class_just
                - downgrade
                - comments


*********************************************************************************************/

CZMIL_DLL void czmil_update_cxy_header (NV_INT32 hnd, CZMIL_CXY_Header cxy_header)
{
  /*  These are the only modifiable fields.  */

  strcpy (czmilh[hnd].cxy_header.modification_software, cxy_header.modification_software);
  czmilh[hnd].cxy_header.local_vertical_datum = cxy_header.local_vertical_datum;
  strcpy (czmilh[hnd].cxy_header.security_classification, cxy_header.security_classification);
  strcpy (czmilh[hnd].cxy_header.distribution, cxy_header.distribution);
  strcpy (czmilh[hnd].cxy_header.declassification, cxy_header.declassification);
  strcpy (czmilh[hnd].cxy_header.class_just, cxy_header.class_just);
  strcpy (czmilh[hnd].cxy_header.downgrade, cxy_header.downgrade);
  strcpy (czmilh[hnd].cxy_header.comments, cxy_header.comments);


  /*  Force a header write when we close the file.  */

  czmilh[hnd].cxy_modified = NVTrue;
}



/********************************************************************************************/
/*!

 - Function:    czmil_update_cwf_header

 - Purpose:     Update the modifiable fields of the CWF header record.  See CZMIL_CWF_Header in
                czmil.h to determine which fields are modifiable after file creation.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - hnd            =    The file handle
                - cwf_header     =    The CZMIL_CWF_Header structure.

 - Returns:
                - void

 - Modifiable fields: 

                - security_classification
                - distribution
                - declassification
                - class_just
                - downgrade
                - comments


 - Caveats:     There is very little cause for using this function unless you want to add 
                comments or security information that aren't the same as the CXY file.  Since
                most people don't even populate these fields this is kind of pointless but it's
                here just in case.  Note that you can just instantiate an empty CZMIL_CWF_Header
                structure and fill the modifiable fields since the rest of the structure is ignored.

*********************************************************************************************/

CZMIL_DLL void czmil_update_cwf_header (NV_INT32 hnd, CZMIL_CXY_Header cwf_header)
{
  /*  These are the only modifiable fields.  */

  strcpy (czmilh[hnd].cwf_header.security_classification, cwf_header.security_classification);
  strcpy (czmilh[hnd].cwf_header.distribution, cwf_header.distribution);
  strcpy (czmilh[hnd].cwf_header.declassification, cwf_header.declassification);
  strcpy (czmilh[hnd].cwf_header.class_just, cwf_header.class_just);
  strcpy (czmilh[hnd].cwf_header.downgrade, cwf_header.downgrade);
  strcpy (czmilh[hnd].cwf_header.comments, cwf_header.comments);


  /*  Force a header write when we close the file.  */

  czmilh[hnd].cwf_modified = NVTrue;
}



/********************************************************************************************/
/*!

 - Function:    czmil_get_errno

 - Purpose:     Returns the latest CZMIL error condition code

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - void

 - Returns:
                - error condition code

 - Caveats:     The only thing this is good for at present is to determine if, when you opened
                the file, the library version was older than the file.  That is, if
                CZMIL_NEWER_FILE_VERSION_WARNING has been set when you called czmil_open_file.
                Otherwise, you can just use czmil_perror or czmil_strerror to get the last
                error information.

*********************************************************************************************/

CZMIL_DLL NV_INT32 czmil_get_errno ()
{
  return (czmil_error.czmil);
}



/********************************************************************************************/
/*!

 - Function:    czmil_strerror

 - Purpose:     Returns the error string related to the latest error.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - void

 - Returns:
                - Error message

*********************************************************************************************/

CZMIL_DLL NV_CHAR *czmil_strerror ()
{
  static NV_CHAR message[1024];

  switch (czmil_error.czmil)
    {
    case CZMIL_SUCCESS:
      sprintf (message, _("SUCCESS!\n"));
      break;

    case CZMIL_CXY_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing CXY data :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CXY_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing CXY data :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CWF_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing waveform data :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CWF_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing waveform data :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CDX_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing CDX data :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CDX_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing CDX data :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CXY_HEADER_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing CXY header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CXY_HEADER_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing CXY header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CWF_HEADER_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing CWF header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CWF_HEADER_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing CWF header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CDX_HEADER_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing CDX header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CDX_HEADER_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing CDX header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CXY_HEADER_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to reading CXY header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CWF_HEADER_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to reading CWF header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CDX_HEADER_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to reading CDX header :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_TOO_MANY_OPEN_FILES:
      sprintf (message, _("Too many CZMIL files are already open.\n"));
      break;

    case CZMIL_CREATE_CXY_ERROR:
      sprintf (message, _("File : %s\nError creating CZMIL CXY file :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CREATE_CWF_ERROR:
      sprintf (message, _("File : %s\nError creating CZMIL CWF file :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_CREATE_CDX_ERROR:
      sprintf (message, _("File : %s\nError creating CZMIL CDX file :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_OPEN_CXY_UPDATE_ERROR:
      sprintf (message, _("File : %s\nError opening CXY file for update :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_OPEN_CXY_READONLY_ERROR:
      sprintf (message, _("File : %s\nError opening CXY file read-only :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_OPEN_CWF_UPDATE_ERROR:
      sprintf (message, _("File : %s\nError opening CWF file for update :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_OPEN_CWF_READONLY_ERROR:
      sprintf (message, _("File : %s\nError opening CWF file read-only :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_OPEN_CDX_UPDATE_ERROR:
      sprintf (message, _("File : %s\nError opening CDX file for update :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_OPEN_CDX_READONLY_ERROR:
      sprintf (message, _("File : %s\nError opening CDX file read-only :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_NOT_CZMIL_FILE_ERROR:
      sprintf (message, _("File : %s\nThe file version string is corrupt or indicates that this is not a CZMIL file.\n"),
               czmil_error.file);
      break;

    case CZMIL_NEWER_FILE_VERSION_WARNING:
      sprintf (message,
               _("File : %s\nThe file version is newer than the CZMIL library version.\nThis may cause problems.\n"),
               czmil_error.file);
      break;

    case CZMIL_CLOSE_ERROR:
      sprintf (message, _("File : %s\nError closing CZMIL file :\n%s\n"),
               czmil_error.file, strerror (czmil_error.system));
      break;

    case CZMIL_INVALID_RECORD_NUMBER:
      sprintf (message, _("File : %s\nRecord : %d\nInvalid record number.\n"),
               czmil_error.file, czmil_error.recnum);
      break;

    case CZMIL_CXY_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError during fseek prior to reading CXY record :\n%s\n"),
               czmil_error.file, czmil_error.recnum, strerror (czmil_error.system));
      break;

    case CZMIL_CXY_READ_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError reading CXY record :\n%s\n"),
               czmil_error.file, czmil_error.recnum, strerror (czmil_error.system));
      break;

    case CZMIL_CWF_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError during fseek prior to reading CWF record :\n%s\n"),
               czmil_error.file, czmil_error.recnum, strerror (czmil_error.system));
      break;

    case CZMIL_CWF_READ_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError reading CWF record :\n%s\n"),
               czmil_error.file, czmil_error.recnum, strerror (czmil_error.system));
      break;

    case CZMIL_CDX_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError during fseek prior to reading CDX record :\n%s\n"),
               czmil_error.file, czmil_error.recnum, strerror (czmil_error.system));
      break;

    case CZMIL_CDX_READ_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nError reading CDX record :\n%s\n"),
               czmil_error.file, czmil_error.recnum, strerror (czmil_error.system));
      break;

    case CZMIL_CXY_APPEND_ERROR:
      sprintf (message, _("Appending to preexisting file %s is not allowed!\n"), czmil_error.file);
      break;

    case CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR:
      sprintf (message, N_("In CXY file %s - %s\n"), czmil_error.file, czmil_error.info);
      break;

    case CZMIL_CWF_VALUE_OUT_OF_RANGE_ERROR:
      sprintf (message, N_("In CWF file %s - %s\n"), czmil_error.file, czmil_error.info);
      break;

    case CZMIL_REGEN_CDX_ERROR:
      sprintf (message, _("File : %s\nError regenerating CDX fileord : %s\n"), czmil_error.file,
               strerror (czmil_error.system));
      break;

    case CZMIL_CWF_APPEND_ERROR:
      sprintf (message, _("File : %s\nAppending to pre-existing CWF file not allowed.\n"), czmil_error.file);
      break;

    case CZMIL_CDX_APPEND_ERROR:
      sprintf (message, _("File : %s\nAppending to pre-existing CDX file not allowed.\n"), czmil_error.file);
      break;

    case CZMIL_CXY_CDX_BUFFER_SIZE_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nBuffer sizes from the CDX and CXY files do not match.\n"),
               czmil_error.file, czmil_error.recnum);
      break;

    case CZMIL_CWF_CDX_BUFFER_SIZE_ERROR:
      sprintf (message, _("File : %s\nRecord : %d\nBuffer sizes from the CDX and CWF files do not match.\n"),
               czmil_error.file, czmil_error.recnum);
      break;
    }

  return (message);
}



/********************************************************************************************/
/*!

 - Function:    czmil_perror

 - Purpose:     Prints (to stderr) the latest error messages.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - void

 - Returns:
                - void

*********************************************************************************************/

CZMIL_DLL void czmil_perror ()
{
  fprintf (stderr, czmil_strerror ());
  fflush (stderr);
}



/********************************************************************************************/
/*!

 - Function:    czmil_get_version

 - Purpose:     Returns the CZMIL library version string

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - void

 - Returns:
                - version string

*********************************************************************************************/

CZMIL_DLL NV_CHAR *czmil_get_version ()
{
  return (CZMIL_VERSION);
}



/********************************************************************************************/
/*!

 - Function:    czmil_dump_cxy_record

 - Purpose:     Print the CZMIL CXY record to stdout.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - record         =    The CZMIL CXY record

 - Returns:
                - void

*********************************************************************************************/

CZMIL_DLL void czmil_dump_cxy_record (CZMIL_CXY_Data record)
{
  NV_INT32        i, j, year, day, hour, minute, month, mday;
  NV_FLOAT32      second;


  czmil_cvtime (record.channel[0].timestamp, &year, &day, &hour, &minute, &second);
  czmil_jday2mday (year, day, &month, &mday);
  month++;

  printf (N_("\n******************************************************************\n"));
  printf (_("timestamp : "));
  printf (NV_INT64_SPECIFIER, record.channel[0].timestamp);
  printf (_("    %d-%02d-%02d (%d) %02d:%02d:%05.2f\n"), year + 1900, month, mday, day, hour, minute, second);

  for (i = 0 ; i < 9 ; i++)
    {
      printf (_("channel[%d] latitude : %f\n"), i, record.channel[i].latitude);
      printf (_("channel[%d] longitude : %f\n"), i, record.channel[i].longitude);
      for (j = 0 ; j < 4 ; j++) printf (_("channel[%d] return elevation[%d] : %f\n"), i, j,
                                        record.channel[i].return_elevation[j]);
      printf (_("channel[%d] waveform class : %f\n"), i, record.channel[i].waveform_class);
    }

  fflush (stdout);
}



/********************************************************************************************/
/*!

 - Function:    czmil_dump_cwf_record

 - Purpose:     Print the CZMIL CWF record to stdout.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - record         =    The CZMIL CWF record

 - Returns:
                - void

*********************************************************************************************/

CZMIL_DLL void czmil_dump_cwf_record (NV_INT32 hnd, CZMIL_CWF_Data record)
{
  NV_INT32        i, j, year, day, hour, minute, month, mday;
  NV_FLOAT32      second;
  NV_INT32        loop = czmilh[hnd].cwf_czmil_max_packets * 64;


  czmil_cvtime (record.timestamp, &year, &day, &hour, &minute, &second);
  czmil_jday2mday (year, day, &month, &mday);
  month++;

  printf (N_("\n******************************************************************\n"));
  printf (_("timestamp : "));
  printf (NV_INT64_SPECIFIER, record.timestamp);
  printf (_("    %d-%02d-%02d (%d) %02d:%02d:%05.2f\n"), year + 1900, month, mday, day, hour, minute, second);

  printf (_("scan angle : %f\n"), record.scan_angle);


  printf (_("\n********************** Indexes ***********************************\n"));

  for (i = 0 ; i < 9 ; i++)
    {
      printf (_("\n********************* Channel[%d] *********************************\n"), i);

      for (j = 0 ; j < czmilh[hnd].cwf_czmil_max_packets ; j++) printf (N_("%02d - %d\n"), j, record.channel_ndx[i][j]);
    }


  printf (_("\n********************* Waveforms **********************************\n"));

  for (i = 0 ; i < 9 ; i++)
    {
      printf (_("\n********************* Channel[%d] *********************************\n"), i);

      for (j = 0 ; j < loop ; j += 8) printf (N_("%03d - %05d %05d %05d %05d %05d %05d %05d %05d\n"), j,
                                              record.channel[i][j], record.channel[i][j + 1], record.channel[i][j + 2],
                                              record.channel[i][j + 3], record.channel[i][j + 4],
                                              record.channel[i][j + 5], record.channel[i][j + 6],
                                              record.channel[i][j + 7]);
    }

  fflush (stdout);
}



/********************************************************************************************/
/*!

 - Function:    czmil_dump_cdx_record

 - Purpose:     Print the CZMIL CDX record to stdout.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - record         =    The CZMIL CDX record

 - Returns:
                - void

*********************************************************************************************/

CZMIL_DLL void czmil_dump_cdx_record (CZMIL_CDX_Data record)
{
  printf (N_("\n******************************************************************\n"));
  printf (_("CXY record address : "NV_INT64_SPECIFIER"\n"), record.cxy_address);
  printf (_("CWF record address : "NV_INT64_SPECIFIER"\n"), record.cwf_address);
  printf (_("CXY buffer size : %d\n"), record.cxy_buffer_size);
  printf (_("CWF buffer size : %d\n"), record.cwf_buffer_size);

  fflush (stdout);
}



/********************************************************************************************/
/*!

 - Function:    czmil_cvtime

 - Purpose:     Convert from CZMIL timestamp to year, day of year, hour, minute, second.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - timestamp      =    Microseconds from epoch (Jan. 1, 1970)
                - year           =    4 digit year - 1900
                - jday           =    day of year
                - hour           =    hour of day
                - minute         =    minute of hour
                - second         =    seconds of minute

 - Returns:
                - void

 - Caveats:     The localtime function will return the year as a 2 digit year (offset from 1900).

*********************************************************************************************/
 
CZMIL_DLL void czmil_cvtime (NV_INT64 micro_sec, NV_INT32 *year, NV_INT32 *jday, NV_INT32 *hour, NV_INT32 *minute, NV_FLOAT32 *second)
{
  static NV_INT32      tz_set = 0;
  struct tm            time_struct, *time_ptr = &time_struct;
  time_t               tv_sec;
  NV_INT32             msec;


  if (!tz_set)
    {
#ifdef NVWIN3X
  #ifdef __MINGW64__
      putenv("TZ=GMT");
      tzset();
  #else
      _putenv("TZ=GMT");
      _tzset();
  #endif
#else
      putenv("TZ=GMT");
      tzset();
#endif
      tz_set = 1;
    }
    

  tv_sec = micro_sec / 1000000;
  time_ptr = localtime (&tv_sec);

  msec = micro_sec % 1000000;

  *year = (short) time_ptr->tm_year;
  *jday = (short) time_ptr->tm_yday + 1;
  *hour = (short) time_ptr->tm_hour;
  *minute = (short) time_ptr->tm_min;
  *second = (NV_FLOAT32) time_ptr->tm_sec + (NV_FLOAT32) ((NV_FLOAT64) msec / 1000000.);
}



/********************************************************************************************/
/*!

 - Function:    czmil_inv_cvtime

 - Purpose:     Convert from year, day of year, hour, minute, second to microseconds from
                01-01-1970.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - year           =    4 digit year - 1900
                - jday           =    day of year
                - hour           =    hour of day
                - minute         =    minute of hour
                - second         =    seconds of minute
                - timestamp      =    Microseconds from epoch (Jan. 1, 1970)

 - Returns:
                - void

 - Caveats:     The mktime function takes a 2 digit year (offset from 1900).

*********************************************************************************************/
 
CZMIL_DLL void czmil_inv_cvtime (NV_INT32 year, NV_INT32 jday, NV_INT32 hour, NV_INT32 min, NV_FLOAT32 sec, NV_INT64 *timestamp)
{
  struct tm                    tm;
  static NV_INT32              tz_set = 0;
  time_t                       tv_sec;
  long                         tv_nsec;


  tm.tm_year = year;

  czmil_jday2mday (year, jday, &tm.tm_mon, &tm.tm_mday);

  tm.tm_hour = hour;
  tm.tm_min = min;
  tm.tm_sec = (NV_INT32) sec;
  tm.tm_isdst = 0;

  if (!tz_set)
    {
#ifdef NVWIN3X
  #ifdef __MINGW64__
      putenv ("TZ=GMT");
      tzset ();
  #else
      _putenv ("TZ=GMT");
      _tzset ();
  #endif
#else
      putenv ("TZ=GMT");
      tzset ();
#endif
      tz_set = 1;
    }

  tv_sec = mktime (&tm);
  tv_nsec = (long)(fmod ((double) sec, 1.0) * 1.0e9);

  *timestamp = (NV_INT64) tv_sec * 1000000L + NINT64 ((NV_FLOAT64) tv_nsec / 1000.0L);
}



/********************************************************************************************/
/*!

 - Function:    czmil_set_status

 - Purpose:     Sets the value of a status bit in the status field of a CZMIL CXY record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/30/11

 - Arguments:
                - status         =    The status record in which to set/unset the flag
                - status_flag    =    The CZMIL status flag or combination of flags (CZMIL_MANUALLY_INVAL,
                                      CZMIL_FILTER_INVAL, CZMIL_SUSPECT, etc.) to be set or
                                      unset in the status record.
                - flag           =    Set to 1 or 0 based on this flag

 - Returns:
                - void

*********************************************************************************************/

CZMIL_DLL void czmil_set_status (NV_U_INT16 *status, NV_U_INT16 status_flag, NV_BOOL flag)
{
  if (flag)
    {
      *status |= status_flag;
    }
  else
    {
      *status &= ~status_flag;
    }
}



/********************************************************************************************/
/*!

 - Function:    czmil_get_status

 - Purpose:     Gets the status of a status bit in a CZMIL CXY record.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/30/11

 - Arguments:
                - record         =    The CZMIL CXY record
                - status         =    The CZMIL status flag (CZMIL_MANUALLY_INVAL,
                                      CZMIL_FILTER_INVAL, CZMIL_SUSPECT, etc.)

 - Returns:
                - Returned status (anything other than zero means that the flag was set).

 - Caveats:     If you use a combination of flags (like CZMIL_SUSPECT & CZMIL_SUSPECT) a value
                of NVTrue will be returned if ANY of the flags are set.  This is actually
                useful for things like CZMIL_INVAL which is a combination of CZMIL_MANUALLY_INVAL
                and CZMIL_FILTER_INVAL.

*********************************************************************************************/

CZMIL_DLL NV_U_INT16 czmil_get_status (NV_U_INT16 status, NV_U_INT16 status_flag)
{
  return (status & status_flag);
}



/********************************************************************************************/
/*!

 - Function:    czmil_jday2mday

 - Purpose:     Convert from day of year to month and day.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/30/11

 - Arguments:
                - year           =    4 digit year or offset from 1900
                - jday           =    day of year
                - mon            =    month
                - mday           =    day of month

 - Returns:
                - void

 - Caveats:     The returned month value will start at 0 for January.

*********************************************************************************************/
 
CZMIL_DLL void czmil_jday2mday (NV_INT32 year, NV_INT32 jday, NV_INT32 *mon, NV_INT32 *mday)
{
  NV_INT32 l_year;


  /*  Months start at zero, days at 1 (go figure).  */

  static NV_INT32              months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


  l_year = year;

  if (year < 1899) l_year += 1900;


  /*  If the year is evenly divisible by 4 but not by 100, or it's evenly divisible by 400, this is a leap year.  */

  if ((!(l_year % 4) && (l_year % 100)) || !(l_year % 400))
    {
      months[1] = 29;
    }
  else
    {
      months[1] = 28;
    }

  *mday = jday;
  for (*mon = 0 ; *mon < 12 ; (*mon)++)
    {
      if (*mday - months[*mon] <= 0) break;

      *mday -= months[*mon];
    }
}
