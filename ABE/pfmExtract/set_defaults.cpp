#include "pfmExtract.hpp"

void set_defaults (OPTIONS *options)
{
  options->source = 3;
  options->format = 1;
  options->ref = NVFalse;
  options->geoid03 = NVFalse;
  options->chk = NVFalse;
  options->flp = NVFalse;
  options->utm = NVFalse;
  options->lnd = NVFalse;
  options->cut = NVFalse;
  options->cutoff = 0.0;
  options->datum_shift = 0.0;
  options->size = 0;
  options->input_dir = ".";
  options->area_dir = ".";

  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 870;
  options->window_height = 460;
}
