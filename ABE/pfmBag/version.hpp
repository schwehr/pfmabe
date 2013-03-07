
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

#define     VERSION     "PFM Software - pfmBag V3.30 - 11/04/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    06/17/08

    First working version.


    Version 1.01
    Jan C. Depner
    12/04/08

    Added code to correct target depths to nearest PFM_SELECTED_FEATURE depth or nearest valid depth in
    the associated PFM.  We also correct the target XML file to match.


    Version 1.02
    Jan C. Depner
    12/11/08

    Fixed free of depth records if none were read.


    Version 1.03
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 1.10
    Jan C. Depner
    04/05/09

    Removed support for NAVO standard target file (XML) and replaced with Binary Feature Data (BFD) file support.


    Version 1.11
    Jan C. Depner
    04/24/09

    Fixed bug if classification authority was N/A.


    Version 1.12
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 1.13
    Jan C. Depner
    05/26/09

    Fix "writing tracking list" progress bar bug.  Make the weighted surface an option.


    Version 1.14
    Jan C. Depner
    06/24/09

    Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


    Version 1.15
    Jan C. Depner
    08/04/09

    Changed to use OpenNS 1.2 RC1.


    Version 1.16
    Jan C. Depner
    07/09/10

    Use diagonal of bin size when creating weighted surface for "enhanced" BAG surface.  Use minimum depth
    anywhere inside the feature search radius (no longer using blend of min and avg).


    Version 1.17
    Jan C. Depner
    09/17/10

    Now uses the "max dist" information from pfmFeature to determine the radius around each feature to be used for
    the "enhanced" surface.
    Uses a blend of min and max based on a log curve moving away from the shoal point when creating the enhanced surface.
    Distance to end of log curve is defined by the feature radius.


    Version 2.00
    Jan C. Depner
    11/15/10

    Allows user to select a bin size other than the bin size used in the PFM.  This is slower but the flexibility is
    greatly enhanced.  Had to remove options other than the Standard Deviation and T-Quantile from the uncertainty since
    we're not tied to the PFM.


    Version 2.01
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 2.02
    Jan C. Depner
    03/18/11

    Fixed bug in standard deviation computation.  Also, added ability to output the average layer to the BAG when we are
    creating an enhanced surface (when it finally gets implemented in BAG).


    Version 3.00
    Jan C. Depner
    03/21/11

    UTM - UTM - UTM!!!  Now produces UTM output.  Is it compatible with SABER?  Who the hell knows.
    Outputs the Average layer when building the enhanced surface.  Unfortunately, I've had to place the Average
    surface in the Nominal_Elevation layer since the Average optional layer has not yet been implemented in BAG.
    Also, removed useless projections from projection combo box.  BAG only supports Geodetic and UTM.


    Version 3.01
    Jan C. Depner
    04/06/11

    Now puts the Average layer in the Average layer (go figure).  I had to patch the BAG library to get it to work.


    Version 3.10
    Jan C. Depner
    04/24/11

    Added ellipsoid/datum separation surface generation from a CHRTR2 datum surface file.  At a meeting with Steve Harrison,
    Wade Ladner, and Bill Rankin, we decided to remove the "Average" surface since it won't significantly affect model
    generation to use the enhanced surface (since most models will use larger bins with averaged enhanced points).


    Version 3.11
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


    Version 3.12
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.


    Version 3.13
    Jan C. Depner
    07/25/11

    Added ability to use ASCII sep file as well as CHRTR2 sep file.


    Version 3.14
    Jan C. Depner
    08/02/11

    Bug fix relating to sep file inclusion.  Segfault if you didn't include the sep file.


    Version 3.15
    Jan C. Depner
    09/21/11

    Replaced compute_index calls with compute_index_ptr calls.


    Version 3.20
    Jan C. Depner
    09/23/11

    Now using BAG 1.4.0 (compressing).


    Version 3.30
    Jan C. Depner
    11/04/11

    Added CUBE Surface to the surface menu.  Added Average TPE to the uncertainty menu.  Blend the
    Average TPE or Final Uncertainty with the TPE fo the minimum sounding in the area of features
    when creating the enhanced navigation surface.

*/
