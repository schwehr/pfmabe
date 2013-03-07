
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



/* One of the following definitions should be made at the command line:
   NVDOS, NVHP-UX, NVIRIX, NVCLIX, NVLinux, NVSUNOS, NVWINNT, NVWIN3X */


#ifndef __NVDEFS__
#define __NVDEFS__


#ifdef  __cplusplus
extern "C" {
#endif


#define NVFalse 0
#define NVTrue 1
#define NV_BOOL unsigned char
#define NV_U_CHAR unsigned char
#define NV_U_BYTE unsigned char
#define NV_CHAR char
#define NV_BYTE signed char


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined (NVWIN3X) || defined (__MINGW32__) || defined (__MINGW64__)
    #include <process.h>
    #define SEPARATOR '\\'
    #define PATH_SEPARATOR ';'
#else
    #include <libgen.h>
    #define SEPARATOR '/'
    #define PATH_SEPARATOR ':'
#endif


#if defined (NVWIN3X) || defined (__MINGW32__) || defined (__MINGW64__)
#define NV_INT64_SPECIFIER "%I64d"
#define NV_INT16 signed short
#define NV_INT32 signed int
#define NV_U_INT16 unsigned short
#define NV_U_INT32 unsigned int
typedef signed __int64 NV_INT64;
typedef unsigned __int64 NV_U_INT64;
#else
#define NV_INT64_SPECIFIER "%lld"
#define NV_INT16 signed short
#define NV_INT32 signed int
#define NV_INT64 signed long long
#define NV_U_INT16 unsigned short
#define NV_U_INT32 unsigned int
#define NV_U_INT64 unsigned long long
#endif


#define NV_U_INT32_MAX  0xffffffff
#define NV_INT32_MAX    0x7fffffff
#define NV_U_INT16_MAX  0xffff
#define NV_INT16_MAX    0x7fff


#define NV_FLOAT32 float
#define NV_FLOAT64 double

#ifdef WINNT
#define NV_FLOAT80 long double
#endif



    typedef struct
    {
        NV_U_INT16      r;
        NV_U_INT16      g;
        NV_U_INT16      b;
    } NV_C_RGB;



    typedef struct
    {
        NV_U_INT16      h;
        NV_U_INT16      s;
        NV_U_INT16      v;
    } NV_C_HSV;



    typedef struct
    {
        NV_FLOAT32      x;
        NV_FLOAT32      y;
    } NV_F32_COORD2;



    typedef struct
    {
        NV_FLOAT32      x;
        NV_FLOAT32      y;
        NV_FLOAT32      z;
    } NV_F32_COORD3;



    typedef struct
    {
        NV_FLOAT64      x;
        NV_FLOAT64      y;
    } NV_F64_COORD2;



    typedef struct
    {
        NV_FLOAT64      x;
        NV_FLOAT64      y;
        NV_FLOAT64      z;
    } NV_F64_COORD3;



    typedef struct
    {
        NV_INT32        x;
        NV_INT32        y;
    } NV_I32_COORD2;



    typedef struct
    {
        NV_INT32        x;
        NV_INT32        y;
        NV_INT32        z;
    } NV_I32_COORD3;



    typedef struct
    {
        NV_FLOAT32      lat;
        NV_FLOAT32      lon;
    } NV_F32_POS;



    typedef struct
    {
        NV_FLOAT32      lat;
        NV_FLOAT32      lon;
        NV_FLOAT32      dep;
    } NV_F32_POSDEP;


    typedef struct
    {
        NV_FLOAT64      lat;
        NV_FLOAT64      lon;
    } NV_F64_POS;



    typedef struct
    {
        NV_FLOAT64      lat;
        NV_FLOAT64      lon;
        NV_FLOAT64      dep;
    } NV_F64_POSDEP;



    typedef struct
    {
        NV_FLOAT64      slat;
        NV_FLOAT64      wlon;
        NV_FLOAT64      nlat;
        NV_FLOAT64      elon;
    } NV_F64_MBR;


    typedef struct
    {
        NV_FLOAT64      min_y;
        NV_FLOAT64      min_x;
        NV_FLOAT64      max_y;
        NV_FLOAT64      max_x;
    } NV_F64_XYMBR;


#ifdef  __cplusplus
}
#endif


#endif
