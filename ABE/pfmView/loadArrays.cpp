
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include <stdlib.h>
#include "nvutility.h"
#include "pfm.h"
#include "pfmViewDef.hpp"

/***************************************************************************/
/*!

  - Module Name:        loadArrays

  - Programmer:         Jan C. Depner

  - Date Written:       November 1999

  - Purpose:            Loads the selected field and the checked flag into
                        the display arrays.

****************************************************************************/

void loadArrays (NV_INT32 layer_type, NV_INT32 count, BIN_RECORD bin_record[], NV_FLOAT32 data[], NV_FLOAT32 attr[], NV_INT32 attr_num,
                 NV_U_CHAR flags[], NV_INT32 highlight, NV_INT32 h_count, NV_INT32 pfm_handle, PFM_OPEN_ARGS open_args, NV_FLOAT32 percent, NV_BOOL surface_val)
{
  NV_INT32            i, j, recnum;
  NV_U_INT32          prev_line;
  DEPTH_RECORD        *depth;


  for (i = 0 ; i < count ; i++)
    {
      //  Do we have any values in the bin or is this possibly an interpolated bin...

      if (bin_record[i].num_soundings || layer_type == AVERAGE_FILTERED_DEPTH)
        {
	  NV_BOOL have_attr = NVFalse;
	  NV_FLOAT32 attr_value = 0.0, attr_null = 0.0;

	  switch (attr_num)
	    {
	    case 1:
	      attr_value = bin_record[i].num_soundings;
	      attr_null = 0.0;
	      have_attr = NVTrue;
	      break;

	    case 2:
	      attr_value = bin_record[i].standard_dev;
	      attr_null = 0.0;
	      have_attr = NVTrue;
	      break;

	    case 3:
              if (surface_val)
                {
                  attr_value = bin_record[i].avg_filtered_depth - bin_record[i].min_filtered_depth;
                }
              else
                {
                  attr_value = bin_record[i].avg_depth - bin_record[i].min_depth;
                }
	      attr_null = -1.0;
	      have_attr = NVTrue;
	      break;

	    case 4:
              if (surface_val)
                {
                  attr_value = bin_record[i].max_filtered_depth - bin_record[i].avg_filtered_depth;
                }
              else
                {
                  attr_value = bin_record[i].max_depth - bin_record[i].avg_depth;
                }
	      attr_null = -1.0;
	      have_attr = NVTrue;
	      break;

	    case 5:
              if (surface_val)
                {
                  attr_value = bin_record[i].max_filtered_depth - bin_record[i].min_filtered_depth;
                }
              else
                {
                  attr_value = bin_record[i].max_depth - bin_record[i].min_depth;
                }
	      attr_null = -1.0;
	      have_attr = NVTrue;
	      break;

	    case PRE_ATTR + 0:
	    case PRE_ATTR + 1:
	    case PRE_ATTR + 2:
	    case PRE_ATTR + 3:
	    case PRE_ATTR + 4:
	    case PRE_ATTR + 5:
	    case PRE_ATTR + 6:
	    case PRE_ATTR + 7:
	    case PRE_ATTR + 8:
	    case PRE_ATTR + 9:
	      attr_value = bin_record[i].attr[attr_num - PRE_ATTR];
	      attr_null = open_args.head.bin_attr_null[attr_num - PRE_ATTR];
	      have_attr = NVTrue;
	      break;
	    }


	  //  Default to null attribute value.

	  if (have_attr) attr[i] = attr_null;


          //  Fill the array with valid data based on the layer type.

          switch (layer_type)
            {
            case AVERAGE_FILTERED_DEPTH:
              if (bin_record[i].validity & (PFM_DATA | PFM_INTERPOLATED))
                {
                  data[i] = bin_record[i].avg_filtered_depth;


		  //  No attribute color for interpolated data

		  if (have_attr && (bin_record[i].validity & PFM_DATA)) attr[i] = attr_value;
                }
              else
                {
                  data[i] = open_args.head.null_depth;
                }
              break;

            case MIN_FILTERED_DEPTH:
              if (bin_record[i].validity & PFM_DATA)
                {
                  data[i] = bin_record[i].min_filtered_depth;
		  if (have_attr) attr[i] = attr_value;
                }
              else
                {
                  data[i] = open_args.head.null_depth;
                }
              break;

            case MAX_FILTERED_DEPTH:
              if (bin_record[i].validity & PFM_DATA)
                {
                  data[i] = bin_record[i].max_filtered_depth;
		  if (have_attr) attr[i] = attr_value;
                }
              else
                {
                  data[i] = open_args.head.null_depth;
                }
              break;

            case AVERAGE_DEPTH:
              data[i] = bin_record[i].avg_depth;
	      if (have_attr) attr[i] = attr_value;
              break;

            case MIN_DEPTH:
              data[i] = bin_record[i].min_depth;
	      if (have_attr) attr[i] = attr_value;
              break;

            case MAX_DEPTH:
              data[i] = bin_record[i].max_depth;
	      if (have_attr) attr[i] = attr_value;
              break;
            }


          //  Fill the flags array for highlighting.

          flags[i] = 0;
          switch (highlight)
            {
            case H_ALL:
              flags[i] = 1;
              break;


            case H_01:
              if (bin_record[i].validity & PFM_USER_01) flags[i] = 1;
              break;


            case H_02:
              if (bin_record[i].validity & PFM_USER_02) flags[i] = 1;
              break;


            case H_03:
              if (bin_record[i].validity & PFM_USER_03) flags[i] = 1;
              break;


            case H_04:
              if (bin_record[i].validity & PFM_USER_04) flags[i] = 1;
              break;


            case H_05:
              if (bin_record[i].validity & PFM_USER_05) flags[i] = 1;
              break;


            case H_INT:
              if ((bin_record[i].validity & PFM_INTERPOLATED) && !(bin_record[i].validity & PFM_DATA)) flags[i] = 1;
              break;


            case H_CHECKED:
              if ((bin_record[i].validity & PFM_CHECKED) || (bin_record[i].validity & PFM_VERIFIED)) flags[i] = 1;
              break;


              //  The following highlight options require us to read ALL of the data (can be slow).

            case H_MULT:
              flags[i] = 0;
              if (!read_depth_array_index (pfm_handle, bin_record[i].coord, &depth, &recnum))
                {
                  prev_line = depth[0].line_number;
                  for (j = 1 ; j < recnum ; j++)
                    {
                      if (depth[j].line_number != prev_line)
                        {
                          flags[i] = 1;
                          break;
                        }
                      prev_line = depth[j].line_number;
                    }
                  free (depth);
                }
              break;


            case H_COUNT:
              flags[i] = 0;
              if (!read_depth_array_index (pfm_handle, bin_record[i].coord, &depth, &recnum))
                {
                  NV_INT32 count = 0;

                  for (j = 0 ; j < recnum ; j++)
                    {
                      if (!(depth[j].validity & (PFM_DELETED | PFM_INVAL)))
                        {
                          count++;
                          if (count > h_count)
                            {
                              flags[i] = 1;
                              break;
                            }
                        }
                    }
                  free (depth);
                }
              break;


            case H_IHO_S:
              flags[i] = 0;


	      //  No need to check against the IHO standards if the average depth is above zero or below 200.

	      if (bin_record[i].avg_filtered_depth > 0.0 && bin_record[i].avg_filtered_depth <= 200.0)
		{
		  if (!read_depth_array_index (pfm_handle, bin_record[i].coord, &depth, &recnum))
		    {
		      NV_FLOAT64 envelope = 0.25 + 0.0075 * bin_record[i].avg_filtered_depth;

		      for (j = 1 ; j < recnum ; j++)
			{
			  if (!(depth[j].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
			    {
			      if (fabs (depth[j].xyz.z - bin_record[i].avg_filtered_depth) > envelope)
				{
				  flags[i] = 1;
				  break;
				}
			    }
			}
		      free (depth);
		    }
		}
              break;


            case H_IHO_1:
              flags[i] = 0;


	      //  No need to check against the IHO standards if the average depth is above zero or below 200.

	      if (bin_record[i].avg_filtered_depth > 0.0 && bin_record[i].avg_filtered_depth <= 200.0)
		{
		  if (!read_depth_array_index (pfm_handle, bin_record[i].coord, &depth, &recnum))
		    {
		      NV_FLOAT64 envelope = 0.5 + 0.013 * bin_record[i].avg_filtered_depth;

		      for (j = 1 ; j < recnum ; j++)
			{
			  if (!(depth[j].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
			    {
			      if (fabs (depth[j].xyz.z - bin_record[i].avg_filtered_depth) > envelope)
				{
				  flags[i] = 1;
				  break;
				}
			    }
			}
		      free (depth);
		    }
		}
              break;


            case H_IHO_2:
              flags[i] = 0;


	      //  No need to check against the IHO standards if the average depth is above zero or below 200.

	      if (bin_record[i].avg_filtered_depth > 0.0 && bin_record[i].avg_filtered_depth <= 200.0)
		{
		  if (!read_depth_array_index (pfm_handle, bin_record[i].coord, &depth, &recnum))
		    {
		      NV_FLOAT64 envelope = 1.0 + 0.023 * bin_record[i].avg_filtered_depth;

		      for (j = 1 ; j < recnum ; j++)
			{
			  if (!(depth[j].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
			    {
			      if (fabs (depth[j].xyz.z - bin_record[i].avg_filtered_depth) > envelope)
				{
				  flags[i] = 1;
				  break;
				}
			    }
			}
		      free (depth);
		    }
		}
	      break;


            case H_PERCENT:
              flags[i] = 0;


	      //  No need to check if the average depth is under 20 meters.

	      if (bin_record[i].avg_filtered_depth > 20.0)
		{
		  if (!read_depth_array_index (pfm_handle, bin_record[i].coord, &depth, &recnum))
		    {
		      NV_FLOAT64 envelope = bin_record[i].avg_filtered_depth * (percent / 100.0);

		      for (j = 1 ; j < recnum ; j++)
			{
			  if (!(depth[j].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
			    {
			      if (fabs (depth[j].xyz.z - bin_record[i].avg_filtered_depth) > envelope)
				{
				  flags[i] = 1;
				  break;
				}
			    }
			}
		      free (depth);
		    }
		}
	      break;
            }
        }


      //  Empty bins need the null value.

      else
        {
          switch (layer_type)
            {
            case AVERAGE_FILTERED_DEPTH:
            case MIN_FILTERED_DEPTH:
            case MAX_FILTERED_DEPTH:
            case AVERAGE_DEPTH:
            case MIN_DEPTH:
            case MAX_DEPTH:
              data[i] = open_args.head.null_depth;
              break;
            }
          flags[i] = 0;
        }
    }
}
