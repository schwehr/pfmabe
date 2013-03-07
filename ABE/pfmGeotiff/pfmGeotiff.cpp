
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



#include "pfmGeotiff.hpp"
#include "pfmGeotiffHelp.hpp"


void set_defaults (OPTIONS *options);
void envin (OPTIONS *options);
void envout (OPTIONS *options);


pfmGeotiff::pfmGeotiff (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  contour = NVFalse;


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmGeotiffWatermark.png"));


  //  Get the sample data for the color and sunshade examples.

  options.sample_pixmap = QPixmap (SAMPLE_WIDTH, SAMPLE_HEIGHT);
  NV_U_CHAR idata[2];
  QFile *dataFile = new QFile (":/icons/data.dat");
  options.sample_min = 99999.0;
  options.sample_max = -99999.0;

  if (dataFile->open (QIODevice::ReadOnly))
    {
      for (NV_INT32 i = 0 ; i < SAMPLE_HEIGHT ; i++)
        {
          for (NV_INT32 j = 0 ; j < SAMPLE_WIDTH ; j++)
            {
              dataFile->read ((NV_CHAR *) idata, 2);
              options.sample_data[i][j] = idata[1] * 256 + idata[0];

              options.sample_min = qMin ((NV_FLOAT32) options.sample_data[i][j], options.sample_min);
              options.sample_max = qMax ((NV_FLOAT32) options.sample_data[i][j], options.sample_max);
            }
        }
      dataFile->close ();
    }


  //  Set the normal defaults

  set_defaults (&options);


  //  Get the user's defaults if available

  envin (&options);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  area_file_name = tr ("NONE");


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, &options, this));

  setPage (1, new surfacePage (this, &options));

  setPage (2, (ip = new imagePage (this, &options)));

  setPage (3, new runPage (this, &progress, &checkList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start generating the GeoTIFF(s)"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}


pfmGeotiff::~pfmGeotiff ()
{
}



void pfmGeotiff::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      options.transparent = field ("transparent_check").toBool ();
      options.packbits = field ("packbits_check").toBool ();
      options.grey = field ("grey_check").toBool ();
      options.dumb = field ("dumb_check").toBool ();
      options.intrp = field ("in_check").toBool ();
      options.smoothing_factor = field ("smoothing").toInt ();
      options.navd88 = field ("navd88").toBool ();
      options.elev = field ("elev_check").toBool ();
      options.cint = (NV_FLOAT32) field ("interval").toDouble ();

      if (options.grey)
        {
          ip->enable (NVFalse);
        }
      else
        {
          ip->enable (NVTrue);
        }
      break;

    case 3:
      button (QWizard::CustomButton1)->setEnabled (TRUE);

      pfm_file_name = field ("pfm_file_edit").toString ();
      area_file_name = field ("area_file_edit").toString ();
      output_file_name = field ("output_file_edit").toString ();


      //  Save the output directory.  It might have been input manually instead of browsed.

      options.output_dir = QFileInfo (output_file_name).absoluteDir ().absolutePath ();


      options.azimuth = field ("sunAz").toDouble ();
      options.elevation = field ("sunEl").toDouble ();
      options.exaggeration = field ("sunEx").toDouble ();
      options.saturation = field ("satSpin").toDouble ();
      options.value = field ("valSpin").toDouble ();
      options.start_hsv = field ("startSpin").toDouble ();
      options.end_hsv = field ("endSpin").toDouble ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      //  Save the options.

      envout (&options);


      QString string;

      checkList->clear ();

      string = tr ("Input PFM file : ") + pfm_file_name;
      checkList->addItem (string);

      string = tr ("Output file(s) : ") + output_file_name;
      checkList->addItem (string);

      if (!area_file_name.isEmpty ())
        {
          string = tr ("Area file : ") + area_file_name;
          checkList->addItem (string);
        }


      switch (options.surface)
        {
        case 0:
          string = tr ("Minimum Filtered Surface");
          checkList->addItem (string);
          break;

        case 1:
          string = tr ("Maximum Filtered Surface");
          checkList->addItem (string);
          break;

        case 2:
          string = tr ("Average Filtered or MISP Surface");
          checkList->addItem (string);
          break;

        case 3:
          string = tr ("Data Type Coverages");
          checkList->addItem (string);
          break;
        }


      switch (options.packbits)
        {
        case FALSE:
          string = tr ("LZW compressed output format");
          checkList->addItem (string);
          break;

        case TRUE:
          string = tr ("Packbits compressed output format");
          checkList->addItem (string);
          break;
        }


      switch (options.elev)
        {
        case FALSE:
          string = tr ("Data will be output as depths");
          checkList->addItem (string);
          break;

        case TRUE:
          string = tr ("Data will be output as elevations");
          checkList->addItem (string);
          break;
        }


      switch (options.navd88)
        {
        case FALSE:
          string = tr ("Data will be output relative to WGS84");
          checkList->addItem (string);
          break;

        case TRUE:
          string = tr ("Data will be output relative to NAVD88");
          checkList->addItem (string);
          break;
        }


      if (options.cint != 0.0 && options.surface != 3)
        {
          contour = NVTrue;
          string = QString (tr ("ESRI contour file will be generated with a contour interval of %1")).arg (options.cint, 6, 'f', 2);
          checkList->addItem (string);
        }
      else
        {
          contour = NVFalse;
        }



      switch (options.units)
        {
        case 0:
          string = tr ("Units : Meters");
          checkList->addItem (string);
          break;

        case 1:
          if (options.dumb)
            {
              string = tr ("Units : Fathoms (1500 m/sec)");
            }
          else
            {
              string = tr ("Units : Fathoms (4800 ft/sec)");
            }
          checkList->addItem (string);
          break;
        }



      QListWidgetItem *cur;

      if (!options.grey)
        {
          switch (options.transparent)
            {
            case FALSE:
              string = tr ("Empty cells are blank");
              checkList->addItem (string);
              break;

            case TRUE:
              string = tr ("Empty cells are transparent");
              checkList->addItem (string);
              break;
            }


          switch (options.restart)
            {
            case FALSE:
              string = tr ("Color map is continuous from minimum to maximum");
              checkList->addItem (string);
              break;

            case TRUE:
              string = tr ("Color map starts over at zero boundary");
              checkList->addItem (string);
              break;
            }

          string = QString (tr ("Sun Azimuth : %1")).arg (options.azimuth, 6, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Sun Elevation : %1")).arg (options.elevation, 5, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Vertical Exaggeration : %1")).arg (options.exaggeration, 4, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Color Saturation : %1")).arg (options.saturation, 4, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Color Value : %1")).arg (options.value, 4, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Start Hue Value : %1")).arg (options.start_hsv, 6, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("End Hue Value : %1")).arg (options.end_hsv, 6, 'f', 2);
          cur = new QListWidgetItem (string);
        }
      else
        {
          string = QString (tr ("Output is 32 bit floating point"));
          cur = new QListWidgetItem (string);
        }


      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);


      if (contour)
        {
          progress.cbox->show ();
        }
      else
        {
          progress.cbox->hide ();
        }


      break;
    }
}



void pfmGeotiff::cleanupPage (int id)
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
    }
}



void pfmGeotiff::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void pfmGeotiff::load_arrays (NV_INT32 layer_type, NV_INT32 count, PFM_OPEN_ARGS *open_args, BIN_RECORD bin_record[], NV_FLOAT32 data[])
{
    NV_INT32            i;


    for (i = 0 ; i < count ; i++)
      {
        data[i] = open_args->head.null_depth;

        if (bin_record[i].num_soundings || (bin_record[i].validity & (PFM_DATA | PFM_INTERPOLATED)))
          {
            switch (layer_type)
              {
              case 2:
              case 3:
                if (bin_record[i].validity & (PFM_DATA | PFM_INTERPOLATED)) data[i] = bin_record[i].avg_filtered_depth;
                if (options.units)
                  {
                    if (options.dumb)
                      {
                        data[i] /= 1.875;
                      }
                    else
                      {
                        data[i] /= 1.8288;
                      }
                  }
                break;

              case 0:
                if (bin_record[i].validity & PFM_DATA) data[i] = bin_record[i].min_filtered_depth;
                if (options.units)
                  {
                    if (options.dumb)
                      {
                        data[i] /= 1.875;
                      }
                    else
                      {
                        data[i] /= 1.8288;
                      }
                  }
                break;

              case 1:
                if (bin_record[i].validity & PFM_DATA) data[i] = bin_record[i].max_filtered_depth;
                if (options.units)
                  {
                    if (options.dumb)
                      {
                        data[i] /= 1.875;
                      }
                    else
                      {
                        data[i] /= 1.8288;
                      }
                  }
                break;
              }


            if (options.elev)
              {
                data[i] = -data[i];

                if (options.navd88 && -data[i] < open_args->head.max_depth)
                  {
                    NV_FLOAT32 value = get_geoid03 (bin_record[i].xy.y, bin_record[i].xy.x);

                    if (value != -999.0) data[i] -= value;
                  }
              }
            else
              { 
                if (options.navd88 && data[i] < open_args->head.max_depth)
                  {
                    NV_FLOAT32 value = get_geoid03 (bin_record[i].xy.y, bin_record[i].xy.x);

                    if (value != -999.0) data[i] += value;
                  }
              }
          }
        else
          {
            if (options.elev) data[i] = -data[i];
          }
      }
}



//  This is where the fun stuff happens.

void 
pfmGeotiff::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  NV_INT32            i, j, k, m, pfm_handle, c_index, numrecs, width, height, x_start, y_start, count = 0;
  NV_INT16            types[32], type_count, filetype[9999];
  NV_FLOAT32          *current_row, *next_row, min_z, max_z, range[2] = {0.0, 0.0}, shade_factor, depth = 0.0, *ar = NULL;
  NV_FLOAT64          conversion_factor, mid_y_radians, x_cell_size, y_cell_size;
  NV_I32_COORD2       coord;
  NV_FLOAT64          polygon_x[200], polygon_y[200];
  NV_F64_XYMBR        mbr;
  BIN_RECORD          *current_record;
  DEPTH_RECORD        *depth_record;
  PFM_OPEN_ARGS       open_args;  
  NV_U_CHAR           hit[32];
  NV_BOOL             *fill, cross_zero = NVFalse, area = NVFalse;
  NV_CHAR             basename[512], name[32][512], file[512], area_file[512];
  FILE                *fp[32];
  QString             string;


  NV_INT32 scribe (NV_INT32 num_cols, NV_INT32 num_rows, NV_FLOAT32 xorig, NV_FLOAT32 yorig, NV_FLOAT32 min_z, NV_FLOAT32 max_z, NV_FLOAT32 *ar,
                   QString shapeName, OPTIONS *options, PFM_OPEN_ARGS open_args);


  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (FALSE);
  button (QWizard::BackButton)->setEnabled (FALSE);
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  //  Note - The sunopts and the color_array get set in display_sample_data (imagePage.cpp).  No point in
  //  doing it twice.


  strcpy (open_args.list_path, pfm_file_name.toAscii ());


  open_args.checkpoint = 0;
  pfm_handle = open_existing_pfm_file (&open_args);


  if (pfm_handle < 0) pfm_error_exit (pfm_error);


  if (options.surface == 3)  // COVERAGE
    {
      memset (types, 0, 32 * sizeof (NV_INT16));

      type_count = 0;
      for (i = 0 ; i < 9999 ; i++)
        {
          if (read_list_file (pfm_handle, i, file, &filetype[i])) break;


          //  We don't want to use pfmView created GSF files for coverage.

          if (filetype[i] != PFM_GSF_DATA || !strstr (file, "pfmView_GSF"))
            {
              filetype[i] = filetype[i];
              hit[0] = 0;
              for (j = 0 ; j < type_count ; j++)
                {
                  if (types[j] == filetype[i])
                    {
                      hit[0] = 1;
                      break;
                    }
                }

              if (!hit[0])
                {
                  types[type_count] = filetype[i];
                  type_count++;
                }
            }
          else
            {
              filetype[i] = -1;
            }
        }


      strcpy (basename, output_file_name.toAscii ());


      for (i = 0 ; i < type_count ; i++)
        {
          //  Strip off the trailing .tif if it's there.

          if (!strcmp (&basename[strlen (basename) - 4], ".tif")) basename[strlen (basename) - 4] = 0;

          sprintf (name[i], "%s_type%02d_coverage.tif", basename, types[i]);
          sprintf (file, "%s_type%02d_coverage.tmp", basename, types[i]);
          if ((fp[i] = fopen (file, "w+")) ==  NULL)
            {
              perror (file);
              exit (-1);
            }
        }
    }
  else
    {
      strcpy (basename, output_file_name.toAscii ());

      if (strcmp (&basename[strlen (basename) - 4], ".tif")) strcat (basename, ".tif");

      type_count = 1;
      strcpy (name[0], basename);
    }


  x_start = 0;
  y_start = 0;
  width = open_args.head.bin_width;
  height = open_args.head.bin_height;


  //  Check for an area file.

  mbr = open_args.head.mbr;
  if (!area_file_name.isEmpty ())
    {
      strcpy (area_file, area_file_name.toAscii ());
      if (get_area_mbr (area_file, &count, polygon_x, polygon_y, &mbr))
        {
          if (mbr.min_y > open_args.head.mbr.max_y || mbr.max_y < open_args.head.mbr.min_y ||
              mbr.min_x > open_args.head.mbr.max_x || mbr.max_x < open_args.head.mbr.min_x)
            {
              fprintf (stderr, tr ("\n\nSpecified area is completely outside of the PFM bounds!\n\n").toAscii ());
              exit (-1);
            }


          //  Match to nearest cell

          x_start = NINT ((mbr.min_x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees);
          y_start = NINT ((mbr.min_y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees);
          width = NINT ((mbr.max_x - mbr.min_x) / open_args.head.x_bin_size_degrees);
          height = NINT ((mbr.max_y - mbr.min_y) / open_args.head.y_bin_size_degrees);


          //  Adjust to PFM bounds if necessary

          if (x_start < 0) x_start = 0;
          if (y_start < 0) y_start = 0;
          if (x_start + width > open_args.head.bin_width) width = open_args.head.bin_width - x_start;
          if (y_start + height > open_args.head.bin_height) height = open_args.head.bin_height - y_start;


          //  Redefine bounds

          mbr.min_x = open_args.head.mbr.min_x + x_start * open_args.head.x_bin_size_degrees;
          mbr.min_y = open_args.head.mbr.min_y + y_start * open_args.head.y_bin_size_degrees;
          mbr.max_x = mbr.min_x + width * open_args.head.x_bin_size_degrees;
          mbr.max_y = mbr.min_y + height * open_args.head.y_bin_size_degrees;

          area = NVTrue;
        }
    }

  progress.mbar->setRange (0, height);


  //  Compute cell sizes for sunshading.

  mid_y_radians = (open_args.head.mbr.max_y - open_args.head.mbr.min_y) * 0.0174532925199432957692;
  conversion_factor = cos (mid_y_radians);
  x_cell_size = open_args.head.x_bin_size_degrees * 111120.0 * conversion_factor;
  y_cell_size = open_args.head.y_bin_size_degrees * 111120.0;


  if ((current_record = (BIN_RECORD *) calloc (width, sizeof (BIN_RECORD))) == NULL)
    {
      perror ("Allocating current_record in pfmGeotiff.cpp");
      exit (-1);
    }


  NV_U_CHAR *red = NULL, *blue = NULL, *green = NULL, *alpha = NULL;

  if ((red = (NV_U_CHAR *) calloc (width, sizeof (NV_U_CHAR))) == NULL)
    {
      perror ("Allocating red in pfmGeotiff.cpp");
      exit (-1);
    }
  if ((green = (NV_U_CHAR *) calloc (width, sizeof (NV_U_CHAR))) == NULL)
    {
      perror ("Allocating green in pfmGeotiff.cpp");
      exit (-1);
    }
  if ((blue = (NV_U_CHAR *) calloc (width, sizeof (NV_U_CHAR))) == NULL)
    {
      perror ("Allocating blue in pfmGeotiff.cpp");
      exit (-1);
    }
  if ((alpha = (NV_U_CHAR *) calloc (width, sizeof (NV_U_CHAR))) == NULL)
    {
      perror ("Allocating alpha in pfmGeotiff.cpp");
      exit (-1);
    }
  if ((next_row = (NV_FLOAT32 *) calloc (width, sizeof (NV_FLOAT32))) == NULL)
    {
      perror ("Allocating next_row in pfmGeotiff.cpp");
      exit (-1);
    }
  if ((current_row = (NV_FLOAT32 *) calloc (width, sizeof (NV_FLOAT32))) == NULL)
    {
      perror ("Allocating current_row in pfmGeotiff.cpp");
      exit (-1);
    }


  if (contour)
    {
      NV_INT32 ar_size = width * height;

      ar = (NV_FLOAT32 *) calloc (ar_size, sizeof (NV_FLOAT32));
      if (ar == NULL)
        {
          perror ("Allocating ar in pfmGeotiff.cpp");
          exit (-1);
        }
    }


  //  Scan for min/max and (optionally) load the contour array.

  min_z = 999999999.0;
  max_z = -999999999.0;

  for (i = 0, m = 1 ; i < height ; i++, m++)
    {
      read_bin_row (pfm_handle, width, y_start + i, x_start, current_record);
      coord.y = y_start + i;

      for (j = 0 ; j < width ; j++)
        {
          coord.x = x_start + j;

          NV_BOOL got_data = NVFalse;

          switch (options.surface)
            {
            case 0:  // MIN_FILTERED_DEPTH
              if (current_record[j].validity & PFM_DATA)
                {
                  depth = current_record[j].min_filtered_depth;
                  got_data = NVTrue;
                }
              break;

            case 1:  // MAX_FILTERED_DEPTH
              if (current_record[j].validity & PFM_DATA)
                {
                  depth = current_record[j].max_filtered_depth;
                  got_data = NVTrue;
                }
              break;

            case 2:  // AVERAGE_FILTERED_DEPTH
              if (options.intrp)
                {
                  if (current_record[j].validity & (PFM_DATA | PFM_INTERPOLATED))
                    {
                      depth = current_record[j].avg_filtered_depth;
                      got_data = NVTrue;
                    }
                }
              else
                {
                  if (current_record[j].validity & PFM_DATA)
                    {
                      depth = current_record[j].avg_filtered_depth;
                      got_data = NVTrue;
                    }
                }
              break;

            case 3:  // COVERAGE
              memset (hit, 0, sizeof (hit));
              if (current_record[j].validity & PFM_DATA)
                {
                  if (!read_depth_array_index (pfm_handle, coord, &depth_record, &numrecs))
                    {
                      for (k = 0 ; k < numrecs ; k++)
                        {
                          //  Don't use pfmView generated GSF files for coverage.

                          if (!(depth_record[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)) && 
                              filetype[depth_record[k].file_number] >= 0)
                            {
                              hit[filetype[depth_record[k].file_number]] = 1;
                              got_data = NVTrue;
                            }
                        }
                      free (depth_record);
                    }
                  depth = current_record[j].avg_filtered_depth;
                }

              for (k = 0 ; k < type_count ; k++) fwrite (&hit[types[k]], 1, 1, fp[k]);

              break;
            }

          if (got_data && depth < open_args.head.max_depth)
            {
              if (options.units)
                {
                  if (options.dumb)
                    {
                      depth /= 1.875;
                    }
                  else
                    {
                      depth /= 1.8288;
                    }
                }


              if (options.elev)
                {
                  depth = -depth;

                  if (options.navd88)
                    {
                      NV_FLOAT32 value = get_geoid03 (current_record[j].xy.y, current_record[j].xy.x);

                      if (value != -999.0) depth -= value;
                    }
                }
              else
                {
                  if (options.navd88)
                    {
                      NV_FLOAT32 value = get_geoid03 (current_record[j].xy.y, current_record[j].xy.x);

                      if (value != -999.0) depth += value;
                    }
                }


              //  Load the 1D contour array.

              if (contour) ar[i * width + j] = depth; 


              //  Compute the min/max

              if (min_z > depth) min_z = depth;
              if (max_z < depth) max_z = depth;
            }
          else
            {
              //  Load the 1D contour array.

              if (contour)
                {
                  ar[i * width + j] = open_args.head.null_depth;
                  if (options.elev) ar[i * width + j] = -ar[i * width + j];
                }
            }
        }

      progress.mbar->setValue (m);

      qApp->processEvents ();
    }


  progress.mbar->setValue (height);



  progress.gbar->setRange (0, height);


  //  Back up past the last record since we'll be reading this puppy backwards.

  if (options.surface == 3)  // COVERAGE
    {
      for (i = 0 ; i < type_count ; i++) 
        {
          fseek (fp[i], -width, SEEK_END);
        }
    }


  if (options.restart && min_z < 0.0)
    {
      range[0] = -min_z;
      range[1] = max_z;

      cross_zero = NVTrue;
    }
  else
    {
      range[0] = max_z - min_z;

      cross_zero = NVFalse;
    }


  fill = (NV_BOOL *) calloc (width, sizeof (NV_BOOL));
  if (fill == NULL)
    {
      perror (tr ("Allocating fill").toAscii ());
      exit (-1);
    }


  for (m = 0 ; m < type_count ; m++)
    {
      OGRSpatialReference ref;
      NV_CHAR             wkt[1024];
      GDALDataset         *df;
      GDALRasterBand      *bd[4];
      NV_FLOAT64          trans[6];
      GDALDriver          *gt;
      NV_CHAR             **papszOptions = NULL;


      //  Set up the output GeoTIFF file.

      GDALAllRegister ();

      gt = GetGDALDriverManager ()->GetDriverByName ("GTiff");
      if (!gt)
        {
          fprintf (stderr, "Could not get GTiff driver\n");
          exit (-1);
        }

      NV_INT32 bands = 3;
      if (options.transparent) bands = 4;
      if (options.grey) bands = 1;


      //  Packbits compression.

      if (options.packbits)
        {
          papszOptions = CSLSetNameValue (papszOptions, "COMPRESS", "PACKBITS");
        }
      else
        {
          papszOptions = CSLSetNameValue (papszOptions, "TILED", "NO");
          papszOptions = CSLSetNameValue (papszOptions, "COMPRESS", "LZW");
        }

      if (options.grey)
        {
          df = gt->Create (name[m], width, height, bands, GDT_Float32, papszOptions);
        }
      else
        {
          df = gt->Create (name[m], width, height, bands, GDT_Byte, papszOptions);
        }

      if (df == NULL)
        {
          fprintf (stderr, "Could not create %s\n", name[m]);
          exit (-1);
        }

      trans[0] = mbr.min_x;
      trans[1] = open_args.head.x_bin_size_degrees;
      trans[2] = 0.0;
      trans[3] = mbr.max_y;
      trans[4] = 0.0;
      trans[5] = -open_args.head.y_bin_size_degrees;
      df->SetGeoTransform (trans);

      if (options.navd88)
        {
          strcpy (wkt, "COMPD_CS[\"WGS84 with ellipsoid Z\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],VERT_CS[\"North American Vertical Datum of 1988\",VERT_DATUM[\"North American Vertical Datum 1988\",2005,AUTHORITY[\"EPSG\",\"5103\"]],UNIT[\"m\",1.0],AXIS[\"Gravity-related height\",UP],AUTHORITY[\"EPSG\",\"5703\"]]]");
        }
      else
        {
          strcpy (wkt, "COMPD_CS[\"WGS84 with ellipsoid Z\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],VERT_CS[\"ellipsoid Z in meters\",VERT_DATUM[\"Ellipsoid\",2002],UNIT[\"metre\",1],AXIS[\"Z\",UP]]]");
        }

      df->SetProjection (wkt);


      for (i = 0 ; i < bands ; i++) bd[i] = df->GetRasterBand (i + 1);


      if (options.grey)
        {
          if (options.elev)
            {
              bd[0]->SetNoDataValue (-open_args.head.null_depth);
            }
          else
            {
              bd[0]->SetNoDataValue (open_args.head.null_depth);
            }
        }


      for (i = height - 1, k = 0 ; i >= 0 ; i--, k++)
        {
          CPLErr err;

          if (options.grey)
            {
              read_bin_row (pfm_handle, width, y_start + i, x_start, current_record);
              load_arrays (options.surface, width, &open_args, current_record, current_row);
            }
          else
            {
              if (i == (height - 1))
                {
                  read_bin_row (pfm_handle, width, y_start + 1, x_start, current_record);
                  load_arrays (options.surface, width, &open_args, current_record, current_row);
                  memcpy (next_row, current_row, width * sizeof (NV_FLOAT32));
                }
              else
                {
                  memcpy (current_row, next_row, width * sizeof (NV_FLOAT32));
                  read_bin_row (pfm_handle, width, y_start + i, x_start,  current_record);
                  load_arrays (options.surface, width, &open_args, current_record, next_row);
                }
            }


          if (options.surface == 3)  // COVERAGE
            {
              fread (fill, width, 1, fp[m]);
              fseek (fp[m], -(2 * width), SEEK_CUR);
            }
          else
            {
              memset (fill, 1, width);
            }


          if (options.grey)
            {
              err = bd[0]->RasterIO (GF_Write, 0, k, width, 1, current_row, width, 1, GDT_Float32, 0, 0);
            }
          else
            {
              for (j = 0 ; j < width ; j++)
                {
                  if (cross_zero)
                    {
                      if (current_row[j] < 0.0)
                        {
                          c_index = (NV_INT32) (NUMHUES - (NV_INT32) (fabsf ((current_row[j] - min_z) / range[0] * NUMHUES))) * NUMSHADES;
                        }
                      else
                        {
                          c_index = (NV_INT32) (NUMHUES - (NV_INT32) (fabsf (current_row[j]) / range[1] * NUMHUES)) * NUMSHADES;
                        }
                    }
                  else
                    {
                      c_index = (NV_INT32) (NUMHUES - (NV_INT32) (fabsf ((current_row[j] - min_z) / range[0] * NUMHUES))) * NUMSHADES;
                    }

                  if (current_row[j] < -999998.0) c_index = -2; 

                  shade_factor = sunshade (next_row, current_row, j, &options.sunopts, x_cell_size, y_cell_size);

                  if (shade_factor < 0.0) shade_factor = options.sunopts.min_shade;

                  c_index -= NINT (NUMSHADES * shade_factor + 0.5);


                  if (fill[j] && c_index >= 0)
                    {
                      red[j] = options.color_array[c_index].red ();
                      green[j] = options.color_array[c_index].green ();
                      blue[j] = options.color_array[c_index].blue ();
                      alpha[j] = 255;
                    }
                  else
                    {
                      red[j] = green[j] = blue[j] = alpha[j] = 0;
                    }
                }

              err = bd[0]->RasterIO (GF_Write, 0, k, width, 1, red, width, 1, GDT_Byte, 0, 0);
              err = bd[1]->RasterIO (GF_Write, 0, k, width, 1, green, width, 1, GDT_Byte, 0, 0);
              err = bd[2]->RasterIO (GF_Write, 0, k, width, 1, blue, width, 1, GDT_Byte, 0, 0);
              if (options.transparent) err = bd[3]->RasterIO (GF_Write, 0, k, width, 1, alpha, width, 1, GDT_Byte, 0, 0);
            }

          if (err == CE_Failure)
            {
              checkList->clear ();

              string = QString (tr ("Failed a TIFF scanline write - row %1")).arg (i);
              checkList->addItem (string);
            }


          progress.gbar->setValue (k + 1);

          qApp->processEvents ();
        }


      checkList->addItem (" ");
      checkList->addItem (" ");

      string = QString (tr ("Created TIFF file %1")).arg (name[m]);
      checkList->addItem (string);

      string = QString (tr ("%1 rows by %2 columns")).arg (height).arg (width);
      QListWidgetItem *cur = new QListWidgetItem (string);
      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);


      if (options.surface == 3)  // COVERAGE
        {
          fclose (fp[m]);
          name[m][strlen (name[m]) - 3] = 't';
          name[m][strlen (name[m]) - 2] = 'm';
          name[m][strlen (name[m]) - 1] = 'p';
          remove (name[m]);
        }

      if (m != type_count - 1) progress.gbar->reset ();

      delete df;
    }


  free (fill);
  free (red);
  free (green);
  free (blue);
  free (alpha);
  free (next_row);
  free (current_row);
  free (current_record);


  close_pfm_file (pfm_handle);


  //  Contouring if requested.

  progress.cbar->setRange (0, 0);

  if (contour)
    {
      QString shapeName = QString (name[0]);
      shapeName.remove (".tif");

      NV_INT32 num_contours = scribe (width, height, mbr.min_x, mbr.min_y, min_z, max_z, ar, shapeName, &options, open_args);


      if (num_contours > 0)
        {
          string.sprintf (tr ("Generated %d contour segments").toAscii (), num_contours);
          QListWidgetItem *cur = new QListWidgetItem (string);


          checkList->addItem (cur);
          checkList->setCurrentItem (cur);
          checkList->scrollToItem (cur);
        }

      free (ar);
    }

  progress.cbar->setRange (0, 100);
  progress.cbar->setValue (100);
  qApp->processEvents ();


  button (QWizard::FinishButton)->setEnabled (TRUE);
  button (QWizard::CancelButton)->setEnabled (FALSE);


  QApplication::restoreOverrideCursor ();
  qApp->processEvents ();


  checkList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Conversion complete, press Finish to exit."));

  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);
}
