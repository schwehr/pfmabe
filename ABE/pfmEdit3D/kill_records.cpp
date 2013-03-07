
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



#include "pfmEdit3D.hpp"


/***************************************************************************/
/*!

  - Module Name:        kill_records

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 2009

  - Purpose:            Invalidates records (pings) that are contained
                        within a rectangle or polygon.  Unlike set_area,
                        this function determines which records (pings) are
                        contained within the rectangle or polygon and then
                        invalidates all visible records (pings) in the
                        displayed data.  Even those outside of the rectangle
                        or polygon.

  - Arguments:
                        - map             =   nvmap object to draw on
                        - options         =   OPTIONS structure
                        - misc            =   MISC structure
                        - data            =   point data used to define bounds

  - Return Value:
                        - void

****************************************************************************/

void kill_records (nvMapGL *map, OPTIONS *options, MISC *misc, NV_INT32 *rb, NV_INT32 x, NV_INT32 y)
{
  typedef struct
  {
    NV_INT16      pfm;
    NV_INT16      file;
    NV_U_INT32    rec;
  } RECORDS;


  NV_INT32        count = 0, record_count = 0, *px, *py, min_x = 0, max_x = 0, min_y = 0, max_y = 0;
  RECORDS         *records = NULL;
  NV_FLOAT64      *mx = NULL, *my = NULL;
  NV_BOOL         scan = NVFalse, hit = NVFalse;


  //  First, get the area to be scanned based on the rectangle or polygon.

  switch (options->function)
    {
    case DELETE_RECTANGLE:
      if (map->rubberbandRectangleIsActive (*rb))
        {
          map->closeRubberbandRectangle (*rb, x, y, &px, &py);

          map->discardRubberbandRectangle (rb);
        }
      count = 4;


      //  Get the minimum bounding rectangle (X and Y in pixels).

      min_y = 9999999;
      max_y = -1;
      min_x = 9999999;
      max_x = -1;
      for (NV_INT32 j = 0 ; j < count ; j++)
        {
          min_y = qMin (min_y, py[j]);
          max_y = qMax (max_y, py[j]);
          min_x = qMin (min_x, px[j]);
          max_x = qMax (max_x, px[j]);
        }

      break;

    case DELETE_POLYGON:

      if (map->rubberbandPolygonIsActive (*rb))
        {
          map->closeRubberbandPolygon (*rb, x, y, &count, &px, &py);


          //  If it's a polygon we have to convert to NV_FLOAT64 so that the "inside" function will work.

          mx = (NV_FLOAT64 *) malloc (count * sizeof (NV_FLOAT64));

          if (mx == NULL)
            {
              perror ("Allocating mx array in set_area");
              exit (-1);
            }

          my = (NV_FLOAT64 *) malloc (count * sizeof (NV_FLOAT64));

          if (my == NULL)
            {
              perror ("Allocating my array in set_area");
              exit (-1);
            }

          for (NV_INT32 i = 0 ; i < count ; i++)
            {
              mx[i] = (NV_FLOAT64) px[i];
              my[i] = (NV_FLOAT64) py[i];
            }

          map->discardRubberbandPolygon (rb);
        }
      break;
    }



  //  Second, scan the area for record numbers and save them in the records array (record_count).

  NV_INT32 cx = -1, cy = -1;

  for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
    {
      //  Check for single line display.

      if (!misc->num_lines || check_line (misc, misc->data[i].line))
        {
          //  Check against the displayed minimum bounding rectangle.  This is all that is needed for 
          //  a rectangle and it cuts down on the computations for a polygon.   Also, DO NOT 
          //  allow changes to null value status.

          if (!check_bounds (options, misc, i, NVFalse, misc->slice) && misc->data[i].z < misc->null_val[misc->data[i].pfm])
            {

              //  Convert the X, Y, and Z value to a projected pixel position

              map->get2DCoords (misc->data[i].x, misc->data[i].y, -misc->data[i].z, &cx, &cy);


              scan = NVFalse;

              switch (options->function)
                {
                case DELETE_RECTANGLE:
                  if (cx >= min_x && cx <= max_x && cy >= min_y && cy <= max_y) scan = NVTrue;
                  break;

                case DELETE_POLYGON:
                  if (inside_polygon2 (mx, my, count, (NV_FLOAT64) cx, (NV_FLOAT64) cy)) scan = NVTrue;
                  break;
                }


              if (scan)
                {
                  hit = NVFalse;

                  for (NV_INT32 k = 0 ; k < record_count ; k++)
                    {
                      if (misc->data[i].pfm == records[k].pfm && misc->data[i].file == records[k].file && misc->data[i].rec == records[k].rec)
                        {
                          hit = NVTrue;
                          break;
                        }
                    }

                  if (!hit)
                    {
                      records = (RECORDS *) realloc (records, (record_count + 1) * sizeof (RECORDS));

                      if (records == NULL)
                        {
                          perror ("Allocating records memory in kill_records.cpp");
                          exit (-1);
                        }

                      records[record_count].pfm = misc->data[i].pfm;
                      records[record_count].file = misc->data[i].file;
                      records[record_count].rec = misc->data[i].rec;

                      record_count++;
                    }
                }
            }
        }
    }


  if (mx != NULL)
    {
      free (mx);
      free (my);
    }


  //  Third, cycle through all of the visible data and invalidate subrecords of records that showed up in the scanned area.

  if (record_count)
    {
      for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
        {
          for (NV_INT32 j = 0 ; j < record_count ; j++)
            {
              if (misc->data[i].pfm == records[j].pfm && misc->data[i].file == records[j].file && misc->data[i].rec == records[j].rec)
                {
                  //  Save the undo information.

                  store_undo (misc, options->undo_levels, misc->data[i].val, i);


                  misc->data[i].val |= PFM_MANUALLY_INVAL;
                  break;
                }
            }
        }

      free (records);


      //  Close the undo block

      end_undo_block (misc);
    }
}
