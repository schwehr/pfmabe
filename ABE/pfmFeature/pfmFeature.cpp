#include "pfmFeature.hpp"
#include "pfmFeatureHelp.hpp"

#include <getopt.h>

NV_FLOAT64 settings_version = 1.0;


pfmFeature::pfmFeature (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmFeatureWatermark.png"));


  //  Get the command line arguments.  The only one that we use now is the hidden option to output
  //  polygons with the features.

  NV_INT32 option_index = 0;
  options.output_polygons = NVFalse;

  while (NVTrue) 
    {
      static struct option long_options[] = {{"polygon", no_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "p", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              options.output_polygons = NVTrue;
              break;
            }
          break;

        case 'p':
          options.output_polygons = NVTrue;
          break;
        }
    }


  //  Get the user's defaults if available

  envin (&options);
  options.replace = NVTrue;


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, this, &options));

  setPage (1, oPage = new optionPage (this, &options));

  setPage (2, new runPage (this, &progress));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start selecting the features"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



pfmFeature::~pfmFeature ()
{
}



void pfmFeature::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  switch (id)
    {
    case 0:
      break;

    case 1:
      pfm_file = field ("pfm_file_edit").toString ();
      area_file = field ("area_file_edit").toString ();
      oPage->setFiles (pfm_file, area_file);
      break;

    case 2:
      button (QWizard::CustomButton1)->setEnabled (TRUE);

      feature_file = field ("output_file_edit").toString ();
      if (!feature_file.endsWith (".bfd")) feature_file.append (".bfd");


      //  Save the output directory.  It might have been input manually instead of browsed.

      options.output_dir = QFileInfo (feature_file).absoluteDir ().absolutePath ();


      ex_file = field ("ex_file_edit").toString ();

      options.order = field ("order").toInt ();
      options.zero = field ("zero").toBool ();
      options.offset = (NV_FLOAT32) field ("offset").toDouble ();
      options.hpc = field ("hpc").toBool ();
      options.description = field ("description").toString ();


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

      progress.list->clear ();

      string = tr ("Input PFM file : ") + pfm_file;
      progress.list->addItem (string);

      string = tr ("Feature file : ") + feature_file;
      progress.list->addItem (string);

      if (!area_file.isEmpty ())
        {
          string = tr ("Area file : ") + area_file;
          progress.list->addItem (string);
        }

      if (!ex_file.isEmpty ())
        {
          string = tr ("Exclusion area file : ") + ex_file;
          progress.list->addItem (string);
        }


      switch (options.order)
        {
        case 0:
          string = tr ("IHO special order");
          progress.list->addItem (string);
          break;

        case 1:
          string = tr ("IHO order 1");
          progress.list->addItem (string);
          break;
        }


      switch (options.zero)
        {
        case FALSE:
          string = tr ("Not selecting features above zero level");
          progress.list->addItem (string);
          break;

        case TRUE:
          string = tr ("Selecting features above zero level");
          progress.list->addItem (string);
          break;
        }

      switch (options.hpc)
        {
        case FALSE:
          string = tr ("Not determining confidence");
          progress.list->addItem (string);
          break;

        case TRUE:
          string = tr ("Determining confidence using HPC algorithm");
          progress.list->addItem (string);
          break;
        }


      if (options.output_polygons)
        {
          string = tr ("Writing estimated feature shape polygons to feature file");
          progress.list->addItem (string);
          break;
        }

      if (NINT (options.offset * 1000.0))
        {
          string.sprintf (tr ("Using artificial offset of %.2f").toAscii (), options.offset);
          progress.list->addItem (string);
          break;
        }

      break;
    }
}



void pfmFeature::cleanupPage (int id)
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



void pfmFeature::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
pfmFeature::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  selectThread        select_thread[SELECT_PASSES];
  NV_FLOAT64          lat, lon;
  static NV_INT32     old_percent = -1;
  QString             string;
  NV_F64_COORD2       order[2] = {{1.0, 0.05}, {2.0, 0.10}};


  NV_INT32 deconflict_feature (NV_F64_COORD2 order, NV_INT32 passes, NV_INT32 pfm_handle, OPTIONS *options, NV_INT32 *features_count,
                               FEATURE **features, RUN_PROGRESS *progress);
  void build_feature (NV_INT32 pfm_handle, PFM_OPEN_ARGS open_args, NV_INT32 iho_order, NV_INT32 passes, OPTIONS *options, QString feature_file,
                      NV_INT32 *features_count, FEATURE **features, RUN_PROGRESS *progress);



  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (FALSE);
  button (QWizard::BackButton)->setEnabled (FALSE);
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  strcpy (open_args.list_path, pfm_file.toAscii ());


  //  Open the PFM file.

  open_args.checkpoint = 0;
  if ((pfm_handle = open_existing_pfm_file (&open_args)) < 0) pfm_error_exit (pfm_error);


  features = (FEATURE **) calloc (SELECT_PASSES, sizeof (FEATURE *));

  if (features == NULL)
    {
      perror ("Allocating features array");
      exit (-1);
    }


  //  Check for an exclusion area file.

  options.ex_polygon_count = 0;
  if (!ex_file.isEmpty ())
    {
      NV_CHAR ef[512];
      strcpy (ef, ex_file.toAscii ());

      get_area_mbr (ef, &options.ex_polygon_count, options.ex_polygon_x, options.ex_polygon_y, &options.ex_mbr);
    }


  //  Set the bin size.

  NV_FLOAT64 start_size = 1.5 * pow (2.0, (NV_FLOAT64) options.order);


  //  Check for an area file.

  if (!area_file.isEmpty ())
    {
      NV_CHAR af[512];
      strcpy (af, area_file.toAscii ());

      get_area_mbr (af, &options.polygon_count, options.polygon_x, options.polygon_y, &options.mbr);


      //  Add one bin at largest bin size all around the MBR.

      NV_FLOAT64 size = start_size * pow (2.0, (NV_FLOAT64) (SELECT_PASSES - 1));


      newgp (options.mbr.min_y, options.mbr.min_x, 180.0, size, &lat, &lon);
      options.mbr.min_y = qMax (lat, open_args.head.mbr.min_y);

      newgp (options.mbr.max_y, options.mbr.min_x, 0.0, size, &lat, &lon);
      options.mbr.max_y = qMin (lat, open_args.head.mbr.max_y);


      //  If we're north of the equator then the south edge of the area will have the largest X bin size.

      if (open_args.head.mbr.min_y >= 0.0)
        {
          newgp (options.mbr.max_y, options.mbr.min_x, 270.0, size, &lat, &lon);
          options.mbr.min_x = qMax (lon, open_args.head.mbr.min_x);

          newgp (options.mbr.max_y, options.mbr.max_x, 90.0, size, &lat, &lon);
          options.mbr.max_x = qMin (lon, open_args.head.mbr.max_x);
        }
      else
        {
          newgp (options.mbr.min_y, options.mbr.min_x, 270.0, size, &lat, &lon);
          options.mbr.min_x = qMax (lon, open_args.head.mbr.min_x);

          newgp (options.mbr.min_y, options.mbr.max_x, 90.0, size, &lat, &lon);
          options.mbr.max_x = qMin (lon, open_args.head.mbr.max_x);
        }
    }
  else
    {
      options.polygon_count = 0;

      options.mbr = open_args.head.mbr;
    }


  //  We're starting all SELECT_PASSES selection passes concurrently using threads.  Note that we're using the Qt::DirectConnection type
  //  for the signal/slot connections.  This causes all of the signals emitted from the threads to be serviced immediately.

  qRegisterMetaType<NV_INT32> ("NV_INT32");

  complete = NVFalse;
  for (NV_INT32 i = 0 ; i < SELECT_PASSES ; i++)
    {
      pass_complete[i] = NVFalse;
      connect (&select_thread[i], SIGNAL (percentValue (NV_INT32, NV_INT32)), this, SLOT (slotPercentValue (NV_INT32, NV_INT32)), Qt::DirectConnection);
      connect (&select_thread[i], SIGNAL (completed (NV_INT32, NV_INT32)), this, SLOT (slotCompleted (NV_INT32, NV_INT32)), Qt::DirectConnection);
    }


  //  We open a different handle for each thread so that we don't have any collision problems in the PFM library.

  PFM_OPEN_ARGS l_open_args[SELECT_PASSES];

  for (NV_INT32 i = 0 ; i < SELECT_PASSES ; i++)
    {
      strcpy (l_open_args[i].list_path, pfm_file.toAscii ());


      //  Open the PFM file.

      l_open_args[i].checkpoint = 0;
      if ((l_pfm_handle[i] = open_existing_pfm_file (&l_open_args[i])) < 0) pfm_error_exit (pfm_error);


      select_thread[i].select (&options, l_pfm_handle[i], &l_open_args[i], features, i, start_size);
    }


  //  We can't exit from this method until the threads are complete but we want to keep our progress bar updated.  This is a bit tricky 
  //  because you can't update the progress bar from within the slots connected to the thread signals.  Those slots are considered part
  //  of the selection thread and not part of the GUI thread.  In the percent slot we just update the minimum percentage value (from
  //  any of the four threads) and monitor it here every second (updating the progress bar accordingly).  When all of the threads are
  //  finished we move on to the deconfliction and BFD file creation steps.

  while (!complete)
    {
#ifdef NVWIN3X
      Sleep (1000);
#else
      sleep (1);
#endif

      if (min_percent != old_percent)
        {
          progress.sbar->setValue (min_percent);
          old_percent = min_percent;
        }

      qApp->processEvents ();
    }


  progress.sbar->setValue (100);
  qApp->processEvents ();


  //  Compute our total possible features.

  NV_INT32 total = 0;
  for (NV_INT32 i = 0 ; i < SELECT_PASSES ; i++)
    {
      close_pfm_file (l_pfm_handle[i]);
      total += features_count[i];
    }


  progress.sbar->setRange (0, 100);
  progress.sbar->setValue (100);
  qApp->processEvents ();


  string.sprintf (tr ("Feature selection, detected %d possible features").toAscii (), total);
  progress.sbox->setTitle (string);
  qApp->processEvents ();


  //  Deconflict the possible features.

  NV_INT32 decon_count = deconflict_feature (order[options.order], SELECT_PASSES, pfm_handle, &options, features_count, features, &progress);


  string.sprintf (tr ("Feature deconfliction, deconflicted or removed %d features").toAscii (), decon_count);
  progress.dbox->setTitle (string);
  qApp->processEvents ();


  //  Write the valid features to a NAVO standard feature file.

  build_feature (pfm_handle, open_args, options.order, SELECT_PASSES, &options, feature_file, features_count, features, &progress);


  close_pfm_file (pfm_handle);


  string.sprintf (tr ("Writing feature file, wrote %d features").toAscii (), total - decon_count);
  progress.wbox->setTitle (string);
  qApp->processEvents ();


  button (QWizard::FinishButton)->setEnabled (TRUE);
  button (QWizard::CancelButton)->setEnabled (FALSE);


  QApplication::restoreOverrideCursor ();


  progress.list->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Feature selection and deconfliction complete, press Finish to exit."));

  progress.list->addItem (cur);
  progress.list->setCurrentItem (cur);
  progress.list->scrollToItem (cur);
}



void 
pfmFeature::slotPercentValue (NV_INT32 percent, NV_INT32 pass)
{
  static NV_INT32 save_percent[SELECT_PASSES];

  save_percent[pass] = percent;

  min_percent = 1001;
  for (NV_INT32 i = 0 ; i < SELECT_PASSES ; i++) min_percent = qMin (min_percent, save_percent[i]);
}



void 
pfmFeature::slotCompleted (NV_INT32 num_features, NV_INT32 pass)
{
  features_count[pass] = num_features;

  pass_complete[pass] = NVTrue;

  NV_BOOL done = NVTrue;
  for (NV_INT32 i = 0 ; i < SELECT_PASSES ; i++) if (!pass_complete[i]) done = NVFalse;


  if (done) complete = NVTrue;
}



//  Get the users defaults.

void pfmFeature::envin (OPTIONS *options)
{
  NV_FLOAT64 saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  options->order = 1;
  options->zero = NVFalse;
  options->hpc = NVFalse;
  options->description = "";
  options->input_dir = ".";
  options->output_dir = ".";
  options->area_dir = ".";
  options->exclude_dir = ".";
  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 640;
  options->window_height = 200;


  QSettings settings (tr ("navo.navy.mil"), tr ("pfmFeature"));

  settings.beginGroup (tr ("pfmFeature"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->order = settings.value (tr ("IHO order"), options->order).toInt ();
  options->zero = settings.value (tr ("zero level"), options->zero).toBool ();
  options->hpc = settings.value (tr ("HPC flag"), options->hpc).toBool ();
  options->input_dir = settings.value (tr ("input directory"), options->input_dir).toString ();
  options->output_dir = settings.value (tr ("output directory"), options->output_dir).toString ();
  options->area_dir = settings.value (tr ("area directory"), options->area_dir).toString ();
  options->exclude_dir = settings.value (tr ("exclude directory"), options->exclude_dir).toString ();

  options->window_width = settings.value (tr ("width"), options->window_width).toInt ();
  options->window_height = settings.value (tr ("height"), options->window_height).toInt ();
  options->window_x = settings.value (tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (tr ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void pfmFeature::envout (OPTIONS *options)
{
  QSettings settings (tr ("navo.navy.mil"), tr ("pfmFeature"));

  settings.beginGroup (tr ("pfmFeature"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("IHO order"), options->order);
  settings.setValue (tr ("zero level"), options->zero);
  settings.setValue (tr ("HPC flag"), options->hpc);
  settings.setValue (tr ("input directory"), options->input_dir);
  settings.setValue (tr ("output directory"), options->output_dir);
  settings.setValue (tr ("area directory"), options->area_dir);
  settings.setValue (tr ("exclude directory"), options->exclude_dir);

  settings.setValue (tr ("width"), options->window_width);
  settings.setValue (tr ("height"), options->window_height);
  settings.setValue (tr ("x position"), options->window_x);
  settings.setValue (tr ("y position"), options->window_y);

  settings.endGroup ();
}
