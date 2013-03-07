
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



#include "chrtr2.h"
#include "chrtr2_internals.h"
#include "chrtr2_version.h"


/*  This is where we'll store the headers of all open CHRTR2 files (in addition to a bunch of other things, see above).  */

static INTERNAL_CHRTR2_STRUCT chrtr2h[CHRTR2_MAX_FILES];


/*  Startup flag used by either chrtr2_create_file or chrtr2_open_file to initialize the internal struct array and
    set the SIGINT handler.  */

static NV_BOOL first = NVTrue;


static CHRTR2_ERROR_STRUCT chrtr2_error;


/*  Insert a bunch of static utility functions that really don't need to live in this file.  */

#include "chrtr2_functions.h"


/********************************************************************************************/
/*!

 - Function:    define_record_fields

 - Purpose:     Computes the size and position, in bits, of each point record based on the
                information in the header of the CHRTR2 file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - creating       =    NVTrue if we are creating a new file

 - Returns:
                - void

 - Caveats:     In the future, when adding fields to the point record structure, add new
                fields AFTER the Z1 separation field.  As long as we don't change the order 
                of the fields we should be able to provide "forward compatibility" since
                the record size is stored in the file header.  That is, suppose we added
                two new fields after the Z1 separation field and changed the version to 3.0.
                A 2.0 implementation of the library would be able to read the file because
                the point record size it would read is based on the [RECORD SIZE] field of
                the header.  Of course, the old library would not unpack the two new fields
                but it should still function normally in relation to the rest of the (2.0
                defined) fields.

*********************************************************************************************/

static void define_record_fields (NV_INT32 hnd, NV_BOOL creating)
{
  NV_FLOAT64 range;
  NV_INT32 total_bits;


  /*  [TV SEC BITS] (chrtr2h[hnd].tv_sec_bits) is either read in chrtr2_open_file or hardwired in chrtr2_create_file.  */

  chrtr2h[hnd].z_pos = 0;


  /*  Z  -  Note that we are adding 1 to the range in order to store our NULL value as one more than max.  */

  range = (NV_FLOAT64) (((chrtr2h[hnd].header.max_z - chrtr2h[hnd].header.min_z) + 1.0) * chrtr2h[hnd].header.z_scale);
  chrtr2h[hnd].z_bits = NINT (log10 (range) / LOG2 + 0.5);


  /*  Horizontal uncertainty  */

  chrtr2h[hnd].h_uncert_pos = chrtr2h[hnd].z_pos + chrtr2h[hnd].z_bits;
  if (chrtr2h[hnd].header.horizontal_uncertainty_scale > 0.0)
    {
      range = (NV_FLOAT64) ((chrtr2h[hnd].header.max_horizontal_uncertainty - chrtr2h[hnd].header.min_horizontal_uncertainty) *
                            chrtr2h[hnd].header.horizontal_uncertainty_scale);
      chrtr2h[hnd].h_uncert_bits = NINT (log10 (range) / LOG2 + 0.5);
    }
  else
    {
      chrtr2h[hnd].h_uncert_bits = 0;
    }


  /*  Vertical uncertainty  */

  chrtr2h[hnd].v_uncert_pos = chrtr2h[hnd].h_uncert_pos + chrtr2h[hnd].h_uncert_bits;
  if (chrtr2h[hnd].header.vertical_uncertainty_scale > 0.0)
    {
      range = (NV_FLOAT64) ((chrtr2h[hnd].header.max_vertical_uncertainty - chrtr2h[hnd].header.min_vertical_uncertainty) *
                            chrtr2h[hnd].header.vertical_uncertainty_scale);
      chrtr2h[hnd].v_uncert_bits = NINT (log10 (range) / LOG2 + 0.5);
    }
  else
    {
      chrtr2h[hnd].v_uncert_bits = 0;
    }


  /*  [STATUS BITS] (chrtr2h[hnd].status_bits) is either read in chrtr2_open_file or hardwired in chrtr2_create_file.  */

  chrtr2h[hnd].status_pos = chrtr2h[hnd].v_uncert_pos + chrtr2h[hnd].v_uncert_bits;

 
  /*  Total uncertainty  */

  chrtr2h[hnd].uncert_pos = chrtr2h[hnd].status_pos + chrtr2h[hnd].status_bits;
  if (chrtr2h[hnd].header.uncertainty_scale > 0.0)
    {
      range = (NV_FLOAT64) ((chrtr2h[hnd].header.max_uncertainty - chrtr2h[hnd].header.min_uncertainty) * chrtr2h[hnd].header.uncertainty_scale);
      chrtr2h[hnd].uncert_bits = NINT (log10 (range) / LOG2 + 0.5);
    }
  else
    {
      chrtr2h[hnd].uncert_bits = 0;
    }


  /*  Number of points  */

  chrtr2h[hnd].num_pos = chrtr2h[hnd].uncert_pos + chrtr2h[hnd].uncert_bits;
  if (chrtr2h[hnd].header.max_number_of_points)
    {
      chrtr2h[hnd].num_bits = NINT (log10 (chrtr2h[hnd].header.max_number_of_points) / LOG2 + 0.5);
    }
  else
    {
      chrtr2h[hnd].num_bits = 0;
    }


  /*  Datum/MSL separation  */

  chrtr2h[hnd].z0_pos = chrtr2h[hnd].num_pos + chrtr2h[hnd].num_bits;
  if (chrtr2h[hnd].header.z0_scale > 0.0)
    {
      range = (NV_FLOAT64) ((chrtr2h[hnd].header.max_z0 - chrtr2h[hnd].header.min_z0) * chrtr2h[hnd].header.z0_scale);
      chrtr2h[hnd].z0_bits = NINT (log10 (range) / LOG2 + 0.5);
    }
  else
    {
      chrtr2h[hnd].z0_bits = 0;
    }


  /*  MSL/ellipsoid separation  */

  chrtr2h[hnd].z1_pos = chrtr2h[hnd].z0_pos + chrtr2h[hnd].z0_bits;
  if (chrtr2h[hnd].header.z1_scale > 0.0)
    {
      range = (NV_FLOAT64) ((chrtr2h[hnd].header.max_z1 - chrtr2h[hnd].header.min_z1) * chrtr2h[hnd].header.z1_scale);
      chrtr2h[hnd].z1_bits = NINT (log10 (range) / LOG2 + 0.5);
    }
  else
    {
      chrtr2h[hnd].z1_bits = 0;
    }


  /*  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  ADD NEW FIELDS AFTER THIS POINT  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  */


  /*  If we are creating a new file we need to compute the record size in bytes, otherwise it will
      be read from the header.  This allows us to be sort of "forward compatible".  */

  if (creating)
    {
      /*  Don't forget to change the computation of the total bits to use the correct _pos and _bits settings
          in the following line (from the newly added last field) !!!!!!!!!!!!!!!!!!!!!!  */

      total_bits = chrtr2h[hnd].z1_pos + chrtr2h[hnd].z1_bits;

      chrtr2h[hnd].record_size = (NV_FLOAT64) total_bits / 8.0;
      if (total_bits % 8) chrtr2h[hnd].record_size++;
    }
}



/********************************************************************************************/
/*!

 - Function:    pack_record

 - Purpose:     Pack the bit packed CHRTR2 record into the byte buffer.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - buffer         =    Unsigned character buffer
                - chrtr2_record  =    The CHRTR2 point record to pack in to the buffer

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_VALUE_OUT_OF_RANGE_ERROR

*********************************************************************************************/

static NV_INT32 pack_record (NV_INT32 hnd, NV_U_BYTE *buffer, CHRTR2_RECORD chrtr2_record)
{
  NV_INT32 value32;


  /*  Z.  */

  if (chrtr2_record.z < chrtr2h[hnd].header.min_z || chrtr2_record.z > chrtr2h[hnd].header.max_z + 1.0)
    {
      chrtr2_error.system = 0;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      sprintf (chrtr2_error.info, _("Z value %f out of range %f to %f"), chrtr2_record.z, chrtr2h[hnd].header.min_z, chrtr2h[hnd].header.max_z);
      return (chrtr2_error.chrtr2 = CHRTR2_VALUE_OUT_OF_RANGE_ERROR);
    }

  value32 = NINT ((chrtr2_record.z - chrtr2h[hnd].header.min_z) * chrtr2h[hnd].header.z_scale);
  chrtr2_bit_pack (buffer, chrtr2h[hnd].z_pos, chrtr2h[hnd].z_bits, value32);


  /*  Horizontal uncertainty  */

  if (chrtr2h[hnd].h_uncert_bits)
    {
      if (chrtr2_record.horizontal_uncertainty < chrtr2h[hnd].header.min_horizontal_uncertainty ||
          chrtr2_record.horizontal_uncertainty > chrtr2h[hnd].header.max_horizontal_uncertainty)
        {
          chrtr2_error.system = 0;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          sprintf (chrtr2_error.info, _("Horizontal uncertainty value %f out of range %f to %f"), chrtr2_record.horizontal_uncertainty,
                   chrtr2h[hnd].header.min_horizontal_uncertainty, chrtr2h[hnd].header.max_horizontal_uncertainty);
          return (chrtr2_error.chrtr2 = CHRTR2_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((chrtr2_record.horizontal_uncertainty - chrtr2h[hnd].header.min_horizontal_uncertainty) *
                      chrtr2h[hnd].header.horizontal_uncertainty_scale);
      chrtr2_bit_pack (buffer, chrtr2h[hnd].h_uncert_pos, chrtr2h[hnd].h_uncert_bits, value32);
    }


  /*  Vertical uncertainty  */

  if (chrtr2h[hnd].v_uncert_bits)
    {
      if (chrtr2_record.vertical_uncertainty < chrtr2h[hnd].header.min_vertical_uncertainty ||
          chrtr2_record.vertical_uncertainty > chrtr2h[hnd].header.max_vertical_uncertainty)
        {
          chrtr2_error.system = 0;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          sprintf (chrtr2_error.info, _("Vertical uncertainty value %f out of range %f to %f"), chrtr2_record.vertical_uncertainty,
                   chrtr2h[hnd].header.min_vertical_uncertainty, chrtr2h[hnd].header.max_vertical_uncertainty);
          return (chrtr2_error.chrtr2 = CHRTR2_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((chrtr2_record.vertical_uncertainty - chrtr2h[hnd].header.min_vertical_uncertainty) *
                      chrtr2h[hnd].header.vertical_uncertainty_scale);
      chrtr2_bit_pack (buffer, chrtr2h[hnd].v_uncert_pos, chrtr2h[hnd].v_uncert_bits, value32);
    }


  /*  Status  */

  if (chrtr2_record.status > chrtr2h[hnd].max_status)
    {
      chrtr2_error.system = 0;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      sprintf (chrtr2_error.info, _("Status value %d out of range 0 to %d"), chrtr2_record.status, chrtr2h[hnd].max_status);
      return (chrtr2_error.chrtr2 = CHRTR2_VALUE_OUT_OF_RANGE_ERROR);
    }
  chrtr2_bit_pack (buffer, chrtr2h[hnd].status_pos, chrtr2h[hnd].status_bits, chrtr2_record.status);


  /*  Total uncertainty  */

  if (chrtr2h[hnd].uncert_bits)
    {
      if (chrtr2_record.uncertainty < chrtr2h[hnd].header.min_uncertainty || chrtr2_record.uncertainty > chrtr2h[hnd].header.max_uncertainty)
        {
          chrtr2_error.system = 0;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          sprintf (chrtr2_error.info, _("Uncertainty value %f out of range %f to %f"), chrtr2_record.uncertainty,
                   chrtr2h[hnd].header.min_uncertainty, chrtr2h[hnd].header.max_uncertainty);
          return (chrtr2_error.chrtr2 = CHRTR2_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((chrtr2_record.uncertainty - chrtr2h[hnd].header.min_uncertainty) * chrtr2h[hnd].header.uncertainty_scale);
      chrtr2_bit_pack (buffer, chrtr2h[hnd].uncert_pos, chrtr2h[hnd].uncert_bits, value32);
    }


  /*  Number of points  */

  if (chrtr2h[hnd].num_bits)
    {
      if (chrtr2_record.number_of_points > chrtr2h[hnd].header.max_number_of_points)
        {
          chrtr2_error.system = 0;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          sprintf (chrtr2_error.info, _("Number of points value %d out of range 0 to %d"), chrtr2_record.number_of_points,
                   chrtr2h[hnd].header.max_number_of_points);
          return (chrtr2_error.chrtr2 = CHRTR2_VALUE_OUT_OF_RANGE_ERROR);
        }
      chrtr2_bit_pack (buffer, chrtr2h[hnd].num_pos, chrtr2h[hnd].num_bits, chrtr2_record.number_of_points);
    }


  /*  Datum/MSL separation  */

  if (chrtr2h[hnd].z0_bits)
    {
      if (chrtr2_record.z0 < chrtr2h[hnd].header.min_z0 || chrtr2_record.z0 > chrtr2h[hnd].header.max_z0)
        {
          chrtr2_error.system = 0;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          sprintf (chrtr2_error.info, _("Z0 separation value %f out of range %f to %f"), chrtr2_record.z0, chrtr2h[hnd].header.min_z0,
                   chrtr2h[hnd].header.max_z0);
          return (chrtr2_error.chrtr2 = CHRTR2_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((chrtr2_record.z0 - chrtr2h[hnd].header.min_z0) * chrtr2h[hnd].header.z0_scale);
      chrtr2_bit_pack (buffer, chrtr2h[hnd].z0_pos, chrtr2h[hnd].z0_bits, value32);
    }


  /*  MSL/ellipsoid separation  */

  if (chrtr2h[hnd].z1_bits)
    {
      if (chrtr2_record.z1 < chrtr2h[hnd].header.min_z1 || chrtr2_record.z1 > chrtr2h[hnd].header.max_z1)
        {
          chrtr2_error.system = 0;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          sprintf (chrtr2_error.info, _("Z1 separation value %f out of range %f to %f"), chrtr2_record.z1, chrtr2h[hnd].header.min_z1,
                   chrtr2h[hnd].header.max_z1);
          return (chrtr2_error.chrtr2 = CHRTR2_VALUE_OUT_OF_RANGE_ERROR);
        }

      value32 = NINT ((chrtr2_record.z1 - chrtr2h[hnd].header.min_z1) * chrtr2h[hnd].header.z1_scale);
      chrtr2_bit_pack (buffer, chrtr2h[hnd].z1_pos, chrtr2h[hnd].z1_bits, value32);
    }


  /*  It doesn't really matter, since the chrtr2_bit_pack functions will jump around in the buffer, but
      it would be nice to add new fields after this point.  The important part is in the define_record_fields function.  */


  chrtr2_error.system = 0;
  return (CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    unpack_record

 - Purpose:     Unpack the bit packed CHRTR2 record from the byte buffer.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - buffer         =    Unsigned character buffer
                - chrtr2_record     =    The CHRTR2 point record to receive the data

 - Returns:
                - void

*********************************************************************************************/

static void unpack_record (NV_INT32 hnd, NV_U_BYTE *buffer, CHRTR2_RECORD *chrtr2_record)
{
  NV_INT32 value32;


  memset (chrtr2_record, 0, sizeof (CHRTR2_RECORD));


  /*  Z  */

  value32 = chrtr2_bit_unpack (buffer, chrtr2h[hnd].z_pos, chrtr2h[hnd].z_bits);
  chrtr2_record->z = ((NV_FLOAT32) value32 / chrtr2h[hnd].header.z_scale) + chrtr2h[hnd].header.min_z;


  /*  Horizontal uncertainty  */

  if (chrtr2h[hnd].h_uncert_bits)
    {
      value32 = chrtr2_bit_unpack (buffer, chrtr2h[hnd].h_uncert_pos, chrtr2h[hnd].h_uncert_bits);
      chrtr2_record->horizontal_uncertainty = ((NV_FLOAT32) value32 / chrtr2h[hnd].header.horizontal_uncertainty_scale) +
        chrtr2h[hnd].header.min_horizontal_uncertainty;
    }


  /*  Vertical uncertainty  */

  if (chrtr2h[hnd].v_uncert_bits)
    {
      value32 = chrtr2_bit_unpack (buffer, chrtr2h[hnd].v_uncert_pos, chrtr2h[hnd].v_uncert_bits);
      chrtr2_record->vertical_uncertainty = ((NV_FLOAT32) value32 / chrtr2h[hnd].header.vertical_uncertainty_scale) +
        chrtr2h[hnd].header.min_vertical_uncertainty;
    }


  /*  Status  */

  chrtr2_record->status = chrtr2_bit_unpack (buffer, chrtr2h[hnd].status_pos, chrtr2h[hnd].status_bits);


  /*  Total uncertainty  */

  if (chrtr2h[hnd].uncert_bits)
    {
      value32 = chrtr2_bit_unpack (buffer, chrtr2h[hnd].uncert_pos, chrtr2h[hnd].uncert_bits);
      chrtr2_record->uncertainty = ((NV_FLOAT32) value32 / chrtr2h[hnd].header.uncertainty_scale) + chrtr2h[hnd].header.min_uncertainty;
    }


  /*  Number of points  */

  if (chrtr2h[hnd].num_bits) chrtr2_record->number_of_points = chrtr2_bit_unpack (buffer, chrtr2h[hnd].num_pos, chrtr2h[hnd].num_bits);


  /*  Datum/MSL separation  */

  if (chrtr2h[hnd].z0_bits)
    {
      value32 = chrtr2_bit_unpack (buffer, chrtr2h[hnd].z0_pos, chrtr2h[hnd].z0_bits);
      chrtr2_record->z0 = ((NV_FLOAT32) value32 / chrtr2h[hnd].header.z0_scale) + chrtr2h[hnd].header.min_z0;
    }


  /*  MSL/ellipsoid separation  */

  if (chrtr2h[hnd].z1_bits)
    {
      value32 = chrtr2_bit_unpack (buffer, chrtr2h[hnd].z1_pos, chrtr2h[hnd].z1_bits);
      chrtr2_record->z1 = ((NV_FLOAT32) value32 / chrtr2h[hnd].header.z1_scale) + chrtr2h[hnd].header.min_z1;
    }


  /*  It doesn't really matter, since the chrtr2_bit_unpack functions will jump around in the buffer, but
      it would be nice to add new fields after this point.  The important part is in the
      define_record_fields function.  */
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_write_header

 - Purpose:     Write the chrtr2_header to the CHRTR2 file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_HEADER_WRITE_FSEEK_ERROR
                - CHRTR2_HEADER_WRITE_ERROR

*********************************************************************************************/

static NV_INT32 chrtr2_write_header (NV_INT32 hnd)
{
  NV_CHAR space = ' ';
  NV_INT32 i, size, year, jday, hour, minute, month, day;
  NV_FLOAT32 second;


  if (fseeko64 (chrtr2h[hnd].fp, 0LL, SEEK_SET) < 0)
    {
      chrtr2_error.system = errno;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_HEADER_WRITE_FSEEK_ERROR);
    }



  /*  If the user flags weren't defined, set the default names.  */

  if (!(strlen (chrtr2h[hnd].header.user_flag_name[0]))) strcpy (chrtr2h[hnd].header.user_flag_name[0], "CHRTR2_USER_01");
  if (!(strlen (chrtr2h[hnd].header.user_flag_name[1]))) strcpy (chrtr2h[hnd].header.user_flag_name[1], "CHRTR2_USER_02");
  if (!(strlen (chrtr2h[hnd].header.user_flag_name[2]))) strcpy (chrtr2h[hnd].header.user_flag_name[2], "CHRTR2_USER_03");
  if (!(strlen (chrtr2h[hnd].header.user_flag_name[3]))) strcpy (chrtr2h[hnd].header.user_flag_name[3], "CHRTR2_USER_04");


  fprintf (chrtr2h[hnd].fp, N_("[VERSION] = %s\n"), CHRTR2_VERSION);

  chrtr2_cvtime (chrtr2h[hnd].header.creation_tv_sec, 0.0, &year, &jday, &hour, &minute, &second);
  chrtr2_jday2mday (year, jday, &month, &day);
  month++;

  fprintf (chrtr2h[hnd].fp, N_("[CREATION YEAR] = %d\n"), year + 1900);
  fprintf (chrtr2h[hnd].fp, N_("[CREATION MONTH] = %02d\n"), month);
  fprintf (chrtr2h[hnd].fp, N_("[CREATION DAY] = %02d\n"), day);
  fprintf (chrtr2h[hnd].fp, N_("[CREATION DAY OF YEAR] = %03d\n"), jday);
  fprintf (chrtr2h[hnd].fp, N_("[CREATION HOUR] = %02d\n"), hour);
  fprintf (chrtr2h[hnd].fp, N_("[CREATION MINUTE] = %02d\n"), minute);
  fprintf (chrtr2h[hnd].fp, N_("[CREATION SECOND] = %5.2f\n"), second);
  if (strlen (chrtr2h[hnd].header.creation_software) > 2) fprintf (chrtr2h[hnd].fp, N_("[CREATION SOFTWARE] = %s\n"),
                                                                   chrtr2h[hnd].header.creation_software);


  chrtr2_cvtime (chrtr2h[hnd].header.modification_tv_sec, 0.0, &year, &jday, &hour, &minute, &second);
  chrtr2_jday2mday (year, jday, &month, &day);
  month++;

  fprintf (chrtr2h[hnd].fp, N_("[MODIFICATION YEAR] = %d\n"), year + 1900);
  fprintf (chrtr2h[hnd].fp, N_("[MODIFICATION MONTH] = %02d\n"), month);
  fprintf (chrtr2h[hnd].fp, N_("[MODIFICATION DAY] = %02d\n"), day);
  fprintf (chrtr2h[hnd].fp, N_("[MODIFICATION DAY OF YEAR] = %03d\n"), jday);
  fprintf (chrtr2h[hnd].fp, N_("[MODIFICATION HOUR] = %02d\n"), hour);
  fprintf (chrtr2h[hnd].fp, N_("[MODIFICATION MINUTE] = %02d\n"), minute);
  fprintf (chrtr2h[hnd].fp, N_("[MODIFICATION SECOND] = %5.2f\n"), second);
  if (strlen (chrtr2h[hnd].header.modification_software) > 2) fprintf (chrtr2h[hnd].fp, N_("[MODIFICATION SOFTWARE] = %s\n"),
                                                                       chrtr2h[hnd].header.modification_software);

  if (strlen (chrtr2h[hnd].header.security_classification) > 2) fprintf (chrtr2h[hnd].fp, N_("[SECURITY CLASSIFICATION] = %s\n"),
                                                                         chrtr2h[hnd].header.security_classification);
  if (strlen (chrtr2h[hnd].header.distribution) > 2) fprintf (chrtr2h[hnd].fp, N_("{DISTRIBUTION = \n%s\n}\n"),
                                                              chrtr2h[hnd].header.distribution);
  if (strlen (chrtr2h[hnd].header.declassification) > 2) fprintf (chrtr2h[hnd].fp, N_("{DECLASSIFICATION = \n%s\n}\n"),
                                                                  chrtr2h[hnd].header.declassification);
  if (strlen (chrtr2h[hnd].header.class_just) > 2) fprintf (chrtr2h[hnd].fp, N_("{SECURITY CLASSIFICATION JUSTIFICATION = \n%s\n}\n"),
                                                            chrtr2h[hnd].header.class_just);
  if (strlen (chrtr2h[hnd].header.downgrade) > 2) fprintf (chrtr2h[hnd].fp, N_("{DOWNGRADE = \n%s\n}\n"),
                                                           chrtr2h[hnd].header.downgrade);

  for (i = 0 ; i < 5 ; i++)
    {
      if (strlen (chrtr2h[hnd].header.user_flag_name[i]) > 2) fprintf (chrtr2h[hnd].fp, N_("[CHRTR2 USER FLAG %02d NAME] = %s\n"),
                                                                       i + 1, chrtr2h[hnd].header.user_flag_name[i]);
    }

  if (strlen (chrtr2h[hnd].header.comments) > 2) fprintf (chrtr2h[hnd].fp, N_("{COMMENTS = \n%s\n}\n"), chrtr2h[hnd].header.comments);

  fprintf (chrtr2h[hnd].fp, N_("[GRID TYPE] = %d\n"), chrtr2h[hnd].header.grid_type);

  fprintf (chrtr2h[hnd].fp, N_("[MIN OBSERVED Z] = %.9f\n"), chrtr2h[hnd].header.min_observed_z);
  fprintf (chrtr2h[hnd].fp, N_("[MAX OBSERVED Z] = %.9f\n"), chrtr2h[hnd].header.max_observed_z);


  switch (chrtr2h[hnd].header.z_units)
    {
    case CHRTR2_METERS:
    default:
      fprintf (chrtr2h[hnd].fp, N_("[Z UNITS] = METERS\n"));
      break;

    case CHRTR2_FEET:
      fprintf (chrtr2h[hnd].fp, N_("[Z UNITS] = FEET\n"));
      break;

    case CHRTR2_FATHOMS:
      fprintf (chrtr2h[hnd].fp, N_("[Z UNITS] = FATHOMS\n"));
      break;
    }

  fprintf (chrtr2h[hnd].fp, N_("[MIN Z] = %.9f\n"), chrtr2h[hnd].header.min_z);
  fprintf (chrtr2h[hnd].fp, N_("[MAX Z] = %.9f\n"), chrtr2h[hnd].header.max_z);
  fprintf (chrtr2h[hnd].fp, N_("[Z SCALE] = %.9f\n"), chrtr2h[hnd].header.z_scale);


  fprintf (chrtr2h[hnd].fp, N_("[MBR WEST LONGITUDE] = %.11f\n"), chrtr2h[hnd].header.mbr.wlon);
  fprintf (chrtr2h[hnd].fp, N_("[MBR SOUTH LATITUDE] = %.11f\n"), chrtr2h[hnd].header.mbr.slat);
  fprintf (chrtr2h[hnd].fp, N_("[MBR EAST LONGITUDE] = %.11f\n"), chrtr2h[hnd].header.mbr.elon);
  fprintf (chrtr2h[hnd].fp, N_("[MBR NORTH LATITUDE] = %.11f\n"), chrtr2h[hnd].header.mbr.nlat);

  fprintf (chrtr2h[hnd].fp, N_("[WIDTH IN GRID CELLS] = %d\n"), chrtr2h[hnd].header.width);
  fprintf (chrtr2h[hnd].fp, N_("[HEIGHT IN GRID CELLS] = %d\n"), chrtr2h[hnd].header.height);

  fprintf (chrtr2h[hnd].fp, N_("[LONGITUDE BIN SIZE IN DEGREES] = %.11f\n"), chrtr2h[hnd].header.lon_grid_size_degrees);
  fprintf (chrtr2h[hnd].fp, N_("[LATITUDE BIN SIZE IN DEGREES] = %.11f\n"), chrtr2h[hnd].header.lat_grid_size_degrees);

  if (chrtr2h[hnd].h_uncert_bits)
    {
      fprintf (chrtr2h[hnd].fp, N_("[MIN HORIZONTAL UNCERTAINTY] = %.9f\n"), chrtr2h[hnd].header.min_horizontal_uncertainty);
      fprintf (chrtr2h[hnd].fp, N_("[MAX HORIZONTAL UNCERTAINTY] = %.9f\n"), chrtr2h[hnd].header.max_horizontal_uncertainty);
      fprintf (chrtr2h[hnd].fp, N_("[HORIZONTAL UNCERTAINTY SCALE] = %.9f\n"), chrtr2h[hnd].header.horizontal_uncertainty_scale);
    }

  if (chrtr2h[hnd].v_uncert_bits)
    {
      fprintf (chrtr2h[hnd].fp, N_("[MIN VERTICAL UNCERTAINTY] = %.9f\n"), chrtr2h[hnd].header.min_vertical_uncertainty);
      fprintf (chrtr2h[hnd].fp, N_("[MAX VERTICAL UNCERTAINTY] = %.9f\n"), chrtr2h[hnd].header.max_vertical_uncertainty);
      fprintf (chrtr2h[hnd].fp, N_("[VERTICAL UNCERTAINTY SCALE] = %.9f\n"), chrtr2h[hnd].header.vertical_uncertainty_scale);
    }

  fprintf (chrtr2h[hnd].fp, N_("[STATUS BITS] = %d\n"), chrtr2h[hnd].status_bits);

  if (chrtr2h[hnd].uncert_bits)
    {
      fprintf (chrtr2h[hnd].fp, N_("[MIN UNCERTAINTY] = %.9f\n"), chrtr2h[hnd].header.min_uncertainty);
      fprintf (chrtr2h[hnd].fp, N_("[MAX UNCERTAINTY] = %.9f\n"), chrtr2h[hnd].header.max_uncertainty);
      fprintf (chrtr2h[hnd].fp, N_("[UNCERTAINTY SCALE] = %.9f\n"), chrtr2h[hnd].header.uncertainty_scale);
      if (strlen (chrtr2h[hnd].header.uncertainty_name)) fprintf (chrtr2h[hnd].fp, N_("[UNCERTAINTY NAME] = %s\n"), chrtr2h[hnd].header.uncertainty_name);
    }

  if (chrtr2h[hnd].z0_bits)
    {
      fprintf (chrtr2h[hnd].fp, N_("[MIN DATUM/MSL (Z0) SEPARATION] = %.9f\n"), chrtr2h[hnd].header.min_z0);
      fprintf (chrtr2h[hnd].fp, N_("[MAX DATUM/MSL (Z0) SEPARATION] = %.9f\n"), chrtr2h[hnd].header.max_z0);
      fprintf (chrtr2h[hnd].fp, N_("[DATUM/MSL (Z0) SEPARATION SCALE] = %.9f\n"), chrtr2h[hnd].header.z0_scale);
      if (strlen (chrtr2h[hnd].header.z0_name)) fprintf (chrtr2h[hnd].fp, N_("[VERTICAL DATUM (Z0) NAME] = %s\n"), chrtr2h[hnd].header.z0_name);
    }

  if (chrtr2h[hnd].z1_bits)
    {
      fprintf (chrtr2h[hnd].fp, N_("[MIN MSL/ELLIPSOID (Z1) SEPARATION] = %.9f\n"), chrtr2h[hnd].header.min_z1);
      fprintf (chrtr2h[hnd].fp, N_("[MAX MSL/ELLIPSOID (Z1) SEPARATION] = %.9f\n"), chrtr2h[hnd].header.max_z1);
      fprintf (chrtr2h[hnd].fp, N_("[MSL/ELLIPSOID (Z1) SEPARATION SCALE] = %.9f\n"), chrtr2h[hnd].header.z1_scale);
    }

  if (chrtr2h[hnd].num_bits) fprintf (chrtr2h[hnd].fp, N_("[MAX NUMBER OF POINTS] = %d\n"), chrtr2h[hnd].header.max_number_of_points);

  chrtr2h[hnd].header_size = CHRTR2_HEADER_SIZE;
  fprintf (chrtr2h[hnd].fp, N_("[HEADER SIZE] = %d\n"), chrtr2h[hnd].header_size);
  fprintf (chrtr2h[hnd].fp, N_("[RECORD SIZE] = %d\n"), chrtr2h[hnd].record_size);

  fprintf (chrtr2h[hnd].fp, N_("[END OF HEADER]\n"));


  /*  Space fill the rest.  */

  size = chrtr2h[hnd].header_size - ftell (chrtr2h[hnd].fp);


  for (i = 0 ; i < size ; i++)
    {
      if (!fwrite (&space, 1, 1, chrtr2h[hnd].fp))
        {
          chrtr2_error.system = errno;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          return (chrtr2_error.chrtr2 = CHRTR2_HEADER_WRITE_ERROR);
        }
    }

  chrtr2_error.system = 0;
  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_create_file

 - Purpose:     Create a CHRTR2 file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - path           =    The CHRTR2 file path
                - chrtr2_header  =    CHRTR2_HEADER structure to be written to the file

 - Returns:     
                - The file handle (0 or positive)
                - CHRTR2_TOO_MANY_OPEN_FILES
                - CHRTR2_CREATE_ERROR
                - CHRTR2_HEADER_WRITE_FSEEK_ERROR
                - CHRTR2_HEADER_WRITE_ERROR
                - CHRTR2_SENSOR_POSITION_RANGE_ERROR
                - CHRTR2_SENSOR_ATTITUDE_RANGE_ERROR

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_create_file (const NV_CHAR *path, CHRTR2_HEADER *chrtr2_header)
{
  NV_INT32             i, hnd, bytes;
  NV_U_BYTE            *buffer = NULL;

    
  /*  The first time through we want to initialize (zero) the CHRTR2 handle array.  */

  if (first)
    {
      for (i = 0 ; i < CHRTR2_MAX_FILES ; i++) 
        {
          memset (&chrtr2h[i], 0, sizeof (INTERNAL_CHRTR2_STRUCT));
          chrtr2h[i].fp = NULL;
          chrtr2h[i].cov_map = NULL;
        }


      /*  Set up the SIGINT handler.  */

      signal (SIGINT, sigint_handler);


      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = CHRTR2_MAX_FILES;
  for (i = 0 ; i < CHRTR2_MAX_FILES ; i++)
    {
      if (chrtr2h[i].fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == CHRTR2_MAX_FILES) return (chrtr2_error.chrtr2 = CHRTR2_TOO_MANY_OPEN_FILES);


  /*  Hardwire the bit sizes for fixed fields.  */

  chrtr2h[hnd].status_bits = CHRTR2_STATUS_BITS;


  chrtr2h[hnd].max_status = NINT (pow (2.0, (NV_FLOAT64) chrtr2h[hnd].status_bits)) - 1;


  /*  Open the file and write the header.  */

  if ((chrtr2h[hnd].fp = fopen64 (path, "wb+")) == NULL)
    {
      chrtr2_error.system = errno;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_CREATE_ERROR);
    }


  /*  Save the file name for error messages.  */

  strcpy (chrtr2h[hnd].path, path);


  /*  Define the max_x and max_y values of the MBR based on the width/height and the x/y grid size.  */

  chrtr2_header->mbr.elon = chrtr2_header->mbr.wlon + (NV_FLOAT64) chrtr2_header->width * chrtr2_header->lon_grid_size_degrees;
  chrtr2_header->mbr.nlat = chrtr2_header->mbr.slat + (NV_FLOAT64) chrtr2_header->height * chrtr2_header->lat_grid_size_degrees;


  chrtr2h[hnd].header = *chrtr2_header;


  /*  Set to the default.  */

  chrtr2h[hnd].status_bits = CHRTR2_STATUS_BITS;


  /*  Define the bit fields and record size.  */

  define_record_fields (hnd, NVTrue);


  /*  Write the header.  */

  if (chrtr2_write_header (hnd) < 0) return (chrtr2_error.chrtr2);


  /*  Zero the file so that all cells will be set to CHRTR2_NULL (i.e. 0) in case someone wants to play sparse fill games
      with the file.  */

  bytes = chrtr2h[hnd].record_size * chrtr2h[hnd].header.width;
  buffer = (NV_U_BYTE *) calloc (bytes, sizeof (NV_U_BYTE));
  if (buffer == NULL)
    {
      perror ("Allocating memory in chrtr2_create_file");
      exit (-1);
    }

  for (i = 0 ; i < chrtr2h[hnd].header.height ; i++) fwrite (buffer, bytes, 1, chrtr2h[hnd].fp);

  free (buffer);


  chrtr2h[hnd].modified = NVTrue;
  chrtr2h[hnd].created = NVTrue;


  chrtr2_error.system = 0;
  return (hnd);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_open_file

 - Purpose:     Open a CHRTR2 file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - path           =    The CHRTR2 file path
                - chrtr2_header  =    CHRTR2_HEADER structure to be populated
                - mode           =    CHRTR2_UPDATE or CHRTR2_READ_ONLY

 - Returns:     
                - The file handle (0 or positive)
                - CHRTR2_TOO_MANY_OPEN_FILES
                - CHRTR2_OPEN_UPDATE_ERROR
                - CHRTR2_OPEN_READONLY_ERROR
                - CHRTR2_NOT_CHRTR2_FILE_ERROR

 - Caveats:     chrtr2_error.chrtr2 may be set to CHRTR2_NEWER_FILE_VERSION_WARNING if the file version
                is newer than the library version.  This shouldn't cause a problem but
                you may want to check it with chrtr2_get_errno () when you open a file so
                you can issue a warning.

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_open_file (const NV_CHAR *path, CHRTR2_HEADER *chrtr2_header, NV_INT32 mode)
{
  NV_INT32 i, hnd, year[4], jday[4], hour[4], minute[4];
  NV_FLOAT32 second[4], tmpf;
  NV_CHAR varin[8192], info[8192];
  long dummy_tv_nsec;


  /*  Due to the stupidity that is Microsoft I can't use %hhd in scanf statements so I have to play games
      to read a value in to an unsigned char.  */

  NV_INT16 tmpi16;


  /*  The first time through we want to initialize (zero) the chrtr2 handle array.  */

  if (first)
    {
      for (i = 0 ; i < CHRTR2_MAX_FILES ; i++) 
        {
          memset (&chrtr2h[i], 0, sizeof (INTERNAL_CHRTR2_STRUCT));
          chrtr2h[i].fp = NULL;
          chrtr2h[i].cov_map = NULL;
        }


      /*  Set up the SIGINT handler.  */

      signal (SIGINT, sigint_handler);


      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = CHRTR2_MAX_FILES;
  for (i = 0 ; i < CHRTR2_MAX_FILES ; i++)
    {
      if (chrtr2h[i].fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == CHRTR2_MAX_FILES) return (chrtr2_error.chrtr2 = CHRTR2_TOO_MANY_OPEN_FILES);


  /*  Internal structs are zeroed above and on close of file so we don't have to do it here.  */


  /*  Open the file and read the header.  */

  switch (mode)
    {
    case CHRTR2_UPDATE:
      if ((chrtr2h[hnd].fp = fopen64 (path, "rb+")) == NULL)
        {
          chrtr2_error.system = errno;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          return (chrtr2_error.chrtr2 = CHRTR2_OPEN_UPDATE_ERROR);
        }
      break;

    case CHRTR2_READONLY:
      if ((chrtr2h[hnd].fp = fopen64 (path, "rb")) == NULL)
        {
          chrtr2_error.system = errno;
          strcpy (chrtr2_error.file, chrtr2h[hnd].path);
          return (chrtr2_error.chrtr2 = CHRTR2_OPEN_READONLY_ERROR);
        }
      break;
    }


  chrtr2h[hnd].header.z_units = CHRTR2_METERS;
  for (i = 0 ; i < 4 ; i++) year[i] = 0;


  /*  Save the file name for error messages.  */

  strcpy (chrtr2h[hnd].path, path);


  /*  We want to try to read the first line (version info) with an fread in case we mistakenly asked to
      load a binary file.  If we try to use chrtr2_ngets to read a binary file and there are no line feeds in 
      the first sizeof (varin) characters we would segfault since fgets doesn't check for overrun.  */

  if (!fread (varin, 128, 1, chrtr2h[hnd].fp))
    {
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_NOT_CHRTR2_FILE_ERROR);
    }


  /*  Check for the CHRTR2 library string at the beginning of the file.  */

  if (!strstr (varin, N_("CHRTR2 library")))
    {
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_NOT_CHRTR2_FILE_ERROR);
    }


  /*  Rewind to the beginning of the file.  Yes, we'll read the version again but it doesn't matter.  */

  fseek (chrtr2h[hnd].fp, 0, SEEK_SET);


  /*  Note, we're using chrtr2_ngets instead of fgets since we really don't want the CR/LF in the strings.  */

  while (chrtr2_ngets (varin, sizeof (varin), chrtr2h[hnd].fp))
    {
      if (strstr (varin, N_("[END OF HEADER]"))) break;


      /*  Put everything to the right of the equals sign into 'info'.   */

      get_string (varin, info);


      /*  Read the version string and check the major version number against the library major version.  */

      if (strstr (varin, "[VERSION]"))
        {
          strcpy (chrtr2h[hnd].header.version, info);
          strcpy (info, strstr (varin, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          chrtr2h[hnd].major_version = (NV_INT16) tmpf;

          strcpy (info, strstr (CHRTR2_VERSION, N_("library V")));
          sscanf (info, N_("library V%f"), &tmpf);
          if (chrtr2h[hnd].major_version > (NV_INT16) tmpf)
            {
              strcpy (chrtr2_error.file, chrtr2h[hnd].path);
              chrtr2_error.chrtr2 = CHRTR2_NEWER_FILE_VERSION_WARNING;
            }
        }


      if (strstr (varin, N_("[CREATION YEAR]"))) sscanf (info, "%d", &year[0]);
      if (strstr (varin, N_("[CREATION DAY OF YEAR]"))) sscanf (info, "%d", &jday[0]);
      if (strstr (varin, N_("[CREATION HOUR]"))) sscanf (info, "%d", &hour[0]);
      if (strstr (varin, N_("[CREATION MINUTE]"))) sscanf (info, "%d", &minute[0]);
      if (strstr (varin, N_("[CREATION SECOND]"))) sscanf (info, "%f", &second[0]);
      if (strstr (varin, N_("[CREATION SOFTWARE]"))) strcpy (chrtr2h[hnd].header.creation_software, info);

      if (strstr (varin, N_("[MODIFICATION YEAR]"))) sscanf (info, "%d", &year[1]);
      if (strstr (varin, N_("[MODIFICATION DAY OF YEAR]"))) sscanf (info, "%d", &jday[1]);
      if (strstr (varin, N_("[MODIFICATION HOUR]"))) sscanf (info, "%d", &hour[1]);
      if (strstr (varin, N_("[MODIFICATION MINUTE]"))) sscanf (info, "%d", &minute[1]);
      if (strstr (varin, N_("[MODIFICATION SECOND]"))) sscanf (info, "%f", &second[1]);
      if (strstr (varin, N_("[MODIFICATION SOFTWARE]"))) strcpy (chrtr2h[hnd].header.modification_software, info);

      if (strstr (varin, N_("[SECURITY CLASSIFICATION]"))) strcpy (chrtr2h[hnd].header.security_classification, info);

      if (strstr (varin, N_("{DISTRIBUTION =")))
        {
          strcpy (chrtr2h[hnd].header.distribution, "");
          while (fgets (varin, sizeof (varin), chrtr2h[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (chrtr2h[hnd].header.distribution, varin);
            }
        }

      if (strstr (varin, N_("{DECLASSIFICATION =")))
        {
          strcpy (chrtr2h[hnd].header.declassification, "");
          while (fgets (varin, sizeof (varin), chrtr2h[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (chrtr2h[hnd].header.declassification, varin);
            }
        }

      if (strstr (varin, N_("{SECURITY CLASSIFICATION JUSTIFICATION =")))
        {
          strcpy (chrtr2h[hnd].header.class_just, "");
          while (fgets (varin, sizeof (varin), chrtr2h[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (chrtr2h[hnd].header.class_just, varin);
            }
        }


      if (strstr (varin, N_("{DOWNGRADE =")))
        {
          strcpy (chrtr2h[hnd].header.downgrade, "");
          while (fgets (varin, sizeof (varin), chrtr2h[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (chrtr2h[hnd].header.downgrade, varin);
            }
        }


      if (strstr (varin, N_("[SECURITY CLASSIFICATION]"))) strcpy (chrtr2h[hnd].header.security_classification, info);


      if (strstr (varin, N_("[CHRTR2 USER FLAG 01 NAME]"))) strcpy (chrtr2h[hnd].header.user_flag_name[0], info);
      if (strstr (varin, N_("[CHRTR2 USER FLAG 02 NAME]"))) strcpy (chrtr2h[hnd].header.user_flag_name[1], info);
      if (strstr (varin, N_("[CHRTR2 USER FLAG 03 NAME]"))) strcpy (chrtr2h[hnd].header.user_flag_name[2], info);
      if (strstr (varin, N_("[CHRTR2 USER FLAG 04 NAME]"))) strcpy (chrtr2h[hnd].header.user_flag_name[3], info);
      if (strstr (varin, N_("[CHRTR2 USER FLAG 05 NAME]"))) strcpy (chrtr2h[hnd].header.user_flag_name[4], info);

      if (strstr (varin, N_("{COMMENTS =")))
        {
          strcpy (chrtr2h[hnd].header.comments, "");
          while (fgets (varin, sizeof (varin), chrtr2h[hnd].fp))
            {
              if (varin[0] == '}') break;
              strcat (chrtr2h[hnd].header.comments, varin);
            }
        }

      if (strstr (varin, N_("[GRID TYPE]"))) sscanf (info, "%hd", &chrtr2h[hnd].header.grid_type);

      if (strstr (varin, N_("[MIN OBSERVED Z]"))) sscanf (info, "%f", &chrtr2h[hnd].header.min_observed_z);
      if (strstr (varin, N_("[MAX OBSERVED Z]"))) sscanf (info, "%f", &chrtr2h[hnd].header.max_observed_z);


      if (strstr (varin, N_("[Z UNITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          chrtr2h[hnd].header.z_units = (NV_U_BYTE) tmpi16;
        }

      if (strstr (varin, N_("[MIN Z]"))) sscanf (info, "%f", &chrtr2h[hnd].header.min_z);
      if (strstr (varin, N_("[MAX Z]"))) sscanf (info, "%f", &chrtr2h[hnd].header.max_z);
      if (strstr (varin, N_("[Z SCALE]"))) sscanf (info, "%f", &chrtr2h[hnd].header.z_scale);

      if (strstr (varin, N_("[MBR WEST LONGITUDE]"))) sscanf (info, "%lf", &chrtr2h[hnd].header.mbr.wlon);
      if (strstr (varin, N_("[MBR SOUTH LATITUDE]"))) sscanf (info, "%lf", &chrtr2h[hnd].header.mbr.slat);
      if (strstr (varin, N_("[MBR EAST LONGITUDE]"))) sscanf (info, "%lf", &chrtr2h[hnd].header.mbr.elon);
      if (strstr (varin, N_("[MBR NORTH LATITUDE]"))) sscanf (info, "%lf", &chrtr2h[hnd].header.mbr.nlat);

      if (strstr (varin, N_("[WIDTH IN GRID CELLS]"))) sscanf (info, "%d", &chrtr2h[hnd].header.width);
      if (strstr (varin, N_("[HEIGHT IN GRID CELLS]"))) sscanf (info, "%d", &chrtr2h[hnd].header.height);

      if (strstr (varin, N_("[LONGITUDE BIN SIZE IN DEGREES]"))) sscanf (info, "%lf", &chrtr2h[hnd].header.lon_grid_size_degrees);
      if (strstr (varin, N_("[LATITUDE BIN SIZE IN DEGREES]"))) sscanf (info, "%lf", &chrtr2h[hnd].header.lat_grid_size_degrees);

      if (strstr (varin, N_("[MIN HORIZONTAL UNCERTAINTY]"))) sscanf (info, "%f", &chrtr2h[hnd].header.min_horizontal_uncertainty);
      if (strstr (varin, N_("[MAX HORIZONTAL UNCERTAINTY]"))) sscanf (info, "%f", &chrtr2h[hnd].header.max_horizontal_uncertainty);
      if (strstr (varin, N_("[HORIZONTAL UNCERTAINTY SCALE]"))) sscanf (info, "%f", &chrtr2h[hnd].header.horizontal_uncertainty_scale);

      if (strstr (varin, N_("[MIN VERTICAL UNCERTAINTY]"))) sscanf (info, "%f", &chrtr2h[hnd].header.min_vertical_uncertainty);
      if (strstr (varin, N_("[MAX VERTICAL UNCERTAINTY]"))) sscanf (info, "%f", &chrtr2h[hnd].header.max_vertical_uncertainty);
      if (strstr (varin, N_("[VERTICAL UNCERTAINTY SCALE]"))) sscanf (info, "%f", &chrtr2h[hnd].header.vertical_uncertainty_scale);

      if (strstr (varin, N_("[STATUS BITS]")))
        {
          sscanf (info, "%hd", &tmpi16);
          chrtr2h[hnd].status_bits = (NV_U_BYTE) tmpi16;
        }

      if (strstr (varin, N_("[MIN UNCERTAINTY]"))) sscanf (info, "%f", &chrtr2h[hnd].header.min_uncertainty);
      if (strstr (varin, N_("[MAX UNCERTAINTY]"))) sscanf (info, "%f", &chrtr2h[hnd].header.max_uncertainty);
      if (strstr (varin, N_("[UNCERTAINTY SCALE]"))) sscanf (info, "%f", &chrtr2h[hnd].header.uncertainty_scale);
      if (strstr (varin, N_("[UNCERTAINTY NAME]"))) strcpy (chrtr2h[hnd].header.uncertainty_name, info);

      if (strstr (varin, N_("[MIN DATUM/MSL (Z0) SEPARATION]"))) sscanf (info, "%f", &chrtr2h[hnd].header.min_z0);
      if (strstr (varin, N_("[MAX DATUM/MSL (Z0) SEPARATION]"))) sscanf (info, "%f", &chrtr2h[hnd].header.max_z0);
      if (strstr (varin, N_("[DATUM/MSL (Z0) SEPARATION SCALE]"))) sscanf (info, "%f", &chrtr2h[hnd].header.z0_scale);
      if (strstr (varin, N_("[VERTICAL DATUM (Z0) NAME]"))) strcpy (chrtr2h[hnd].header.z0_name, info);

      if (strstr (varin, N_("[MIN MSL/ELLIPSOID (Z1) SEPARATION]"))) sscanf (info, "%f", &chrtr2h[hnd].header.min_z1);
      if (strstr (varin, N_("[MAX MSL/ELLIPSOID (Z1) SEPARATION]"))) sscanf (info, "%f", &chrtr2h[hnd].header.max_z1);
      if (strstr (varin, N_("[MSL/ELLIPSOID (Z1) SEPARATION SCALE]"))) sscanf (info, "%f", &chrtr2h[hnd].header.z1_scale);

      if (strstr (varin, N_("[MAX NUMBER OF POINTS]"))) sscanf (info, "%d", &chrtr2h[hnd].header.max_number_of_points);

      if (strstr (varin, N_("[HEADER SIZE]"))) sscanf (info, "%d", &chrtr2h[hnd].header_size);
      if (strstr (varin, N_("[RECORD SIZE]"))) sscanf (info, "%hd", &chrtr2h[hnd].record_size);
    }


  /*  If the user flags weren't defined, set the default names.  */

  if (!(strlen (chrtr2h[hnd].header.user_flag_name[0]))) strcpy (chrtr2h[hnd].header.user_flag_name[0], "CHRTR2_USER_01");
  if (!(strlen (chrtr2h[hnd].header.user_flag_name[1]))) strcpy (chrtr2h[hnd].header.user_flag_name[1], "CHRTR2_USER_02");
  if (!(strlen (chrtr2h[hnd].header.user_flag_name[2]))) strcpy (chrtr2h[hnd].header.user_flag_name[2], "CHRTR2_USER_03");
  if (!(strlen (chrtr2h[hnd].header.user_flag_name[3]))) strcpy (chrtr2h[hnd].header.user_flag_name[3], "CHRTR2_USER_04");


  /*  Compute min and max sizes for modifiable hardwired fields so we can check the range in chrtr2_write_record.  */

  chrtr2h[hnd].max_status = NINT (pow (2.0, (NV_FLOAT64) chrtr2h[hnd].status_bits)) - 1;


  chrtr2_inv_cvtime (year[0] - 1900, jday[0], hour[0], minute[0], second[0], &chrtr2h[hnd].header.creation_tv_sec, &dummy_tv_nsec);

  chrtr2_inv_cvtime (year[1] - 1900, jday[1], hour[1], minute[1], second[1], &chrtr2h[hnd].header.modification_tv_sec, &dummy_tv_nsec);

  define_record_fields (hnd, NVFalse);

  *chrtr2_header = chrtr2h[hnd].header;

  chrtr2h[hnd].modified = NVFalse;
  chrtr2h[hnd].created = NVFalse;

  chrtr2_error.system = 0;
  return (hnd);
}


/********************************************************************************************/
/*!

 - Function:    chrtr2_close_file

 - Purpose:     Close a CHRTR2 file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The file handle

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_HEADER_WRITE_FSEEK_ERROR
                - CHRTR2_HEADER_WRITE_ERROR
                - CHRTR2_CLOSE_ERROR

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_close_file (NV_INT32 hnd)
{
  time_t t;
  struct tm *cur_tm;
  long dummy_tv_nsec;


  /*  Just in case someone tries to close a file more than once... */

  if (chrtr2h[hnd].fp == NULL) return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);


  /*  If the file was created or modified we need to save the time.  */

  if (chrtr2h[hnd].modified)
    {
      t = time (&t);
      cur_tm = gmtime (&t);
      chrtr2_inv_cvtime (cur_tm->tm_year, cur_tm->tm_yday + 1, cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec,
                         &chrtr2h[hnd].header.modification_tv_sec, &dummy_tv_nsec);
    }


  if (chrtr2h[hnd].created)
    {
      t = time (&t);
      cur_tm = gmtime (&t);
      chrtr2_inv_cvtime (cur_tm->tm_year, cur_tm->tm_yday + 1, cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec,
                         &chrtr2h[hnd].header.creation_tv_sec, &dummy_tv_nsec);
    }


  /*  If we created or modified the file, update the header.  */

  if (chrtr2h[hnd].created || chrtr2h[hnd].modified)
    {
      if (chrtr2_write_header (hnd) < 0) return (chrtr2_error.chrtr2);
    }


  if (fclose (chrtr2h[hnd].fp))
    {
      chrtr2_error.system = errno;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_CLOSE_ERROR);
    }


  /*  Clear the internal structure.  */

  memset (&chrtr2h[hnd], 0, sizeof (INTERNAL_CHRTR2_STRUCT));
  chrtr2h[hnd].fp = NULL;
  if (chrtr2h[hnd].cov_map != NULL) free (chrtr2h[hnd].cov_map);
  chrtr2h[hnd].cov_map = NULL;


  chrtr2_error.system = 0;
  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_read_record

 - Purpose:     Retrieve a CHRTR2 point from a CHRTR2 file.  Normally you should use
                chrtr2_read_row.  This is only useful if you're bouncing all
                over the grid grabbing cells at random.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The file handle
                - coord          =    The coordinates (row, column) of the CHRTR2 cell to be written
                - chrtr2_record  =    The returned CHRTR2 record

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_INVALID_RECORD_NUMBER
                - CHRTR2_READ_FSEEK_ERROR
                - CHRTR2_READ_ERROR

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_read_record (NV_INT32 hnd, NV_I32_COORD2 coord, CHRTR2_RECORD *chrtr2_record)
{
  NV_INT64 pos;
  NV_U_BYTE buffer[1024];


  /*  Check for record out of bounds.  */

  if (coord.x < 0 || coord.x >= chrtr2h[hnd].header.width || coord.y < 0 || coord.y >= chrtr2h[hnd].header.height)
    {
      chrtr2_error.coord = coord;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_INVALID_COORD);
    }


  pos = (NV_INT64) chrtr2h[hnd].header_size + (((NV_INT64) coord.y * (NV_INT64) chrtr2h[hnd].header.width) + (NV_INT64) coord.x) *
    (NV_INT64) chrtr2h[hnd].record_size;

  if (fseeko64 (chrtr2h[hnd].fp, pos, SEEK_SET) < 0)
    {
      chrtr2_error.system = errno;
      chrtr2_error.coord = coord;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_READ_FSEEK_ERROR);
    }

  if (!fread (buffer, chrtr2h[hnd].record_size, 1, chrtr2h[hnd].fp))
    {
      chrtr2_error.system = errno;
      chrtr2_error.coord = coord;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_READ_ERROR);
    }


  /*  Unpack the record.  */

  unpack_record (hnd, buffer, chrtr2_record);


  chrtr2_error.system = 0;
  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_read_record_row_col

 - Purpose:     Retrieve a CHRTR2 point from a CHRTR2 file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The file handle
                - row            =    Row of record to be read
                - col            =    Column of record to be read
                - chrtr2_record  =    The returned CHRTR2 record

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_INVALID_RECORD_NUMBER
                - CHRTR2_READ_FSEEK_ERROR
                - CHRTR2_READ_ERROR

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_read_record_row_col (NV_INT32 hnd, NV_INT32 row, NV_INT32 col, CHRTR2_RECORD *chrtr2_record)
{
  NV_I32_COORD2 coord;

  coord.x = col;
  coord.y = row;

  return (chrtr2_read_record (hnd, coord, chrtr2_record));
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_read_record_lat_lon

 - Purpose:     Retrieve a CHRTR2 point from a CHRTR2 file.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The file handle
                - lat            =    Latitude record to be read
                - lon            =    Longitude of record to be read
                - chrtr2_record  =    The returned CHRTR2 record

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_INVALID_RECORD_NUMBER
                - CHRTR2_READ_FSEEK_ERROR
                - CHRTR2_READ_ERROR

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_read_record_lat_lon (NV_INT32 hnd, NV_FLOAT64 lat, NV_FLOAT64 lon, CHRTR2_RECORD *chrtr2_record)
{
  NV_I32_COORD2 coord;

  coord.x = (NV_INT32) ((lon - chrtr2h[hnd].header.mbr.wlon) / chrtr2h[hnd].header.lon_grid_size_degrees);
  coord.y = (NV_INT32) ((lat - chrtr2h[hnd].header.mbr.slat) / chrtr2h[hnd].header.lat_grid_size_degrees);

  return (chrtr2_read_record (hnd, coord, chrtr2_record));
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_read_row

 - Purpose:     Retrieve a row of CHRTR2 data from a CHRTR2 file.  This does not always mean
                a complete row.  You can use the arguments to read a partial row.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The file handle
                - row            =    Row of data to be read
                - start_column   =    Start column of the data to be read
                - length         =    Number of columns to read (if 0, read entire row - forces start_colum to 0)
                - chrtr2_record  =    The returned array of CHRTR2 records

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_INVALID_RECORD_NUMBER
                - CHRTR2_READ_FSEEK_ERROR
                - CHRTR2_READ_ERROR

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_read_row (NV_INT32 hnd, NV_INT32 row, NV_INT32 start_column, NV_INT32 length, CHRTR2_RECORD *chrtr2_record)
{
  NV_INT64 pos;
  NV_U_BYTE *buffer = NULL, *ptr;
  NV_INT32 i, bytes;


  /*  Check for record out of bounds.  */

  if (row < 0 || start_column < 0 || row >= chrtr2h[hnd].header.height || start_column + length > chrtr2h[hnd].header.width)
    {
      chrtr2_error.coord.x = start_column;
      chrtr2_error.coord.y = row;
      chrtr2_error.length = length;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_INVALID_ROW_COLUMN);
    }


  pos = (NV_INT64) chrtr2h[hnd].header_size + (((NV_INT64) row * (NV_INT64) chrtr2h[hnd].header.width) + (NV_INT64) start_column) *
    (NV_INT64) chrtr2h[hnd].record_size;


  if (fseeko64 (chrtr2h[hnd].fp, pos, SEEK_SET) < 0)
    {
      chrtr2_error.system = errno;
      chrtr2_error.coord.x = start_column;
      chrtr2_error.coord.y = row;
      chrtr2_error.length = length;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_READ_FSEEK_ERROR);
    }


  bytes = chrtr2h[hnd].record_size * length;
  buffer = (NV_U_BYTE *) calloc (bytes, sizeof (NV_U_BYTE));
  if (buffer == NULL)
    {
      perror ("Allocating memory in chrtr2_read_row");
      exit (-1);
    }


  if (!fread (buffer, bytes, 1, chrtr2h[hnd].fp))
    {
      chrtr2_error.system = errno;
      chrtr2_error.coord.x = start_column;
      chrtr2_error.coord.y = row;
      chrtr2_error.length = length;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_READ_ERROR);
    }


  /*  Unpack the records.  */

  ptr = buffer;
  for (i = 0 ; i < length ; i++)
    {
      unpack_record (hnd, ptr, &chrtr2_record[i]);
      ptr += chrtr2h[hnd].record_size;
    }

  free (buffer);


  chrtr2_error.system = 0;
  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_write_record

 - Purpose:     Bit pack the CHRTR2 record into the byte buffer and then write it out.  This
                is the scatter shot way of writing records.  It's much more efficient to
                use chrtr2_write_row.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - coord          =    The coordinates (row, column) of the CHRTR2 cell to be written
                - chrtr2_record  =    The CHRTR2 record to write out

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_WRITE_FSEEK_ERROR
                - CHRTR2_WRITE_ERROR
                - CHRTR2_VALUE_OUT_OF_RANGE_ERROR
                - CHRTR2_INVALID_COORD

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_write_record (NV_INT32 hnd, NV_I32_COORD2 coord, CHRTR2_RECORD chrtr2_record)
{
  NV_U_BYTE buffer[1024];
  NV_INT32 stat;
  NV_U_INT32 cov_pos;
  NV_INT64 pos;


  /*  Check for record out of bounds.  */

  if (coord.x < 0 || coord.x >= chrtr2h[hnd].header.width || coord.y < 0 || coord.y >= chrtr2h[hnd].header.height)
    {
      chrtr2_error.coord = coord;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_INVALID_COORD);
    }


  if (pack_record (hnd, buffer, chrtr2_record)) return (chrtr2_error.chrtr2);


  pos = (NV_INT64) chrtr2h[hnd].header_size + (((NV_INT64) coord.y * (NV_INT64) chrtr2h[hnd].header.width) + (NV_INT64) coord.x) *
    (NV_INT64) chrtr2h[hnd].record_size;

  if (fseeko64 (chrtr2h[hnd].fp, pos, SEEK_SET) < 0)
    {
      chrtr2_error.system = errno;
      chrtr2_error.coord = coord;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_WRITE_FSEEK_ERROR);
    }

  if (!fwrite (buffer, chrtr2h[hnd].record_size, 1, chrtr2h[hnd].fp))
    {
      chrtr2_error.system = errno;
      chrtr2_error.coord = coord;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_WRITE_ERROR);
    }


  /*  If we're maintaining a coverage map for this file, update the coverage status.  */

  if (chrtr2h[hnd].cov_map != NULL)
    {
      stat = chrtr2_record.status & COV_MAP_MASK;

      cov_pos = (coord.y * chrtr2h[hnd].header.width + coord.x) * COV_MAP_BITS;

      chrtr2_bit_pack (chrtr2h[hnd].cov_map, cov_pos, COV_MAP_BITS, stat);
    }


  chrtr2_error.system = 0;
  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_write_record_row_col

 - Purpose:     Bit pack the CHRTR2 record into the byte buffer and then write it out.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - row            =    Row of record to be written
                - col            =    Column of record to be written
                - chrtr2_record  =    The CHRTR2 record to write out

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_WRITE_FSEEK_ERROR
                - CHRTR2_WRITE_ERROR
                - CHRTR2_VALUE_OUT_OF_RANGE_ERROR
                - CHRTR2_INVALID_COORD

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_write_record_row_col (NV_INT32 hnd, NV_INT32 row, NV_INT32 col, CHRTR2_RECORD chrtr2_record)
{
  NV_I32_COORD2 coord;

  coord.x = col;
  coord.y = row;

  return (chrtr2_write_record (hnd, coord, chrtr2_record));
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_write_record_lat_lon

 - Purpose:     Bit pack the CHRTR2 record into the byte buffer and then write it out.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - lat            =    Latitude of record to be written
                - lon            =    Longitude of record to be written
                - chrtr2_record  =    The CHRTR2 record to write out

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_WRITE_FSEEK_ERROR
                - CHRTR2_WRITE_ERROR
                - CHRTR2_VALUE_OUT_OF_RANGE_ERROR
                - CHRTR2_INVALID_COORD

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_write_record_lat_lon (NV_INT32 hnd, NV_FLOAT64 lat, NV_FLOAT64 lon, CHRTR2_RECORD chrtr2_record)
{
  NV_I32_COORD2 coord;

  coord.x = (NV_INT32) ((lon - chrtr2h[hnd].header.mbr.wlon) / chrtr2h[hnd].header.lon_grid_size_degrees);
  coord.y = (NV_INT32) ((lat - chrtr2h[hnd].header.mbr.slat) / chrtr2h[hnd].header.lat_grid_size_degrees);

  return (chrtr2_write_record (hnd, coord, chrtr2_record));
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_write_row

 - Purpose:     Bit pack the array of CHRTR2 records into the byte buffer and then write it out.
                This is much more efficient than writing one record at a time.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - row            =    Row of data to be read
                - start_col      =    Start column of the data to be read
                - length         =    Number of columns to read (if 0, read entire row - forces start_colum to 0)
                - chrtr2_record  =    The CHRTR2 record array to write out

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_WRITE_FSEEK_ERROR
                - CHRTR2_WRITE_ERROR
                - CHRTR2_VALUE_OUT_OF_RANGE_ERROR
                - CHRTR2_INVALID_ROW_COLUMN

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_write_row (NV_INT32 hnd, NV_INT32 row, NV_INT32 start_col, NV_INT32 length, CHRTR2_RECORD *chrtr2_record)
{
  NV_U_BYTE *buffer = NULL, *ptr;
  NV_INT32 i, bytes, stat;
  NV_U_INT32 cov_pos;
  NV_INT64 pos;


  /*  Check for record out of bounds.  */

  if (row < 0 || start_col < 0 || row >= chrtr2h[hnd].header.height || start_col + length >= chrtr2h[hnd].header.width)
    {
      chrtr2_error.coord.x = start_col;
      chrtr2_error.coord.y = row;
      chrtr2_error.length = length;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_INVALID_ROW_COLUMN);
    }


  bytes = chrtr2h[hnd].record_size * length;
  buffer = (NV_U_BYTE *) calloc (bytes, sizeof (NV_U_BYTE));
  if (buffer == NULL)
    {
      perror ("Allocating memory in chrtr2_read_row");
      exit (-1);
    }


  /*  Pack the records.  */

  ptr = buffer;
  for (i = 0 ; i < length ; i++)
    {
      if (pack_record (hnd, ptr, chrtr2_record[i]))
        {
          free (buffer);
          return (chrtr2_error.chrtr2);
        }
      ptr += chrtr2h[hnd].record_size;


      /*  If we're maintaining a coverage map for this file, update the coverage status.  */

      if (chrtr2h[hnd].cov_map != NULL)
        {
          stat = chrtr2_record[i].status & COV_MAP_MASK;

          cov_pos = (row * chrtr2h[hnd].header.width + start_col + i) * COV_MAP_BITS;

          chrtr2_bit_pack (chrtr2h[hnd].cov_map, cov_pos, COV_MAP_BITS, stat);
        }
    }


  pos = (NV_INT64) chrtr2h[hnd].header_size + (((NV_INT64) row * (NV_INT64) chrtr2h[hnd].header.width) + (NV_INT64) start_col) *
    (NV_INT64) chrtr2h[hnd].record_size;


  if (fseeko64 (chrtr2h[hnd].fp, pos, SEEK_SET) < 0)
    {
      chrtr2_error.system = errno;
      chrtr2_error.coord.x = start_col;
      chrtr2_error.coord.y = row;
      chrtr2_error.length = length;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_WRITE_FSEEK_ERROR);
    }


  if (!fwrite (buffer, bytes, 1, chrtr2h[hnd].fp))
    {
      chrtr2_error.system = errno;
      chrtr2_error.coord.x = start_col;
      chrtr2_error.coord.y = row;
      chrtr2_error.length = length;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_WRITE_ERROR);
    }


  free (buffer);


  chrtr2_error.system = 0;
  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_update_header

 - Purpose:     Update the modifiable fields of the header record.  See CHRTR2_HEADER in chrtr2.h to
                determine which fields are modifiable after file creation.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The file handle
                - chrtr2_header     =    The CHRTR2_HEADER structure.

 - Returns:
                - void

*********************************************************************************************/

CHRTR2_DLL void chrtr2_update_header (NV_INT32 hnd, CHRTR2_HEADER chrtr2_header)
{
  NV_INT32      i;

  strcpy (chrtr2h[hnd].header.modification_software, chrtr2_header.modification_software);
  strcpy (chrtr2h[hnd].header.security_classification, chrtr2_header.security_classification);
  strcpy (chrtr2h[hnd].header.distribution, chrtr2_header.distribution);
  strcpy (chrtr2h[hnd].header.declassification, chrtr2_header.declassification);
  strcpy (chrtr2h[hnd].header.class_just, chrtr2_header.class_just);
  strcpy (chrtr2h[hnd].header.downgrade, chrtr2_header.downgrade);
  strcpy (chrtr2h[hnd].header.comments, chrtr2_header.comments);
  for (i = 0 ; i < 5 ; i++) strcpy (chrtr2h[hnd].header.user_flag_name[i], chrtr2_header.user_flag_name[i]);
  strcpy (chrtr2h[hnd].header.uncertainty_name, chrtr2_header.uncertainty_name);
  chrtr2h[hnd].header.min_observed_z = chrtr2_header.min_observed_z;
  chrtr2h[hnd].header.max_observed_z = chrtr2_header.max_observed_z;


  /*  Force a header write when we close the file.  */

  chrtr2h[hnd].modified = NVTrue;
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_open_cov_map

 - Purpose:     Creates and maintains a CHRTR2 coverage map in memory.  The coverage map only supports 
                CHRTR2_NULL, CHRTR2_REAL, CHRTR2_DIGITIZED_CONTOUR, CHRTR2_INTERPOLATED, and CHRTR2_CHECKED.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle

 - Returns:
                - CHRTR2_SUCCESS

 - Caveats:     This can use quite a bit of memory for large files.  We're only using 4 bits per field but,
                for a file of 10,000 by 10,000 that can add up to 50MB.  This memory is freed in 
                chrtr2_close_file or chrtr2_close_cov_map.  You don't have to open the coverage map to
                read or write CHRTR2 files.  It's only useful for graphical applications that have a coverage
                map.

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_open_cov_map (NV_INT32 hnd)
{
  NV_INT32 i, j, size, stat;
  NV_U_INT32 cov_pos;
  CHRTR2_RECORD *chrtr2_record;


  /*  Make sure we haven't already created this.  */

  if (chrtr2h[hnd].cov_map != NULL) return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);


  /*  Compute the bytes needed.  */

  size = ((chrtr2h[hnd].header.height * chrtr2h[hnd].header.width) * COV_MAP_BITS) / 8 + 1;


  /*  Allocate and clear the coverage map memory.  */

  chrtr2h[hnd].cov_map = (NV_U_BYTE *) calloc (size, sizeof (NV_U_BYTE));

  if (chrtr2h[hnd].cov_map == NULL)
    {
      perror ("Allocating cov_map memory in chrtr2_open_cov_map");
      exit (-1);
    }


  /*  Allocate and clear the chrtr2 record memory.  */

  chrtr2_record = (CHRTR2_RECORD *) calloc (chrtr2h[hnd].header.width, sizeof (CHRTR2_RECORD));

  if (chrtr2_record == NULL)
    {
      perror ("Allocating chrtr2_record memory in chrtr2_open_cov_map");
      exit (-1);
    }


  cov_pos = 0;
  for (i = 0 ; i < chrtr2h[hnd].header.height ; i++)
    {
      chrtr2_read_row (hnd, i, 0, chrtr2h[hnd].header.width, chrtr2_record);

      for (j = 0 ; j < chrtr2h[hnd].header.width ; j++)
        {
          stat = chrtr2_record[j].status & COV_MAP_MASK;

          chrtr2_bit_pack (chrtr2h[hnd].cov_map, cov_pos, COV_MAP_BITS, stat); cov_pos += COV_MAP_BITS;
        }
    }

  free (chrtr2_record);

  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_close_cov_map

 - Purpose:     Closes and frees a CHRTR2 coverage map in memory.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle

 - Returns:
                - CHRTR2_SUCCESS

 - Caveats:     You would normally let chrtr2_close_file handle this.

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_close_cov_map (NV_INT32 hnd)
{
  if (chrtr2h[hnd].cov_map != NULL) free (chrtr2h[hnd].cov_map);
  chrtr2h[hnd].cov_map = NULL;

  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_read_cov_map

 - Purpose:     Returns the coverage map status (CHRTR2_NULL, CHRTR2_REAL, CHRTR2_DIGITIZED_CONTOUR,
                CHRTR2_INTERPOLATED, and CHRTR2_CHECKED) from the coverage map in memory.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - coord          =    Coordinates of the bin
                - status         =    Returned coverage status

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_NO_COV_MAP
                - CHRTR2_INVALID_COORD

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_read_cov_map (NV_INT32 hnd, NV_I32_COORD2 coord, NV_U_INT16 *status)
{
  NV_U_INT32 cov_pos;


  /*  Make sure we have already created the coverage map memory for this CHRTR2 file.  */

  if (chrtr2h[hnd].cov_map == NULL)
    {
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_NO_COV_MAP);
    }


  /*  Check for record out of bounds.  */

  if (coord.x < 0 || coord.x >= chrtr2h[hnd].header.width || coord.y < 0 || coord.y >= chrtr2h[hnd].header.height)
    {
      chrtr2_error.coord = coord;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_INVALID_COORD);
    }


  cov_pos = (coord.y * chrtr2h[hnd].header.width + coord.x) * COV_MAP_BITS;

  *status = chrtr2_bit_unpack (chrtr2h[hnd].cov_map, cov_pos, COV_MAP_BITS);


  return (chrtr2_error.chrtr2 = CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_read_cov_map_row_col

 - Purpose:     Returns the coverage map status (CHRTR2_NULL, CHRTR2_REAL, CHRTR2_DIGITIZED_CONTOUR,
                CHRTR2_INTERPOLATED, and CHRTR2_CHECKED) from the coverage map in memory using row
                and column to determine bin location.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        08/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - row            =    Row of the requested bin
                - col            =    Column of the requested bin
                - status         =    Returned coverage status

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_NO_COV_MAP
                - CHRTR2_INVALID_COORD

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_read_cov_map_row_col (NV_INT32 hnd, NV_INT32 row, NV_INT32 col, NV_U_INT16 *status)
{
  NV_I32_COORD2 coord;

  coord.x = col;
  coord.y = row;

  return (chrtr2_read_cov_map (hnd, coord, status));
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_get_coord

 - Purpose:     Returns the grid coordinates of a CHRTR2 bin based on lat and lon

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - lat            =    Latitude
                - lon            =    Longitude
                - coord          =    Grid coordinates returned

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_INVALID_LAT_LON

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_get_coord (NV_INT32 hnd, NV_FLOAT64 lat, NV_FLOAT64 lon, NV_I32_COORD2 *coord)
{
  /*  Check for record out of bounds.  */

  if (lat < chrtr2h[hnd].header.mbr.slat || lat > chrtr2h[hnd].header.mbr.nlat || lon < chrtr2h[hnd].header.mbr.wlon || lon > chrtr2h[hnd].header.mbr.elon)
    {
      chrtr2_error.lat = lat;
      chrtr2_error.lon = lon;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_INVALID_LAT_LON);
    }

  coord->x = (NV_INT32) ((lon - chrtr2h[hnd].header.mbr.wlon) / chrtr2h[hnd].header.lon_grid_size_degrees);
  coord->y = (NV_INT32) ((lat - chrtr2h[hnd].header.mbr.slat) / chrtr2h[hnd].header.lat_grid_size_degrees);

  return (CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_get_lat_lon

 - Purpose:     Returns the lat and lon of the center of a CHRTR2 bin based on the grid coordinates

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - hnd            =    The CHRTR2 file handle
                - lat            =    Latitude
                - lon            =    Longitude
                - coord          =    Grid coordinates

 - Returns:
                - CHRTR2_SUCCESS
                - CHRTR2_INVALID_COORD

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_get_lat_lon (NV_INT32 hnd, NV_FLOAT64 *lat, NV_FLOAT64 *lon, NV_I32_COORD2 coord)
{
  /*  Check for record out of bounds.  */

  if (coord.x < 0 || coord.x >= chrtr2h[hnd].header.width || coord.y < 0 || coord.y >= chrtr2h[hnd].header.height)
    {
      chrtr2_error.coord = coord;
      strcpy (chrtr2_error.file, chrtr2h[hnd].path);
      return (chrtr2_error.chrtr2 = CHRTR2_INVALID_COORD);
    }


  *lon = chrtr2h[hnd].header.mbr.wlon + ((NV_FLOAT64) coord.x + 0.5L) * chrtr2h[hnd].header.lon_grid_size_degrees;
  *lat = chrtr2h[hnd].header.mbr.slat + ((NV_FLOAT64) coord.y + 0.5L) * chrtr2h[hnd].header.lat_grid_size_degrees;


  return (CHRTR2_SUCCESS);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_get_errno

 - Purpose:     Returns the latest CHRTR2 error condition code

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - void

 - Returns:
                - error condition code

 - Caveats:     The only thing this is good for at present is to determine if, when you opened
                the file, the library version was older than the file.  That is, if
                CHRTR2_NEWER_FILE_VERSION_WARNING has been set when you called chrtr2_open_file.
                Otherwise, you can just use chrtr2_perror or chrtr2_strerror to get the last
                error information.

*********************************************************************************************/

CHRTR2_DLL NV_INT32 chrtr2_get_errno ()
{
  return (chrtr2_error.chrtr2);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_strerror

 - Purpose:     Returns the error string related to the latest error.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - void

 - Returns:
                - Error message

*********************************************************************************************/

CHRTR2_DLL NV_CHAR *chrtr2_strerror ()
{
  static NV_CHAR message[1024];

  switch (chrtr2_error.chrtr2)
    {
    case CHRTR2_SUCCESS:
      sprintf (message, _("SUCCESS!\n"));
      break;

    case CHRTR2_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing record data :\n%s\n"),
               chrtr2_error.file, strerror (chrtr2_error.system));
      break;

    case CHRTR2_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing record data :\n%s\n"),
               chrtr2_error.file, strerror (chrtr2_error.system));
      break;

    case CHRTR2_HEADER_WRITE_FSEEK_ERROR:
      sprintf (message, _("File : %s\nError during fseek prior to writing header :\n%s\n"),
               chrtr2_error.file, strerror (chrtr2_error.system));
      break;

    case CHRTR2_HEADER_WRITE_ERROR:
      sprintf (message, _("File : %s\nError writing header :\n%s\n"),
               chrtr2_error.file, strerror (chrtr2_error.system));
      break;

    case CHRTR2_TOO_MANY_OPEN_FILES:
      sprintf (message, _("Too many CHRTR2 files are already open.\n"));
      break;

    case CHRTR2_CREATE_ERROR:
      sprintf (message, _("File : %s\nError creating CHRTR2 file :\n%s\n"),
               chrtr2_error.file, strerror (chrtr2_error.system));
      break;

    case CHRTR2_OPEN_UPDATE_ERROR:
      sprintf (message, _("File : %s\nError opening CHRTR2 file for update :\n%s\n"),
               chrtr2_error.file, strerror (chrtr2_error.system));
      break;

    case CHRTR2_OPEN_READONLY_ERROR:
      sprintf (message, _("File : %s\nError opening CHRTR2 file read-only :\n%s\n"),
               chrtr2_error.file, strerror (chrtr2_error.system));
      break;

    case CHRTR2_NOT_CHRTR2_FILE_ERROR:
      sprintf (message, _("File : %s\nThe file version string is corrupt or indicates that this is not a CHRTR2 file.\n"),
               chrtr2_error.file);
      break;

    case CHRTR2_NEWER_FILE_VERSION_WARNING:
      sprintf (message, _("File : %s\nThe file version is newer than the CHRTR2 library version.\nThis may cause problems.\n"),
               chrtr2_error.file);
      break;

    case CHRTR2_CLOSE_ERROR:
      sprintf (message, _("File : %s\nError closing CHRTR2 file :\n%s\n"),
               chrtr2_error.file, strerror (chrtr2_error.system));
      break;

    case CHRTR2_INVALID_COORD:
      sprintf (message, _("File : %s\nCoordinates : %d,%d\nInvalid coordinates.\n"),
               chrtr2_error.file, chrtr2_error.coord.x, chrtr2_error.coord.y);
      break;

    case CHRTR2_INVALID_ROW_COLUMN:
      sprintf (message, _("File : %s\nRow, col, length : %d,%d,%d\nInvalid row/column.\n"),
               chrtr2_error.file, chrtr2_error.coord.y, chrtr2_error.coord.x, chrtr2_error.length);
      break;

    case CHRTR2_READ_FSEEK_ERROR:
      sprintf (message, _("File : %s\nCoordinates : %d,%d\nError during fseek prior reading a record :\n%s\n"),
               chrtr2_error.file, chrtr2_error.coord.y, chrtr2_error.coord.x, strerror (chrtr2_error.system));
      break;

    case CHRTR2_READ_ERROR:
      sprintf (message, _("File : %s\nCoordinates : %d,%d\nError reading record :\n%s\n"),
               chrtr2_error.file, chrtr2_error.coord.y, chrtr2_error.coord.x, strerror (chrtr2_error.system));
      break;

    case CHRTR2_VALUE_OUT_OF_RANGE_ERROR:
      sprintf (message, _("File : %s\n%s\n"), chrtr2_error.file, chrtr2_error.info);
      break;

    case CHRTR2_INVALID_LAT_LON:
      sprintf (message, _("File : %s\nLat/Lon : %.9f,%.9f\nInvalid position.\n"),
               chrtr2_error.file, chrtr2_error.lat, chrtr2_error.lon);
      break;

    case CHRTR2_NO_COV_MAP:
      sprintf (message, _("File : %s\nNo coverage map has been allocated for this file.  Call chrtr2_open_cov_map first.\n"),
               chrtr2_error.file);
      break;
    }

  return (message);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_perror

 - Purpose:     Prints (to stderr) the latest error messages.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - void

 - Returns:
                - void

*********************************************************************************************/

CHRTR2_DLL void chrtr2_perror ()
{
  fprintf (stderr, chrtr2_strerror ());
  fflush (stderr);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_get_version

 - Purpose:     Returns the CHRTR2 library version string

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - void

 - Returns:
                - version string

*********************************************************************************************/

CHRTR2_DLL NV_CHAR *chrtr2_get_version ()
{
  return (CHRTR2_VERSION);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_cvtime

 - Purpose:     Convert from POSIX time to year, day of year, hour, minute,
                second.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Arguments:
                - time_t tv_sec  =    POSIX seconds from epoch (Jan. 1, 1970)
                - long tv_nsec   =    POSIX nanoseconds of second
                - year           =    4 digit year - 1900
                - jday           =    day of year
                - hour           =    hour of day
                - minute         =    minute of hour
                - second         =    seconds of minute

 - Returns:
                - void

 - Caveats:     The localtime function will return the year as a 2 digit year (offset from 1900).

*********************************************************************************************/
 
CHRTR2_DLL void chrtr2_cvtime (time_t tv_sec, long tv_nsec, NV_INT32 *year, NV_INT32 *jday, NV_INT32 *hour,
                               NV_INT32 *minute, NV_FLOAT32 *second)
{
  static NV_INT32      tz_set = 0;
  struct tm            time_struct, *time_ptr = &time_struct;

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

  time_ptr = localtime (&tv_sec);

  *year = (NV_INT16) time_ptr->tm_year;
  *jday = (NV_INT16) time_ptr->tm_yday + 1;
  *hour = (NV_INT16) time_ptr->tm_hour;
  *minute = (NV_INT16) time_ptr->tm_min;
  *second = (NV_FLOAT32) time_ptr->tm_sec + (NV_FLOAT32) ((NV_FLOAT64) tv_nsec / 1000000000.);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_jday2mday

 - Purpose:     Convert from day of year to month and day.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Arguments:
                - year           =    4 digit year or offset from 1900
                - jday           =    day of year
                - mon            =    month
                - mday           =    day of month

 - Returns:
                - void

 - Caveats:     The returned month value will start at 0 for January.

*********************************************************************************************/

CHRTR2_DLL void chrtr2_jday2mday (NV_INT32 year, NV_INT32 jday, NV_INT32 *mon, NV_INT32 *mday)
{
  NV_INT32 l_year;

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



/********************************************************************************************/
/*!

 - Function:    chrtr2_inv_cvtime

 - Purpose:     Convert from year, day of year, hour, minute, second to POSIX time.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - year           =    4 digit year - 1900
                - jday           =    day of year
                - hour           =    hour of day
                - minute         =    minute of hour
                - second         =    seconds of minute
                - time_t tv_sec  =    POSIX seconds from epoch (Jan. 1, 1970)
                - long tv_nsec   =    POSIX nanoseconds of second

 - Returns:
                - void

 - Caveats:     The mktime function takes a 2 digit year (offset from 1900).

*********************************************************************************************/
 
CHRTR2_DLL void chrtr2_inv_cvtime (NV_INT32 year, NV_INT32 jday, NV_INT32 hour, NV_INT32 min, NV_FLOAT32 sec,
                                   time_t *tv_sec, long *tv_nsec)
{
  struct tm                    tm;
  static NV_INT32              tz_set = 0;


  tm.tm_year = year;

  chrtr2_jday2mday (year, jday, &tm.tm_mon, &tm.tm_mday);

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

  *tv_sec = mktime (&tm);
  *tv_nsec = (long)(fmod ((double) sec, 1.0) * 1.0e9);
}



/********************************************************************************************/
/*!

 - Function:    chrtr2_mday2jday

 - Purpose:     Convert from month and day to day of year.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Arguments:
                - year           =    4 digit year or offset from 1900
                - mon            =    month
                - mday           =    day of month
                - jday           =    day of year

 - Returns:
                - void

 - Caveats:     The month value must start at 0 for January.

*********************************************************************************************/

CHRTR2_DLL void chrtr2_mday2jday (NV_INT32 year, NV_INT32 mon, NV_INT32 mday, NV_INT32 *jday)
{
  NV_INT32 i, l_year;

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


  *jday = mday;
  for (i = 0 ; i < mon - 1 ; i++) *jday += months[i];
}
