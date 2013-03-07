
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



/*  PFM_ABE standalone toolbar.  */


#ifndef VERSION

#define     VERSION     "PFM Software - abe V1.11 - 10/20/11"

#endif

/*!< <pre>

    Version 1.00
    Jan C. Depner
    01/15/10

    First working version.


    Version 1.01
    Jan C. Depner
    01/28/10

    Swapped bathyQuery and areaCheck because it seemed to make more sense that way.


    Version 1.02
    Jan C. Depner
    01/03/11

    Make sure that unavailable programs are insensitive when you switch the orientation of the box.


    Version 1.03
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 1.10
    Jan C. Depner
    09/26/11

    Removed CUBE, CHRTR, and PFM gridding programs and added charts2LAS.  Now this only supports actual ABE programs.


    Version 1.11
    Jan C. Depner
    10/20/11

    Removed chartsLAS program and added trackLine.

*/
