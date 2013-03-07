
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

#define     VERSION     "PFM Software - hofWaveFilter V1.12 - 07/08/11"

#endif

/*

    Version 1.00
    Jan C. Depner
    04/12/11

    First version.


    Version 1.01
    Jan C. Depner
    04/21/11

    Speed improvements.


    Version 1.02
    Jan C. Depner
    05/03/11

    Now sorts on line and then original record number in order to reduce disk thrashing when reading HOF
    and INH records.


    Version 1.03
    Jan C. Depner
    05/05/11

    We're honoring the mask flag set by pfmEdit(3D).  That is, if the point is masked out in the editor we
    won't mark it for deletion.  Happy Cinco de Mayo!!!


    Version 1.10
    Jan C. Depner
    05/09/11

    Re-wrote the apd and pmt return filters to not try to process the entire waveform.  They now just use the
    bin that Optech has determined for each return and check before and after the bin to find the slopes.  This
    is WAAAAAAAAAYYYYYYYYYYYY faster than the old way.


    Version 1.11
    Jan C. Depner
    05/18/11

    Fixed the slope filter.  It wasn't finding the first drop correctly if it was a low slope first drop.
    Moved masking responsibility back into the editors (pfmEdit and pfmEdit3D).


    Version 1.12
    Jan C. Depner
    07/08/11

    Changed the default low slope val;ue to 0.5 instead of 0.75.  We were killing way too many valid points when
    looking for the steeper slope.  Removed slope filter kill for points before first drop.  We were killing points
    where they got near the surface.  The original plan for the prior to first drop kill was to remove surface
    returns but those are pretty easy to spot anyway.

*/
