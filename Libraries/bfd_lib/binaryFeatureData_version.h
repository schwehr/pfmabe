
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



#ifndef BFDATA_VERSION

#define     BFDATA_VERSION "PFM Software - Binary Feature Data library V2.03 - 10/26/10"

#endif

/*!< <pre>

    Version 1.00
    03/27/09
    Jan C. Depner

    First version.


    Version 1.01
    04/24/09
    Jan C. Depner

    Added event_tv_sec and event_tv_nsec to SHORT_FEATURE structure.


    Version 1.02
    06/24/09
    Jan C. Depner

    Changed function, .h, and library names to avoid collision with GNU GNU Binary File Descriptor library.


    Version 1.03
    Jan C. Depner
    07/29/09

    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 2.00
    Jan C. Depner
    09/21/09

    Fixed a complete and utter screwup on my part.  Prior to 2.00, files created on 64 bit and 32 bit systems were not compatible with
    each other.  DOH!!!!  What the hell was I smoking!?!


    Version 2.01
    Jan C. Depner
    12/28/09

    Wasn't placing the path into the error message in binaryFeatureData_create_file.


    Version 2.02
    Jan C. Depner
    04/30/10

    Now use "const NV_CHAR *" arguments instead of "NV_CHAR *" arguments where applicable (gcc 4.4 doesn't like 
    you calling functions with constant strings [i.e. "fred"] if the argument in the function isn't declared const).


    Version 2.03
    Jan C. Depner
    10/26/10

    Fix screwup when reading version string.

</pre>*/
