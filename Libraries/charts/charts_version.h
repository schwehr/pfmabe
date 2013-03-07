
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef CHARTS_VERSION

#define     CHARTS_VERSION     "PFM Software - charts library V1.12 - 10/06/11"

#endif


/*

    Version 1.00
    08/02/03
    Jan C. Depner

    First version.


    Version 1.01
    04/08/08
    Jan C. Depner

    Added this file to track version changes.  Replaced %Ld formats with NV_INT64_SPECIFIER so things work properly
    on Windows.


    Version 1.02
    04/25/08
    Jan C. Depner

    Fixed leap year error in charts_cvtime.c


    Version 1.03
    04/28/08
    Jan C. Depner

    Added tide status to dump routine.


    Version 1.04
    06/02/08
    Jan C. Depner

    Added hof_get_errors function.


    Version 1.05
    03/12/09
    Jan C. Depner

    Added depth check (< -998.0) to hof_uncertainty calc.


    Version 1.06
    05/27/09
    Jan C. Depner

    Added start_timestamp and end_timestamp to WAVE_HEADER_T structure.


    Version 1.07
    05/28/09
    Jan C. Depner

    Added file_size to image file header structure.


    Version 1.08
    06/08/09
    Jan C. Depner

    Removed LAS I/O.  Now using open source package liblas.


    Version 1.09
    07/29/09
    Jan C. Depner

    Added code to retrieve the ac_zero_level from the hardware header block of the wave (.inh) files.


    Version 1.10
    10/27/10
    Jan C. Depner

    Now reads the unknown shot data as NV_U_BYTE so we can try to figure out what it is.


    Version 1.11
    09/08/11
    Kevin M Byrd

    Added pos_get_timestamp() function to pos_io.c and FilePOSOutput.h and it returns a NV_INT64.
    It returns the time in microseconds from 1970 so that it may be used in conjunction with
    cvtime to convert to a year, julian day, hour, minute, second format.
    Also tweeked the WEEK_OFFSET defined variable to negate a bad conversion calculation for when
    the midnight flag is on and needing to calculate time_found.  Code altered where WEEK_OFFSET was
    defined and where "...checking for crossing midnight at end of GPS week..." code snippet is at.


    Version 1.12
    10/06/11
    Jan C. Depner

    Added 3 decimal places to seconds in dump_hof, dump_tof, and dump_wave.

*/
