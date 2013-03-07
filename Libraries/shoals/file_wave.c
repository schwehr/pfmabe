/*****************************************************************************
 * file_wave.c
 *
 * This file contains all routines required for accessing the waveform
 * files. 
 *
 * Contains:
 *==========
 * file_open_wave()	-Open the waveform file
 * file_read_wave()	-read a record from the waveform file
 * file_write_wave()	-write a record to the waveform file
 *
 *****************************************************************************/
#include <stdio.h>
#include <math.h>

#include "file_globals.h"
#include "waveforms.h"


static short        count[4] = {PMT_LEN, APD_LEN, IR_LEN, RAMAN_LEN};
#define NINT(a)     ((a)<0.0 ? (int) ((a) - 0.5) : (int) ((a) + 0.5))

/***************************************************************************\ 
*                                                                           * 
*   Function        bit_pack - Packs a long value into consecutive bits in  * 
*                   buffer.                                                 * 
*                                                                           * 
*   Synopsis        bit_pack (buffer, start, numbits, value);               * 
*                                                                           * 
*                   unsigned char buffer[]  address of buffer to use        * 
*                   unsigned long start     start bit position in buffer    * 
*                   unsigned long numbits   number of bits to store         * 
*                   long value              value to store                  * 
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
 
 
static void bit_pack (unsigned char buffer[], unsigned long start, 
unsigned long numbits, long value) 
{ 
    static unsigned char    mask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 
                            0xfe}, notmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 
                            0x07, 0x03, 0x01}; 
    long                    start_byte, end_byte, start_bit, end_bit, i; 
 
 
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
*                   unsigned char buffer[]  address of buffer to use        * 
*                   unsigned long start     start bit position in buffer    * 
*                   unsigned long numbits   number of bits to retrieve      * 
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
*   Returns         unsigned long           value retrieved from buffer     * 
*                                                                           * 
*   Author          Jan C. Depner                                           * 
*                                                                           * 
\***************************************************************************/ 
 
 
static unsigned long bit_unpack (unsigned char buffer[], unsigned long start, 
unsigned long numbits) 
{ 
    static unsigned char    mask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 
                            0xfe}, notmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 
                            0x07, 0x03, 0x01}; 
    long                    start_byte, end_byte, start_bit, end_bit, i; 
    unsigned long           value; 
 
 
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
         
        value = (unsigned long) buffer[start_byte] & (notmask[start_bit] &
            mask[end_bit]); 
 
        /*  Now we shift the value to the right.                            */ 
         
        value >>= (8 - end_bit); 
    } 
 
    /*  If the value covers more than 1 byte, retrieve it.                  */ 
     
    else 
    { 
        /*  Here we mask out data prior to the start bit of the first byte  */ 
        /*  and shift to the left the necessary amount.                     */ 
         
        value = (unsigned long) (buffer[start_byte++] & notmask[start_bit]) <<
            (numbits - (8 - start_bit)); 
 
        /*  Loop while decrementing the byte counter.                       */ 
         
        while (i--) 
        { 
       	    /*  Get the next 8 bits from the buffer.                        */ 
       	     
       	    value += (unsigned long) buffer[start_byte++] << ((i << 3) + end_bit); 
       	} 
 
       	/*  For the last byte we mask out anything after the end bit and    */ 
       	/*  then shift to the right (8 - end_bit) bits.                       */ 
       	 
        value += (unsigned long) (buffer[start_byte] & mask[end_bit]) >> (8 - end_bit); 
    } 
 
    return (value); 
} 



/***************************************************************************\
*                                                                           *
*   Module Name:        get_pack_data                                       *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Purpose:            Given the WAVEFORMS_T structure, which contains all *
*                       of the waveform data, this function returns all of  *
*                       the required packing field sizes and builds the     *
*                       arrays of delta values.                             *
*                                                                           *
*   Arguments:          wave       -  waveform data                         *
*                       pack_data  -  pointer to the pack_data structure    *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
\***************************************************************************/

static void get_pack_data (WAVEFORMS_T wave, PACK_DATA *pack_data)
{
    short               i, j, delta, min_delta, max_value, previous, *array[4],
                        *o_array[4];
    static double       log2 = 0.3010299957;


    array[0] = wave.pmt_raw_waveform_ra;
    array[1] = wave.apd_raw_waveform_ra;
    array[2] = wave.ir_raw_waveform_ra;
    array[3] = wave.raman_raw_waveform_ra;
    o_array[0] = pack_data->pmt;
    o_array[1] = pack_data->apd;
    o_array[2] = pack_data->ir;
    o_array[3] = pack_data->raman;


    /*  Compute the minimum difference between points for each wave array.  */

    for (j = 0 ; j < 4 ; j++)
    {
        previous = -1;
        min_delta = 32767;
        pack_data->start[j] = 0;

        for (i = 1 ; i < count[j] + 1 ; i++)
        {
            /*  Check for negative numbers in waveform.  */

            if (array[j][i] < 0)
            {
                fprintf (stderr, 
                    "\n\nNegative number in waveform, file is corrupted!\n");
                fprintf (stderr, "Terminated!\n\n");
                exit (-1);
            }


            if (array[j][i])
            {
                /*  Save the first non-zero point as the reference point.  */

                if (previous == -1)
                { 
                    pack_data->reference[j] = array[j][i];
                    previous = array[j][i];
                    pack_data->start[j] = i;
                }
                else
                {
                    delta = array[j][i] - previous;
                    min_delta = MIN (delta, min_delta);
                    previous = array[j][i];
                }
            }
        }

        min_delta -= 1;
        if (min_delta >= 0) min_delta = -1;


        /*  If the array is all 0's...  */

        if (previous == -1) pack_data->reference[j] = 0;


        /*  The min difference is the offset. */

        pack_data->offset[j] = -min_delta;


        /*  Compute the number of bits needed to store the reference, start,
            and the offset.  */

        pack_data->ref_bits[j] = 
            NINT (log10 ((double) (pack_data->reference[j] + 1)) / log2 + 0.5);
        pack_data->start_bits[j] = 
            NINT (log10 ((double) (pack_data->start[j] + 1)) / log2 + 0.5);
        pack_data->offset_bits[j] = 
            NINT (log10 ((double) (pack_data->offset[j] + 1)) / log2 + 0.5);


        /*  Compute the maximum value to be stored while computing the 
            values.  */

        previous = -1;
        max_value = 0;
        for (i = 1 ; i < count[j] + 1 ; i++)
        {
            if (array[j][i])
            {
                if (previous == -1)
                {
                    previous = array[j][i];
                }
                else
                {
                    o_array[j][i - 1] = (array[j][i] - previous) + 
                        pack_data->offset[j];
                    max_value = MAX (o_array[j][i - 1], max_value);
                    previous = array[j][i];
                }
            }
            else
            {
                o_array[j][i - 1] = 0;
            }
        }


        /*  Compute the number of bits needed to store values based on the 
            maximum value.  */

        pack_data->delta_bits[j] = NINT (log10 ((double) (max_value + 1)) / 
            log2 + 0.5);
        if (!max_value)
        {
            pack_data->offset_bits[j] = 1;
            pack_data->delta_bits[j] = 1;
        }
    }
}



/***************************************************************************\
*                                                                           *
*   Module Name:        pack_wave                                           *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Purpose:            Given the WAVEFORMS_T structure, which contains all *
*                       of the waveform data, this function returns the     *
*                       unsigned character array to be written to the       *
*                       compressed wave file.                               *
*                                                                           *
*   Arguments:          wave       -  pack_data structure                   *
*                       data       -  compressed unsigned character data    *
*                                                                           *
*   Return Value:       Number of bytes in the compressed unsigned          *
*                       character data array to be written                  *
*                                                                           *
\***************************************************************************/

int pack_wave (WAVEFORMS_T wave, unsigned char *data)
{
    int             position, i, j;
    short           *array[4];
    PACK_DATA       pack_data;


    /*  Get the delta arrays and reference values.  */

    get_pack_data (wave, &pack_data);


    array[0] = pack_data.pmt;
    array[1] = pack_data.apd;
    array[2] = pack_data.ir;
    array[3] = pack_data.raman;


    position = 16;

    bit_pack (data, position, 32, wave.time_stamp);
    position += 32;


    for (j = 0 ; j < 4 ; j++)
    {
        bit_pack (data, position, 4, pack_data.ref_bits[j]);
        position += 4;

        bit_pack (data, position, 4, pack_data.start_bits[j]);
        position += 4;

        bit_pack (data, position, 4, pack_data.offset_bits[j]);
        position += 4;

        bit_pack (data, position, 4, pack_data.delta_bits[j]);
        position += 4;

        bit_pack (data, position, pack_data.ref_bits[j], 
            pack_data.reference[j]);
        position += pack_data.ref_bits[j];

        bit_pack (data, position, pack_data.start_bits[j], 
            pack_data.start[j]);
        position += pack_data.start_bits[j];

        bit_pack (data, position, pack_data.offset_bits[j], 
            pack_data.offset[j]);
        position += pack_data.offset_bits[j];

        for (i = 0 ; i < count[j] ; i++)
        {
            bit_pack (data, position, pack_data.delta_bits[j], array[j][i]);
            position += pack_data.delta_bits[j];
        }
    }

    i = position / 8;


    /*  Set the size to the next larger whole byte if it wasn't an even 
        multiple of 8.   */
    
    if (position % 8) i++;


    bit_pack (data, 0, 16, i);

    return (i);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        unpack_wave                                         *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Purpose:            Given the compressed unsigned character array of    *
*                       wave data, this function unpacks it into the delta  *
*                       arrays in the PACK_DATA structure and from there    *
*                       into the waveforms.                                 *
*                                                                           *
*   Arguments:          pack_data  -  pointer to the pack_data structure    *
*                       data       -  compressed unsigned character data    *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
\***************************************************************************/

void unpack_wave (WAVEFORMS_T *wave, unsigned char *data)
{
    int             position, i, j, value;
    short           *array[4], *o_array[4], previous;
    PACK_DATA       pack_data;


    array[0] = wave->pmt_raw_waveform_ra;
    array[1] = wave->apd_raw_waveform_ra;
    array[2] = wave->ir_raw_waveform_ra;
    array[3] = wave->raman_raw_waveform_ra;
    o_array[0] = pack_data.pmt;
    o_array[1] = pack_data.apd;
    o_array[2] = pack_data.ir;
    o_array[3] = pack_data.raman;


    /*  Skip the size of the compressed waveform record (16 bits).  */

    position = 16;

    wave->time_stamp = bit_unpack (data, position, 32);
    position += 32;


    for (j = 0 ; j < 4 ; j++)
    {
        pack_data.ref_bits[j] = bit_unpack (data, position, 4);
        position += 4;

        pack_data.start_bits[j] = bit_unpack (data, position, 4);
        position += 4;

        pack_data.offset_bits[j] = bit_unpack (data, position, 4);
        position += 4;

        pack_data.delta_bits[j] = bit_unpack (data, position, 4);
        position += 4;

        pack_data.reference[j] = bit_unpack (data, position, 
            pack_data.ref_bits[j]);
        position += pack_data.ref_bits[j];

        pack_data.start[j] = bit_unpack (data, position, 
            pack_data.start_bits[j]);
        position += pack_data.start_bits[j];

        pack_data.offset[j] = bit_unpack (data, position, 
            pack_data.offset_bits[j]);
        position += pack_data.offset_bits[j];


        /*  The first value of the waveforms is always 0.  */

        array[j][0] = 0;
        previous = -1;

        for (i = 1 ; i < count[j] + 1 ; i++)
        {
            value = bit_unpack (data, position, pack_data.delta_bits[j]);
            if (i >= pack_data.start[j] && (value > 0 || previous == -1))
            {
                if (previous == -1)
                {
                    previous = pack_data.reference[j];
                    array[j][i] = previous;
                }
                else
                {
                    value -= pack_data.offset[j];
                    array[j][i] = previous + value;
                    if (array[j][i] < 0) array[j][i] = 0;
                    previous = array[j][i];
                }
            }
            else
            {
                array[j][i] = 0;
            }
            position += pack_data.delta_bits[j];
        }
    }
}







/*****************************************************************************
 * file_open_wavc
 *
 * Purpose:  	To open the waveform file in rb+ mode.  Either a compressed
 *              (wavc) file or an uncompressed (wave) file will be opened.
 *
 * Inputs:	1)   Database name
 *		2)   flightline
 *              3)   returned flag for a compressed file (1 - compressed)
 *              4)   returned number of records in file
 *
 *****************************************************************************/

FILE *file_open_wavc (char *db_name, int flightline, int *compression_flag,
int *numrecs)
{
	FILE            *wave_file;
	char            filename[100];
        unsigned char   recs[4];


	wave_file = NULL;
	*compression_flag = 1;


	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.wavc", file_get_GB_DATA_DIR(), db_name,
		 flightline);
#else
	sprintf(filename, "%s\\%s_%d.wavc", file_get_GB_DATA_DIR(), db_name,
		 flightline);
#endif

	if( (wave_file = fopen(filename,"rb+"))==NULL)
	{
            filename[strlen (filename) - 1] = 'e';

            if( (wave_file = fopen(filename,"rb+"))==NULL) return (wave_file); 

            *compression_flag = 0;
	}


        /*  Get the number of records in the file.  */

        if (!(*compression_flag))
        {
            /*  Compute the number of records for an uncompressed file.  */

            fseek (wave_file, 0, SEEK_END);
            *numrecs = ftell (wave_file) / sizeof (WAVEFORMS_T);
            fseek (wave_file, 0, SEEK_SET);
        }
        else
        {
            /*  Read the number of records from the beginning of the compressed
                wave file.  */

            fread (recs, 4, 1, wave_file);
            *numrecs = bit_unpack (recs, 0, 32);
        }

	return (wave_file);
}


/*****************************************************************************
 * file_read_wavc
 *
 * Purpose:  	Reads a record from the open compressed file
 *
 * Description:	
 *		-read in record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold waveform data..
 *		3)   record number
 *
 *****************************************************************************/
int file_read_wavc (FILE *wave_file, WAVEFORMS_T *wave_p, int record)
{
    unsigned char   data[(PMT_LEN + 1) * 2 + (APD_LEN + 1) * 2 +
                    (IR_LEN + 1) * 2 + (RAMAN_LEN + 1) * 2], ndxc[4];
    unsigned long   address;
    int             size;


    address = record * sizeof (int) + sizeof (int);
    fseek (wave_file, address, SEEK_SET);
    fread (ndxc, 4, 1, wave_file);
    address = bit_unpack (ndxc, 0, 32); 


    fseek (wave_file, address, SEEK_SET);
    fread (data, 2, 1, wave_file);

    size = bit_unpack (data, 0, 16);
    fread (&data[2], size - 2, 1, wave_file);


    unpack_wave (wave_p, data);

    
    return(FILE_OK);
}












/*****************************************************************************
 * file_open_wave
 *
 * Purpose:  	To open the waveform file...[will be opened in "w+" mode
 *		which will allow both reading and writing]  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *		2)   flightline
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote routine
 *****************************************************************************/

FILE *file_open_wave( char *db_name, int	flightline )
{
	FILE	*wave_file;
	char	filename[100];

	wave_file = NULL;
	

	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.wave", file_get_GB_DATA_DIR(), db_name,
		 flightline);
#else
	sprintf(filename, "%s\\%s_%d.wave", file_get_GB_DATA_DIR(), db_name,
		 flightline);
#endif

	if( (wave_file = fopen(filename,"rb+"))==NULL)
	{
		 return(wave_file); 
	}

	return(wave_file);
}


/*****************************************************************************
 * file_read_wave CSU
 *
 * Purpose:  	To read a record from the open waveform file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold waveform data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote routine
 *****************************************************************************/
int file_read_wave( FILE *wave_file, WAVEFORMS_T *wave_p )
{

	/* Get a record */
 	if(fread(wave_p, sizeof(char),sizeof(WAVEFORMS_T), 
		   wave_file) == 0)
	{		
		if(feof(wave_file))
		{
			return(FILE_EOF);
		}
		else
		{
			return(FILE_READ_ERR);
		}
	}
	return(FILE_OK);
}

/*****************************************************************************
 * file_write_wave CSU
 *
 * Purpose:  	To write a record into waveform file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold waveform data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/

int file_write_wave( FILE *wave_file, WAVEFORMS_T *wave_p)
{
	int	ret_val;
	if((ret_val = fwrite(wave_p, sizeof(char), sizeof(WAVEFORMS_T), 
	    wave_file)) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}
/*****************************************************************************
 * file_get_wave CSU
 *
 * Purpose:  	To get the requested record from waveform file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the wave data to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold waveform data...
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/10/28 DR	-wrote routine
 *
 *****************************************************************************/

int file_get_wave( char *db_name, short	flightline, int timestamp, WAVEFORMS_T *wave_p)
{
	int	ret_val = FILE_OK;
	int	FILECODE;
	FILE 	*wave_file;

	/*
	*******************************
	*   OPEN THE WAVEFORM file 
	*******************************
	*/

	if(( wave_file = file_open_wave(db_name, flightline))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}


	/* Go to the approximate location of the record in the file */
	file_guess_record_location(wave_file, db_name, flightline,
		timestamp, sizeof(WAVEFORMS_T));

	for(;;)
	{
		/* Get a record */
	 	FILECODE = file_read_wave(wave_file, wave_p);
		if(FILECODE < 0)
		{
			ret_val = FILECODE;
			if(FILECODE != FILE_EOF)
			{
				printf("/tRead error\n");
			}
			break;
		}

		/* CHECK IF IT IS THE ONE */
		if(wave_p->time_stamp  == timestamp) 
		{
			/* FOUND IT!! */
			ret_val = 1;
			break;
		}
	}

	fclose(wave_file);

	return(ret_val);
}
