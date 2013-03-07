
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

#define     VERSION     "PFM Software - charts2LAS V1.04 - 10/21/11"

#endif


/*! <pre>

    Version 1.00
    Jan C. Depner
    07/18/11

    First version.


    Version 1.01
    Jan C. Depner
    07/27/11

    Added option to output invalid data as well as valid data.  Also, added _h or _t to the output file name to 
    differentiate between hydro and topo data.


    Version 1.02
    Jan C. Depner
    08/29/11

    Added land, water, invalid classifications for HOF data.


    Version 1.03
    Jan C. Depner
    10/06/11

    Added land, water, invalid classifications for HOF data.


    Version 1.04
    Jan C. Depner
    10/06/11

    To match a change in the TOF PFM loader we now check input files start times against 10/07/2011.
    Files prior to that date will ignore the first return if the last return was -998.0.


    Version 1.05
    Jan C. Depner
    10/21/11

    Changed ACE classification values to 21, 27, and 29 from 101, 107, and 109.

</pre>*/
