#include "hotkeyPolygon.hpp"


/*  This is the sort function for qsort.  */

static NV_INT32 compare_file_numbers (const void *a, const void *b)
{
    SHARED_FILE_STRUCT *sa = (SHARED_FILE_STRUCT *)(a);
    SHARED_FILE_STRUCT *sb = (SHARED_FILE_STRUCT *)(b);

    return (sa->file < sb->file ? 0 : 1);
}



hotkeyPolygon::hotkeyPolygon (QWidget *parent, nvMapGL *ma, POINT_DATA *da, OPTIONS *op, MISC *mi, NV_INT32 prog, NV_BOOL *failed):
  QProcess (parent)
{
  NV_INT32 buildCommand (QString progString, QString actionString, POINT_DATA *data, MISC *misc, 
                         NV_INT32 nearest_point, QString *cmd, QStringList *args, NV_INT32 kill_switch);

  map = ma;
  options = op;
  misc = mi;
  data = da;
  pa = parent;



  //  We have to convert to NV_FLOAT64 so that the "inside" function will work.

  NV_FLOAT64 *mx = (NV_FLOAT64 *) malloc (misc->hotkey_poly_count * sizeof (NV_FLOAT64));

  if (mx == NULL)
    {
      perror ("Allocating mx array in hotkeyPolygon");
      exit (-1);
    }

  NV_FLOAT64 *my = (NV_FLOAT64 *) malloc (misc->hotkey_poly_count * sizeof (NV_FLOAT64));

  if (my == NULL)
    {
      perror ("Allocating my array in hotkeyPolygon");
      exit (-1);
    }

  for (NV_INT32 i = 0 ; i < misc->hotkey_poly_count ; i++)
    {
      mx[i] = (NV_FLOAT64) misc->hotkey_poly_x[i];
      my[i] = (NV_FLOAT64) misc->hotkey_poly_y[i];
    }

  NV_INT32 count = misc->hotkey_poly_count;


  //  Get rid of the hotkey polygon.

  misc->hotkey_poly_count = 0;
  free (misc->hotkey_poly_x);
  free (misc->hotkey_poly_y);
  misc->hotkey_poly_x = NULL;
  misc->hotkey_poly_y = NULL;



  NV_INT32 px = -1, py = -1, inc;
  static SHARED_FILE_STRUCT *shared_data = NULL;
  NV_FLOAT64 min_x = 999.0, min_y = 999.0, max_x = -999.0, max_y = -999.0;


  misc->statusProgLabel->setText (tr ("Reading..."));
  misc->statusProgLabel->setVisible (TRUE);
  misc->statusProg->setRange (0, data->count);
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  //  Don't let the progress bar eat up all the time.

  inc = data->count / 5;
  if (!inc) inc = 1;


  shared_count = 0;

  for (NV_INT32 j = 0 ; j < data->count ;  j++)
    {
      if (!(j % inc))
        {
          misc->statusProg->setValue (j);
          qApp->processEvents ();
        }


      if (!check_bounds (options, misc, data, j, NVTrue, misc->slice))
        {

          //  Convert the X, Y, and Z value to a projected pixel position

          map->get2DCoords (data->x[j], data->y[j], -data->z[j], &px, &py);


          //  Now check the point against the polygon.

          if (inside_polygon2 (mx, my, count, (NV_FLOAT64) px, (NV_FLOAT64) py))
            {
              //  We need to try to build a geographic MBR of the points that fall inside the polygon
              //  in case we want to run an external program that needs one.  Right now (11/14/08) that
              //  would only be tofWaterkill.

              min_x = qMin (min_x, data->x[j]);
              min_y = qMin (min_y, data->y[j]);
              max_x = qMax (max_x, data->x[j]);
              max_y = qMax (max_y, data->y[j]);


              if (options->data_type[prog][misc->data_type])
                {
                  shared_data = (SHARED_FILE_STRUCT *) realloc (shared_data, sizeof (SHARED_FILE_STRUCT) * (shared_count + 1));

                  if (shared_data == NULL)
                    {
                      perror ("Allocating shared file data in hotkeyPolygon.cpp");
                      exit (-1);
                    }

                  shared_data[shared_count].point = j;
                  shared_data[shared_count].x = data->x[j];
                  shared_data[shared_count].y = data->y[j];
                  shared_data[shared_count].z = data->z[j];
                  shared_data[shared_count].val = data->val[j];
                  shared_data[shared_count].rec = data->rec[j];
                  shared_data[shared_count].sub = data->sub[j];
                  shared_data[shared_count].exflag = NVFalse;

                  shared_count++;
                }
            }
        }
    }


  misc->statusProg->reset ();
  misc->statusProgLabel->setVisible (FALSE);
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents();


  if (shared_count)
    {
      //  Save the geographic minimum bounding rectangle for use in external ancillary programs.

      misc->abe_share->polygon_count = 4;
      misc->abe_share->polygon_x[0] = min_x;
      misc->abe_share->polygon_y[0] = min_y;
      misc->abe_share->polygon_x[1] = min_x;
      misc->abe_share->polygon_y[1] = max_y;
      misc->abe_share->polygon_x[2] = max_x;
      misc->abe_share->polygon_y[2] = max_y;
      misc->abe_share->polygon_x[3] = max_x;
      misc->abe_share->polygon_y[3] = min_y;


      qsort (shared_data, shared_count, sizeof (SHARED_FILE_STRUCT), compare_file_numbers);

      FILE *fp;
      misc->shared_file.sprintf ("GEOSWATH3D_EDIT_SHARED_FILE_%d.tmp", getpid ());

      if ((fp = fopen (misc->shared_file.toAscii (), "w")) != NULL)
        {
          NV_INT32 prev_f = -1;


          //  Don't let the progress bar eat up all the time.

          inc = shared_count / 5;
          if (!inc) inc = 1;


          misc->statusProgLabel->setText (tr ("Writing..."));
          misc->statusProgLabel->setVisible (TRUE);
          misc->statusProg->setRange (0, shared_count);
          misc->statusProg->setTextVisible (TRUE);
          qApp->processEvents();

          for (NV_INT32 i = 0 ; i < shared_count ; i++)
            {
              if (!(i % inc))
                {
                  misc->statusProg->setValue (i);
                  qApp->processEvents ();
                }


              if (!i || prev_f != shared_data[i].file)
                {
                  if (i) fprintf (fp, "EOD\n");

                  shared_data[i].file = 0;

                  NV_CHAR tmp[512];
                  strcpy (tmp, misc->file.toAscii ());
                  fprintf (fp, "%s\n", tmp);

                  prev_f = shared_data[i].file;
                }

              fprintf (fp, "%d %d %0.9f %0.9f %0.3f %0.3f %0.3f %d %d %d %d %d %d %d\n", shared_data[i].point, i, shared_data[i].x, 
                       shared_data[i].y, shared_data[i].z, 0.0, 0.0, shared_data[i].val, 0, 0, 0, shared_data[i].rec, shared_data[i].sub, 0);
            }

          fprintf (fp, "EOF\n");
          fclose (fp);


          misc->statusProg->reset ();
          misc->statusProgLabel->setVisible (FALSE);
          misc->statusProg->setTextVisible (FALSE);
          qApp->processEvents();


          misc->statusProgLabel->setText (tr ("Processing..."));
          misc->statusProgLabel->setVisible (TRUE);
          misc->statusProg->setRange (0, 0);
          misc->statusProg->setTextVisible (TRUE);
          qApp->processEvents();


          //  We need to strip out [INPUT_FILE] and [RECORD] and replace them with the shared file name and 
          //  --shared_file option.

          QString progString = options->prog[prog];

          progString.remove (QString ("[INPUT_FILE]"));
          progString.remove (QString ("[RECORD]"));


          //  Append the shared file stuff.

          progString.append (" --shared_file ");
          progString.append (misc->shared_file);


          QString actionString = options->action[prog];
          buildCommand (progString, actionString, data, misc, -999, &cmd, &args, -1);


          connect (this, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotDone (int, QProcess::ExitStatus)));
          connect (this, SIGNAL (readyReadStandardError ()), this, SLOT (slotReadyReadStandardError ()));
          connect (this, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotReadyReadStandardOutput ()));
          connect (this, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotError (QProcess::ProcessError)));

          this->start (cmd, args);
        }
      else
        {
          QString msg = tr ("Unable to open shared data file ") + misc->shared_file;
          QMessageBox::warning (parent, tr ("Ancillary Program"), msg);

          *failed = NVTrue;
        }


      free (shared_data);
      shared_data = NULL;
    }
  else
    {
      QString msg = tr ("No points found that match ancillary program data type for:\n") + options->name[prog];
      QMessageBox::warning (parent, tr ("geoSwath3D Ancillary Program"), msg);

      *failed = NVTrue;
    }

  free (mx);
  free (my);


  misc->statusProg->reset ();
  misc->statusProgLabel->setVisible (FALSE);
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents();
}



hotkeyPolygon::~hotkeyPolygon ()
{
}



void 
hotkeyPolygon::slotError (QProcess::ProcessError error)
{
  QString commandLine = cmd;
  for (NV_INT32 i = 0 ; i < args.size () ; i++) commandLine += (" " + args.at (i));

  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (pa, tr ("geoSwath3D polygon program"), tr ("Unable to start the polygon program!") + 
                             "\n" + commandLine);
      break;

    case QProcess::Crashed:
      QMessageBox::critical (pa, tr ("geoSwath3D polygon program"), tr ("The polygon program crashed!") + 
                             "\n" + commandLine);
      break;

    case QProcess::Timedout:
      QMessageBox::critical (pa, tr ("geoSwath3D polygon program"), tr ("The polygon program timed out!") + 
                             "\n" + commandLine);
      break;

    case QProcess::WriteError:
      QMessageBox::critical (pa, tr ("geoSwath3D polygon program"), 
                             tr ("There was a write error to the polygon program!") + "\n" + commandLine);
      break;

    case QProcess::ReadError:
      QMessageBox::critical (pa, tr ("geoSwath3D polygon program"),
                             tr ("There was a read error from the polygon program!") + "\n" + commandLine);
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (pa, tr ("geoSwath3D polygon program"), 
                             tr ("The polygon program died with an unknown error!") + "\n" + commandLine);
      break;
    }
}



void 
hotkeyPolygon::slotDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  FILE *fp;


  misc->statusProgLabel->setText (tr ("Retrieving..."));
  misc->statusProgLabel->setVisible (TRUE);
  misc->statusProg->setRange (0, shared_count);
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  if ((fp = fopen (misc->shared_file.toAscii (), "r")) != NULL)
    {
      NV_CHAR string[512];
      SHARED_FILE_STRUCT ret;
      NV_INT32 count, inc;
      NV_U_INT32 modified;
      NV_INT16 tmpi16;


      //  Don't let the progress dialog eat up all the time.

      inc = shared_count / 5;
      if (!inc) inc = 1;


      switch (misc->abe_share->modcode)
        {
        default:
        case NO_ACTION_REQUIRED:
          misc->abe_share->modcode = 0;
          break;


          //  From tofWaterKill.

        case PFMEDIT_FORCE_SHARE:
          while (fgets (string, sizeof (string), fp) != NULL)
            {
              //  EOF is the end of data sentinel

              if (strstr (string, "EOF"))
                {
                  end_undo_block (misc);
                  break;
                }


              //  If we forced the read there will be a NEW COUNT record at the beginning of the file.

              if (strstr (string, "NEW COUNT"))
                {
                  sscanf (string, "NEW COUNT = %d", &shared_count);
                  misc->statusProg->setRange (0, shared_count);
                  qApp->processEvents ();
                  inc = shared_count / 5;
                  if (!inc) inc = 1;
                }
              else
                {
                  sscanf (string, "%d %d %lf %lf %f %f %f %d %hd %hd %d %d %d %hd %d\n", &ret.point, &count, &ret.x, &ret.y, &ret.z,
                          &ret.herr, &ret.verr, &ret.val, &ret.pfm, &ret.file, &ret.line, &ret.rec, &ret.sub, &tmpi16, &modified);
                  ret.exflag = tmpi16;


                  for (NV_INT32 i = 0 ; i < data->count ; i++)
                    {
                      if (!check_bounds (options, misc, data, i, NVTrue, misc->slice))
                        {
                          if (modified && data->rec[i] == ret.rec && data->sub[i] == ret.sub)
                            {
                              //  Save the undo information.

                              store_undo (misc, options->undo_levels, data->val[i], i);

                              data->val[i] = ret.val;
                              break;
                            }
                        }
                    }
                }

              if (!(count % inc))
                {
                  misc->statusProg->setValue (count);
                  qApp->processEvents ();
                }
            }
          break;


          //  From hofReturnKill

        case PFM_CHARTS_HOF_DATA:
          while (fgets (string, sizeof (string), fp) != NULL)
            {
              //  EOF is the end of data sentinel

              if (strstr (string, "EOF"))
                {
                  end_undo_block (misc);
                  break;
                }


              sscanf (string, "%d %d %lf %lf %f %f %f %d %hd %hd %d %d %hd %d\n", &ret.point, &count, &ret.x, &ret.y, &ret.z,
                      &ret.herr, &ret.verr, &ret.val, &ret.pfm, &ret.file, &ret.rec, &ret.sub, &tmpi16, &modified);
              ret.exflag = tmpi16;

              if (modified)
                {
                  //  Save the undo information.

                  store_undo (misc, options->undo_levels, data->val[ret.point], ret.point);


                  data->val[ret.point] = ret.val;
                }


              if (!(count % inc))
                {
                  misc->statusProg->setValue (count);
                  qApp->processEvents ();
                }
            }
          break;
        }
    }

  misc->statusProg->reset ();
  misc->statusProgLabel->setVisible (FALSE);
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents();


  remove (misc->shared_file.toAscii ());


  //  Force a redraw of the waveMonitor just in case.

  misc->abe_share->modcode = WAVEMONITOR_FORCE_REDRAW;


  emit hotkeyPolygonDone ();
}



void 
hotkeyPolygon::slotReadyReadStandardError ()
{
  QByteArray response = this->readAllStandardError ();
  NV_CHAR *res = response.data ();


  //  Let's not print out the "QProcess: Destroyed while process is still running" messages that come from
  //  killing ancillary programs in the editor.

  if (!strstr (res, "Destroyed while"))
    {
      fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
      fflush (stderr);
    }
}



void 
hotkeyPolygon::slotReadyReadStandardOutput ()
{
  QByteArray response = this->readAllStandardOutput ();
  NV_CHAR *res = response.data ();

  fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}
