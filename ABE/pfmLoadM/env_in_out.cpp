
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



#include "pfmLoadM.hpp"


NV_FLOAT64 settings_version = 4.40;



//  Get the users defaults.

void
pfmLoadM::envin ()
{
  NV_FLOAT64 saved_version = 0.0;


  // Set defaults so that if keys don't exist the parameters are defined

  pfm_global.horizontal_error = 1.5;
  pfm_global.vertical_error = 0.25;


  //  We're not saving these since we don't really know what the hell they do ;-)

  pfm_global.iho = 0;
  pfm_global.capture = 5.0;
  pfm_global.queue = 11;
  pfm_global.horiz = 0.0;
  pfm_global.distance = 2.0;
  pfm_global.min_context = 0.0;
  pfm_global.max_context = 5.0;
  pfm_global.std2conf = 1;
  pfm_global.disambiguation = 0;
  pfm_global.appending = NVFalse;
  pfm_global.attribute_count = 0;

  pfm_global.time_attribute_num = 1;
  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++) pfm_global.gsf_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++) pfm_global.hof_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++) pfm_global.tof_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++) pfm_global.wlf_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++) pfm_global.czmil_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++) pfm_global.bag_attribute_num[i] = 0;
  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++) pfm_global.hawkeye_attribute_num[i] = 0;

  pfm_global.input_dir = ".";
  pfm_global.output_dir = ".";
  pfm_global.area_dir = ".";
  pfm_global.mosaic_dir = ".";
  pfm_global.feature_dir = ".";

  flags.chk = NVTrue;
  flags.old = NVFalse;
  flags.hof = NVFalse;
  flags.sub = NVFalse;
  flags.lnd = NVFalse;
  flags.tof = NVFalse;
  flags.lid = NVTrue;
  flags.nom = NVFalse;
  flags.sec = NVFalse;
  flags.srtmb = NVFalse;
  flags.srtm1 = NVFalse;
  flags.srtm3 = NVFalse;
  flags.srtm30 = NVFalse;
  flags.srtmr = NVFalse;
  flags.srtme = NVTrue;
  flags.attr = NVTrue;
  flags.cube = NVFalse;
  inputFilter = pfmLoadM::tr ("GSF (*.d\?\? *.gsf)");
  ref_def.mbin_size = 2.0;
  ref_def.gbin_size = 0.0;
  ref_def.min_depth = -500.0;
  ref_def.max_depth = 1000.0;
  ref_def.precision = 0.01;
  ref_def.apply_area_filter = NVFalse;
  ref_def.deep_filter_only = NVTrue;
  ref_def.cellstd = 2.4;
  ref_def.radius = 20.0;
  window_x = 0;
  window_y = 0;
  window_width = 880;
  window_height = 520;


  QSettings settings (pfmLoadM::tr ("navo.navy.mil"), pfmLoadM::tr ("pfmLoadM"));

  settings.beginGroup (pfmLoadM::tr ("pfmLoadM"));

  saved_version = settings.value (pfmLoadM::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  flags.chk = settings.value (pfmLoadM::tr ("check files"), flags.chk).toBool ();

  flags.old = settings.value (pfmLoadM::tr ("load HOF in GCS form"), flags.old).toBool ();

  flags.hof = settings.value (pfmLoadM::tr ("load HOF null values"), flags.hof).toBool ();

  flags.sub = settings.value (pfmLoadM::tr ("invert substitute paths"), flags.sub).toBool ();

  flags.lnd = settings.value (pfmLoadM::tr ("invalidate HOF land values"), flags.lnd).toBool ();

  flags.tof = settings.value (pfmLoadM::tr ("make TOF first returns reference"), flags.tof).toBool ();

  flags.lid = settings.value (pfmLoadM::tr ("load HOF standard user flags"), flags.lid).toBool ();

  flags.sec = settings.value (pfmLoadM::tr ("invalidate HOF secondary depths"), flags.sec).toBool ();

  flags.nom = settings.value (pfmLoadM::tr ("load nominal depths"), flags.nom).toBool ();

  flags.srtmb = settings.value (pfmLoadM::tr ("load best SRTM"), flags.srtmb).toBool ();

  flags.srtm1 = settings.value (pfmLoadM::tr ("load 1 second SRTM"), flags.srtm1).toBool ();

  flags.srtm3 = settings.value (pfmLoadM::tr ("load 3 second SRTM"), flags.srtm3).toBool ();

  flags.srtm30 = settings.value (pfmLoadM::tr ("load 30 second SRTM"), flags.srtm30).toBool ();

  flags.srtmr = settings.value (pfmLoadM::tr ("load SRTM as reference"), flags.srtmr).toBool ();

  flags.srtme = settings.value (pfmLoadM::tr ("Exclude SRTM2 data"), flags.srtme).toBool ();

  flags.cube = settings.value (pfmLoadM::tr ("Run CUBE"), flags.cube).toBool ();

  flags.attr = settings.value (pfmLoadM::tr ("Save CUBE placeholders"), flags.attr).toBool ();

  inputFilter = settings.value (pfmLoadM::tr ("input filter"), inputFilter).toString ();

  ref_def.max_depth = (NV_FLOAT32) settings.value (pfmLoadM::tr ("max depth"), (NV_FLOAT64) ref_def.max_depth).toDouble ();

  ref_def.min_depth = (NV_FLOAT32) settings.value (pfmLoadM::tr ("min depth"), (NV_FLOAT64) ref_def.min_depth).toDouble ();

  ref_def.precision = (NV_FLOAT32) settings.value (pfmLoadM::tr ("precision"), (NV_FLOAT64) ref_def.precision).toDouble ();

  ref_def.mbin_size = (NV_FLOAT32) settings.value (pfmLoadM::tr ("bin size meters"), (NV_FLOAT64) ref_def.mbin_size).toDouble ();
  ref_def.gbin_size = (NV_FLOAT32) settings.value (pfmLoadM::tr ("bin size minutes"), (NV_FLOAT64) ref_def.gbin_size).toDouble ();

  ref_def.apply_area_filter = settings.value (pfmLoadM::tr ("apply area filter"), ref_def.apply_area_filter).toBool ();
  ref_def.deep_filter_only = settings.value (pfmLoadM::tr ("deep filter only"), ref_def.deep_filter_only).toBool ();

  ref_def.cellstd = (NV_FLOAT32) settings.value (pfmLoadM::tr ("bin standard deviation"), (NV_FLOAT64) ref_def.cellstd).toDouble ();

  ref_def.radius = (NV_FLOAT32) settings.value (pfmLoadM::tr ("feature radius"), (NV_FLOAT64) ref_def.radius).toDouble ();

  pfm_global.horizontal_error = (NV_FLOAT32) settings.value (pfmLoadM::tr ("horizontal error"), 
							     (NV_FLOAT64) pfm_global.horizontal_error).toDouble ();
  pfm_global.vertical_error = (NV_FLOAT32) settings.value (pfmLoadM::tr ("vertical error"), 
							     (NV_FLOAT64) pfm_global.vertical_error).toDouble ();

  pfm_global.time_attribute_num = settings.value (pfm_global.time_attribute_name, pfm_global.time_attribute_num).toInt ();

  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    pfm_global.gsf_attribute_num[i] = settings.value (pfm_global.gsf_attribute_name[i], pfm_global.gsf_attribute_num[i]).toInt ();

  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    pfm_global.hof_attribute_num[i] = settings.value (pfm_global.hof_attribute_name[i], pfm_global.hof_attribute_num[i]).toInt ();

  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    pfm_global.tof_attribute_num[i] = settings.value (pfm_global.tof_attribute_name[i], pfm_global.tof_attribute_num[i]).toInt ();

  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    pfm_global.wlf_attribute_num[i] = settings.value (pfm_global.wlf_attribute_name[i], pfm_global.wlf_attribute_num[i]).toInt ();

  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
    pfm_global.czmil_attribute_num[i] = settings.value (pfm_global.czmil_attribute_name[i], pfm_global.czmil_attribute_num[i]).toInt ();

  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
    pfm_global.bag_attribute_num[i] = settings.value (pfm_global.bag_attribute_name[i], pfm_global.bag_attribute_num[i]).toInt ();

  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
    pfm_global.hawkeye_attribute_num[i] = settings.value (pfm_global.hawkeye_attribute_name[i], pfm_global.hawkeye_attribute_num[i]).toInt ();

  pfm_global.input_dir = settings.value (pfmLoadM::tr ("input directory"), pfm_global.input_dir).toString ();
  pfm_global.output_dir = settings.value (pfmLoadM::tr ("output directory"), pfm_global.output_dir).toString ();
  pfm_global.area_dir = settings.value (pfmLoadM::tr ("area directory"), pfm_global.area_dir).toString ();
  pfm_global.mosaic_dir = settings.value (pfmLoadM::tr ("mosaic directory"), pfm_global.mosaic_dir).toString ();
  pfm_global.feature_dir = settings.value (pfmLoadM::tr ("feature directory"), pfm_global.feature_dir).toString ();

  window_width = settings.value (pfmLoadM::tr ("width"), window_width).toInt ();
  window_height = settings.value (pfmLoadM::tr ("height"), window_height).toInt ();
  window_x = settings.value (pfmLoadM::tr ("x position"), window_x).toInt ();
  window_y = settings.value (pfmLoadM::tr ("y position"), window_y).toInt ();


  if (pfm_global.horizontal_error == 0.0) pfm_global.horizontal_error = 1.5;
  if (pfm_global.vertical_error == 0.0) pfm_global.vertical_error = 0.25;


  settings.endGroup ();
}



//  Save the users defaults.

void
pfmLoadM::envout ()
{
  QSettings settings (pfmLoadM::tr ("navo.navy.mil"), pfmLoadM::tr ("pfmLoadM"));

  settings.beginGroup (pfmLoadM::tr ("pfmLoadM"));


  settings.setValue (pfmLoadM::tr ("settings version"), settings_version);

  settings.setValue (pfmLoadM::tr ("check files"), flags.chk);

  settings.setValue (pfmLoadM::tr ("load HOF in GCS form"), flags.old);

  settings.setValue (pfmLoadM::tr ("load HOF null values"), flags.hof);

  settings.setValue (pfmLoadM::tr ("invert substitute paths"), flags.sub);

  settings.setValue (pfmLoadM::tr ("invalidate HOF land values"), flags.lnd);

  settings.setValue (pfmLoadM::tr ("make TOF first returns reference"), flags.tof);

  settings.setValue (pfmLoadM::tr ("load HOF standard user flags"), flags.lid);

  settings.setValue (pfmLoadM::tr ("invalidate HOF secondary depths"), flags.sec);

  settings.setValue (pfmLoadM::tr ("load nominal depths"), flags.nom);

  settings.setValue (pfmLoadM::tr ("load best SRTM"), flags.srtmb);

  settings.setValue (pfmLoadM::tr ("load 1 second SRTM"), flags.srtm1);

  settings.setValue (pfmLoadM::tr ("load 3 second SRTM"), flags.srtm3);

  settings.setValue (pfmLoadM::tr ("load 30 second SRTM"), flags.srtm30);

  settings.setValue (pfmLoadM::tr ("load SRTM as reference"), flags.srtmr);

  settings.setValue (pfmLoadM::tr ("Exclude SRTM2 data"), flags.srtme);

  settings.setValue (pfmLoadM::tr ("Run CUBE"), flags.cube);

  settings.setValue (pfmLoadM::tr ("Save CUBE placeholders"), flags.attr);

  settings.setValue (pfmLoadM::tr ("input filter"), inputFilter);

  settings.setValue (pfmLoadM::tr ("max depth"), (NV_FLOAT64) ref_def.max_depth);

  settings.setValue (pfmLoadM::tr ("min depth"), (NV_FLOAT64) ref_def.min_depth);

  settings.setValue (pfmLoadM::tr ("precision"), (NV_FLOAT64) ref_def.precision);

  settings.setValue (pfmLoadM::tr ("bin size meters"), (NV_FLOAT64) ref_def.mbin_size);
  settings.setValue (pfmLoadM::tr ("bin size minutes"), (NV_FLOAT64) ref_def.gbin_size);

  settings.setValue (pfmLoadM::tr ("apply area filter"), ref_def.apply_area_filter);

  settings.setValue (pfmLoadM::tr ("deep filter only"), ref_def.deep_filter_only);

  settings.setValue (pfmLoadM::tr ("bin standard deviation"), (NV_FLOAT64) ref_def.cellstd);

  settings.setValue (pfmLoadM::tr ("feature radius"), (NV_FLOAT64) ref_def.radius);

  settings.setValue (pfmLoadM::tr ("horizontal error"), (NV_FLOAT64) pfm_global.horizontal_error);
  settings.setValue (pfmLoadM::tr ("vertical error"), (NV_FLOAT64) pfm_global.vertical_error);


  settings.setValue (pfm_global.time_attribute_name, pfm_global.time_attribute_num);

  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    settings.setValue (pfm_global.gsf_attribute_name[i], pfm_global.gsf_attribute_num[i]);

  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    settings.setValue (pfm_global.hof_attribute_name[i], pfm_global.hof_attribute_num[i]);

  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    settings.setValue (pfm_global.tof_attribute_name[i], pfm_global.tof_attribute_num[i]);

  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    settings.setValue (pfm_global.wlf_attribute_name[i], pfm_global.wlf_attribute_num[i]);

  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
    settings.setValue (pfm_global.czmil_attribute_name[i], pfm_global.czmil_attribute_num[i]);

  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
    settings.setValue (pfm_global.bag_attribute_name[i], pfm_global.bag_attribute_num[i]);

  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
    settings.setValue (pfm_global.hawkeye_attribute_name[i], pfm_global.hawkeye_attribute_num[i]);

  settings.setValue (pfmLoadM::tr ("input directory"), pfm_global.input_dir);
  settings.setValue (pfmLoadM::tr ("output directory"), pfm_global.output_dir);
  settings.setValue (pfmLoadM::tr ("area directory"), pfm_global.area_dir);
  settings.setValue (pfmLoadM::tr ("mosaic directory"), pfm_global.mosaic_dir);
  settings.setValue (pfmLoadM::tr ("feature directory"), pfm_global.feature_dir);

  settings.setValue (pfmLoadM::tr ("width"), window_width);
  settings.setValue (pfmLoadM::tr ("height"), window_height);
  settings.setValue (pfmLoadM::tr ("x position"), window_x);
  settings.setValue (pfmLoadM::tr ("y position"), window_y);

  settings.endGroup ();
}
