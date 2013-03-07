#include "pfmExtract.hpp"


NV_FLOAT64 settings_version = 3.00;


void envin (OPTIONS *options)
{
  NV_FLOAT64 saved_version = 0.0;


  QSettings settings (pfmExtract::tr ("navo.navy.mil"), pfmExtract::tr ("pfmExtract"));

  settings.beginGroup (pfmExtract::tr ("pfmExtract"));

  saved_version = settings.value (pfmExtract::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;



  options->source = settings.value (pfmExtract::tr ("data source"), options->source).toInt ();
  options->format = settings.value (pfmExtract::tr ("data format"), options->format).toInt ();
  options->ref = settings.value (pfmExtract::tr ("reference data flag"), options->ref).toBool ();
  options->geoid03 = settings.value (pfmExtract::tr ("orthometric correction flag"), options->geoid03).toBool ();
  options->chk = settings.value (pfmExtract::tr ("checked data flag"), options->chk).toBool ();
  options->flp = settings.value (pfmExtract::tr ("invert Z flag"), options->flp).toBool ();
  options->utm = settings.value (pfmExtract::tr ("utm flag"), options->utm).toBool ();
  options->lnd = settings.value (pfmExtract::tr ("land flag"), options->lnd).toBool ();
  options->unc = settings.value (pfmExtract::tr ("uncertainty flag"), options->unc).toBool ();
  options->cut = settings.value (pfmExtract::tr ("cutoff flag"), options->cut).toBool ();
  options->cutoff = settings.value (pfmExtract::tr ("depth cutoff"), options->cutoff).toDouble ();
  options->datum_shift = settings.value (pfmExtract::tr ("datum shift"), options->datum_shift).toDouble ();
  options->size = settings.value (pfmExtract::tr ("file size limit"), options->size).toInt ();
  options->input_dir = settings.value (pfmExtract::tr ("input directory"), options->input_dir).toString ();
  options->area_dir = settings.value (pfmExtract::tr ("area directory"), options->area_dir).toString ();

  options->window_width = settings.value (pfmExtract::tr ("width"), options->window_width).toInt ();
  options->window_height = settings.value (pfmExtract::tr ("height"), options->window_height).toInt ();
  options->window_x = settings.value (pfmExtract::tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (pfmExtract::tr ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}


void envout (OPTIONS *options)
{
  QSettings settings (pfmExtract::tr ("navo.navy.mil"), pfmExtract::tr ("pfmExtract"));

  settings.beginGroup (pfmExtract::tr ("pfmExtract"));


  settings.setValue (pfmExtract::tr ("settings version"), settings_version);


  settings.setValue (pfmExtract::tr ("data source"), options->source);
  settings.setValue (pfmExtract::tr ("data format"), options->format);
  settings.setValue (pfmExtract::tr ("reference data flag"), options->ref);
  settings.setValue (pfmExtract::tr ("orthometric correction flag"), options->geoid03);
  settings.setValue (pfmExtract::tr ("checked data flag"), options->chk);
  settings.setValue (pfmExtract::tr ("invert Z flag"), options->flp);
  settings.setValue (pfmExtract::tr ("utm flag"), options->utm);
  settings.setValue (pfmExtract::tr ("land flag"), options->lnd);
  settings.setValue (pfmExtract::tr ("uncertainty flag"), options->unc);
  settings.setValue (pfmExtract::tr ("cutoff flag"), options->cut);
  settings.setValue (pfmExtract::tr ("depth cutoff"), options->cutoff);
  settings.setValue (pfmExtract::tr ("datum shift"), options->datum_shift);
  settings.setValue (pfmExtract::tr ("file size limit"), options->size);
  settings.setValue (pfmExtract::tr ("input directory"), options->input_dir);
  settings.setValue (pfmExtract::tr ("area directory"), options->area_dir);


  settings.setValue (pfmExtract::tr ("width"), options->window_width);
  settings.setValue (pfmExtract::tr ("height"), options->window_height);
  settings.setValue (pfmExtract::tr ("x position"), options->window_x);
  settings.setValue (pfmExtract::tr ("y position"), options->window_y);

  settings.endGroup ();
}
