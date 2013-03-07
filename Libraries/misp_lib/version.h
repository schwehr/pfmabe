
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef VERSION

#define     VERSION     "PFM Software - libmisp V1.42 - 06/11/09"

#endif



/*

    Version 1.0
    Jan C. Depner
    09/18/02


    Version 1.1
    Jan C. Depner
    05/30/03

    Fixed bug in positioning for filter borders.


    Version 1.2
    Jan C. Depner
    10/22/03

    Fixed a tiny bug that wasn't actually hurting anything and removed a
    couple of useless lines.


    Version 1.21
    Jan C. Depner
    03/30/04

    Added an error return from misp_init if it couldn't get enough memory.
    Fixed a couple of small problems and removed some useless code.


    Version 1.22
    Jan C. Depner
    04/11/06

    Changed to handle different x and y grid intervals.  Not tested yet but it doesn't break
    anything as long as they're the same.


    Version 1.23
    Jan C. Depner
    10/13/06

    Modified misp_rtrv.  It wasn't dumping all of the rows properly.


    Version 1.3
    Jan C. Depner
    02/16/07

    Modified misp_rtrv.  It still wasn't dumping all of the rows properly, nor the columns
    (for chrtr).


    Version 1.4
    Jan C. Depner
    07/17/07

    Added NV_BOOL returns for a couple of calls.  Also added the misp_progress_callback
    stuff.


    Version 1.41
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 1.42
    Jan C. Depner
    06/11/09

    Changed iterate to misp_iterate due to collision with GMT crap.

*/

