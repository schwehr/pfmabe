
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



#include "get_egm08.h"

#define     NINT(a)     ((a) < 0.0 ? (int) ((a) - 0.5) : (int) ((a) + 0.5))

#define     IWINDO      6  /*  Don't change! If you do, check in "interp" for caveats.  */
#define     IPA1        20


static NV_INT32 grid_type = 1;   /* 0 - 1.0 minute grid,  1 - 2.5 minute grid  */
static NV_FLOAT64 latgrid[2], longrid[2], slice_size[2];
static NV_FLOAT64 half_slice, dlat, dlon;
static NV_INT32 nlat, width, row_size;

static NV_BOOL first = NVTrue;
static NV_FLOAT32 **h;


/***************************************************************************\
*                                                                           *
*   Module Name:        swap_float                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       July 1992                                           *
*                                                                           *
*   Purpose:            This function swaps bytes in a four byte float.     *
*                                                                           *
*   Arguments:          word                -   pointer to the float        *
*                                                                           *
\***************************************************************************/

static void swap_float (NV_FLOAT32 *word)
{
  NV_U_INT32        temp[4];

  union
  {
    NV_U_INT32      iword;
    NV_FLOAT32      fword;
  } eq;

  eq.fword = *word;

  temp[0] = eq.iword & 0x000000ff;
  temp[1] = (eq.iword & 0x0000ff00) >> 8;
  temp[2] = (eq.iword & 0x00ff0000) >> 16;
  temp[3] = (eq.iword & 0xff000000) >> 24;

  eq.iword = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];

  *word = eq.fword;

  return;
}




/***************************************************************************\
*                                                                           *
*   Module Name:        big_endian                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       July 1992                                           *
*                                                                           *
*   Purpose:            This function checks to see if the system is        *
*                       big-endian or little-endian.                        *
*                                                                           *
*   Arguments:          NONE                                                *
*                                                                           *
*   Returns:            3 if big-endian, 0 if little-endian                 *
*                                                                           *
\***************************************************************************/

static NV_INT32 big_endian ()
{
  union
  {
    NV_INT32        word;
    NV_U_BYTE       byte[4];
  } a;

  a.word = 0x00010203;
  return ((NV_INT32) a.byte[3]);
}




/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
  C                                                                      C
  C                      I N I T S P                                     C
  C                                                                      C
  C  INITIALIZATION PROCEDURE FOR FAST 1-DIMENSIONAL EQUIDISTANT         C
  C  SPLINE INTERPOLATION, WITH FREE BOUNDARY END CONDITIONS             C
  C  REFERENCE: JOSEF STOER: EINFUHRUNG IN DIE NUMERISCHE MATHEMATIK     C
  C  I, SPRINGER 1972, PAGE 82 AND 86.                                   C
  C                                                                      C
  C----------------------------------------------------------------------C
  C                                                                      C
  C  PARAMETERS (REAL):                                                  C
  C                                                                      C
  C  Y...   GIVEN VALUES, Y(1), ..., Y(N)                                C
  C                                                                      C
  C  R...   SPLINE MOMENTS (1 ... N), TO BE USED BY FUNCTION 'SPLINE'    C
  C                                                                      C
  C  Q...   WORK-ARRAY, DECLARED AT LEAST 1:N                            C
  C                                                                      C
  C----------------------------------------------------------------------C
  C                                                                      C
  C  RENE FORSBERG, JULY 1983                                            C
  C                                                                      C
  CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/

static void initsp (NV_FLOAT64 *y, NV_INT32 n, NV_FLOAT64 *r, NV_FLOAT64 *q)
{
  NV_INT32 k;
  NV_FLOAT64 p;

  q[0] = 0.0;
  r[0] = 0.0;
  for (k = 1 ; k < n - 1 ; k++)
    {
      p = q[k - 1] / 2.0 + 2.0;
      q[k] = -0.5 / p;
      r[k] = (3.0 * (y[k + 1] - 2.0 * y[k] + y[k - 1]) - r[k - 1] / 2.0) / p;
    }

  r[n - 1] = 0.0;
  for (k = n - 2 ; k >= 1 ; k--)
    {
      r[k] = q[k] * r[k + 1] + r[k];
    }

  return;
}




/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
  C                                                                      C
  C                          S P L I N E                                 C
  C                                                                      C
  C  FAST ONE-DIMENSIONAL EQUIDISTANT SPLINE INTERPOLATION FUNCTION.     C
  C  REFERENCE: JOSEF STOER: EINFUHRUNG IN DIE NUMERISCHE MATHEMATIK     C
  C  I, SPRINGER 1972, PAGE 81.                                          C
  C                                                                      C
  C----------------------------------------------------------------------C
  C                                                                      C
  C  PARAMETERS:                                                         C
  C                                                                      C
  C  X...  INTERPOLATION ARGUMENT (REAL), X = 1 FIRST DATA-POINT,        C
  C        X = N LAST DATA-POINT. OUTSIDE THE RANGE LINEAR EXTRA-        C
  C        POLATION IS USED.                                             C
  C                                                                      C
  C  Y...  REAL*8 ARRAY, 1 .. N : DATA VALUES                            C
  C                                                                      C
  C  R...  DO: SPLINE MOMENTS CALCULATED BY SUBROUTINE 'INITSP'          C
  C                                                                      C
  C----------------------------------------------------------------------C
  C                                                                      C
  C  PROGRAMMER:                                                         C
  C  RENE FORSBERG, JUNE 1983                                            C
  C                                                                      C
  CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/

static NV_FLOAT64 spline (NV_FLOAT64 x, NV_FLOAT64 *y, NV_INT32 n, NV_FLOAT64 *r)
{
  NV_INT32 j;
  NV_FLOAT64 xx, result = 0.0;

  if (x < 0.0)
    {
      result = y[0] + x * (y[1] - y[0] - r[1] / 6.0);
    }
  else if (x >= n)
    {
      result = y[n - 1] + (x - (n - 1)) * (y[n - 1] - y[n - 2] + r[n - 2] / 6.0);
    }
  else
    {
      j = x;
      if (x < 0.0 && x != (NV_FLOAT64) j) j -= 1;


      xx = x - j;
      result = y[j] + xx * ((y[j + 1] - y[j] - r[j] / 3.0 - r[j + 1] / 6.0) + xx * (r[j] / 2.0 + xx * (r[j + 1] - r[j]) / 6.0));
    }

  return (result);
}




/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
  C                                                                      C
  C     SUBROUTINE FOR INTERPOLATION OF VALUES FROM A STANDARD DTM-GRID  C
  C     TO INDIVIDUAL STATION LOCATIONS.                                 C
  C                                                                      C
  C                                                                      C
  C     INPUT PARAMETERS...                                              C
  C     ===================                                              C
  C     IWINDO...    A SPLINE WINDOW OF SIZE 'IWINDO' X 'IWINDO' WILL BE C
  C                  USED AROUND EACH STATION.                           C
  C     H...         2D DATA ARRAY (ELEMENT (1,1) IN SW CORNER).         C
  C     PHIS,DLAW... LATITUDE AND LONGITUDE OF SW GRID POINT.            C
  C     DDFI,DDLA... GRID SPACING IN LATITUDE AND LONGITUDE DIRECTION.   C
  C     NPHI,NDLA... NUMBER OF GRID POINTS IN LATITUDE AND LONGITUDE     C
  C                  DIRECTION.                                          C
  C     PHI,DLA...   LATITUDE AND LONGITUDE OF INTERPOLATION POINT.      C
  C                                                                      C
  C                                                                      C
  C     OUTPUT PARAMETERS...                                             C
  C     ====================                                             C
  C     VALINT...    INTERPOLATED VALUE.                                 C
  C                                                                      C
  C                                                                      C
  C     EXECUTION TIME ON CDC 990 IS...                                  C
  C     ===============================                                  C
  C     +------------------+-------------------+-------------------+     C
  C     I  INTERPOLATION   I  OPT=LOW          I  OPT=HIGH         I     C
  C     I------------------I-------------------I-------------------I     C
  C     I  BILINEAR        I  1.44 MSEC/STAT.  I  1.44 MSEC/STAT.  I     C
  C     I  3 X 3 SPLINE    I  1.53 MSEC/STAT.  I  1.51 MSEC/STAT.  I     C
  C     I  5 X 5 SPLINE    I  1.70 MSEC/STAT.  I  1.67 MSEC/STAT.  I     C
  C     I  7 X 7 SPLINE    I  2.02 MSEC/STAT.  I  1.74 MSEC/STAT.  I     C
  C     I  9 X 9 SPLINE    I  2.31 MSEC/STAT.  I  2.00 MSEC/STAT.  I     C
  C     +------------------+-------------------+-------------------+     C
  C                                                                      C
  C                                                                      C
  C     PROGRAM CREATION BY...   H. DENKER          MAY 30, 1987         C
  C                              H. DENKER          MARCH 13, 1989       C
  C                                                                      C
  CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/

static void interp (NV_INT32 *iwindo, NV_FLOAT64 phis, NV_FLOAT64 dlaw, NV_FLOAT64 ddfi, NV_FLOAT64 ddla,
                    NV_INT32 nphi, NV_INT32 ndla __attribute__ ((unused)), NV_FLOAT64 phi, NV_FLOAT64 dla, NV_FLOAT64 *valint)
{
  NV_FLOAT64 a[IPA1], r[IPA1], q[IPA1], hc[IPA1], ri, rj;
  NV_INT32 i0, j0, ii, jj, i, j;


  /*  If you EVER change IWINDO to be less than 2 or greater than IPA1 uncomment this section.  "If" statements
      are really slow.  */
  /*
  if (*iwindo < 2) *iwindo = 2;
  if (*iwindo > IPA1) *iwindo = IPA1;
  */


  ri = (phi - phis) / ddfi;
  rj = (dla - dlaw) / ddla;


  /*  If you EVER change IWINDO to an odd number, uncomment this section.  */

  /*
  if (*iwindo % 2)
    {
      i0 = ri - 0.5;
      j0 = rj - 0.5;
    }
  else
  */
    {
      i0 = ri;
      j0 = rj;
    }

  i0 = i0 - *iwindo / 2 + 1;
  j0 = j0 - *iwindo / 2 + 1;
  ii = i0 + *iwindo - 1;
  jj = j0 + *iwindo - 1;


  /*  Since we're centering the slice on the longitude (with wrap at 0/360) we don't need to check for longitude edge effects.
      We do, however, need to check for being too near the poles, in which case we'll just grab the nearest point instead
      of interpolating.  */

  if (i0 < 0 || ii >= nphi)
    {
      i0 = ri;
      j0 = rj;

      fprintf (stderr, "%f %f station too near grid boundary  - no int. possible\n", phi, dla);
      fprintf (stderr, "Returning nearest grid point.\n");
      fflush (stderr);
      *valint = h[i0][j0];
      return;
    }


  for (i = 0 ; i < *iwindo ; i++)
    {
      for (j = 0 ; j < *iwindo ; j++)
        {
          a[j] = h[i0 + i][j0 + j];
        }

      initsp (a, *iwindo, r, q);
      hc[i] = spline (rj - j0, a, *iwindo, r);
    }

  initsp (hc, *iwindo, r, q);
  *valint = spline (ri - i0, hc, *iwindo, r);

  return;
}




/*  Allows you to set the grid type to be retrieved.  0 is for the 1 minute grid and 1 is for the 2.5 minute grid.
    This returns the value that grid_type was set to.  It returns -1 if the requested grid type is not available.
    IMPORTANT NOTE:  This must be called prior to the first call to get_egm08 if you want to use the 1.0 minute grid.  */

NV_INT32 set_egm08_grid_type (NV_INT32 gt)
{
  NV_CHAR dirfil[512], big_file[2][512], little_file[2][512];
  FILE *dfp;


  if (gt < 0 || gt > 1) return (-1);


  /*  Use the environment variable WVS_DIR to get the directory name.  */

  if (getenv ("WVS_DIR") == NULL)
    {
      fprintf (stderr, ("\n\nEnvironment variable WVS_DIR is not set.\n\n"));
      fflush (stderr);
      return (-1);
    }

  strcpy (dirfil, getenv ("WVS_DIR"));
  if (dirfil[0] == 0)
    {
      fprintf (stderr, ("\n\nWVS_DIR directory is not available.\n\n"));
      fflush (stderr);
      return (-1);
    }


  /*  Define the big-endian and little-endian file names.  */

  /*  1.0 minute grid files.  */

  sprintf (big_file[0], "%s%1cUnd_min1x1_egm2008_isw=82_WGS84_TideFree", dirfil, (NV_CHAR) SEPARATOR);
  sprintf (little_file[0], "%s%1cUnd_min1x1_egm2008_isw=82_WGS84_TideFree_SE", dirfil, (NV_CHAR) SEPARATOR);


  /*  2.5 minute grid files.  */

  sprintf (big_file[1], "%s%1cUnd_min2.5x2.5_egm2008_isw=82_WGS84_TideFree", dirfil, (NV_CHAR) SEPARATOR);
  sprintf (little_file[1], "%s%1cUnd_min2.5x2.5_egm2008_isw=82_WGS84_TideFree_SE", dirfil, (NV_CHAR) SEPARATOR);


  if ((dfp = fopen (big_file[gt], "rb")) == NULL)
    {
      if ((dfp = fopen (little_file[gt], "rb")) == NULL)
        {
          return (-1);
        }
    }

  fclose (dfp);

  grid_type = gt;

  return (grid_type);
}




/*! <pre>

  DESCRIPTION:

    The get_egm08 function was loosely based on a FORTRAN program that was available at NGA
    (formerly NIMA, formerly DMA).  The initsp, spline, and interp functions were taken from
    NGA and converted from FORTRAN to C.  A few changes have been made to simplify things:

        I removed the bilinear interpolation method of the original routine and went with a
        hardwired 6 point spline since we aren't going to set IWINDO to anything else.

        I removed the distance check from interp and removed the unused variables as well.

        Values within 3 points (IWINDOW / 2 or either 7.5 or 3.0 minutes depending on the
        grid file used) of the north and south poles are not interpolated - I just hand you
        the nearest value (it's pretty darn close).

        I only load a longitudinal slice of the data, not the entire file.  This is done to
        conserve memory.  The advantage is that you will never hit the longitudinal edges of
        the data since I build the slice as a continuous piece with the first point that is 
        requested in the center of the slice (this is even done if you cross the 0/360
        boundary).  Whenever a requested input point gets within 1 degree of the
        longitudinal edges of the slice I load a new slice centered on that point.

        I allocate the memory for the slice of data instead of declaring it static so you can
        free it if you want to conserve the memory (cleanup_egm08).

        I store the array in 32 bit floating point format instead of 64 bit floating point
        format.  This cuts our memory usage in half.  Since the data in the file is stored
        as 32 bit floating point values it made no sense to use 64 bit values anyway.

        I handle either the big-endian or the little-endian file and will swap if needed
        based on endian-ness of the system that it is being run on.  If you're going to
        usually run on one type of system it makes sense to use the same endian-ness file
        (although the function will check for both).

        I am using types defined in the file nvtypes.h.  These have been in use at the Naval
        Oceanographic Office for about 15 years.  We do this to make sure we get the correct
        variable sizes on different architectures.  The types are easy to understand, for
        example, NV_U_INT32 is an unsigned 32 bit integer.


    This function can be very easily changed to use the 1 minute grid file(s).  Just look for
    the two comments that begin with "1.0 minute grid".  Comment out the "2.5 minute grid"
    sections and uncomment the "1.0 minute grid sections.  The answers are going to be almost
    identical but, if you want to slog through the MUCH larger file, be my guest.  Note that
    the slice size for the 1.0 minute grid implementation is set at 10 degrees.  That can be
    changed to something larger but it uses about 26MB of memory as it is.  The 45 degree
    slice for the 2.5 minute grid uses about 19MB of memory.  If you used 45 degrees for the
    1.0 minute slice it would need about 117MB of memory.

    The synopsis for the get_egm08 function is directly above the function itself.  I have
    left the original FORTRAN comments in place where they made sense.



  CAVEATS:

    You have to set an environment variable (WVS_DIR) to point to the directory (that's 
    a 'folder' for the Windoze crowd) that contains the Und... file(s).

    These functions have been tested on Linux using gcc and on 32 bit Windows XP using MinGW
    and gcc.

    There are FORTRAN callable versions of the get_egm08 and cleanup_egm08 functions but they
    haven't been tested (I don't do FORTRAN anymore).

    These functions are callable from C++ (and have been tested).  If you want to call them
    from Pascal, ADA, Java, or anything else, you're on your own ;-)



  DATA:

    The EGM files were available at NGA:

    http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/egm08_wgs84.html

    Due to the excessive name changing that happens at NGA this page may disappear
    in short order, however, a Google (TM) search for "EGM08 WGS84" might find it.


    The EGM96 model was approved for official use, as it says on their web site:


    "The NGA/NASA 15-minute worldwide geoid height file and the WGS 84, n=m=360, EGM96 
    coefficient file are approved for official DoD use as documented in NGA TR8350.2, 
    Third Edition, 4 July 1997."


    I'm not sure about the EGM08 model but we're not using it for anything earth shaking ;-)



  AUTHOR:

    Jan C. Depner
    Naval Oceanographic Office
    Code NP4T
    jan.depner@navy.mil
    12/02/2009




    Comments from NGA's INTERP program (on which get_egm08 was loosely based).
    Note that the file is not exactly as described here.  It was written with
    FORTRAN on some strange type of system (I know not which) and has a single
    control word preceeding and following each record.  The control words consist
    of the four hex bytes {0x00 0x87 0x00 0x00} (or {0x00 0x00 0x87 0x00}
    depending on the endian-ness of the file).

c      
c-----------------------------------------------------------------------
c
c     INTERP IS A F77 PROGRAM FOR INTERPOLATING FROM A GLOBAL GRID OF
c     REGULARLY SPACED POINT VALUES TO SELECTED SCATTERED COORDINATES.
c     INTERP REQUIRES AN INPUT FILE CONTAINING THE GLOBALLY GRIDDED
c     VALUES, AND AN INPUT FILE CONTAINING THE GEOGRAPHICAL COORDINATES 
c     OF THE SCATTERED POINTS FOR WHICH INTERPLATED VALUES ARE REQUIRED.
c
c     THE INPUT FILE OF GLOBALLY GRIDDED POINT VALUES WILL STORE THESE  
c     VALUES IN REAL*4 SEQUENTIAL BINARY FORMAT, WITH ONE RECORD FOR 
c     EACH PARALLEL BAND. THESE INPUT VALUES ARE EQUALLY SPACED IN 
c     LATITUDE AND LONGITUDE, AND ARE SITUATED AT THE CORNERS OF THEIR 
c     RESPECTIVE CELLS, SUCH THAT THE TOP-LEFT POINT IN THE GRID HAS A 
c     LONGITUDE OF ZERO DEGREES AND A LATITUDE OF NINETY DEGREES. THE 
c     FIRST RECORD CONTAINS THE NORTHERN-MOST PARALLEL, AND THE FIRST 
c     VALUE IN EACH RECORD IS THE WESTERNMOST VALUE FOR THAT PARALLEL. 
c     NOTE THAT GRID VALUES SITUATED ON THE ZERO MERIDIAN APPEAR ONLY
c     ONCE, AS THE FIRST VALUE IN THEIR RESPECTIVE RECORD AT ZERO 
c     LONGITUDE. THESE VALUES ARE NOT REPEATED AT THE END OF THEIR 
c     RESPECTIVE RECORDS AT LONGITUDE = 360 DEGREEES. 
c
c     THE ASCII INPUT FILE CONTAINING THE GEOGRAPHICAL COORDINATES OF
c     THE SCATTERED POINTS CONTAINS ONE RECORD FOR EACH POINT. EACH 
c     RECORD CONTAINS THE GEODETIC LATITUDE AND THEN THE LONGITUDE 
c     IN DECIMAL DEGREES FOR ITS RESPECTIVE POINT. 
c
c     THE ASCII OUTPUT FILE CONTAINING THE INTERPOLATED VALUES FOR THE
c     SCATTERED POINTS ALSO CONTAINS ONE RECORD FOR EACH POINT. EACH 
c     RECORD CONTAINS THE GEODETIC LATITUDE AND LONGITUDE IN DECIMAL 
c     DEGREES, FOLLOWED BY THE INTERPOLATED VALUE FOR THAT POINT.
c
c     THE INPUT PARAMETERS FOR INTERP ARE:
c
c     path_gd  :  PATH TO THE INPUT FILE OF GLOBALLY GRIDDED VALUES, 
c     name_gd  :  NAME OF THE INPUT FILE OF GLOBALLY GRIDDED VALUES,
c     path_pt  :  PATH TO THE INPUT FILE OF INTERPOLATION LATS AND LONS,  
c     name_pt  :  NAME OF THE INPUT FILE OF INTERPOLATION LATS AND LONS,
c     path_out :  PATH TO THE OUTPUT FILE OF INTERPOLATED VALUES,  
c     name_out :  NAME OF THE OUTPUT FILE OF INTERPOLATED VALUES,
c
c     dlat     :  LAT SPACING (DEGREES) OF INPUT GRID,
c     dlon     :  LON SPACING (DEGREES) OF INPUT GRID.
c      
c-----------------------------------------------------------------------
c     ORIGINAL PROGRAM:                           NIKOS PAVLIS
c     MODIFIED FOR USE BY NGA WITH 30 SEC GRIDS   SIMON HOLMES, AUG 2007
c     CORNER-CELL REGISTRATION                    SIMON HOLMES, MAY 2008 
c-----------------------------------------------------------------------



    This function will return an interpolated difference between the ellipsoid and the geoid 
    for the latitude and longitude passed to it (EGM08 model).  The longitude may be in -180 to 180
    or 0 to 360 coordinate systems.  An environment variable pointing to the directory that
    contains the EGM08 2.5 minute file(s) must be defined.  The environment variable is WVS_DIR
    (I'm lazy, this is a holdover from where we store the WVS coastlines ;-)  This function
    returns 999999.0 for bad input or no model file available.

</pre>*/


NV_FLOAT32 get_egm08 (NV_FLOAT64 lat, NV_FLOAT64 lon)
{
  static NV_F64_XYMBR mbr = {-999.0, -999.0, -999.0, -999.0};
  NV_BOOL swap = NVFalse;
  FILE *dfp;
  NV_CHAR dirfil[512], big_file[2][512], little_file[2][512];
  NV_FLOAT64 flat, flon, un;
  NV_INT32 i, j, k, lon_offset, cross_offset, strip_size[2], iwindo, nlon;



  /*  The first time through we want to allocate the memory for the H array.  This can be freed later (cleanup_egm08) 
      if we don't want to hang on to the memory.  */

  if (first)
    {
      /*  1.0 minute grid setup.  */

      latgrid[0] = 1.0L;
      longrid[0] = 1.0L;
      slice_size[0] = 10.0L;


      /*  2.5 minute grid setup.  */

      latgrid[1] = 2.5L;
      longrid[1] = 2.5L;
      slice_size[1] = 45.0L;


      half_slice = slice_size[grid_type] / 2.0L;
      nlat = (NV_INT32) ((60.0L / latgrid[grid_type])) * 180 + 1;
      nlon = (NV_INT32) (60.0L / longrid[grid_type]) * 360;
      dlat = latgrid[grid_type] / 60.0L;
      dlon = longrid[grid_type] / 60.0L;
      width = (NV_INT32) (nlon / (360 / (NV_INT32) slice_size[grid_type]));


      /*  We add 2 to nlon because there is a Fortran control word preceeding and following each record in the input file.  */

      row_size = (NV_INT32) ((nlon + 2) * sizeof (NV_FLOAT32));


      h = (NV_FLOAT32 **) malloc (nlat * sizeof (NV_FLOAT32 *));

      if (h == NULL)
        {
          perror ("Allocating first dimension of H array in get_egm08.c");
          exit (-1);
        }

      for (i = 0 ; i < nlat ; i++)
        {
          h[i] = (NV_FLOAT32 *) malloc (width * sizeof (NV_FLOAT32));
          if (h[i] == NULL)
            {
              perror ("Allocating second dimension of H array in get_egm08.c");
              exit (-1);
            }
        }

      first = NVFalse;
    }


  /*  Move to 0/360 world.  */

  flat = lat;
  flon = lon;
  if (flon < 0.0) flon += 360;


  /*  Check for bad input.  */

  if (flat < -90.0 || flat > 90.0 || flon < 0.0 || flon > 360.0) return (999999.0);


  /*  If we get within 1 degree of the edge (longitudinal) of our slice we need to read a new slice centered on the current point.  */

  if (flon - mbr.min_x < 1.0 || mbr.max_x - flon < 1.0)
    {
      /*  Compute the slice MBR (minimum bounding rectangle).  We always go from -90.0 to +90.0 latitude
          in order to keep things simple.  */

      mbr.min_x = (NV_FLOAT64) (NINT ((flon - half_slice) / dlon)) * dlon;
      mbr.max_x = mbr.min_x + slice_size[grid_type];
      mbr.min_y = -90.0;
      mbr.max_y = 90.0;

      lon_offset = NINT (mbr.min_x / dlon) * sizeof (NV_INT32);

      strip_size[0] = width * sizeof (NV_FLOAT32);
      strip_size[1] = 0;

      cross_offset = 0;


      /*  If our slice crosses the 0/360 boundary we're going to have to read it in two sections and then
          put them together.  */

      if (mbr.max_x > 360.0)
        {
          cross_offset = NINT ((360.0 - mbr.min_x) / dlon);
          strip_size[1] = cross_offset * sizeof (NV_FLOAT32);
          strip_size[0] = NINT ((mbr.max_x - 360.0) / dlon) * sizeof (NV_FLOAT32);
        }


      /*  Use the environment variable WVS_DIR to get the directory name.  */

      if (getenv ("WVS_DIR") == NULL)
        {
          fprintf (stderr, ("\n\nEnvironment variable WVS_DIR is not set\n\n"));
          fflush (stderr);
          return (999999.0);
        }

      strcpy (dirfil, getenv ("WVS_DIR"));
      if (dirfil[0] == 0)
        {
          fprintf (stderr, ("\n\nWVS_DIR directory is not available.\n\n"));
          fflush (stderr);
          return (999999.0);
        }


      /*  Define the big-endian and little-endian file names.  */

      /*  1.0 minute grid files.  */

      sprintf (big_file[0], "%s%1cUnd_min1x1_egm2008_isw=82_WGS84_TideFree", dirfil, (NV_CHAR) SEPARATOR);
      sprintf (little_file[0], "%s%1cUnd_min1x1_egm2008_isw=82_WGS84_TideFree_SE", dirfil, (NV_CHAR) SEPARATOR);


      /*  2.5 minute grid files.  */

      sprintf (big_file[1], "%s%1cUnd_min2.5x2.5_egm2008_isw=82_WGS84_TideFree", dirfil, (NV_CHAR) SEPARATOR);
      sprintf (little_file[1], "%s%1cUnd_min2.5x2.5_egm2008_isw=82_WGS84_TideFree_SE", dirfil, (NV_CHAR) SEPARATOR);


      /*  Check to see if this is a big-endian system.  If so, we want to try to open the big-endian version.  If that's
          not available we'll try to open the little-endian version and swap the data after we read it.  Vice-versa for 
          little-endian systems.  */

      swap = NVFalse;
      if (big_endian ())
        {
          if ((dfp = fopen (big_file[grid_type], "rb")) == NULL)
            {
              if ((dfp = fopen (little_file[grid_type], "rb")) == NULL)
                {
                  perror (little_file[grid_type]);
                  return (999999.0);
                }
              swap = NVTrue;
            }
        }
      else
        {
          if ((dfp = fopen (little_file[grid_type], "rb")) == NULL)
            {
              if ((dfp = fopen (big_file[grid_type], "rb")) == NULL)
                {
                  perror (big_file[grid_type]);
                  return (999999.0);
                }
              swap = NVTrue;
            }
        }


      /*  Read input grid file and store in array h.  */

      for (i = 0 ; i < nlat ; i++)
        {
          /*  Compute the index into the H array since we want to flip north and south.  */

          k = nlat - i - 1;


          /*  If we cross the 0/360 boundary we need to read the data in two sections.  */

          if (cross_offset)
            {
              /*  Read the stuff from 0 to the end of our slice  (the + 4 skips the FORTRAN control word preceeding the record).  */

              fseek (dfp, i * row_size + 4, SEEK_SET);
              fread (&h[k][cross_offset], strip_size[0], 1, dfp);


              /*  Read the stuff from the beginning of the slice to 0 (the + 4 skips the FORTRAN control word preceeding the record).  */

              fseek (dfp, i * row_size + 4 + lon_offset, SEEK_SET);
              fread (h[k], strip_size[1], 1, dfp);
            }


          /*  We didn't cross the 0/360 boundary so we just read the slice.  */

          else
            {
              /*  The + 4 skips the FORTRAN control word preceeding the record.  */

              fseek (dfp, i * row_size + 4 + lon_offset, SEEK_SET);
              fread (h[k], strip_size[0], 1, dfp);
            }


          /*  If this system is not the same endian-ness as the data file we have to swap the data.  */

          if (swap) for (j = 0 ; j < width ; j++) swap_float (&h[k][j]);
        }

      fclose (dfp);
    }


  iwindo = IWINDO;

  interp (&iwindo, mbr.min_y, mbr.min_x, dlat, dlon, nlat, width, flat, flon, &un);

  return ((NV_FLOAT32) un);
}




/*!  Frees the slice memory.  */

void cleanup_egm08 ()
{
  NV_INT32 i;

  if (!first)
    {
      for (i = 0 ; i < nlat ; i++) free (h[i]);

      free (h);

      first = NVTrue;
    }
}



/*  A little test program.  Just change #undef to #define and then follow the directions below.  */


#undef TEST_MAIN


/*  Test main


    Compile the program:

    gcc -O -ansi -Wall get_egm08.c -lm -o egm08


    Then run the program using:

    ./egm08


    The following test values were generated using the hsynth_WGS84.exe program:

        Lat         Lon          Z

     37.000000  241.000000    -26.151
     37.000000 -119.000000    -26.151
     36.000000  242.983333    -29.170
    -36.000000 -119.000000    -11.877
    -36.000000  119.000000    -38.295
     36.000000  119.000000      1.457
    -35.000000   -5.000000     17.893
     35.000000   -5.000000     44.366
     90.000000    0.000000     14.899
    -90.000000  359.983333    -30.150

*/


#ifdef TEST_MAIN

NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_F64_COORD3 xyz[10] = {{ 241.000000,  37.000000, -26.151},
                           {-119.000000,  37.000000, -26.151},
                           { 242.983333,  36.000000, -29.170},
                           {-119.000000, -36.000000, -11.877},
                           { 119.000000, -36.000000, -38.295},
                           { 119.000000,  36.000000,   1.457},
                           {  -5.000000, -35.000000,  17.893},
                           {  -5.000000,  35.000000,  44.366},
                           {   0.000000,  90.000000,  14.899},
                           { 359.983333, -90.000000, -30.150}};
  NV_FLOAT32 result;
  NV_INT32 i;

  for (i = 0 ; i < 10 ; i++)
    {
      result = get_egm08 (xyz[i].y, xyz[i].x);

      fprintf (stderr,"%f %f %.3f should equal %.3f\n", xyz[i].y, xyz[i].x, xyz[i].z, result);
      fflush (stderr);
    }


  cleanup_egm08 ();


  return (0);
}

#endif

