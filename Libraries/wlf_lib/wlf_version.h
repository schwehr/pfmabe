
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef WLF_VERSION

#define     WLF_VERSION "PFM Software - WLF library V1.05 - 10/26/10"

#endif

/*

    Version 1.00
    02/25/09
    Jan C. Depner

    First version.


    Version 1.01
    08/06/09
    Jan C. Depner

    Added waveform and waveform_point to point record.  This still hasn't been released yet so
    adding pieces isn't a problem.


    Version 1.02
    04/29/10
    Jan C. Depner

    Microsoft, in their infinite wisdom, doesn't support %hhd in scanf statements (like the rest of
    the world does) so I had to use %hd, slap it into a short int and then copy the value to an
    unsigned char.  What's really weird is that %hhd works fine for printf.  DOH!


    Version 1.03
    Jan C. Depner
    04/30/10

    Now use "const NV_CHAR *" arguments instead of "NV_CHAR *" arguments where applicable (gcc 4.4 doesn't like 
    you calling functions with constant strings [i.e. "fred"] if the argument in the function isn't declared const).


    Version 1.04
    Jan C. Depner
    08/12/10

    Sets default wlf_user_flag names if they weren't defined.


    Version 1.05
    Jan C. Depner
    10/26/10

    Fix screwup when reading version string.

*/
