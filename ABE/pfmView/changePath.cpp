
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "changePath.hpp"
#include "changePathHelp.hpp"


/*!
  This dialog lets you change the input file paths in the .ctl file.  This can be used if you move the input files
  although I usually prefer a text editor ;-)
*/

static NV_INT32 substitute_cnt;
static NV_CHAR substitute_path[10][2][1024];


static NV_BOOL sub_in ()
{
  char        varin[1024], info[1024];
  FILE        *fp;


  substitute_cnt = 0;


  //  If the startup file was found...
    
  if ((fp = find_startup (".pfm_cfg")) != NULL)
    {
      //  Read each entry.
        
      while (ngets (varin, sizeof (varin), fp) != NULL)
        {
	  if (varin[0] != '#')
	    {
	      //  Check input for matching strings and load values if found.
            
	      if (strstr (varin, "[SUBSTITUTE PATH]") != NULL && substitute_cnt < 10)
		{
		  //  Put everything to the right of the equals sign in 'info'.
            
		  get_string (varin, info);


		  /*  Throw out malformed substitute paths.  */

		  if (strchr (info, ','))
		    {
		      /*  Check for more than 1 UNIX substitute path.  */

		      if (strchr (info, ',') == strrchr (info, ','))
			{
			  strcpy (substitute_path[substitute_cnt][0], strtok (info, ","));
			  strcpy (substitute_path[substitute_cnt][1], strtok (NULL, ","));
			  substitute_path[substitute_cnt][2][0] = 0;
			}
		      else
			{
			  strcpy (substitute_path[substitute_cnt][0], strtok (info, ","));
			  strcpy (substitute_path[substitute_cnt][1], strtok (NULL, ","));
			  strcpy (substitute_path[substitute_cnt][2], strtok (NULL, ","));
			}

		      substitute_cnt++;
		    }
		}
	    }
	}

      fclose (fp);
    }

  if (substitute_cnt) return (NVTrue);

  return (NVFalse);
}


static void inv_sub (NV_CHAR *path, NV_BOOL windoze)
{
  NV_CHAR        string[1024];
  NV_BOOL        hit;


  for (NV_INT32 i = 0 ; i < substitute_cnt ; i++)
    {
      //  Set the hit flag so we can break out if we do the substitution.

      hit = NVFalse;


      if (windoze)
	{
	  //  Check for the Windows path (for example X:)

	  if (strstr (path, substitute_path[i][0]))
	    {
	      //  Always use the first UNIX path (which should be the networked path).

	      strcpy (string, substitute_path[i][1]);

	      NV_INT32 j = strlen (substitute_path[i][0]);
	      strcat (string, &path[j]);
	      strcpy (path, string);

	      for (NV_INT32 j = 0 ; j < (NV_INT32) strlen (path) ; j++)
		{
		  if (path[j] == '\\') path[j] = '/';
		}

	      break;
	    }
	}
      else
	{
	  for (NV_INT32 k = 1 ; k < 3 ; k++)
	    {
	      //  Make sure that we had two UNIX substitutes, otherwise it will substitute for a blank string
	      //  (this only works when k is 2).

	      if (substitute_path[i][k][0] && strstr (path, substitute_path[i][k]))
		{
		  strcpy (string, substitute_path[i][0]);

		  NV_INT32 j = strlen (substitute_path[i][k]);
		  strcat (string, &path[j]);
		  strcpy (path, string);

		  for (NV_INT32 j = 0 ; j < (NV_INT32) strlen (path) ; j++)
		    {
		      if (path[j] == '/') path[j] = '\\';
		    }

		  hit = NVTrue;

		  break;
		}
	    }


	  //  Break out if we did a substitution.

	  if (hit) break;
	}
    }
}



changePath::changePath (QWidget *parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;


  NV_CHAR pfm_version[512];
  FILE *hfp;
  if ((hfp = fopen (misc->abe_share->open_args[0].list_path, "r")) == NULL)
    {
      QMessageBox::warning (this, tr ("pfmView Change Input File Paths"), tr ("The file\n") + 
			    QString (misc->abe_share->open_args[0].list_path) +
			    tr ("\ncould not be opened"));
      return;
    }
  fgets (pfm_version, sizeof (pfm_version), hfp);
  fclose (hfp);


  NV_INT32 major = 0;
  sscanf (strstr (pfm_version, "library V"), "library V%d", &major);

  if (major < 5)
    {
      QMessageBox::warning (this, tr ("pfmView Change Input File Paths"), 
			    tr ("This function not available for pre-5.0 PFM structures."));
      return;
    }


  clipboard = QApplication::clipboard ();
  clipboard->setText ("");

  dir = ".";


  setWindowTitle (tr ("pfmView Change Input File Paths"));


  setSizeGripEnabled (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  count = get_next_list_file_number (misc->pfm_handle[0]);

  fileList = new QTextEdit (this);
  fileList->setLineWrapMode (QTextEdit::FixedColumnWidth);
  fileList->setLineWrapColumnOrWidth (512);
  fileList->setWhatsThis (fileListText);
  fileList->setReadOnly (TRUE);


  //  Read the .ctl file without applying the pfm_substitute function in the PFM library.  This would 
  //  normally automatically switch the names to match your system type but we want to work with the
  //  names as they exist in the file (without substituting) so we don't use read_list_file.

  QString name = QString (misc->abe_share->open_args[0].list_path) + ".data/" + 
    QString (pfm_basename (misc->abe_share->open_args[0].list_path)) + ".ctl";

  NV_CHAR cname[512];
  strcpy (cname, name.toAscii ());
  FILE *ifp;

  if ((ifp = fopen (cname, "r")) == NULL)
    {
      QMessageBox::warning (this, tr ("pfmView Change Input File Paths"), tr ("Unable to open the file\n") + name);
      return;
    }


  NV_CHAR data[1024];


  //  Skip the first five lines of the control file so we can get to the input files.

  for (NV_INT32 i = 0 ; i < 5 ; i++) fgets (data, sizeof (data), ifp);
  

  NV_CHAR first_char = '/';
  QStringList tmpList;

  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      ngets (data, sizeof (data), ifp);


      //  Trying to figure out what the normal first character is just in case we get a "deleted" file.

      if (data[11] != '*') first_char = data[11];

      tmpList += QString (&data[11]);
    }


  fclose (ifp);


  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      QString tmp = tmpList.at (i);


      //  Replace the * in "deleted" file names (hopefully with the correct character).

      if (tmp.startsWith ('*')) tmp.replace (0, 1, first_char);

      fileNameList += tmp;
      fileList->append (tmp);
    }


  vbox->addWidget (fileList);


  QGroupBox *cbox = new QGroupBox (tr ("Change path"), this);
  QHBoxLayout *cboxLayout = new QHBoxLayout;
  cbox->setLayout (cboxLayout);

  QLabel *fLabel = new QLabel (tr ("From :"), this);
  cboxLayout->addWidget (fLabel);
  fromString = new QLineEdit (this);
  fromString->setWhatsThis (fromText);
  cboxLayout->addWidget (fromString);

  QLabel *tLabel = new QLabel (tr ("To :"), this);
  cboxLayout->addWidget (tLabel);
  toString = new QLineEdit (this);
  toString->setWhatsThis (toText);
  cboxLayout->addWidget (toString);

  QPushButton *dirBrowse = new QPushButton (tr ("Browse"), this);
  dirBrowse->setToolTip (tr ("Select the directory that contains the moved input files"));
  dirBrowse->setWhatsThis (dirBrowseText);
  connect (dirBrowse, SIGNAL (clicked ()), this, SLOT (slotDirBrowseClicked ()));
  cboxLayout->addWidget (dirBrowse);


  QPushButton *applyButton = new QPushButton (tr ("Apply"), this);
  applyButton->setToolTip (tr ("Apply path change"));
  applyButton->setWhatsThis (applyIFPText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApply ()));
  cboxLayout->addWidget (applyButton);


  QPushButton *invertButton = new QPushButton (tr ("Invert"), this);
  invertButton->setToolTip (tr ("Switch between Windows and UNIX file names"));
  invertButton->setWhatsThis (invertIFPText);
  connect (invertButton, SIGNAL (clicked ()), this, SLOT (slotInvert ()));
  cboxLayout->addWidget (invertButton);

  if (!sub_in ()) invertButton->setEnabled (FALSE);


  vbox->addWidget (cbox);


  QHBoxLayout *actions = new QHBoxLayout (0);
  actions->setMargin (5);
  actions->setSpacing (5);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *acceptButton = new QPushButton (tr ("Accept"), this);
  acceptButton->setToolTip (tr ("Accept all changes made to paths"));
  acceptButton->setWhatsThis (acceptIFPText);
  connect (acceptButton, SIGNAL (clicked ()), this, SLOT (slotAccept ()));
  actions->addWidget (acceptButton);

  QPushButton *closeButton = new QPushButton (tr ("Discard"), this);
  closeButton->setToolTip (tr ("Discard all changes made to paths"));
  closeButton->setWhatsThis (closeIFPText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  //  Move the dialog to the center of the parent if possible.

  QRect tmp = parent->frameGeometry ();
  NV_INT32 window_x = tmp.x ();
  NV_INT32 window_y = tmp.y ();


  tmp = parent->geometry ();
  NV_INT32 width = tmp.width ();
  NV_INT32 height = tmp.height ();


  QSize dlg = sizeHint ();
  NV_INT32 dlg_width = dlg.width ();

  move (window_x + width / 2 - dlg_width / 2, window_y + height / 2);


  show ();


  //  We don't want the cursorPositionChanged signal happening until we actually click in the fileList.

  connect (fileList, SIGNAL (cursorPositionChanged ()), this, SLOT (slotCursorPositionChanged ()));
}



changePath::~changePath ()
{
}



void
changePath::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
changePath::slotCursorPositionChanged ()
{
  toString->setText ("");


  //  Copy the selection to the clipboard.

  fileList->copy ();


  //  Set the From : field with the selection from the clipboard.

  fromString->setText (clipboard->text ());


  toString->setFocus ();
}



void 
changePath::slotDirBrowseClicked ()
{
  QString title = tr ("pfmView Browse directories");
  QFileDialog *fd = new QFileDialog (this, title);
  fd->setViewMode (QFileDialog::List);


  fd->setDirectory (dir);


  QStringList filters;
  filters << tr ("All files (*)");

  fd->setFilters (filters);

  fd->setFileMode (QFileDialog::Directory);


  if (fd->exec () == QDialog::Accepted) 
    {
      //  Save the directory that we were in when we selected a directory.

      dir = fd->directory ().absolutePath ();


      toString->setText (fd->directory ().absolutePath ());
    }
}



void
changePath::slotApply ()
{
  if (!fromString->text ().isEmpty ())
    {
      fileNameList.replaceInStrings (fromString->text (), toString->text (), Qt::CaseSensitive);

      fileList->clear ();

      for (NV_INT32 i = 0 ; i < count ; i++)
	{
	  fileList->append (fileNameList.at (i));
	}
    }
}



void
changePath::slotInvert ()
{
  NV_BOOL windoze = NVFalse;

  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      if (fileNameList.at (i).contains ("\\") || fileNameList.at (i).contains (":")) windoze = NVTrue;
    }


  QStringList tmpList;
  NV_CHAR name[512];
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      strcpy (name, fileNameList.at (i).toAscii ());

      inv_sub (name, windoze);

      tmpList.append (QString (name));
    }


  fileList->clear ();
  fileNameList.clear ();

  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      fileNameList.append (tmpList.at (i));
      fileList->append (fileNameList.at (i));
    }
}



void
changePath::slotAccept ()
{
  NV_BOOL inverted = NVFalse;
  NV_BOOL windoze;

#ifdef NVLinux

  windoze = NVTrue;
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      if (fileNameList.at (i).contains ("\\") || fileNameList.at (i).contains (":"))
	{
	  inverted = NVTrue;
	  break;
	}
    }

#else

  windoze = NVFalse;
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      if (fileNameList.at (i).contains ("/"))
	{
	  inverted = NVTrue;
	  break;
	}
    }

#endif


  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      QString check;

      if (inverted)
	{
	  NV_CHAR name[512];
	  strcpy (name, fileNameList.at (i).toAscii ());

	  inv_sub (name, windoze);

	  check = QString (name);
	}
      else
	{
	  check = fileNameList.at (i);
	}

      if (!QFile (check).exists ())
	{
          NV_INT32 ret = QMessageBox::Yes;

	  ret = QMessageBox::warning (this, tr ("pfmView Change Input File Paths"), tr ("The file\n") + check +
                                      tr ("\ndoes not exist.\n") + 
                                      tr ("Do you want to make the changes anyway?"),
                                      QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton);

          if (ret == QMessageBox::No)
            {
              return;
            }
          else
            {
              break;
            }
	}
    }


  //  Apply the changes to the PFM .ctl file.

  QString name = QString (misc->abe_share->open_args[0].list_path) + ".data/" + QString (pfm_basename (misc->abe_share->open_args[0].list_path)) + ".ctl";
  QString tmp_name = name + ".tmp";

  NV_CHAR cname[512], tmp_cname[512];
  strcpy (cname, name.toAscii ());
  strcpy (tmp_cname, tmp_name.toAscii ());
  FILE *ifp, *ofp;

  if ((ifp = fopen (cname, "r")) == NULL)
    {
      QMessageBox::warning (this, tr ("pfmView Change Input File Paths"), tr ("Unable to open the file\n") + name);
      return;
    }


  if ((ofp = fopen (tmp_cname, "w")) == NULL)
    {
      QMessageBox::warning (this, tr ("pfmView Change Input File Paths"), tr ("Unable to open the file\n") + tmp_name);
      return;
    }


  NV_CHAR data[1024];


  //  Copy the first five lines of the control file so we can get to the input files.

  for (NV_INT32 i = 0 ; i < 5 ; i++)
    {
      fgets (data, sizeof (data), ifp);
      fprintf (ofp, "%s", data);
    }
  
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      fgets (data, sizeof (data), ifp);
      data[11] = 0;

      strcat (data, fileNameList.at (i).toAscii ());
      strcat (data, "\n");
      fprintf (ofp, "%s", data);
    }


  QString bak = name;
  bak.replace (".ctl", ".bak");
  NV_CHAR bname[512];
  strcpy (bname, bak.toAscii ());
  remove (bname);

  fclose (ifp);
  fclose (ofp);

  rename (cname, bname);
  rename (tmp_cname, cname);


  emit dataChangedSignal ();


  close ();
}



void
changePath::slotClose ()
{
  close ();
}
