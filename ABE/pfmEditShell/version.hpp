
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

#define     VERSION     "PFM Software - pfmEditShell V1.05 - 09/27/11"

#endif

/*!< <pre>

    Version 1.00
    Jan C. Depner
    12/15/10

    First version.


    Version 1.01
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 1.02
    Jan C. Depner
    04/07/11

    The min and max depth HSV values, colors, and locks are now handled in ABE shared memory and are no longer passed as
    arguments to the editors.


    Version 1.03
    Jan C. Depner
    05/06/11

    Fixed problem with getopt that only happens on Windows.


    Version 1.04
    Jan C. Depner
    09/12/11

    Doesn't automatically kill child processes now.


    Version 1.05
    Jan C. Depner
    09/27/11

    Process ID is now supplied on the command line so that we can keep child of child processes
    alive over multiple instances of start and stop.  Also, setting the process ID to its 
    negative value will kill all children.

</pre>*/
