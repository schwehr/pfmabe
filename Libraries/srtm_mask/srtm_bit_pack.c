
/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/

#include <math.h>
#include <stdio.h>

#include "nvtypes.h"
#include "nvdef.h"



/*****************************************************************************\

                           IMPORTANT NOTE

    The double_bit_pack and double_bit_unpack routines have been shown
    to have inconsistencies in some rare cases.  Unfortunately, I built the
    SRTM files using these so we have to keep them around.  Due to the fact
    that hardly any of the files exceed 2GB I doubt that this will cause
    any problems.  If it did it would be in the area of the north pole as that 
    is the last data that is loaded in to each file.  So far I have not seen
    any problems.  JCD

\*****************************************************************************/




static NV_U_BYTE        mask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 
                        0xfe}, notmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 
                        0x07, 0x03, 0x01}; 


/***************************************************************************\ 
*                                                                           * 
*   Function        bit_pack - Packs a long value into consecutive bits in  * 
*                   buffer.                                                 * 
*                                                                           * 
*   Synopsis        bit_pack (buffer, start, numbits, value);               * 
*                                                                           * 
*                   NV_U_BYTE buffer[]      address of buffer to use        * 
*                   NV_U_INT32 start        start bit position in buffer    * 
*                   NV_U_INT32 numbits      number of bits to store         * 
*                   NV_INT32 value          value to store                  * 
*                                                                           * 
*   Description     Packs the value 'value' into 'numbits' bits in 'buffer' * 
*                   starting at bit position 'start'.  The majority of      * 
*                   this code is based on Appendix C of Naval Ocean         * 
*                   Research and Development Activity Report #236, 'Data    * 
*                   Base Structure to Support the Production of the Digital * 
*                   Bathymetric Data Base', Nov. 1989, James E. Braud,      * 
*                   John L. Breckenridge, James E. Current, Jerry L.        * 
*                   Landrum.                                                * 
*                                                                           * 
*   Returns         void                                                    * 
*                                                                           * 
*   Author          Jan C. Depner                                           * 
*                                                                           * 
\***************************************************************************/ 
 
 
void srtm_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT32 value) 
{ 
    NV_INT32                start_byte, end_byte, start_bit, end_bit, i; 
 
 
    i = start + numbits; 

    /*  Right shift the start and end by 3 bits, this is the same as        */ 
    /*  dividing by 8 but is faster.  This is computing the start and end   */ 
    /*  bytes for the field.                                                */ 
     
    start_byte = start >> 3; 
    end_byte = i >> 3; 
 
    /*  AND the start and end bit positions with 7, this is the same as     */ 
    /*  doing a mod with 8 but is faster.  Here we are computing the start  */ 
    /*  and end bits within the start and end bytes for the field.          */ 
     
    start_bit = start & 7; 
    end_bit = i & 7; 
 
    /*  Compute the number of bytes covered.                                */ 
     
    i = end_byte - start_byte - 1; 

    /*  If the value is to be stored in one byte, store it.                 */ 
     
    if (start_byte == end_byte) 
    { 
        /*  Rather tricky.  We are masking out anything prior to the start  */ 
        /*  bit and after the end bit in order to not corrupt data that has */ 
        /*  already been stored there.                                      */ 
         
        buffer[start_byte] &= mask[start_bit] | notmask[end_bit]; 
 
        /*  Now we mask out anything in the value that is prior to the      */ 
        /*  start bit and after the end bit.  This is, of course, after we  */ 
        /*  have shifted the value left past the end bit.                   */ 
         
        buffer[start_byte] |= (value << (8 - end_bit)) & 
            (notmask[start_bit] & mask[end_bit]); 
    } 
 
    /*  If the value covers more than 1 byte, store it.                     */ 
     
    else 
    { 
        /*  Here we mask out data prior to the start bit of the first byte. */ 
         
        buffer[start_byte] &= mask[start_bit]; 
 
        /*  Get the upper bits of the value and mask out anything prior to  */ 
        /*  the start bit.  As an example of what's happening here, if we   */ 
        /*  wanted to store a 14 bit field and the start bit for the first  */ 
        /*  byte is 3, we would be storing the upper 5 bits of the value in */ 
        /*  the first byte.                                                 */ 
         
        buffer[start_byte++] |= (value >> (numbits - (8 - start_bit))) & 
            notmask[start_bit]; 
 
        /*  Loop while decrementing the byte counter.                       */ 
         
        while (i--) 
        { 
            /*  Clear the entire byte.                                      */ 
             
       	    buffer[start_byte] &= 0; 
 
       	    /*  Get the next 8 bits from the value.                         */ 
       	     
       	    buffer[start_byte++] |= (value >> ((i << 3) + end_bit)) & 255; 
       	} 
 
       	/*  For the last byte we mask out anything after the end bit.       */ 
       	 
        buffer[start_byte] &= notmask[end_bit]; 
 
        /*  Get the last part of the value and stuff it in the end byte.    */ 
        /*  The left shift effectively erases anything above 8 - end_bit    */ 
        /*  bits in the value so that it will fit in the last byte.         */ 
         
        buffer[start_byte] |= (value << (8 - end_bit)); 
    } 
} 
 
 
 
/***************************************************************************\ 
*                                                                           * 
*   Function        bit_unpack - Unpacks a long value from consecutive bits * 
*                   in buffer.                                              * 
*                                                                           * 
*   Synopsis        bit_unpack (buffer, start, numbits);                    * 
*                                                                           * 
*                   NV_U_BYTE buffer[]      address of buffer to use        * 
*                   NV_U_INT32 start        start bit position in buffer    * 
*                   NV_U_INT32 numbits      number of bits to retrieve      * 
*                                                                           * 
*   Description     Unpacks the value from 'numbits' bits in 'buffer'       * 
*                   starting at bit position 'start'.  The value is assumed * 
*                   to be unsigned.  The majority of this code is based on  * 
*                   Appendix C of Naval Ocean Research and Development      * 
*                   Activity Report #236, 'Data Base Structure to Support   * 
*                   the Production of the Digital Bathymetric Data Base',   * 
*                   Nov. 1989, James E. Braud, John L. Breckenridge, James  * 
*                   E. Current, Jerry L. Landrum.                           * 
*                                                                           * 
*   Returns         NV_U_INT32              value retrieved from buffer     * 
*                                                                           * 
*   Author          Jan C. Depner                                           * 
*                                                                           * 
\***************************************************************************/ 
 
 
NV_U_INT32 srtm_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits) 
{ 
    NV_INT32                start_byte, end_byte, start_bit, end_bit, i; 
    NV_U_INT32              value; 
 
 
    i = start + numbits; 
 
    /*  Right shift the start and end by 3 bits, this is the same as        */ 
    /*  dividing by 8 but is faster.  This is computing the start and end   */ 
    /*  bytes for the field.                                                */ 
     
    start_byte = start >> 3; 
    end_byte = i >> 3; 
 
    /*  AND the start and end bit positions with 7, this is the same as     */ 
    /*  doing a mod with 8 but is faster.  Here we are computing the start  */ 
    /*  and end bits within the start and end bytes for the field.          */ 
     
    start_bit = start & 7; 
    end_bit = i & 7; 
 
    /*  Compute the number of bytes covered.                                */ 
     
    i = end_byte - start_byte - 1; 
 
    /*  If the value is stored in one byte, retrieve it.                    */ 
     
    if (start_byte == end_byte) 
    { 
        /*  Mask out anything prior to the start bit and after the end bit. */ 
         
        value = (NV_U_INT32) buffer[start_byte] & (notmask[start_bit] &
            mask[end_bit]); 
 
        /*  Now we shift the value to the right.                            */ 
         
        value >>= (8 - end_bit); 
    } 
 
    /*  If the value covers more than 1 byte, retrieve it.                  */ 
     
    else 
    { 
        /*  Here we mask out data prior to the start bit of the first byte  */ 
        /*  and shift to the left the necessary amount.                     */ 
         
        value = (NV_U_INT32) (buffer[start_byte++] & notmask[start_bit]) <<
            (numbits - (8 - start_bit)); 
 
        /*  Loop while decrementing the byte counter.                       */ 
         
        while (i--) 
        { 
       	    /*  Get the next 8 bits from the buffer.                        */ 
       	     
       	    value += (NV_U_INT32) buffer[start_byte++] << ((i << 3) + end_bit); 
       	} 
 
       	/*  For the last byte we mask out anything after the end bit and    */ 
       	/*  then shift to the right (8 - end_bit) bits.                       */ 
       	 
        value += (NV_U_INT32) (buffer[start_byte] & mask[end_bit]) >> (8 - end_bit); 
    } 

    return (value); 
} 



/***************************************************************************\ 
*                                                                           * 
*   Function        double_bit_pack - Packs a long long integer value into  *
*                   consecutive bits in buffer.                             *
*                                                                           * 
*   Synopsis        double_bit_pack (buffer, start, numbits, value);        * 
*                                                                           * 
*                   NV_U_BYTE buffer[]      address of buffer to use        * 
*                   NV_U_INT32 start        start bit position in buffer    * 
*                   NV_U_INT32 numbits      number of bits to store         * 
*                   NV_INT64 value          value to store                  * 
*                                                                           * 
*   Description     Packs the value 'value' into 'numbits' bits in 'buffer' *
*                   starting at bit position 'start'.                       *
*                                                                           * 
*   Returns         void                                                    * 
*                                                                           * 
*   Author          Jan C. Depner                                           * 
*                                                                           * 
\***************************************************************************/ 
 
 
void srtm_double_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT64 value) 
{
    NV_INT32            high_order, low_order;


    high_order = (NV_INT32) (value / NV_INT32_MAX);
    low_order = (NV_INT32) (value % (NV_INT64) NV_INT32_MAX);

    srtm_bit_pack (buffer, start, numbits - 31, high_order);

    srtm_bit_pack (buffer, start + (numbits - 31), 31, low_order);
}



/***************************************************************************\ 
*                                                                           * 
*   Function        double_bit_unpack - Unpacks a long long integer value   *
*                   from consecutive bits in buffer.                        *
*                                                                           * 
*   Synopsis        double_bit_unpack (buffer, start, numbits);             * 
*                                                                           * 
*                   NV_U_BYTE buffer[]      address of buffer to use        * 
*                   NV_U_INT32 start        start bit position in buffer    * 
*                   NV_U_INT32 numbits      number of bits to store         * 
*                                                                           * 
*   Description     Unpacks a value from 'numbits' bits in 'buffer'         *
*                   starting at bit position 'start'.  It is then unscaled  *
*                   by 'scale'.                                             *
*                                                                           * 
*   Returns         NV_INT64                Value unpacked from buffer      * 
*                                                                           * 
*   Author          Jan C. Depner                                           * 
*                                                                           * 
\***************************************************************************/ 
 
 
NV_INT64 srtm_double_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits) 
{
    NV_INT32            high_order, low_order;


    high_order = srtm_bit_unpack (buffer, start, numbits - 31);

    low_order = srtm_bit_unpack (buffer, start + (numbits - 31), 31);

    return ((NV_INT64) high_order * NV_INT32_MAX + (NV_INT64) low_order);
}
