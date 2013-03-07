#include "geoSwath3D.hpp"

/*

    Ancillary program command line substitutions:


    [INPUT_FILE] - input data file associated with the current point
    [SHARED_MEMORY_ID] - ABE shared memory ID (some programs like chartsPic require this)
    [Z_VALUE] - Z value of the current point
    [X_VALUE] - X value (usually longitude) associated with the current point
    [Y_VALUE] - Y value (usually latitude) associated with the current point
    [MIN_Y] - minimum Y value in the currently displayed area
    [MIN_X] - minimum X value in the currently displayed area
    [MAX_Y] - maximum Y value in the currently displayed area
    [MAX_X] - maximum X value in the currently displayed area
    [VALIDITY] - PFM validity word for the current point
    [RECORD] - input file record number associated with the current point
    [SUBRECORD] - input file subrecord (usually beam) number associated with the current point

    [CL] - run this program as a command line program (creates a dialog for output)
    [SHARED_MEMORY_KEY] - Add the shared memory ID so the outboard program can track it
    [KILL_SWITCH] - Add a kill switch value so that we can set the shared key to kill the outboard program.
                    This option requires [SHARED_MEMORY_KEY] but not all shared programs will have a kill switch.
                    In fact, only "kill and respawn" GUI programs (those with state set) will do this.


    Note: the commands used to be options (i.e. user modifiable but it became too complicated).
    If you change these you must update the documentation in hotkeysHelp.cpp.

*/


NV_INT32 buildCommand (QString progString, QString actionString, POINT_DATA *data, MISC *misc, NV_INT32 nearest_point,
                       QString *cmd, QStringList *args, NV_INT32 kill_switch)
{
  NV_INT32             ret;
  QString              command, cut, num;


  cut = progString;
  args->clear ();

  ret = 1;
  if (cut.contains ("[CL]")) ret = 2;
  cut.remove (QString ("[CL]"));

  *cmd = cut.section (' ', 0, 0);

  command = cut.simplified ();


  for (NV_INT32 i = 1 ; i < 999 ; i++)
    {
      cut = command.section (' ', i, i);
      if (cut.isEmpty ()) break;

      cut.replace (QString ("[MIN_Y]"), num.setNum (misc->displayed_area.min_y));
      cut.replace (QString ("[MIN_X]"), num.setNum (misc->displayed_area.min_x));
      cut.replace (QString ("[MAX_Y]"), num.setNum (misc->displayed_area.max_y));
      cut.replace (QString ("[MAX_X]"), num.setNum (misc->displayed_area.max_x));
      cut.replace (QString ("[SHARED_MEMORY_KEY]"), QString ("--shared_memory_key=%1").arg (misc->abe_share->ppid));
      cut.replace (QString ("[KILL_SWITCH]"), QString ("--kill_switch=%1").arg (kill_switch));


      //  Don't check for these if we're using a shared file.
      //  Nearest point set to negative indicates a shared file command.

      if (nearest_point >= 0)
        {
          if (cut.contains ("[INPUT_FILE]"))
            {
              cut.replace (QString ("[INPUT_FILE]"), QString (misc->file));
            }
          cut.replace (QString ("[Z_VALUE]"), num.setNum (data->z[nearest_point]));
          cut.replace (QString ("[X_VALUE]"), num.setNum (data->x[nearest_point]));
          cut.replace (QString ("[Y_VALUE]"), num.setNum (data->y[nearest_point]));
          cut.replace (QString ("[VALIDITY]"), num.setNum (data->val[nearest_point], 16));
          cut.replace (QString ("[RECORD]"), num.setNum (data->rec[nearest_point]));
          cut.replace (QString ("[SUBRECORD]"), num.setNum (data->sub[nearest_point]));
        }

      *args += cut;
    }



  //  Check for action keys and add them to the command as action arguments

  if (!actionString.isEmpty ())
    {
      for (NV_INT32 i = 0 ; i < 10 ; i++)
        {
          cut = actionString.section (',', i, i);
          if (cut.isEmpty ()) break;

          QString key;
          key.sprintf ("--actionkey%02d=", i);
          key += cut;

          *args += key;
        }
    }

  return (ret);
}
