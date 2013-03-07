
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
#include <errno.h>
#include <string.h>

#include "nvutility.h"

#include "pfm.h"
#include "gsf.h"
#include "unisips_io.h"

NV_INT32 get_file_type (NV_CHAR *path)
{
  NV_CHAR          *ptr;
  NV_INT32         gsfhnd, swap;
  FILE             *uni_fp;
  UNISIPS_HEADER   unisips_header;



  /*  Check for "deleted" files.  */

  if (path[0] == '*') return (-1);


  //  Make sure the file exists.

  if ((uni_fp = fopen (path, "rb")) == NULL) return (-2);
  fclose (uni_fp);


  ptr = strrchr (path, '/');

  if (ptr == NULL) ptr = path;


  /*  Check for GSF file.  */

  if (!gsfOpen (path, GSF_READONLY, &gsfhnd))
    {
      gsfClose (gsfhnd);
      return (PFM_GSF_DATA);
    }


  /*  Check for WLF (.wlf) file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".wlf") || !strcmp (&path[strlen (path) - 4], ".wtf") ||
      !strcmp (&path[strlen (path) - 4], ".whf")) return (PFM_WLF_DATA);


  /*  Check for HAWKEYE (.bin) file names.  */

  if (!strcmp (&path[strlen (path) - 7], "_HD.bin")) return (PFM_HAWKEYE_HYDRO_DATA);
  if (!strcmp (&path[strlen (path) - 7], "_TD.bin")) return (PFM_HAWKEYE_TOPO_DATA);


  /*  Check for BAG (.bag) file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".bag")) return (PFM_BAG_DATA);


  /*  Check for SHOALS .hof file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".hof")) return (PFM_CHARTS_HOF_DATA);


  /*  Check for SHOALS .tof file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".tof")) return (PFM_SHOALS_TOF_DATA);


  /*  Check for Hypack yxz file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".txt") || !strcmp (&path[strlen (path) - 4], ".yxz") || 
      !strcmp (&path[strlen (path) - 4], ".raw")) return (PFM_NAVO_ASCII_DATA);


  /*  Check for IVS or other xyz file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".xyz")) return (PFM_ASCXYZ_DATA);


  /*  Check for NAVO LLZ file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".llz")) return (PFM_NAVO_LLZ_DATA);


  /*  Check for DTED file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".dt1") || !strcmp (&path[strlen (path) - 4], ".dt2")) return (PFM_DTED_DATA);


  /*  Check for CHRTR file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".fin") || !strcmp (&path[strlen (path) - 4], ".ch2")) return (PFM_CHRTR_DATA);


  /*  Check for CZMIL file names.  */

  if (!strcmp (&path[strlen (path) - 4], ".cxy")) return (PFM_CZMIL_DATA);


  //  Check for UNISIPS file

  if ((uni_fp = fopen (path, "rb")) != NULL)
    {
      /* Read UNISIPS header information          */

      if (read_unisips_header (uni_fp, &unisips_header, &swap))
        {
          fclose (uni_fp);
          return (PFM_UNISIPS_DEPTH_DATA);
        }
    }


  /*  Can't figure this one out.  */

  return (PFM_UNDEFINED_DATA);
}
