
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

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef LLZ_VERSION

#define     LLZ_VERSION "PFM Software - llz library V2.08 - 08/31/10"

#endif

/*< <pre>

    Version 1.0
    08/31/06
    Jan C. Depner

    Improved the error string code so that it puts out more descriptive
    text.


    Version 2.0
    07/25/07
    Jan C. Depner

    Added TIME_FLAG and ability to optionally store time (POSIX) in the record.  Also, made stat 32 bits.


    Version 2.01
    09/26/07
    Jan C. Depner

    Fixed backward compatibility problem between 1.0 and 2.0


    Version 2.02
    10/12/07
    Jan C. Depner

    Fixed swap short bug for stat.


    Version 2.03
    04/07/08
    Jan C. Depner

    Replaced single .h files from utility library with include of nvutility.h


    Version 2.04
    05/09/08
    Jan C. Depner

    Fixed bug with time_flag and depth_units not being placed in the header correctly.  Also, had to open for write with
    "wb" instead of "w" because Windoze is too stupid to figure it out.


    Version 2.05
    06/10/08
    Jan C. Depner

    Added ftell_llz function.


    Version 2.06
    04/09/09
    Jan C. Depner

    Handle a non-llz binary file mistakenly being opened as LLZ.


    Version 2.07
    Jan C. Depner
    04/30/10

    Now use "const NV_CHAR *" arguments instead of "NV_CHAR *" arguments where applicable (gcc 4.4 doesn't like 
    you calling functions with constant strings [i.e. "fred"] if the argument in the function isn't declared const).


    Version 2.08
    Jan C. Depner
    08/31/10

    Added _LARGEFILE64_SOURCE support.

</pre>*/
