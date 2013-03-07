
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



/***************************************************************************/
/*!

  - Programmer(s):      Dominic Avery, Jan C. Depner

  - Date Written:       April 1992

  - Module Name:        spline

  - Purpose:            Computes 53 point bicubic spline interpolated data
                        from input data.

  - Inputs:
                        - x               =   input x data
                        - y               =   input y data
                        - pos             =   position in array
                        - x_pos           =   x value
                        - y_pos           =   y value computed
                        - ater            =

  - Outputs:
                        - void

  - Calling Routines:   interpolate

  - Glossary:
                        - endloop1        =   end of loop flag
                        - endloop2        =   end of loop flag
                        - coeffs          =   coefficient array
                        - valpos          =

  - Method:             bicubic spline

****************************************************************************/

#include <stdio.h>
#include <cmath>
#include "interp.hpp"
#include "nvtypes.h"


void spline (NV_FLOAT32 *x, NV_FLOAT32 *y, NV_INT32 pos, NV_FLOAT32 x_pos, NV_FLOAT32 *y_pos, NV_FLOAT32 *ater)
{
  NV_FLOAT32          coeffs[SPLINE_ROW + 1][SPLINE_COL];
  static NV_INT32     valpos;
                
  void    spline_cof (NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_FLOAT32 *);


  if ((*x + *(y + pos) + *(y + (pos - 1)) + *(x + (pos - 1)) + *(y + (pos - 2)) - *ater) != 0.0)
    {
      spline_cof (x, y, pos, &coeffs[0][0]);

      *ater = *x + *(y + pos) + *(y + (pos - 1)) + *(x + (pos - 1)) + *(y + (pos - 2));
      valpos = 0;
    }

  NV_BOOL endloop1 = NVFalse;
  NV_BOOL endloop2 = NVFalse;

  while (!endloop1)
    {
      endloop1 = NVTrue;
      if (fabs ((NV_FLOAT64) (x_pos - *x)) >= 0.00001)
        {
            if ((x_pos - *x) < 0) 
              {
                valpos = 0;

               
                //  Extrapolate with average slope of closest two intervals.

                *y_pos = *y + (x_pos - *x) * ((*(y + 1) - *y) / (*(x + 1) - *x) + (*(y + 2) - *(y + 1)) / 
                                              (*(x + 2) - *(x + 1))) * 0.5;

                return;
              }
            else if ((x_pos - *x) == 0) 
              {
                *y_pos = *y;
                return;
              }
            else
              {
                if ((fabs ((NV_FLOAT64) (x_pos - *(x + (valpos + 1))))) >= 0.00001)
                  {
                    if ((x_pos - *(x + (valpos + 1))) < 0)
                      {
                        while (!endloop2)
                          {
                            endloop2 = NVTrue;
                            if ((fabs ((NV_FLOAT64) (x_pos - *(x + (valpos + 1))))) >= 0.00001)
                              {
                                if (fabs ((NV_FLOAT64) (x_pos - *(x + valpos))) < 0)
                                  {
                                    valpos--;
                                    endloop2 = NVFalse;
                                  }
                                else if (fabs ((NV_FLOAT64) (x_pos - *(x + valpos))) == 0)
                                  {
                                    *y_pos = *(y + valpos);
                                    return;
                                  }
                                else
                                  {
                                    *y_pos = (*(x + (valpos + 1)) - x_pos) * (coeffs[0][valpos] *
                                                                              (*(x + (valpos + 1)) - x_pos) * 
                                                                              (*(x + (valpos + 1)) - x_pos) + 
                                                                              coeffs[2][valpos]);

                                    *y_pos += (x_pos - *(x + valpos)) * (coeffs[1][valpos] * 
                                                                         (x_pos - *(x + valpos)) *
                                                                         (x_pos - *(x + valpos)) +
                                                                         coeffs[3][valpos]);
                                    return;
                                  }
                              }
                            else
                              {
                                *y_pos = *(y + valpos);
                                return;
                              }
                          }
                      }
                    else if ((x_pos - *(x + (valpos + 1))) == 0)
                      {
                        *y_pos = *(y + (valpos + 1));
                        return;
                      }
                    else
                      {
                        valpos++;
                        if ((pos - valpos) > 0)
                          {
                            endloop1 = NVFalse;
                          }
                        else
                          {
                            valpos = pos - 1;
    
                    
                            //  Extrapolate with average slope of closest two intervals.
    
                            if (valpos == 1)
                              {
                                *y_pos = *y + (x_pos - *x) * ((*(y + 1) - *y) / (*(x + 1) - *x) + 
                                                              (*(y + 2) - *(y + 1)) / (*(x + 2) - *(x + 1))) * 0.5;
                              }
                            else
                              {
                                *y_pos = *(y + pos) + (x_pos - *(x + pos)) * 
                                  ((*(y + valpos) - *(y + valpos - 1)) / 
                                   (*(x + valpos) - *(x + (valpos - 1))) +
                                   (*(y + pos) - *(y + valpos)) /
                                   (*(x + pos) - *(x + valpos))) * 0.5;
                              }
                            return;
                          }
                      }
                  }
                else
                  {
                    *y_pos = *(y + (valpos + 1));
                    return;
                  }
              }
        }
      else
        {
          *y_pos = *y;
          return;
        }
    }
}
