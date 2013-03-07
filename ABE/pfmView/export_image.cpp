
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

//!  This function exports the displayed image (as displayed) in .jpg, .png, GeoTIFF, .bmp, or .xpm format.

void export_image (QWidget *parent, MISC *misc, OPTIONS *options, nvMap *map)
{
  QStringList files, filters;
  QString file;
  static QString filt = "GeoTIFF (*.tif *.TIF)";


  if (!QDir (options->geotiff_dir).exists ()) options->geotiff_dir = options->input_pfm_dir;


  QFileDialog *fd = new QFileDialog (parent, pfmView::tr ("pfmView Export GeoTIFF File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->geotiff_dir);


  filters << "JPG (*.jpg *.jpeg *.JPG *.JPEG)"
	  << "PNG (*.png *.PNG)"
	  << "GeoTIFF (*.tif *.TIF)"
	  << "BMP (*.bmp *.BMP)"
	  << "XPM (*.xpm *.XPM)";

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::AnyFile);
  fd->selectFilter (filt);

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      NV_INT32 type = 0;

      if (!file.isEmpty())
	{
	  //  Get the display pixmap.

	  QPixmap screen = map->grabPixmap ();


	  NV_INT32 width = screen.width ();
	  NV_INT32 height = screen.height ();


	  QImage image = screen.toImage ();


	  if (file.endsWith (".jpg") || file.endsWith (".jpeg") || file.endsWith (".JPG") ||
	      file.endsWith (".JPEG")) type = 1;
	  if (file.endsWith (".png") || file.endsWith (".PNG")) type = 2;
	  if (file.endsWith (".tif") || file.endsWith (".TIF")) type = 3;
	  if (file.endsWith (".bmp") || file.endsWith (".BMP")) type = 4;
	  if (file.endsWith (".xpm") || file.endsWith (".XPM")) type = 5;

	  if (!type)
	    {
	      filt = fd->selectedFilter ();

	      if (filt == "JPG (*.jpg *.jpeg *.JPG *.JPEG)") type = 1;
	      if (filt == "PNG (*.png *.PNG)") type = 2;
	      if (filt == "GeoTIFF (*.tif *.TIF)") type = 3;
	      if (filt == "BMP (*.bmp *.BMP)") type = 4;
	      if (filt == "XPM (*.xpm *.XPM)") type = 1;
	    }


	  QString typeString = "";

	  switch (type)
	    {
	    case 1:
	      typeString = ".jpg";
	      break;

	    case 2:
	      typeString = ".png";
	      break;

	    case 4:
	      typeString = ".bmp";
	      break;

	    case 5:
	      typeString = ".xpm";
	      break;
	    }


	  switch (type)
	    {
	    case 1:
	    case 2:
	    case 4:
	    case 5:

	      //  Add extension to filename if not there.

	      if (!file.endsWith (typeString)) file.append (typeString);


	      screen.save (file, typeString.remove ('.').toAscii ());
	      break;


	    case 3:

	      //  Add .tif to filename if not there.

	      if (!file.endsWith (".tif")) file.append (".tif");


	      NV_CHAR fname[512];
	      strcpy (fname, file.toAscii ());



	      if (getenv ("GDAL_DATA") == NULL)
		{
		  QMessageBox::warning (parent, pfmView::tr ("pfmView Export GeoTIFF"),
					pfmView::tr ("Environment variable GDAL_DATA is not set!"));
		  return;
		}


	      OGRSpatialReference ref;
	      GDALDataset         *df;
	      char                *wkt = NULL;
	      GDALRasterBand      *bd[4];
	      NV_FLOAT64          trans[6];
	      GDALDriver          *gt;
	      NV_CHAR             **papszOptions = NULL;


	      //  Set up the output GeoTIFF file.

	      GDALAllRegister ();

	      gt = GetGDALDriverManager ()->GetDriverByName ("GTiff");
	      if (!gt)
		{
		  QMessageBox::warning (parent, pfmView::tr ("pfmView Export GeoTIFF"),
					pfmView::tr ("Could not get GTiff driver."));
		  return;
		}


	      //  Stupid Caris software can't read normal files!

	      papszOptions = CSLSetNameValue (papszOptions, "COMPRESS", "PACKBITS");

	      //papszOptions = CSLSetNameValue (papszOptions, "TILED", "NO");
	      //papszOptions = CSLSetNameValue (papszOptions, "COMPRESS", "LZW");


	      df = gt->Create (fname, width, height, 4, GDT_Byte, papszOptions);
	      if (df == NULL)
		{
		  QMessageBox::warning (parent, pfmView::tr ("pfmView Export GeoTIFF"),
					pfmView::tr ("Could not create GeoTIFF file ") + file);
		  return;
		}

	      trans[0] = misc->total_displayed_area.min_x;
	      trans[1] = (misc->total_displayed_area.max_x - misc->total_displayed_area.min_x) / (NV_FLOAT64) width;
	      trans[2] = 0.0;
	      trans[3] = misc->total_displayed_area.max_y;
	      trans[4] = 0.0;
	      trans[5] = -(misc->total_displayed_area.max_y - misc->total_displayed_area.min_y) / (NV_FLOAT64) height;
	      df->SetGeoTransform (trans);
	      ref.SetWellKnownGeogCS ("EPSG:4326");
	      ref.exportToWkt (&wkt);
	      df->SetProjection (wkt);
	      CPLFree (wkt);
	      for (NV_INT32 i = 0 ; i < 4 ; i++) bd[i] = df->GetRasterBand (i + 1);


	      NV_U_CHAR *red = (NV_U_CHAR *) calloc (width, sizeof (NV_U_CHAR));
	      NV_U_CHAR *green = (NV_U_CHAR *) calloc (width, sizeof (NV_U_CHAR));
	      NV_U_CHAR *blue = (NV_U_CHAR *) calloc (width, sizeof (NV_U_CHAR));
	      NV_U_CHAR *alpha = (NV_U_CHAR *) calloc (width, sizeof (NV_U_CHAR));


              misc->statusProg->setRange (0, height);
              misc->statusProgLabel->setText (pfmView::tr (" Exporting GeoTIFF "));
              misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
              misc->statusProgLabel->setPalette (misc->statusProgPalette);
              misc->statusProg->setTextVisible (TRUE);
              qApp->processEvents();


	      for (NV_INT32 i = 0 ; i < height ; i++)
		{
		  for (NV_INT32 j = 0 ; j < width ; j++)
		    {
		      QColor pixel = QColor (image.pixel (j, i));

		      if (pixel == options->background_color)
			{
			  red[j] = green[j] = blue[j] = alpha[j] = 0;
			}
		      else
			{
			  red[j] = pixel.red ();
			  green[j] = pixel.green ();
			  blue[j] = pixel.blue ();
			  alpha[j] = 255;
			}
		    }

		  CPLErr err = bd[0]->RasterIO (GF_Write, 0, i, width, 1, red, width, 1, GDT_Byte, 0, 0);
		  err = bd[1]->RasterIO (GF_Write, 0, i, width, 1, green, width, 1, GDT_Byte, 0, 0);
		  err = bd[2]->RasterIO (GF_Write, 0, i, width, 1, blue, width, 1, GDT_Byte, 0, 0);
		  err = bd[3]->RasterIO (GF_Write, 0, i, width, 1, alpha, width, 1, GDT_Byte, 0, 0);

		  if (err == CE_Failure)
		    {
		      QMessageBox::warning (parent, pfmView::tr ("pfmView Export GeoTIFF"),
					    pfmView::tr ("Failed TIFF scanline write."));

		      free (red);
		      free (green);
		      free (blue);
		      free (alpha);

		      delete df;

		      return;
		    }


		  misc->statusProg->setValue (i);
		  qApp->processEvents ();
		}

	      free (red);
	      free (green);
	      free (blue);
	      free (alpha);

              misc->statusProg->reset ();
              misc->statusProg->setTextVisible (FALSE);
              qApp->processEvents();

	      delete df;

	      break;
	    }

          options->geotiff_dir = fd->directory ().absolutePath ();
	}
    }
}
