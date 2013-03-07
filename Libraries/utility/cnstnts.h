
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



/* Constants  */

#ifndef _CNSTNTS_H
#define _CNSTNTS_H


#ifdef  __cplusplus
extern "C" {
#endif


#ifndef OK
#define OK    (0)
#endif

/* Mathematical constants */
#include <math.h>
#include <float.h>
#ifndef M_PI
#define M_PI        (3.14159265358979323846) 
#endif
#ifndef M_PI_2
#define M_PI_2      (1.57079632679489661923)
#endif
#ifndef M_PI_4
#define M_PI_4      (0.78539816339744830962)
#endif

#define PI           M_PI
#define HALF_PI      M_PI_2
#define TWO_PI      (M_PI+M_PI)

#ifndef RADDEG
#define RADDEG      (M_PI/180.0)
#endif
#ifndef EPSILON
#define EPSILON     (0.0000001)
#endif

/* NULL constants */
#ifndef DOUBLE_NULL
#define DOUBLE_NULL     (-1.7976931348623158e+304)     /*!< minimum double8 value */ 
#endif
#ifndef FLOAT_NULL
#define FLOAT_NULL      (-3.402823466e+38F)            /*!< minimum float value */ 
#endif
#ifndef SHORT_NULL
#define SHORT_NULL      (-32767)                       /*!< minimum short value */
#endif
#ifndef LONG_NULL
#define LONG_NULL       (-2147483647)                  /*!< minimum long value */
#endif


#ifdef  __cplusplus
}
#endif

#endif /* _CNSTNTS_H */
