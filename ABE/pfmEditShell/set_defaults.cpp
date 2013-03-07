
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



#include "pfmEditShell.hpp"


//  Set defaults for all of the program's user defined options.  This called before envin.

void set_defaults (MISC *misc, OPTIONS *options)
{
  misc->qsettings_org = pfmEditShell::tr ("navo.navy.mil");
  misc->qsettings_app = pfmEditShell::tr ("pfmView");
  misc->dateline = NVFalse;
  misc->process_id = getpid ();

  options->position_form = 0;
  strcpy (options->edit_name, "pfmEdit");
  strcpy (options->edit_name_3D, "pfmEdit3D");
  options->smoothing_factor = 0;
  options->z_factor = 1.0;
  options->z_offset = 0.0;
  options->min_hsv_color = 0;
  options->max_hsv_color = 315;
  options->min_hsv_locked = NVFalse;
  options->max_hsv_locked = NVFalse;
  options->min_hsv_value = 0.0;
  options->max_hsv_value = 0.0;
  options->feature_search_string = "";


  options->cint = 5.0;
  options->layer_type = MIN_FILTERED_DEPTH;


  //  Default contour levels

  options->num_levels = 52;
  options->contour_levels[0] = -30;
  options->contour_levels[1] = -20;
  options->contour_levels[2] = -10;
  options->contour_levels[3] = -5;
  options->contour_levels[4] = -2;
  options->contour_levels[5] = 0;
  options->contour_levels[6] = 2;
  options->contour_levels[7] = 5;
  options->contour_levels[8] = 10;
  options->contour_levels[9] = 20;
  options->contour_levels[10] = 30;
  options->contour_levels[11] = 40;
  options->contour_levels[12] = 50;
  options->contour_levels[13] = 60;
  options->contour_levels[14] = 70;
  options->contour_levels[15] = 80;
  options->contour_levels[16] = 90;
  options->contour_levels[17] = 100;
  options->contour_levels[18] = 120;
  options->contour_levels[19] = 130;
  options->contour_levels[20] = 140;
  options->contour_levels[21] = 160;
  options->contour_levels[22] = 180;
  options->contour_levels[23] = 200;
  options->contour_levels[24] = 250;
  options->contour_levels[25] = 300;
  options->contour_levels[26] = 350;
  options->contour_levels[27] = 400;
  options->contour_levels[28] = 450;
  options->contour_levels[29] = 500;
  options->contour_levels[30] = 600;
  options->contour_levels[31] = 700;
  options->contour_levels[32] = 800;
  options->contour_levels[33] = 900;
  options->contour_levels[34] = 1000;
  options->contour_levels[35] = 1500;
  options->contour_levels[36] = 2000;
  options->contour_levels[37] = 2500;
  options->contour_levels[38] = 3000;
  options->contour_levels[39] = 3500;
  options->contour_levels[40] = 4000;
  options->contour_levels[41] = 4500;
  options->contour_levels[42] = 5000;
  options->contour_levels[43] = 5500;
  options->contour_levels[44] = 6000;
  options->contour_levels[45] = 6500;
  options->contour_levels[46] = 7000;
  options->contour_levels[47] = 7500;
  options->contour_levels[48] = 8000;
  options->contour_levels[49] = 8500;
  options->contour_levels[50] = 9000;
  options->contour_levels[51] = 10000;
}


