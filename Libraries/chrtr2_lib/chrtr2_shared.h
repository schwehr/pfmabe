
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



#ifndef _CHRTR2_SHARED_H_
#define _CHRTR2_SHARED_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include "nvtypes.h"
#include "chrtr2.h"


#define         CHRTR2_NULL_Z_VALUE         1000000.0  /*!<  Null value  */


#define         MAX_CHRTR2_FILES            16   /*!<  Maximum number of CHRTR2 layers for gridView and gridEdit  */
#define         MAX_CHRTR2_CONTOUR_LEVELS   200  /*!<  Maximum number of contour levels  */


  /*  Command codes for CHRTR2 programs.  */

#define         CHRTR2_LAYERS_CHANGED       6978 /*!<  Set whenever gridEdit or gridPointEdit change CHRTR2 layers, display of
                                                       CHRTR2 layers, or order of CHRTR2 layers (for multiple CHRTR2 display
                                                       in gridEdit or gridPointEdit)  */
#define         GRID3D_FORCE_RELOAD         6979 /*!<  Force grid3D to reload and redraw its display (changed display
                                                       in gridEdit).  */
#define         GRIDPOINTEDIT3D_OPENED      6983 /*!<  Set when gridPointEdit3D starts.  Used to clear grid3D memory.  */
#define         GRIDPOINTEDIT3D_CLOSED      6984 /*!<  Set when gridPointEdit3D exits.  Used to reload grid3D memory.  */




  /*  This structure is used for shared memory Inter-Process Communication (IPC) between CHRTR2 GUI programs (gridEdit, gridPointEdit,
      gridPointEdit3D, and grid3D at the moment).  */

  typedef struct
  {
    NV_BOOL       settings_changed;           /*!<  Set if gridPointEdit has changed any of the shared settings  */
    NV_BOOL       zoom_requested;             /*!<  Set if gridPointEdit has requested a mosaicView zoom (so gridEdit can
                                                    inform any linked apps of the zoom request).   */
    NV_INT32      position_form;              /*!<  Position format number (position formats are described in MISC)  */
    NV_INT32      smoothing_factor;           /*!<  Contour smoothing factor (0-10)  */
    NV_FLOAT32    z_factor;                   /*!<  Z scaling factor  */
    NV_FLOAT32    z_offset;                   /*!<  Z offset value  */
    NV_INT32      active_window_id;           /*!<  Process ID of the active window (set by the mouseMove function)  */
    NV_F64_COORD3 cursor_position;            /*!<  Position of the cursor (and possibly depth) in the application  */
    NV_INT32      polygon_count;              /*!<  Number of points in polygon  */
    NV_FLOAT64    polygon_x[2000];            /*!<  Polygon X positions  */
    NV_FLOAT64    polygon_y[2000];            /*!<  Polygon Y positions  */
    NV_F64_XYMBR  edit_area;                  /*!<  Total rectangular area to be edited  */
    NV_INT32      ppid;                       /*!<  Parent process ID (gridEdit)  */
    NV_FLOAT32    cint;                       /*!<  Current contour interval (0 for user defined)  */
    NV_FLOAT32    contour_levels[MAX_CHRTR2_CONTOUR_LEVELS]; /*!<  User defined contour levels  */
    NV_INT32      num_levels;                 /*!<  Number of user defined contour levels   */
    NV_F64_XYMBR  displayed_area;             /*!<  Displayed area in gridPointEdit(3D)  */
    NV_F64_XYMBR  viewer_displayed_area;      /*!<  Displayed area in gridEdit  */
    NV_CHAR       chrtr2_file[MAX_CHRTR2_FILES][512]; /*!<  CHRTR2 file names  */
    CHRTR2_HEADER chrtr2_header[MAX_CHRTR2_FILES]; /*!<  CHRTR2 headers  */
    NV_BOOL       display_chrtr2[MAX_CHRTR2_FILES]; /*!<  Whether the CHRTR2 is visible (if not, don't use it).  */
    NV_INT32      chrtr2_count;                  /*!<  Number of currently open CHRTR2 files (layers).  */
    NV_U_INT32    key;                        /*!<  Key or command code.  */
  } CHRTR2_SHARE;


#ifdef  __cplusplus
}
#endif

#endif
