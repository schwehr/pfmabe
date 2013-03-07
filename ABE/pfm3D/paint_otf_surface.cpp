#include "pfm3D.hpp"

void paint_otf_surface (pfm3D *parent, MISC *misc, nvMapGL *map, NV_BOOL reset)
{
  static NV_F64_XYMBR prev_mbr = {-999.0, -999.0, -999.0, -999.0};
  static NV_FLOAT64 prev_min_z = CHRTRNULL, prev_max_z = -CHRTRNULL;
  static NV_INT32 prev_display_GeoTIFF = 0;
  NV_FLOAT32 **data = NULL;
  NV_F64_XYMBC mbc;


  QImage *geotiff (NV_F64_XYMBR mbr, NV_CHAR *geotiff_file, NV_F64_XYMBR *geotiff_mbr);
  void geotiff_clear ();


  //  Make an MBC from the MBR and the data.

  mbc.min_x = misc->abe_share->viewer_displayed_area.min_x;
  mbc.min_y = misc->abe_share->viewer_displayed_area.min_y;
  mbc.max_x = misc->abe_share->viewer_displayed_area.max_x;
  mbc.max_y = misc->abe_share->viewer_displayed_area.max_y;


  //  Flip the min and max.

  mbc.max_z = -misc->abe_share->otf_min_z;
  mbc.min_z = -misc->abe_share->otf_max_z;


  //  Have to set the bounds prior to doing anything

  if (prev_mbr.min_x != misc->abe_share->viewer_displayed_area.min_x || prev_mbr.min_y != misc->abe_share->viewer_displayed_area.min_y ||
      prev_mbr.max_x != misc->abe_share->viewer_displayed_area.max_x || prev_mbr.max_y != misc->abe_share->viewer_displayed_area.max_y ||
      mbc.min_z != prev_min_z || mbc.max_z != prev_max_z)
    {
      map->setBounds (mbc);

      if (reset)
        {
          //  Get the map center so we can reset the view.

          map->getMapCenter (&misc->map_center_x, &misc->map_center_y, &misc->map_center_z);


          map->setMapCenter (misc->map_center_x, misc->map_center_y, misc->map_center_z);

          map->resetPOV ();
        }
    }


  //  If we want to display a GeoTIFF we must load the texture, unless it (or the displayed area) hasn't changed.

  if (misc->display_GeoTIFF && ((prev_mbr.min_x != misc->abe_share->viewer_displayed_area.min_x ||
                                 prev_mbr.min_y != misc->abe_share->viewer_displayed_area.min_y ||
                                 prev_mbr.max_x != misc->abe_share->viewer_displayed_area.max_x ||
                                 prev_mbr.max_y != misc->abe_share->viewer_displayed_area.max_y ||
                                 mbc.min_z != prev_min_z || mbc.max_z != prev_max_z ||
                                 misc->display_GeoTIFF != prev_display_GeoTIFF) || misc->GeoTIFF_init))
    {
      NV_F64_XYMBR geotiff_mbr;
      QImage *subImage = geotiff (misc->abe_share->viewer_displayed_area, misc->GeoTIFF_name, &geotiff_mbr);
      if (subImage != NULL)
        {
          map->setGeotiffTexture (subImage, geotiff_mbr, misc->display_GeoTIFF);
          geotiff_clear ();
          misc->GeoTIFF_init = NVFalse;
        }
      prev_display_GeoTIFF = misc->display_GeoTIFF;
    }
  prev_mbr = misc->displayed_area = misc->abe_share->viewer_displayed_area;
  prev_min_z = mbc.min_z;
  prev_max_z = mbc.max_z;


  //  Clear ALL of the data layers prior to loading.

  for (NV_INT32 pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++) map->clearDataLayer (pfm);


  //  Now we load the otf grid from shared memory.

  QString skey;
  skey.sprintf ("%d_abe_otf_grid", misc->abe_share->ppid);

  misc->otfShare = new QSharedMemory (skey);

  if (!misc->otfShare->attach (QSharedMemory::ReadWrite))
    {
      QMessageBox::critical (parent, pfm3D::tr ("pfm3D"), pfm3D::tr ("Cannot attach the OTF grid shared memory - %1").arg (skey));
      return;
    }


  misc->otf_grid = (OTF_GRID_RECORD *) misc->otfShare->data ();


  NV_INT32 layer_type = 0;
  switch (misc->abe_share->layer_type)
    {
    case AVERAGE_FILTERED_DEPTH:
    case AVERAGE_DEPTH:
      layer_type = 0;
      break;

    case MIN_FILTERED_DEPTH:
    case MIN_DEPTH:
      layer_type = -1;
      break;

    case MAX_FILTERED_DEPTH:
    case MAX_DEPTH:
      layer_type = 1;
      break;
    }


  //  I hate allocating the memory for this again but we have to flip the sign for the Z values.  On the 
  //  bright side, we free it up after we give it to OpenGL.

  data = (NV_FLOAT32 **) malloc (sizeof (NV_FLOAT32 *) * misc->abe_share->otf_height);
  if (data == NULL) 
    {
      perror ("Allocating data memory in paint_otf_surface");
      parent->clean_exit (-1);
    }


  for (NV_INT32 i = 0 ; i < misc->abe_share->otf_height ; i++)
    {
      data[i] = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * misc->abe_share->otf_width);
      if (data[i] == NULL) 
        {
          perror ("Allocating data[i] memory in paint_otf_surface");
          parent->clean_exit (-1);
        }

      if (layer_type < 0)
        {
          for (NV_INT32 j = 0 ; j < misc->abe_share->otf_width ; j++) data[i][j] = -misc->otf_grid[i * misc->abe_share->otf_width + j].min;
        }
      else
        {
          for (NV_INT32 j = 0 ; j < misc->abe_share->otf_width ; j++) data[i][j] = -misc->otf_grid[i * misc->abe_share->otf_width + j].max;
        }
    }


  //  Let go of the shared OTF memory.

  misc->otfShare->detach ();

  map->setDataLayer (0, data, NULL, 0, 0, misc->abe_share->otf_height, misc->abe_share->otf_width, misc->abe_share->otf_y_bin_size,
                     misc->abe_share->otf_x_bin_size, -misc->abe_share->otf_null_value, misc->displayed_area, layer_type);

  for (NV_INT32 i = 0 ; i < misc->abe_share->otf_height ; i++) free (data[i]);

  free (data);
}
