
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "pfmView.hpp"

/*!
  This function is specifically designed for a list file containing PFM files passed from pfmWDBView.  The file will contain the 
  bounds of interest followed by the file names.
*/

void open_pfm_files (MISC *misc, NV_CHAR *file, NV_F64_XYMBR *bounds)
{
  FILE *fp;

  if ((fp = fopen (file, "r")) == NULL)
    {
      QMessageBox::warning (0, pfmView::tr ("pfmView Open PFM Structure"),
                            pfmView::tr ("The file ") + QDir::toNativeSeparators (QString (file)) + 
                            pfmView::tr (" cannot be opened or there was an error reading the file.") +
                            pfmView::tr ("  The error message returned was:\n\n") +
                            QString (strerror (errno)));
      exit (-1);
    }

  NV_CHAR string[512];
  NV_FLOAT64 mx[4], my[4];

  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      fgets (string, sizeof (string), fp);
      sscanf (string, "%lf %lf", &my[i], &mx[i]);
    }

  bounds->min_x = 999999.0;
  bounds->min_y = 999999.0;
  bounds->max_x = -999999.0;
  bounds->max_y = -999999.0;
  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      if (misc->dateline && mx[i] < 0.0) mx[i] += 360.0;

      if (mx[i] < bounds->min_x) bounds->min_x = mx[i];
      if (my[i] < bounds->min_y) bounds->min_y = my[i];
      if (mx[i] > bounds->max_x) bounds->max_x = mx[i];
      if (my[i] > bounds->max_y) bounds->max_y = my[i];
    }

  fgets (string, sizeof (string), fp);
  sscanf (string, "%d", &misc->abe_share->pfm_count);

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      ngets (misc->abe_share->open_args[pfm].list_path, sizeof (misc->abe_share->open_args[pfm].list_path), fp);


      //  Open the file and make sure it is a valid PFM file.

      misc->abe_share->open_args[pfm].checkpoint = 0;
      misc->pfm_handle[pfm] = open_existing_pfm_file (&misc->abe_share->open_args[pfm]);

      if (misc->pfm_handle[pfm] < 0)
        {
          if (QMessageBox::warning (0, pfmView::tr ("pfmView Open PFM Structure"), pfmView::tr ("The file ") + 
                                    QDir::toNativeSeparators (QString (misc->abe_share->open_args[pfm].list_path)) + 
                                    pfmView::tr (" is not a PFM structure or there was an error reading the file.") +
                                    pfmView::tr ("  The error message returned was:\n\n") + QString (pfm_error_str (pfm_error))))

            if (pfm_error == CHECKPOINT_FILE_EXISTS_ERROR)
              {
                fprintf (stderr, "\n\n%s\n", pfm_error_str (pfm_error));
                exit (-1);
              }

          exit (-1);
        }


      //  Compute cell sizes for sunshading.

      misc->ss_cell_size_x[pfm] = misc->abe_share->open_args[pfm].head.bin_size_xy;
      misc->ss_cell_size_y[pfm] = misc->abe_share->open_args[pfm].head.bin_size_xy;


      misc->average_type[pfm] = 0;


      //  Check to see if we're using something other than the standard AVERAGE EDITED DEPTH (MISP interpolated surface).

      if (strstr (misc->abe_share->open_args[pfm].head.average_filt_name, "MINIMUM MISP"))
        {
          misc->average_type[pfm] = 1;
        }
      else if (strstr (misc->abe_share->open_args[pfm].head.average_filt_name, "AVERAGE MISP"))
        {
          misc->average_type[pfm] = 2;
        }
      else if (strstr (misc->abe_share->open_args[pfm].head.average_filt_name, "MAXIMUM MISP"))
        {
          misc->average_type[pfm] = 3;
        }

      misc->abe_share->display_pfm[pfm] = NVTrue;
      misc->pfm_alpha[pfm] = 255;
    }
}
