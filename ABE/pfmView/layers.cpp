
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



#include "layers.hpp"


//!  Swap the PFM layers l1 and l2.

void swap_layers (MISC *misc, NV_INT32 l1, NV_INT32 l2)
{
  //  Save the information for the first layer

  PFM_OPEN_ARGS open_args = misc->abe_share->open_args[l1];
  NV_BOOL display_pfm = misc->abe_share->display_pfm[l1];

  NV_FLOAT64 ss_cell_size_x = misc->ss_cell_size_x[l1];
  NV_FLOAT64 ss_cell_size_y = misc->ss_cell_size_y[l1];
  NV_CHAR attr_format[NUM_ATTR][20];
  NV_INT32 num_bin_attr = misc->abe_share->open_args[l1].head.num_bin_attr;
  for (NV_INT32 i = 0 ; i < num_bin_attr ; i++) strcpy (attr_format[i], misc->attr_format[l1][i]);
  NV_INT32 pfm_handle = misc->pfm_handle[l1];
  NV_INT32 average_type = misc->average_type[l1];
  NV_BOOL cube_attr_available = misc->cube_attr_available[l1];
  NV_INT32 pfm_alpha = misc->pfm_alpha[l1];
  NV_INT32 last_saved_contour_record = misc->last_saved_contour_record[l1];


  //  Replace first layer with second layer

  misc->abe_share->open_args[l1] = misc->abe_share->open_args[l2];
  misc->abe_share->display_pfm[l1] = misc->abe_share->display_pfm[l2];

  misc->ss_cell_size_x[l1] = misc->ss_cell_size_x[l2];
  misc->ss_cell_size_y[l1] = misc->ss_cell_size_y[l2];
  for (NV_INT32 i = 0 ; i < misc->abe_share->open_args[l2].head.num_bin_attr ; i++)
    strcpy (misc->attr_format[l1][i], misc->attr_format[l2][i]);
  misc->pfm_handle[l1] = misc->pfm_handle[l2];
  misc->average_type[l1] = misc->average_type[l2];
  misc->cube_attr_available[l1] = misc->cube_attr_available[l2];
  misc->pfm_alpha[l1] = misc->pfm_alpha[l2];
  misc->last_saved_contour_record[l1] = misc->last_saved_contour_record[l2];


  //  Replace second layer with saved first layer

  misc->abe_share->open_args[l2] = open_args;
  misc->abe_share->display_pfm[l2] = display_pfm;

  misc->ss_cell_size_x[l2] = ss_cell_size_x;
  misc->ss_cell_size_y[l2] = ss_cell_size_y;
  for (NV_INT32 i = 0 ; i < num_bin_attr ; i++) strcpy (misc->attr_format[l2][i], attr_format[i]);
  misc->pfm_handle[l2] = pfm_handle;
  misc->average_type[l2] = average_type;
  misc->cube_attr_available[l2] = cube_attr_available;
  misc->pfm_alpha[l2] = pfm_alpha;
  misc->last_saved_contour_record[l2] = last_saved_contour_record;
}



//!  Remove PFM layer l1.

void remove_layer (MISC *misc, NV_INT32 l1)
{
  close_pfm_file (misc->pfm_handle[l1]);


  //  Move everything below the selected layer up one level

  for (NV_INT32 pfm = l1 ; pfm < misc->abe_share->pfm_count - 1 ; pfm++)
    {
      misc->abe_share->open_args[pfm] = misc->abe_share->open_args[pfm + 1];
      misc->abe_share->display_pfm[pfm] = misc->abe_share->display_pfm[pfm + 1];

      misc->ss_cell_size_x[pfm] = misc->ss_cell_size_x[pfm + 1];
      misc->ss_cell_size_y[pfm] = misc->ss_cell_size_y[pfm + 1];
      for (NV_INT32 i = 0 ; i < misc->abe_share->open_args[pfm + 1].head.num_bin_attr ; i++)
	strcpy (misc->attr_format[pfm][i], misc->attr_format[pfm + 1][i]);
      misc->pfm_handle[pfm] = misc->pfm_handle[pfm + 1];
      misc->average_type[pfm] = misc->average_type[pfm + 1];
      misc->cube_attr_available[pfm] = misc->cube_attr_available[pfm + 1];
      misc->pfm_alpha[pfm] = misc->pfm_alpha[pfm + 1];
    }

  misc->abe_share->pfm_count--;
}



//!  Move PFM layer l1 to the top layer.

void move_layer_to_top (MISC *misc, NV_INT32 l1)
{
  //  Save the information for the layer we selected

  PFM_OPEN_ARGS open_args = misc->abe_share->open_args[l1];
  NV_BOOL display_pfm = misc->abe_share->display_pfm[l1];

  NV_FLOAT64 ss_cell_size_x = misc->ss_cell_size_x[l1];
  NV_FLOAT64 ss_cell_size_y = misc->ss_cell_size_y[l1];
  NV_CHAR attr_format[NUM_ATTR][20];
  NV_INT32 num_bin_attr = misc->abe_share->open_args[l1].head.num_bin_attr;
  for (NV_INT32 i = 0 ; i < num_bin_attr ; i++) strcpy (attr_format[i], misc->attr_format[l1][i]);
  NV_INT32 pfm_handle = misc->pfm_handle[l1];
  NV_INT32 average_type = misc->average_type[l1];
  NV_BOOL cube_attr_available = misc->cube_attr_available[l1];
  NV_INT32 pfm_alpha = misc->pfm_alpha[l1];
  NV_INT32 last_saved_contour_record = misc->last_saved_contour_record[l1];


  //  Move everything above the selected layer down one level

  for (NV_INT32 pfm = l1 ; pfm > 0 ; pfm--)
    {
      misc->abe_share->open_args[pfm] = misc->abe_share->open_args[pfm - 1];
      misc->abe_share->display_pfm[pfm] = misc->abe_share->display_pfm[pfm - 1];

      misc->ss_cell_size_x[pfm] = misc->ss_cell_size_x[pfm - 1];
      misc->ss_cell_size_y[pfm] = misc->ss_cell_size_y[pfm - 1];
      for (NV_INT32 i = 0 ; i < misc->abe_share->open_args[pfm - 1].head.num_bin_attr ; i++)
	strcpy (misc->attr_format[pfm][i], misc->attr_format[pfm - 1][i]);
      misc->pfm_handle[pfm] = misc->pfm_handle[pfm - 1];
      misc->average_type[pfm] = misc->average_type[pfm - 1];
      misc->cube_attr_available[pfm] = misc->cube_attr_available[pfm - 1];
      misc->pfm_alpha[pfm] = misc->pfm_alpha[pfm - 1];
      misc->last_saved_contour_record[pfm] = misc->last_saved_contour_record[pfm - 1];
    }


  //  Set the 0 layer to be the selected layer

  misc->abe_share->open_args[0] = open_args;
  misc->abe_share->display_pfm[0] = display_pfm;

  misc->ss_cell_size_x[0] = ss_cell_size_x;
  misc->ss_cell_size_y[0] = ss_cell_size_y;
  for (NV_INT32 i = 0 ; i < num_bin_attr ; i++) strcpy (misc->attr_format[0][i], attr_format[i]);
  misc->pfm_handle[0] = pfm_handle;
  misc->average_type[0] = average_type;
  misc->cube_attr_available[0] = cube_attr_available;
  misc->pfm_alpha[0] = pfm_alpha;
  misc->last_saved_contour_record[0] = last_saved_contour_record;
}



//!  Move PFM layer l1 to the bottom layer.

void move_layer_to_bottom (MISC *misc, NV_INT32 l1)
{
  //  Save the information for the layer we selected

  PFM_OPEN_ARGS open_args = misc->abe_share->open_args[l1];
  NV_BOOL display_pfm = misc->abe_share->display_pfm[l1];

  NV_FLOAT64 ss_cell_size_x = misc->ss_cell_size_x[l1];
  NV_FLOAT64 ss_cell_size_y = misc->ss_cell_size_y[l1];
  NV_CHAR attr_format[NUM_ATTR][20];
  NV_INT32 num_bin_attr = misc->abe_share->open_args[l1].head.num_bin_attr;
  for (NV_INT32 i = 0 ; i < num_bin_attr ; i++) strcpy (attr_format[i], misc->attr_format[l1][i]);
  NV_INT32 pfm_handle = misc->pfm_handle[l1];
  NV_INT32 average_type = misc->average_type[l1];
  NV_BOOL cube_attr_available = misc->cube_attr_available[l1];
  NV_INT32 pfm_alpha = misc->pfm_alpha[l1];
  NV_INT32 last_saved_contour_record = misc->last_saved_contour_record[l1];


  //  Move everything below the selected layer up one level

  for (NV_INT32 pfm = l1 ; pfm < misc->abe_share->pfm_count - 1 ; pfm++)
    {
      misc->abe_share->open_args[pfm] = misc->abe_share->open_args[pfm + 1];
      misc->abe_share->display_pfm[pfm] = misc->abe_share->display_pfm[pfm + 1];

      misc->ss_cell_size_x[pfm] = misc->ss_cell_size_x[pfm + 1];
      misc->ss_cell_size_y[pfm] = misc->ss_cell_size_y[pfm + 1];
      for (NV_INT32 i = 0 ; i < misc->abe_share->open_args[pfm + 1].head.num_bin_attr ; i++)
	strcpy (misc->attr_format[pfm][i], misc->attr_format[pfm + 1][i]);
      misc->pfm_handle[pfm] = misc->pfm_handle[pfm + 1];
      misc->average_type[pfm] = misc->average_type[pfm + 1];
      misc->cube_attr_available[pfm] = misc->cube_attr_available[pfm + 1];
      misc->pfm_alpha[pfm] = misc->pfm_alpha[pfm + 1];
      misc->last_saved_contour_record[pfm] = misc->last_saved_contour_record[pfm + 1];
    }


  //  Set the bottom [misc->abe_share->pfm_count - 1] layer to be the selected layer

  misc->abe_share->open_args[misc->abe_share->pfm_count - 1] = open_args;
  misc->abe_share->display_pfm[misc->abe_share->pfm_count - 1] = display_pfm;

  misc->ss_cell_size_x[misc->abe_share->pfm_count - 1] = ss_cell_size_x;
  misc->ss_cell_size_y[misc->abe_share->pfm_count - 1] = ss_cell_size_y;
  for (NV_INT32 i = 0 ; i < num_bin_attr ; i++) strcpy (misc->attr_format[misc->abe_share->pfm_count - 1][i], attr_format[i]);
  misc->pfm_handle[misc->abe_share->pfm_count - 1] = pfm_handle;
  misc->average_type[misc->abe_share->pfm_count - 1] = average_type;
  misc->cube_attr_available[misc->abe_share->pfm_count - 1] = cube_attr_available;
  misc->pfm_alpha[misc->abe_share->pfm_count - 1] = pfm_alpha;
  misc->last_saved_contour_record[misc->abe_share->pfm_count - 1] = last_saved_contour_record;
}
