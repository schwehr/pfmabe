#include "trackLine.hpp"

void set_defaults (OPTIONS *options)
{
  options->window_x = 0;
  options->window_y = 0;
  options->width = 800;
  options->height = 600;
  options->inputFilter = trackLine::tr ("GSF (*.d\?\? *.gsf)");
  options->heading[TRACKLINE_GSF] = 3.0;
  options->speed[TRACKLINE_GSF] = 20.0;
  options->heading[TRACKLINE_POS] = 5.0;
  options->speed[TRACKLINE_POS] = 240.0;
  options->heading[TRACKLINE_WLF] = 5.0;
  options->speed[TRACKLINE_WLF] = 240.0;
  options->heading[TRACKLINE_HWK] = 5.0;
  options->speed[TRACKLINE_HWK] = 240.0;
}
