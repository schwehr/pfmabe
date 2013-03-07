
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



#include "pfmLoad.hpp"


NV_BOOL clear_features (NV_INT32 pfm_handle, NV_I32_COORD2 coord, NV_CHAR *lst, NV_FLOAT64 feature_radius)
{
  BIN_RECORD                  bin;
  NV_INT32                    bfd_handle;

  static BFDATA_HEADER        bfd_header;
  static BFDATA_SHORT_FEATURE *feature;

  NV_CHAR                     ftr[512];
  NV_FLOAT64                  az, dist;
  NV_FLOAT64                  a0 = 6378137.0, b0 = 6356752.314245;
  NV_BOOL                     hit;
  static NV_BOOL              first = NVTrue;



  if (first)
    {
      first = NVFalse;

      get_target_file (pfm_handle, lst, ftr);

      if (strcmp (ftr, "NONE"))
        {
          if (strstr (ftr, ".xml"))
            {
              fprintf (stderr, "NAVO standard target file formst (XML) is no longer supported in PFM_ABE.\n");
              fflush (stderr);
            }
          else
            {
              if ((bfd_handle = binaryFeatureData_open_file (ftr, &bfd_header, BFDATA_READONLY)) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (0, pfmLoad::tr ("pfmLoad"), pfmLoad::tr ("Unable to open feature file\nReason: ") + msg);
                  return (NVFalse);
                }


              if (binaryFeatureData_read_all_short_features (bfd_handle, &feature) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (0, pfmLoad::tr ("pfmLoad"), pfmLoad::tr ("Unable to read feature records\nReason: ") + msg);
                  binaryFeatureData_close_file (bfd_handle);
                  return (NVFalse);
                }

              binaryFeatureData_close_file (bfd_handle);
            }
        }
    }


  if (!bfd_header.number_of_records) return (NVFalse);


  read_bin_record_index (pfm_handle, coord, &bin);

  if (!bin.num_soundings) return (NVFalse);


  hit = NVFalse;
  for (NV_U_INT32 i = 0 ; i < bfd_header.number_of_records ; i++)
    {
      if (feature[i].confidence_level)
        {
          invgp (a0, b0, bin.xy.y, bin.xy.x, feature[i].latitude, feature[i].longitude, &dist, &az);


          /*  If the distance from the center of the bin to the feature is less than the feature radius, return a hit.  */

          if (dist < feature_radius)
            {
              hit = NVTrue;
              break;
            }
        }
    }
  return (hit);
}
