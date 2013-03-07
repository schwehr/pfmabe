
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cerrno>
#include <cmath>

#include "nvutility.h"

#include "pfm.h"

#include "check_flag.h"


/***************************************************************************\
*                                                                           *
*   Module Name:        gsf_to_pfm_flags                                    *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       March 2000                                          *
*                                                                           *
*   Purpose:            Converts GSF flags to PFM flags.                    *
*                                                                           *
*   Arguments:          pfm_flags                                           *
*                       gsf_flags                                           *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
\***************************************************************************/

void gsf_to_pfm_flags (NV_U_INT32 *pfm_flags, NV_U_BYTE gsf_flags)
{
  *pfm_flags = 0;


  if (check_flag (gsf_flags, HMPS_IGNORE_MANUALLY_EDITED)) *pfm_flags |= PFM_MANUALLY_INVAL;

  if (check_flag (gsf_flags, HMPS_IGNORE_FILTER_EDITED)) *pfm_flags |= PFM_FILTER_INVAL;

  if (check_flag (gsf_flags, HMPS_IGNORE_FOOTPRINT_TOO_BIG)) *pfm_flags |= (PFM_FILTER_INVAL | PFM_USER_04);

  if (check_flag (gsf_flags, HMPS_IGNORE_NOT_2X_IHO)) *pfm_flags |= (PFM_FILTER_INVAL | PFM_USER_03);

  if (check_flag (gsf_flags, HMPS_INFO_NOT_1X_IHO))
    {
      *pfm_flags |= PFM_USER_02;
    }
  else
    {
      if (!(*pfm_flags & PFM_INVAL)) *pfm_flags |= PFM_USER_01;
    }

  if (check_flag (gsf_flags, HMPS_SELECTED_LEAST)) *pfm_flags |= PFM_SELECTED_SOUNDING;

  if (check_flag (gsf_flags, HMPS_SELECTED_CONTACT)) *pfm_flags |= PFM_SELECTED_FEATURE;

  if (check_flag (gsf_flags, HMPS_SELECTED_SPARE_1)) *pfm_flags |= PFM_SUSPECT;
}



/***************************************************************************\
*                                                                           *
*   Module Name:        pfm_to_gsf_flags                                    *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       March 2000                                          *
*                                                                           *
*   Purpose:            Converts PFM flags to GSF flags.                    *
*                                                                           *
*   Arguments:          pfm_flags                                           *
*                       gsf_flags                                           *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
\***************************************************************************/

void pfm_to_gsf_flags (NV_U_INT32 pfm_flags, NV_U_BYTE *gsf_flags)
{
  *gsf_flags = 0;


  if ((pfm_flags & PFM_MANUALLY_INVAL) || (pfm_flags & PFM_FILTER_INVAL))
    {
      if (pfm_flags & PFM_MANUALLY_INVAL)
        {
          set_flag (gsf_flags, HMPS_IGNORE_MANUALLY_EDITED);
        }
      else
        {
          set_flag (gsf_flags, HMPS_IGNORE_FILTER_EDITED);
        }
    }
  else
    {
      if (pfm_flags & PFM_SELECTED_SOUNDING) set_flag (gsf_flags, HMPS_SELECTED_LEAST);

      if (pfm_flags & PFM_SUSPECT) set_flag (gsf_flags, HMPS_SELECTED_SPARE_1);

      if (pfm_flags & PFM_SELECTED_FEATURE) set_flag (gsf_flags, HMPS_SELECTED_CONTACT);
    }

  if (pfm_flags & PFM_USER_02) set_flag (gsf_flags, HMPS_INFO_NOT_1X_IHO);

  if (pfm_flags & PFM_USER_03) set_flag (gsf_flags, HMPS_IGNORE_NOT_2X_IHO);

  if (pfm_flags & PFM_USER_04) set_flag (gsf_flags, HMPS_IGNORE_FOOTPRINT_TOO_BIG);
}
