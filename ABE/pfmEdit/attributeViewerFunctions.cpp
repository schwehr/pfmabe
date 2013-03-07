
/*********************************************************************************************

    This is public domain software that was developed for the U.S. Army Corps of Engineers
    in support of the Coastal Zone Mapping and Imaging LiDAR (CZMIL) project.

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


/*!
  - method:	ComputeLineDistanceFilter

  - This method will poll through the shots within the edit window.  It will determine for
    each shot the closest point on our line that had been previously drawn for the Distance
    Threshold tool.  Once that point is determined if the shot falls within a certain distance
    we keep it and send it to the Attribute Viewer.
*/

NV_INT32 ComputeLineDistanceFilter (NV_F64_COORD2 lineStart, NV_F64_COORD2 lineEnd, nvMap *map, MISC *misc, OPTIONS *options)
{
  NV_INT32 numPointsWithin = 0;
  NV_FLOAT64 utmE, utmN;
  NV_BOOL hitMax = NVFalse;
  QColor color;


  QColor GetAVShotColor (NV_INT32 index, MISC *misc, OPTIONS *options);


  for (NV_INT32 i = 0; i < misc->abe_share->point_cloud_count; i++)
    {
      //  Only send over what's viewable on the screen
	  
      if ((!check_bounds (map, options, misc, i, NVTrue, NVFalse)) && misc->data[i].type == PFM_CZMIL_DATA)
        {
          NV_FLOAT64 x = misc->data[i].x * NV_DEG_TO_RAD;
          NV_FLOAT64 y = misc->data[i].y * NV_DEG_TO_RAD;

          pj_transform (misc->pj_latlon, misc->pj_utm, 1, 1, &x, &y, NULL);

          utmE = x;
          utmN = y;			

          Vertex ptOfInterest (utmE, utmN, 0.0f);

          Vertex closestPt = Vector::ClosestPtOnLine (Vertex (lineStart.x, lineStart.y, 0.0f), Vertex (lineEnd.x, lineEnd.y, 0.0f), ptOfInterest);


          // compare against our class variable not a constant

          if (Vector::GetDistance (ptOfInterest, closestPt) < options->distThresh)
            {
              // load up the shots array for AV

              misc->avb.shotArray[numPointsWithin].recordNumber = misc->data[i].rec;
              misc->avb.shotArray[numPointsWithin].subRecordNumber = misc->data[i].sub;
              misc->avb.shotArray[numPointsWithin].pfmHandle = misc->pfm_handle[misc->data[i].pfm];
              misc->avb.shotArray[numPointsWithin].fileNo = misc->data[i].file;
              misc->avb.shotArray[numPointsWithin].masterIdx = i;
              misc->avb.shotArray[numPointsWithin].type = misc->data[i].type;

              color = GetAVShotColor (i, misc, options);

              misc->avb.shotArray[numPointsWithin].colorH = color.hue ();
              misc->avb.shotArray[numPointsWithin].colorS = color.saturation();
              misc->avb.shotArray[numPointsWithin].colorV = color.value();

              numPointsWithin++;

              if (numPointsWithin == MAX_ATTRIBUTE_SHOTS)
                {
                  hitMax = NVTrue;


                  //  If we hit the maximum, send a message out for the user to re-select.

                  QString msg = QString (pfmEdit::tr ("The line specified results in more shots satisfying the threshold than can be held (MAX = %1).\n").arg
                                         (MAX_ATTRIBUTE_SHOTS) +
                                         pfmEdit::tr ("Please draw a shorter line or change the distance threshold parameter"));
                  QMessageBox::warning (0, pfmEdit::tr ("pfmEdit"), msg);
                  break;
                }
            }
        }
    }


  // lock shared memory and load up shots and notify AV new data is coming

  misc->abeShare->lock ();

  if (numPointsWithin > 0)
    {
      misc->av_dist_count = numPointsWithin;
      misc->av_dist_list = (NV_INT32 *) realloc (misc->av_dist_list, (misc->av_dist_count + 1) * sizeof (NV_INT32));

      if (misc->av_dist_list == NULL)
        {
          perror ("Allocating misc->av_dist_list memory in pfmEdit");
          exit (-1);
        }

      for (NV_INT32 i = 0; i < numPointsWithin; i++)
        {
          misc->abe_share->avShare.shots[i] = misc->avb.shotArray[i];
          misc->av_dist_list[i] = misc->avb.shotArray[i].masterIdx;
        }
    }

  misc->abe_share->avShare.numShots = numPointsWithin;
  misc->abe_share->avShare.avNewData = NVTrue;
  misc->abe_share->avShare.hitMax = hitMax;

  misc->abeShare->unlock ();

  return (numPointsWithin);
}



/*!
  - method:		GetPixelOffsetByMeter

  - This method will return the amount of pixels on the nvMap that match the argument in
    terms of meters.  The misc->displayed_area class variable comes in handy because it knows
    what the current bounds of the displayed area are.
*/

NV_INT32 GetPixelOffsetByMeter (MISC *misc, nvMap *map)
{
  NV_FLOAT64 lat, lon, dummyMapZ;
  NV_INT32 dummyScreenZ, totalPixels, pix_x[2], pix_y[2];

  newgp (misc->map_center_y, misc->map_center_x, 90.0, misc->avb.avBoxSize, &lat, &lon);
  map->map_to_screen (1, &misc->map_center_x, &misc->map_center_y, &dummyMapZ, &pix_x[0], &pix_y[0], &dummyScreenZ);
  map->map_to_screen (1, &lon, &lat, &dummyMapZ, &pix_x[1], &pix_y[1], &dummyScreenZ);

  totalPixels = pix_x[1] - pix_x[0];

  return (totalPixels);
}



/*!
  - method:		LoadShotsWithinAVBox

  - This method will loop through the shots and compare the screen coordinates of
    the shots to where the target screen position is.  By comparing whether it is
    within a rectangle developed by a pixel offset calculated earlier, it will either
    be rejected or accepted.  If accepted, it will be stored in shared memory.
*/

void LoadShotsWithinAVBox (MISC *misc, OPTIONS *options, nvMap *map, NV_I32_COORD2 targetPt)
{
  NV_FLOAT64 dz;
  NV_INT32 xyz_x, xyz_y, xyz_z, lock_point;
  NV_I32_COORD2 llSquare, urSquare;
  int numShots = 0;
  NV_BOOL hitMax = NVFalse;

  QColor color;

  QColor GetAVShotColor (NV_INT32 index, MISC *misc, OPTIONS *options);


  if (misc->nearest_point != -1)
    {
      llSquare.x = targetPt.x - misc->avb.avInterfacePixelBuffer;
      llSquare.y = targetPt.y + misc->avb.avInterfacePixelBuffer;

      urSquare.x = targetPt.x + misc->avb.avInterfacePixelBuffer;
      urSquare.y = targetPt.y - misc->avb.avInterfacePixelBuffer;


      //  load the target pt first

      lock_point = misc->nearest_point;


      //  Get the screen coordinates of the primary cursor.  Make sure it is within the AV ROI box.
      //  It is possible that it could be outside in the event that everything within the AV box is deleted by the AV

      map->map_to_screen (1, &misc->data[lock_point].x, &misc->data[lock_point].y, &dz, &xyz_x, &xyz_y, &xyz_z);

      if ((xyz_x <= urSquare.x) && (xyz_x >= llSquare.x) && (xyz_y >= urSquare.y) && (xyz_y <= llSquare.y) && misc->data[lock_point].type == PFM_CZMIL_DATA)
        {
          misc->avb.shotArray[numShots].recordNumber = misc->data[lock_point].rec;
          misc->avb.shotArray[numShots].pfmHandle = misc->pfm_handle[misc->data[lock_point].pfm];
          misc->avb.shotArray[numShots].fileNo = misc->data[lock_point].file;
          misc->avb.shotArray[numShots].type = misc->data[lock_point].type;


          //  Save the master idx

          misc->avb.shotArray[numShots].masterIdx = lock_point;

          color = GetAVShotColor (lock_point, misc, options);

          misc->avb.shotArray[numShots].colorH = color.hue ();
          misc->avb.shotArray[numShots].colorS = color.saturation();
          misc->avb.shotArray[numShots].colorV = color.value();

          numShots++;
        }


      for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
        {
          if (i != lock_point)
            {
              //  Check for single line display.

              if (!misc->num_lines || check_line (misc, misc->data[i].line))
                {
                  //  Do not use null points.  Do not use invalid points unless the 
                  //  display_invalid flag is set.  Do not check points that are not on the 
                  //  display.

                  if (!check_bounds (map, options, misc, i, NVTrue, NVFalse))
                    {
                      map->map_to_screen (1, &misc->data[i].x, &misc->data[i].y, &dz, &xyz_x, &xyz_y, &xyz_z);


                      //  Check the area and the data type ('cause there might be mixed types in the editor).

                      if ((xyz_x <= urSquare.x) && (xyz_x >= llSquare.x) && (xyz_y >= urSquare.y) && (xyz_y <= llSquare.y) &&
                          misc->data[i].type == PFM_CZMIL_DATA)
                        {
                          if (numShots == MAX_ATTRIBUTE_SHOTS)
                            {
                              hitMax = NVTrue;
                              break;
                            }

                          misc->avb.shotArray[numShots].recordNumber = misc->data[i].rec;
                          misc->avb.shotArray[numShots].subRecordNumber = misc->data[i].sub;
                          misc->avb.shotArray[numShots].pfmHandle = misc->pfm_handle[misc->data[i].pfm];
                          misc->avb.shotArray[numShots].fileNo = misc->data[i].file;
                          misc->avb.shotArray[numShots].masterIdx = i;
                          misc->avb.shotArray[numShots].type = misc->data[i].type;

                          color = GetAVShotColor (i, misc, options);

                          misc->avb.shotArray[numShots].colorH = color.hue ();
                          misc->avb.shotArray[numShots].colorS = color.saturation();
                          misc->avb.shotArray[numShots].colorV = color.value();

                          numShots++;
                        }
                    }
                }
            }
        }
    }

  misc->abeShare->lock ();

  for (int i = 0; i < numShots; i++) misc->abe_share->avShare.shots[i] = misc->avb.shotArray[i];

  misc->abe_share->avShare.numShots = numShots;
  misc->abe_share->avShare.avNewData = NVTrue;
  misc->abe_share->avShare.hitMax = hitMax;

  misc->abeShare->unlock ();
}



/*!
  - function:	GetAVShotColor

  - This function will look at all of the options set by the interface and will formulate
    a color based on the attributes we are coloring by and the min and maxes.  This code
    is taken from slotPostRedraw.
*/

QColor GetAVShotColor (NV_INT32 index, MISC *misc, OPTIONS *options)
{
  NV_INT32 c_index = -1;
  static NV_FLOAT32 numshades = (NV_FLOAT32) (NUMSHADES - 1);


  //  Use the contour color for reference and null data.

  if ((misc->data[index].val & PFM_REFERENCE) || (misc->data[index].z >= misc->null_val[misc->data[index].pfm]))
    {
      c_index = -1;
    }


  //  Check for color by attribute

  else
    {
      //  Color by depth

      if (!options->color_index)
        {
          //  Color by line in alternate views no matter what.

          if (misc->view)
            {
              for (NV_INT32 j = 0 ; j < misc->line_count ; j++)
                {
                  if (misc->data[index].line == misc->line_number[j])
                    {
                      c_index = j * misc->line_interval;
                      break;
                    }
                }
            }
          else
            {
              c_index = NINT ((misc->data[index].z - misc->avb.actualMin) / misc->avb.actualRange * numshades);


              //  Check for out of range data.

              if (c_index < 0) c_index = 0;
              if (c_index > NUMSHADES - 1) c_index = NUMSHADES - 1;
            }
        }


      //  Color by line

      else if (options->color_index == 1)
        {
          for (NV_INT32 j = 0 ; j < misc->line_count ; j++)
            {
              if (misc->data[index].line == misc->line_number[j])
                {
                  c_index = j * misc->line_interval;
                  break;
                }
            }
        }


      //  Color by horizontal uncertainty

      else if (options->color_index == 2)
        {
          c_index = (NUMSHADES - 1) - NINT ((misc->data[index].herr - misc->avb.attrMin) / misc->avb.attrRange * numshades);
        }


      //  Color by vertical uncertainty

      else if (options->color_index == 3)
        {
          c_index = (NUMSHADES - 1) - NINT ((misc->data[index].verr - misc->avb.attrMin) / misc->avb.attrRange * numshades);
        }


      //  Color by attribute

      else if (options->color_index >= PRE_ATTR && options->color_index < PRE_ATTR + 10)
        {
          NV_INT32 ndx = options->color_index - PRE_ATTR;
          c_index = (NUMSHADES - 1) - NINT ((misc->data[index].attr[ndx] - misc->avb.attrMin) / misc->avb.attrRange * numshades);
        }
    }


  QColor color;

  if (options->color_index == 1)
    {
      if (c_index > (NUMSHADES * 2 - 1)) c_index = NUMSHADES * 2 - 1;
    }
  else
    {
      if (c_index > NUMSHADES - 1) c_index = NUMSHADES - 1;
    }

  if (c_index < 0)
    {
      c_index = 0;
      color = misc->color_array[0][c_index];
    }
  else
    {
	if (options->color_index == 1 || misc->view)
          {
            color = misc->line_color_array[0][c_index];
          }
        else
          {
            color = misc->color_array[0][c_index];
          }
    }

  return color;
}



/*!
  This function will look at the flightline number of a shot and compare it to the line_number array
  stored for the current viewing area and based on the line interval return a color index representing
  the flightline
*/

NV_INT32 GetFlightlineColorIndex (NV_INT32 flightIndex, NV_INT32 *flightLnArray, NV_INT32 noFlights, NV_INT32 interval)
{
  for (NV_INT32 i = 0 ; i < noFlights ; i++)
    {
      if (flightIndex == flightLnArray[i]) return (i * interval);
    }

  return -1;
}



/*!
  - method:		DrawAVInterfaceBox

  - This method is responsible for drawing a red box that will tie to the attribute viewer.  
*/

void DrawAVInterfaceBox (nvMap *map, MISC *misc, NV_F64_COORD3 latlon)
{
  NV_FLOAT64 dummyMapZ;
  NV_INT32 dummyScreenZ;
  NV_I32_COORD2 centerOfBox, llBox, urBox;
	
  NV_FLOAT64 lat = latlon.y;
  NV_FLOAT64 lon = latlon.x;

  map->map_to_screen (1, &lon, &lat, &dummyMapZ, &centerOfBox.x, &centerOfBox.y, &dummyScreenZ);

  llBox.x = centerOfBox.x - misc->avb.avInterfacePixelBuffer;
  llBox.y = centerOfBox.y + misc->avb.avInterfacePixelBuffer;

  urBox.x = centerOfBox.x + misc->avb.avInterfacePixelBuffer;
  urBox.y = centerOfBox.y - misc->avb.avInterfacePixelBuffer;  

  map->setMovingRectangle (&misc->avb.av_rectangle, llBox.x, llBox.y, urBox.x, urBox.y, Qt::red, 2, NVFalse, Qt::SolidLine);	
}
