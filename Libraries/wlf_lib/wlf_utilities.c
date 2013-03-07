
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#include "wlf.h"


static NV_U_BYTE        mask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 
                        0xfe}, notmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 
                        0x07, 0x03, 0x01}; 


/*  Months start at zero, days at 1 (go figure).  */

static NV_INT32              months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};



/*********************************************************************************************

    Function        wlf_bit_pack - Packs a long value into consecutive bits in buffer.

    Synopsis        wlf_bit_pack (buffer, start, numbits, value);

                    NV_U_BYTE buffer[]      address of buffer to use
                    NV_U_INT32 start        start bit position in buffer
                    NV_U_INT32 numbits      number of bits to store
                    NV_INT32 value          value to store

    Description     Packs the value 'value' into 'numbits' bits in 'buffer' starting at bit
                    position 'start'.  The majority of this code is based on Appendix C of
                    Naval Ocean Research and Development Activity Report #236, 'Data Base
                    Structure to Support the Production of the Digital Bathymetric Data Base',
                    Nov. 1989, James E. Braud, John L. Breckenridge, James E. Current, Jerry L.
                    Landrum.

    Returns         void

    Author          Jan C. Depner

*********************************************************************************************/

WLF_DLL void wlf_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT32 value) 
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

      buffer[start_byte] |= (value << (8 - end_bit)) & (notmask[start_bit] & mask[end_bit]); 
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

      buffer[start_byte++] |= (value >> (numbits - (8 - start_bit))) & notmask[start_bit]; 


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
 
 
 
/*********************************************************************************************

    Function        bit_unpack - Unpacks a long value from consecutive bits in buffer.

    Synopsis        bit_unpack (buffer, start, numbits);

                    NV_U_BYTE buffer[]      address of buffer to use
                    NV_U_INT32 start        start bit position in buffer
                    NV_U_INT32 numbits      number of bits to retrieve

    Description     Unpacks the value from 'numbits' bits in 'buffer' starting at bit position
                    'start'.  The value is assumed to be unsigned.  The majority of this code
                    is based on Appendix C of Naval Ocean Research and Development Activity
                    Report #236, 'Data Base Structure to Support the Production of the
                    Digital Bathymetric Data Base', Nov. 1989, James E. Braud, John L.
                    Breckenridge, James E. Current, Jerry L. Landrum.

    Returns         NV_U_INT32              value retrieved from buffer

    Caveats         Note that the value that is output from this function is an unsigned 32
                    bit integer.  Even though you may passed in a signed 32 bit value to
                    wlf_bit_pack it will not be sign extended on the way out.  If you just
                    happenned to store it in 32 bits you can just typecast it to a signed
                    32 bit number and, lo and behold, you have a nice, signed number.  
                    Otherwise, you have to do the sign extension yourself.

    Author          Jan C. Depner

*********************************************************************************************/
 
WLF_DLL NV_U_INT32 wlf_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits) 
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

      value = (NV_U_INT32) buffer[start_byte] & (notmask[start_bit] & mask[end_bit]); 


      /*  Now we shift the value to the right.                            */ 

      value >>= (8 - end_bit); 
    } 


  /*  If the value covers more than 1 byte, retrieve it.                  */ 

  else
    {
      /*  Here we mask out data prior to the start bit of the first byte  */ 
      /*  and shift to the left the necessary amount.                     */ 

      value = (NV_U_INT32) (buffer[start_byte++] & notmask[start_bit]) << (numbits - (8 - start_bit)); 


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



/*********************************************************************************************

    Function        wlf_double_bit_pack - Packs a long long integer value into consecutive
                    bits in buffer.

    Synopsis        wlf_double_bit_pack (buffer, start, numbits, value);

                    NV_U_BYTE buffer[]      address of buffer to use
                    NV_U_INT32 start        start bit position in buffer
                    NV_U_INT32 numbits      number of bits to store
                    NV_INT64 value          value to store

    Description     Packs the value 'value' into 'numbits' bits in 'buffer' starting at bit
                    position 'start'.

    Returns         void

    Author          Jan C. Depner

*********************************************************************************************/
 
WLF_DLL void wlf_double_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT64 value) 
{
  NV_INT32            high_order, low_order;


  high_order = (NV_INT32) (((NV_U_INT64) value) >> 32);
  low_order  = (NV_INT32) (value & NV_U_INT32_MAX);


  /*  I hate doing this but I keep getting caught by computing the number of bits for something
      that might be a 64 bit integer and only asking for less than 33 bits.  If statements are 
      SLOOOOOOOOOOOOW.  */

  if (numbits < 33)
    {
      wlf_bit_pack (buffer, start, numbits, low_order);
    }
  else
    {
      wlf_bit_pack (buffer, start, numbits - 32, high_order);
      wlf_bit_pack (buffer, start + (numbits - 32), 32, low_order);
    }
}



/*********************************************************************************************

    Function        wlf_double_bit_unpack - Unpacks a long long integer value from consecutive
                    bits in buffer.

    Synopsis        wlf_double_bit_unpack (buffer, start, numbits);

                    NV_U_BYTE buffer[]      address of buffer to use
                    NV_U_INT32 start        start bit position in buffer
                    NV_U_INT32 numbits      number of bits to store

    Description     Unpacks a value from 'numbits' bits in 'buffer' starting at bit position
                    'start'.

    Returns         NV_U_INT64              Value unpacked from buffer

    Author          Jan C. Depner

*********************************************************************************************/
 
WLF_DLL NV_U_INT64 wlf_double_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits) 
{
  NV_U_INT64          result;
  NV_U_INT32          high_order, low_order;


  /*  I hate doing this but I keep getting caught by computing the number of bits for something
      that might be a 64 bit integer and only asking for less than 33 bits.  If statements are 
      SLOOOOOOOOOOOOW.  */

  if (numbits < 33)
    {
      high_order = 0;
      low_order  = wlf_bit_unpack (buffer, start, numbits);
    }
  else
    {
      high_order = wlf_bit_unpack (buffer, start, numbits - 32);
      low_order  = wlf_bit_unpack (buffer, start + (numbits - 32), 32);
    }


  result = ((NV_U_INT64) high_order) << 32;
  result |= (NV_U_INT64) low_order;

  return (result);
}




/*********************************************************************************************

    Function        wlf_cvtime - Convert from POSIX time to year, day of year, hour, minute,
                    second.

    Synopsis        wlf_cvtime (tv_sec, tv_nsec, &year, &jday, &hour, &minute, &second);

                    time_t tv_sec           POSIX seconds from epoch (Jan. 1, 1970)
                    long tv_nsec            POSIX nanoseconds of second
                    NV_INT32 year           4 digit year - 1900
                    NV_INT32 jday           day of year
                    NV_INT32 hour           hour of day
                    NV_INT32 minute         minute of hour
                    NV_FLOAT32 second       seconds of minute

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The localtime function will return the year as a 2 digit year (offset
                    from 1900).

*********************************************************************************************/
 
WLF_DLL void wlf_cvtime (time_t tv_sec, long tv_nsec, NV_INT32 *year, NV_INT32 *jday, NV_INT32 *hour,
                 NV_INT32 *minute, NV_FLOAT32 *second)
{
  static NV_INT32      tz_set = 0;
  struct tm            time_struct, *time_ptr = &time_struct;

  if (!tz_set)
    {
#ifdef NVWIN3X
  #ifdef __MINGW64__
      putenv ("TZ=GMT");
      tzset ();
  #else
      _putenv ("TZ=GMT");
      _tzset ();
  #endif
#else
      putenv ("TZ=GMT");
      tzset ();
#endif
      tz_set = 1;
    }

  time_ptr = localtime (&tv_sec);

  *year = (NV_INT16) time_ptr->tm_year;
  *jday = (NV_INT16) time_ptr->tm_yday + 1;
  *hour = (NV_INT16) time_ptr->tm_hour;
  *minute = (NV_INT16) time_ptr->tm_min;
  *second = (NV_FLOAT32) time_ptr->tm_sec + (NV_FLOAT32) ((NV_FLOAT64) tv_nsec / 1000000000.);
}



/*********************************************************************************************

    Function        wlf_jday2mday - Convert from day of year to month and day.

    Synopsis        wlf_jday2mday (year, jday, &mon, &mday);

                    NV_INT32 year           4 digit year or offset from 1900
                    NV_INT32 jday           day of year
                    NV_INT32 mon            month
                    NV_INT32 mday           day of month

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The returned month value will start at 0 for January.

*********************************************************************************************/
 
WLF_DLL void wlf_jday2mday (NV_INT32 year, NV_INT32 jday, NV_INT32 *mon, NV_INT32 *mday)
{
  NV_INT32 l_year;

  l_year = year;

  if (year < 1899) l_year += 1900;


  /*  If the year is evenly divisible by 4 but not by 100, or it's evenly divisible by 400, this is a leap year.  */

  if ((!(l_year % 4) && (l_year % 100)) || !(l_year % 400))
    {
      months[1] = 29;
    }
  else
    {
      months[1] = 28;
    }

  *mday = jday;
  for (*mon = 0 ; *mon < 12 ; (*mon)++)
    {
      if (*mday - months[*mon] <= 0) break;

      *mday -= months[*mon];
    }
}



/*********************************************************************************************

    Function        wlf_inv_cvtime - Convert from year, day of year, hour, minute, second to
                    POSIX time.

    Synopsis        wlf_inv_cvtime (year, jday, hour, minute, second, &tv_sec, &tv_nsec);

                    NV_INT32 year           4 digit year - 1900
                    NV_INT32 jday           day of year
                    NV_INT32 hour           hour of day
                    NV_INT32 minute         minute of hour
                    NV_FLOAT32 second       seconds of minute
                    time_t tv_sec           POSIX seconds from epoch (Jan. 1, 1970)
                    long tv_nsec            POSIX nanoseconds of second

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The mktime function wants a 2 digit year (offset from 1900).

*********************************************************************************************/
 
WLF_DLL void wlf_inv_cvtime (NV_INT32 year, NV_INT32 jday, NV_INT32 hour, NV_INT32 min, NV_FLOAT32 sec,
                             time_t *tv_sec, long *tv_nsec)
{
  struct tm                    tm;
  static NV_INT32              tz_set = 0;


  tm.tm_year = year;

  wlf_jday2mday (year, jday, &tm.tm_mon, &tm.tm_mday);

  tm.tm_hour = hour;
  tm.tm_min = min;
  tm.tm_sec = (NV_INT32) sec;
  tm.tm_isdst = 0;

  if (!tz_set)
    {
#ifdef NVWIN3X
  #ifdef __MINGW64__
      putenv ("TZ=GMT");
      tzset ();
  #else
      _putenv ("TZ=GMT");
      _tzset ();
  #endif
#else
      putenv ("TZ=GMT");
      tzset ();
#endif
      tz_set = 1;
    }

  *tv_sec = mktime (&tm);
  *tv_nsec = (long)(fmod ((double) sec, 1.0) * 1.0e9);
}



/*********************************************************************************************

    Function        wlf_mday2jday - Given year, month, and day, return day of year.

    Synopsis        wlf_mday2jday (year, mon. mday, &jday);

                    NV_INT32 year           4 digit year or offset from 1900
                    NV_INT32 mon            month
                    NV_INT32 mday           day of month
                    NV_INT32 jday           day of year

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The month value must start at 0 for January.

*********************************************************************************************/
 
WLF_DLL void wlf_mday2jday (NV_INT32 year, NV_INT32 mon, NV_INT32 mday, NV_INT32 *jday)
{
  NV_INT32 i, l_year;

  l_year = year;

  if (year < 1899) l_year += 1900;


  /*  If the year is evenly divisible by 4 but not by 100, or it's evenly divisible by 400, this is a leap year.  */

  if ((!(l_year % 4) && (l_year % 100)) || !(l_year % 400))
    {
      months[1] = 29;
    }
  else
    {
      months[1] = 28;
    }


  *jday = mday;
  for (i = 0 ; i < mon - 1 ; i++) *jday += months[i];
}



/*********************************************************************************************

    Module Name:        wlf_ngets

    Programmer(s):      Jan C. Depner

    Date Written:       May 1999

    Purpose:            This is an implementation of fgets that strips the line feed (or
                        carriage return/line feed) off of the end of the string if present.

    Arguments:          See fgets

    Return Value:       See fgets

*********************************************************************************************/

WLF_DLL NV_CHAR *wlf_ngets (NV_CHAR *s, NV_INT32 size, FILE *stream)
{
  if (fgets (s, size, stream) == NULL) return (NULL);

  while (strlen(s) > 0 && (s[strlen(s) - 1] == '\n' || s[strlen(s) - 1] == '\r')) s[strlen(s) - 1] = '\0';

  if (s[strlen (s) - 1] == '\n') s[strlen (s) - 1] = 0;


  return (s);
}
