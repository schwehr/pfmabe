
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

#define     VERSION     "PFM Software - hofReturnKill V3.00 - 05/18/11"

#endif

/*

    Version 1.00
    Jan C. Depner
    06/15/09

    First version.


    Version 1.01
    Jan C. Depner
    08/20/09

    Added shallow water algorithm and shoreline depth swap kill switch.


    Version 2.00
    Jan C. Depner
    01/11/01

    Replaced the old first return kill with just a return kill based on slope and run required.  This actually makes
    more sense now that we're loading both primary and secondary returns.  Also, lowered the acceptable slope to 
    0.75 (based on empirical evidence).


    Version 2.01
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 2.02
    Jan C. Depner
    04/12/11

    Fixed the APD and PMT return filters.  It appears that Optech does not allow a single drop or any number of flat returns to
    end a run.


    Version 3.00
    Jan C. Depner
    05/18/11

    Removed the first return kill option since we weren't using it anymore.  Also, replaced the apd and pmt
    return filters with the simplified versions from hofWaveFilter.  These are faster and better.

*/
