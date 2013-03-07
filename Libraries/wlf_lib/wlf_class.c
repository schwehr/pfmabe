
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#include "wlf.h"


/*********************************************************************************************

  Function:    wlf_get_classification

  Purpose:     Returns a descriptive string associated with the classification value passed in.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     NV_CHAR       -    Error message

  Caveats:     There are, as far as I know, no standards for topo and/or bathy LIDAR
               surface/bottom classification.  Due to this we have decided to implement our
               own numbering scheme for surface/bottom classification.  We have borrowed the
               classification values that were defined in LAS 1.1 as a starting point.
               Instead of using 4 bits (like LAS) we are using 12.  This gives us a range of
               0 to 4095 as opposed to 0 to 31.  That may seem like a lot but when you start
               including things like silty-muddy-clay and muddy-clayey-silt you can see that
               we won't take long to fill up most of our range.  The only ordering that we
               want to use is to divide the range between topo and hydro LIDAR
               classifications.  So, given that, the 2 to 2048 range will be for topo
               classifications and the 2049 to 4095 range will be for bathy classifications.
               The 0 and 1 classifications will be taken from the LAS 1.1 spec and will apply
               to both bathy and topo.  The already defined classifications from the LAS spec
               are as follows:

                   0     -    Created, never classified
                   1     -    Unclassified
                   2     -    Ground
                   3     -    Low Vegetation
                   4     -    Medium Vegetation
                   5     -    High Vegetation
                   6     -    Building
                   7     -    Low Point (noise)
                   8     -    Model Key-point (mass point)
                   9     -    Water
                   10    -    Reserved for ASPRS Definition
                   11    -    Reserved for ASPRS Definition
                   12    -    Overlap Points2
                   13-31 -    Reserved for ASPRS Definition

               New definitions will be added as requested with, of course, some due
               diligence to make sure that the classification hasn't already been defined.

*********************************************************************************************/

WLF_DLL NV_CHAR *wlf_get_classification (NV_U_INT16 classification)
{
  static NV_CHAR class_type[1024];

  switch (classification)
    {
    case WLF_CLASS_NEVER_CLASSIFIED:
      strcpy (class_type, _("Created, never classified"));
      break;

    case WLF_CLASS_UNCLASSIFIED:
      strcpy (class_type, _("Unclassified"));
      break;

    case WLF_CLASS_GROUND:
      strcpy (class_type, _("Ground"));
      break;

    case WLF_CLASS_LOW_VEGETATION:
      strcpy (class_type, _("Low Vegetation"));
      break;

    case WLF_CLASS_MEDIUM_VEGETATION:
      strcpy (class_type, _("Medium Vegetation"));
      break;

    case WLF_CLASS_HIGH_VEGETATION:
      strcpy (class_type, _("High Vegetation"));
      break;

    case WLF_CLASS_BUILDING:
      strcpy (class_type, _("Building"));
      break;

    case WLF_CLASS_LOW_POINT:
      strcpy (class_type, _("Low Point (noise)"));
      break;

    case WLF_CLASS_MODEL_KEY_POINT:
      strcpy (class_type, _("Model Key-point (mass point)"));
      break;

    case WLF_CLASS_WATER:
      strcpy (class_type, _("Water"));
      break;

    case WLF_CLASS_OVERLAP_POINTS:
      strcpy (class_type, _("Overlap Points"));
      break;

    case WLF_CLASS_LAND:
      strcpy (class_type, _("Land"));
      break;

    default:
      strcpy (class_type, _("Undefined"));
      break;
    }

  return (class_type);
}

