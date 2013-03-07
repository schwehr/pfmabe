
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



#ifndef ANCILLARYPROG_H
#define ANCILLARYPROG_H

#include "pfmEditDef.hpp"


void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore);


class ancillaryProg:public QProcess
{
  Q_OBJECT 


public:

  ancillaryProg (QWidget *parent = 0, nvMap *ma = NULL, OPTIONS *op = NULL, MISC *mi = NULL, NV_INT32 prog = 0, NV_INT32 lock_point = 0);
  ~ancillaryProg ();


signals:

  void redrawSignal (NV_BOOL clearmap);


protected:

  OPTIONS         *options;

  MISC            *misc;

  nvMap           *map;

  NV_BOOL         command_line, command_line_up;

  QString         cmd;

  QStringList     args;

  QWidget         *pa;

  QDialog         *commandLineD;

  QTextEdit       *commandText;

  QTimer          *appTimer;

  NV_INT32        appTimerCount;


protected slots:

  void slotDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotError (QProcess::ProcessError error);
  void slotReadyReadStandardError ();
  void slotReadyReadStandardOutput ();
  void slotAppTimer ();
  void slotHelp ();
  void slotCloseCommand ();

private:
};

#endif
