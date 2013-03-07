
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



#include "pfmEdit3D.hpp"

/*!

    - buildCommand

    - Builds commands used to execute external ancillary programs.

    - Ancillary program command line substitutions:
        - [MOSAIC_FILE] = associated mosaic file name
        - [TARGET_FILE] = associated feature (target) file name
        - [PFM_FILE] = PFM list or handle file name
        - [BIN_FILE] = PFM bin directory name
        - [INDEX_FILE] = PFM index directory name
        - [INPUT_FILE] = input data file associated with the current point
        - [SHARED_MEMORY_ID] = ABE shared memory ID (some programs like chartsPic require this)
        - [LINE] = line name associated with the current point
        - [Z_VALUE] = Z value of the current point
        - [X_VALUE] = X value (usually longitude) associated with the current point
        - [Y_VALUE] = Y value (usually latitude) associated with the current point
        - [MIN_Y] = minimum Y value in the currently displayed area
        - [MIN_X] = minimum X value in the currently displayed area
        - [MAX_Y] = maximum Y value in the currently displayed area
        - [MAX_X] = maximum X value in the currently displayed area
        - [FILE_NUMBER] = PFM input file number associated with the current point
        - [LINE_NUMBER] = PFM input line number associated with the current point
        - [VALIDITY] = PFM validity word for the current point
        - [RECORD] = input file record number associated with the current point
        - [SUBRECORD] = input file subrecord (usually beam) number associated with the current point
        - [DATA_TYPE] = PFM data type of the current point

        - [CL] = run this program as a command line program (creates a dialog for output)
        - [SHARED_MEMORY_KEY] = Add the shared memory ID so the outboard program can track it
        - [KILL_SWITCH] = Add a kill switch value so that we can set the shared key to kill the outboard program.
                          This option requires [SHARED_MEMORY_KEY] but not all shared programs will have a kill switch.
                          In fact, only "kill and respawn" GUI programs (those with state set) will do this.


    - Note: the commands used to be options (i.e. user modifiable but it became too complicated).
            If you change these you must update the documentation in hotkeysHelp.cpp.

*/

NV_INT32 buildCommand (QString progString, QString actionString, MISC *misc, NV_INT32 nearest_point, QString *cmd, QStringList *args, NV_INT32 kill_switch)
{
  NV_CHAR              tmp[512];
  NV_INT16             type;
  NV_INT32             ret;
  QString              command, cut, num;


  cut = progString;
  args->clear ();

  ret = 1;
  if (cut.contains ("[CL]")) ret = 2;
  cut.remove (QString ("[CL]"));

  *cmd = cut.section (' ', 0, 0);

  command = cut.simplified ();


  //  Nearest point set to negative indicates a shared file command.

  NV_INT32 pfm = 0;
  if (nearest_point >= 0) pfm = misc->data[nearest_point].pfm;

  for (NV_INT32 i = 1 ; i < 999 ; i++)
    {
      cut = command.section (' ', i, i);
      if (cut.isEmpty ()) break;

      cut.replace (QString ("[MOSAIC_FILE]"), QString (misc->abe_share->open_args[pfm].image_path));
      cut.replace (QString ("[TARGET_FILE]"), QString (misc->abe_share->open_args[pfm].target_path));
      cut.replace (QString ("[PFM_FILE]"), QString (misc->abe_share->open_args[pfm].list_path));
      cut.replace (QString ("[BIN_FILE]"), QString (misc->abe_share->open_args[pfm].bin_path));
      cut.replace (QString ("[INDEX_FILE]"), QString (misc->abe_share->open_args[pfm].index_path));
      cut.replace (QString ("[MIN_Y]"), num.setNum (misc->displayed_area.min_y));
      cut.replace (QString ("[MIN_X]"), num.setNum (misc->displayed_area.min_x));
      cut.replace (QString ("[MAX_Y]"), num.setNum (misc->displayed_area.max_y));
      cut.replace (QString ("[MAX_X]"), num.setNum (misc->displayed_area.max_x));
      cut.replace (QString ("[SHARED_MEMORY_KEY]"), QString ("--shared_memory_key=%1").arg (misc->abe_share->ppid));
      cut.replace (QString ("[KILL_SWITCH]"), QString ("--kill_switch=%1").arg (kill_switch));


      //  Don't check for these if we're using a shared file.

      if (nearest_point >= 0)
        {
          if (cut.contains ("[INPUT_FILE]"))
            {
              read_list_file (misc->pfm_handle[misc->data[nearest_point].pfm], misc->data[nearest_point].file, tmp, &type);


              //  If the file has /PFMWDB:: as the beginning of the file then we are tying to unload from a PFM World Data Base
              //  (PFMWDB) file and we need to strip the /PFMWDB:: off of the file name and hope that it has been placed in the 
              //  current directory.

              QString str = QString (tmp).remove ("/PFMWDB::");

              cut.replace (QString ("[INPUT_FILE]"), str);
            }
          cut.replace (QString ("[LINE]"), QString (read_line_file (misc->pfm_handle[misc->data[nearest_point].pfm], misc->data[nearest_point].line)));
          cut.replace (QString ("[Z_VALUE]"), num.setNum (misc->data[nearest_point].z));
          cut.replace (QString ("[X_VALUE]"), num.setNum (misc->data[nearest_point].x));
          cut.replace (QString ("[Y_VALUE]"), num.setNum (misc->data[nearest_point].y));
          cut.replace (QString ("[FILE_NUMBER]"), num.setNum (misc->data[nearest_point].file));
          cut.replace (QString ("[LINE_NUMBER]"), num.setNum (misc->data[nearest_point].file));
          cut.replace (QString ("[VALIDITY]"), num.setNum (misc->data[nearest_point].val, 16));
          cut.replace (QString ("[RECORD]"), num.setNum (misc->data[nearest_point].rec));
          cut.replace (QString ("[SUBRECORD]"), num.setNum (misc->data[nearest_point].sub));
          if (cut.contains ("[DATA_TYPE]"))
            {
              read_list_file (misc->pfm_handle[misc->data[nearest_point].pfm], misc->data[nearest_point].file, tmp, &type);
              cut.replace (QString ("[DATA_TYPE]"), num.setNum (type));
            }
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
