
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



#include "pfmLoad.hpp"

NV_BOOL writeParameterFile (QString parameter_file, QStringList input_files, PFM_DEFINITION *pfm_def, 
                            PFM_GLOBAL pfm_global, FLAGS flags)
{
  NV_CHAR fname [1024];
  strcpy (fname, parameter_file.toAscii ());


  FILE *fp;
  if ((fp = fopen (fname, "w")) == NULL) return (NVFalse);

  NV_CHAR tmp[1024];


  //  If the output filename has a .upr extension we're saving a pfmLoader "update" parameters file.
  //  In that case we need to put a warning at the top of the file.

  if (parameter_file.endsWith (".upr"))
    {
      fprintf (fp, "\n\n** WARNING WARNING WARNING WARNING WARNING WARNING WARNING **\n");
      fprintf (fp, "*                                                           *\n");
      fprintf (fp, "*  This is an 'update' parameter file (.upr).  Please see   *\n");
      fprintf (fp, "*  IMPORTANT NOTE below if you manually edit this file.     *\n");
      fprintf (fp, "*                                                           *\n");
      fprintf (fp, "** WARNING WARNING WARNING WARNING WARNING WARNING WARNING **\n\n\n");
    }


  for (NV_INT32 i = 0 ; i < MAX_LOAD_FILES ; i++)
    {
      if (!pfm_def[i].name.isEmpty ())
        {
          if (!pfm_def[i].name.endsWith (".pfm")) pfm_def[i].name.append (".pfm");

          fprintf (fp, "\n************  PFM Definition %d  ************\n", i);
          strcpy (tmp, pfm_def[i].name.toAscii ());
          fprintf (fp, "[PFM Handle File] = %s\n", tmp);
          fprintf (fp, "[Bin Size Meters] = %f\n", pfm_def[i].mbin_size);
          fprintf (fp, "[Bin Size Minutes] = %f\n", pfm_def[i].gbin_size);
          fprintf (fp, "[Minimum Depth] = %f\n", pfm_def[i].min_depth);
          fprintf (fp, "[Maximum Depth] = %f\n", pfm_def[i].max_depth);
          fprintf (fp, "[Depth Precision] = %f\n", pfm_def[i].precision);


          //  Convert the header polygon to points so that losing or changing the area file won't effect the .prm file.

          for (NV_INT32 j = 0 ; j < pfm_def[i].open_args.head.polygon_count ; j++)
            {
              fprintf (fp, "[Polygon Latitude,Longitude] = %.11f,%.11f\n", pfm_def[i].open_args.head.polygon[j].y, pfm_def[i].open_args.head.polygon[j].x);
            }


          strcpy (tmp, pfm_def[i].mosaic.toAscii ());
          fprintf (fp, "[Mosaic File] = %s\n", tmp);
          strcpy (tmp, pfm_def[i].feature.toAscii ());
          fprintf (fp, "[Feature File] = %s\n", tmp);
          fprintf (fp, "[Apply Area Filter Flag] = %d\n", pfm_def[i].apply_area_filter);
          fprintf (fp, "[Deep Filter Only Flag] = %d\n", pfm_def[i].deep_filter_only);
          fprintf (fp, "[Area Filter Bin Standard Deviation] = %f\n", pfm_def[i].cellstd);
          fprintf (fp, "[Area Filter Feature Radius] = %f\n", pfm_def[i].radius);
          fprintf (fp, "************  End Definition  ************\n");
        }
    }

  fprintf (fp, "\n************  PFM Global Options  ************\n");

  /*
  fprintf (fp, "[Maximum Input Files] = %d\n", pfm_global.max_files);
  fprintf (fp, "[Maximum Input Lines] = %d\n", pfm_global.max_lines);
  fprintf (fp, "[Maximum Input Pings] = %d\n", pfm_global.max_pings);
  fprintf (fp, "[Maximum Input Beams] = %d\n", pfm_global.max_beams);
  */

  fprintf (fp, "[Cached Memory Size] = %d\n", pfm_global.cache_mem);

  fprintf (fp, "[Check Files Flag] = %d\n", flags.chk);
  fprintf (fp, "[Load GSF Nominal Depth Flag] = %d\n", flags.nom);
  fprintf (fp, "[Insert CUBE Attributes Flag] = %d\n", flags.attr);
  fprintf (fp, "[Run CUBE Flag] = %d\n", flags.cube);
  fprintf (fp, "[Reference Data Flag] = %d\n", flags.ref);
  fprintf (fp, "[Invert Substitute Paths Flag] = %d\n", flags.sub);

  fprintf (fp, "[Load HOF GCS Compatible Flag] = %d\n", flags.old);
  fprintf (fp, "[HOF Load Null Flag] = %d\n", flags.hof);
  fprintf (fp, "[Invalidate HOF Land Flag] = %d\n", flags.lnd);
  fprintf (fp, "[Invalidate HOF Secondary Flag] = %d\n", flags.sec);
  fprintf (fp, "[Reference TOF First Return Flag] = %d\n", flags.tof);
  fprintf (fp, "[HOF Standard User Flags Flag] = %d\n", flags.lid);

  fprintf (fp, "[Load Best SRTM Flag] = %d\n", flags.srtmb);
  fprintf (fp, "[Load 1 Second SRTM Flag] = %d\n", flags.srtm1);
  fprintf (fp, "[Load 3 Second SRTM Flag] = %d\n", flags.srtm3);
  fprintf (fp, "[Load 30 Second SRTM Flag] = %d\n", flags.srtm30);
  fprintf (fp, "[Load SRTM As Reference Flag] = %d\n", flags.srtmr);
  fprintf (fp, "[Exclude SRTM2 Data Flag] = %d\n", flags.srtme);

  fprintf (fp, "[Default Horizontal Error] = %f\n", pfm_global.horizontal_error);
  fprintf (fp, "[Default Vertical Error] = %f\n", pfm_global.vertical_error);

  fprintf (fp,   "**********  End PFM Global Options  **********\n");


  if (pfm_global.attribute_count)
    {
      fprintf (fp, "\n************  Index Attribute Types  ************\n");

      NV_CHAR attr[128];

      if (pfm_global.time_attribute_num)
        {
          strcpy (attr, pfm_global.time_attribute_name.toAscii ());
          fprintf (fp, "[%s] = %d\n", attr, pfm_global.time_attribute_num);
        }

      for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
        {
          if (pfm_global.gsf_attribute_num[i])
            {
              strcpy (attr, pfm_global.gsf_attribute_name[i].toAscii ());
              fprintf (fp, "[%s] = %d\n", attr, pfm_global.gsf_attribute_num[i]);
            }
        }

      for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
        {
          if (pfm_global.hof_attribute_num[i])
            {
              strcpy (attr, pfm_global.hof_attribute_name[i].toAscii ());
              fprintf (fp, "[%s] = %d\n", attr, pfm_global.hof_attribute_num[i]);
            }
        }

      for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
        {
          if (pfm_global.tof_attribute_num[i])
            {
              strcpy (attr, pfm_global.tof_attribute_name[i].toAscii ());
              fprintf (fp, "[%s] = %d\n", attr, pfm_global.tof_attribute_num[i]);
            }
        }

      for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
        {
          if (pfm_global.wlf_attribute_num[i])
            {
              strcpy (attr, pfm_global.wlf_attribute_name[i].toAscii ());
              fprintf (fp, "[%s] = %d\n", attr, pfm_global.wlf_attribute_num[i]);
            }
        }

      for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
        {
          if (pfm_global.czmil_attribute_num[i])
            {
              strcpy (attr, pfm_global.czmil_attribute_name[i].toAscii ());
              fprintf (fp, "[%s] = %d\n", attr, pfm_global.czmil_attribute_num[i]);
            }
        }

      for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
        {
          if (pfm_global.bag_attribute_num[i])
            {
              strcpy (attr, pfm_global.bag_attribute_name[i].toAscii ());
              fprintf (fp, "[%s] = %d\n", attr, pfm_global.bag_attribute_num[i]);
            }
        }

      for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
        {
          if (pfm_global.hawkeye_attribute_num[i])
            {
              strcpy (attr, pfm_global.hawkeye_attribute_name[i].toAscii ());
              fprintf (fp, "[%s] = %d\n", attr, pfm_global.hawkeye_attribute_num[i]);
            }
        }

      fprintf (fp, "************  End Index Attribute Types  ************\n");
    }

  fprintf (fp, "\n************  Input Files  ************\n");


  //  If the output filename has a .upr extension we're saving a pfmLoader "update" parameters file.

  if (parameter_file.endsWith (".upr"))
    {
      for (NV_INT32 i = 0 ; i < pfm_global.input_dirs.size () ; i++)
        {
          NV_CHAR type[128];
          strcpy (type, pfm_global.input_dirs.at (i).section (':', 0, 0).toAscii ());
          strcpy (tmp, pfm_global.input_dirs.at (i).section (':', 1, 1).toAscii ());
          fprintf (fp, "[DIR:%s] = %s\n", type, tmp);
        }

      fprintf (fp, "************  End Input Files  ************\n");

      fprintf (fp, "\n\n*******************  IMPORTANT NOTE  ************************\n");
      fprintf (fp, "*                                                           *\n");
      fprintf (fp, "*  If you manually modify this file you must remove all of  *\n");
      fprintf (fp, "*  the following file names so that they will be rescanned. *\n");
      fprintf (fp, "*                                                           *\n");
      fprintf (fp, "************  Update Parameter File Input Files  ************\n");
      fprintf (fp, "**********  End Update Parameter File Input Files  **********\n");
    }
  else
    {
      for (NV_INT32 i = 0 ; i < input_files.size () ; i++)
        {
          strcpy (tmp, input_files.at (i).toAscii ());
          fprintf (fp, "%s\n", tmp);
        }

      fprintf (fp, "************  End Input Files  ************\n");
    }


  fclose (fp);


  return (NVTrue);
}
