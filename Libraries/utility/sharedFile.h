
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



#ifndef __SHARED_FILE__
#define __SHARED_FILE__


#ifdef  __cplusplus
extern "C" {
#endif


#include "nvtypes.h"


#define SHARED_FILE_VERSION         2.1     /* 04/12/11 */

#define SHARED_LINE_MULT            65536


  /*! <pre>

      This structure is used in pfmEdit and ancillary programs to share data by passing it in a file.  The file will be
      ASCII and the format will be as follows:

      file name from PFM list file
      point# count X Y Z herr verr validity pfm# file# line# record# subrecord# wave_flag
      point# count X Y Z herr verr validity pfm# file# line# record# subrecord# wave_flag
      point# count X Y Z herr verr validity pfm# file# line# record# subrecord# wave_flag

      o
      o
      o

      EOD
      file name from PFM list file
      point# count X Y Z herr verr validity pfm# file# line# record# subrecord# wave_flag
      point# count X Y Z herr verr validity pfm# file# line# record# subrecord# wave_flag
      point# count X Y Z herr verr validity pfm# file# line# record# subrecord# wave_flag

      o
      o
      o

      EOD

      Wash, rinse, repeat...

      EOF



      Example:


      /data1/datasets/jcdtest/wks/05283.hof
      273 0 -89.901827650 28.367454535 14.710 0.712 0.410 0 1 15 65551 48442 52 0
      586 1 -89.901839050 28.367410226 14.740 0.745 0.410 0 1 15 65551 48406 54 0
      EOD
      /data1/datasets/jcdtest/wks/09971.hof
      121 2 -89.901833056 28.367282435 14.720 0.667 0.410 0 1 19 65555 72535 91 0
      134 3 -89.901839461 28.367310227 14.710 0.723 0.410 0 1 19 65555 72542 92 0
      547 4 -89.901852507 28.367262371 14.740 0.565 0.410 0 1 19 65555 72449 89 0
      982 5 -89.901829706 28.367244531 14.700 0.692 0.410 0 1 19 65555 72450 87 0
      EOD
      /data1/datasets/jcdtest/wks/10001.hof
      15 6 -89.901845338 28.367544899 14.740 0.701 0.410 64 1 28 65564 124047 59 0
      126 7 -89.901842341 28.367512396 14.690 0.726 0.410 0 1 28 65564 123989 58 0
      879 8 -89.901846278 28.367470840 14.700 0.687 0.410 0 1 28 65564 123950 56 0
      EOF


      The standard C format for reading these records will look like this:

      sscanf (string, "%d %d %lf %lf %f %f %f %d %hd %hd %d %d %c", &point, &count, &data.x, &data.y, &data.z, 
              &data.herr, &data.verr, &data.val, &data.pfm, &data.file, &data.rec, &data.sub, &data.wave);



      Ancillary programs that read this data will be expected (by pfmEdit) to output the modified data records, with an 
      added "modified" flag as the last value, otherwise in the same format, to stdout.  This does not include the file 
      names or the EOD/EOF records.  The only modifications that will be accepted are to Z, validity, and the wave_flag.


      Example:

      273 0 -89.901827650 28.367454535 14.710 0.712 0.410 0 1 15 65551 48442 52 0 1
      586 1 -89.901839050 28.367410226 14.740 0.745 0.410 0 1 15 65551 48406 54 0 0
      121 2 -89.901833056 28.367282435 14.720 0.667 0.410 0 1 19 65555 72535 91 0 1
      134 3 -89.901839461 28.367310227 14.710 0.723 0.410 0 1 19 65555 72542 92 0 1
      547 4 -89.901852507 28.367262371 14.740 0.565 0.410 0 1 19 65555 72449 89 0 0
      982 5 -89.901829706 28.367244531 14.700 0.692 0.410 0 1 19 65555 72450 87 0 1
      15 6 -89.901845338 28.367544899 14.740 0.701 0.410 64 1 28 65564 124047 59 0 0
      126 7 -89.901842341 28.367512396 14.690 0.726 0.410 0 1 28 65564 123989 58 0 1
      879 8 -89.901846278 28.367470840 14.700 0.687 0.410 0 1 28 65564 123950 56 0 1

                                                                             ^
                                                                             |
                                                                             |
                                                       "modified flag, 1 = modified, 0 = unchanged"


      The standard C format for writing these records will look like this:

      fprintf (stdout, "%d %d %0.9f %0.9f %0.3f %0.3f %0.3f %d %d %d %d %d %d %d\n", point, count, data.x, data.y, 
               data.z, data.herr, data.verr, data.val, data.pfm, data.file, data.rec, data.sub, data.wave, modified);


  </pre>*/



typedef struct
{
  NV_INT32    point;                     /*  pfmEdit(3D) point number */
  NV_FLOAT64  x;                         /*  X position               */
  NV_FLOAT64  y;                         /*  Y position               */
  NV_FLOAT32  z;                         /*  Z value                  */
  NV_FLOAT32  herr;                      /*  Horizontal uncertainty   */
  NV_FLOAT32  verr;                      /*  Vertical uncertainty     */
  NV_U_INT32  val;                       /*  Validity                 */
  NV_INT16    pfm;                       /*  PFM number               */
  NV_INT16    file;                      /*  File number              */
  NV_U_INT32  line;                      /*  Line number (PFM * SHARED_LINE_MULT + line)  */
  NV_U_INT32  rec;                       /*  Ping (record) number     */
  NV_INT32    sub;                       /*  Beam (subrecord) number  */
  NV_BOOL     exflag;                    /*  Flag set by external program  */
} SHARED_FILE_STRUCT;



#ifdef  __cplusplus
}
#endif


#endif
