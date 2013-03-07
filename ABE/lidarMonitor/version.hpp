
/*********************************************************************************************

    This program is public domain software that was developed by the U.S. Naval Oceanographic
    Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef VERSION

#ifdef OPTECH_CZMIL
#define     VERSION     "CME Software - Attribute List V2.10 - 07/21/11"
#else
#define     VERSION     "PFM Software - lidarMonitor V2.10 - 07/21/11"
#endif

#endif

/*

    Version 1.0
    Jan C. Depner
    11/25/08

    First working version.


    Version 1.01
    Jan C. Depner
    11/28/08

    Added kill_switch option.


    Version 1.02
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 1.03
    Jan C. Depner
    06/15/09

    Added support for PFM_CHARTS_HOF_DATA and PFM_WLF_DATA.


    Version 1.04
    Jan C. Depner
    09/16/09

    Set killed flag in abe_share when program exits other than from kill switch from parent.


    Version 1.05
    Jan C. Depner
    04/29/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 1.06
    Jan C. Depner
    06/25/10

    Added support for PFM_HAWKEYE_DATA.


    Version 2.00
    Jan C. Depner
    09/20/10

    Changed name to lidarMonitor from chartsMonitor since we're now supporting more data types than
    just CHARTS.


    Version 2.01
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 2.02
    Jan C. Depner
    02/25/11

    Switched to using PFM_HAWKEYE_HYDRO_DATA and PFM_HAWKEYE_TOPO_DATA to replace the deprecated PFM_HAWKEYE_DATA.


    Version 2.10
    Jan C. Depner
    07/21/11

    Added DRAFT CZMIL data support.

*/
