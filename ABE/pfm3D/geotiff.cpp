#include "pfm3D.hpp"

static QImage      *subImage = NULL;


NV_U_INT32 getColorOffset (QString colorInt)
{
  static NV_U_INT32 mult;

  if (colorInt.contains ("Alpha"))
    {
      mult = 0x1000000;
    }
  else if (colorInt.contains ("Red"))
    {
      mult = 0x10000;
    }
  else if (colorInt.contains ("Green"))
    {
      mult = 0x100;
    }
  else if (colorInt.contains ("Blue"))
    {
      mult = 0x1;
    }

  return (mult);
}



QImage *geotiff (NV_F64_XYMBR mbr, NV_CHAR *geotiff_file, NV_F64_XYMBR *geotiff_mbr)
{
  NV_FLOAT64         GeoTIFF_wlon, GeoTIFF_nlat, GeoTIFF_lon_step, GeoTIFF_lat_step, GeoTIFF_elon, GeoTIFF_slat;
  NV_INT32           width, height, start_y, end_y, start_x, end_x;
  QImage             *full_res_image;
  GDALDataset        *poDataset;
  NV_FLOAT64         adfGeoTransform[6];
  static NV_INT32    powers_of_two[12] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};


  GDALAllRegister ();


  poDataset = (GDALDataset *) GDALOpen (geotiff_file, GA_ReadOnly);
  if (poDataset != NULL)
    {
      if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
        {
          GeoTIFF_lon_step = adfGeoTransform[1];
          GeoTIFF_lat_step = -adfGeoTransform[5];


          width = poDataset->GetRasterXSize ();
          height = poDataset->GetRasterYSize ();


          GeoTIFF_wlon = adfGeoTransform[0];
          GeoTIFF_nlat = adfGeoTransform[3];


          GeoTIFF_slat = GeoTIFF_nlat - height * GeoTIFF_lat_step;
          GeoTIFF_elon = GeoTIFF_wlon + width * GeoTIFF_lon_step;


          if (mbr.max_y >= GeoTIFF_slat && mbr.min_y <= GeoTIFF_nlat && mbr.max_x >= GeoTIFF_wlon && mbr.min_x <= GeoTIFF_elon)
            {
              /*  This is how I used to read geoTIFFs but for some reason it stopped working along about Qt 4.7.2 so now I use GDAL to
                  read it.  I really need to keep testing to see if this starts working again because it's a bit faster and probably
                  a lot more bullet proof.


              full_res_image = new QImage (geotiff_file, "tiff");
              if (full_res_image == NULL || full_res_image->width () == 0 || full_res_image->height () == 0)
                {
                  QMessageBox::critical (0, "pfm3D", pfm3D::tr ("Unable to open image!"));
                  delete poDataset;
                  return (NULL);
                }
              */


              //  This is how I read geoTIFFs now...

              GDALRasterBand *poBand[4];
              QString dataType[4], colorInt[4];
              NV_U_INT32 mult[4] = {0, 0, 0, 0};


              NV_INT32 rasterCount = poDataset->GetRasterCount ();
              if (rasterCount < 3)
                {
                  delete poDataset;
                  QMessageBox::critical (0, "pfm3D", pfm3D::tr ("Not enough raster bands in geoTIFF"));
                  return (NULL);
                }

              for (NV_INT32 i = 0 ; i < rasterCount ; i++)
                {
                  poBand[i] = poDataset->GetRasterBand (i + 1);

                  dataType[i] = QString (GDALGetDataTypeName (poBand[i]->GetRasterDataType ()));
                  colorInt[i] = QString (GDALGetColorInterpretationName (poBand[i]->GetColorInterpretation ()));


                  //  We can only handle Byte data (i.e. RGB or ARGB)

                  if (dataType[i] != "Byte")
                    {
                      delete poDataset;
                      QMessageBox::critical (0, "pfm3D", pfm3D::tr ("Cannot handle %1 data type").arg (dataType[i]));
                      return (NULL);
                    }

                  mult[i] = getColorOffset (colorInt[i]);
                }

              NV_INT32 nXSize = poBand[0]->GetXSize ();
              NV_INT32 nYSize = poBand[0]->GetYSize ();

              full_res_image = new QImage (nXSize, nYSize, QImage::Format_ARGB32);
              if (full_res_image == NULL || full_res_image->width () == 0 || full_res_image->height () == 0)
                {
                  QMessageBox::critical (0, "pfm3D", pfm3D::tr ("Unable to open image!"));
                  delete poDataset;
                  return (NULL);
                }

              NV_U_INT32 *color = new NV_U_INT32[nXSize];
              NV_U_BYTE *pafScanline = (NV_U_BYTE *) CPLMalloc (sizeof (NV_U_BYTE) * nXSize);

              for (NV_INT32 i = 0 ; i < nYSize ; i++)
                {
                  //  If we don't have an alpha band set it to 255.

                  for (NV_INT32 k = 0 ; k < nXSize ; k++)
                    {
                      if (rasterCount < 4)
                        {
                          color[k] = 0xff000000;
                        }
                      else
                        {
                          color[k] = 0x0;
                        }
                    }


                  //  Read the raster bands.

                  for (NV_INT32 j = 0 ; j < rasterCount ; j++)
                    {
                      poBand[j]->RasterIO (GF_Read, 0, i, nXSize, 1, pafScanline, nXSize, 1, GDT_Byte, 0, 0);
                      for (NV_INT32 k = 0 ; k < nXSize ; k++) color[k] += ((NV_U_INT32) pafScanline[k]) * mult[j];
                    }


                  //  Set the image pixels.

                  for (NV_INT32 k = 0 ; k < nXSize ; k++)
                    {
                      full_res_image->setPixel (k, i, color[k]);
                    }
                }

              delete (color);
              CPLFree (pafScanline);


              //  Set the bounds

              if (mbr.min_y < GeoTIFF_slat)
                {
                  start_y = 0;
                  geotiff_mbr->min_y = GeoTIFF_slat;
                }
              else
                {
                  start_y = (NV_INT32) ((GeoTIFF_nlat - mbr.max_y) / GeoTIFF_lat_step);
                  geotiff_mbr->min_y = mbr.min_y;
                }
              if (mbr.max_y > GeoTIFF_nlat)
                {
                  end_y = height;
                  geotiff_mbr->max_y = GeoTIFF_nlat;
                }
              else
                {
                  end_y = (NV_INT32) ((GeoTIFF_nlat - mbr.min_y) / GeoTIFF_lat_step);
                  geotiff_mbr->max_y = mbr.max_y;
                }

              if (mbr.min_x < GeoTIFF_wlon)
                {
                  start_x = 0;
                  geotiff_mbr->min_x = GeoTIFF_wlon;
                }
              else
                {
                  start_x = (NV_INT32) ((mbr.min_x - GeoTIFF_wlon) / GeoTIFF_lon_step);
                  geotiff_mbr->min_x = mbr.min_x;
                }
              if (mbr.max_x > GeoTIFF_elon)
                {
                  end_x = width;
                  geotiff_mbr->max_x = GeoTIFF_elon;
                }
              else
                {
                  end_x = (NV_INT32) ((mbr.max_x - GeoTIFF_wlon) / GeoTIFF_lon_step);
                  geotiff_mbr->max_x = mbr.max_x;
                }


              if (start_y < 0)
                {
                  end_y = (NV_INT32) ((GeoTIFF_nlat - geotiff_mbr->min_y) / GeoTIFF_lat_step);
                  start_y = 0;
                }

              if (end_y > height)
                {
                  end_y = height;
                  start_y = (NV_INT32) ((GeoTIFF_nlat - geotiff_mbr->max_y) / GeoTIFF_lat_step);
                }


              //  Number of rows and columns of the GeoTIFF to paint.

              NV_INT32 rows = end_y - start_y;
              NV_INT32 cols = end_x - start_x;


              //  I'm limiting the overlay to 2048 by 2048 max.

              NV_INT32 wide = powers_of_two[11], high = powers_of_two[11];

              for (NV_INT32 i = 0 ; i < 12 ; i++)
                {
                  if (powers_of_two[i] >= cols)
                    {
                      wide = powers_of_two[i];
                      break;
                    }
                }

              for (NV_INT32 i = 0 ; i < 12 ; i++)
                {
                  if (powers_of_two[i] >= rows)
                    {
                      high = powers_of_two[i];
                      break;
                    }
                }


              //  First make sure we have enough resources to handle the texture.

              glTexImage2D (GL_PROXY_TEXTURE_2D, 0, GL_RGBA, wide, high, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

              NV_INT32 w, h, d;
              glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
              glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
              glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_DEPTH, &d);

              if (!w || !h || !d)
                {
                  QMessageBox::critical (0, "pfm3D", pfm3D::tr ("The image is too large for OpenGL (%1 X %2)").arg (wide).arg (high));
                  delete full_res_image;
                  delete poDataset;
                  return (NULL);
                }


              //  Have to convert to a power of two prior to using as an OpenGL texture.  We're also mirroring 
              //  in the y direction (OpenGL Z direction).

              subImage = new QImage (full_res_image->copy (start_x, start_y, cols, rows).scaled
                                     (wide, high, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).mirrored
                                     (FALSE, TRUE));

              delete full_res_image;
            }
          delete poDataset;
        }
    }
  else
    {
      QMessageBox::warning (0, "pfm3D", pfm3D::tr ("Unable to read tiff file."));
    }

  return (subImage);
}



void geotiff_clear ()
{
  if (subImage != NULL) delete subImage;
  subImage = NULL;
}
