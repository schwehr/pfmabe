
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



#include "pfmView.hpp"


//!  Sets a BFD point at the lowest or highest point in the supplied rectangle.

NV_BOOL setHighLow (NV_FLOAT64 *mx, NV_FLOAT64 *my, MISC *misc, OPTIONS *options, nvMap *map)
{
  NV_INT32            i, j;
  QString             string;
  NV_FLOAT32          min_val = 999999999.0;
  NV_FLOAT32          max_val = -999999999.0;


  NV_F64_XYMBR bounds;

  bounds.min_x = 999999.0;
  bounds.min_y = 999999.0;
  bounds.max_x = -999999.0;
  bounds.max_y = -999999.0;

  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      if (misc->dateline && mx[i] < 0.0) mx[i] += 360.0;

      if (mx[i] < bounds.min_x) bounds.min_x = mx[i];
      if (my[i] < bounds.min_y) bounds.min_y = my[i];
      if (mx[i] > bounds.max_x) bounds.max_x = mx[i];
      if (my[i] > bounds.max_y) bounds.max_y = my[i];
    }


  NV_INT32 width = (NINT ((bounds.max_x - bounds.min_x) / misc->abe_share->open_args[0].head.x_bin_size_degrees));
  NV_INT32 height = (NINT ((bounds.max_y - bounds.min_y) / misc->abe_share->open_args[0].head.y_bin_size_degrees));
  NV_INT32 row = NINT ((bounds.min_y - misc->abe_share->open_args[0].head.mbr.min_y) /
                       misc->abe_share->open_args[0].head.y_bin_size_degrees);
  NV_INT32 column = NINT ((bounds.min_x - misc->abe_share->open_args[0].head.mbr.min_x) / 
                          misc->abe_share->open_args[0].head.x_bin_size_degrees);


  BIN_RECORD *bin, min_bin, max_bin;
  bin = (BIN_RECORD *) malloc (width * sizeof (BIN_RECORD));
  if (bin == NULL)
    {
      perror ("Allocating bin record in setHighLow");
      exit (-1);
    }

  memset (&min_bin, 0, sizeof (BIN_RECORD));
  memset (&max_bin, 0, sizeof (BIN_RECORD));


  //  Loop for the height of the rectangle.

  for (i = 0 ; i < height ; i++)
    {
      read_bin_row (misc->pfm_handle[0], width, row + i, column, bin);

      //  Loop for the width of the rectangle.
    
      for (j = 0 ; j < width ; j++)
        {
          if (bin[j].validity & PFM_DATA)
            {
              if (bin[j].min_filtered_depth < min_val)
                {
                  min_val = bin[j].min_filtered_depth;
                  min_bin = bin[j];
                }

              if (bin[j].max_filtered_depth > max_val)
                {
                  max_val = bin[j].max_filtered_depth;
                  max_bin = bin[j];
                }
            }
        }
    }

  free (bin);


  NV_INT32 x, y, z, recnum;
  QString depth_string, question;
  DEPTH_RECORD *depth;
  BFDATA_RECORD bfd_record;

  memset (&bfd_record, 0, sizeof (BFDATA_RECORD));

  bfd_record.record_number = misc->bfd_header.number_of_records;
  bfd_record.confidence_level = 3;
  strcpy (bfd_record.analyst_activity, "NAVOCEANO BHY");
  bfd_record.equip_type = 3;
  bfd_record.nav_system = 1;
  bfd_record.platform_type = 4;
  bfd_record.sonar_type = 3;


  switch (misc->function)
    {
    case SELECT_HIGH_POINT:
      read_depth_array_index (misc->pfm_handle[0], min_bin.coord, &depth, &recnum);

      for (NV_INT32 i = 0 ; i < recnum ; i++)
        {
          if (!(depth[i].validity & (PFM_INVAL | PFM_REFERENCE | PFM_DELETED)) && fabs (depth[i].xyz.z - min_val) < 0.001)
            {
              bfd_record.latitude = depth[i].xyz.y;
              bfd_record.longitude = depth[i].xyz.x;
              bfd_record.depth = (NV_FLOAT32) depth[i].xyz.z;
              break;
            }
        }
      strcpy (bfd_record.description, "High point");
      depth_string.sprintf ("High point :: %.2f ", bfd_record.depth);
      question = pfmView::tr ("Do you wish to accept the high point?");
      break;

    case SELECT_LOW_POINT:
      read_depth_array_index (misc->pfm_handle[0], max_bin.coord, &depth, &recnum);
      for (NV_INT32 i = 0 ; i < recnum ; i++)
        {
          if (fabs (depth[i].xyz.z - max_val) < 0.001)
            {
              bfd_record.latitude = depth[i].xyz.y;
              bfd_record.longitude = depth[i].xyz.x;
              bfd_record.depth = (NV_FLOAT32) depth[i].xyz.z;
              break;
            }
        }
      strcpy (bfd_record.description, "Low point");
      depth_string.sprintf ("Low point :: %.2f ", bfd_record.depth);
      question = pfmView::tr ("Do you wish to accept the low point?");
      break;
    }
  sprintf (bfd_record.remarks, "%.2f", bfd_record.depth);



  free (depth);


  NV_FLOAT64 dz = 0.0;
  NV_INT32 mvc = -1, mvt = -1;
  map->map_to_screen (1, &bfd_record.longitude, &bfd_record.latitude, &dz, &x, &y, &z);
  map->setMovingCircle (&mvc, x, y, 12, 0.0, 360.0, LINE_WIDTH, options->feature_color, NVFalse, Qt::SolidLine);
  map->setMovingText (&mvt, depth_string, x + 6, y + 3, options->feature_color, 90.0, 12);
  map->flush ();


  QMessageBox msgBox (map);
  msgBox.setIcon (QMessageBox::Question);
  msgBox.setInformativeText (question);
  msgBox.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton (QMessageBox::Yes);
  NV_INT32 ret = msgBox.exec ();

  map->closeMovingCircle (&mvc);
  map->closeMovingText (&mvt);

  if (ret == QMessageBox::Yes)
    {
      /*  If there was no feature file, create one.  */

      if (!misc->bfd_open)
        {
          QString name = misc->abe_share->open_args[0].list_path;
          name += ".bfd";


          QFileDialog fd (map, pfmView::tr ("pfmView Assign Feature File"));
          fd.setViewMode (QFileDialog::List);
          fd.setDirectory (QFileInfo (misc->abe_share->open_args[0].list_path).dir ());

          fd.setFilter (pfmView::tr ("Binary Feature Data file (*.bfd)"));
          fd.setFileMode (QFileDialog::AnyFile);
          fd.selectFile (name);


          QStringList files;
          QString file;
          if (fd.exec () == QDialog::Accepted)
            {
              files = fd.selectedFiles ();

              file = files.at (0);

              if (file.isEmpty()) return (NVFalse);


              if (!file.endsWith (".bfd")) file.append (".bfd");


              //  Check for a pre-existing file

              NV_CHAR filename[512];
              strcpy (filename, file.toAscii ());

              if ((misc->bfd_handle = binaryFeatureData_open_file (filename, &misc->bfd_header, BFDATA_UPDATE)) < 0)
                {
                  strcpy (misc->abe_share->open_args[0].target_path, file.toAscii ());

                  update_target_file (misc->pfm_handle[0], misc->abe_share->open_args[0].list_path,
                                      misc->abe_share->open_args[0].target_path);

                  memset (&misc->bfd_header, 0, sizeof (BFDATA_HEADER));

                  strcpy (misc->bfd_header.creation_software, VERSION);

                  if ((misc->bfd_handle = binaryFeatureData_create_file (misc->abe_share->open_args[0].target_path, misc->bfd_header)) < 0)
                    {
                      QString msg = QString (binaryFeatureData_strerror ());
                      QMessageBox::warning (map, pfmView::tr ("pfmView"), pfmView::tr ("Unable to create feature file\nReason: ") + msg);
                      return (NVFalse);
                    }

                  misc->bfd_open = NVTrue;
                }
              else
                {
                  if (misc->bfd_open) binaryFeatureData_close_file (misc->bfd_handle);
                  misc->bfd_open = NVFalse;

                  if ((misc->bfd_handle = bfd_check_file (misc, misc->abe_share->open_args[0].target_path, &misc->bfd_header, BFDATA_UPDATE)) < 0)
                    {
                      QString msg = QString (binaryFeatureData_strerror ());
                      QMessageBox::warning (map, pfmView::tr ("pfmView"), pfmView::tr ("Unable to open feature file\nReason: ") + msg);
                      return (NVFalse);
                    }

                  if (binaryFeatureData_read_all_short_features (misc->bfd_handle, &misc->feature) < 0)
                    {
                      QString msg = QString (binaryFeatureData_strerror ());
                      QMessageBox::warning (map, pfmView::tr ("pfmView"), pfmView::tr ("Unable to read feature records\nReason: ") + msg);
                      binaryFeatureData_close_file (misc->bfd_handle);
                      return (NVFalse);
                    }
                  else
                    {
                      misc->bfd_open = NVTrue;
                    }
                }
            }
          else
            {
              return (NVFalse);
            }
        }


      map->drawCircle (x, y, 12, 0.0, 360.0, LINE_WIDTH, options->feature_color, Qt::SolidLine, NVFalse);
      map->drawText (depth_string, x + 6, y + 3, options->feature_color, NVFalse);
      map->flush ();


      //  Append a record to the file.

      binaryFeatureData_write_record (misc->bfd_handle, BFDATA_NEXT_RECORD, &bfd_record, NULL, NULL);


      //  Try to open the feature file and re-read the features into memory.

      binaryFeatureData_close_file (misc->bfd_handle);
      misc->bfd_open = NVFalse;


      if ((misc->bfd_handle = bfd_check_file (misc, misc->abe_share->open_args[0].target_path, &misc->bfd_header, BFDATA_UPDATE)) >= 0)
        {
          if (binaryFeatureData_read_all_short_features (misc->bfd_handle, &misc->feature) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (map, pfmView::tr ("pfmView"), pfmView::tr ("Unable to read feature records\nReason: ") + msg);
              binaryFeatureData_close_file (misc->bfd_handle);
            }
          else
            {
              misc->bfd_open = NVTrue;
            }
        }
    }
  else
    {
      return (NVFalse);
    }

  return (NVTrue);
}
