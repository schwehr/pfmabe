
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
#include "version.hpp"


//!  Converts an old MIW feature file to the new BFD format.

NV_INT32 miw2bfd (NV_CHAR *path, BFDATA_HEADER *header, MISC *misc)
{
  BFDATA_RECORD        bfd_record;
  BFDATA_POLYGON       bfd_polygon;
  NV_CHAR              bfd_file[512], image_file[512], string[512], *curElem, contact_id[15];
  NV_INT32             i, j, bfd_handle, target_count = 0, count = 0, year, month, mday, jday, hour, minute, second, bfd_error;
  NV_INT16             tmpi16;
  SHORT_TARGET         *target = NULL;
  CNCTDocumentHandle   docHandle;
  REPRESENTATION       rep;


  /*  Make sure we can find the schema.  This will replace it in the file if the one that's there doesn't exist.  */

  check_target_schema (path);


  if ((docHandle = CNCTEditDoc (path, 0)) <= 0)
    {
      QString msg = QString (strerror (errno));
      QMessageBox::warning (0, pfmView::tr ("pfmView"), pfmView::tr ("Unable to open XML file\n%1\nReason: ").arg (path) + msg);
      return (-999);
    }


  strcpy (bfd_file, path);
  sprintf (&bfd_file[strlen (bfd_file) - 4], ".bfd");


  memset (header, 0, sizeof (BFDATA_HEADER));


  strcpy (header->creation_software, VERSION);

  if ((bfd_handle = binaryFeatureData_create_file (bfd_file, *header)) < 0) return (bfd_handle);


  target_count = ReadAllShortTargets (docHandle, &target);


  misc->statusProg->setRange (0, target_count);
  misc->statusProgLabel->setText (pfmView::tr (" Converting MIW to BFD "));
  misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
  misc->statusProgLabel->setPalette (misc->statusProgPalette);
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  MoveRoot (docHandle);

  i = 0;

  if (MoveFirstChild (docHandle) == 1)
    {
      do
        {
          memset (&bfd_record, 0, sizeof (BFDATA_RECORD));

          GetAttribute (docHandle, "contactId", bfd_record.contact_id);

          MoveFirstChild (docHandle);
          
          if (GetAttribute (docHandle, "eventDTG", string) > 0)
            {
              sscanf (string, "%04d-%02d-%02dT%02d:%02d:%02d", &year, &month, &mday, &hour, &minute, &second);
              mday2jday (year, month - 1, mday, &jday);

              inv_cvtime (year - 1900, jday, hour, minute, (NV_FLOAT32) second, &bfd_record.event_tv_sec, &bfd_record.event_tv_nsec);
            }

          GetAttribute (docHandle, "latitude", string);
          sscanf (string, "%lf", &bfd_record.latitude);

          GetAttribute (docHandle, "longitude", string);
          sscanf (string, "%lf", &bfd_record.longitude);

          if (GetAttribute (docHandle, "length", string) > 0) sscanf (string, "%f", &bfd_record.length);

          if (GetAttribute (docHandle, "width", string) > 0) sscanf (string, "%f", &bfd_record.width);

          if (GetAttribute (docHandle, "height", string) > 0) sscanf (string, "%f", &bfd_record.height);

          if (GetAttribute (docHandle, "depth", string) > 0) sscanf (string, "%f", &bfd_record.depth);

          if (GetAttribute (docHandle, "horizontalOrientation", string) > 0) sscanf (string, "%f", &bfd_record.horizontal_orientation);

          if (GetAttribute (docHandle, "verticalOrientation", string) > 0) sscanf (string, "%f", &bfd_record.vertical_orientation);

          if (GetAttribute (docHandle, "description", string) > 0) strcpy (bfd_record.description, string);

          if (GetAttribute (docHandle, "remarks", string) > 0) strcpy (bfd_record.remarks, string);

          if (GetAttribute (docHandle, "confidenceLevel", string) > 0)
            {
              sscanf (string, "%hd", &tmpi16);
              bfd_record.confidence_level = (NV_U_BYTE) tmpi16;
            }

          if (GetAttribute (docHandle, "analystActivity", string) > 0) strcpy (bfd_record.analyst_activity, string);

          GetAttribute (docHandle, "remarks", string);
          strcpy (bfd_record.remarks, string);

          GetAttribute (docHandle, "depth", string);
          sscanf (string, "%f", &bfd_record.depth);


          if (GetRepresentation (docHandle, &rep) == 1)
            {
              bfd_record.poly_count = rep.numPoints;
              bfd_record.poly_type = rep.type;

              for (j = 0 ; j < rep.numPoints ; j++)
                {
                  bfd_polygon.latitude[j] = rep.lat[j];
                  bfd_polygon.longitude[j] = rep.lon[j];
                }
            }


          MoveFirstChild (docHandle);

          strcpy (image_file, "");

          while ((curElem = GetCurrentElementType (docHandle)) != NULL)
            {
              if (!strcmp (curElem, "Sonar"))
                {
                  if (GetAttribute (docHandle, "type", string) > 0)
                    {
                      sscanf (string, "%hd", &tmpi16);
                      bfd_record.sonar_type = (NV_U_BYTE) tmpi16;
                    }
                }

              if (!strcmp (curElem, "Platform"))
                {
                  if (GetAttribute (docHandle, "equipType", string) > 0)
                    {
                      sscanf (string, "%hd", &tmpi16);
                      bfd_record.equip_type = (NV_U_BYTE) tmpi16;
                    }
                  if (GetAttribute (docHandle, "platformType", string) > 0)
                    {
                      sscanf (string, "%hd", &tmpi16);
                      bfd_record.platform_type = (NV_U_BYTE) tmpi16;
                    }
                  if (GetAttribute (docHandle, "heading", string) > 0) sscanf (string, "%f", &bfd_record.heading);
                }

              if (!strcmp (curElem, "Image"))
                {
                  if (GetAttribute (docHandle, "snippetFileName", string) > 0) strcpy (image_file, string);
                }

              if (MoveNextSibling (docHandle) == -1) break;
            }


          if ((bfd_error = binaryFeatureData_write_record_image_file (bfd_handle, BFDATA_NEXT_RECORD, &bfd_record, &bfd_polygon, image_file)) < 0)
            return (bfd_error);

          i++;
          count++;


          //  See if there are children.

          MoveUp (docHandle);
          strcpy (contact_id, bfd_record.contact_id);

          while (MoveNextSibling (docHandle) != -1)
            {
              //  Save the child record number.

              bfd_record.child_record = count + 1;
              if (binaryFeatureData_write_record (bfd_handle, count - 1, &bfd_record, NULL, NULL) < 0)
                {
                  binaryFeatureData_perror ();
                  exit (-1);
                }


              memset (&bfd_record, 0, sizeof (BFDATA_RECORD));

              strcpy (bfd_record.contact_id, contact_id);

              if (GetAttribute (docHandle, "eventDTG", string) > 0)
                {
                  sscanf (string, "%04d-%02d-%02dT%02d:%02d:%02d", &year, &month, &mday, &hour, &minute, &second);
                  mday2jday (year, month - 1, mday, &jday);

                  inv_cvtime (year - 1900, jday, hour, minute, (NV_FLOAT32) second, &bfd_record.event_tv_sec, &bfd_record.event_tv_nsec);
                }


              GetAttribute (docHandle, "latitude", string);
              sscanf (string, "%lf", &bfd_record.latitude);

              GetAttribute (docHandle, "longitude", string);
              sscanf (string, "%lf", &bfd_record.longitude);

              if (GetAttribute (docHandle, "length", string) > 0) sscanf (string, "%f", &bfd_record.length);

              if (GetAttribute (docHandle, "width", string) > 0) sscanf (string, "%f", &bfd_record.width);

              if (GetAttribute (docHandle, "height", string) > 0) sscanf (string, "%f", &bfd_record.height);

              if (GetAttribute (docHandle, "depth", string) > 0) sscanf (string, "%f", &bfd_record.depth);

              if (GetAttribute (docHandle, "horizontalOrientation", string) > 0) sscanf (string, "%f", &bfd_record.horizontal_orientation);

              if (GetAttribute (docHandle, "verticalOrientation", string) > 0) sscanf (string, "%f", &bfd_record.vertical_orientation);

              if (GetAttribute (docHandle, "description", string) > 0) strcpy (bfd_record.description, string);

              if (GetAttribute (docHandle, "remarks", string) > 0) strcpy (bfd_record.remarks, string);

              if (GetAttribute (docHandle, "confidenceLevel", string) > 0)
                {
                  sscanf (string, "%hd", &tmpi16);
                  bfd_record.confidence_level = (NV_U_BYTE) tmpi16;
                }

              if (GetAttribute (docHandle, "analystActivity", string) > 0) strcpy (bfd_record.analyst_activity, string);

              GetAttribute (docHandle, "remarks", string);
              strcpy (bfd_record.remarks, string);

              GetAttribute (docHandle, "depth", string);
              sscanf (string, "%f", &bfd_record.depth);


              if (GetRepresentation (docHandle, &rep) == 1)
                {
                  bfd_record.poly_count = rep.numPoints;
                  bfd_record.poly_type = rep.type;

                  for (j = 0 ; j < rep.numPoints ; j++)
                    {
                      bfd_polygon.latitude[j] = rep.lat[j];
                      bfd_polygon.longitude[j] = rep.lon[j];
                    }
                }


              MoveFirstChild (docHandle);

              strcpy (image_file, "");

              while ((curElem = GetCurrentElementType (docHandle)) != NULL)
                {
                  if (!strcmp (curElem, "Sonar"))
                    {
                      if (GetAttribute (docHandle, "type", string) > 0)
                        {
                          sscanf (string, "%hd", &tmpi16);
                          bfd_record.sonar_type = (NV_U_BYTE) tmpi16;
                        }
                    }

                  if (!strcmp (curElem, "Platform"))
                    {
                      if (GetAttribute (docHandle, "equipType", string) > 0)
                        {
                          sscanf (string, "%hd", &tmpi16);
                          bfd_record.equip_type = (NV_U_BYTE) tmpi16;
                        }
                      if (GetAttribute (docHandle, "platformType", string) > 0)
                        {
                          sscanf (string, "%hd", &tmpi16);
                          bfd_record.platform_type = (NV_U_BYTE) tmpi16;
                        }
                      if (GetAttribute (docHandle, "heading", string) > 0) sscanf (string, "%f", &bfd_record.heading);
                    }

                  if (!strcmp (curElem, "Image"))
                    {
                      if (GetAttribute (docHandle, "snippetFileName", string) > 0) strcpy (image_file, string);
                    }

                  if (MoveNextSibling (docHandle) == -1) break;
                }


              bfd_record.parent_record = count;

              if (binaryFeatureData_write_record_image_file (bfd_handle, BFDATA_NEXT_RECORD, &bfd_record, &bfd_polygon, image_file) < 0)
                {
                  binaryFeatureData_perror ();
                  exit (-1);
                }

              MoveUp (docHandle);

              count++;
            }

          misc->statusProg->setValue (i);
          qApp->processEvents ();

          MoveUp (docHandle);
        } while (MoveNextSibling (docHandle) == 1);
    }


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents();


  CNCTCloseDoc (docHandle, 0);


  TerminateXML ();


  //  Flush the header by closing and opening the file.

  binaryFeatureData_close_file (bfd_handle);
  bfd_handle = binaryFeatureData_open_file (bfd_file, header, BFDATA_UPDATE);


  strcpy (path, bfd_file);


  return (bfd_handle);
}
