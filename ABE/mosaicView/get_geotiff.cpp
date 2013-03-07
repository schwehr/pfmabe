#include "mosaicViewDef.hpp"


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



NV_CHAR *get_geotiff (NV_CHAR *mosaic_file, MISC *misc)
{
  static NV_CHAR string[512];

  strcpy (string, "Success");


  if (strstr (mosaic_file, ".tif") || strstr (mosaic_file, ".TIF"))
    {
      GDALDataset     *poDataset;
      NV_FLOAT64      adfGeoTransform[6];


      GDALAllRegister ();


      poDataset = (GDALDataset *) GDALOpen (mosaic_file, GA_ReadOnly);
      if (poDataset != NULL)
        {
          if (poDataset->GetProjectionRef ()  != NULL)
            {
              QString projRef = QString (poDataset->GetProjectionRef ());

              if (projRef.contains ("GEOGCS"))
                {
                  if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
                    {
                      misc->lon_step = adfGeoTransform[1];
                      misc->lat_step = -adfGeoTransform[5];


                      misc->mosaic_width = poDataset->GetRasterXSize ();
                      misc->mosaic_height = poDataset->GetRasterYSize ();


                      misc->geotiff_area.min_x = adfGeoTransform[0];
                      misc->geotiff_area.max_y = adfGeoTransform[3];


                      misc->geotiff_area.min_y = misc->geotiff_area.max_y - misc->mosaic_height * misc->lat_step;
                      misc->geotiff_area.max_x = misc->geotiff_area.min_x + misc->mosaic_width * misc->lon_step;
                    }
                  else
                    {
                      delete poDataset;
                      sprintf (string, "File %s contains projected data", gen_basename (mosaic_file));
                      return (string);
                    }
                }
              else
                {
                  delete poDataset;
                  sprintf (string, "File %s contains Non-geographic coordinate system", gen_basename (mosaic_file));
                  return (string);
                }
            }
          else
            {
              delete poDataset;
              sprintf (string, "File %s contains no datum/projection information", gen_basename (mosaic_file));
              return (string);
            }

          if (misc->full_res_image != NULL) delete misc->full_res_image;


          /*  This is how I used to read geoTIFFs but for some reason it stopped working along about Qt 4.7.2 so now I use GDAL to
              read it.  I really need to keep testing to see if this starts working again because it's a bit faster and probably
              a lot more bullet proof.

          misc->full_res_image = new QImage (mosaic_file);

          if (misc->full_res_image == NULL || misc->full_res_image->width () == 0 || fmisc->ull_res_image->height () == 0)
            {
              sprintf (string, "Unable to read file %s", gen_basename (mosaic_file));
              delete poDataset;
              return (string);
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
              sprintf (string, "Not enough raster bands in geoTIFF");
              return (string);
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
                  sprintf (string, "Can only handle Byte data type");
                  return (string);
                }

              mult[i] = getColorOffset (colorInt[i]);
            }

          NV_INT32 nXSize = poBand[0]->GetXSize ();
          NV_INT32 nYSize = poBand[0]->GetYSize ();

          misc->full_res_image = new QImage (nXSize, nYSize, QImage::Format_ARGB32);
          if (misc->full_res_image == NULL || misc->full_res_image->width () == 0 || misc->full_res_image->height () == 0)
            {
              sprintf (string, "Unable to open image!");
              delete poDataset;
              return (string);
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
                  misc->full_res_image->setPixel (k, i, color[k]);
                }
            }

          delete (color);
          CPLFree (pafScanline);
          delete poDataset;
        }
      else
        {
          sprintf (string, "Unable to open file %s", gen_basename (mosaic_file));
          return (string);
        }
    }
  else
    {
      sprintf (string, "File %s is not a GeoTIFF file", gen_basename (mosaic_file));
      return (string);
    }


  return (string);
}
