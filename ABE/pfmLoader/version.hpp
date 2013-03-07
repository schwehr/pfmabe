
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

#define     VERSION     "PFM Software - pfmLoader V1.35 - 10/31/11"

#endif

/*!< <pre>

    Version 1.00
    Jan C. Depner
    09/21/10

    First version.


    Version 1.01
    Jan C. Depner
    10/20/10

    Time as a depth attribute is now set across all input files that have time.


    Version 1.02
    Jan C. Depner
    12/13/10

    Fixed bug when setting HOF standard user flags.  I forgot that abdc of -70 is land just as abdc of 70 is.


    Version 1.03
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 1.10
    Jan C. Depner
    01/17/11

    Added support for CHRTR2 data as we phase out the old CHRTR format.  Still using PFM_CHRTR_DATA flag.  Check the 
    file name on unload to determine the actual type (for now).


    Version 1.20
    Jan C. Depner
    02/08/11

    Added support for AHAB Hawkeye attributes.


    Version 1.21
    Jan C. Depner
    02/16/11

    Now converts path separators to native separators (\ on Windows) before writing the file name to the list file.


    Version 1.22
    Jan C. Depner
    02/23/11

    Shortened HAWKEYE attribute names because they were blowing out the PFM attribute name fields.


    Version 1.23
    Jan C. Depner
    02/25/11

    Now loads both PFM_HAWKEYE_HYDRO_DATA and PFM_HAWKEYE_TOPO_DATA.


    Version 1.24
    Jan C. Depner
    04/25/11

    Removed the "shoreline depth swapped" HOF lidar user flag and combined it with the "shallow water processed" HOF
    lidar user flag.  Replaced the "shoreline depth swapped" user flag with an "APD" user flag.  Since "shoreline depth swapped"
    (the old shallow water processing method) is never used with "shallow water processed" data processing this should
    cause no problems.


    Version 1.25
    Jan C. Depner
    06/16/11

    Removed HMPS_SPARE_1 flag check since we don't do swath filtering anymore and it's been removed from hmpsflag.h.


    Version 1.30
    Jan C. Depner
    07/21/11

    Added support for DRAFT CZMIL data.


    Version 1.31
    Jan C. Depner
    08/15/11

    Added in all of the currently available CZMIL attributes.  Many more to come.


    Version 1.32
    Jan C. Depner
    09/21/11

    Added ability to read an "update" parameter file (.upr) from pfmLoad.
    Replaced bin_inside calls with bin_inside_ptr calls.
    Added U.S. Government diclaimer comments to all files.


    Version 1.33
    Jan C. Depner
    10/06/11

    Fixed bug in TOF loader.  I was under the impression that if the last return was bad (-998.0)
    then the first return must be bad as well.  This is not the case.


    Version 1.34
    Jan C. Depner
    10/21/11

    Added a bunch of GSF attributes from the gsfSwathBathyPing record arrays.


    Version 1.35
    Jan C. Depner
    10/31/11

    Now sets max cache memory size (from parameter file) for PFM library.  Happy Halloween!

</pre>*/
