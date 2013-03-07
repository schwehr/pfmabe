
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



#ifndef CHRTR2_VERSION

#define     CHRTR2_VERSION "PFM Software - CHRTR2 library V2.00 - 03/31/11"

#endif

/*!< <pre>

    Version 1.00
    04/12/10
    Jan C. Depner

    First version.


    Version 1.01
    04/29/10
    Jan C. Depner

    Microsoft, in their infinite wisdom, doesn't support %hhd in scanf statements (like the rest of
    the world does) so I had to use %hd, slap it into a short int and then copy the value to an
    unsigned char.  What's really weird is that %hhd works fine for printf.  DOH!


    Version 1.02
    Jan C. Depner
    04/30/10

    Now use "const NV_CHAR *" arguments instead of "NV_CHAR *" arguments where applicable (gcc 4.4 doesn't like 
    you calling functions with constant strings [i.e. "fred"] if the argument in the function isn't declared const).


    Version 1.03
    Jan C. Depner
    08/17/10

    Added total uncertainty and number of points to chrtr2_record.


    Version 1.04
    Jan C. Depner
    10/26/10

    Fix screwup when reading version string.


    Version 1.05
    Jan C. Depner
    01/24/11

    Changed CHRTR2_USER_05 to CHRTR2_LAND_MASK.  That's all we ever used it for anyway.  It makes more sense
    as a fixed field since we use it quite a bit.


    Version 2.00
    Jan C. Depner
    03/31/11

    Added ellipsoid_separation and z00_value optional fields to the record.  This is a major format change.

</pre>*/
