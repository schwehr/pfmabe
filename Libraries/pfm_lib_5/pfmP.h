
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


#ifndef __PFMP_H
#define __PFMP_H

#ifdef  __cplusplus
extern "C" {
#endif


/*!
   
   - The pfmP.h file contains the private header file components of
   the PFM API. These functions are required internally but are not
   directly exposed to the user. The public pfm.h file exposes the
   set of API functions availble to applications that use the 
   libraries.
   
   - Mark A. Paton

*/

   
/*!  Bin record size and bit offsets in the bit-packed index record buffer.  */

typedef struct
{
  NV_U_INT32                  record_size;             /*!<  Record size in bytes  */
  NV_U_INT32                  num_soundings_pos;       /*!<  Bit position of the number of soundings in the record buffer  */
  NV_U_INT32                  std_pos;                 /*!<  Bit position of the standard deviation  */
  NV_U_INT32                  avg_filtered_depth_pos;  /*!<  Bit position of the average filtered depth  */
  NV_U_INT32                  min_filtered_depth_pos;  /*!<  Bit position of the minimum filtered depth  */
  NV_U_INT32                  max_filtered_depth_pos;  /*!<  Bit position of the maximum filtered depth  */
  NV_U_INT32                  avg_depth_pos;           /*!<  Bit position of the average unfiltered depth  */
  NV_U_INT32                  min_depth_pos;           /*!<  Bit position of the minimum unfiltered depth  */
  NV_U_INT32                  max_depth_pos;           /*!<  Bit position of the maximum unfiltered depth  */
  NV_U_INT32                  edited_flag_pos;         /*!<  Bit position of the PFM_EDITED flag (pre 4.0)  */
  NV_U_INT32                  checked_flag_pos;        /*!<  Bit position of the PFM_CHECKED flag (pre 4.0)  */
  NV_U_INT32                  suspect_flag_pos;        /*!<  Bit position of the PFM_SUSPECT flag (pre 4.0)  */
  NV_U_INT32                  data_flag_pos;           /*!<  Bit position of the PFM_DATA flag (pre 4.0)  */
  NV_U_INT32                  selected_flag_pos;       /*!<  Bit position of the PFM_SELECTED flag (pre 4.0)  */
  NV_U_INT32                  class1_flag_pos;         /*!<  Bit position of the PFM_CLASS1 flag (pre 4.0)  */
  NV_U_INT32                  class2_flag_pos;         /*!<  Bit position of the PFM_CLASS2 flag (pre 4.0)  */
  NV_U_INT32                  projected_x_pos;         /*!<  Bit position of the projected X offset (pre 4.0)  */
  NV_U_INT32                  attr_pos[NUM_ATTR];      /*!<  Bit position of the attributes  */
  NV_U_INT32                  validity_pos;            /*!<  Bit position of the validity data  */
  NV_U_INT32                  head_pointer_pos;        /*!<  Bit position of the depth chain head pointer  */
  NV_U_INT32                  tail_pointer_pos;        /*!<  Bit position of the depth chain tail pointer  */
} BIN_RECORD_OFFSETS;

/* MISSING FUCTION DECLARATIONS (from documents)   */
/*
create_list_file
set_offsets
open_bin
close_bin
write_depth_buffer
open_index
close_index
cfg_in
open_line_file
create_line_file
checkpoint_pfm_file
recover_pfm_file
get_file_type
read_bin_record
write_bin_record
read_depth_record
update_depth_record
add_depth_record
recompute_bin_values
change_depth_record
*/

/* **** Internal Functions (I believe - MP) **** */

/* NV_INT32 bin_inside (BIN_HEADER bin, NV_F64_COORD2 xy);  - Declared in pfm.h already? */

static NV_INT32 update_cov_map (NV_INT32 hnd, NV_INT64 address);
static void destroy_depth_buffer ( NV_INT32 hnd, DEPTH_SUMMARY *depth);
static void prepare_cached_depth_buffer (NV_INT32 hnd, DEPTH_SUMMARY *depth, NV_U_BYTE *depth_buffer, NV_BOOL preallocateBuffer );
static NV_INT32 write_cached_depth_buffer (NV_INT32 hnd, NV_U_BYTE *buffer, NV_INT64 address);
static NV_INT32 write_cached_depth_summary (NV_INT32 hnd, BIN_RECORD_SUMMARY *depth);
static NV_INT32 pack_bin_record (NV_INT32 hnd,  NV_U_BYTE *bin_data, BIN_RECORD *bin, BIN_RECORD_OFFSETS *offsets, BIN_HEADER_DATA *hd,
                                 NV_INT16 list_file_ver, CHAIN *depth_chain);
static NV_INT32 pack_depth_record (NV_U_BYTE *depth_buffer, DEPTH_RECORD *depth, NV_INT32 record_pos, NV_INT32 hnd);

#ifdef  __cplusplus
}
#endif

#endif
