
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



#include "cosang.hpp"
#include <cmath>

/***************************************************************************/
/*!

  - Module Name:        sunshade

  - Programmer(s):      David B. Johanson

  - Date Written:       March 29, 1998

  - Purpose:            Determine the shading factor for a given grid cell.
                        This is a floating point number in the range of
                        0.0 to 1.0, adjusted based on the amount of sun
                        illumination and the number of shades.  Based
                        loosely on code supplied by NRL.  Uses sun shading
                        functions supplied by NRL.

  - Arguments:
                        - lower_row  =  IN:  current row of input file
                        - upper_row  =  IN:  next row of input file (above)
                        - col_num    =  IN:  current column number
                        - sun_opts   =  IN:  struct containing sunshade opts

  - Return Value:       None

****************************************************************************/

NV_FLOAT32 sunshade (NV_FLOAT32 *lower_row, NV_FLOAT32 *upper_row, NV_INT32 col_num, SUN_OPT *sunopts, NV_FLOAT64 x_cell_size, NV_FLOAT64 y_cell_size)
{
    NV_F64_COORD3         vertex[3];
    NV_FLOAT64            cosine;
    NV_FLOAT32            shade_factor;
    NV_INT32              shade;


    vertex[0].x = 0.0;
    vertex[0].y = 0.0;
    vertex[0].z = -((NV_FLOAT64) upper_row[col_num] * sunopts->exag);
    
    vertex[1].x = 0.0;
    vertex[1].y = -y_cell_size;
    vertex[1].z = -((NV_FLOAT64) lower_row[col_num] * sunopts->exag);

    vertex[2].x = x_cell_size;
    vertex[2].y = 0.0;
    vertex[2].z = -((NV_FLOAT64) upper_row[col_num + 1] * sunopts->exag);

    if ((cosine = cosang (vertex, sunopts->sun)) < 0.0)
    {
       return (-1.0);
    }
    
    cosine = pow (cosine, sunopts->power_cos);
    shade  = (NV_INT32) (cosine * (NV_FLOAT64) sunopts->num_shades + 0.5);
    shade_factor = (NV_FLOAT32) shade / (NV_FLOAT32) sunopts->num_shades;

    return (shade_factor);
}



/***************************************************************************/
/*!

  - Module Name:        sunshade_null

  - Programmer(s):      David B. Johanson

  - Date Written:       March 29, 1998

  - Purpose:            Determine the shading factor for a given grid cell.
                        This is a floating point number in the range of
                        0.0 to 1.0, adjusted based on the amount of sun
                        illumination and the number of shades.  Based
                        loosely on code supplied by NRL.  Uses sun shading
                        functions supplied by NRL.

  - Arguments:
                        - lower_left =  IN:  lower left value in triangle
                        - upper_left =  IN:  upper left value in triangle
                        - upper_right=  IN:  upper right value in triangle
                        - null_value =  IN:  null value
                        - sun_opts   =  IN:  struct containing sunshade opts
                        - cell_size_x=  IN:  cell size in X
                        - cell_size_y=  IN:  cell size in Y

  - Return Value:       None

  - History:            Modified for direct input of three values and null
                        JCD - 04/06/04

****************************************************************************/

NV_FLOAT32 sunshade_null (NV_FLOAT32 lower_left, NV_FLOAT32 upper_left, NV_FLOAT32 upper_right, NV_FLOAT32 null_value, SUN_OPT *sunopts, NV_FLOAT64 cell_size_x, 
                          NV_FLOAT64 cell_size_y)
{
  NV_F64_COORD3         vertex[3];
  NV_FLOAT64            cosine;
  NV_FLOAT32            shade_factor, ul, ur;
  NV_INT32              shade;


  ul = upper_left;
  ur = upper_right;
  if (ul == null_value) ul = lower_left;
  if (ur == null_value) ur = lower_left;

  vertex[0].x = 0.0;
  vertex[0].y = 0.0;
  vertex[0].z = -((NV_FLOAT64) ul * sunopts->exag);
    
  vertex[1].x = 0.0;
  vertex[1].y = -cell_size_y;
  vertex[1].z = -((NV_FLOAT64) lower_left * sunopts->exag);

  vertex[2].x = cell_size_x;
  vertex[2].y = 0.0;
  vertex[2].z = -((NV_FLOAT64) ur * sunopts->exag);

  if ((cosine = cosang (vertex, sunopts->sun)) < 0.0) return (-1.0);
    
  cosine = pow (cosine, sunopts->power_cos);
  shade  = (NV_INT32) (cosine * (NV_FLOAT64) sunopts->num_shades + 0.5);
  shade_factor = (NV_FLOAT32) shade / (NV_FLOAT32) sunopts->num_shades;

  return (shade_factor);
}
