
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



#include "bagViewer.hpp"

void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore)
{
  //  Set Defaults so that if keys don't for envin the parms are defined.

  if (!restore)
    {
      strcpy (misc->GeoTIFF_name, "");
      misc->GeoTIFF_init = NVFalse;
      misc->GeoTIFF_open = NVFalse;
      misc->drawing_canceled = NVFalse;
      misc->drawing = NVFalse;
      misc->display_GeoTIFF = NVFalse;


      misc->width = 990;
      misc->height = 964;
      misc->window_x = 0;
      misc->window_y = 0;
      misc->tracking_list_len = 0;
      misc->tracking_list = NULL;

      misc->html_help_tag = "";


#ifdef NVWIN3X

      misc->help_browser = "C:\\Program Files\\Mozilla Firefox\\firefox.exe";

      QFileInfo br (misc->help_browser);

      if (!br.exists () || !br.isExecutable ()) misc->help_browser = "C:\\Program Files\\Internet Explorer\\iexplore.exe";

#else

      misc->help_browser = "firefox";

#endif
    }


  options->position_form = 0;
  options->z_factor = 1.0;
  options->z_offset = 0.0;
  options->exaggeration = 3.0;
  options->background_color = QColor (0, 0, 0);
  options->tracker_color = QColor (255, 255, 255, 255);
  options->feature_color = QColor (255, 255, 255, 192);
  options->feature_info_color = QColor (255, 255, 255);
  options->scale_color = QColor (255, 255, 255, 255);
  options->draw_scale = NVTrue;
  options->surface = Elevation;
  options->color_option = NVFalse;
  options->display_tracking_list = NVFalse;
  options->display_tracking_info = NVFalse;
  options->feature_size = 0.010;
  options->input_dir = ".";
  options->image_dir = ".";
}
