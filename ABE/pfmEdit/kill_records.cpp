
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



#include "pfmEdit.hpp"


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

void kill_records (nvMap *map, OPTIONS *options, MISC *misc, NV_INT32 rb, NV_FLOAT64 lat, NV_FLOAT64 lon)
{
  typedef struct
  {
    NV_INT16      pfm;
    NV_INT16      file;
    NV_U_INT32    rec;
  } RECORDS;


  NV_INT32        x_value, dummy, count = 0, record_count = 0, *px, *py;
  RECORDS         *records = NULL;
  NV_FLOAT64      min_x, max_x, min_y, max_y;
  NV_FLOAT64      *cur_x = NULL, *cur_y = NULL, *cur_z = NULL, *mx, *my;
  NV_BOOL         scan = NVFalse, hit = NVFalse;


  //  First, get the area to be scanned based on the rectangle or polygon.

  switch (options->function)
    {
    case DELETE_RECTANGLE:
      if (map->rubberbandRectangleIsActive (rb))
        {
          map->closeRubberbandRectangle (rb, lon, lat, &px, &py, &mx, &my);

          cur_x = (NV_FLOAT64 *) malloc (5 * sizeof (NV_FLOAT64));
          cur_y = (NV_FLOAT64 *) malloc (5 * sizeof (NV_FLOAT64));
          cur_z = (NV_FLOAT64 *) malloc (5 * sizeof (NV_FLOAT64));
          if (cur_z == NULL)
            {
              fprintf (stderr , pfmEdit::tr ("Error allocating memory - %s %d\n").toAscii (), __FILE__, __LINE__);
              exit (-1);
            }

          if (!misc->view)
            {
              for (NV_INT32 i = 0 ; i < 4 ; i++)
                {
                  if (misc->dateline && mx[i] < 0.0) mx[i] += 360.0;
                  cur_x[i] = mx[i];
                  cur_y[i] = my[i];
                }
              cur_x[4] = cur_x[0];
              cur_y[4] = cur_y[0];
            }
          else
            {
              for (NV_INT32 i = 0 ; i < 4 ; i ++)
                {
                  cur_x[i] = px[i];
                  unscale_view (misc, py[i], &cur_z[i]);
                }
              cur_x[4] = cur_x[0];
              cur_z[4] = cur_z[0];
            }
        }
      count = 5;
      break;

    case DELETE_POLYGON:
      if (map->rubberbandPolygonIsActive (rb))
        {
          map->closeRubberbandPolygon (rb, lon, lat, &count, &px, &py, &mx, &my);

          cur_x = (NV_FLOAT64 *) malloc ((count + 1) * sizeof (NV_FLOAT64));
          cur_y = (NV_FLOAT64 *) malloc ((count + 1) * sizeof (NV_FLOAT64));
          cur_z = (NV_FLOAT64 *) malloc ((count + 1) * sizeof (NV_FLOAT64));
          if (cur_z == NULL)
            {
              fprintf (stderr , pfmEdit::tr ("Error allocating memory - %s %d\n").toAscii (), __FILE__, __LINE__);
              exit (-1);
            }


          if (!misc->view)
            {
              for (NV_INT32 i = 0 ; i < count ; i++)
                {
		  if (misc->dateline && mx[i] < 0.0) mx[i] += 360.0;
                  cur_x[i] = mx[i];
                  cur_y[i] = my[i];
                }
              cur_x[count] = cur_x[0];
              cur_y[count] = cur_y[0];
              count++;
            }
          else
            {
              for (NV_INT32 i = 0 ; i < count ; i ++)
                {
                  cur_x[i] = px[i];
                  unscale_view (misc, py[i], &cur_z[i]);
                }
              cur_x[count] = cur_x[0];
              cur_z[count] = cur_z[0];
              count++;
            }
        }
      break;
    }


  //  Second, scan the area for record numbers and save them in the records array (record_count).

  if (!misc->view)
    {
      //  Get the minimum bounding rectangle (X and Y).

      min_y = cur_y[0];
      max_y = cur_y[0];
      min_x = cur_x[0];
      max_x = cur_x[0];

      for (NV_INT32 j = 0 ; j < count ; j++)
        {
          min_y = qMin (min_y , cur_y[j]);
          max_y = qMax (max_y , cur_y[j]);
          min_x = qMin (min_x , cur_x[j]);
          max_x = qMax (max_x , cur_x[j]);
        }

      for (NV_INT32 i = 0 ; i < misc->last_drawn_index ; i++)
        {
          //  Check for single line display.

          if (!misc->num_lines || check_line (misc, misc->data[i].line))
            {

              //  Check against the minimum bounding rectangle.  This is all that is needed for 
              //  a rectangle and it cuts down on the computations for a polygon.   Also, DO NOT 
              //  allow changes to null value status.

              if (!check_bounds (map, options, misc, i, NVFalse, misc->slice) && misc->data[i].z < misc->null_val[misc->data[i].pfm])
                {
                  scan = NVFalse;

                  switch (options->function)
                    {
                    case DELETE_RECTANGLE:
                      if (misc->data[i].x >= min_x && misc->data[i].x <= max_x && misc->data[i].y >= min_y && misc->data[i].y <= max_y) scan = NVTrue;
                      break;

                    case DELETE_POLYGON:
                      if (inside_polygon2 (cur_x, cur_y, count, misc->data[i].x, misc->data[i].y)) scan = NVTrue;
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
    }
  else
    {
      //  Get the minimum bounding rectangle (X and Z).

      min_y = cur_z[0];
      max_y = cur_z[0];
      min_x = cur_x[0];
      max_x = cur_x[0];
      for (NV_INT32 j = 0 ; j < count ; j++)
        {
          min_y = qMin (min_y , cur_z[j]);
          max_y = qMax (max_y , cur_z[j]);
          min_x = qMin (min_x , cur_x[j]);
          max_x = qMax (max_x , cur_x[j]);
        }

      for (NV_INT32 i = 0 ; i < misc->last_drawn_index ; i++)
        {
          //  Check for single line display.

          if (!misc->num_lines || check_line (misc, misc->data[i].line))
            {
              //  Rotate the points to check them since we can't unrotate from the x and z pixel values.
                
              scale_view (map, misc, misc->data[i].x, misc->data[i].y, misc->data[i].z, &x_value, &dummy, &dummy);


              //  Check against the rectangular or polygonal area that is displayed.

              if (!check_bounds (map, options, misc, i, NVFalse, misc->slice) && misc->data[i].z < misc->null_val[misc->data[i].pfm])
                {
                  scan = NVFalse;

                  switch (options->function)
                    {
                    case DELETE_RECTANGLE:
                      if (x_value >= min_x && x_value <= max_x && misc->data[i].z >= min_y && misc->data[i].z <= max_y) scan = NVTrue;
                      break;

                    case DELETE_POLYGON:
                      if (inside_polygon2 (cur_x, cur_z, count, (NV_FLOAT64) x_value, (NV_FLOAT64) misc->data[i].z)) scan = NVTrue;
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
    }


  free (cur_x);
  free (cur_y);
  free (cur_z);


  //  Third, cycle through all of the visible data and invalidate subrecords of records that showed up in the scanned area.

  if (record_count)
    {
      for (NV_INT32 i = 0 ; i < misc->last_drawn_index ; i++)
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


      //  Close the undo block.

      end_undo_block (misc);
    }
}
