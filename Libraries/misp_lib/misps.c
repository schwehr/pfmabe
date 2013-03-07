/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        misps                                               *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            misps calls the minimum curvature routines and the  *
*                       bicubic spline interpolation routines to compute a  *
*                       regional grid at the final grid spacing for the     *
*                       current subarea.                                    *
*                                                                           *
*   Inputs:             gridint             - Grid interval.                *
*                       x_max               - Maximum X value for subarea   *
*                       x_min               - Minimum X value for subarea   *
*                       y_max               - Maximum Y value for subarea   *
*                       y_min               - Minimum Y value for subarea   *
*                       x_max_bord          - Maximum X value for subarea   *
*                                             including the border.         *
*                       x_min_bord          - Minimum X value for subarea   *
*                                             including the border.         *
*                       y_max_bord          - Maximum Y value for subarea   *
*                                             including the border.         *
*                       y_min_bord          - Minimum Y value for subarea   *
*                                             including the border.         *
*                       search_radius       - Maximum distance from a grid  *
*                                             point that data points are    *
*                                             to be used for regional       *
*                                             computation.  This is used in *
*                                             the weight_mean function.     *
*                                             (default = 20.0)              *
*                       reg_multfact        - Multiplicative factor for     *
*                                             initial coarse grid in        *
*                                             regional computation          *
*                                             (default = 4).                *
*                       row                 - Starting row for misps.       *
*                       column              - Starting column for misps.    *
*                       memblock_height     - Number of grid rows in memory *
*                                             to be used in loadsparse and  *
*                                             readsparse.                   *
*                       memblock_width      - Number of grid columns in     *
*                                             memory for loadsparse and     *
*                                             readsparse.                   *
*                       memblock_total      - Total number of grid cells    *
*                                             in memory for loadsparse and  *
*                                             readsparse.                   *
*                       memblock1           - Memory block 1.               *
*                       memblock2           - Memory block 2.               *
*                       memblock3           - Memory block 3.               *
*                       memblock4           - Memory block 4.               *
*                       memblock5           - Memory block 5.               *
*                       delta               - The value of the maximum      *
*                                             change in the surface between *
*                                             iterations at which           *
*                                             convergence is accepted in    *
*                                             regional computation (see     *
*                                             misp_iterate function).       *
*                                             (default = 0.05).             *
*                       error_control       - Number of times to retry      *
*                                             min_curve after increasing    *
*                                             search_radius.                *
*                                                                           *
*   Outputs:            none                                                *
*                                                                           *
*   Calling Routines:   main                                                *
*                                                                           *
*   Glossary:           subgrid_cols        -   number of columns in the    *
*                                               subarea                     *
*                       subgrid_rows        -   number of rows in the       *
*                                               subarea                     *
*                       reg_subgrid_cols    -   number of columns in the    *
*                                               subarea at regional spacing *
*                       reg_subgrid_rows    -   number of rows in the       *
*                                               subarea at regional spacing *
*                       xtemp1              -   temp variable               *
*                       ytemp1              -   temp variable               *
*                       final_width         -   width of the subarea        *
*                       final_height        -   height of the subarea       *
*                       x_diff1             -                               *
*                       y_diff1             -                               *
*                       reg_width           -   width of the subarea at     *
*                                               regional spacing            *
*                       reg_height          -   height of the subarea at    *
*                                               regional spacing            *
*                       x_end               -   last column of the subarea  *
*                       y_end               -   last row of the subarea     *
*                       x_start             -   first column of the subarea *
*                       y_start             -   first row of the subarea    *
*                       x_edge              -   last column for the edge    *
*                                               match data                  *
*                       width_mult          -   width - mult                *
*                       height_mult         -   height - mult               *
*                       x_max_num           -   loop upper x bound for      *
*                                               sortdata                    *
*                       y_max_num           -   loop upper y bound for      *
*                                               sortdata                    *
*                       xtemp2              -   temp variable               *
*                       ytemp2              -   temp variable               *
*                       row_points          -   number of points returned   *
*                                               from readsparse             *
*                       index1              -   utility integer             *
*                       index2              -   utility integer             *
*                       index3              -   utility integer             *
*                       index4              -   utility integer             *
*                       index5              -   utility integer             *
*                       count1              -   number of non-null points   *
*                                               used for regional           *
*                       count2              -   number of non-null points   *
*                       gridrec             -   offset into memblock1       *
*                       mult2               -                               *
*                       mult3               -                               *
*                       mult4               -                               *
*                       num_points          -   number of points used in    *
*                                               regional computation        *
*                       offset              -   temp offset variable        *
*                       squares             -                               *
*                       error               -   error flag from min_curve   *
*                       colpos              -   column position for x       *
*                                               interpolation               *
*                       rowpos              -   row position for y          *
*                                               interpolation               *
*                       lerror_control      -   local error_control         *
*                       length              -   length of interpolated data *
*                                               returned from interpolate   *
*                       reg_columns         -   number of columns in the    *
*                                               regional field              *
*                       reg_rows            -   number of rows in the       *
*                                               regional field              *
*                       reg_grid            -   total number of grids in    *
*                                               the regional field          *
*                       max_length          -   larger of the two axes      *
*                       reg_quadrant_size   -   size of the reg_quadrant    *
*                                               memory area                 *
*                       sorted_pos_size     -   size of the sorted_pos      *
*                                               memory area                 *
*                       reg_output_size     -   size of the reg_output      *
*                                               memory area                 *
*                       weighted_mean_size  -   size of the weighted_mean   *
*                                               memory area                 *
*                       y_interp_size       -   size of the y_interp        *
*                                               memory area                 *
*                       x_interp_size       -   size of the x_interp        *
*                                               memory area                 *
*                       x_size              -   size of the x memory area   *
*                       y_size              -   size of the y memory area   *
*                       sparse2reg_size     -   size of the sparse2reg      *
*                                               memory area                 *
*                       reg_output          -   pointer to the reg_output   *
*                                               memory area                 *
*                       reg_quadrant        -   pointer to the reg_quadrant *
*                                               memory area                 *
*                       reg_gridint         -   regional grid spacing       *
*                       reg_x_max           -   max x for the regional grid *
*                       reg_y_max           -   max y for the regional grid *
*                       reg_x_min           -   min x for the regional grid *
*                       reg_y_min           -   min y for the regional grid *
*                       reg_x_max_bord      -   max x for the regional plus *
*                                               the border                  *
*                       reg_y_max_bord      -   max y for the regional plus *
*                                               the border                  *
*                       reg_x_min_bord      -   min x for the regional plus *
*                                               the border                  *
*                       reg_y_min_bord      -   min y for the regional plus *
*                                               the border                  *
*                       x_sum               -   sum of non-null x values    *
*                       y_sum               -   sum of non-null y values    *
*                       z_sum               -   sum of non-null z values    *
*                       convergence         -   convergence value for use   *
*                                               in weight_mean              *
*                       x_max_dist          -   max x distance for          *
*                                               weight_mean                 *
*                       y_max_dist          -   max y distance for          *
*                                               weight_mean                 *
*                       search_radius2      -   local search radius squared *
*                       lsearch_radius      -   local search radius         *
*                       endpos              -                               *
*                       sorted_pos          -   pointer to sorted_pos       *
*                                               memory area                 *
*                       z                   -   pointer to z memory area    *
*                       weighted_mean       -   pointer to weighted_mean    *
*                                               memory area                 *
*                       y_interp            -   pointer to y_interp memory  *
*                                               area                        *
*                       x_interp            -   pointer to x_interp memory  *
*                                               area                        *
*                       x                   -   pointer to x memory area    *
*                       y                   -   pointer to y memory area    *
*                       sparse2reg          -   pointer to sparse2reg       *
*                                               memory area                 *
*                       reg_data            -   pointer to reg_data memory  *
*                                               area                        *
*                                                                           *
*   Method:             Calls the minimum curvature routines to compute the *
*                       regional field then uses the bicubic spline         *
*                       interpolation routines to interpolate in the x and  *
*                       then y directions.  This generates a regional field *
*                       at the final grid spacing.                          *
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include "misp.h"


static  MISP_PROGRESS_CALLBACK  misp_progress_callback = NULL;
void misp_register_progress_callback (MISP_PROGRESS_CALLBACK progressCB)
{
    misp_progress_callback = progressCB;
}

NV_BOOL misp_progress_callback_registered ()
{
  if (misp_progress_callback)
    {
      return (NVTrue);
    }
  else
    {
      return (NVFalse);
    }
}


void misp_progress (NV_CHAR *info)
{
  (*misp_progress_callback) (info);
}



/*  Returns NVTrue on error.  */

NV_BOOL misps (NV_FLOAT32 x_gridint, NV_FLOAT32 y_gridint, NV_FLOAT32 x_max, NV_FLOAT32 x_min, 
               NV_FLOAT32 y_max, NV_FLOAT32 y_min, NV_FLOAT32 x_max_bord, 
               NV_FLOAT32 x_min_bord, NV_FLOAT32 y_max_bord, NV_FLOAT32 y_min_bord,
               NV_FLOAT32 search_radius, NV_INT32 reg_multfact, NV_INT32 row, 
               NV_INT32 column, NV_INT32 memblock_height, NV_INT32 memblock_width, 
               NV_INT32 memblock_total, NV_FLOAT32 *memblock1, NV_FLOAT32 *memblock2, 
               NV_FLOAT32 *memblock3, NV_FLOAT32 *memblock4, NV_FLOAT32 *memblock5, 
               NV_FLOAT32 delta, NV_INT32 error_control, MISP_HEADER *final)
{
    NV_INT32   subgrid_cols, subgrid_rows, reg_subgrid_cols, reg_subgrid_rows,
               xtemp1, ytemp1, final_width, final_height, x_diff1, y_diff1,
               reg_width, reg_height, x_end, y_end, x_start, y_start, x_edge, 
               width_mult, height_mult, x_max_num, y_max_num, xtemp2, ytemp2,
               row_points, index1, index2, index3, index5, count1, count2,
               gridrec, mult2, mult3, mult4, num_points, offset, squares, 
               error, colpos, rowpos, lerror_control, length, reg_columns, 
               reg_rows, reg_grid, max_length, reg_quadrant_size, 
               sorted_pos_size, reg_output_size, weighted_mean_size, 
               y_interp_size, x_interp_size, x_size, y_size, sparse2reg_size, 
               *reg_output, *reg_quadrant;
                   
    NV_FLOAT32 reg_x_gridint, reg_y_gridint, reg_x_max, reg_y_max, reg_x_max_bord, 
               reg_y_max_bord, reg_x_min_bord, reg_y_min_bord,
               x_sum, y_sum, z_sum, convergence, x_max_dist, y_max_dist,
               search_radius2, lsearch_radius, endpos, *sorted_pos, *z,
               *weighted_mean, *y_interp, *x_interp, *x, *y, *sparse2reg,
               *reg_data;

    NV_FLOAT64 reg_x_min, reg_y_min;


    NV_CHAR    info[512];


    void readsparse (NV_INT32, NV_INT32, NV_INT32, NV_INT32, NV_INT32, 
        NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, 
        NV_INT32, NV_INT32 *);
    void sortdata (NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, 
        NV_FLOAT32 *, NV_FLOAT32, NV_FLOAT64, NV_FLOAT32, NV_FLOAT64, NV_INT32,
        NV_INT32, NV_FLOAT32, NV_FLOAT32, NV_INT32, NV_INT32);
    void min_curve (NV_FLOAT32 *, NV_INT32 *, NV_FLOAT32 *, NV_FLOAT32 *, 
        NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32 *, NV_INT32, NV_INT32, NV_INT32, 
        NV_INT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, NV_INT32 *, NV_FLOAT64, 
        NV_FLOAT64, NV_FLOAT32, NV_FLOAT32, NV_INT32, NV_INT32, NV_FLOAT32, 
        NV_FLOAT32, NV_FLOAT32, NV_FLOAT32);
    void interpolate (NV_FLOAT32, NV_INT32, NV_FLOAT32, NV_FLOAT32, NV_INT32 *,
        NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *);


    /*  Set the local variables.                                        */
    
    lsearch_radius = search_radius;
    lerror_control = error_control;
    reg_x_gridint = x_gridint * MULT;
    reg_y_gridint = y_gridint * MULT;
    if (lsearch_radius < reg_x_gridint) lsearch_radius = reg_x_gridint;
    if (lsearch_radius < reg_y_gridint) lsearch_radius = reg_y_gridint;
    gridrec = 0;


    /*  Compute the memory sizes.  Minimum curvature routines require   */
    /*  the regional grid to be dimensioned a multiple of reg_multfact, */
    /*  and at least of size reg_multfact * MULT + 1.  Similarly, the   */
    /*  final grid is dimensioned a multiple of MULT.                   */

    subgrid_cols = (NV_INT32) (((x_max - x_min) / x_gridint + 1.0) + 0.5);
    subgrid_rows = (NV_INT32) (((y_max - y_min) / x_gridint + 1.0) + 0.5);
    reg_subgrid_cols = (NV_INT32) (((x_max_bord - x_min_bord - x_gridint) /
        reg_x_gridint + MULT - 1.0));
    reg_subgrid_rows = (NV_INT32) (((y_max_bord - y_min_bord - y_gridint) /
        reg_y_gridint + MULT - 1.0));
    xtemp1 = (NV_INT32) (((x_max_bord - x_min_bord - x_gridint) / (reg_x_gridint * 
        reg_multfact) + MULT - 1.0));
    if (xtemp1 < 4) xtemp1 = 4;
    ytemp1 = (NV_INT32) (((y_max_bord - y_min_bord - y_gridint) / (reg_y_gridint * 
        reg_multfact) + MULT - 1.0));
    if (ytemp1 < 4) ytemp1 = 4;

    xtemp2  = ((xtemp1 - 1) * reg_multfact - reg_subgrid_cols + 1) / 2;
    ytemp2 = ((ytemp1 - 1) * reg_multfact - reg_subgrid_rows + 1) / 2;
    reg_x_min  = x_min_bord - xtemp2 * reg_x_gridint;
    reg_y_min = y_min_bord - ytemp2 * reg_y_gridint;

    reg_x_max  = reg_x_min + (xtemp1 - 1) * reg_multfact * reg_x_gridint;
    reg_y_max = reg_y_min + (ytemp1 - 1) * reg_multfact * reg_y_gridint;
    reg_x_min_bord  = x_min_bord;
    reg_y_min_bord = y_min_bord;
    reg_x_max_bord  = reg_x_min_bord + (reg_subgrid_cols - 1) * reg_x_gridint;
    reg_y_max_bord = reg_y_min_bord + (reg_subgrid_rows - 1) * reg_y_gridint;
    final_width = (NV_INT32) (((reg_x_max_bord - reg_x_min_bord) / x_gridint + 
        1.0) + 0.5);
    final_height = (NV_INT32) (((reg_y_max_bord - reg_y_min_bord) / y_gridint + 
        1.0) + 0.5);
    x_diff1 = (NV_INT32) (((x_min - x_min_bord) / x_gridint) + 0.5);
    y_diff1 = (NV_INT32) (((y_min - y_min_bord) / y_gridint) + 0.5);
    reg_width = (NV_INT32) (((reg_x_max - reg_x_min) / reg_x_gridint + 1.0) + 
        0.5);
    reg_height = (NV_INT32) (((reg_y_max - reg_y_min) / reg_y_gridint + 1.0) + 
        0.5);


    if (misp_progress_callback)
      {
        sprintf (info, "%d Rows",subgrid_rows);
        (*misp_progress_callback) (info);
        sprintf (info, "%d Columns",subgrid_cols);
        (*misp_progress_callback) (info);
      }
    else
      {
        printf ("       %d Rows\n",subgrid_rows);
        printf ("       %d Columns\n\n\n",subgrid_cols);
      }

    reg_columns = memblock_width / MULT + 1;
    reg_rows    = memblock_height / MULT + 1;
    reg_grid    = (reg_columns + 1) * (reg_rows + 1);
    
    if (memblock_width > memblock_height)
    {
        max_length = memblock_width;
    }
    else
    {
        max_length = memblock_height;
    }

    /*  Compute the offsets for all the pointers.                       */
    
    reg_quadrant_size = (reg_width + 1) * reg_height;
    sorted_pos_size = (reg_width + 1) * reg_height;
    reg_output_size = reg_width + 1;
    weighted_mean_size = (reg_width + 1) * reg_height;
    y_interp_size = max_length;
    x_interp_size = max_length;
    x_size = (reg_width + 1) * reg_height;
    y_size = (reg_width + 1) * reg_height;
    sparse2reg_size = (MULT + 1) * max_length + final_width;

    /*  Calculate the addresses for all the pointers.                   */
    
    reg_quadrant = (NV_INT32 *) (memblock1);
    sorted_pos = (memblock1 + reg_quadrant_size);
    reg_output = (NV_INT32 *) (memblock1 + reg_quadrant_size + 
        sorted_pos_size);
    z = (memblock1 + reg_quadrant_size + sorted_pos_size + reg_output_size);
    weighted_mean = (memblock2);
    y_interp = (memblock2 + weighted_mean_size);                
    x_interp = (memblock2 + weighted_mean_size + y_interp_size);
    x = (memblock2 + weighted_mean_size + y_interp_size + x_interp_size);
    y = (memblock2 + weighted_mean_size + y_interp_size + x_interp_size +
        x_size);               
    sparse2reg = (memblock2 + weighted_mean_size + y_interp_size +
        x_interp_size + x_size + y_size);
    reg_data = (memblock2 + weighted_mean_size + y_interp_size +
        x_interp_size + x_size + y_size + sparse2reg_size);

    row_points = 0;

    /*  Read the appropriate section of preliminary sparse input grid   */
    /*  and store it into memory.                                       */

    readsparse (row, column, memblock_height, memblock_width, memblock_total,
        memblock1, memblock3, memblock4, memblock5, final_height,
        final_width, &row_points);

    /*  If no points were found for this area output a header with the  */
    /*  x_start value set negative to signify this.                     */
    
    if (row_points <= 1)
    {
      if (misp_progress_callback)
        {
          (*misp_progress_callback) ("ERROR - No input points found for enlarged grid:");
        }
      else
        {
          printf ("ERROR - No input points found for enlarged grid:\n");
        }

        /*
        printf ("        Longitude West = %f\n",reg_x_min_bord);
        printf ("        Longitude East = %f\n",reg_x_max_bord);
        printf ("        Latitude South = %f\n",reg_y_min_bord);
        printf ("        Latitude North = %f\n\n\n",reg_y_max_bord);
        */

        x_end = x_diff1 + subgrid_cols - 1;
        y_end = y_diff1 + subgrid_rows - 1;
        x_start = -x_diff1;
        x_edge = x_diff1 + 18;

        final->grid_cols = subgrid_cols;
        final->grid_rows = subgrid_rows;
        final->x_min = x_min;
        final->y_min = y_min;
        final->x_max = x_max;
        final->y_max = y_max;
        final->width = final_width;
        final->height = final_height;

        return (NVTrue);
    }

    /*  Definition of 'actual' output indexes and array dimensions.     */

    x_start = x_diff1;
    y_start = y_diff1;
    x_end = x_start + subgrid_cols;
    y_end = y_start + subgrid_rows;

    gridrec = 0;


    /*  Start the regional computation, use a smoothed set of data from */
    /*  the grid as input to min_curve (regional) both position and     */
    /*  value are smoothed in this step.                                */

    count1 = 0;
    height_mult = final_height - MULT;
    width_mult = final_width - MULT;
    for (index1 = 0; index1 < height_mult; index1 += MULT)
    {
        mult2 = MULT;
        if (index1 == (height_mult - 1)) mult2 = MULT + 1;

        for (index2 = 0; index2 < mult2; index2++)
        {
            offset = index2 * max_length;
            for (index3 = 0; index3 < final_width; index3++)
            {
                *(sparse2reg + offset + index3) = *(memblock1 + (index3 * 
                    final_height) + gridrec);
            }
            gridrec++;
        }

        for (index2 = 0; index2 < width_mult; index2 += MULT)
        {
            mult3 = MULT;
            if (index2 == (width_mult - 1)) mult3 += 1;
            mult4 = index2 + mult3;
            x_sum = 0.0;
            y_sum = 0.0;
            z_sum = 0.0;
            count2 = 0;
            for (index3 = index2; index3 < mult4; index3++)
            {
                for (index5 = 0; index5 < mult2; index5++)
                {

                    /*  Sum the non-null values.                        */
                    
                    if (*(sparse2reg + (index5 * max_length) + index3) <
                        MISPNULL)
                    {
                        x_sum += index3;
                        y_sum += (index5 + index1);
                        z_sum += *(sparse2reg + (index5 * max_length) + 
                            index3);
                        count2++;
                    }
                }
            }

            /*  If there were non-null values, average them.            */
            
            if (count2)
            {
                *(reg_data + (0 * reg_grid) + count1) =
                    (x_sum / (NV_FLOAT32) count2) * x_gridint + reg_x_min_bord;
                *(reg_data + (1 * reg_grid) + count1) =
                    (y_sum / (NV_FLOAT32) count2) * y_gridint + reg_y_min_bord;
                *(reg_data + (2 * reg_grid) + count1) =
                    z_sum / (NV_FLOAT32) count2;
                count1++;
            }
        }
    }


    if (misp_progress_callback)
      {
        sprintf (info, "%d Points used for regional computation.", count1);
        (*misp_progress_callback) (info);
      }
    else
      {
        printf ("%d Points used for regional computation.\n\n\n", count1);
      }


    num_points = count1;

    /*  Use the minimum curvature routines to compute the regional grid */
    /*  at the regional grid spacing.                                   */
    
    if (misp_progress_callback) (*misp_progress_callback) ("Computing regional surface");

    do
    {
        convergence = (reg_x_max - reg_x_min) * .000001;
        squares = 4;

        /*  Compute the loop upper bounds for the sortdata routine.     */
        
        do
        {
            x_max_num = (NV_INT32) ((reg_x_max - reg_x_min) * squares / (2.0 * 
                lsearch_radius) + .0001);
            y_max_num = (NV_INT32) ((reg_y_max - reg_y_min) * squares / (2.0 * 
                lsearch_radius) + .0001);
            if (x_max_num > reg_columns) lsearch_radius *= 1.5;
            if (y_max_num > reg_rows) lsearch_radius *= 1.5;
        } while ((x_max_num > reg_columns) || (y_max_num > reg_rows));

        if (x_max_num == 0) x_max_num = 1;
        if (y_max_num == 0) y_max_num = 1;

        x_max_dist = (reg_x_max - reg_x_min) / x_max_num;
        y_max_dist = (reg_y_max - reg_y_min) / y_max_num;
    
        search_radius2 = lsearch_radius * lsearch_radius;
        error = NVFalse;


        /*  Initialize the arrays.                                      */

        memset (weighted_mean, 0, weighted_mean_size * sizeof (NV_FLOAT32));
        memset (sorted_pos, 0, sorted_pos_size * sizeof (NV_FLOAT32));
        memset (reg_quadrant, 0, reg_quadrant_size * sizeof (NV_INT32));
        memset (reg_output, 0, reg_output_size * sizeof (NV_INT32));
        

        /*  Call minimum curvature subroutines to compute regional      */
        /*  grid.                                                       */

        if (misp_progress_callback) (*misp_progress_callback) ("");
        sortdata (sorted_pos, x, y, z, reg_data, reg_x_max, reg_x_min,
                  reg_y_max, reg_y_min, x_max_num, y_max_num, x_max_dist, y_max_dist,
                  num_points, reg_grid);


        if (misp_progress_callback) (*misp_progress_callback) ("");
        min_curve (weighted_mean, reg_quadrant, x, y, z, sorted_pos,
                   reg_output, reg_width, reg_height, x_max_num, y_max_num,
                   reg_x_gridint, reg_y_gridint, delta, &error, reg_x_min, reg_y_min,
                   x_max_dist, y_max_dist, num_points, reg_multfact, squares,
                   lsearch_radius, search_radius2, convergence);
        if (misp_progress_callback) (*misp_progress_callback) ("");


        /*  Increase the search radius if neccessary, continue loop     */
        /*  until no longer neccessary or lerror_control has been       */
        /*  exceeded.                                                   */
        
        if (error)
          {
            if (lerror_control == 0) exit (0);

            if (misp_progress_callback)
              {
                sprintf (info, "Input search radius %f, too small...", lsearch_radius);
                (*misp_progress_callback) (info);
              }
            else
              {
                printf ("\n\nInput search radius %f, too small...\n", lsearch_radius);
              }

            lsearch_radius *= 1.5;

            if (misp_progress_callback)
              {
                sprintf (info, "It is increased to %f", lsearch_radius);
                (*misp_progress_callback) (info);
              }
            else
              {
                printf ("It is increased to %f\n\n\n", lsearch_radius);
              }

            lerror_control--;
          }
    } while (error);

    /*  The regional grid is completed at a reg_(x,y)gridint spacing for     */
    /*  working regional area (current subarea) use cubic spline to     */
    /*  fill memblock1 with regional values at a (x,y)gridint spacing.       */
    
    colpos = (reg_x_min - (reg_x_min_bord + 0.001)) / x_gridint;
    rowpos = (reg_y_min - (reg_y_min_bord + 0.001)) / y_gridint;
    

    /*  Interpolate in the x direction (at regional spacing in y, final */
    /*  spacing in x).                                                  */
    
    if (misp_progress_callback) (*misp_progress_callback) ("Interpolating in X direction");

    for (index1 = 0; index1 < reg_height; index1++)
    {
        for (index2 = 0; index2 < reg_width; index2++)
        {
            *(x + index2) = index2 * MULT + colpos;
            *(y + index2) = *(weighted_mean + (index2 * reg_height) + index1);
        }

        endpos = final_width;
        interpolate (1.0, reg_width, 0.0, endpos, &length, x, y, x_interp,
            y_interp);

        for (index2 = 0; index2 < final_width; index2++)
        {
            *(memblock1 + (index2 * final_height) + index1) = *(y_interp +
                index2);
        }
        if (misp_progress_callback) (*misp_progress_callback) ("");
    }


    /*  Interpolate in the y direction (at final spacing in x and y).   */

    if (misp_progress_callback) (*misp_progress_callback) ("Interpolating in Y direction");

    for (index1 = 0; index1 < final_width; index1++)
    {
        for (index2 = 0; index2 < reg_height; index2++)
        {
            *(x + index2) = index2 * MULT + rowpos;
            *(y + index2) = *(memblock1 + (index1 * final_height) + index2);
        }

        endpos = final_height;
        interpolate (1.0, reg_height, 0.0, endpos, &length, x, y, x_interp,
            y_interp);

        for (index2 = 0; index2 < final_height; index2++)
        {
            *(memblock1 + (index1 * final_height) + index2) =
                *(y_interp + index2);
        }
        if (misp_progress_callback) (*misp_progress_callback) ("");
    }


    /*  Save the header for the subgrid.                           */
    
    final->grid_cols = subgrid_cols;
    final->grid_rows = subgrid_rows;
    final->x_min = x_min;
    final->y_min = y_min;
    final->x_max = x_max;
    final->y_max = y_max;
    final->width = final_width;
    final->height = final_height;

    return (NVFalse);
}
