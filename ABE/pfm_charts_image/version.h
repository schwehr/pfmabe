
/*********************************************************************************************

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef VERSION

#define     VERSION     "PFM Software - pfm_charts_image V5.25 - 05/06/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    10/26/04

    First working version.


    Version 1.01
    Jan C. Depner
    11/05/04

    Changed sign on heading for ERDAS, output R,P instead of P,R.


    Version 1.02
    Jan C. Depner
    12/16/04

    Changed Usage message for PFM 4.5 directory input.


    Version 1.03
    Jan C. Depner
    01/14/05

    Output DEM in UTM coordinates.


    Version 1.04
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 1.1
    Jan C. Depner
    03/14/05

    Now handles HOF files for those rare occasions when you go over
    land but don't have TOF files.


    Version 2.0
    Jan C. Depner
    08/09/05

    Hopefully we now have everything right for ERDAS to do the photo mosaicing.


    Version 2.01
    Jan C. Depner
    08/12/05

    Added -n option to override limits on too dark or light pictures.


    Version 2.02
    Jan C. Depner
    10/12/05

    Added -m option to decimate the number of pictures output (Every mth picture).


    Version 2.03
    Jan C. Depner
    10/26/05

    Changed usage for PFM 4.6 handle file use.


    Version 2.04
    Jan C. Depner
    01/03/06

    Minor change to image_read_record call.


    Version 2.05
    Jan C. Depner
    06/05/06

    Removed inside.c, get_area_mbr.c, ellpfns.c, projfns.c, and tmfns.c.  Moved to utility.


    Version 2.1
    Jan C. Depner
    07/31/06

    Added pos time offset option.  Changed name of pos.txt output file to pos.dat.  Added area file name to
    beginning of pos.dat and dem.dat (areaname_pos.dat).  Added flip sign option.  Fixed the omega, phi, kappa
    computations and the interpolation in the charts pos_io function...  DOH!!!


    Version 2.11
    Jan C. Depner
    10/16/06

    Added zone to end of DEM and .txt files.


    Version 2.2
    Jan C. Depner
    10/17/06

    Added JAM output option.


    Version 2.21
    Jan C. Depner
    01/25/07

    Check for upper case SBET file name to correct for GCS screwup.


    Version 3.00
    Jan C. Depner
    06/18/07

    Check for filename in .kin file actually being correct.  Fix my seriously major
    screwup trying to find the related image records.


    Version 4.00
    Jan C. Depner
    06/28/07

    Use MISP to create a DEM of any grid size.  Set water values to 0.0


    Version 5.00
    Jan C. Depner
    09/25/07

    Changed most of the options and added normalization, pos exclusion, data type, and other options.


    Version 5.01
    Jan C. Depner
    10/22/07

    Added fflush calls after prints to stderr since flush is not automatic in Windows.


    Version 5.02
    Jan C. Depner
    11/06/07

    Added SRTM data input.


    Version 5.10
    Jan C. Depner
    11/13/07

    New version of mosaic now allows output in utm or geographic therefor, removed GDALWARP stuff
    and added utm/geographic option.


    Version 5.11
    Jan C. Depner
    01/13/08

    Keeps hits for NULL data since the picture should be OK anyway.


    Version 5.12
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 5.13
    Jan C. Depner
    07/16/08

    Check for availablility of SRTM data.


    Version 5.14
    Jan C. Depner/Chris Macon
    07/29/08

    Fixed DEM logic for EGM96 datum shift in misp.  Added computation for convergence into the omega,
    phi, kappa calculations.


    Version 5.15
    Jan C. Depner/Chris Macon
    07/30/08

    Fixed redundant fclose of pos files when skipping POS vs SBET.


    Version 5.16
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 5.17
    Chris Macon
    04/09/09

    Added "Flip" option detection when shifting the SRTM data to ellipsoid heights
    Added a check to eliminate "0" values from being added into the misp surface


    Version 5.18
    Chris Macon
    04/10/09

    Fixed SRTM single point read problem.  Now generates DEM from all valid data points
    instead of just the bins from one of the surfaces.


    Version 5.19
    Jan Depner
    04/28/09

    We were misping both the depth values and the bins.  We only want to misp the depth values.


    Version 5.20
    Jan C. Depner
    07/29/09

    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 5.21
    Jan C. Depner
    08/20/09

    Fixed bug when dealing with PFM_CHARTS_HOF_DATA and/or PFM_SHOALS_1K_DATA.


    Version 5.22
    Jan C. Depner
    12/02/09

    Replaced get_egm96 with get_egm08.


    Version 5.23
    Jan C. Depner
    08/17/10

    Replaced our kludgy old UTM transform with calls to the PROJ 4 library functions.  All hail the PROJ 4 developers!


    Version 5.24
    Jan C. Depner
    10/04/10

    Skip image records with bad time stamps (up to 100).


    Version 5.25
    Jan C. Depner
    05/06/11

    Fixed problem with getopt that only happens on Windows.

*/
