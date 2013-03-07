
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



#include "registerABE.hpp"


/*  registerABE class.  */

/***************************************************************************/
/*!

   - Module :        registerABE

   - Programmer :    Jan C. Depner

   - Date :          04/09/08

   - Purpose :       We want to be able to register ABE (or CNILE) applications
                     to groups and keep track of their shared memory IDs so that
                     we can start applications (other than by shelling)
                     and still share some positional information.  Each process
                     will have a group number and a member ID.  The member ID
                     will always be the calling application's PID.  If you look
                     at ABE_register.hpp you'll see how the memory block is
                     structured.  The main things that get shared between programs
                     are the active_window_id (set to your PID in mouseMove function)
                     and the cursor_position.  We can expand the ABE_REGISTER_DATA
                     block whenever we need to as long as we coordinate
                     between applications.  Note that pfmView and other mapping
                     GUIs may have many of their own children (like pfmEdit is
                     the child of pfmView).  We use the ABE_SHARE block in the
                     children and parents to share information.  We don't register
                     the children.  The parent process ID will be passed to the
                     children by the parent (usually on the command line).  The 
                     parent will have to coordinate cursor movement and other 
                     things between its children and other applications in the
                     group.

   - Arguments :
                     - parent             =  QWidget parent
                     - name               =  name of the calling master application
                     - key                =  key value (ALWAYS the caller's PID)
                     - file               =  associated file name (may be set to NULL)
                     - abeRegister        =  pointer to QSharedMemory pointer
                     - abe_register       =  pointer to ABE_REGISTER structure pointer

   - Caveats :
                     - The shared memory ID of the register shared memory is ALWAYS
                       the user's UID.
                     - This function will always emit keySetSignal.  If the value passed
                       is not -1 then that is the group number.

\***************************************************************************/

registerABE::registerABE (QWidget *parent, const NV_CHAR *name, NV_INT32 key, const NV_CHAR *file, QSharedMemory **abeRegister,
			  ABE_REGISTER **abe_register):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  member_id = key;
  strcpy (member_name, name);
  if (file == NULL)
    {
      strcpy (member_file, "N/A");
    }
  else
    {
      strcpy (member_file, file);
    }
  selected = 0;


  setModal (TRUE);


  //  Grab or create the memory block.

  QString skey;
#ifdef NVWIN3X
  skey.sprintf ("%d", windows_getuid ());
#else
  skey.sprintf ("%d", getuid ());
#endif

  *abeRegister = new QSharedMemory (skey);

  NV_BOOL attached = NVFalse;

  if (!(*abeRegister)->create (sizeof (ABE_REGISTER), QSharedMemory::ReadWrite))
    {
      (*abeRegister)->attach (QSharedMemory::ReadWrite);
      attached = NVTrue;
    }
  abeRegister_ptr = *abeRegister;


  //  Lock this while we're working with it.

  abeRegister_ptr->lock ();


  *abe_register = (ABE_REGISTER *) (*abeRegister)->data ();


  //  It's easier to deal with a pointer than (*abe_register)->

  abe_register_ptr = *abe_register;


  //  If we created the ABE_REGISTER memory block we need to clear it.

  if (!attached)
    {
      //  Clear the block.

      for (NV_INT32 i = 0 ; i < MAX_ABE_GROUPS ; i++)
	{
	  abe_register_ptr->group[i] = NVFalse;
	  abe_register_ptr->member_count[i] = 0;
	  memset (&abe_register_ptr->data[i], 0, sizeof (ABE_REGISTER_DATA));


	  //  Clear the members.

	  for (NV_INT32 j = 0 ; j < MAX_ABE_MEMBERS ; j++)
	    {
	      abe_register_ptr->member[i][j].key = 0;
	      strcpy (abe_register_ptr->member[i][j].name, "N/A");
	      strcpy (abe_register_ptr->member[i][j].file, "N/A");
	    }
	}


      //  Unlock when we're done.

      abeRegister_ptr->unlock ();
    }


  setWindowTitle (tr ("registerABE process table"));


  setSizeGripEnabled (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


#ifdef NVLinux

  //  On Linux we need to make sure that all processes listed in the ABE_REGISTER are actually
  //  still running.  See the check_dead_processes function for an explanation and caveats.

  check_dead_processes ();

#endif


  //  Search for available groups.

  NV_INT32 count = 0;
  NV_INT32 max_members = 0;

  for (NV_INT32 i = 0 ; i < MAX_ABE_GROUPS ; i++)
    {
      if (abe_register_ptr->group[i])
	{
	  available_group[count] = i;
	  max_members = qMax (max_members, abe_register_ptr->member_count[i]);
	  count++;
	}
    }


  processTable = new QTableWidget (count, 2 + max_members * 2, this);
  processTable->setToolTip (tr ("Connect to another group's shared memory, create a new group, or <b>Cancel</b>"));
  processTable->setWhatsThis (tr ("Use this table to select a group of ABE applications with which "
				  "to share information.  If there are no available groups or you wish to create "
				  "a new group you may use the <b>New group</b> button to do so.  If you don't "
				  "want to connect to an existing group or create a new group, use the "
				  "<b>Cancel</b> button."));
  processTable->setAlternatingRowColors (TRUE);
  QTableWidgetItem *aItem = new QTableWidgetItem (tr ("Connect"));
  processTable->setHorizontalHeaderItem (0, aItem);
  aItem->setTextAlignment (Qt::AlignHCenter);
  QTableWidgetItem *pItem = new QTableWidgetItem (tr ("Group #"));
  processTable->setHorizontalHeaderItem (1, pItem);
  pItem->setTextAlignment (Qt::AlignHCenter);


  QTableWidgetItem *hItem[max_members * 2];
  for (NV_INT32 i = 0 ; i < max_members * 2 ; i += 2)
    {
      QString tmp = QString (tr ("Application %1")).arg (i / 2);
      hItem[i] = new QTableWidgetItem (tmp);
      processTable->setHorizontalHeaderItem (i + 2, hItem[i]);
      hItem[i]->setTextAlignment (Qt::AlignHCenter);

      hItem[i + 1] = new QTableWidgetItem (tr ("File"));
      processTable->setHorizontalHeaderItem (i + 3, hItem[i + 1]);
      hItem[i + 1]->setTextAlignment (Qt::AlignHCenter);
    }


  QCheckBox *join_group[count];


  processGrp = new QButtonGroup (this);
  processGrp->setExclusive (TRUE);


  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      join_group[i] = new QCheckBox (this);
      processTable->setCellWidget (i, 0, join_group[i]);
      processGrp->addButton (join_group[i], i);
      join_group[i]->setCheckState (Qt::Unchecked);


      QString tmp;

      tmp.sprintf ("%02d", available_group[i]);
      QTableWidgetItem *pI = new QTableWidgetItem (tmp);
      processTable->setItem (i, 1, pI);


      NV_INT32 memcount = 0;
      for (NV_INT32 j = 0 ; j < MAX_ABE_MEMBERS ; j++)
	{
	  if (abe_register_ptr->member[available_group[i]][j].key)
	    {
	      QTableWidgetItem *nI = new QTableWidgetItem (QString (abe_register_ptr->member[available_group[i]][j].name));
	      processTable->setItem (i, memcount + 2, nI);

	      QTableWidgetItem *fI = new QTableWidgetItem (QString (abe_register_ptr->member[available_group[i]][j].file));
	      processTable->setItem (i, memcount + 3, fI);

	      memcount += 2;
	    }
	}
      processTable->resizeColumnsToContents ();
      processTable->resizeRowsToContents ();
    }

  vbox->addWidget (processTable, 0, 0);


  //  Connect after populating table so the slot doesn't get called.

  connect (processGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotValueChanged (int)));


  resize (800, 600);


  processTable->show ();


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *newButton = new QPushButton (tr ("New group"), this);
  newButton->setToolTip (tr ("Create a new group and close dialog"));
  connect (newButton, SIGNAL (clicked ()), this, SLOT (slotNew ()));
  actions->addWidget (newButton);

  QPushButton *applyButton = new QPushButton (tr ("Apply"), this);
  applyButton->setToolTip (tr ("Connect to selected process and close dialog"));
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApply ()));
  actions->addWidget (applyButton);

  QPushButton *closeButton = new QPushButton (tr ("Cancel"), this);
  closeButton->setToolTip (tr ("Discard selection and close dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);

  show ();
}



registerABE::~registerABE ()
{
}



void
registerABE::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



/*!

    On Linux we need to make sure that all processes listed in the ABE_REGISTER are actually
    still running.  The reason is that, on Linux, shared memory will not be released when the
    last process that was connected to it dies.  The situation we are trying to avoid will be
    caused by a process dying and leaving it's process ID in the master list or a member list.
    On Windows, when the last process connected to a shared memory block dies, the shared memory is
    released.  As you can see, there are advantages and disadvantages to both methods of dealing
    with this.  I don't know of a simple way to do this so I'm just going to check on a directory
    in the proc directory with the same name as the PID.

    <pre>
    WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 

                                   THIS ONLY WORKS ON LINUX!

    WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
    </pre>

*/

void 
registerABE::check_dead_processes ()
{
  FILE *fp;
  NV_CHAR check[512];

  for (NV_INT32 i = 0 ; i < MAX_ABE_GROUPS ; i++)
    {
      if (abe_register_ptr->group[i])
	{
	  for (NV_INT32 j = 0 ; j < MAX_ABE_MEMBERS ; j++)
	    {
	      if (abe_register_ptr->member[i][j].key)
		{
		  sprintf (check, "/proc/%d", abe_register_ptr->member[i][j].key);

		  if ((fp = fopen (check, "r")) == NULL)
		    {
		      abe_register_ptr->member[i][j].key = 0;
		      strcpy (abe_register_ptr->member[i][j].name, "N/A");
		      strcpy (abe_register_ptr->member[i][j].file, "N/A");
		      abe_register_ptr->member_count[i]--;
		    }
		  else
		    {
		      fclose (fp);
		    }
		}
	    }
	}
    }
}



void
registerABE::slotValueChanged (int row)
{
  selected = available_group[row];
}



void
registerABE::slotApply ()
{
  //  Set the member information since we selected a group.

  for (NV_INT32 j = 0 ; j < MAX_ABE_MEMBERS ; j++)
    {
      if (!abe_register_ptr->member[selected][j].key)
	{
	  abe_register_ptr->member[selected][j].key = member_id;
	  strcpy (abe_register_ptr->member[selected][j].name, member_name);
	  strcpy (abe_register_ptr->member[selected][j].file, member_file);
	  abe_register_ptr->member_count[selected]++;
	  break;
	}
    }


  //  Unlock when we're done.

  abeRegister_ptr->unlock ();


  //  Emit the key set signal.

  emit keySetSignal (selected);


  close ();
}



void
registerABE::slotNew ()
{
  //  Find an available group to populate.

  for (NV_INT32 i = 0 ; i < MAX_ABE_GROUPS ; i++)
    {
      if (!abe_register_ptr->group[i])
	{
	  abe_register_ptr->group[i] = NVTrue;
	  abe_register_ptr->member[i][0].key = member_id;
	  strcpy (abe_register_ptr->member[i][0].name, member_name);
	  strcpy (abe_register_ptr->member[i][0].file, member_file);
	  abe_register_ptr->member_count[i]++;
	  selected = i;
	  abe_register_ptr->command[i].id = 0;
	  abe_register_ptr->command[i].number = 0;
	  break;
	}
    }


  //  Unlock when we're done.

  abeRegister_ptr->unlock ();


  //  Emit the key set signal.

  emit keySetSignal (selected);


  close ();
}



void
registerABE::slotClose ()
{
  //  Unlock when we're done.

  abeRegister_ptr->unlock ();


  //  Emit the key set signal.

  emit keySetSignal (-1);


  close ();
}
