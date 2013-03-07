
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



#include "pfmView.hpp"


//!  This function checks the feature file to see if it is an old NAVO xml file.  If so it converts it to BFD and then opens it.

NV_INT32 bfd_check_file (MISC *misc, NV_CHAR *path, BFDATA_HEADER *header, NV_INT32 mode)
{
  NV_INT32 handle;

  NV_INT32 miw2bfd (NV_CHAR *path, BFDATA_HEADER *header, MISC *misc);


  //  If the file was a NAVO standard target (XML) file.

  if (!strcmp (&path[strlen (path) - 4], ".xml"))
    {
      QMessageBox::information (misc->map_widget, pfmView::tr ("pfmView"), 
                                pfmView::tr ("PFM_ABE no longer supports XML target files.\n\n") + QString (path) +
                                pfmView::tr ("\n\nwill be converted to Binary Feature Data (BFD) format."));


      qApp->setOverrideCursor (Qt::WaitCursor);
      qApp->processEvents ();


      handle = miw2bfd (path, header, misc);


      //  We successfully converted the XML file to BFD.

      if (handle >= 0)
        {
          strcpy (misc->abe_share->open_args[0].target_path, path);

          update_target_file (misc->pfm_handle[0], misc->abe_share->open_args[0].list_path, misc->abe_share->open_args[0].target_path);
        }
      else
        {
          misc->statusProg->reset ();
          misc->statusProg->setTextVisible (FALSE);
          qApp->processEvents();
        }

      qApp->restoreOverrideCursor ();
    }
  else
    {
      handle = binaryFeatureData_open_file (path, header, mode);
    }

  return (handle);
}
