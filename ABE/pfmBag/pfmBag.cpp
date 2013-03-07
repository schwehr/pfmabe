#include "pfmBag.hpp"
#include "pfmBagHelp.hpp"


NV_FLOAT64 settings_version = 1.0;


pfmBag::pfmBag (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  options.has_z0 = NVFalse;
  options.has_z1 = NVFalse;


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmBagWatermark.png"));


  /*  Override the HDF5 version check so that we can read BAGs created with an older version of HDF5.  */

  putenv ((NV_CHAR *) "HDF5_DISABLE_VERSION_CHECK=2");


  //  Read in the datum information.  This is really just here for possible future use.  Right now we're hard-wired to WGS84/WGS84E

  options.h_datum_count = 0;
  options.v_datum_count = 0;

  NV_CHAR string[256];
  QString qstring;


  QFile *hDataFile = new QFile (":/icons/horizontal_datums.txt");

  if (hDataFile->open (QIODevice::ReadOnly))
    {
      while (hDataFile->readLine (string, sizeof (string)) > 0)
        {
          qstring = QString (string);
          options.h_datums[options.h_datum_count].active = (NV_BOOL) (qstring.section (':', 0, 0).toInt ());
          options.h_datums[options.h_datum_count].abbrev = qstring.section (':', 1, 1);
          options.h_datums[options.h_datum_count].name = qstring.section (':', 2, 2);

          options.h_datum_count++;
        }
      hDataFile->close ();
    }
  else
    {
      fprintf (stderr, "Can't open the horizontal datum file");
      exit (-1);
    }


  QFile *vDataFile = new QFile (":/icons/vertical_datums.txt");

  if (vDataFile->open (QIODevice::ReadOnly))
    {
      while (vDataFile->readLine (string, sizeof (string)) > 0)
        {
          qstring = QString (string);
          options.v_datums[options.v_datum_count].active = (NV_BOOL) (qstring.section (':', 0, 0).toInt ());
          options.v_datums[options.v_datum_count].abbrev = qstring.section (':', 1, 1);
          options.v_datums[options.v_datum_count].name = qstring.section (':', 2, 2);

          options.v_datum_count++;
        }
      vDataFile->close ();
    }
  else
    {
      fprintf (stderr, "Can't open the vertical datum file");
      exit (-1);
    }


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


  setPage (0, new startPage (argc, argv, this, &options));

  setPage (1, sPage = new surfacePage (this, &options));

  setPage (2, new datumPage (this, &options));

  setPage (3, new classPage (this, &options));

  setPage (4, new runPage (this, &progress, &checkList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start generating the BAG"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);


  if (getenv ("BAG_HOME") == NULL)
    {
      QMessageBox::information (this, tr ("pfmBag Error"), tr ("BAG_HOME environment variable is not set.\n") +
                                tr ("This must point to the configdata directory or pfmBag will fail."), QMessageBox::Ok,  QMessageBox::Ok);

      exit (-1);
    }
}



pfmBag::~pfmBag ()
{
}



void pfmBag::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  switch (id)
    {
    case 0:    //  Start page

      break;

    case 1:    //  Surface page

      options.pfm_file_name = field ("pfm_file_edit").toString ();
      sPage->setFields (&options);
      break;

    case 2:    //  Datum page

      break;

    case 3:    //  Classification page

      break;

    case 4:    //  Run page

      button (QWizard::CustomButton1)->setEnabled (TRUE);

      area_file_name = field ("area_file_edit").toString ();
      sep_file_name = field ("sep_file_edit").toString ();
      output_file_name = field ("output_file_edit").toString ();


      //  Save the output directory.  It might have been input manually instead of browsed.

      options.output_dir = QFileInfo (output_file_name).absoluteDir ().absolutePath ();

      options.uncertainty = field ("uncertainty").toInt ();

      options.units = field ("units").toInt ();
      options.depth_cor = field ("depthCor").toInt ();
      options.projection = field ("projection").toInt ();

      options.group = field ("group").toString ();
      options.source = field ("source").toString ();
      options.version = field ("version").toString ();

      options.classification = field ("classification").toInt ();
      options.authority = field ("authority").toInt ();
      options.declassDate = field ("declassDate").toDate ();
      options.compDate = field ("compDate").toDate ();

      options.distStatement = field ("distStatement").toString ();

      options.title = field ("title").toString ();
      options.pi_name = field ("individualName").toString ();
      options.pi_title = field ("positionName").toString ();
      options.poc_name = field ("individualName2").toString ();
      options.abstract = field ("abstract").toString ();


      //  Can't make an enhanced surface using minutes.

      if (options.mbin_size == 0.0) options.enhanced = NVFalse;


      if (!options.enhanced)
        {
          progress.wbar->hide ();
          progress.wbox->hide ();
        }
      else
        {
          progress.wbar->show ();
          progress.wbox->show ();
        }


      //  Got to have a title.

      if (options.title.isEmpty ()) options.title = QFileInfo (output_file_name).baseName ();


      //  Got to have an abstract.

      if (options.abstract.isEmpty ()) options.abstract = QFileInfo (output_file_name).baseName ();


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

      string = tr ("Input PFM file : ") + options.pfm_file_name;
      checkList->addItem (string);

      string = tr ("Output file(s) : ") + output_file_name;
      checkList->addItem (string);

      if (!area_file_name.isEmpty ())
        {
          string = tr ("Area file : ") + area_file_name;
          checkList->addItem (string);
        }

      if (!sep_file_name.isEmpty ())
        {
          string = tr ("Separation file : ") + sep_file_name;
          checkList->addItem (string);
        }

      switch (options.surface)
        {
        case MIN_SURFACE:
          string = tr ("Minimum Filtered Surface");
          checkList->addItem (string);
          break;

        case MAX_SURFACE:
          string = tr ("Maximum Filtered Surface");
          checkList->addItem (string);
          break;

        case AVG_SURFACE: 
          string = tr ("Average Filtered Surface");
          checkList->addItem (string);
          break;

        case CUBE_SURFACE: 
          string = tr ("CUBE Surface");
          checkList->addItem (string);
          break;
        }


      switch (options.units)
        {
        case 0:
          string = tr ("Units : meters");
          checkList->addItem (string);
          break;

        case 1:
          string = tr ("Units : feet");
          checkList->addItem (string);
          break;

        case 2:
          string = tr ("Units : fathoms");
          checkList->addItem (string);
          break;
        }


      switch (options.depth_cor)
        {
        case 0:
          string = tr ("Using corrected depth");
          checkList->addItem (string);
          break;

        case 1:
          string = tr ("Using uncorrected Depth @ 1500 m/s");
          checkList->addItem (string);
          break;

        case 2:
          string = tr ("Using uncorrected Depth @ 4800 ft/s");
          checkList->addItem (string);
          break;

        case 3:
          string = tr ("Using uncorrected Depth @ 800 fm/s");
          checkList->addItem (string);
          break;

        case 4:
          string = tr ("Using mixed corrections");
          checkList->addItem (string);
          break;
        }


      string = tr ("Horizontal datum : ") + options.h_datums[options.h_datum].name;
      checkList->addItem (string);

      string = tr ("Vertical datum : ") + options.v_datums[options.v_datum].name;
      checkList->addItem (string);


      switch (options.projection)
        {
        case 0:
          string = tr ("Geodetic (no projection)");
          break;

        case 1:
          string = tr ("UTM projection");
          break;
        }
      checkList->addItem (string);

      switch (options.uncertainty)
        {
        case 0:
          string = tr ("No uncertainty values");
          break;

        case 1:
          string = tr ("Using Standard Deviation for uncertainty values");
          break;

        case 2:
          string = tr ("Using Max of CUBE Standard Deviation and average TPE for uncertainty values");
          break;

        case 3:
          string = tr ("Using TPE for uncertainty values");
          break;
        }
      checkList->addItem (string);


      if (options.enhanced)
        {
          string = tr ("Using feature points to create enhanced navigation surface");
          checkList->addItem (string);
        }


      string = tr ("Group name : ") + options.group;
      checkList->addItem (string);

      string = tr ("Data source : ") + options.source;
      checkList->addItem (string);

      string = tr ("Data version : ") + options.version;
      checkList->addItem (string);

      if (options.authority)
        {
          switch (options.classification)
            {
            case 0:
              string = tr ("Classification : Unclassified");
              checkList->addItem (string);
              break;

            case 1:
              string = tr ("Classification : Confidential");
              checkList->addItem (string);
              break;

            case 2:
              string = tr ("Classification : Secret");
              checkList->addItem (string);
              break;

            case 3:
              string = tr ("Classification : Top Secret");
              checkList->addItem (string);
              break;
            }

          switch (options.authority)
            {
            case 0:
              string = tr ("Classifying Authority : N/A");
              checkList->addItem (string);
              break;

            case 1:
              string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(23)");
              checkList->addItem (string);
              break;

            case 2:
              string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(24)");
              checkList->addItem (string);
              break;

            case 3:
              string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(27)");
              checkList->addItem (string);
              break;

            case 4:
              string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(28)");
              checkList->addItem (string);
              break;
            }


          string = tr ("Declassification date : ") + options.declassDate.toString ("yyyy-MM-dd");
          checkList->addItem (string);


          string = tr ("Distribution statement : ") + options.distStatement;
          checkList->addItem (string);
        }


      string = tr ("Extract/Compile date : ") + options.compDate.toString ("yyyy-MM-dd");
      checkList->addItem (string);

      string = tr ("BAG Title : ") + options.title;
      checkList->addItem (string);

      string = tr ("Certifying official's name : ") + options.pi_name;
      checkList->addItem (string);

      string = tr ("Certifying official's position : ") + options.pi_title;
      checkList->addItem (string);

      string = tr ("POC name : ") + options.poc_name;
      checkList->addItem (string);

      string = tr ("BAG comments : ") + options.abstract;
      checkList->addItem (string);

      break;
    }
}



void pfmBag::cleanupPage (int id)
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



void pfmBag::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
pfmBag::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  PFM_OPEN_ARGS                open_args;  
  QString                      string;
  NV_INT32                     pfm_handle, sep_handle = -1, width, height;
  FILE                         *sep_fp = NULL;
  NV_CHAR                      area_file[512], sep_file[512], sep_string[128];
  u8                           name[512];
  NV_U_BYTE                    **weight = NULL;
  NV_F64_XYMBR                 proj_mbr = {0.0, 0.0, 0.0, 0.0};
  CHRTR2_HEADER                sep_header;
  CHRTR2_RECORD                sep_record;

  IDENTIFICATION_INFO          identification_info;
  MD_LEGAL_CONSTRAINTS         md_legal_constraints;
  MD_SECURITY_CONSTRAINTS      md_security_constraints;
  DATA_QUALITY_INFO            data_quality_info;
  SPATIAL_REPRESENTATION_INFO  spatial_representation_info;
  REFERENCE_SYSTEM_INFO        reference_system_info;
  RESPONSIBLE_PARTY            contact;
  SOURCE_INFO                  source;


  InitDataIdentificationInfo (&identification_info);
  InitLegalConstraints (&md_legal_constraints);
  InitSecurityConstraints (&md_security_constraints);
  InitDataQualityInfo (&data_quality_info);
  InitReferenceSystemInfo (&reference_system_info);
  InitSpatialRepresentationInfo (&spatial_representation_info);
  InitResponsibleParty (&contact);


  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (FALSE);
  button (QWizard::BackButton)->setEnabled (FALSE);
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  //  Move the data into the BAG metadata structures.

  strcpy (open_args.list_path, options.pfm_file_name.toAscii ());


  open_args.checkpoint = 0;
  pfm_handle = open_existing_pfm_file (&open_args);


  if (pfm_handle < 0) pfm_error_exit (pfm_error);


  strcpy (identification_info.title, options.title.toAscii ());

  QDate current_date = QDate::currentDate ();
  string = current_date.toString ("yyyy-MM-dd");
  strcpy (identification_info.date, string.toAscii ());

  strcpy (identification_info.dateType, tr ("publication").toAscii ());
  strcpy (identification_info.responsibleParties[0].individualName, options.pi_name.toAscii ());
  strcpy (identification_info.responsibleParties[0].positionName, options.pi_title.toAscii ());
  strcpy (identification_info.responsibleParties[0].organisationName, tr ("Naval Oceanographic Office").toAscii ());
  strcpy (identification_info.responsibleParties[0].role, tr ("Principal investigator").toAscii ());
  strcpy (identification_info.abstract, options.abstract.toAscii ());
  strcpy (identification_info.purpose, "Not for navigation");
  strcpy (identification_info.status, "");
  strcpy (identification_info.language, "en");

  spatial_representation_info.numberOfDimensions = 2;
  strcpy (spatial_representation_info.dimensionName[0], tr ("row").toAscii ());
  strcpy (spatial_representation_info.dimensionName[1], tr ("column").toAscii ());
  strcpy (spatial_representation_info.dimensionName[2], tr ("").toAscii ());


  NV_F64_XYMBR mbr = open_args.head.mbr;


  if (!area_file_name.isEmpty ())
    {
      NV_FLOAT64 polygon_x[200], polygon_y[200];
      NV_INT32 polygon_count;

      strcpy (area_file, area_file_name.toAscii ());
      get_area_mbr (area_file, &polygon_count, polygon_x, polygon_y, &mbr);


      if (mbr.min_y > open_args.head.mbr.max_y || mbr.max_y < open_args.head.mbr.min_y ||
          mbr.min_x > open_args.head.mbr.max_x || mbr.max_x < open_args.head.mbr.min_x)
        {
          fprintf (stderr, tr ("\n\nSpecified area is completely outside of the PFM bounds!\n\n").toAscii ());
          exit (-1);
        }
    }


  //  If mbin_size is 0.0 then we're defining bin sizes in minutes of lat/lon

  NV_FLOAT64 x_bin_size_degrees = 0.0;
  NV_FLOAT64 y_bin_size_degrees = 0.0;

  if (options.mbin_size == 0.0)
    {
      y_bin_size_degrees = options.gbin_size / 60.0;


      //  We're going to use approximately spatially equivalent geographic bin sizes north or 64N and south of 64S.
      //  Otherwise we're going to use equal lat and lon bin sizes.

      if (mbr.min_y >= 64.0 || mbr.max_y <= -64.0)
        {
          NV_FLOAT64 dist, az, y, x;
          if (mbr.min_y <= -64.0)
            {
              invgp (NV_A0, NV_B0, mbr.max_y, mbr.min_x, mbr.max_y - (options.gbin_size / 60.0), mbr.min_x, &dist, &az);
            }
          else
            {
              invgp (NV_A0, NV_B0, mbr.min_y, mbr.min_x, mbr.min_y + (options.gbin_size / 60.0), mbr.min_x, &dist, &az);
            }

          newgp (mbr.min_y, mbr.min_x, 90.0, dist, &y, &x);

          x_bin_size_degrees = x - mbr.min_x;
        }
      else
        {
          x_bin_size_degrees = y_bin_size_degrees;
        }
    }
  else
    {
      NV_F64_COORD2 central, xy;

      central.x = (mbr.max_x - mbr.min_x) / 2.0;
      central.y = (mbr.max_y - mbr.min_y) / 2.0;


      //  Convert from meters.

      newgp (central.y, central.x, 90.0, options.mbin_size, &xy.y, &xy.x);


      //  Check if the longitude is in the form 0 to 360.

      if (central.x > 180) xy.x = xy.x + 360;


      x_bin_size_degrees = xy.x - central.x;
      newgp (central.y, central.x, 0.0, options.mbin_size, &xy.y, &xy.x);
      y_bin_size_degrees = xy.y - central.y;
    }


  height = NINT ((mbr.max_y - mbr.min_y) / y_bin_size_degrees + 0.05);
  width = NINT ((mbr.max_x - mbr.min_x) / x_bin_size_degrees + 0.05);


  //  Redefine upper and right bounds

  mbr.max_x = mbr.min_x + width * x_bin_size_degrees;
  mbr.max_y = mbr.min_y + height * y_bin_size_degrees;


  strcpy (spatial_representation_info.cellGeometry, "grid");
  strcpy (spatial_representation_info.transformationParameterAvailability, "false");
  strcpy (spatial_representation_info.checkPointAvailability, "0");


  switch (options.depth_cor)
    {
    case 0:
      strcpy (identification_info.depthCorrectionType, "Corrected depth");
      break;

    case 1:
      strcpy (identification_info.depthCorrectionType, "Uncorrected 1500 m/s");
      break;

    case 2:
      strcpy (identification_info.depthCorrectionType, "Uncorrected 4800 ft/s");
      break;

    case 3:
      strcpy (identification_info.depthCorrectionType, "Uncorrected 800 fm/s");
      break;

    case 4:
      strcpy (identification_info.depthCorrectionType, "Mixed corrections");
      break;
    }


  switch (options.uncertainty)
    {
    case 0:
      strcpy (identification_info.verticalUncertaintyType, tr ("None").toAscii ());
      break;

    case 1:
      strcpy (identification_info.verticalUncertaintyType, tr ("Std Dev").toAscii ());
      break;

    case 2:
      strcpy (identification_info.verticalUncertaintyType, tr ("Final uncertainty").toAscii ());
      break;

    case 3:
      strcpy (identification_info.verticalUncertaintyType, tr ("TPE").toAscii ());
      break;
    }

  strcpy (md_legal_constraints.useConstraints, tr ("otherRestrictions").toAscii ());
  strcpy (md_legal_constraints.otherConstraints, tr ("Not for Navigation").toAscii ());

  switch (options.classification)
    {
    case 0:
      strcpy (md_security_constraints.classification, tr ("Unclassified").toAscii ());
      break;

    case 1:
      strcpy (md_security_constraints.classification, tr ("Confidential").toAscii ());
      break;

    case 2:
      strcpy (md_security_constraints.classification, tr ("Secret").toAscii ());
      break;

    case 3:
      strcpy (md_security_constraints.classification, tr ("Top Secret").toAscii ());
      break;
    }

  switch (options.authority)
    {
    case 0:
      string = tr ("Classifying Authority : N/A");
      break;

    case 1:
      string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(23)\n");
      break;

    case 2:
      string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(24)\n");
      break;

    case 3:
      string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(27)\n");
      break;

    case 4:
      string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(28)\n");
      break;
    }

  string += tr ("Declassification date : ") + options.declassDate.toString ("yyyy-MM-dd") + "\n";

  string += tr ("Distribution statement : ") + options.distStatement;

  strcpy (md_security_constraints.userNote, string.toAscii ());


  strcpy (data_quality_info.scope, tr ("dataset").toAscii ());

  NV_FLOAT64 half_x = 0.0, half_y = 0.0;


  //  If we're doing UTM output, set the projection and get the zone.

  if (options.projection)
    {
      strcpy (reference_system_info.projection, tr ("UTM").toAscii ());
      

      //  From Webb McDonald at SAIC Newport (I have no idea why).

      reference_system_info.scaleFactorAtEquator = 0.9996;


      reference_system_info.falseNorthing = 0.0;
      reference_system_info.falseEasting = 0.0;

      reference_system_info.latitudeOfProjectionCenter = mbr.min_y + (mbr.max_y - mbr.min_y) / 2.0;
      reference_system_info.longitudeOfCentralMeridian = mbr.min_x + (mbr.max_x - mbr.min_x) / 2.0;


      //  Set up the UTM projection.

      NV_CHAR string[60];
      if (mbr.max_y < 0.0)
        {
          sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +south +lon_0=%.9f", reference_system_info.longitudeOfCentralMeridian);
        }
      else
        {
          sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +lon_0=%.9f", reference_system_info.longitudeOfCentralMeridian);
        }

      if (!(pj_utm = pj_init_plus (string)))
        {
          QMessageBox::critical (this, tr ("pfmBag"), tr ("Error initializing UTM projection\n"));
          exit (-1);
        }

      if (!(pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
        {
          QMessageBox::critical (this, tr ("pfmBag"), tr ("Error initializing latlon projection\n"));
          exit (-1);
        }


      //  Compute the zone.

      reference_system_info.zone = (NV_INT32) (31.0 + reference_system_info.longitudeOfCentralMeridian / 6.0);
      if (reference_system_info.zone >= 61) reference_system_info.zone = 60;	
      if (reference_system_info.zone <= 0) reference_system_info.zone = 1;


      //  Get the min and max northings and eastings.  We're still going to need the actual lat/lon MBR so we store these in proj_mbr.
      //  Note that we're trying to get the largest extents available.  This is mostly due to distortion in longitude.

      NV_FLOAT64 llx, ulx, lrx, urx, lly, uly, lry, ury;

      llx = mbr.min_x * NV_DEG_TO_RAD;
      lly = mbr.min_y * NV_DEG_TO_RAD;
      pj_transform (pj_latlon, pj_utm, 1, 1, &llx, &lly, NULL);

      ulx = mbr.min_x * NV_DEG_TO_RAD;
      uly = mbr.max_y * NV_DEG_TO_RAD;
      pj_transform (pj_latlon, pj_utm, 1, 1, &ulx, &uly, NULL);

      lrx = mbr.max_x * NV_DEG_TO_RAD;
      lry = mbr.min_y * NV_DEG_TO_RAD;
      pj_transform (pj_latlon, pj_utm, 1, 1, &lrx, &lry, NULL);

      urx = mbr.max_x * NV_DEG_TO_RAD;
      ury = mbr.max_y * NV_DEG_TO_RAD;
      pj_transform (pj_latlon, pj_utm, 1, 1, &urx, &ury, NULL);

      if (llx < lrx)
        {
          proj_mbr.min_x = llx;
        }
      else
        {
          proj_mbr.min_x = lrx;
        }

      if (lrx > urx)
        {
          proj_mbr.max_x = lrx;
        }
      else
        {
          proj_mbr.max_x = urx;
        }

      if (lly < lry)
        {
          proj_mbr.min_y = lly;
        }
      else
        {
          proj_mbr.min_y = lry;
        }

      if (uly > ury)
        {
          proj_mbr.max_y = uly;
        }
      else
        {
          proj_mbr.max_y = ury;
        }


      spatial_representation_info.dimensionSize[0] = height = NINT ((proj_mbr.max_y - proj_mbr.min_y) / options.mbin_size + 0.05);
      spatial_representation_info.dimensionSize[1] = width = NINT ((proj_mbr.max_x - proj_mbr.min_x) / options.mbin_size + 0.05);
      spatial_representation_info.dimensionSize[2] = 0;

      spatial_representation_info.resolutionValue[0] = options.mbin_size;
      spatial_representation_info.resolutionValue[1] = options.mbin_size;
      spatial_representation_info.resolutionValue[2] = 0.0;


      //  Make sure we have an exact number of bins.

      proj_mbr.max_x = proj_mbr.min_x + width * options.mbin_size;
      proj_mbr.max_y = proj_mbr.min_y + height * options.mbin_size;


      //  Redefine the geodetic area from the northings and eastings so that projected and unprojected match.

      NV_FLOAT64 x, y;

      x = proj_mbr.max_x;
      y = proj_mbr.max_y;
      pj_transform (pj_utm, pj_latlon, 1, 1, &x, &y, NULL);
      mbr.max_x = x * NV_RAD_TO_DEG;
      mbr.max_y = y * NV_RAD_TO_DEG;

      x = proj_mbr.min_x;
      y = proj_mbr.min_y;
      pj_transform (pj_utm, pj_latlon, 1, 1, &x, &y, NULL);
      mbr.min_x = x * NV_RAD_TO_DEG;
      mbr.min_y = y * NV_RAD_TO_DEG;

      identification_info.westBoundingLongitude = mbr.min_x;
      identification_info.eastBoundingLongitude = mbr.max_x;
      identification_info.southBoundingLatitude = mbr.min_y;
      identification_info.northBoundingLatitude = mbr.max_y;

      spatial_representation_info.llCornerX = proj_mbr.min_x;
      spatial_representation_info.urCornerX = proj_mbr.max_x;
      spatial_representation_info.llCornerY = proj_mbr.min_y;
      spatial_representation_info.urCornerY = proj_mbr.max_y;

      half_x = half_y = options.mbin_size / 2.0;
    }
  else
    {
      strcpy (reference_system_info.projection, tr ("Geodetic").toAscii ());

      spatial_representation_info.dimensionSize[0] = height = NINT ((mbr.max_y - mbr.min_y) / y_bin_size_degrees + 0.05);
      spatial_representation_info.dimensionSize[1] = width = NINT ((mbr.max_x - mbr.min_x) / x_bin_size_degrees + 0.05);
      spatial_representation_info.dimensionSize[2] = 0;

      spatial_representation_info.resolutionValue[0] = y_bin_size_degrees;
      spatial_representation_info.resolutionValue[1] = x_bin_size_degrees;
      spatial_representation_info.resolutionValue[2] = 0.0;

      identification_info.westBoundingLongitude = spatial_representation_info.llCornerX = mbr.min_x;
      identification_info.eastBoundingLongitude = spatial_representation_info.urCornerX = mbr.max_x;
      identification_info.southBoundingLatitude = spatial_representation_info.llCornerY = mbr.min_y;
      identification_info.northBoundingLatitude = spatial_representation_info.urCornerY = mbr.max_y;

      half_x = open_args.head.x_bin_size_degrees * 0.5;
      half_y = open_args.head.y_bin_size_degrees * 0.5;
    }


  strcpy (reference_system_info.ellipsoid, "WGS84E");

  strcpy (reference_system_info.horizontalDatum, options.h_datums[options.h_datum].abbrev.toAscii ());
  strcpy (reference_system_info.verticalDatum, options.v_datums[options.v_datum].abbrev.toAscii ());

  strcpy (contact.individualName, options.poc_name.toAscii ());
  strcpy (contact.organisationName, tr ("Naval Oceanographic Office").toAscii ());
  strcpy (contact.positionName, options.pi_title.toAscii ());
  strcpy (contact.role, tr ("Point of Contact").toAscii ());


  if (!output_file_name.endsWith (".bag")) output_file_name.append (".bag");


  //  If the output bag already exists we have to remove it.

  if (QFile (output_file_name).exists ()) QFile (output_file_name).remove ();


  strcpy ((NV_CHAR *) name, output_file_name.toAscii ());


  bagError err;
  bagData data;
  bagHandle bagHandle;
  bagDataOpt opt_data_sep;
  bagHandle_opt bagHandle_sep;
  bagTrackingItem trackItem;
  NV_U_CHAR *xmlBuffer;
  NV_INT32 bfd_handle = -1;
  BFDATA_HEADER bfd_header;
  BFDATA_SHORT_FEATURE *feature;
  NV_BOOL features = NVFalse;
  NV_FLOAT64 *radius = NULL, log_array[100], lat = 0.0, lon = 0.0, northing = 0.0, easting = 0.0;
  NV_F64_COORD2 xy[2] = {{0.0, 0.0}, {0.0, 0.0}};


  //  Set up the log array for scaling so we don't have to keep computing powers of ten in the main loop.  Note that I'm
  //  subtracting 1.0 from the results at 0 and going up to 0.0 at 100.  This is so that the curve is zero based.  It's not
  //  exactly a log curve but it's pretty darn close.

  for (NV_INT32 i = 0 ; i < 100 ; i++) log_array[i] = pow (10.0L, ((NV_FLOAT64) i / 100.0)) - (1.0L * ((99.0L - (NV_FLOAT64) i) / 100.0L));


  memset (&data, 0, sizeof (data));


  if (strcmp (open_args.target_path, "NONE"))
    {
      if ((bfd_handle = binaryFeatureData_open_file (open_args.target_path, &bfd_header, BFDATA_READONLY)) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::warning (this, tr ("pfmBag"), tr ("Unable to open feature file\nReason: ") + msg);
          features = NVFalse;
        }
      else
        {
          binaryFeatureData_read_all_short_features (bfd_handle, &feature);
          features = NVTrue;
        }
    }


  //  If we are using the feature file to create an enhanced surface we have to create a weight array.

  if (options.enhanced && features)
    {
      //  Compute the pfmFeature search radius (if present) and save it in the radius array.

      radius = (NV_FLOAT64 *) calloc (bfd_header.number_of_records, sizeof (NV_FLOAT64));
      if (radius == NULL)
        {
          QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Allocating radius memory.\n") + QString (strerror (errno)));
          exit (-1);
        }

      for (NV_U_INT32 i = 0 ; i < bfd_header.number_of_records ; i++)
        {
          QString remarks = QString (feature[i].remarks);


          //  Compute the radius based on the diagonal of the bin size of features selected by pfmFeature.

          if (remarks.contains ("pfmFeature") && remarks.contains (", bin size "))
            {
              //  This is the description of how we defined the search radius prior to adding the "max dist" output
              //  to the feature remarks in pfmFeature.  If it is available we'll use the max dist otherwise we'll use
              //  the method described below.

              //  When running pfmFeature we use bin sizes of 3, 6, 12, and 24 meters (for IHO order 1).  To understand
              //  how we apply the search radius for the bin sizes from pfmFeature you have to visualize possible locations
              //  for the shoalest point in the center bin.  If the shoalest point is in the lower left corner of the 
              //  bin then the maximum distance that a trigger point (nearest point that meets IHO criteria) can be from the 
              //  shoal point (assuming 3 meter bins) is 7.071 meters.  That would be if the trigger point is in the upper
              //  right corner of the upper right bin cell.  The effect of this would be that the maximum distance of the 
              //  trigger point from the shoal point in the opposite direction would only be 2.83 meters.  To get a balanced
              //  search radius to be used for our enhanced surface we will assume that the shoalest point is exactly in the
              //  center of the center bin.  In that case the maximum distance in any direction to the trigger point would be
              //  4.95 meters.  That is the sum of the diagonal of a square that is half the bin size plus the diagonal of
              //  a square that is two thirds of the bin size (i.e. in the upper right corner of the upper right bin cell).


              //  Check for the "max dist" string in the feature record.

              if (remarks.contains (", max dist "))
                {
                  radius[i] = remarks.section (',', 6, 6).section (' ', 3, 3).toDouble ();
                }
              else
                {
                  NV_FLOAT64 bin_size = remarks.section (',', 2, 2).section (' ', 3, 3).toDouble ();
                  NV_FLOAT64 half = bin_size / 2.0L;
                  NV_FLOAT64 two_thirds = bin_size * 2.0L / 3.0L;
                  NV_FLOAT64 half_square = half * half;
                  NV_FLOAT64 two_thirds_square = two_thirds * two_thirds;
                  radius[i] = sqrt (half_square + half_square) + sqrt (two_thirds_square + two_thirds_square);
                }
            }
          else
            {
              //  Setting the radius to zero makes sure that we won't use non-pfmFeature features to build the enhanced surface.

              radius[i] = 0.0;
            }
        }


      //  Allocate the weight array.

      weight = (NV_U_BYTE **) calloc (height, sizeof (NV_U_BYTE *));
      if (weight == NULL)
        {
          QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Allocating weight grid memory.\n") + QString (strerror (errno)));
          exit (-1);
        }

      for (NV_INT32 i = 0 ; i < height ; i++)
        {
          weight[i] = (NV_U_BYTE *) calloc (width, sizeof (NV_U_BYTE));
          if (weight[i] == NULL)
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Allocating weight grid memory.\n") + QString (strerror (errno)));
              exit (-1);
            }
        }


      //  Populate the weight array using the features.

      progress.wbar->setRange (0, height);
      for (NV_INT32 i = 0 ; i < height ; i++)
        {
          progress.wbar->setValue (i);

          if (options.projection)
            {
              xy[0].y = proj_mbr.min_y + (NV_FLOAT64) i * options.mbin_size;
              xy[1].y = xy[0].y + options.mbin_size;

              northing = xy[0].y + half_y;
            }
          else
            {
              xy[0].y = mbr.min_y + (NV_FLOAT64) i * y_bin_size_degrees;
              xy[1].y = xy[0].y + y_bin_size_degrees;

              lat = xy[0].y + half_y;
            }

          for (NV_INT32 j = 0 ; j < width ; j++)
            {
              if (options.projection)
                {
                  xy[0].x = proj_mbr.min_x + (NV_FLOAT64) j * options.mbin_size;
                  xy[1].x = xy[0].x + options.mbin_size;

                  easting = xy[0].x + half_x;
                }
              else
                {
                  xy[0].x = mbr.min_x + (NV_FLOAT64) j * x_bin_size_degrees;
                  xy[1].x = xy[0].x + x_bin_size_degrees;

                  lon = xy[0].x + half_x;
                }

              NV_FLOAT64 sum = 0.0;
              NV_BOOL hit = NVFalse;


              for (NV_U_INT32 k = 0 ; k < bfd_header.number_of_records ; k++)
                {
                  //  Make sure the feature that has been read is inside the bounds of the BAG being built.
                  //  Also check the confidence.  If it is 0 it's invalid.  If it is 2 it was probably
                  //  set with mosaicView and is non-sonar.  If it's 1 it's probably not very good.

                  if (feature[k].confidence_level > 2 && feature[k].longitude >= mbr.min_x && feature[k].longitude <= mbr.max_x &&
                      feature[k].latitude >= mbr.min_y && feature[k].latitude <= mbr.max_y)
                    {
                      //  Simple check first...  If it's in the same bin then we set the sum to 100.0 and move on.

                      if (feature[k].longitude >= xy[0].x && feature[k].longitude <= xy[1].x &&
                          feature[k].latitude >= xy[0].y && feature[k].latitude <= xy[1].y)
                        {
                          sum = 100.0;
                          hit = NVTrue;
                          break;
                        }


                      //  Now for the more complicated stuff...  We have to compute the distance from the feature to the
                      //  cell center to compute the weight.

                      NV_FLOAT64 dist;

                      if (options.projection)
                        {
                          NV_FLOAT64 x = feature[k].longitude * NV_DEG_TO_RAD;
                          NV_FLOAT64 y = feature[k].latitude * NV_DEG_TO_RAD;
                          pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);
                          dist = sqrt ((northing - y) * (northing - y) + (easting - x) * (easting - x));
                        }
                      else
                        {
                          pfm_geo_distance (pfm_handle, lat, lon, feature[k].latitude, feature[k].longitude, &dist);
                        }


                      //  If we're less than our prescribed distance away from any feature, we want to use a combination of
                      //  the minimum depth in the bin and the average depth for the bin.  We use a power of ten, or log,
                      //  curve to blend the two depths together.  Linear blending falls off too quickly and leaves you
                      //  with the same old spike sticking up (like we used to have with the tracking list).  The blending
                      //  works by taking 100 percent of the minimum depth in the bin in which the feature is located and
                      //  100 percent of the average depth in bins that are more than the feature search radius away from
                      //  the feature.  As we move away from the feature (but still inside the search radius) we include
                      //  more of the average and less of the minimum (based on the precomputed log curve).  If the search
                      //  radii of two features overlap we add the blended minimum depth components (not to exceed 100 percent).
                      //  If, at any point in the feature comparison for a single bin, we exceed 100 percent we stop doing
                      //  the feature comparison for that bin.  This saves us a bit of time.

                      if (dist < radius[k])
                        {
                          NV_FLOAT64 percent = dist / radius[k];
                          NV_INT32 index = NINT (percent * 100.0);

                          if (index < 100)
                            {
                              sum += 100.0 - (log_array[index] * 10.0);
                              hit = NVTrue;
                              if (sum >= 100.0) break;
                            }
                        }
                    }
                }

              if (hit) weight[i][j] = qMin (NINT (sum), 100);
            }

          qApp->processEvents ();
        }

      progress.wbar->setValue (height);
      qApp->processEvents ();
    }


  //  Have to have either a source or a processStep if you want to create valid XML descriptions for a tracking list.

  strcpy (data_quality_info.scope, "dataset");
  data_quality_info.numberOfSources = 1;
  data_quality_info.numberOfProcessSteps = 0;
  memset (&source, 0, sizeof (SOURCE_INFO));
  strcpy (source.description, "NAVO PFM");
  data_quality_info.lineageSources = &source;


  //  Create the XML metadata

  NV_INT32 len = CreateXmlMetadataString (identification_info, md_legal_constraints, md_security_constraints, data_quality_info,
                                          spatial_representation_info, reference_system_info, contact, identification_info.date, &xmlBuffer);


  //  Initialize the BAG definition using the XML metadata

  if ((err = bagInitDefinitionFromBuffer (&data, xmlBuffer, len)) != BAG_SUCCESS)
    {
      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
        {
          string.sprintf (tr ("Error initializing BAG XML definition : %s").toAscii (), errstr);
          QMessageBox::warning (this, tr ("pfmBag Error"), string);
        }

      free (xmlBuffer);
      return;
    }


  //  Allocate the elevation array.

  NV_FLOAT32 *elevation = (NV_FLOAT32 *) calloc (width, sizeof (NV_FLOAT32));

  if (elevation == NULL)
    {
      perror (tr ("Allocating elevation memory").toAscii ());
      exit (-1);
    }


  //  Allocate the uncertainty array.

  NV_FLOAT32 *uncert = NULL;
  if (options.uncertainty)
    {
      uncert = (NV_FLOAT32 *) calloc (width, sizeof (NV_FLOAT32));

      if (uncert == NULL)
        {
          perror (tr ("Allocating uncertainty memory").toAscii ());
          exit (-1);
        }
    }


  //  Set to compressionLevel 1.

  opt_data_sep.compressionLevel = 1;
  data.compressionLevel = 1;


  //  Create the BAG file.

  if ((err = bagFileCreate (name, &data, &bagHandle)) != BAG_SUCCESS)
    {
      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
        {
          string.sprintf (tr ("Error creating BAG file : %s").toAscii (), errstr);
          QMessageBox::warning (this, tr ("pfmBag Error"), string);
        }

      free (xmlBuffer);
      return;
    }


  //  Store the values in the BAG.

  progress.mbar->setRange (0, height);

  NV_FLOAT64 py[2] = {0.0, 0.0};


  //  Figure out where (if anywhere) the final uncertainty attribute is stored.

  NV_INT32 fu_attr = -1;
  for (NV_INT32 i = 0 ; i < open_args.head.num_bin_attr ; i++)
    {
      if (strstr (open_args.head.bin_attr_name[i], "###5"))
        {
          fu_attr = i;
          break;
        }
    }


  //  Loop for the height of the PFM.

  for (NV_INT32 i = 0 ; i < height ; i++)
    {
      progress.mbar->setValue (i);

      if (options.projection)
        {
          py[0] = proj_mbr.min_y + (NV_FLOAT64) i * options.mbin_size;
          py[1] = py[0] + options.mbin_size;
        }
      else
        {
          xy[0].y = mbr.min_y + (NV_FLOAT64) i * y_bin_size_degrees;
          xy[1].y = xy[0].y + y_bin_size_degrees;
        }


      //  Loop for the width of the PFM.

      for (NV_INT32 j = 0 ; j < width ; j++)
        {
          NV_I32_COORD2 coord[2];


          //  Determine the range of the cell coordinates of the cells that have data in the output bin.

          if (options.projection)
            {
              xy[0].x = proj_mbr.min_x + (NV_FLOAT64) j * options.mbin_size;
              xy[1].x = xy[0].x + options.mbin_size;

              NV_FLOAT64 x = xy[0].x;
              NV_FLOAT64 y = py[0];
              pj_transform (pj_utm, pj_latlon, 1, 1, &x, &y, NULL);
              xy[0].x = x * NV_RAD_TO_DEG;
              xy[0].y = y * NV_RAD_TO_DEG;

              x = xy[1].x;
              y = py[1];
              pj_transform (pj_utm, pj_latlon, 1, 1, &x, &y, NULL);
              xy[1].x = x * NV_RAD_TO_DEG;
              xy[1].y = y * NV_RAD_TO_DEG;
            }
          else
            {
              xy[0].x = mbr.min_x + (NV_FLOAT64) j * x_bin_size_degrees;
              xy[1].x = xy[0].x + x_bin_size_degrees;
            }


          compute_index_ptr (xy[0], &coord[0], &open_args.head);
          compute_index_ptr (xy[1], &coord[1], &open_args.head);


          elevation[j] = NULL_ELEVATION;
          if (options.uncertainty) uncert[j] = NULL_UNCERTAINTY;


          NV_FLOAT64 sum = 0.0;
          NV_FLOAT64 sum2 = 0.0;
          NV_FLOAT64 uncert_sum = 0.0;
          NV_FLOAT64 uncert_sum2 = 0.0;
          NV_FLOAT64 min_uncert = 0.0;
          NV_INT32 count = 0;
          NV_FLOAT64 max_z = -999999999.0;
          NV_FLOAT64 min_z = 999999999.0;


          //  If we're running a CUBE surface we can't change the bin size or select the uncertainty type.
          //  These will be hard-wired.

          if (options.surface == CUBE_SURFACE)
            {
              BIN_RECORD bin;

              read_bin_record_index (pfm_handle, coord[0], &bin);

              if (bin.validity & PFM_DATA)
                {
                  sum = bin.avg_filtered_depth;
                  min_z = bin.min_filtered_depth;
                  min_uncert = uncert_sum = bin.attr[fu_attr];
                  count = 1;


                  //  If we are creating the enhanced surface we need to get the uncertainty of the minimum depth.

                  if (options.enhanced)
                    {
                      DEPTH_RECORD *depth;
                      NV_INT32 numrecs;

                      if (!read_depth_array_index (pfm_handle, coord[0], &depth, &numrecs))
                        {
                          for (NV_INT32 p = 0 ; p < numrecs ; p++)
                            {
                              if (!(depth[p].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                                {
                                  if (depth[p].xyz.z <= min_z)
                                    {
                                      min_uncert = depth[p].vertical_error;
                                      break;
                                    }
                                }
                            }

                          free (depth);
                        }
                    }
                }
            }
          else
            {
              //  Loop over the height and width of the covering cells.

              for (NV_INT32 m = coord[0].y ; m <= coord[1].y ; m++)
                {
                  if (m >= 0 && m < open_args.head.bin_height)
                    {
                      NV_I32_COORD2 icoord;
                      icoord.y = m;

                      for (NV_INT32 n = coord[0].x ; n <= coord[1].x ; n++)
                        {
                          if (n >= 0 && n < open_args.head.bin_width)
                            {
                              icoord.x = n;


                              DEPTH_RECORD *depth;
                              NV_INT32 numrecs;

                              if (!read_depth_array_index (pfm_handle, icoord, &depth, &numrecs))
                                {
                                  for (NV_INT32 p = 0 ; p < numrecs ; p++)
                                    {
                                      if ((!(depth[p].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) &&
                                          depth[p].xyz.x >= xy[0].x && depth[p].xyz.x <= xy[1].x &&
                                          depth[p].xyz.y >= xy[0].y && depth[p].xyz.y <= xy[1].y)
                                        {

                                          //  Get the minimum depth and the uncertainty of that depth.

                                          if (depth[p].xyz.z <= min_z)
                                            {
                                              min_uncert = depth[p].vertical_error;
                                              min_z = depth[p].xyz.z;
                                            }

                                          max_z = qMax (max_z, depth[p].xyz.z);

                                          if (options.surface != CUBE_SURFACE)
                                            {
                                              sum += depth[p].xyz.z;
                                              sum2 += depth[p].xyz.z * depth[p].xyz.z;
                                              uncert_sum += depth[p].vertical_error;
                                              uncert_sum2 += depth[p].vertical_error * depth[p].vertical_error;
                                              count++;
                                            }
                                        }
                                    }

                                  free (depth);
                                }
                            }
                        }
                    }
                }
            }


          qApp->processEvents ();


          if (min_z > 99999999.0) count = 0;


          if (count)
            {
              NV_FLOAT64 avg = sum / (NV_FLOAT64) count;


              switch (options.uncertainty)
                {
                case NO_UNCERT:
                  break;

                case STD_UNCERT:
                  uncert[j] = 0.0;

                  if (count > 1)
                    {
                      NV_FLOAT64 variance = ((sum2 - ((NV_FLOAT64) count * (pow (avg, 2.0)))) / ((NV_FLOAT64) count - 1.0));
                      if (variance >= 0.0) uncert[j] = sqrt (variance);
                    }
                  break;

                case FIN_UNCERT:
                  if (options.enhanced)
                    {
                      NV_FLOAT32 weight1 = (100.0 - (NV_FLOAT32) weight[i][j]) / 100.0;
                      NV_FLOAT32 weight2 = (NV_FLOAT32) weight[i][j] / 100.0;
                      uncert[j] = -((uncert_sum / (NV_FLOAT64) count) * weight1 + min_uncert * weight2);
                    }
                  else
                    {
                      uncert[j] = uncert_sum / (NV_FLOAT64) count;
                    }
                  break;

                case TPE_UNCERT:
                  if (options.enhanced)
                    {
                      NV_FLOAT32 weight1 = (100.0 - (NV_FLOAT32) weight[i][j]) / 100.0;
                      NV_FLOAT32 weight2 = (NV_FLOAT32) weight[i][j] / 100.0;
                      uncert[j] = -((sqrt (uncert_sum2 / (NV_FLOAT64) count)) * weight1 + min_uncert * weight2);
                    }
                  else
                    {
                      uncert[j] = sqrt (uncert_sum2 / (NV_FLOAT64) count);
                    }
                  break;
                }


              switch (options.surface)
                {
                case MIN_SURFACE:
                  elevation[j] = -min_z;
                  break;

                case MAX_SURFACE:
                  elevation[j] = -max_z;
                  break;

                case AVG_SURFACE:
                case CUBE_SURFACE:
                  if (options.enhanced)
                    {
                      NV_FLOAT32 weight1 = (100.0 - (NV_FLOAT32) weight[i][j]) / 100.0;
                      NV_FLOAT32 weight2 = (NV_FLOAT32) weight[i][j] / 100.0;
                      elevation[j] = -(avg * weight1 + min_z * weight2);
                    }
                  else
                    {
                      elevation[j] = -avg;
                    }
                }
            }
        }


      err = bagWriteRow (bagHandle, i, 0, width - 1, Elevation, (void *) elevation);
      if (options.uncertainty) err = bagWriteRow (bagHandle, i, 0, width - 1, Uncertainty, (void *) uncert);
    }


  free (elevation);
  if (options.uncertainty) free (uncert);

  progress.mbar->setValue (height);
  qApp->processEvents ();



  //  Put the non-pfmFeature features in the tracking list.

  NV_INT32 count = 0;
  data_quality_info.lineageProcessSteps = NULL; 
  NV_FLOAT32 value;


  progress.gbar->setRange (0, bfd_header.number_of_records);

  if (features)
    {
      for (NV_U_INT32 i = 0 ; i < bfd_header.number_of_records ; i++)
        {
          progress.gbar->setValue (i);
          qApp->processEvents ();


          //  Make sure the feature that has been read is inside the bounds of the BAG being built.  Also check the confidence.
          //  If it is 0 it's invalid.  If it is 2 it was probably set with mosaicView and is non-sonar.  If it's 1 it's
          //  probably not very good.

          if (feature[i].confidence_level > 2 && feature[i].longitude >= mbr.min_x && feature[i].longitude <= mbr.max_x &&
              feature[i].latitude >= mbr.min_y && feature[i].latitude <= mbr.max_y)
            {
              QString remarks = QString (feature[i].remarks);


              //  Only add non-pfmFeature features to the tracking list.

              if (!remarks.contains ("pfmFeature"))
                {
                  if (options.projection)
                    {
                      NV_FLOAT64 x = feature[i].longitude * NV_DEG_TO_RAD;
                      NV_FLOAT64 y = feature[i].latitude * NV_DEG_TO_RAD;
                      pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                      trackItem.row = NINT (((y - proj_mbr.min_y) / options.mbin_size) + 0.5) ;
                      trackItem.col = NINT (((x - proj_mbr.min_x) / options.mbin_size) + 0.5) ;
                    }
                  else
                    {
                      trackItem.row = NINT (((feature[i].latitude - mbr.min_y) / y_bin_size_degrees) + 0.5) ;
                      trackItem.col = NINT (((feature[i].longitude - mbr.min_x) / x_bin_size_degrees) + 0.5) ;
                    }

                  if ((err = bagReadNode (bagHandle, trackItem.row, trackItem.col, Elevation, (void *) &value)) != BAG_SUCCESS)
                    {
                      u8 *errstr;

                      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
                        {
                          string.sprintf (tr ("Error reading elevation node at %d,%d : %s").toAscii (), trackItem.row, trackItem.col, errstr);
                          QMessageBox::warning (this, tr ("pfmBag Error"), string);
                        }

                      free (xmlBuffer);
                      return;
                    }


                  trackItem.depth = -value;

                  trackItem.track_code = bagDesignatedSndg;


                  //  Put the description and remarks into the XML data.

                  trackItem.list_series = count++;

                  data_quality_info.lineageProcessSteps = (PROCESS_STEP_INFO *) realloc (data_quality_info.lineageProcessSteps, count *
                                                                                         sizeof (PROCESS_STEP_INFO));

                  if (data_quality_info.lineageProcessSteps == NULL)
                    {
                      perror ("Allocating lineage_process_steps");
                      exit (-1);
                    }


                  //  Make sure that we clear the structure so we don't get garbage.

                  memset (&data_quality_info.lineageProcessSteps[trackItem.list_series], 0, sizeof (PROCESS_STEP_INFO));


                  QString string0 (feature[i].description);

                  QString string1 (feature[i].remarks);

                  QString new_string = "No description available";
                  if (string1.isEmpty ())
                    {
                      if (!string0.isEmpty ()) new_string = string0;
                    }
                  else
                    {
                      if (string0.isEmpty ())
                        {
                          new_string = string1;
                        }
                      else
                        {
                          new_string = string0 + " :: " + string1;
                        }
                    }

                  strcpy (data_quality_info.lineageProcessSteps[trackItem.list_series].description, new_string.toAscii ());

                  NV_INT32 year, jday, month, mday, hour, minute;
                  NV_FLOAT32 second;
                  cvtime (feature[i].event_tv_sec, feature[i].event_tv_nsec, &year, &jday, &hour, &minute, &second);
                  jday2mday (year, jday, &month, &mday);
                  month++;

                  sprintf (data_quality_info.lineageProcessSteps[trackItem.list_series].dateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                           year + 1900, month, mday, hour, minute, NINT (second));

                  sprintf (data_quality_info.lineageProcessSteps[trackItem.list_series].trackingId, "%d", trackItem.list_series);


                  //  Write the tracking list item.

                  if ((err = bagWriteTrackingListItem (bagHandle, &trackItem)) != BAG_SUCCESS)
                    {
                      u8 *errstr;

                      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
                        {
                          string.sprintf (tr ("Error adding tracking list item : %s").toAscii (), errstr);
                          QMessageBox::warning (this, tr ("pfmBag Error"), string);
                        }

                      free (xmlBuffer);
                      return;
                    }


                  //  Write the modified nodes.

                  value = -feature[i].depth;
                  if ((err = bagWriteNode (bagHandle, trackItem.row, trackItem.col, Elevation, (void *) &value)) != BAG_SUCCESS)
                    {
                      u8 *errstr;

                      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
                        {
                          string.sprintf (tr ("Error writing elevation node at %d,%d : %s").toAscii (), trackItem.row, trackItem.col, errstr);
                          QMessageBox::warning (this, tr ("pfmBag Error"), string);
                        }

                      free (xmlBuffer);
                      return;
                    }
                }
            }
        }


      progress.gbar->setValue (bfd_header.number_of_records);


      //  Close the bfd file here.  This frees the short feature structure.

      binaryFeatureData_close_file (bfd_handle);
    }


  if ((err = bagUpdateSurface (bagHandle, Elevation)) != BAG_SUCCESS)
    {
      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
        {
          string.sprintf (tr ("Error updating elevation surface : %s").toAscii (), errstr);
          QMessageBox::warning (this, tr ("pfmBag Error"), string);
        }

      free (xmlBuffer);
      return;
    }


  if (options.uncertainty)
    {
      if ((err = bagUpdateSurface (bagHandle, Uncertainty)) != BAG_SUCCESS)
        {
          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
            {
              string.sprintf (tr ("Error updating uncertainty surface : %s").toAscii (), errstr);
              QMessageBox::warning (this, tr ("pfmBag Error"), string);
            }

          free (xmlBuffer);
          return;
        }
    }


  //  If we added any non-pfmFeature features to the tracking list we need to redo the XML metadata.

  if (count)
    {
      data_quality_info.numberOfProcessSteps = count;
      SetDataQualityInfo (data_quality_info, &xmlBuffer);


      bagGetDataPointer (bagHandle)->metadata = xmlBuffer;


      if ((err = bagWriteXMLStream (bagHandle)) != BAG_SUCCESS)
        {
          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
            {
              string.sprintf (tr ("Error writing XML stream : %s").toAscii (), errstr);
              QMessageBox::warning (this, tr ("pfmBag Error"), string);
            }

          free (xmlBuffer);
          return;
        }
    }


  //  Adding optional separation surface if requested.

  if (!sep_file_name.isEmpty ())
    {
      memset (&opt_data_sep, 0, sizeof(opt_data_sep));

      strcpy (sep_file, sep_file_name.toAscii ());


      //  Copy most of the default setup for the main BAG.

      opt_data_sep.def = data.def;


      //  Check for .ch2 extension.

      if (sep_file_name.endsWith (".ch2"))
        {
          if ((sep_handle = chrtr2_open_file (sep_file, &sep_header, CHRTR2_READONLY)) < 0)
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Unable to open CHRTR2 separation file %1\nReason: %2").arg (sep_file_name).arg
                                     (QString (chrtr2_strerror ())));
              exit (-1);
            }

          opt_data_sep.def.ncols = sep_header.width;
          opt_data_sep.def.nrows = sep_header.height;
          opt_data_sep.def.nodeSpacingX = sep_header.lon_grid_size_degrees;
          opt_data_sep.def.nodeSpacingY = sep_header.lat_grid_size_degrees;
          opt_data_sep.def.swCornerX = sep_header.mbr.wlon;
          opt_data_sep.def.swCornerY = sep_header.mbr.slat;
        }
      else
        {
          if ((sep_fp = fopen (sep_file, "r")) == NULL)
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Unable to open ASCII separation file %1\nReason: %2").arg (sep_file_name).arg
                                     (QString (strerror (errno))));
              exit (-1);
            }


          ngets (sep_string, sizeof (sep_string), sep_fp);

          if (!strstr (sep_string, "LAT,LONG,Z0,Z1"))
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("ASCII separation file %1 format incorrect").arg (sep_file_name));
              exit (-1);
            }


          options.has_z0 = NVTrue;
          options.has_z1 = NVTrue;


          //  Unfortunately, for ASCII files we have to read the whole file to determine the width, height, and other stuff.

          NV_INT32 pos = ftell (sep_fp);
          NV_INT32 y_count = 0, x_count = 0;
          NV_FLOAT64 x, y, z0, z1, prev_x = 999.0, prev_y = 999.0;
          NV_BOOL first_x = NVTrue;

          while (ngets (sep_string, sizeof (sep_string), sep_fp) != NULL)
            {
              sscanf (sep_string, "%lf,%lf,%lf,%lf", &y, &x, &z0, &z1);

              if (first_x)
                {
                  opt_data_sep.def.swCornerX = x;
                  first_x = NVFalse;
                }

              if (prev_y != y)
                {
                  y_count++;
                  opt_data_sep.def.nodeSpacingY = prev_y - y;
                }

              if (prev_x != x) x_count++;

              prev_y = y;
              prev_x = x;
            }

          fseek (sep_fp, pos, SEEK_SET);

          x_count /= y_count;

          opt_data_sep.def.ncols = x_count;
          opt_data_sep.def.nrows = y_count;
          opt_data_sep.def.nodeSpacingX = x - prev_x;
          opt_data_sep.def.swCornerY = y;
        }

      err = bagCreateCorrectorDataset (bagHandle, &bagHandle_sep, &opt_data_sep, 2, BAG_SURFACE_GRID_EXTENTS);
      if (err != BAG_SUCCESS)
        {
          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
            {
              string.sprintf (tr ("Error creating corrector dataset : %s").toAscii (), errstr);
              QMessageBox::critical (this, tr ("pfmBag Error"), string);
              exit (-1);
            }
        }

      bagVerticalCorrector *sep_depth = (bagVerticalCorrector *) calloc (opt_data_sep.def.ncols, sizeof (bagVerticalCorrector));
      if (sep_depth == NULL)
        {
          string.sprintf (tr ("Error allocating sep_depth : %s").toAscii (), strerror (errno));
          QMessageBox::critical (this, tr ("pfmBag Error"), string);
          exit (-1);
        }

      for (NV_U_INT32 i = 0 ; i < opt_data_sep.def.nrows ; i++)
        {
          NV_I32_COORD2 coord;

          coord.y = i;

          for (NV_U_INT32 j = 0 ; j < opt_data_sep.def.ncols ; j++)
            {
              coord.x = j;

              if (sep_fp)
                {
                  ngets (sep_string, sizeof (sep_string), sep_fp);

                  sscanf (sep_string, "%lf,%lf,%f,%f", &sep_depth[j].y, &sep_depth[j].x, &sep_depth[j].z[0], &sep_depth[j].z[1]);
                }
              else
                {
                  chrtr2_read_record (sep_handle, coord, &sep_record);

                  chrtr2_get_lat_lon (sep_handle, &sep_depth[j].y, &sep_depth[j].x, coord);

                  if (options.has_z0)
                    {
                      sep_depth[j].z[0] = sep_record.z0;
                    }
                  else
                    {
                      sep_depth[j].z[0] = 0.0;
                    }

                  if (options.has_z1)
                    {
                      sep_depth[j].z[1] = sep_record.z1;
                    }
                  else
                    {
                      sep_depth[j].z[1] = sep_record.z;
                    }
                }


              //  If we're making a UTM projected BAG, convert positions to UTM.

              if (options.projection)
                {
                  NV_FLOAT64 x = sep_depth[j].x * NV_DEG_TO_RAD;
                  NV_FLOAT64 y = sep_depth[j].y * NV_DEG_TO_RAD;
                  pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);
                }
            }

          err = bagWriteOptRow (bagHandle, bagHandle_sep, i, 0, opt_data_sep.def.ncols - 1, Surface_Correction, (void *) sep_depth);
        }

      if (sep_fp)
        {
          fclose (sep_fp);
        }
      else
        {
          chrtr2_close_file (sep_handle);
        }

      free (sep_depth);


      bagFreeInfoOpt (bagHandle_sep);


      bagWriteCorrectorVerticalDatum (bagHandle, bagHandle_sep, 1, (u8 *) "Mean lower low water = Vertical Datum");
      bagWriteCorrectorVerticalDatum (bagHandle, bagHandle_sep, 2, (u8 *) "WGS84 = Ellipsoid Datum");
    }


  //  IMPORTANT NOTE: bagFileClose will free the xmlBuffer (i.e. metadata).

  if ((err = bagFileClose (bagHandle)) != BAG_SUCCESS)
    {
      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
        {
          string.sprintf (tr ("Error closing BAG file : %s").toAscii (), errstr);
          QMessageBox::warning (this, tr ("pfmBag Error"), string);
        }

      return;
    }


  button (QWizard::FinishButton)->setEnabled (TRUE);
  button (QWizard::CancelButton)->setEnabled (FALSE);


  QApplication::restoreOverrideCursor ();


  checkList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Conversion complete, press Finish to exit."));

  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);
}



//  Get the users defaults.

void pfmBag::envin (OPTIONS *options)
{
  NV_FLOAT64 saved_version = 2.0;


  // Set defaults so that if keys don't exist the parameters are defined

  options->surface = 2;
  options->uncertainty = 1;
  options->enhanced = NVTrue;
  options->units = 0;
  options->depth_cor = 0;
  options->projection = 0;
  options->h_datum = 800;
  options->v_datum = 53;
  options->group = tr ("generic");
  options->source = tr ("Naval Oceanographic Office");
  options->version = tr ("1.0");
  options->classification = 0;
  options->authority = 0;
  options->distStatement = tr ("Approved for public release; distribution is unlimited.");
  options->pi_name = "";
  options->pi_title = "";
  options->poc_name = "";
  options->input_dir = ".";
  options->output_dir = ".";
  options->area_dir = ".";
  options->sep_dir = ".";
  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 900;
  options->window_height = 500;


  QSettings settings (tr ("navo.navy.mil"), tr ("pfmBag"));

  settings.beginGroup (tr ("pfmBag"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->surface = settings.value (tr ("surface"), options->surface).toInt ();

  options->uncertainty = settings.value (tr ("uncertainty"), options->uncertainty).toInt ();

  options->enhanced = settings.value (tr ("enhanced surface flag"), options->enhanced).toBool ();

  options->units = settings.value (tr ("units"), options->units).toInt ();

  options->depth_cor = settings.value (tr ("depth correction"), options->depth_cor).toInt ();

  options->projection = settings.value (tr ("projection"), options->projection).toInt ();


  //  These aren't supported yet.

  //options->h_datum = settings.value (tr ("horizontal datum"), options->h_datum).toInt ();

  //options->v_datum = settings.value (tr ("vertical datum"), options->v_datum).toInt ();


  options->group = settings.value (tr ("group name"), options->group).toString ();

  options->source = settings.value (tr ("data source"), options->source).toString ();

  options->version = settings.value (tr ("data version"), options->version).toString ();

  options->classification = settings.value (tr ("classification"), options->classification).toInt ();

  options->authority = settings.value (tr ("declassification authority"), options->authority).toInt ();


  options->distStatement = settings.value (tr ("distribution statement"), options->distStatement).toString ();


  options->pi_name = settings.value (tr ("PI name"), options->pi_name).toString ();

  options->pi_title = settings.value (tr ("PI title"), options->pi_title).toString ();

  options->poc_name = settings.value (tr ("POC name"), options->poc_name).toString ();

  options->input_dir = settings.value (tr ("input directory"), options->input_dir).toString ();
  options->output_dir = settings.value (tr ("output directory"), options->output_dir).toString ();
  options->area_dir = settings.value (tr ("area directory"), options->area_dir).toString ();
  options->sep_dir = settings.value (tr ("separation directory"), options->sep_dir).toString ();

  options->window_width = settings.value (tr ("width"), options->window_width).toInt ();
  options->window_height = settings.value (tr ("height"), options->window_height).toInt ();
  options->window_x = settings.value (tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (tr ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void pfmBag::envout (OPTIONS *options)
{
  QSettings settings (tr ("navo.navy.mil"), tr ("pfmBag"));

  settings.beginGroup (tr ("pfmBag"));


  settings.setValue (tr ("settings version"), settings_version);


  settings.setValue (tr ("surface"), options->surface);

  settings.setValue (tr ("uncertainty"), options->uncertainty);

  settings.setValue (tr ("enhanced surface flag"), options->enhanced);

  settings.setValue (tr ("units"), options->units);

  settings.setValue (tr ("depth correction"), options->depth_cor);

  settings.setValue (tr ("projection"), options->projection);


  //  These aren't supported yet.

  //settings.setValue (tr ("horizontal datum"), options->h_datum);

  //settings.setValue (tr ("vertical datum"), options->v_datum);


  settings.setValue (tr ("group name"), options->group);

  settings.setValue (tr ("data source"), options->source);

  settings.setValue (tr ("data version"), options->version);

  settings.setValue (tr ("classification"), options->classification);

  settings.setValue (tr ("declassification authority"), options->authority);


  settings.setValue (tr ("distribution statement"), options->distStatement);


  settings.setValue (tr ("PI name"), options->pi_name);

  settings.setValue (tr ("PI title"), options->pi_title);

  settings.setValue (tr ("POC name"), options->poc_name);

  settings.setValue (tr ("input directory"), options->input_dir);
  settings.setValue (tr ("output directory"), options->output_dir);
  settings.setValue (tr ("area directory"), options->area_dir);
  settings.setValue (tr ("separation directory"), options->sep_dir);

  settings.setValue (tr ("width"), options->window_width);
  settings.setValue (tr ("height"), options->window_height);
  settings.setValue (tr ("x position"), options->window_x);
  settings.setValue (tr ("y position"), options->window_y);

  settings.endGroup ();
}
