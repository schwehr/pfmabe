
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



#ifndef __LOAD_FILE_H_
#define __LOAD_FILE_H_


/*  CHARTS header files.  */

#include "FileHydroOutput.h"
#include "FileTopoOutput.h"


#include "nvutility.h"

#include "wlf.h"
#include "hawkeye.h"
#include "gsf.h"
#include "gsf_indx.h"

#include "pfm.h"


/*  LLZ header files.  */

#include "llz.h"


/*  CZMIL header files.  */

#include "czmil.h"


/*  BAG header files.  */

#include "bag.h"
#include "bag_xml_meta.h"


#include "hmpsflag.h"


#define MAX_LOAD_FILES    30


typedef struct PFM_Load_Parameters 
{
  FLAGS                  flags;
  PFM_GLOBAL             pfm_global;
} PFM_LOAD_PARAMETERS;


NV_BOOL check_gsf_file (NV_CHAR *path);
NV_BOOL check_wlf_file (NV_CHAR *path);
NV_BOOL check_hawkeye_file (NV_CHAR *path);
NV_BOOL check_hof_file (NV_CHAR *path);
NV_BOOL check_tof_file (NV_CHAR *path);
NV_BOOL check_unisips_depth_file (NV_CHAR *path);
NV_BOOL check_hypack_xyz_file (NV_CHAR *path);
NV_BOOL check_llz_file (NV_CHAR *path);
NV_BOOL check_czmil_file (NV_CHAR *path);
NV_BOOL check_bag_file (NV_CHAR *path);
NV_BOOL check_ivs_xyz_file (NV_CHAR *path);
NV_BOOL check_dted_file (NV_CHAR *path);
NV_BOOL check_chrtr_file (NV_CHAR *path);

NV_INT32 load_gsf_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_wlf_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_hawkeye_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_hof_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_tof_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_unisips_depth_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_hypack_xyz_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_llz_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_czmil_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_bag_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_ivs_xyz_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_srtm_file (NV_INT32 pfm_fc, NV_INT32 *count, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_dted_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);
NV_INT32 load_chrtr_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[]);

void Do_PFM_Processing (NV_INT32 beam, NV_FLOAT64 depth, NV_F64_COORD2 nxy, NV_U_INT32 flags, NV_FLOAT64 heading, NV_BOOL force,
                        NV_FLOAT32 *l_attr, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[], NV_INT32 percent);

#endif
