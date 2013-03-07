
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



#ifndef PFMLOAD_H
#define PFMLOAD_H

#include "pfmLoadDef.hpp"
#include "startPage.hpp"
#include "pfmPage.hpp"
#include "globalPage.hpp"
#include "inputPage.hpp"
#include "runPage.hpp"
#include "pfmFilter.hpp"
#include "version.hpp"


class pfmLoad : public QWizard
{
  Q_OBJECT


public:

  pfmLoad (QWidget *parent = 0, QString parm_file = "");
  ~pfmLoad ();


signals:


protected:

  void initializePage (int id);
  void cleanupPage (int id);


  NV_INT32         pfm_file_count, input_file_count, window_x, window_y, window_width, window_height;

  NV_BOOL          cube_available, upr_file;

  FILE             *errfp;

  NV_CHAR          error_file[512], cube_name[50];

  PFM_DEFINITION   pfm_def[MAX_LOAD_FILES], ref_def;

  PFM_GLOBAL       pfm_global;

  GLOBAL_DIALOG    global_dialog;

  globalPage       *globalPg;

  FILE_DEFINITION  *input_file_def;

  FLAGS            flags;

  RUN_PROGRESS     progress;

  NV_CHAR          fname[512];

  QTextEdit        *inputFiles;

  QStringList      input_files;

  QString          inputFilter, parameter_file;

  QProcess         *loadProc, *cubeProc;

  QListWidget      *checkList;


  void writeParameters (FILE *fp);
  void envin ();
  void envout ();
  void error_and_summary ();


protected slots:

  void slotHelpClicked ();
  void slotCustomButtonClicked (int id);
  void slotParameterFilterSelected (const QString &filter);

  void slotCubeReadyReadStandardError ();
  void slotCubeReadyReadStandardOutput ();
  void slotCubeError (QProcess::ProcessError error);
  void slotCubeDone (int exitCode, QProcess::ExitStatus exitStatus);


};

#endif
