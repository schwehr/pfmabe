/*

    libmisp.a - MISP gridding library

    Author : Jan C. Depner and a cast of tens of original authors

    Date : 09/18/02


    The following library is a single pass implementation of the CHRTR/MISP
    gridding package.  It does not do subgridding or edge matching.  It 
    consists of four functions:

        misp_init - initializes all variables and structures need for the
                    gridding operations
        misp_load - loads all of the user's input data into the sparse grid
        misp_proc - does the actual gridding
        misp_rtrv - retrieves the gridded data from the memory structures

    If you do not have enough memory to grid the area you have specified you
    have a few options.  Move to another system with more memory, buy more 
    memory, use the CHRTR program, re-write this to do subgridding and
    edge matching, or go find something else to do.

    To give a better description of the algorithms I am going to shamelessly
    steal the documentation from the C port of the FORTRAN CHRTR program done 
    by Dominic Avery and myself in 1992 (since I stole most of the code from 
    that anyway).


*   Purpose:            This program produces a uniform grid at a specified *
*                       grid spacing from irregularly spaced input data.    *
*                                                                           *
*                                                                           *
*   Glossary:           error_control       - Number of times to retry      *
*                                             min_curve after increasing    *
*                                             search_radius.                *
*                       filter_points       - Filter size in grid points.   *
*                       gridcols            - Number of grid columns in     *
*                                             chart.                        *
*                       gridrows            - Number of grid rows in chart. *
*                       memcols             - Number of grid columns in     *
*                                             memory                        *
*                       memrows             - Number of grid rows in memory.*
*                       memblock_height     - Number of grid rows in memory *
*                                             to be used in loadsparse and  *
*                                             readsparse.                   *
*                       memblock_width      - Number of grid columns in     *
*                                             memory for loadsparse and     *
*                                             readsparse.                   *
*                       memblock_total      - Total number of grid cells    *
*                                             in memory for loadsparse and  *
*                                             readsparse.                   *
*                       reg_multfact        - Multiplicative factor for     *
*                                             initial coarse grid in        *
*                                             regional computation          *
*                                             (default = 4).                *
*                       weight_factor       - Weight factor for merging     *
*                                             the regional field with the   *
*                                             original sparse input data.   *
*                                             The weight factor may be set  *
*                                             to 1,2, or 3.  The larger the *
*                                             value, the more weight that   *
*                                             is put on the input data.     *
*                                             (default = 2).                *
*                       delta               - The value of the maximum      *
*                                             change in the surface between *
*                                             iterations at which           *
*                                             convergence is accepted in    *
*                                             regional computation (see     *
*                                             misp_iterate function).       *
*                                             (default = 0.05).             *
*                       gridint             - Grid interval.                *
*                       memblock1           - Memory block 1.               *
*                       memblock2           - Memory block 2.               *
*                       memblock3           - Memory block 3.               *
*                       memblock4           - Memory block 4.               *
*                       memblock5           - Memory block 5.               *
*                       search_radius       - Maximum distance from a grid  *
*                                             point that data points are    *
*                                             to be used for regional       *
*                                             computation.  This is used in *
*                                             the weight_mean function.     *
*                                             (default = 20.0)              *
*                                                                           *
*   Method:             This program takes randomly spaced data points and  *
*                       bins them in a sparsely populated grid.  These data *
*                       are averaged for both position and value.  Next a   *
*                       regional grid is created from this data using the   *
*                       methods described in  the article "A FORTRAN IV     *
*                       PROGRAM FOR INTERPOLATING IRREGULARLY SPACED DATA   *
*                       USING THE DIFFERENCE EQUATIONS FOR MINIMUM          *
*                       CURVATURE" by C. J. Swain in Computers and          *
*                       Geosciences, Vol. 1 pp. 231-240 and the article     *
*                       "MACHINE CONTOURING USING MINIMUM CURVATURE" by     *
*                       Ian C. Briggs in Geophysics, Vol 39, No. 1          *
*                       (February 1974), pp. 39-48.  The regional grid is   *
*                       interpolated to the final grid spacing using a      *
*                       cubic spline interpolation function.  The next step *
*                       is to shift the original data to the grid point     *
*                       based on the slope of the regional field.  Then the *
*                       regional field and the sparse input data are        *
*                       merged together.                                    *
*                                                                           *
*   History:            The CHRTR interpolating package is an extension of  *
*                       the MISP interpolation algorithm developed under    *
*                       the auspices of Dr. Tom Davis and the Advanced      *
*                       Technology Staff (currently the Scientific          *
*                       Technology Staff) of the Naval Oceanographic        *
*                       Office.  The development of the program was         *
*                       motivated by a need for a general purpose           *
*                       interpolation routine capable of handling a wide    *
*                       variety of data types, from well-behaved potential  *
*                       fields to more erratic fields, such as bathymetry.  *
*                       Additionally, it was required that the interpolator *
*                       be able to transcend as gracefully as possible from *
*                       areas of adequate data coverage to data sparse      *
*                       areas.  Finally, it was desired that the routine be *
*                       able to accept an unlimited amount of input data    *
*                       and be effectively unbounded in the dimensions of   *
*                       the grids which could be produced.  Originally      *
*                       configured in approximately 1975, the routines have *
*                       periodically been modified to improve performance.  *
*                       The following individuals share varying degrees of  *
*                       complicity in creating, modifying, and maintaining  *
*                       the integrity of the philosophical and technical    *
*                       content of the algorithm:  Tom Davis, Rudy Michlik, *
*                       Ang Kontis, Ken Countryman, and Bill Rankin.  This  *
*                       first C version was written by Dominic Avery and    *
*                       Jan Depner of the Geophysics Department of the      *
*                       Naval Oceanographic Office.  This version will be   *
*                       released as version 2.0 (since nobody has kept up   *
*                       with the first 17 years of changes we will call     *
*                       that version 1.X).                                  *
*                                                                           *
*   Restrictions:       This program was converted directly from FORTRAN 77 *
*                       in order to take advantage of the memory allocation *
*                       capabilities inherent in the C language.  Since the *
*                       original program relied heavily on two dimensional  *
*                       arrays and the allocated memory blocks are          *
*                       essentially one degree arrays (pointers), accessing *
*                       the memory blocks can be a bit tricky.  The basic   *
*                       idea is to take the first index times the maximum   *
*                       size of the second index, plus the second index.    *
*                       You must be very careful to make sure that the max  *
*                       size is consistent from place to place.  It should  *
*                       be noted that the term 'array' is used in the       *
*                       comments throughout this program to refer to an     *
*                       area of memory that has been allocated.  Also, the  *
*                       loadsparse and readsparse functions use a different *
*                       memory management scheme from the rest of the       *
*                       functions in that they break the entire area up     *
*                       into east/west bands (if neccessary).  The          *
*                       memblock_width, memblock_height, and memblock_total *
*                       variables refer to loadsparse and readsparse memory *
*                       blocks while memrows and memcols are for the rest   *
*                       of the program.                                     *
*                                                                           *
*                                                                           *

    Note that THIS implementation WON'T be "able to accept an unlimited amount
    of input data and be effectively unbounded in the dimensions of the grids 
    which could be produced".  As stated above, if you ain't got the memory
    you're effectively SOL.

    Now, on to some more interesting points.  I wouldn't mess with the delta
    value or the reg_multfact value unless you REALLY understand this stuff (I
    certainly don't).  The min_x, min_y, max_x, and max_y values are the
    rectangular bounds of your area in whatever units you see fit to use
    (furlongs, chains, rods, leagues, all of the popular units are accepted).
    The gridint value is in these units.  The MISP algorithms expect a uniform
    grid so you can't have different X and Y intervals.  If you need different 
    X and Y intervals you can change the units prior to defining the area and
    loading points (in other words, lie to the damn thing).  The weight_factor
    value is used to weight the actual input values (sparse grid) against the
    regional grid.  A weight of 3 makes the final grid more closely match the
    sparse grid while a weight of 1 makes it match the regional.  The way Tom
    Davis explained it to me once (back in the days before electricity) was
    that if you have randomly spaced data use a weight of 1.  If the data is
    uniform but does not have complete, or nearly complete, coverage, use 2.
    If you have uniform, nearly complete coverage, use 3.  For the anal
    retentive, there is the heretofor undocumented feature - use a negative
    weight to slam the final grid to the sparse grid values (where they exist).


    IMPORTANT NOTE IMPORTANT NOTE IMPORTANT NOTE IMPORTANT NOTE IMPORTANT NOTE 

    This library creates grid points on the corners of the grid.  That is, if 
    you say you want a one-degree square at a tenth-minute grid spacing it will
    return 601 points.  Everything is shifted down and to the left.  If, like a
    normal human being, you want posts in the center of the bins you have to lie
    to the loader.  The way you do this is to add half of your grid spacing to 
    each X and Y position as you load it.  When you retrieve the data points,
    only use N-1 values.  In the case of the above mentioned one-degree square 
    you would only retrieve the first 600 points and ignore the last point.
    You would also ignore the last row.

    IMPORTANT NOTE IMPORTANT NOTE IMPORTANT NOTE IMPORTANT NOTE IMPORTANT NOTE 


    Happy gridding!
    Jan Depner

*/


#include "misp.h"
#include "version.h"


static NV_F64_XYMBR chrt_mbr;
static NV_INT32     reg_multfact, weight_factor, filter_points, gridcols, 
                    gridrows, memblock_height, memblock_width, memblock_total, 
                    chart_width, chart_height, init, error_control, gridcols, 
                    gridrows;
static NV_FLOAT32   delta, maxvalue, minvalue, *memblock1,
                    *memblock2, *memblock3, *memblock4, *memblock5, 
                    search_radius, x_gridint, y_gridint;
static NV_FLOAT64   x_filter_int, y_filter_int;
static MISP_HEADER  final;
static NV_BOOL      prog_reg = NVFalse;


NV_INT32 misp_init (NV_FLOAT64 x_interval, NV_FLOAT64 y_interval, NV_FLOAT32 dlta, NV_INT32 reg_mfact, 
                    NV_FLOAT32 srch_rad, NV_INT32 err_cont, NV_FLOAT32 maxz, NV_FLOAT32 minz, 
                    NV_INT32 weight, NV_F64_XYMBR mbr)
{
  prog_reg = misp_progress_callback_registered ();

  if (prog_reg)
    {
      misp_progress ("\nMISP - Minimum Curvature Spline Interpolation Gridding\n");
      misp_progress (VERSION);
    }
  else
    {
      printf ("\nMISP - Minimum Curvature Spline Interpolation Gridding\n");
      printf ("\n\n %s \n\n", VERSION);
    }


  init = 1;
  x_gridint = x_interval;
  y_gridint = y_interval;
  delta = dlta;
  reg_multfact = reg_mfact;
  search_radius = srch_rad;
  error_control = err_cont;
  minvalue = minz;
  maxvalue = maxz;
  weight_factor = weight;


  filter_points = 9;
  x_filter_int = filter_points * x_interval;
  y_filter_int = filter_points * y_interval;


  /*  Add the filter margin to the boundary limits.        */

  chrt_mbr.min_y = mbr.min_y - y_filter_int;
  chrt_mbr.max_y = mbr.max_y + y_filter_int;
  chrt_mbr.min_x = mbr.min_x - x_filter_int;
  chrt_mbr.max_x = mbr.max_x + x_filter_int;


  /*  Calculate grid rows and columns of final file.                  */

  gridcols = (chrt_mbr.max_x - chrt_mbr.min_x) / x_interval + MULT + 1.01;
  gridrows = (chrt_mbr.max_y - chrt_mbr.min_y) / y_interval + MULT + 1.01;


  /*  Set memory values for use by misps and by loadsparse and        */
  /*  readsparse.                                                     */
    
  memblock_width = gridcols;
  memblock_height = gridrows;
  memblock_total  = gridcols * gridrows;


  /*  Allocate memory for the five major blocks.                      */

  memblock1 = (NV_FLOAT32 *) calloc (memblock_total, sizeof (NV_FLOAT32));
  memblock2 = (NV_FLOAT32 *) calloc (memblock_total, sizeof (NV_FLOAT32));
  memblock3 = (NV_FLOAT32 *) calloc (memblock_total, sizeof (NV_FLOAT32));
  memblock4 = (NV_FLOAT32 *) calloc (memblock_total, sizeof (NV_FLOAT32));
  memblock5 = (NV_FLOAT32 *) calloc (memblock_total, sizeof (NV_FLOAT32));

  if (memblock5 == NULL)
    {
      perror ("Allocating main blocks in MISP");
      exit (-1);
    }

  return (0);
}    



NV_INT32 misp_load (NV_F64_COORD3 xyz)
{
  NV_FLOAT32   yvalue, xvalue, zvalue;

  void loadsparse (NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_INT32, NV_INT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, 
                   NV_INT32);


  /*  We're setting X and Y to be some fraction of the grid interval so that we don't have to worry about precision (like in the case of decimal
      degrees at ~2 meters).  */

  xvalue = (xyz.x - chrt_mbr.min_x) / x_gridint;
  yvalue = (xyz.y - chrt_mbr.min_y) / y_gridint;
  zvalue = (NV_FLOAT32) xyz.z;


  /*  Check the data for out of area conditions.                  */

  if (((xvalue >= -0.5) && (xvalue < ((NV_FLOAT32) gridcols - 0.5))) && ((yvalue >= -0.5) && (yvalue < ((NV_FLOAT32) gridrows - 0.5))) &&
      ((zvalue >= minvalue) && (zvalue <= maxvalue)))
    {
      /*  Load the data into the sparsely populated grid in       */
      /*  memory.                                                 */

      loadsparse (memblock2, memblock3, memblock4, memblock5, memblock_width, memblock_height, memblock_total, xvalue,
                  yvalue, zvalue, 1);

      return (1);
    }
  return (0);
}



/*  Returns NVTrue on error.  */

NV_BOOL misp_proc ()
{
  NV_INT32     numcols, numrows;
  NV_FLOAT32   x_min, y_min, x_max, y_max, x_min_bord, y_min_bord, 
               x_max_bord, y_max_bord;

  void loadsparse (NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_INT32, NV_INT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, 
                   NV_INT32);
  NV_BOOL misps (NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_INT32, 
                 NV_INT32, NV_INT32, NV_INT32, NV_INT32, NV_INT32, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32, 
                 NV_INT32, MISP_HEADER *);
  void merge_grid (NV_INT32, NV_INT32, NV_INT32, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, MISP_HEADER *);



  /*  Call loadsparse to compute the averages after all of the input  */
  /*  data has been loaded.                                           */

  loadsparse (memblock2, memblock3, memblock4, memblock5, memblock_width, memblock_height, memblock_total, 0.0, 0.0, 0.0, 0);


  chart_width = (NV_INT32) ((chrt_mbr.max_x - chrt_mbr.min_x) / x_gridint + 0.5);
  chart_height = (NV_INT32) ((chrt_mbr.max_y - chrt_mbr.min_y ) / y_gridint + 0.5);

  y_max  = chrt_mbr.max_y + y_filter_int * 2.0;
  numrows = chart_height - 1;
  y_min = y_max - y_filter_int * 2.0;
  y_min_bord = y_min;
  y_max = y_min + numrows * y_gridint;
  y_max_bord = y_max;

  x_max = chrt_mbr.max_x + x_filter_int * 2.0;                    
  x_min = x_max - x_filter_int * 2.0;
  x_min_bord = x_min;
  numcols = chart_width - 1;
  x_max = x_min + numcols * x_gridint;
  x_max_bord = x_max;


  if (misps (x_gridint, y_gridint, x_max, x_min, y_max, y_min, x_max_bord, x_min_bord, y_max_bord, y_min_bord, search_radius, reg_multfact, 1, 1, 
             memblock_height, memblock_width, memblock_total, memblock1, memblock2, memblock3, memblock4, memblock5, delta, error_control, 
             &final)) return (NVTrue);


  /*  Merge the regional grid and the original input data from the    */
  /*  sparse grid.                                                    */

  merge_grid (memblock_width, memblock_height, weight_factor, memblock1, memblock2, memblock3, memblock4, memblock5, &final);

  return (NVFalse);
}



/*  IMPORTANT NOTE: Due to the way the old chrtr program handled posts this function will return one more point than grid_cols.
    Make sure that you have allocated enough memory to handle all of the points.  In addition, misp_rtrv will return one more
    row than you expect.  */

NV_INT32 misp_rtrv (NV_FLOAT32 *array)
{
  NV_INT32           j;
  static NV_INT32    row, column3, row2;


  if (init)
    {
      /*  Free the unused memory.  */

      free (memblock2);
      free (memblock3);
      free (memblock4);
      free (memblock5);


      column3 = final.grid_cols - filter_points;
      row2 = final.grid_rows - filter_points;

      row = filter_points;


      init = 0;
    }


  for (j = filter_points ; j <= column3 ; j++) array[j - filter_points] = *(memblock1 + (j * final.height) + row);


  row++;


  if (row == (row2 + 2)) 
    {
      /*  Free memory.  */

      free (memblock1);

      return (0);
    }


  return ((column3 - filter_points) + 1);
}
