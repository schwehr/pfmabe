
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



/********************************************************************************************/
/*!

  - Module Name:        get_string

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1994

  - Purpose:            Parses the input string for the equals sign and returns everything to
                        the right between the first and last non-blank character (inclusive).

  - Arguments:
                        - *in     =   Input string
                        - *out    =   Output string

  - Return Value:
                        - void

*********************************************************************************************/

static void get_string (const NV_CHAR *in, NV_CHAR *out)
{
    NV_INT32            i, start, length;
    NV_CHAR             *ptr;


    start = 0;
    length = 0;


    ptr = strchr (in, '=') + 1;


    /*  Search for first non-blank character.   */

    for (i = 0 ; i < strlen (ptr) ; i++)
    {
        if (ptr[i] != ' ')
        {
            start = i;
            break;
        }
    }


    /*  Search for last non-blank character.    */

    for (i = strlen (ptr) ; i >= 0 ; i--)
    {
        if (ptr[i] != ' ' && ptr[i] != 0)
        {
            length = (i + 1) - start;
            break;
        }
    }

    strncpy (out, &ptr[start], length);
    out[length] = 0;
}



/********************************************************************************************/
/*!

 - Function:    clean_exit

 - Purpose:     Exit from the application after first cleaning up memory and orphaned files.
                This will only be called in the case of an abnormal exit (like a failed malloc).

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - ret            =    Value to be passed to exit ();  If set to -999 we were
                                      called from the SIGINT signal handler so we must return
                                      to allow it to SIGINT itself.

 - Returns:
                - void

*********************************************************************************************/

static void clean_exit (NV_INT32 ret)
{
  NV_INT32 i;

  for (i = 0 ; i < CHRTR2_MAX_FILES ; i++)
    {
      if (chrtr2h[i].fp != NULL)
        {
          if (chrtr2h[i].created)
            {
              /*  If we were in the process of creating a file we need to remove it since it isn't finished.  */

              fclose (chrtr2h[i].fp);
              remove (chrtr2h[i].path);
            }
        }
    }


  /*  Return to the SIGINT handler.  */

  if (ret == -999 && getpid () > 1) return;


  exit (ret);
}



/********************************************************************************************/
/*!

 - Function:    sigint_handler

 - Purpose:     Simple little SIGINT handler.  Allows us to clean up the files if we were 
                creating a new CHRTR2 file and someone does a CTRL-C.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        04/12/10

 - Arguments:
                - sig            =    The signal

 - Returns:
                - void

 - Caveats:     The way to do this was borrowed from "Proper handling of SIGINT/SIGQUIT",
                http://www.cons.org/cracauer/sigint.html

*********************************************************************************************/

static void sigint_handler (int sig)
{
  clean_exit (-999);

  signal (SIGINT, SIG_DFL);

#ifdef NVWIN3X
  raise (sig);
#else
  kill (getpid (), SIGINT);
#endif
}



static NV_U_BYTE        mask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe}, notmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01}; 


/*  Months start at zero, days at 1 (go figure).  */

static NV_INT32              months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};



/*******************************************************************************************/
/*!

  - Function        bit_pack - Packs a long value into consecutive bits in buffer.

  - Synopsis        bit_pack (buffer, start, numbits, value);
                    - NV_U_BYTE buffer[]      address of buffer to use
                    - NV_U_INT32 start        start bit position in buffer
                    - NV_U_INT32 numbits      number of bits to store
                    - NV_INT32 value          value to store

  - Description     Packs the value 'value' into 'numbits' bits in 'buffer' starting at bit
                    position 'start'.  The majority of this code is based on Appendix C of
                    Naval Ocean Research and Development Activity Report #236, 'Data Base
                    Structure to Support the Production of the Digital Bathymetric Data Base',
                    Nov. 1989, James E. Braud, John L. Breckenridge, James E. Current, Jerry L.
                    Landrum.

  - Returns
                    - void

  - Author          Jan C. Depner

*********************************************************************************************/

static void chrtr2_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT32 value) 
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
 
 
 
/*******************************************************************************************/
/*!

  - Function        bit_unpack - Unpacks a long value from consecutive bits in buffer.

  - Synopsis        bit_unpack (buffer, start, numbits);
                    - NV_U_BYTE buffer[]      address of buffer to use
                    - NV_U_INT32 start        start bit position in buffer
                    - NV_U_INT32 numbits      number of bits to retrieve

  - Description     Unpacks the value from 'numbits' bits in 'buffer' starting at bit position
                    'start'.  The value is assumed to be unsigned.  The majority of this code
                    is based on Appendix C of Naval Ocean Research and Development Activity
                    Report #236, 'Data Base Structure to Support the Production of the
                    Digital Bathymetric Data Base', Nov. 1989, James E. Braud, John L.
                    Breckenridge, James E. Current, Jerry L. Landrum.

  - Returns
                    - value retrieved from buffer

  - Caveats         Note that the value that is output from this function is an unsigned 32
                    bit integer.  Even though you may have passed in a signed 32 bit value to
                    bit_pack it will not be sign extended on the way out.  If you just
                    happen to store it in 32 bits you can just typecast it to a signed
                    32 bit number and, lo and behold, you have a nice, signed number.  
                    Otherwise, you have to do the sign extension yourself.

    Author          Jan C. Depner

*********************************************************************************************/
 
static NV_U_INT32 chrtr2_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits) 
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



/********************************************************************************************/
/*!

    Module Name:        chrtr2_ngets

    Programmer(s):      Jan C. Depner

    Date Written:       May 1999

    Purpose:            This is an implementation of fgets that strips the line feed (or
                        carriage return/line feed) off of the end of the string if present.

    Arguments:          See fgets

    Return Value:       See fgets

*********************************************************************************************/

static NV_CHAR *chrtr2_ngets (NV_CHAR *s, NV_INT32 size, FILE *stream)
{
  if (fgets (s, size, stream) == NULL) return (NULL);

  while (strlen(s) > 0 && (s[strlen(s) - 1] == '\n' || s[strlen(s) - 1] == '\r')) s[strlen(s) - 1] = '\0';

  if (s[strlen (s) - 1] == '\n') s[strlen (s) - 1] = 0;


  return (s);
}
