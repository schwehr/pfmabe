
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

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef __UTILITY_NVDEFS_H__
#define __UTILITY_NVDEFS_H__

#ifdef  __cplusplus
extern "C" {
#endif


/* WGS-84 semi-major & semi-minor axes, resp. */

#define         NV_A0           6378137.0L
#define         NV_B0           6356752.314245L

#define         NV_RF           298.257223563L

#define         NV_DEG_TO_RAD   0.017453293L
#define         NV_RAD_TO_DEG   57.2957795147195L


#ifndef LOG2
  #define       LOG2            0.3010299956639812L
#endif

#ifndef NINT
  #define       NINT(a)         ((a) < 0.0 ? (int) ((a) - 0.5) : (int) ((a) + 0.5))
#endif

#ifndef SIGN
  #define       SIGN(a)         ((a) < 0.0 ? -1.0 : 1.0)
#endif

#ifndef MAX
  #define       MAX(x,y)        (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
  #define       MIN(x,y)        (((x) < (y)) ? (x) : (y))
#endif

#ifndef DPRINT
  #define       DPRINT          fprintf (stderr, "%s %d\n", __FILE__, __LINE__);fflush (stderr);
#endif


#ifdef  __cplusplus
}
#endif

#endif
