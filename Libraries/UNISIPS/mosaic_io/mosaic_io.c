/****************************************************************
 *								*
 *	File Name    :	mosaic_io.c				*
 *	Programmer(s):  Chris Robinson				*
 *	Purpose	     :	Contains low level routines to read     *
 *			MOSAIC formatted data.			*
 *	Date	     :	June 1995				*
 *								*
 ****************************************************************/
#include <stdio.h>
#include <string.h>

#include "mosaic_io.h"


/****************************************************************
 ****************************************************************
 ************************** INPUT ROUTINES **********************
 ****************************************************************/ 


/****************************************************************
 *								*
 *	Module Name   : read_mosaic_header()			*
 *	Programmer(s) : Chris Robinson				*
 *	Purpose	      : To read the file header of a MOSAIC     *
 *			formatted file.				*
 *	Date	      : June 1995				*
 *								*
 ****************************************************************/

int read_mosaic_header(FILE **fp, MOSAIC_HEADER  *mosaic_header)
{
 /* Local Variables */

 /* Begin */

  /* Make sure that we are at the beginning of the file */

     fseek (*fp,0,SEEK_SET);

  /* Try reading file header record */
     if (fread(mosaic_header,sizeof(MOSAIC_HEADER),1,*fp) != 1)
     {
      return (0);  /* Failure */
     }


     return (1);   /* Success */

 }  /* end of read_mosaic_header() */


/****************************************************************
 *								*
 *	Module Name   :	read_mosaic_data()			*
 *	Programmer(s) : Chris Robinson				*
 *	Purpose	      : To read the data of a MOSAIC formatted  *
 *			file.  This routine will read all of    *
 *			the data in one read.			*
 *								*
 *	Date	      : June 1995				*
 *								*
 ****************************************************************/

int read_mosaic_data(FILE **fp, unsigned char *data, long int data_size)
{
 /* Local Variables */

    long int   bytes_read;

 /* Begin */

    bytes_read = fread (data,sizeof(unsigned char),data_size,*fp);

    if (bytes_read != data_size)
    {
	printf(" ERROR IN READING MOSAIC DATA\n   Bytes Read = %d \n",bytes_read);
	if (feof(*fp)) printf(" End of File encountered\n");
	return (0);  /*Failure*/
    }

  return (1); /*Success*/

 } /* end of read_mosaic_data() */

/****************************************************************
 *								*
 *	Module Name:	read_mosaic_record()			*
 *	Author	   :	Chris Robinson				*
 *	Purpose	   :	To read one record from a MOSAIC 	*
 *			formatted file.  It reads at the current*
 *			file pointer location.			*
 *	Date	   :    June 1995				*
 *								*
 ****************************************************************/

int read_mosaic_record (FILE **fp,
			MOSAIC_HEADER   mosaic_header,
			unsigned char *data)
{
  /* Local Variables */

     long int  bytes_read;

  /* Begin */


     bytes_read = fread (data,sizeof(unsigned char),
			 mosaic_header.columns,*fp);

     if (bytes_read != mosaic_header.columns)
     {
	printf(" ERROR IN READING MOSAIC RECORD \n    Bytes Read = %d \n",bytes_read);
	if (feof(*fp)) printf(" End of File encountered \n");
	return(0); /*Failure*/
     }

 
   return (1);  /* Success */

 
}     /* End of read_mosaic_record() routine */
 
/****************************************************************
 ****************************************************************
 ************************* OUTPUT ROUTINES **********************
 ****************************************************************/ 


/****************************************************************
 *								*
 *	Module Name   : write_mosaic_header()			*
 *	Programmer(s) : Chris Robinson				*
 *	Purpose	      : To write the file header to a MOSAIC    *
 *			formatted file.				*
 *	Date	      : June 1995				*
 *								*
 ****************************************************************/

int write_mosaic_header(FILE **fp, MOSAIC_HEADER  *mosaic_header)
{
 /* Local Variables */

 /* Begin */

  /* Make sure that we are at the beginning of the file */

     fseek (*fp,0,SEEK_SET);

  /* Try writing file header record */
     if (fwrite(mosaic_header,sizeof(MOSAIC_HEADER),1,*fp) != 1)
     {
      return (0);  /* Failure */
     }


     return (1);   /* Success */

 }  /* end of write_mosaic_header() */


/****************************************************************
 *								*
 *	Module Name   :	write_mosaic_data()			*
 *	Programmer(s) : Chris Robinson				*
 *	Purpose	      : To write the data to a MOSAIC formatted *
 *			file.  This routine will write all of    *
 *			the data in one write.			*
 *								*
 *	Date	      : June 1995				*
 *								*
 ****************************************************************/

int write_mosaic_data(FILE **fp, unsigned char *data, long int data_size)
{
 /* Local Variables */

    long int   bytes_write;

 /* Begin */

    bytes_write = fwrite (data,sizeof(unsigned char),data_size,*fp);

    if (bytes_write != data_size)
    {
	printf(" ERROR IN WRITING MOSAIC DATA\n   Bytes Read = %d \n",bytes_write);
	if (feof(*fp)) printf(" End of File encountered\n");
	return (0);  /*Failure*/
    }

  return (1); /*Success*/

 } /* end of write_mosaic_data() */

/****************************************************************
 *								*
 *	Module Name:	write_mosaic_record()			*
 *	Author	   :	Chris Robinson				*
 *	Purpose	   :	To write one record to a MOSAIC 	*
 *			formatted file. It writes at the current*
 *			file pointer location.			*
 *	Date	   :    June 1995				*
 *								*
 ****************************************************************/

int write_mosaic_record (FILE **fp,
			 MOSAIC_HEADER mosaic_header,
			 unsigned char *data)
{
  /* Local Variables */

     long int  bytes_write;

  /* Begin */


     bytes_write = fwrite (data,sizeof(unsigned char),
			   mosaic_header.columns,*fp);

     if (bytes_write != mosaic_header.columns)
     {
	printf(" ERROR IN WRITING MOSAIC RECORD \n    Bytes Read = %d \n",bytes_write);
	if (feof(*fp)) printf(" End of File encountered \n");
	return(0); /*Failure*/
     }

 
   return (1);  /* Success */

 
}     /* End of write_mosaic_record() routine */
