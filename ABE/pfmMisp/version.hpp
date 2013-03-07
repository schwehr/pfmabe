
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




#ifndef VERSION

#define     VERSION     "PFM Software - pfmMisp V4.07 - 11/18/11"

#endif

/*! <pre>


    Version 1.0
    Jan C. Depner
    09/18/02


    Version 1.1
    Jan C. Depner
    11/13/02

    Added option to use min, max, or average to compute surface.


    Version 1.11
    Jan C. Depner
    12/13/02

    Put a min/max limit on interpolated data.


    Version 1.12
    Jan C. Depner
    02/04/03

    Hardly worth a version change.  Added a final print line.


    Version 1.2
    Jan C. Depner
    07/10/03

    Added the ability to nibble away interpolated data in cells that are not
    within a specified distance of cells containing valid data.  Also, changed
    the options to match pfm2geotiff.  This is a lot more flexible for the
    user.


    Version 1.3
    Jan C. Depner
    07/18/03

    Went to 32 bits for validity fields.


    Version 1.4
    Jan C. Depner
    10/21/03

    Incorrectly computing bin width - bugfix.


    Version 1.41
    Jan C. Depner
    12/16/04

    Changed Usage message for PFM 4.5 directory input.


    Version 1.42
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 1.43
    Jan C. Depner
    10/26/05

    Changed usage for PFM 4.6 handle file use.


    Version 2.0
    Jan C. Depner
    04/21/06

    Now grids by units of bin size.  Assumes approximately square bins.  Added weight as command line option.


    Version 2.1
    Jan C. Depner
    09/12/06

    Added land masking option.


    Version 2.11
    Jan C. Depner
    10/13/06

    Minor adjustment to mbr computation.


    Version 2.2
    Jan C. Depner
    10/16/06

    Replaced old 1 minute landmask with new SRTM 3 second landmask.


    Version 2.3
    Jan C. Depner
    02/16/07

    Allocating one more column for misp_rtrv.


    Version 3.0
    Jan C. Depner
    07/30/07

    Ported to a Qt 4 Wizard.


    Version 3.01
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 3.02
    Jan C. Depner
    12/22/07

    Handle command line file name.


    Version 3.03
    Jan C. Depner
    04/03/08

    Modified the help info for the nibbler.


    Version 3.04
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 3.05
    Jan C. Depner
    07/10/08

    Limit the output to the polygon defined in the PFM header (not the MBR).


    Version 3.06
    Jan C. Depner
    08/15/08

    We were dropping the last bin due to wrong constraints for the 3.05 version change.


    Version 3.07
    Stacy Johnson, Jan C. Depner
    01/28/09

    Fix checkpoint problem when opening PFM (on 64 bit).  Also, a stab at fixing the MISP out of
    PFM bounds problem.


    Version 3.07
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 4.00
    Jan C. Depner
    06/11/09

    Added GMT Surface generation option.


    Version 4.01
    Jan C. Depner
    01/08/10

    Stopped misp from gridding over land masked points.


    Version 4.02
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 4.03
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


    Version 4.04
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.


    Version 4.05
    Jan C. Depner
    09/21/11

    Replaced bin_inside calls with bin_inside_ptr calls.


    Version 4.06
    Jan C. Depner
    11/02/11

    Changed the nibble option so that -1 means we don't want to nibble and 0 means we don't
    want to put interpolated values into empty bins at all.


    Version 4.07
    Jan C. Depner
    11/18/11

    Removed the GMT options since they were not being used (by anyone).

</pre>*/
