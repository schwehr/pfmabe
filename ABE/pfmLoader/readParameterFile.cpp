
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



#include "pfmLoader.hpp"



void get_dir_files (QStringList *input_files, QStringList *input_dirs)
{
  for (NV_INT32 i = 0 ; i < input_dirs->size () ; i++)
    {
      QStringList nameFilter;
      QString type = input_dirs->at (i).section (':', 0, 0);

      if (type == "GSF")
        {
          nameFilter << "*.d\?\?" << "*.gsf";
        }
      else if (type == "WLF")
        {
          nameFilter << "*.wlf" << "*.wtf" << "*.whf";
        }
      else if (type == "HAWKEYE")
        {
          nameFilter << "*.bin";
        }
      else if (type == "HOF")
        {
          nameFilter << "*.hof";
        }
      else if (type == "TOF")
        {
          nameFilter << "*.tof";
        }
      else if (type == "UNISIPS")
        {
          nameFilter << "*.u";
        }
      else if (type == "YXZ")
        { 
          nameFilter << "*.yxz" << "*.txt";
        }
      else if (type == "HYPACK")
        {
          nameFilter << "*.raw";
        }
      else if (type == "IVS XYZ")
        {
          nameFilter << "*.xyz";
        }
      else if (type == "LLZ")
        {
          nameFilter << "*.llz";
        }
      else if (type == "CZMIL")
        {
          nameFilter << "*.cxy";
        }
      else if (type == "DTED")
        {
          nameFilter << "*.dt1" << "*.dt2";
        }
      else if (type == "CHRTR")
        {
          nameFilter << "*.fin" << "*.ch2";
        }
      else if (type == "BAG")
        {
          nameFilter << "*.bag";
        }
      else
        {
          NV_CHAR atype[128];
          strcpy (atype, type.toAscii ());
          fprintf (stderr, "\n\nUnknown data type %s on [DIR] = field, ignoring!\n\n", atype);
          continue;
        }

      
      QString file = input_dirs->at (i).section (':', 1, 1).trimmed ();
      QDir dirs;
      dirs.cd (file);

      dirs.setFilter (QDir::Dirs | QDir::Readable);


      //  Get all matching files in this directory.

      QDir files;
      files.setFilter (QDir::Files | QDir::Readable);
      files.setNameFilters (nameFilter);


      if (files.cd (file))
        {
          QFileInfoList flist = files.entryInfoList ();
          for (NV_INT32 i = 0 ; i < flist.size () ; i++)
            {
              //  Don't load HOF timing lines.

              QString tst = flist.at (i).absoluteFilePath ();

              if (!nameFilter.contains ("*.hof") || tst.mid (tst.length () - 13, 4) != "_TA_")
                {
                  input_files->append (tst);
                }
            }
        }


      //  Get all directories in this directory.

      QFileInfoList dlist = dirs.entryInfoList ();
      QStringList dirList;
      for (NV_INT32 i = 0 ; i < dlist.size () ; i++)
        {
          if (dlist.at (i).fileName () != "." && dlist.at (i).fileName () != "..") 
            dirList.append (dlist.at (i).absoluteFilePath ());
        }


      //  Get all subordinate directories.

      for (NV_INT32 i = 0 ; i < dirList.size () ; i++)
        {
          QString dirName = dirList.at (i);

          if (dirs.cd (dirName))
            {
              QFileInfoList nlist = dirs.entryInfoList ();
              for (NV_INT32 i = 0 ; i < nlist.size () ; i++)
                {
                  if (nlist.at (i).fileName () != "." && nlist.at (i).fileName () != "..") 
                    dirList.append (nlist.at (i).absoluteFilePath ());
                }
            }
        }


      //  Get all matching files in all subordinate directories

      for (NV_INT32 i = 0 ; i < dirList.size () ; i++)
        {
          files.setFilter (QDir::Files | QDir::Readable);
          files.setNameFilters (nameFilter);

          QString dirName = dirList.at (i);

          if (files.cd (dirName))
            {
              QFileInfoList flist = files.entryInfoList ();
              for (NV_INT32 i = 0 ; i < flist.size () ; i++)
                {
                  //  Don't load HOF timing lines.

                  QString tst = flist.at (i).absoluteFilePath ();

                  if (!nameFilter.contains ("*.hof") || tst.mid (tst.length () - 13, 4) != "_TA_")
                    {
                      input_files->append (tst);
                    }
                }
            }
        }
    }
}



NV_BOOL readParameterFile (QString parameter_file, QStringList *input_files, PFM_DEFINITION *pfm_def, PFM_GLOBAL *pfm_global, FLAGS *flags)
{
  NV_CHAR file[1024];
  strcpy (file, parameter_file.toAscii ());
  FILE *fp;

  if ((fp = fopen (file, "r")) == NULL)
    {
      perror (file);
      fflush (stderr);
      return (NVFalse);
    }


  NV_CHAR string[1024];
  NV_INT32 pfm_index = 0;
  NV_BOOL input_started = NVFalse;
  NV_BOOL upr_started = NVFalse;
  QString qstring, cut, half_cut;
  QStringList input_dirs;


  pfm_global->attribute_count = 0;
  pfm_global->time_attribute_num = 0;


  pfm_global->upr_flag = NVFalse;
  if (parameter_file.endsWith (".upr")) pfm_global->upr_flag = NVTrue;


  while (fgets (string, sizeof (string), fp) != NULL)
    {
      qstring = string;


      //  We have a different end sentinel for .upr parameter files.

      if (pfm_global->upr_flag)
        {
          if (qstring.contains ("End Update Parameter File Input Files")) break;
        }
      else
        {
          if (qstring.contains ("End Input Files")) break;
        }


      if (qstring.contains ("PFM Definition "))
        {
          cut = qstring.section ("PFM Definition ", 1, 1);
          pfm_index = cut.mid (0, 3).toInt ();
        }


      //  The [PFM Directory] = string allows us to use old .prm files without barfing.
 
      if (qstring.contains ("[PFM Handle File] = ") || qstring.contains ("[PFM Directory] = "))
        {
          cut = qstring.section (" = ", 1, 1);
          pfm_def[pfm_index].name = cut.trimmed ();


          //  Try to open the file so we can set the existing flag if necessary.

          pfm_def[pfm_index].existing = NVFalse;
          PFM_OPEN_ARGS open_args;
          FILE *pfm_fp;
          open_args.checkpoint = 0;
          strcpy (open_args.list_path, pfm_def[pfm_index].name.toAscii ());

          if ((pfm_fp = fopen (open_args.list_path, "r")) != NULL)
            {
              fclose (pfm_fp);
              pfm_def[pfm_index].existing = NVTrue;
            }
        }


      if (qstring.contains ("[Bin Size Meters] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].mbin_size = cut.toFloat ();
        }

      if (qstring.contains ("[Bin Size Minutes] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].gbin_size = cut.toFloat ();
        }

      if (qstring.contains ("[Minimum Depth] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].min_depth = cut.toFloat ();
        }

      if (qstring.contains ("[Maximum Depth] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].max_depth = cut.toFloat ();
        }

      if (qstring.contains ("[Depth Precision] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].precision = cut.toFloat ();
        }

      if (qstring.contains ("[Polygon Latitude,Longitude] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          half_cut = cut.section (',', 0, 0);
          pfm_def[pfm_index].polygon[pfm_def[pfm_index].polygon_count].y = half_cut.toDouble ();
          half_cut = cut.section (',', 1, 1);
          pfm_def[pfm_index].polygon[pfm_def[pfm_index].polygon_count].x = half_cut.toDouble ();
          pfm_def[pfm_index].polygon_count++;
        }

      if (qstring.contains ("[Mosaic File] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].mosaic = cut.trimmed ();
        }

      if (qstring.contains ("[Feature File] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].feature = cut.trimmed ();
        }

      if (qstring.contains ("[Apply Area Filter Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].apply_area_filter = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Deep Filter Only Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].deep_filter_only = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Area Filter Bin Standard Deviation] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].cellstd = cut.toFloat ();
        }

      if (qstring.contains ("[Area Filter Feature Radius] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_def[pfm_index].radius = cut.toFloat ();
        }

      /*
      if (qstring.contains ("[Maximum Input Files] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_global->max_files = cut.toInt ();
        }

      if (qstring.contains ("[Maximum Input Lines] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_global->max_lines = cut.toInt ();
        }

      if (qstring.contains ("[Maximum Input Pings] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_global->max_pings = cut.toInt ();
        }

      if (qstring.contains ("[Maximum Input Beams] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_global->max_beams = cut.toInt ();
        }
      */


      if (qstring.contains ("[Cached Memory Size] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_global->cache_mem = cut.toInt ();
        }


      //  Check for the attribute flags which will be saved as [Attribute name] without an equals sign.

      if (qstring.contains (pfm_global->time_attribute_name))
        {
          pfm_global->attribute_count++;
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_global->time_attribute_num = cut.toInt ();
        }

      for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
        {
          if (qstring.contains (pfm_global->gsf_attribute_name[i]))
            {
              pfm_global->attribute_count++;
              cut = qstring.section (" = ", 1, 1).trimmed ();
              pfm_global->gsf_attribute_num[i] = cut.toInt ();
              break;
            }
        }

      for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
        {
          if (qstring.contains (pfm_global->hof_attribute_name[i]))
            {
              pfm_global->attribute_count++;
              cut = qstring.section (" = ", 1, 1).trimmed ();
              pfm_global->hof_attribute_num[i] = cut.toInt ();
              break;
            }
        }

      for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
        {
          if (qstring.contains (pfm_global->tof_attribute_name[i]))
            {
              pfm_global->attribute_count++;
              cut = qstring.section (" = ", 1, 1).trimmed ();
              pfm_global->tof_attribute_num[i] = cut.toInt ();
              break;
            }
        }

      for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
        {
          if (qstring.contains (pfm_global->wlf_attribute_name[i]))
            {
              pfm_global->attribute_count++;
              cut = qstring.section (" = ", 1, 1).trimmed ();
              pfm_global->wlf_attribute_num[i] = cut.toInt ();
              break;
            }
        }
          
      for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
        {
          if (qstring.contains (pfm_global->czmil_attribute_name[i]))
            {
              pfm_global->attribute_count++;
              cut = qstring.section (" = ", 1, 1).trimmed ();
              pfm_global->czmil_attribute_num[i] = cut.toInt ();
              break;
            }
        }

      for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
        {
          if (qstring.contains (pfm_global->bag_attribute_name[i]))
            {
              pfm_global->attribute_count++;
              cut = qstring.section (" = ", 1, 1).trimmed ();
              pfm_global->bag_attribute_num[i] = cut.toInt ();
              break;
            }
        }

      for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
        {
          if (qstring.contains (pfm_global->hawkeye_attribute_name[i]))
            {
              pfm_global->attribute_count++;
              cut = qstring.section (" = ", 1, 1).trimmed ();
              pfm_global->hawkeye_attribute_num[i] = cut.toInt ();
              break;
            }
        }


      if (qstring.contains ("[Check Files Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->chk = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Load GSF Nominal Depth Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->nom = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Insert CUBE Attributes Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->attr = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Run CUBE Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->cube = (NV_BOOL) (cut.toInt ());
          if (flags->cube) flags->attr = NVTrue;
        }

      if (qstring.contains ("[Reference Data Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->ref = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Invert Substitute Paths Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->sub = (NV_BOOL) (cut.toInt ());
        }



      if (qstring.contains ("[Load HOF GCS Compatible Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->old = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[HOF Load Null Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->hof = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Invalidate HOF Land Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->lnd = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Invalidate HOF Secondary Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->sec = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Reference TOF First Return Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->tof = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[HOF Standard User Flags Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->lid = (NV_BOOL) (cut.toInt ());
        }



      if (qstring.contains ("[Load Best SRTM Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->srtmb = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Load 1 Second SRTM Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->srtm1 = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Load 3 Second SRTM Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->srtm3 = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Load 30 Second SRTM Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->srtm30 = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Load SRTM As Reference Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->srtmr = (NV_BOOL) (cut.toInt ());
        }

      if (qstring.contains ("[Exclude SRTM2 Data Flag] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          flags->srtme = (NV_BOOL) (cut.toInt ());
        }



      if (qstring.contains ("[Default Horizontal Error] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_global->horizontal_error = cut.toInt ();
        }

      if (qstring.contains ("[Default Vertical Error] = "))
        {
          cut = qstring.section (" = ", 1, 1).trimmed ();
          pfm_global->vertical_error = cut.toInt ();
        }



      if (input_started)
        {
          //  Only use [DIR] = tagged input file lines if we are reading a .upr file.

          if (pfm_global->upr_flag)
            {
              if (qstring.contains ("[DIR:"))
                {
                  QString type = qstring.section (':', 1, 1).section (']', 0, 0).trimmed ();
                  
                  cut = qstring.section (" = ", 1, 1).trimmed ();

                  input_dirs += (type + ":" + cut);
                }
            }
          else
            {
              input_files->append (qstring.trimmed ());
            }
        }


      if (upr_started) pfm_global->upr_input_files += qstring.trimmed ();


      if (qstring.contains ("**  Input Files  **")) input_started = NVTrue;
      if (qstring.contains ("**  Update Parameter File Input Files  **")) upr_started = NVTrue;
    }


  if (input_dirs.size ()) get_dir_files (input_files, &input_dirs);


  fclose (fp);


  return (NVTrue);
}
