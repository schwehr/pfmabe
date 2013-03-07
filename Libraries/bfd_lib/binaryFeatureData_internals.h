
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



#ifndef __BFDATA_INTERNALS_H__
#define __BFDATA_INTERNALS_H__

#ifdef  __cplusplus
extern "C" {
#endif


#define         DPRINT          fprintf (stderr, "%s %d\n", __FILE__, __LINE__);fflush (stderr);
#define         NINT(a)         ((a) < 0.0 ? (int) ((a) - 0.5) : (int) ((a) + 0.5))


/*!  This is the structure we use to keep track of important formatting data for an open BFD file.  */

typedef struct
{
  FILE          *fp;                        /*!<  BFD file pointer.  */
  FILE          *afp;                       /*!<  File pointer for associated polygons and images.  */
  NV_BOOL       swap;                       /*!<  NVTrue if we need to byte swap the records.  */
  NV_CHAR       path[1024];                 /*!<  File name.  */
  NV_CHAR       a_path[1024];               /*!<  Associated polygon/polyline/image file name.  */
  NV_BOOL       modified;                   /*!<  Set if the file has been modified.  */
  NV_BOOL       created;                    /*!<  Set if we created the file.  */
  NV_BOOL       write;                      /*!<  Set if the last action to the file was a write.  */
  NV_U_INT32    recnum;                     /*!<  Number of next record to read or write (for BFDATA_NEXT_RECORD).  */
  NV_U_INT32    last_rec;                   /*!<  Number of the last record read (so we might not have to re-read the
                                                  BFDATA_RECORD to get the polygon data.  */
  BFDATA_RECORD record;                     /*!<  Last BFDATA_RECORD read (not written).  */
  NV_INT16      major_version;              /*!<  Major version number for backward compatibility.  */
  NV_U_INT32    header_size;                /*!<  Header size in bytes.  */
  NV_U_INT32    record_size;                /*!<  Record size in bytes.  */
  BFDATA_SHORT_FEATURE *short_feature;      /*!<  Allocated array of truncated records for fast memory access in applications.  */
  BFDATA_HEADER    header;                  /*!<  BFD file header.  */
} INTERNAL_BFDATA_STRUCT;


/*!  BFD error handling variables.  */

typedef struct 
{
  NV_INT32      system;            /*!<  Last system error condition encountered.  */
  NV_INT32      bfd;               /*!<  Last BFD error condition encountered.  */
  NV_CHAR       file[512];         /*!<  Name of file being accessed when last error encountered (if applicable).  */
  NV_INT32      recnum;            /*!<  Record number being accessed when last error encountered (if applicable).  */
  NV_CHAR       info[128];
} BFDATA_ERROR_STRUCT;


#ifdef  __cplusplus
}
#endif


#endif


