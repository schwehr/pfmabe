#include "pfmExtract.hpp"
#include "pfmExtractHelp.hpp"



pfmExtract::pfmExtract (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  void set_defaults (OPTIONS *options);
  void envin (OPTIONS *options);


  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmExtractWatermark.png"));


  set_defaults (&options);


  envin (&options);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, this, &options));
  setPage (1, new optionsPage (this, &options));
  setPage (2, new runPage (this, &progress, &extractList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start the extraction process"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



void pfmExtract::initializePage (int id)
{
  void envout (OPTIONS *options);


  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);

  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      button (QWizard::CustomButton1)->setEnabled (TRUE);


      options.format = field ("format").toInt ();
      options.ref = field ("reference").toBool ();
      options.geoid03 = field ("geoid").toBool ();
      options.chk = field ("checked").toBool ();
      options.flp = field ("flip").toBool ();
      options.utm = field ("utm").toBool ();
      options.lnd = field ("land").toBool ();
      options.unc = field ("uncert").toBool ();
      options.cutoff = field ("cutoff").toDouble ();
      options.datum_shift = field ("datumShift").toDouble ();
      options.size = field ("size").toInt ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      envout (&options);



      break;
    }
}



void pfmExtract::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;
    }
}



void 
pfmExtract::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
pfmExtract::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  FILE                *fp = NULL;
  NV_INT32            i, j, k, endian, recnum, rdp_data[3], pfm_handle, out_count = 0, file_count = 0, polygon_count = 0,
                      x_start, y_start, width, height, llz_hnd = -1, zone = 0;
  NV_U_INT32          max_file_size = 0, size = 0;
  NV_FLOAT32          total;
  BIN_RECORD          bin;
  DEPTH_RECORD        *depth;
  NV_CHAR             out_file[256], orig[256], areafile[512];
  NV_I32_COORD2       coord;
  PFM_OPEN_ARGS       open_args;
  NV_FLOAT64          polygon_x[200], polygon_y[200], central_meridian, x, y, z;
  NV_F64_XYMBR        mbr;
  LLZ_REC             llz_rec;
  LLZ_HEADER          llz_header;
  QString             string;
  SHPHandle           shp_hnd = NULL;
  SHPObject           *shape;



  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (FALSE);
  button (QWizard::BackButton)->setEnabled (FALSE);
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  QString pfm_file_name = field ("pfm_file_edit").toString ();
  QString output_file_name = field ("output_file_edit").toString ();
  QString area_file_name = field ("area_file_edit").toString ();

  switch (options.size)
    {
    case 0:
      max_file_size = 0;
      break;

    case 1:
      max_file_size = 1073676288;
      break;

    case 2:
      max_file_size = 2147418112;
      break;

    case 3:
      max_file_size = (NV_U_INT32) (pow (2.0, 32.0) - 65536.0);
      break;
    }


  strcpy (open_args.list_path, pfm_file_name.toAscii ());
  open_args.checkpoint = 0;
  if ((pfm_handle = open_existing_pfm_file (&open_args)) < 0) pfm_error_exit (pfm_error);


  mbr = open_args.head.mbr;
  strcpy (areafile, area_file_name.toAscii ());
  if (!area_file_name.isEmpty()) get_area_mbr (areafile, &polygon_count, polygon_x, polygon_y, &mbr);


  extractList->clear ();


  if (output_file_name.endsWith (".txt") || output_file_name.endsWith (".llz") ||
      output_file_name.endsWith (".rdp")) output_file_name.chop (4);

  strcpy (orig, output_file_name.toAscii ());

  switch (options.format)
    {
    case 0:
      if (options.size)
        {
          sprintf (out_file, "%s.%02d.txt", orig, file_count);
        }
      else
        {
          sprintf (out_file, "%s.txt", orig);
        }

      if ((fp = fopen64 (out_file, "w")) == NULL)
	{
          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Unable to open output file:") + QString (out_file) +
                                 tr ("\nReason:") + QString (strerror (errno)));
	  exit (-1);
	}
      break;

    case 1:
      if (options.size)
        {
          sprintf (out_file, "%s.%02d.llz", orig, file_count);
        }
      else
        {
          sprintf (out_file, "%s.llz", orig);
        }


      //  Boilerplate LLZ header.

      memset (&llz_header, 0, sizeof (LLZ_HEADER));
      sprintf (llz_header.comments, "Created from %s using %s", gen_basename (open_args.list_path), VERSION);
      llz_header.time_flag = NVFalse;
      llz_header.depth_units = 0;

      if ((llz_hnd = create_llz (out_file, llz_header)) < 0)
	{
          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Unable to open output file:") + QString (out_file) +
                                 tr ("\nReason:") + QString (strerror (errno)));
	  exit (-1);
	}
      break;

    case 2:
      if (options.size)
        {
          sprintf (out_file, "%s.%02d.rdp", orig, file_count);
        }
      else
        {
          sprintf (out_file, "%s.rdp", orig);
        }

      if ((fp = fopen64 (out_file, "w")) == NULL)
	{
          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Unable to open output file:") + QString (out_file) +
                                 tr ("\nReason:") + QString (strerror (errno)));
	  exit (-1);
	}
      break;

    case 3:
      if (options.size)
        {
          sprintf (out_file, "%s.%02d.shp", orig, file_count);
        }
      else
        {
          sprintf (out_file, "%s.shp", orig);
        }

      if ((shp_hnd = SHPCreate (out_file, SHPT_POINTZ)) == NULL)
        {
          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Unable to open output file:") + QString (out_file) +
                                 tr ("\nReason:") + QString (strerror (errno)));
          exit (-1);
        }
      break;
    }


  if (open_args.head.proj_data.projection)
    {
      QMessageBox::critical (this, tr ("pfmExtract"), tr ("Sorry, this program does not work with projected PFM structures."));
      exit (-1);
    }


  //  If we're doing UTM output, set the projection and get the zone.

  if (options.utm)
    {
      central_meridian = open_args.head.mbr.min_x + (open_args.head.mbr.max_x - open_args.head.mbr.min_x) / 2.0;

      NV_CHAR string[60];
      if (open_args.head.mbr.max_y < 0.0)
        {
          sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +south +lon_0=%.9f", central_meridian);
        }
      else
        {
          sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +lon_0=%.9f", central_meridian);
        }

      if (!(pj_utm = pj_init_plus (string)))
        {
          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Error initializing UTM projection\n"));
          exit (-1);
        }

      if (!(pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
        {
          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Error initializing latlon projection\n"));
          exit (-1);
        }

      zone = (NV_INT32) (31.0 + central_meridian / 6.0);
      if (zone >= 61) zone = 60;	
      if (zone <= 0) zone = 1;
    }


  extractList->addItem (tr ("FILE : ") + QString (out_file));


  endian = 0x00010203;
  if (options.format == 2) fwrite (&endian, sizeof (NV_INT32), 1, fp);


  //  If we're doing this by area there is no need to go through the entire file so we'll generate starts and ends based on
  //  the mbr.

  x_start = 0;
  y_start = 0;
  width = open_args.head.bin_width;
  height = open_args.head.bin_height;

  if (polygon_count)
    {
      if (mbr.min_y > open_args.head.mbr.max_y || mbr.max_y < open_args.head.mbr.min_y ||
          mbr.min_x > open_args.head.mbr.max_x || mbr.max_x < open_args.head.mbr.min_x)
        {
          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Specified area is completely outside of the PFM bounds!"));
          exit (-1);
        }


      //  Match to nearest cell.

      x_start = NINT ((mbr.min_x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees);
      y_start = NINT ((mbr.min_y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees);
      width = NINT ((mbr.max_x - mbr.min_x) / open_args.head.x_bin_size_degrees);
      height = NINT ((mbr.max_y - mbr.min_y) / open_args.head.y_bin_size_degrees);


      //  Adjust to PFM bounds if necessary.

      if (x_start < 0) x_start = 0;
      if (y_start < 0) y_start = 0;
      if (x_start + width > open_args.head.bin_width) width = open_args.head.bin_width - x_start;
      if (y_start + height > open_args.head.bin_height) height = open_args.head.bin_height - y_start;


      //  Redefine bounds.

      mbr.min_x = open_args.head.mbr.min_x + x_start * open_args.head.x_bin_size_degrees;
      mbr.min_y = open_args.head.mbr.min_y + y_start * open_args.head.y_bin_size_degrees;
      mbr.max_x = mbr.min_x + width * open_args.head.x_bin_size_degrees;
      mbr.max_y = mbr.min_y + height * open_args.head.y_bin_size_degrees;
    }


  total = open_args.head.bin_height;


  //  Loop over height & width of bins or area

  progress.ebar->setRange (0, height);

  NV_FLOAT32 value = 0.0;
  for (i = y_start ; i < y_start + height ; i++)
    {
      coord.y = i;
      for (j = x_start ; j < x_start + width ; j++)
        {
          coord.x = j;

          read_bin_record_index (pfm_handle, coord, &bin);

          if (!options.chk || (bin.validity & (PFM_CHECKED | PFM_VERIFIED)))
            {
              switch (options.source)
                {
                case 3:
                  if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                    {
                      for (k = 0 ; k < recnum ; k++)
                        {
                          if (!(depth[k].validity & (PFM_INVAL | PFM_DELETED)))
                            {
                              if (!polygon_count || inside (polygon_x, polygon_y, polygon_count, depth[k].xyz.x, depth[k].xyz.y))
                                {
                                  if (!(depth[k].validity & PFM_REFERENCE) || options.ref)
                                    {
                                      out_count++;

                                      value = depth[k].xyz.z;

                                      if (options.geoid03)
                                        {
                                          NV_FLOAT32 corr = get_geoid03 (depth[k].xyz.y, depth[k].xyz.x);

                                          value += corr;
                                        }

                                      if (options.cut && value > options.cutoff) value = options.cutoff;
                                      value += options.datum_shift;


                                      //  Check for exclude land option.

                                      NV_BOOL skip = NVFalse;
                                      if (options.lnd && value < 0.0) skip = NVTrue;

                                      if (!skip)
                                        {
                                          if (options.flp) value = -value;

                                          switch (options.format)
                                            {
                                            case 0:
                                              if (options.utm)
                                                {
                                                  x = depth[k].xyz.x * NV_DEG_TO_RAD;
                                                  y = depth[k].xyz.y * NV_DEG_TO_RAD;
                                                  pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                                  if (options.unc)
                                                    {
                                                      fprintf (fp, "%02d,%.2f,%.2f,%.2f,%.2f,%.2f\n", zone, x, y, value, depth[k].horizontal_error,
                                                               depth[k].vertical_error);
                                                    }
                                                  else
                                                    {
                                                      fprintf (fp, "%02d,%.2f,%.2f,%.2f\n", zone, x, y, value);
                                                    }
                                                }
                                              else
                                                {
                                                  if (options.unc)
                                                    {
                                                      fprintf (fp, "%.9f,%.9f,%.2f,%.2f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x, value, depth[k].horizontal_error,
                                                               depth[k].vertical_error);
                                                    }
                                                  else
                                                    {
                                                      fprintf (fp, "%.9f,%.9f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x, value);
                                                    }
                                                }
                                              break;

                                            case 1:
                                              llz_rec.xy.lat = depth[k].xyz.y;
                                              llz_rec.xy.lon = depth[k].xyz.x;
                                              llz_rec.depth = value;
                                              llz_rec.status = 0;

                                              append_llz (llz_hnd, llz_rec);
                                              break;

                                            case 2:
                                              rdp_data[0] = NINT (depth[k].xyz.y * 10000000.0);
                                              rdp_data[1] = NINT (depth[k].xyz.x * 10000000.0);
                                              rdp_data[2] = NINT (value * 10000.0);

                                              fwrite (rdp_data, sizeof (rdp_data), 1, fp);
                                              break;

                                            case 3:
                                              z = (NV_FLOAT64) value;
                                              shape = SHPCreateObject (SHPT_POINTZ, -1, 0, NULL, NULL, 1, &depth[k].xyz.x, &depth[k].xyz.y, &z, NULL);

                                              SHPWriteObject (shp_hnd, -1, shape);
                                              SHPDestroyObject (shape);
                                              break;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                      free (depth);
                    }
                  break;

                case 0:
                case 1:

                  if (bin.validity & PFM_DATA)
                    {
                      if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                        {
                          for (k = 0 ; k < recnum ; k++)
                            {
                              if (!(depth[k].validity & (PFM_INVAL | PFM_DELETED)))
                                {
                                  if (options.source == 0)
                                    {
                                      if (fabs ((NV_FLOAT64) (bin.min_filtered_depth - depth[k].xyz.z)) < 0.0005)
                                        {
                                          value = depth[k].xyz.z;

                                          if (options.geoid03)
                                            {
                                              NV_FLOAT32 corr = get_geoid03 (depth[k].xyz.y, depth[k].xyz.x);

                                              value += corr;
                                            }

                                          if (options.cut && value > options.cutoff) value = options.cutoff;
                                          value += options.datum_shift;


                                          //  Check for exclude land option.

                                          NV_BOOL skip = NVFalse;
                                          if (options.lnd && value < 0.0) skip = NVTrue;

                                          if (!skip)
                                            {
                                              if (options.flp) value = -value;

                                              switch (options.format)
                                                {
                                                case 0:
                                                  if (options.utm)
                                                    {
                                                      x = depth[k].xyz.x * NV_DEG_TO_RAD;
                                                      y = depth[k].xyz.y * NV_DEG_TO_RAD;
                                                      pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                                      if (options.unc)
                                                        {
                                                          fprintf (fp, "%02d,%.2f,%.2f,%.2f,%.2f\n", zone, x, y, value, bin.standard_dev);
                                                        }
                                                      else
                                                        {
                                                          fprintf (fp, "%02d,%.2f,%.2f,%.2f\n", zone, x, y, value);
                                                        }
                                                    }
                                                  else
                                                    {
                                                      if (options.unc)
                                                        {
                                                          fprintf (fp, "%.9f,%.9f,%.2f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x, value, bin.standard_dev);
                                                        }
                                                      else
                                                        {
                                                          fprintf (fp, "%.9f,%.9f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x, value);
                                                        }
                                                    }
                                                  break;

                                                case 1:
                                                  llz_rec.xy.lat = depth[k].xyz.y;
                                                  llz_rec.xy.lon = depth[k].xyz.x;
                                                  llz_rec.depth = value;
                                                  llz_rec.status = 0;

                                                  append_llz (llz_hnd, llz_rec);
                                                  break;

                                                case 2:
                                                  rdp_data[0] = NINT (depth[k].xyz.y * 10000000.0);
                                                  rdp_data[1] = NINT (depth[k].xyz.x * 10000000.0);
                                                  rdp_data[2] = NINT (value);

                                                  fwrite (rdp_data, sizeof (rdp_data), 1, fp);
                                                  break;

                                                case 3:
                                                  z = (NV_FLOAT64) value;
                                                  shape = SHPCreateObject (SHPT_POINTZ, -1, 0, NULL, NULL, 1, &depth[k].xyz.x, &depth[k].xyz.y, &z, NULL);

                                                  SHPWriteObject (shp_hnd, -1, shape);
                                                  SHPDestroyObject (shape);
                                                  break;
                                                }

                                              out_count++;
                                            }
                                          break;
                                        }
                                    }
                                  else
                                    {
                                      if (fabs ((NV_FLOAT64) (bin.max_filtered_depth - depth[k].xyz.z)) < 0.005)
                                        {
                                          value = depth[k].xyz.z;

                                          if (options.geoid03)
                                            {
                                              NV_FLOAT32 corr = get_geoid03 (depth[k].xyz.y, depth[k].xyz.x);

                                              value += corr;
                                            }

                                          if (options.cut && value > options.cutoff) value = options.cutoff;
                                          value += options.datum_shift;


                                          //  Check for exclude land option.

                                          NV_BOOL skip = NVFalse;
                                          if (options.lnd && value < 0.0) skip = NVTrue;

                                          if (!skip)
                                            {
                                              if (options.flp) value = -value;

                                              switch (options.format)
                                                {
                                                case 0:
                                                  if (options.utm)
                                                    {
                                                      x = depth[k].xyz.x * NV_DEG_TO_RAD;
                                                      y = depth[k].xyz.y * NV_DEG_TO_RAD;
                                                      pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                                      if (options.unc)
                                                        {
                                                          fprintf (fp, "%02d,%.2f,%.2f,%.2f,%.2f\n", zone, x, y, value, bin.standard_dev);
                                                        }
                                                      else
                                                        {
                                                          fprintf (fp, "%02d,%.2f,%.2f,%.2f\n", zone, x, y, value);
                                                        }
                                                    }
                                                  else
                                                    {
                                                      if (options.unc)
                                                        {
                                                          fprintf (fp, "%.9f,%.9f,%.2f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x, value, bin.standard_dev);
                                                        }
                                                      else
                                                        {
                                                          fprintf (fp, "%.9f,%.9f,%.2f\n", depth[k].xyz.y, depth[k].xyz.x, value);
                                                        }
                                                    }
                                                  break;

                                                case 1:
                                                  llz_rec.xy.lat = depth[k].xyz.y;
                                                  llz_rec.xy.lon = depth[k].xyz.x;
                                                  llz_rec.depth = value;
                                                  llz_rec.status = 0;

                                                  append_llz (llz_hnd, llz_rec);
                                                  break;

                                                case 2:
                                                  rdp_data[0] = NINT (depth[k].xyz.y * 10000000.0);
                                                  rdp_data[1] = NINT (depth[k].xyz.x * 10000000.0);
                                                  rdp_data[2] = NINT (value * 10000.0);

                                                  fwrite (rdp_data, sizeof (rdp_data), 1, fp);
                                                  break;

                                                case 3:
                                                  z = (NV_FLOAT64) value;
                                                  shape = SHPCreateObject (SHPT_POINTZ, -1, 0, NULL, NULL, 1, &depth[k].xyz.x, &depth[k].xyz.y, &z, NULL);

                                                  SHPWriteObject (shp_hnd, -1, shape);
                                                  SHPDestroyObject (shape);
                                                  break;
                                                }

                                              out_count++;
                                            }
                                          break;
                                        }
                                    }
                                }
                            }
                        }
                      free (depth);
                    }
                  break;


                case 2:

                  if (bin.validity & (PFM_DATA | PFM_INTERPOLATED))
                    {
                      value = bin.avg_filtered_depth;

                      if (options.geoid03)
                        {
                          NV_FLOAT32 corr = get_geoid03 (bin.xy.y, bin.xy.x);

                          value += corr;
                        }

                      if (options.cut && value > options.cutoff) value = options.cutoff;
                      value += options.datum_shift;


                      //  Check for exclude land option.

                      NV_BOOL skip = NVFalse;
                      if (options.lnd && value < 0.0) skip = NVTrue;

                      if (!skip)
                        {
                          if (options.flp) value = -value;

                          switch (options.format)
                            {
                            case 0:
                              if (options.utm)
                                {
                                  x = bin.xy.x * NV_DEG_TO_RAD;
                                  y = bin.xy.y * NV_DEG_TO_RAD;
                                  pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                  if (options.unc)
                                    {
                                      fprintf (fp, "%02d,%.2f,%.2f,%.2f,%.2f\n", zone, x, y, value, bin.standard_dev);
                                    }
                                  else
                                    {
                                      fprintf (fp, "%02d,%.2f,%.2f,%.2f\n", zone, x, y, value);
                                    }
                                }
                              else
                                {
                                  if (options.unc)
                                    {
                                      fprintf (fp, "%.9f,%.9f,%.2f,%.2f\n", bin.xy.y, bin.xy.x, value, bin.standard_dev);
                                    }
                                  else
                                    {
                                      fprintf (fp, "%.9f,%.9f,%.2f\n", bin.xy.y, bin.xy.x, value);
                                    }
                                }
                              break;

                            case 1:
                              llz_rec.xy.lat = bin.xy.y;
                              llz_rec.xy.lon = bin.xy.x;
                              llz_rec.depth = value;
                              llz_rec.status = 0;

                              append_llz (llz_hnd, llz_rec);
                              break;

                            case 2:
                              rdp_data[0] = NINT (bin.xy.y * 10000000.0);
                              rdp_data[1] = NINT (bin.xy.x * 10000000.0);
                              rdp_data[2] = NINT (value * 10000.0);

                              fwrite (rdp_data, sizeof (rdp_data), 1, fp);
                              break;

                            case 3:
                              z = (NV_FLOAT64) value;
                              shape = SHPCreateObject (SHPT_POINTZ, -1, 0, NULL, NULL, 1, &bin.xy.x, &bin.xy.y, &z, NULL);

                              SHPWriteObject (shp_hnd, -1, shape);
                              SHPDestroyObject (shape);
                              break;
                            }

                          out_count++;
                        }
                    }
                  break;
                }
            }


          //  Make sure file size does not exceed 2GB.

          if (options.size)
            {
              size = QFileInfo (out_file).size ();

              if (size >= max_file_size)
                {
                  file_count++;


                  switch (options.format)
                    {
                    case 0:
                      fclose (fp);
                      sprintf (out_file, "%s.%02d.txt", orig, file_count);

                      if ((fp = fopen64 (out_file, "w")) == NULL)
                        {
                          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Unable to open output file:") + QString (out_file) +
                                                 tr ("\nReason:") + QString (strerror (errno)));
                          exit (-1);
                        }
                      break;

                    case 1:
                      close_llz (llz_hnd);
                      sprintf (out_file, "%s.%02d.llz", orig, file_count);


                      //  Boilerplate LLZ header.

                      sprintf (llz_header.comments, "Created from %s using %s", gen_basename (open_args.list_path), VERSION);
                      llz_header.time_flag = NVFalse;
                      llz_header.depth_units = 0;

                      if ((llz_hnd = create_llz (out_file, llz_header)) < 0)
                        {
                          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Unable to open output file:") + QString (out_file) +
                                                 tr ("\nReason:") + QString (strerror (errno)));
                          exit (-1);
                        }  
                      break;

                    case 2:
                      fclose (fp);
                      sprintf (out_file, "%s.%02d.rdp", orig, file_count);

                      if ((fp = fopen64 (out_file, "w")) == NULL)
                        {
                          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Unable to open output file:") + QString (out_file) +
                                                 tr ("\nReason:") + QString (strerror (errno)));
                          exit (-1);
                        }

                      fwrite (&endian, sizeof (NV_INT32), 1, fp);
                      break;

                    case 3:
                      SHPClose (shp_hnd);

                      sprintf (out_file, "%s.%02d.shp", orig, file_count);

                      if ((shp_hnd = SHPCreate (out_file, SHPT_POINTZ)) == NULL)
                        {
                          QMessageBox::critical (this, tr ("pfmExtract"), tr ("Unable to open output file:") + QString (out_file) +
                                                 tr ("\nReason:") + QString (strerror (errno)));
                          exit (-1);
                        }
                      break;
                    }


                  extractList->addItem (tr ("FILE : ") + QString (out_file));
                }
            }
        }


      progress.ebar->setValue (i - y_start);
      qApp->processEvents ();
    }

  progress.ebar->setValue (height);
  qApp->processEvents ();


  switch (options.format)
    {
    case 0:
      string.sprintf (tr ("%d ASCII records output").toAscii (), out_count);
      extractList->addItem (string);
      fclose (fp);
      break;

    case 1:
      string.sprintf (tr ("%d LLZ records output").toAscii (), out_count);
      extractList->addItem (string);
      close_llz (llz_hnd);
      break;

    case 2:
      string.sprintf (tr ("%d RDP records output").toAscii (), out_count);
      extractList->addItem (string);
      fclose (fp);
      break;

    case 3:
      string.sprintf (tr ("%d SHP records output").toAscii (), out_count);
      extractList->addItem (string);
      SHPClose (shp_hnd);
      break;
    }

  qApp->processEvents ();


  close_pfm_file (pfm_handle);


  button (QWizard::FinishButton)->setEnabled (TRUE);
  button (QWizard::CancelButton)->setEnabled (FALSE);


  QApplication::restoreOverrideCursor ();


  extractList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Extraction complete, press Finish to exit."));

  extractList->addItem (cur);
  extractList->setCurrentItem (cur);
  extractList->scrollToItem (cur);
}
