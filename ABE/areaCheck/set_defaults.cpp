#include "areaCheck.hpp"

void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore)
{
  //  Set Defaults so that if keys don't for envin the parms are defined.

  if (!restore)
    {
      misc->poly_count = 0;
      misc->drawing_canceled = NVFalse;
      misc->drawing = NVFalse;
      misc->prev_mbr.min_x = 999.0;

      for (NV_INT32 k = 0 ; k < NUM_TYPES ; k++)
        {
          misc->overlays[k] = NULL;
          misc->num_overlays[k] = 0;
        }

      misc->type_name[SHAPE] = areaCheck::tr ("Shape");
      misc->type_name[ISS60_AREA] = areaCheck::tr ("ISS60 Area");
      misc->type_name[GENERIC_AREA] = areaCheck::tr ("Generic Area");
      misc->type_name[ACE_AREA] = areaCheck::tr ("Army Corps Area");
      misc->type_name[ISS60_ZONE] = areaCheck::tr ("ISS60 Zone");
      misc->type_name[GENERIC_ZONE] = areaCheck::tr ("Generic Zone");
      misc->type_name[TRACK] = areaCheck::tr ("Trackline");
      misc->type_name[FEATURE] = areaCheck::tr ("Feature");
      misc->type_name[YXZ] = areaCheck::tr ("YXZ");
      misc->type_name[LLZ_DATA] = areaCheck::tr ("LLZ");
      misc->type_name[GEOTIFF] = areaCheck::tr ("GeoTIFF");
      misc->type_name[CHRTR] = areaCheck::tr ("CHRTR");
      misc->type_name[BAG] = areaCheck::tr ("BAG");
      misc->type_name[OPTECH_FLT] = areaCheck::tr ("Optech flight");

      misc->width = 950;
      misc->height = 750;
      misc->window_x = 0;
      misc->window_y = 0;

      misc->linked = NVFalse;
    }


  options->coast = NVTrue;
  options->mask = NVFalse;
  options->position_form = 0;
  options->inputFilter = areaCheck::tr ("AREA (*.ARE *.are *.afs)");
  options->outputFilter = areaCheck::tr ("Generic area file (*.are)");
  options->coast_color = Qt::black;
  options->mask_color = QColor (170, 85, 0);
  options->background_color = Qt::white;
  options->color[ISS60_AREA] = QColor (0, 170, 0);
  options->color[GENERIC_AREA] = QColor (0, 170, 0);
  options->color[ACE_AREA] = QColor (0, 170, 0);
  options->color[ISS60_ZONE] = QColor (0, 0, 255);
  options->color[GENERIC_ZONE] = QColor (0, 0, 255);
  options->color[SHAPE] = QColor (255, 0, 255, 127);
  options->color[TRACK] = QColor (0, 0, 0);
  options->color[FEATURE] = QColor (0, 0, 255);
  options->color[YXZ] = QColor (0, 0, 0);
  options->color[LLZ_DATA] = QColor (0, 85, 255);
  options->color[GEOTIFF] = QColor (255, 0, 0, 255);
  options->color[CHRTR] = QColor (0, 255, 0, 255);
  options->color[BAG] = QColor (0, 0, 255, 255);
  options->highlight_color = Qt::yellow;
  options->display_children = NVTrue;
  options->display_feature_info = NVFalse;
  options->display_feature_poly = NVFalse;
  options->stoplight = NVFalse;
  options->minstop = 1.5;
  options->maxstop = 2.0;
  options->sunopts.azimuth = 30.0;
  options->sunopts.elevation = 30.0;
  options->sunopts.exag = 1.0;
  options->sunopts.power_cos = 1.0;
  options->sunopts.num_shades = NUMSHADES;
  options->input_dir = ".";
  options->output_dir = ".";
}
