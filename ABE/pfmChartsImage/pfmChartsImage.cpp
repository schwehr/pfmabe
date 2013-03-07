#include "pfmChartsImage.hpp"
#include "pfmChartsImageHelp.hpp"


NV_FLOAT64       settings_version = 1.02;

QListWidget      *wCheckList;


static void misp_progress_callback (NV_CHAR *info)
{
  if (strlen (info) >= 2)
    {
      QListWidgetItem *cur = new QListWidgetItem (QString (info));
      wCheckList->addItem (cur);
      wCheckList->setCurrentItem (cur);
      wCheckList->scrollToItem (cur);
    }

  qApp->processEvents ();
}



pfmChartsImage::pfmChartsImage (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  restricted = NVFalse;
  writer_stat = NVFalse;
  run_counter = 0;


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmChartsImageWatermark.png"));


  //  Get the user's defaults if available

  envin (&options);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);


  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  area_file_name = "";


  //  If we don't have SRTM 1, 2, or 3 there's really no point in asking.

  options.srtm_available = NVFalse;
  if (check_srtm1_topo () || check_srtm2_topo () || check_srtm3_topo ()) options.srtm_available = NVTrue;


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  //  This is so we can drag and drop files on the Desktop icon.

  open_args.list_path[0] = 0;
  if (*argc == 2) strcpy (open_args.list_path, argv[1]);


  setPage (0, new startPage (this, &open_args, &options));

  setPage (1, new imagePage (this, &options));

  setPage (2, new mosaicPage (this, &options));

  setPage (3, new extractPage (this, &progress, &eCheckList));

  setPage (4, iPage = new inputPage (this));

  setPage (5, rPage = new runPage (this, &options, &progress, &wCheckList));


  setButtonText (QWizard::CustomButton1, tr ("&Extract"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start the extraction process"));
  button (QWizard::CustomButton1)->setWhatsThis (extractText);

  setButtonText (QWizard::CustomButton2, tr ("&Mosaic"));
  setOption (QWizard::HaveCustomButton2, TRUE);
  button (QWizard::CustomButton2)->setToolTip (tr ("Start the mosaic process"));
  button (QWizard::CustomButton2)->setWhatsThis (mosaicText);

  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}


pfmChartsImage::~pfmChartsImage ()
{
}



void pfmChartsImage::initializePage (int id)
{
  QString string;
  QRect tmp;
  NV_INT32 lmt;


  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);
  button (QWizard::CustomButton2)->setEnabled (FALSE);


  switch (id)
    {
    case 0:
      break;

    case 1:
      pfm_file_name = field ("pfm_file_edit").toString ();
      area_file_name = field ("area_file_edit").toString ();
      level_file_name = field ("level_file_edit").toString ();
      options.level = field ("levelSpin").toDouble ();
      image_dir_name = field ("image_dir_edit").toString ();


      //  Save the image output directory.  It might have been modified by the user.

      options.output_dir = QFileInfo (image_dir_name).absoluteDir ().absolutePath ();


      strcpy (areafile, area_file_name.toAscii ());

      get_area_mbr (areafile, &polygon_count, polygon_x, polygon_y, &options.mbr);

      break;

    case 2:
      break;

    case 3:
      button (QWizard::CustomButton1)->setEnabled (TRUE);


      options.casi = field ("casi_check").toBool ();
      options.datum_offset = field ("datumSpin").toDouble ();
      options.time_offset = field ("timeSpin").toDouble ();
      options.grid_size = field ("demSpin").toDouble ();
      options.interval = field ("intervalSpin").toInt ();

      if (options.low_limit > options.high_limit) options.high_limit += 360;

      options.cell_size = field ("cellSpin").toDouble ();
      options.roll_bias = field ("rollSpin").toDouble ();
      options.pitch_bias = field ("pitchSpin").toDouble ();
      options.heading_bias = field ("headingSpin").toDouble ();
      options.focal_length = field ("focalSpin").toDouble ();
      options.pixel_size = field ("pixelSpin").toDouble ();
      options.column_offset = field ("colSpin").toDouble ();
      options.row_offset = field ("rowSpin").toDouble ();


      //  Use frame geometry to get the absolute x and y.

      tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      //  Save options.

      envout (&options);


      eCheckList->clear ();

      string = tr ("Input PFM file : ") + pfm_file_name;
      eCheckList->addItem (string);

      string = tr ("Area file : ") + area_file_name;
      eCheckList->addItem (string);

      if (!level_file_name.isEmpty ())
        {
          string = tr ("Level area file : ") + level_file_name;
          eCheckList->addItem (string);
          string.sprintf (tr ("Level : %.2f").toAscii (), options.level);
          eCheckList->addItem (string);
        }


      if (options.casi)
        {
          string = tr ("Output CASI format DEM file");
          eCheckList->addItem (string);
        }

      if (options.exclude)
        {
          string = tr ("Too dark or too bright images are excluded");
          eCheckList->addItem (string);
        }

      if (options.opposite)
        {
          string = tr ("Lines in direction opposite the limits are included");
          eCheckList->addItem (string);
        }

      if (options.srtm_available && options.srtm)
        {
          if (check_srtm2_topo ()) restricted = NVTrue;

          string = tr ("Adding SRTM data to empty land areas of DEM");
          eCheckList->addItem (string);
        }

      if (options.normalize)
        {
          string = tr ("Images will be normalized");
          eCheckList->addItem (string);
        }

      if (options.flip)
        {
          string = tr ("Z value signs are inverted");
          eCheckList->addItem (string);
        }

      if (options.pos)
        {
          string = tr (".pos navigation files will not be used");
          eCheckList->addItem (string);
        }


      string = QString (tr ("Roll bias : %1")).arg (options.roll_bias, 7, 'f', 3);
      eCheckList->addItem (string);


      string = QString (tr ("Pitch bias : %1")).arg (options.pitch_bias, 7, 'f', 3);
      eCheckList->addItem (string);


      string = QString (tr ("Heading bias : %1")).arg (options.heading_bias, 8, 'f', 3);
      eCheckList->addItem (string);


      string = QString (tr ("Datum offset (m) : %1")).arg (options.datum_offset, 8, 'f', 2);
      eCheckList->addItem (string);


      string = QString (tr ("Time offset (s) : %1")).arg (options.time_offset, 3, 'f', 1);
      eCheckList->addItem (string);


      string = QString (tr ("DEM grid size (m) : %1")).arg (options.grid_size, 3, 'f', 1);
      eCheckList->addItem (string);


      string = QString (tr ("Image interval : %1")).arg (options.interval);
      eCheckList->addItem (string);


      string = QString (tr ("Lower line direction limit : %1")).arg (options.low_limit);
      eCheckList->addItem (string);


      lmt = options.high_limit;
      if (lmt > 360) lmt -= 360;
      string = QString (tr ("Upper line direction limit : %1")).arg (lmt);
      eCheckList->addItem (string);


      string = QString (tr ("Output cell size (m) : %1")).arg (options.cell_size, 3, 'f', 1);
      eCheckList->addItem (string);


      string = QString (tr ("Camera focal length (mm) : %1")).arg (options.focal_length, 6, 'f', 3);
      eCheckList->addItem (string);


      string = QString (tr ("Camera pixel size (u) : %1")).arg (options.pixel_size, 5, 'f', 2);
      eCheckList->addItem (string);


      string = QString (tr ("Camera column offset (u) : %1")).arg (options.column_offset, 6, 'f', 2);
      eCheckList->addItem (string);


      string = QString (tr ("Camera row offset (u) : %1\n\n")).arg (options.row_offset, 6, 'f', 2);
      eCheckList->addItem (string);
      break;

    case 4:
      //button (QWizard::BackButton)->setEnabled (FALSE);
      button (QWizard::CustomButton1)->setEnabled (FALSE);

      if (!hasVisitedPage (id)) iPage->initializeTable (icount, image_dir_name, QString (txt_file));

      break;

    case 5:
      button (QWizard::CustomButton2)->setEnabled (TRUE);

      if (!run_counter)
        {
          if (options.utm)
            {
              out_file_name = pfm_file_name + "__" + QFileInfo (area_file_name).fileName () + "_utm.tif";
            }
          else
            {
              out_file_name = pfm_file_name + "__" + QFileInfo (area_file_name).fileName () + ".tif";
            }
        }


      //  If this is the first time or you have accepted the default output file name on previous passes I will
      //  increment a counter in the name.  If you modified the name yourself, you're on your own.

      string.setNum (run_counter - 1);
      QString tmp;
      tmp = "_" + string + ".tif";

      if (!run_counter || out_file_name.contains (tmp))
        {
          string.setNum (run_counter);
          if (!run_counter)
            {
              out_file_name.replace (".tif", "_" + string + ".tif");
            }
          else
            {
              out_file_name.replace (tmp, "_" + string + ".tif");
            }
        }


      rPage->setOutFileName (out_file_name);

      wCheckList->clear ();

      break;
    }
}



void pfmChartsImage::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;

    case 3:
      break;

    case 4:
      break;

    case 5:
      break;
    }
}



bool pfmChartsImage::validateCurrentPage ()
{
  void remove_dir (QString dir);


  if (currentPage () == rPage)
    {
      //  Remove the image directory if we requested it.

      if (options.remove) remove_dir (image_dir_name);
    }

  return (TRUE);
}



void pfmChartsImage::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
pfmChartsImage::slotCustomButtonClicked (int id)
{
  FILE                *txt_fp, *pars_fp;
  static NV_CHAR      dem_file[512], casi_file[512];
  NV_CHAR             pfm_file[512], areafile[512], levelfile[512], path[512];
  NV_INT32            i, level_count = 0, x_start, y_start, width,
                      height, total;
  NV_INT16            type;
  LIST_NUM            list[10000];
  NV_FLOAT64          level_x[200], level_y[200];
  NV_F64_XYMBR        level_mbr;


  QApplication::setOverrideCursor (Qt::WaitCursor);


  if (id == 6)
    {
      button (QWizard::NextButton)->setEnabled (FALSE);
      button (QWizard::BackButton)->setEnabled (FALSE);
      button (QWizard::CustomButton1)->setEnabled (FALSE);


      //  Make the image directory.

      strcpy (dir, image_dir_name.toAscii ());

      QDir topDir = QDir (image_dir_name);

      if (!topDir.exists ())
        {
          if (!topDir.mkpath (image_dir_name))
            {
              perror (image_dir_name.toAscii ());
              exit (-1);
            }
        }


      //  Set all hits to false.  

      for (i = 0 ; i < 10000 ; i++)
        {
          list[i].hit = NVFalse;
          list[i].start = NV_U_INT32_MAX;
          list[i].end = 0;
        }


      strcpy (pfm_file, pfm_file_name.toAscii ());
      strcpy (areafile, area_file_name.toAscii ());


      //  We're doing this in UTM for now because MOSAIC wants it.  Later we should be able to change back.

      NV_FLOAT64 cen_x = options.mbr.min_x + (options.mbr.max_x - options.mbr.min_x) / 2.0;

      NV_CHAR string[60];
      if (options.mbr.max_y < 0.0)
        {
          sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +south +lon_0=%.9f", cen_x);
        }
      else
        {
          sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +lon_0=%.9f", cen_x);
        }

      if (!(pj_utm = pj_init_plus (string)))
        {
          QMessageBox::critical (this, tr ("pfmChartsImage"), tr ("Error initializing UTM projection\n"));
          exit (-1);
        }

      if (!(pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
        {
          QMessageBox::critical (this, tr ("pfmChartsImage"), tr ("Error initializing latlon projection\n"));
          exit (-1);
        }

      zone = (NV_INT32) (31.0 + cen_x / 6.0);
      if (zone >= 61) zone = 60;	
      if (zone <= 0) zone = 1;


      QListWidgetItem *z = new QListWidgetItem (QString (tr ("UTM Zone %1")).arg (zone));
      eCheckList->addItem (z);

      min_ne_x = options.mbr.min_x * NV_DEG_TO_RAD;
      min_ne_y = options.mbr.min_y * NV_DEG_TO_RAD;
      pj_transform (pj_latlon, pj_utm, 1, 1, &min_ne_x, &min_ne_y, NULL);

      QListWidgetItem *min_ne = new QListWidgetItem (QString (tr ("Min northing and easting : %1 %2"))
                                                     .arg (min_ne_y, 12, 'f', 2)
                                                     .arg (min_ne_x, 12, 'f', 2));
      eCheckList->addItem (min_ne);

      max_ne_x = options.mbr.max_x * NV_DEG_TO_RAD;
      max_ne_y = options.mbr.max_y * NV_DEG_TO_RAD;
      pj_transform (pj_latlon, pj_utm, 1, 1, &max_ne_x, &max_ne_y, NULL);

      QListWidgetItem *max_ne = new QListWidgetItem (QString (tr ("Max northing and easting : %1 %2"))
                                                     .arg (max_ne_y, 12, 'f', 2)
                                                     .arg (max_ne_x, 12, 'f', 2));
      eCheckList->addItem (max_ne);
      eCheckList->setCurrentItem (max_ne);
      eCheckList->scrollToItem (max_ne);


      if (!level_file_name.isEmpty ())
        {
          strcpy (levelfile, level_file_name.toAscii ());
          get_area_mbr (levelfile, &level_count, level_x, level_y, &level_mbr);
        }


      x_start = 0;
      y_start = 0;
      width = open_args.head.bin_width;
      height = open_args.head.bin_height;

      if (options.mbr.min_y > open_args.head.mbr.max_y || options.mbr.max_y < open_args.head.mbr.min_y || 
          options.mbr.min_x > open_args.head.mbr.max_x || options.mbr.max_x < open_args.head.mbr.min_x)
        {
          QMessageBox::critical (this, tr ("pfmChartsImage"), tr ("Specified area is completely outside of the PFM bounds!\n"));
          return;
        }


      //  Match to nearest cell.  

      x_start = NINT ((options.mbr.min_x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees);
      y_start = NINT ((options.mbr.min_y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees);
      width = NINT ((options.mbr.max_x - options.mbr.min_x) / open_args.head.x_bin_size_degrees);
      height = NINT ((options.mbr.max_y - options.mbr.min_y) / open_args.head.y_bin_size_degrees);


      //  Adjust to PFM bounds if necessary.  

      if (x_start < 0) x_start = 0;
      if (y_start < 0) y_start = 0;
      if (x_start + width > open_args.head.bin_width) width = open_args.head.bin_width - x_start;
      if (y_start + height > open_args.head.bin_height) height = open_args.head.bin_height - y_start;


      //  Create the dem tif file.  

      if (restricted)
        {
          sprintf (dem_file, "%s%1c%s_DOD_restricted_if_outside_US__dem.tif", dir, (NV_CHAR) SEPARATOR, pfm_basename (areafile));
          sprintf (casi_file, "%s%1c%s_DOD_restricted_if_outside_US__dem.xyz", dir, (NV_CHAR) SEPARATOR, pfm_basename (areafile));
        }
      else
        {
          sprintf (dem_file, "%s%1c%s_dem.tif", dir, (NV_CHAR) SEPARATOR, pfm_basename (areafile));
          sprintf (casi_file, "%s%1c%s_dem.xyz", dir, (NV_CHAR) SEPARATOR, pfm_basename (areafile));
        }

      if (misp (options.grid_size, dem_file, casi_file, x_start, y_start, width, height, level_x, level_y, level_count, options.level,
                list, options.datum_offset, options.flip, options.casi))
        {
          QMessageBox::critical (this, tr ("pfmChartsImage"), tr ("Error, no input points found running misp_proc!\nUnable to create DEM!\n"));
          return;
        }


      if (restricted)
        {
          if (check_srtm2_restricted_data_read ())
            {
              sprintf (path, "%s%1c%s_DOD_restricted__dem.tif", dir, (NV_CHAR) SEPARATOR, pfm_basename (areafile));
            }
          else
            {
              sprintf (path, "%s%1c%s_dem.tif", dir, (NV_CHAR) SEPARATOR, pfm_basename (areafile));
            }

          rename (dem_file, path);
          strcpy (dem_file, path);
        }


      total = 0;
      for (i = 0 ; i < 10000 ; i++)
        {
          if (list[i].hit) total += ((list[i].end - list[i].start) + 1);
        }


      //  Open the pos.dat file.  

      sprintf (txt_file, "%s%1c%s_pos.dat", dir, (NV_CHAR) SEPARATOR, pfm_basename (areafile));
      if ((txt_fp = fopen (txt_file, "w")) == NULL) 
        {
          perror (txt_file);
          exit (-1);
        }


      //  Extract the images.

      progress.ebar->setRange (0, total);

      icount = 0;
      for (i = 0 ; i < 10000 ; i++)
        {
          if (list[i].hit)
            {
              read_list_file (options.pfm_handle, (NV_INT16) i, path, &type);

              icount += get_images (path, type, dir, polygon_x, polygon_y, polygon_count, list[i].start, list[i].end, 
                                    txt_fp, areafile);
            }
        }

      progress.ebar->setValue (total);

      qApp->processEvents ();


      if (!icount)
        {
          QMessageBox::critical (this, tr ("pfmChartsImage Extract images"),
                                 tr ("No images were returned.  Please check your mosaic/DEM parameters.\nPress Cancel to exit."));

          return;
        }



      QListWidgetItem *cur = new QListWidgetItem (QString (tr ("Extracted %1 images\n\n\n")).arg (icount));
      eCheckList->addItem (cur);
      eCheckList->setCurrentItem (cur);
      eCheckList->scrollToItem (cur);

      close_pfm_file (options.pfm_handle);
      fclose (txt_fp);



      button (QWizard::NextButton)->setEnabled (TRUE);


      QApplication::restoreOverrideCursor ();

      qApp->processEvents ();
    }
  else
    {
      //  Generate the mosaic.

      QFileInfo *pfi = new QFileInfo (pfm_file_name);
      QString name_string;
      name_string.setNum (getpid ());
      tmp_file_name = pfi->absolutePath () + "/temporary_mosaic_file_" + name_string + ".tif";
      NV_CHAR pars_dir[512], pars_name[512], area_name[512];
      QFileInfo *afi = new QFileInfo (area_file_name);
      strcpy (pars_dir, pfi->absolutePath ().toAscii ());
      strcpy (pars_name, pfi->baseName ().toAscii ());
      strcpy (area_name, afi->baseName ().toAscii ());
      sprintf (pars_file, "%s%1c%s__%s.pars", pars_dir, (NV_CHAR) SEPARATOR, pars_name, area_name);

      if ((pars_fp = fopen (pars_file, "w")) == NULL)
        {
          perror (pars_file);
          exit (-1);
        }


      //  Fill the mosaic parameter file.

      NV_CHAR tmp_file[512];
      sprintf (tmp_file, "%s__tmp", txt_file);


      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "images_file:         %s\n", tmp_file);
      fprintf (pars_fp, "dem_file:            %s\n", dem_file);
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "readers:             20\n");
      fprintf (pars_fp, "mappers:             8\n");
      fprintf (pars_fp, "writers:             1\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "block_x:             512\n");
      fprintf (pars_fp, "block_y:             512\n");
      fprintf (pars_fp, "slack:               50\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   easting_left, easting_right, northing_top and northern_bottom define\n");
      fprintf (pars_fp, "#   the geographic region to process for UTM output\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "easting_left:        %.3f\n", min_ne_x);
      fprintf (pars_fp, "easting_right:       %.3f\n", max_ne_x);
      fprintf (pars_fp, "northing_top:        %.3f\n", max_ne_y);
      fprintf (pars_fp, "northing_bottom:     %.3f\n", min_ne_y);
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   longitude_left, longitude_right, latitude_bottom, and latitude_top define\n");
      fprintf (pars_fp, "#   the geographic region to process for geographic output\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "longitude_left:      %.3f\n", options.mbr.min_x);
      fprintf (pars_fp, "longitude_right:     %.3f\n", options.mbr.max_x);
      fprintf (pars_fp, "latitude_top:        %.3f\n", options.mbr.max_y);
      fprintf (pars_fp, "latitude_bottom:     %.3f\n", options.mbr.min_y);
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   utm_zone defines which zone is in use\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "utm_zone:            %d\n", zone);
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   output_cell_size defines the output cell size\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   output_projection is either utm for UTM projection or blank for geographic\n");
      fprintf (pars_fp, "#\n");

      if (options.utm)
        {
          fprintf (pars_fp, "output_projection:            utm\n");
        }
      else
        {
          fprintf (pars_fp, "output_projection:               \n");
        }
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "output_cell_size:    %.2f\n", options.cell_size);
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   input_cell_size defines the input cell size\n");
      fprintf (pars_fp, "#   input_rows and input_columns define the imput image size.\n");
      fprintf (pars_fp, "#   These are used in the approximation of the maximum distance.\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "input_cell_size:     0.2\n");
      fprintf (pars_fp, "input_rows:          1200\n");
      fprintf (pars_fp, "input_columns:       1600\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   Sea level needs to be defined\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "sea_level:           0.0\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   focal_length is the focal length of the camera in millimeters\n");
      fprintf (pars_fp, "#   pixel_size is the camera's pixel size in microns\n");
      fprintf (pars_fp, "#   camera_roll, camera_pitch, and camera_heading are adjustments to these\n");
      fprintf (pars_fp, "#   factors as set in the image position file and should normally be left at 0.0\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "focal_length:        %.3f\n", options.focal_length);
      fprintf (pars_fp, "pixel_size:          %.2f\n", options.pixel_size);
      fprintf (pars_fp, "column_offset:       %.2f\n", options.column_offset);
      fprintf (pars_fp, "row_offset:          %.2f\n", options.row_offset);
      fprintf (pars_fp, "camera_roll:         0.0\n");
      fprintf (pars_fp, "camera_pitch:        0.0\n");
      fprintf (pars_fp, "camera_yaw:          0.0\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   The output will be a GeoTiff file\n");
      fprintf (pars_fp, "#\n");
      NV_CHAR out_name[512];
      strcpy (out_name, tmp_file_name.toAscii ());
      fprintf (pars_fp, "output_file:         %s\n", out_name);
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "#   Overviews will be defined by nearest neighbor sampling\n");
      fprintf (pars_fp, "#   0 means no overview; 1 means sampled at 2; 2 means sampled at 2 and 4;\n");
      fprintf (pars_fp, "#   3 means sampled at 2, 4 and 8.\n");
      fprintf (pars_fp, "#   The maximum number of overview levels is set to 5.\n");
      fprintf (pars_fp, "#\n");
      fprintf (pars_fp, "overviews:           0\n");

      fclose (pars_fp);


      run_counter++;

      button (QWizard::FinishButton)->setEnabled (FALSE);
      button (QWizard::CustomButton2)->setEnabled (FALSE);


      out_file_name = field ("out_file_edit").toString ();

      if (!out_file_name.endsWith (".tif")) out_file_name.append (".tif");

      if (options.utm) out_file_name.replace (".tif", "_utm.tif");


      //  Only deliver the selected images via the images file.

      FILE *ifp, *ofp;

      if ((ifp = fopen (txt_file, "r")) == NULL)
        {
          perror (txt_file);
          exit (-1);
        }

      NV_CHAR string[1024];
      sprintf (tmp_file, "%s__tmp", txt_file);

      if ((ofp = fopen (tmp_file, "w")) == NULL)
        {
          perror (tmp_file);
          exit (-1);
        }

      for (NV_INT32 i = 0 ; i < icount ; i++)
        {
          fgets (string, sizeof (string), ifp);

          if (!iPage->getCheckBox (i)) fprintf (ofp, "%s", string);
        }

      fclose (ifp);
      fclose (ofp);


      QListWidgetItem *cur2 = new QListWidgetItem (tr ("Running MOSAIC program\n\n"));
      wCheckList->addItem (cur2);
      wCheckList->setCurrentItem (cur2);
      wCheckList->scrollToItem (cur2);


      progress.mbar->setRange (0, 0);
      qApp->processEvents ();


      QStringList arguments;

      mosaicProc = new QProcess (this);

      arguments += pars_file;

      connect (mosaicProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
               SLOT (slotMosaicDone (int, QProcess::ExitStatus)));
      connect (mosaicProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotMosaicReadyReadStandardError ()));
      connect (mosaicProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotMosaicReadyReadStandardOutput ()));
      connect (mosaicProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotMosaicError (QProcess::ProcessError)));

      mosaicProc->start ("mosaic", arguments);
    }
}



NV_INT32 pfmChartsImage::get_images (NV_CHAR *path, NV_INT32 type, char *img_dir, NV_FLOAT64 *polygon_x, NV_FLOAT64 *polygon_y,
                                     NV_INT32 polygon_count, NV_INT32 start, NV_INT32 end, 
                                     FILE *txt_fp, NV_CHAR *areafile)
{
  FILE                   *data_fp, *jpg_fp, *img_fp;
  POS_OUTPUT_T           pos;
  NV_INT64               new_stamp, image_time, data_timestamp, offset;
  NV_INT32               i, img_rec_num, gicount, interval_count;
  static NV_INT32        prev_img_rec_num = -1;
  NV_U_INT32             size;
  TOPO_OUTPUT_T          tof;
  static HYDRO_OUTPUT_T  hof;
  static NV_INT32        count = 0, total_count = 0, bad_count = 0;
  NV_CHAR                img_file[512], pos_file[512], temp[512], jpg_file[512];
  static NV_CHAR         prev_img_file[512] = {""};
  static FILE            *pos_fp = NULL;
  NV_U_CHAR              *image;
  IMAGE_INDEX_T          image_index;
  static NV_FLOAT32      prev_local_datum = 99999.0, local_datum = 99999.0;
  NV_FLOAT32             heading;
  NV_FLOAT64             lat_degs, lon_degs, x, y, p, r, h, cos_p, cos_r, cos_hc, sin_p, sin_r, sin_hc, c11, c21, 
                         c31, c32, c33, omega, phi, kappa;
  QString                string;
  NV_BOOL                skip_pos = NVFalse;



  //  Check for the data type if needed.

  if (options.type)
    {
      if (options.type == PFM_CHARTS_HOF_DATA)
        {
          if (type != PFM_CHARTS_HOF_DATA && type != PFM_SHOALS_1K_DATA)
            {
              QListWidgetItem *msg = new QListWidgetItem (QString (tr ("Wrong file type %1 for HOF file : %2")).arg (type).arg (path));
              eCheckList->addItem (msg);
              return (0);
            }
        }
      else
        {
          if (type != options.type)
            {
              QListWidgetItem *msg = new QListWidgetItem (QString (tr ("Wrong file type %1 for TOF file : %2")).arg (type).arg (path));
              eCheckList->addItem (msg);
              return (0);
            }
        }
    }
  else
    {
      if (type != PFM_CHARTS_HOF_DATA && type != PFM_SHOALS_1K_DATA && type != PFM_SHOALS_TOF_DATA)
        {
          QListWidgetItem *msg = new QListWidgetItem (QString (tr ("Wrong file type %1 for file : %2")).arg (type).arg (path));
          eCheckList->addItem (msg);
          return (0);
        }
    }


  strcpy (img_file, path);
  strcpy (&img_file[strlen (img_file) - 4], ".img");

  if ((img_fp = open_image_file (img_file)) == NULL)
    {
      QListWidgetItem *msg = new QListWidgetItem (QString (tr ("Unable to open image file : %1")).arg (img_file));
      eCheckList->addItem (msg);
      return (0);
    }


  if (type == PFM_SHOALS_TOF_DATA)
    {
      if ((data_fp = open_tof_file (path)) == NULL)
        {
          perror (path);
          exit (-1);
        }
    }
  else
    {
      if ((data_fp = open_hof_file (path)) == NULL)
        {
          perror (path);
          exit (-1);
        }
    }


  gicount = 0;
  interval_count = 0;
  offset = NINT (options.time_offset * 1000000.0);

  for (i = start ; i <= end ; i++)
    {
      //  Find the record based on the timestamp from the hof or tof file.  

      if (type == PFM_SHOALS_TOF_DATA)
        {
          tof_read_record (data_fp, i, &tof);
          data_timestamp = tof.timestamp;


          if (options.datum_offset == 0.0)
            {
              if (tof.elevation_last > -997.0)
                {
                  local_datum = tof.result_elevation_last - tof.elevation_last;
                }
              else
                {
                  if (prev_local_datum < 99999.0)
                    {
                      local_datum = prev_local_datum;
                    }
                  else
                    {
                      continue;
                    }
                }
            }
          else
            {
              local_datum = options.datum_offset;
            }
        }
      else
        {
          hof_read_record (data_fp, i, &hof);
          data_timestamp = hof.timestamp;


          if (options.datum_offset == 0.0)
            {
              if (hof.correct_depth > -997.0)
                {
                  if (hof.data_type)
                    {
                      local_datum = hof.kgps_datum;
                    }
                  else
                    {
                      fprintf (stderr,"\n\nI don't know how to deal with DGPS data - file %s\n\n", path);
                      exit (-1);
                    }
                }
              else
                {
                  if (prev_local_datum < 99999.0)
                    {
                      local_datum = prev_local_datum;
                    }
                  else
                    {
                      continue;
                    }
                }
            }
          else
            {
              local_datum = options.datum_offset;
            }
        }

      if (local_datum != 0.0) prev_local_datum = local_datum;


      if ((img_rec_num = image_find_record (img_fp, data_timestamp)) != 0)
        {
          image_get_metadata (img_fp, img_rec_num, &image_index);


          //  Don't get the same image twice.  

          if (img_rec_num != prev_img_rec_num) 
            {
              prev_img_rec_num = img_rec_num;


              //  If the image file has changed, find, open, and read the sbet (reprocessed) or pos file information.  

              if (strcmp (img_file, prev_img_file))
                {
                  strcpy (prev_img_file, img_file);


                  if (!get_pos_file (img_file, pos_file))
                    {
                      string.sprintf (tr ("Unable to find pos/sbet file for image file %s").toAscii (), img_file);
                      eCheckList->addItem (string);
                      continue;
                    }


                  //  Check to see if we want to skip .pos files.

                  if (options.pos)
                    {
                      if (strstr (pos_file, "sbet") || strstr (pos_file, "SBET"))
                        {
                          skip_pos = NVFalse;
                        }
                      else
                        {
                          skip_pos = NVTrue;
                        }
                    }

                  if (!skip_pos)
                    {
                      if (pos_fp != NULL) fclose (pos_fp);
                      pos_fp = NULL;

                      if ((pos_fp = open_pos_file (pos_file)) == NULL)
                        {
                          string.sprintf (tr ("Unable to open pos/sbet file for image file %s").toAscii (), pos_file);
                          eCheckList->addItem (string);
                          continue;
                        }
                    }
                }


              //  I HATE using continue but we get WAAAAAAYYYY too indented here if I don't.  In other words, watch for 
              //  these little bastards in the code.

              if (skip_pos) continue;


              //  Get the attitude data for this image.  

              new_stamp = pos_find_record (pos_fp, &pos, image_index.timestamp + offset);


              NV_BOOL good_rec = NVTrue;
              if (!new_stamp) 
                {
                  fprintf (stderr, "\n\nUnable to get timestamp ");
                  fprintf (stderr, NV_INT64_SPECIFIER, image_index.timestamp);
                  fprintf (stderr, " for pos/sbet file %s\n", pos_file);
                  fprintf (stderr, "This usually indicates that the above pos/sbet file is FUBAR or the name is incorrect!\n");
                  fprintf (stderr, "Make sure the file name conforms to the naming convention (_YYMMDD_NNNN.out or .pos) and\n");
                  fprintf (stderr, "check the start and end times of this file (dump_pos) against the data in the HOF/TOF/IMG files.\n\n\n");
                  bad_count++;
                  good_rec = NVFalse;

                  if (bad_count > 100) exit (-1);
                }


              lat_degs = pos.latitude * RAD_TO_DEG;
              lon_degs = pos.longitude * RAD_TO_DEG;


              //  Make sure we're inside the area we specified.  

              if (good_rec && inside (polygon_x, polygon_y, polygon_count, lon_degs, lat_degs))
                {
                  //  If the size is less than 150000 this image is either too bright or too dark.  

                  if (!options.exclude || image_index.image_size >= 150000)
                    {
                      heading = fmod ((pos.platform_heading - pos.wander_angle) * RAD_TO_DEG, 360.0);
                      if (heading < -180.0) heading += 360.0;
                      if (heading > 180.0) heading -=360.0;


                      //  Check for direction limits.

                      NV_INT32 norm_heading = (NV_INT32) heading + 360;
                      if (options.high_limit <= 360) norm_heading %= 360;

                      NV_INT32 opposite_low, opposite_high, opposite_norm_heading;
                      opposite_low = (options.low_limit + 180) % 360;
                      opposite_high = (options.high_limit + 180) % 360;
                      opposite_norm_heading = norm_heading;
                      if (opposite_high <= 360) opposite_norm_heading %= 360;

                      if ((norm_heading >= options.low_limit && norm_heading <= options.high_limit) || 
                          (options.opposite && (opposite_norm_heading >= opposite_low && opposite_norm_heading <= opposite_high)))
                        {
                          //  Check for interval.

                          if (!(interval_count % options.interval) || i == end)
                            {
                              //  Actually read the image.  

                              image = image_read_record_recnum (img_fp, img_rec_num, &size, &image_time);


                              //  Make sure we got an image.

                              if (image != NULL)
                                {
                                  //  Open the output jpg file.  

                                  strcpy (temp, img_file);
                                  strcpy (jpg_file, pfm_basename (temp));
                                  sprintf (&jpg_file[strlen (jpg_file) - 4], "__%s_%02d_%03d.jpg", 
                                           pfm_basename (areafile), gicount, NINT (heading));
                                  sprintf (temp, "%s%1c%s", img_dir, (NV_CHAR) SEPARATOR, jpg_file);

                                  if ((jpg_fp = fopen (temp, "wb")) == NULL) 
                                    {
                                      free (image);
                                      continue;
                                    }


                                  //  Write the image to the jpg file.  

                                  fwrite (image, size, 1, jpg_fp);

                                  fclose (jpg_fp);


                                  if (options.normalize)
                                    {
                                      NV_CHAR norm_file[512];
                                      sprintf (norm_file, "%s.norm.tmp", temp);

                                      QStringList args;
                                      args << QString (temp) << "-normalize" << QString (norm_file);

                                      QString msg;

                                      QProcess normalize;

                                      normalize.start ("convert", args);
                                      if (!normalize.waitForStarted ())
                                        {
                                          msg = tr ("Unable to start the normalize process for file\n") + temp;
                                          QMessageBox::critical (this, tr ("pfmChartsImage Normalize image"), msg);
                                        }
                                      else
                                        {
                                          if (!normalize.waitForFinished())
                                            {
                                              msg = tr ("Normalize process crashed for file\n") + temp;
                                              QMessageBox::critical (this, tr ("pfmChartsImage Normalize image"), msg);
                                            }
                                          else
                                            {
                                              rename (norm_file, temp);
                                            }
                                        }
                                    }


                                  //  Convert to omega, phi, kappa.  

                                  r = (pos.roll * NV_RAD_TO_DEG + options.roll_bias) * NV_DEG_TO_RAD;
                                  p = (pos.pitch * NV_RAD_TO_DEG + options.pitch_bias) * NV_DEG_TO_RAD;
                                  h = (heading + options.heading_bias) * NV_DEG_TO_RAD;
                                  
                                  // Calculate the convergence (true north to grid north) correction to be added to the
                                  // heading value.
                                  // This calculation is from http://www.ordnancesurvey.co.uk/
                                  // Ellipsoid axis dimensions ( a (NV_A0) & b (NV_B0) ) in meters and central meridian
                                  // scale factor ( f0 or 0.9996 )

                                  NV_FLOAT64 f0  = 0.9996L;
                                  NV_FLOAT64 af0 = NV_A0 * f0;
                                  NV_FLOAT64 bf0 = NV_B0 * f0;
                                  NV_FLOAT64 e2  = ((af0 * af0) - (bf0 * bf0)) / (af0 * af0);

                                  NV_FLOAT64 xiii = sin (pos.latitude);
                                  NV_FLOAT64 xiii2 = cos (pos.latitude);
                                  NV_FLOAT64 xiii3 = tan (pos.latitude);

                                  NV_FLOAT64 nu = af0 / sqrt (1.0L - (e2 * xiii * xiii));
                                  NV_FLOAT64 rho = af0 * (1.0 - e2) / pow (1.0 - (e2 * xiii * xiii), 1.5);
                                  NV_FLOAT64 eta2 = (nu / rho) - 1.0L;
                                  NV_FLOAT64 p2 = pos.longitude - (((NV_FLOAT64) (zone - 31) * 6.0L + 3.0L) * NV_DEG_TO_RAD);


                                  // Compute convergence

                                  NV_FLOAT64 xiv = ((-xiii * xiii2 * xiii2) / 3.0L) * (1.0L + (3.0L * eta2) + (2.0L * eta2 * eta2));
                                  NV_FLOAT64 xv = ((-xiii * pow (xiii2, 4.0L)) / 15.0L) * (2.0L - (xiii3 * xiii3));


                                  NV_FLOAT64 lat_long_to_c_rad = (p2 * -xiii) + (pow (p2, 3.0L) * xiv) + (pow (p2, 5.0L) * xv);


                                  //  Add convergence to heading.

                                  NV_FLOAT64 hc = h + lat_long_to_c_rad;


                                  //  Convert lat/lon to northing/easting because MOSAIC wants it that way.  

                                  x = lon_degs * NV_DEG_TO_RAD;
                                  y = lat_degs * NV_DEG_TO_RAD;
                                  pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);


                                  cos_p = cos (p);
                                  sin_p = sin (p);
                                  cos_r = cos (r);
                                  sin_r = sin (r);
                                  cos_hc = cos (hc);
                                  sin_hc = sin (hc);
                                  
                                  c11 = cos_r * cos_hc;
                                  c21 = sin_p * sin_r * cos_hc + cos_p * sin_hc;
                                  c31 = cos_p * sin_r * cos_hc - sin_p * sin_hc;
                                  c32 = -cos_p * sin_r * sin_hc - sin_p * cos_hc;
                                  c33 = cos_p * cos_r;

                                  phi = asin (c31) * RAD_TO_DEG;
                                  omega = atan (-c32 / c33) * RAD_TO_DEG;
                                  kappa = atan2 (-c21, c11) * RAD_TO_DEG;
                                  
                                  fprintf (txt_fp, 
                                           "%d %s%1c%s %0.9f %0.9f %f %f %f %f %d %0.9lf %0.9lf %f %f %f "NV_INT64_SPECIFIER" "NV_INT64_SPECIFIER" %f %f\n",
                                           total_count, img_dir, (NV_CHAR) SEPARATOR, jpg_file, x, y, 
                                           pos.altitude - local_datum, omega, phi, kappa, zone, lat_degs, lon_degs,
                                           pos.roll * RAD_TO_DEG + options.roll_bias,
                                           pos.pitch * RAD_TO_DEG + options.pitch_bias,
                                           heading + options.heading_bias, image_index.timestamp + offset, new_stamp,
                                           pos.platform_heading * RAD_TO_DEG, pos.wander_angle * RAD_TO_DEG);


                                  gicount++;
                                  total_count++;


                                  QString num;
                                  num.sprintf (tr ("Extracting images : %d").toAscii (), total_count);
                                  progress.ebox->setTitle (num);

                                  qApp->processEvents ();

                                  free (image);
                                }
                            }
                          interval_count++;
                        }
                    }
                }
            }
        }


      count++;
      progress.ebar->setValue (count);

      qApp->processEvents ();
    }


  fclose (img_fp);
  fclose (data_fp);

  return (gicount);
}


NV_BOOL pfmChartsImage::misp (NV_FLOAT64 bin_size_meters, NV_CHAR *dem_file, NV_CHAR *casi_file, NV_INT32 pfm_start_x, NV_INT32 pfm_start_y,
                              NV_INT32 pfm_width, NV_INT32 pfm_height, NV_FLOAT64 *level_x, NV_FLOAT64 *level_y, NV_INT32 level_count,
                              NV_FLOAT64 level, LIST_NUM *list, NV_FLOAT32 datum_offset, NV_BOOL flip, NV_BOOL casi)
{
  NV_BOOL             first = NVTrue, no_datum_shift = NVFalse;
  TOPO_OUTPUT_T       tof;
  HYDRO_OUTPUT_T      hof;
  NV_F64_COORD3       xyz;
  NV_I32_COORD2       coord;
  NV_FLOAT64          x, y;
  BIN_RECORD          bin;
  DEPTH_RECORD        *depth;
  NV_INT32            recnum, gridcols, gridrows;
  NV_FLOAT32          *array;
  NV_F64_XYMBR        new_mbr;
  QString             datFile;

  OGRSpatialReference ref;
  GDALDataset         *df;
  char                *wkt = NULL;
  GDALRasterBand      *bd;
  NV_FLOAT64          trans[6];
  GDALDriver          *gt;
  NV_CHAR             **gdal_options = NULL;


  //  Register the progress callback with MISP.

  misp_register_progress_callback (misp_progress_callback);


  NV_INT32 minnex = (NV_INT32) (min_ne_x - (NV_FLOAT64) FILTER * bin_size_meters);
  NV_INT32 minney = (NV_INT32) (min_ne_y - (NV_FLOAT64) FILTER * bin_size_meters);

  min_ne_x = (NV_FLOAT64) minnex;
  min_ne_y = (NV_FLOAT64) minney;

  NV_INT32 maxnex = (NV_INT32) (max_ne_x + (NV_FLOAT64) FILTER * bin_size_meters + 1.0);
  NV_INT32 maxney = (NV_INT32) (max_ne_y + (NV_FLOAT64) FILTER * bin_size_meters + 1.0);

  gridcols = (NV_INT32) ((maxnex - minnex) / bin_size_meters) + 1;
  gridrows = (NV_INT32) ((maxney - minney) / bin_size_meters) + 1;

  max_ne_x = min_ne_x + gridcols * bin_size_meters;
  max_ne_y = min_ne_y + gridrows * bin_size_meters;


  //  We're going to let MISP handle everything in zero based units of the bin size.  That is, we subtract off the
  //  west lon from longitudes then divide by the grid size in the X direction.  We do the same with the latitude using
  //  the south latitude.  This will give us values that range from 0.0 to gridcols in longitude and 0.0 to gridrows
  //  in latitude.

  new_mbr.min_x = 0.0;
  new_mbr.min_y = 0.0;
  new_mbr.max_x = (NV_FLOAT64) gridcols;
  new_mbr.max_y = (NV_FLOAT64) gridrows;


  //  Initialize the MISP engine.  

  misp_init (1.0, 1.0, 0.05, 4, 20.0, 20, 999999.0, -999999.0, -2, new_mbr);


  progress.dbox->setTitle (tr ("Reading data for DEM"));

  progress.dbar->setRange (0, pfm_height);


  // double loop over height & width of area 

  for (NV_INT32 i = pfm_start_y ; i < pfm_start_y + pfm_height ; i++)
    {
      coord.y = i;
      for (NV_INT32 j = pfm_start_x ; j < pfm_start_x + pfm_width ; j++)
        {
          coord.x = j;

          read_bin_record_index (options.pfm_handle, coord, &bin);


          //  Force level value in optional "level" area.

          if (level_count && inside (level_x, level_y, level_count, bin.xy.x, bin.xy.y))
            {
              x = bin.xy.x * NV_DEG_TO_RAD;
              y = bin.xy.y * NV_DEG_TO_RAD;
              pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

              xyz.x = (x - min_ne_x) / bin_size_meters;
              xyz.y = (y - min_ne_y) / bin_size_meters;

              xyz.z = level;

              misp_load (xyz);
            }
          else
            {
              //  We'll take anything that has data, valid or otherwise, since the pictures will be OK even if the data isn't.

              if (bin.num_soundings)
                {
                  //  Get file numbers and min and max record numbers in file so we can figure out which 
                  //  images to retrieve.

                  if (!read_depth_array_index (options.pfm_handle, coord, &depth, &recnum))
                    {
                      for (NV_INT32 k = 0 ; k < recnum ; k++)
                        {
                          if (!(depth[k].validity & PFM_DELETED))
                            {

                              //  The first time through we have to find out if the data has been corrected to local datum.
                              //  This is for later use if we wish to add SRTM data to the DEM.

                              if (first && options.srtm_available && options.srtm && !(depth[k].validity & PFM_INVAL))
                                {
                                  NV_INT16 type;
                                  NV_CHAR path[512];
                                  FILE *data_fp;
 
                                  read_list_file (options.pfm_handle, depth[k].file_number, path, &type);

                                  if (type == PFM_SHOALS_TOF_DATA)
                                    {
                                      if ((data_fp = open_tof_file (path)) == NULL)
                                        {
                                          perror (path);
                                          exit (-1);
                                        }

                                      tof_read_record (data_fp, depth[k].ping_number, &tof);

                                      if (tof.result_elevation_last == tof.elevation_last) no_datum_shift = NVTrue;
                                    }
                                  else
                                    {
                                      if ((data_fp = open_hof_file (path)) == NULL)
                                        {
                                          perror (path);
                                          exit (-1);
                                        }


                                      hof_read_record (data_fp, depth[k].ping_number, &hof);

                                      if (hof.kgps_datum == 0.0) no_datum_shift = NVTrue;
                                    }

                                  fclose (data_fp);


                                  first = NVFalse;
                                }


                              NV_INT32 m = depth[k].file_number;

                              if (m > 10000)
                                {
                                  fprintf (stderr, "\n\nFile number out of bounds - %d\n\n", depth[k].file_number);
                                  exit (-1);
                                }
                              list[m].hit = NVTrue;
                              if (depth[k].ping_number < list[m].start) list[m].start = depth[k].ping_number;
                              if (depth[k].ping_number > list[m].end) list[m].end = depth[k].ping_number;


                              if (!(depth[k].validity & (PFM_INVAL | PFM_REFERENCE)))
                                {
                                  x = depth[k].xyz.x * NV_DEG_TO_RAD;
                                  y = depth[k].xyz.y * NV_DEG_TO_RAD;
                                  pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                  xyz.x = (x - min_ne_x) / bin_size_meters;
                                  xyz.y = (y - min_ne_y) / bin_size_meters;

                                  xyz.z = depth[k].xyz.z;

                                  if (flip) xyz.z = -xyz.z;


                                  //  Zero out water areas.

                                  if (flip)
                                    {
                                      xyz.z -= datum_offset;
  
                                      if (xyz.z < 0.0) xyz.z = 0.0;
                                    }
                                  else
                                    {
                                      xyz.z += datum_offset;

                                      if (xyz.z > 0.0) xyz.z = 0.0;
                                    }

                                  misp_load (xyz);
                                }
                            }
                        }

                      free (depth);
                    }
                }
              else
                {
                  if (options.srtm_available && options.srtm)
                    {
                      NV_INT16 srtm = read_srtm_topo (bin.xy.y, bin.xy.x);

                      if (srtm > -32768)
                        {
                          xyz.z = (NV_FLOAT64) srtm;

                          //  This should switch the SRTM data back to ellipsoid heights.

                          if (no_datum_shift)
                            {
                              NV_FLOAT32 adjust = get_egm08 (bin.xy.y, bin.xy.x);


                              // Adjusting the SRTM up or down depending upon the flip option

                              if (flip)
                                {
                                  xyz.z += (NV_FLOAT64) adjust;
                                }
                              else
                                {
                                  xyz.z -= (NV_FLOAT64) adjust;
                                }
                            }


                          xyz.z -= datum_offset;
                          
                          if (xyz.z < 0.0) xyz.z = 0.0;

                          x = bin.xy.x * NV_DEG_TO_RAD;
                          y = bin.xy.y * NV_DEG_TO_RAD;
                          pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                          xyz.x = (x - min_ne_x) / bin_size_meters;
                          xyz.y = (y - min_ne_y) / bin_size_meters;

                          misp_load (xyz);
                        }
                    }
                }
            }
        }
      progress.dbar->setValue (i - pfm_start_y);

      qApp->processEvents ();
    }
  progress.dbar->reset ();


  if (options.srtm_available && options.srtm) cleanup_srtm_topo ();


  progress.dbox->setTitle (tr ("Generating DEM"));


  //  Setting range to 0, 0 makes the bar just show movement.

  progress.dbar->setRange (0, 0);

  qApp->processEvents ();

  if (misp_proc ()) return (NVTrue);


  //  Allocating one more than gridcols due to constraints of old chrtr (see comments in misp_funcs.c)  

  array = (NV_FLOAT32 *) malloc ((gridcols + 1) * sizeof (NV_FLOAT32));

  if (array == NULL)
    {
      perror ("Allocating array");
      exit (-1);
    }


  gdal_options = CSLSetNameValue (gdal_options, "COMPRESS", "NONE");


  //  Set up the output GeoTIFF file.

  GDALAllRegister ();

  gt = GetGDALDriverManager ()->GetDriverByName ("GTiff");
  if (!gt)
    {
      fprintf (stderr, "Could not get GTiff driver\n");
      exit (-1);
    }

  df = gt->Create (dem_file, gridcols, gridrows, 1, GDT_Float32, gdal_options);
  if (df == NULL)
    {
      fprintf (stderr, "Could not create %s\n", dem_file);
      exit (-1);
    }

  trans[0] = min_ne_x;
  trans[1] = bin_size_meters;
  trans[2] = 0.0;
  trans[3] = max_ne_y;
  trans[4] = 0.0;
  trans[5] = -bin_size_meters;
  df->SetGeoTransform (trans);
  ref.SetUTM (zone);
  ref.SetWellKnownGeogCS ("WGS84");
  ref.exportToWkt (&wkt);
  df->SetProjection (wkt);
  CPLFree (wkt);
  bd = df->GetRasterBand (1);


  progress.dbar->setRange (0, gridrows);

  progress.dbox->setTitle (tr ("Retrieving DEM"));

  FILE *casi_fp = NULL;
  NV_INT32 casi_count = 0;
  if (casi)
    {
      if ((casi_fp = fopen (casi_file, "w")) == NULL)
        QMessageBox::warning (this, tr ("pfmChartsImage"), tr ("Unable to open CASI xyz output file %1").arg (casi_file));
    }

  
  for (NV_INT32 i = 0 ; i < gridrows ; i++)
    {
      if (!misp_rtrv (array)) break;

      bd->RasterIO (GF_Write, 0, (gridrows - 1) - i, gridcols, 1, array, gridcols, 1, GDT_Float32, 0, 0);


      //  Output DEM in CASI format if requested.

      if (casi_fp != NULL)
        {
          NV_FLOAT64 northing = min_ne_y + (NV_FLOAT64) i * bin_size_meters;

          for (NV_INT32 j = 0 ; j < gridcols ; j++)
            {
              casi_count++;
              NV_FLOAT64 easting = min_ne_x + (NV_FLOAT64) j * bin_size_meters;
              fprintf (casi_fp, "%06d %10.3f   %11.3f %.2f\n", casi_count, easting, northing, array[j]);
            }
        }

      progress.dbar->setValue (i);

      qApp->processEvents ();
    }


  if (casi_fp != NULL) fclose (casi_fp);


  delete df;


  progress.dbar->setValue (gridrows);

  qApp->processEvents ();


  free (array);

  return (NVFalse);
}



void 
pfmChartsImage::slotMosaicReadyReadStandardError ()
{
  static QString resp_string = "";


  QByteArray response = mosaicProc->readAllStandardError ();


  //  Parse the return response for carriage returns and line feeds

  for (NV_INT32 i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n')
        {
          QListWidgetItem *cur = new QListWidgetItem (resp_string);

          wCheckList->addItem (cur);
          wCheckList->setCurrentItem (cur);
          wCheckList->scrollToItem (cur);

          resp_string = "";
        }
      else if (response.at (i) == '\r')
        {
          QListWidgetItem *rep = wCheckList->item (wCheckList->currentRow ());
          rep->setText (resp_string);

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



void 
pfmChartsImage::slotMosaicReadyReadStandardOutput ()
{
  static QString resp_string = "";


  QByteArray response = mosaicProc->readAllStandardOutput ();


  //  Parse the return response for carriage returns and line feeds

  for (NV_INT32 i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n' || response.at (i) == '\r')
        {
          if (resp_string.contains ("Writer status -"))
            {
              NV_CHAR string[1024];
              strcpy (string, resp_string.toAscii ());

              NV_INT32 t;
              sscanf (string, "Writer status - %d of %d", &t, &n_blocks);

              if (!writer_stat)
                {
                  progress.mbar->reset ();
                  progress.mbar->setRange (0, n_blocks);
                  writer_stat = NVTrue;
                }

              if (n_blocks > 0)
                {
                  progress.mbar->setValue (t);
                }
            }


          if (response.at (i) == '\r')
            {
              QListWidgetItem *rep = wCheckList->item (wCheckList->currentRow ());
              rep->setText (resp_string);
            }
          else
            {
              QListWidgetItem *cur = new QListWidgetItem (resp_string);

              wCheckList->addItem (cur);
              wCheckList->setCurrentItem (cur);
              wCheckList->scrollToItem (cur);
            }
          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



void 
pfmChartsImage::slotMosaicError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmChartsImage mosaic"), tr ("Unable to start the mosaic process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmChartsImage mosaic"), tr ("The mosaic process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmChartsImage mosaic"), tr ("The mosaic process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmChartsImage mosaic"), 
                             tr ("There was a write error from the mosaic process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmChartsImage mosaic"),
                             tr ("There was a read error from the mosaic process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmChartsImage mosaic"), tr ("The mosaic process died with an unknown error!"));
      break;
    }
}



void 
pfmChartsImage::slotMosaicDone (int exitCode __attribute__ ((unused)), 
                                QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  QFile *tmp_file = new QFile (tmp_file_name);
  QFile *out_file = new QFile (out_file_name);
  out_file->remove ();
  tmp_file->rename (out_file_name);


  progress.mbar->setRange (0, 100);
  progress.mbar->reset ();
  writer_stat = NVFalse;


  qApp->processEvents ();

 
  button (QWizard::FinishButton)->setEnabled (TRUE);
  button (QWizard::CancelButton)->setEnabled (FALSE);


  wCheckList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Mosaic complete, press Finish to exit or Back to select new images."));

  wCheckList->addItem (cur);
  wCheckList->setCurrentItem (cur);
  wCheckList->scrollToItem (cur);


  QApplication::restoreOverrideCursor ();
}



//  Get the users defaults.

void pfmChartsImage::envin (OPTIONS *options)
{
  NV_FLOAT64 saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  options->type = 0;
  options->roll_bias = 0.0;
  options->pitch_bias = 0.0;
  options->heading_bias = 0.0;
  options->datum_offset = 0.0;
  options->grid_size = 1.0;
  options->time_offset = 0.1;
  options->interval = 1;
  options->low_limit = 0;
  options->high_limit = 360;
  options->casi = NVFalse;
  options->remove = NVTrue;
  options->flip = NVTrue;
  options->exclude = NVFalse;
  options->opposite = NVFalse;
  options->normalize = NVFalse;
  options->utm = NVFalse;
  options->caris = NVFalse;  //  Don't save this - bad juju!
  options->srtm = NVTrue;
  options->pos = NVTrue;
  options->cell_size = 0.2;
  options->focal_length = 16.065;
  options->pixel_size = 7.40;
  options->column_offset = 30.11;
  options->row_offset = -11.68;
  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 800;
  options->window_height = 400;
  options->level = 0.0;
  options->param_dir = ".";
  options->output_dir = ".";
  options->input_dir = ".";
  options->area_dir = ".";
  options->level_dir = ".";


  QSettings settings (tr ("navo.navy.mil"), tr ("pfmChartsImage"));

  settings.beginGroup (tr ("pfmChartsImage"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->casi = settings.value (tr ("casi"), options->casi).toBool ();

  options->flip = settings.value (tr ("flip"), options->flip).toBool ();

  options->exclude = settings.value (tr ("exclude"), options->exclude).toBool ();

  options->opposite = settings.value (tr ("opposite"), options->opposite).toBool ();

  options->normalize = settings.value (tr ("normalize"), options->normalize).toBool ();

  options->pos = settings.value (tr ("pos"), options->pos).toBool ();

  options->srtm = settings.value (tr ("srtm"), options->srtm).toBool ();

  options->remove = settings.value (tr ("delete directory"), options->remove).toBool ();

  options->utm = settings.value (tr ("UTM projected output"), options->utm).toBool ();

  options->roll_bias = settings.value (tr ("roll bias"), options->roll_bias).toDouble ();

  options->pitch_bias = settings.value (tr ("pitch bias"), options->pitch_bias).toDouble ();

  options->heading_bias = settings.value (tr ("heading bias"), options->heading_bias).toDouble ();

  options->datum_offset = settings.value (tr ("datum offset"), options->datum_offset).toDouble ();

  options->time_offset = settings.value (tr ("time offset"), options->time_offset).toDouble ();

  options->grid_size = settings.value (tr ("grid size"), options->grid_size).toDouble ();

  options->interval = settings.value (tr ("interval"), options->interval).toInt ();

  options->low_limit = settings.value (tr ("low limit"), options->low_limit).toInt ();

  options->high_limit = settings.value (tr ("high limit"), options->high_limit).toInt ();

  options->type = settings.value (tr ("data type option"), options->type).toInt ();

  options->cell_size = settings.value (tr ("cell size"), options->cell_size).toDouble ();

  options->focal_length = settings.value (tr ("focal length"), options->focal_length).toDouble ();

  options->pixel_size = settings.value (tr ("pixel size"), options->pixel_size).toDouble ();

  options->column_offset = settings.value (tr ("column offset"), options->column_offset).toDouble ();

  options->row_offset = settings.value (tr ("row offset"), options->row_offset).toDouble ();

  options->level = settings.value (tr ("area file level"), options->level).toDouble ();

  options->param_dir = settings.value (tr ("parameters directory"), options->param_dir).toString ();
  options->output_dir = settings.value (tr ("output directory"), options->output_dir).toString ();
  options->input_dir = settings.value (tr ("input directory"), options->input_dir).toString ();
  options->area_dir = settings.value (tr ("area directory"), options->area_dir).toString ();
  options->level_dir = settings.value (tr ("level area directory"), options->level_dir).toString ();

  options->window_width = settings.value (tr ("width"), options->window_width).toInt ();
  options->window_height = settings.value (tr ("height"), options->window_height).toInt ();
  options->window_x = settings.value (tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (tr ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void pfmChartsImage::envout (OPTIONS *options)
{
  QSettings settings (pfmChartsImage::tr ("navo.navy.mil"), pfmChartsImage::tr ("pfmChartsImage"));

  settings.beginGroup (pfmChartsImage::tr ("pfmChartsImage"));


  settings.setValue (tr ("settings version"), settings_version);


  settings.setValue (tr ("casi"), options->casi);

  settings.setValue (tr ("flip"), options->flip);

  settings.setValue (tr ("exclude"), options->exclude);

  settings.setValue (tr ("opposite"), options->opposite);

  settings.setValue (tr ("normalize"), options->normalize);

  settings.setValue (tr ("pos"), options->pos);

  settings.setValue (tr ("srtm"), options->srtm);

  settings.setValue (tr ("delete directory"), options->remove);

  settings.setValue (tr ("UTM projected output"), options->utm);

  settings.setValue (tr ("roll bias"), options->roll_bias);

  settings.setValue (tr ("pitch bias"), options->pitch_bias);

  settings.setValue (tr ("heading bias"), options->heading_bias);

  settings.setValue (tr ("datum offset"), options->datum_offset);

  settings.setValue (tr ("time offset"), options->time_offset);

  settings.setValue (tr ("grid size"), options->grid_size);

  settings.setValue (tr ("interval"), options->interval);

  settings.setValue (tr ("low limit"), options->low_limit);


  NV_INT32 tmp = options->high_limit;
  if (options->high_limit > 360) tmp -= 360;
  settings.setValue (tr ("high limit"), tmp);


  settings.setValue (tr ("data type option"), options->type);

  settings.setValue (tr ("cell size"), options->cell_size);

  settings.setValue (tr ("focal length"), options->focal_length);

  settings.setValue (tr ("pixel size"), options->pixel_size);

  settings.setValue (tr ("column offset"), options->column_offset);

  settings.setValue (tr ("row offset"), options->row_offset);

  settings.setValue (tr ("area file level"), options->level);

  settings.setValue (tr ("parameters directory"), options->param_dir);
  settings.setValue (tr ("output directory"), options->output_dir);
  settings.setValue (tr ("input directory"), options->input_dir);
  settings.setValue (tr ("area directory"), options->area_dir);
  settings.setValue (tr ("level area directory"), options->level_dir);

  settings.setValue (tr ("width"), options->window_width);
  settings.setValue (tr ("height"), options->window_height);
  settings.setValue (tr ("x position"), options->window_x);
  settings.setValue (tr ("y position"), options->window_y);

  settings.endGroup ();
}
