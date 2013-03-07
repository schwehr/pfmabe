/*****************************************************************************
*
*   File Name : unisips_io.c
*   Author    : Chris Robinson
*   Purpose   : Contains low-level routines to read/write
*               UNISIPS formatted data.
*   Date      : Dec 1999
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include <unisips_io.h>
#include <bytes2func.h>


/*****************************************************************************
******************************************************************************
******************* THESE ARE THE BYTE SWAPPING ROUTINES *********************
******************************************************************************
*****************************************************************************/

/**********************************************************************************
*
*   Module Name   : big_endian()
*   Author        : Chris Robinson 
*   Purpose       : Routine to determine if computer is a big endian computer
*   Date          : Dec 1999
*
**********************************************************************************/
static int big_endian()
{

   union
   {
     unsigned char bytes[4];
     long int      word;
   }number;


    number.word =  0xff000000;

    if (number.bytes[0])
     return(1);
    else
     return(0);


}/*big_endian*/

/*****************************************************************************
*
*   Module Name : swap_header()
*   Author      : Chris Robinson
*   Purpose     : This routine performs byte swapping of the header file.
*   Date        : Nov 1998
*
*   Modified By : Greg Ip
*                 Lockheed Martin Stennis Operations
*                 August 1999
*            
*                 Modifications:
*                 Added new variables for byteswapping to reflect
*                 the new UNISIPS format.
*
*****************************************************************************/

int swap_header(UNISIPS_HEADER *uhdr)
{
    int i;   

    
    uhdr->rows       = bytes2int((unsigned char *)&(uhdr->rows), 1);
    uhdr->cols       = bytes2int((unsigned char *)&(uhdr->cols), 1);
    uhdr->pixel_res  = bytes2float((unsigned char *)&(uhdr->pixel_res), 1);
    uhdr->max_lat    = bytes2double((unsigned char *)&(uhdr->max_lat), 1);
    uhdr->min_lat    = bytes2double((unsigned char *)&(uhdr->min_lat), 1);
    uhdr->max_lon    = bytes2double((unsigned char *)&(uhdr->max_lon), 1);
    uhdr->min_lon    = bytes2double((unsigned char *)&(uhdr->min_lon), 1);
    uhdr->version    = bytes2float((unsigned char *)&(uhdr->version), 1);
    uhdr->sub_header_flag =
                     bytes2short((unsigned char *)&(uhdr->sub_header_flag),1);
    uhdr->bathy_units= bytes2short((unsigned char *)&(uhdr->bathy_units), 1);
    uhdr->total_channels = 
		     bytes2short((unsigned char *)&(uhdr->total_channels),1);
    uhdr->datatype = 
	             bytes2short((unsigned char *)&(uhdr->datatype),1);
    for(i = 0; i < MAX_CHANNELS; i++)
    {
        uhdr->channel_type[i]      = 
                     bytes2short((unsigned char *)&(uhdr->channel_type[i]), 1);
        uhdr->channel_width[i]     = 
                     bytes2short((unsigned char *)&(uhdr->channel_width[i]), 1);
        uhdr->channel_frequency[i] = 
                     bytes2float((unsigned char *)&(uhdr->channel_frequency[i]), 1);
        uhdr->channel_resolution[i] =
                    bytes2float((unsigned char *)&(uhdr->channel_resolution[i]), 1);
			
    }    
    uhdr->pulse_length = 
                     bytes2float((unsigned char *)&(uhdr->pulse_length), 1);
    uhdr->frequency  = bytes2float((unsigned char *)&(uhdr->frequency), 1);
    uhdr->port_frequency = 
                     bytes2float((unsigned char *)&(uhdr->port_frequency), 1);
    uhdr->starboard_frequency = 
                     bytes2float((unsigned char *)&(uhdr->starboard_frequency), 1);
    uhdr->band_width = bytes2float((unsigned char *)&(uhdr->band_width), 1);
    uhdr->processing_history[0] = 
                     bytes2int((unsigned char *)&(uhdr->processing_history[0]), 1);
    uhdr->processing_history[1] = 
                     bytes2int((unsigned char *)&(uhdr->processing_history[1]), 1);
    uhdr->bits_per_pixel = 
                     bytes2short((unsigned char *)&(uhdr->bits_per_pixel), 1);
    uhdr->num_boundary_pts = 
                     bytes2short((unsigned char *)&(uhdr->num_boundary_pts), 1);
    uhdr->boundary_pts_loc = 
                     bytes2int((unsigned char *)&(uhdr->boundary_pts_loc), 1);

    return (1);
}

/****************************************************************************
*
*   Module Name : swap_unisips_rec_hdr()
*   Author      : Chris Robinson
*   Purpose     : This routine performs byte swapping of the record header.
*   Date        : Nov 1999
*
****************************************************************************/

int swap_unisips_rec_hdr(UNISIPS_REC_HDR *urh)
{
    urh->year      = bytes2int((unsigned char *)&(urh->year), 1);
    urh->tide_v    = bytes2short((unsigned char *)&(urh->tide_v), 1);
    urh->layback   = bytes2short((unsigned char *)&(urh->layback), 1);
    urh->time      = bytes2double((unsigned char *)&(urh->time), 1);
    urh->roll      = bytes2float((unsigned char *)&(urh->roll), 1);
    urh->pitch     = bytes2float((unsigned char *)&(urh->pitch), 1);
    urh->heading   = bytes2float((unsigned char *)&(urh->heading), 1);
    urh->fish_spd  = bytes2float((unsigned char *)&(urh->fish_spd), 1);
    urh->lat       = bytes2double((unsigned char *)&(urh->lat), 1);
    urh->lon       = bytes2double((unsigned char *)&(urh->lon), 1);
    urh->cb_depth  = bytes2float((unsigned char *)&(urh->cb_depth), 1);
    urh->tow_depth = bytes2float((unsigned char *)&(urh->tow_depth), 1);

    return (1);
}
/****************************************************************************
*
*   Module Name : swap_unisips_rec_hdr()
*   Author      : Chris Robinson
*   Purpose     : This routine performs byte swapping of the record subheader.
*   Date        : Dec 1999
*
****************************************************************************/

int swap_unisips_rec_subhdr(UNISIPS_REC_SUBHDR *ursh)
{
   ursh->Pgain		= bytes2float((unsigned char *)&(ursh->Pgain),1);
   ursh->Sgain		= bytes2float((unsigned char *)&(ursh->Sgain),1);
   ursh->Pulse_length	= bytes2float((unsigned char *)&(ursh->Pulse_length),1);
   ursh->Bandwidth	= bytes2float((unsigned char *)&(ursh->Bandwidth),1);
   ursh->Ping_period	= bytes2float((unsigned char *)&(ursh->Ping_period),1);
   ursh->Temperature	= bytes2float((unsigned char *)&(ursh->Temperature),1);
   ursh->Sound_vel	= bytes2float((unsigned char *)&(ursh->Sound_vel),1);
   ursh->Ship_lat	= bytes2double((unsigned char *)&(ursh->Ship_lat),1);
   ursh->Ship_lon	= bytes2double((unsigned char *)&(ursh->Ship_lon),1);
   ursh->Prange 	= bytes2float((unsigned char *)&(ursh->Prange),1);
   ursh->Srange 	= bytes2float((unsigned char *)&(ursh->Srange),1);
   ursh->Samp_freq	= bytes2float((unsigned char *)&(ursh->Samp_freq),1);
    return (1);
}


/*****************************************************************************
 *****************************************************************************
 *********************** INPUT ROUTINES **************************************
 *****************************************************************************
 ****************************************************************************/ 


/*****************************************************************************
*
*   Module Name  : read_unisips_header() 
*   Author       : Chris Robinson
*   Purpose      : To read the file header of a UNISIPS formatted file.
*   Date         : NOV 98
*
*   Modified By  : Greg Ip
*                  Lockheed Martin Stennis Operations
*                  August 1999
*
*                  Modifications:
*                  Added fseek to seek to the beginning of the file.
*		
*		   December 1999
*		   Added check to see if byteswapping was needed
*		   Added 'swap' to the function parameter list.
*
*****************************************************************************/

int read_unisips_header(FILE *fp, UNISIPS_HEADER *uheader, int *swap)
{
 /* Local Variables */


    int big_endian();

 /* Begin */

 /**********************************
  * Seek to the begin of the file  *
  **********************************/

  fseek(fp,0,SEEK_SET);

 /**********************************
  * Try reading file header record *
  **********************************/

  if (fread(uheader,sizeof(UNISIPS_HEADER),1,fp) != 1)
  {
   return (0);  /* Failure */
  }

 /*****************************************
  * Determine if bytes need to be swapped *
  *****************************************/

  switch (uheader->endian)
  {
    case (0):   /**************************
		 *Old unisips format files*
		 *without endian flag,    *
		 *check the old way.      *
		 *All old unisips files   *
		 *were created on a BIG   *
		 *ENDIAN computer         *
		 **************************/

		 *swap = (!big_endian());

		 break;

    case (0x00010203):

		/*******************************
		 * New format - No swap needed *
		 *******************************/

		*swap = 0;

		break;

    case (0x3020100):

		/******************************
		 * New format - Swap needed   *
		 ******************************/

		*swap = 1;

		break;
		 
   }/*switch*/


 /***********************
  * byte swap if needed *
  ***********************/
  
  if (*swap)
  {
       swap_header(uheader);
  }
 
 /* Determine the number of bits per pixel to use */
     if (uheader->bits_per_pixel != 8  &&
	 uheader->bits_per_pixel != 16 &&
	 uheader->bits_per_pixel != 32 &&
	 uheader->bits_per_pixel != 64)
      uheader->bits_per_pixel = 8;


  /* If old format file,set the uheader->channel_width[0] to uheader->cols*/

     if (uheader->total_channels == 0)
     {
      uheader->total_channels = 1;
      uheader->channel_width[0] = uheader->cols;
      uheader->channel_resolution[0]=uheader->pixel_res;
     }
 

 return (1);    /* Success */

} /* end of read_unisips_header() */


/*****************************************************************************
*
*   Module Name  : read_unisips_rec_hdr()
*   Author       : Chris Robinson
*   Purpose      : To read one record header from a UNISIPS formatted file.
*                  The position of the record is calculated.
*   Date         : June 1995
*
*   Modified By  : December 1999 to reflect multichannel data changes  
*		    -Added 'recnum' and 'swap' to parameters list	  
*		    -Added 'get_rec_pos' to calculate the file position
*		     of desired record.    
*		    -Added call to swap bytes if needed.
*		    -Added ability to read Record Sub Header , if there is one.
* 
*****************************************************************************/

int read_unisips_rec_hdr(FILE *fp, UNISIPS_HEADER *uhead, UNISIPS_REC_HDR *urechdr, 
                         UNISIPS_REC_SUBHDR *urec_shdr,int recnum, int swap)
{
    int  rval;
    long filepos;

    long get_rec_pos(UNISIPS_HEADER *,int);
 	
    /* Checks for valid record number */

    if ((recnum >= uhead->rows) || (recnum < 0))
    {    
         fprintf(stderr,"\nInvalid Record Number (%d)\n",recnum);
         return(0);  /*  Failure  */
    }
    else
    {    
         /* Checks for single or multi-channel file and 
            calculates the record header position    */

         filepos = get_rec_pos(uhead,recnum);
 
 
         /* Checks if file position is within the valid range */

            if ((uhead->num_boundary_pts > 0) && (filepos >= uhead->boundary_pts_loc))
            {
              fprintf(stderr,"ERROR, that position falls into the boundary points location\n");
              return(0);  /* Failure */
            }
   }
   
    /* Seeks and reads the record header position */    

    fseek(fp,filepos,SEEK_SET);

    if(rval = fread(urechdr, sizeof(UNISIPS_REC_HDR), 1, fp) != 1)
    {
      return(0);  /* Failure */
    }

    /* Read the record subheader,if there is one*/

    if (uhead->sub_header_flag)
    {
      /*The subheader imediately follows the record header,no need to position file pointer*/

        if (rval= fread(urec_shdr,sizeof(UNISIPS_REC_SUBHDR),1,fp)!=1)
         return(0); /*Failure*/

    }

    /* Perform byte swapping if needed */
    if (swap)
    {
        swap_unisips_rec_hdr(urechdr);

        if(uhead->sub_header_flag)
         swap_unisips_rec_subhdr(urec_shdr);
    }

    return (1);    /* Success */
}


/**********************************************************************************
*
*   Module Name   : read_unisips_record()
*   Author        : Chris Robinson    
*   Purpose       : To read one record (record header and data) from a 
*                   UNISIPS formatted file.
*   Date          : Nov 98
*
*   Modified      : December 1999 to reflect multichannel data changes
*		     -Added 'recnum' ,'channel'  parameters to indicate which
*		      record and channel to read and 'swap' parameter to 
*                     indicate if data needs to be byteswaped.
**********************************************************************************/

int read_unisips_record(FILE *inf, UNISIPS_HEADER *uHead, 
			UNISIPS_REC_HDR *urechdr,UNISIPS_REC_SUBHDR *urec_shdr,
		        int recnum,int channel,
			unsigned char *data, int swap)
{
    /*  Local Variables  */

    register int  ii;

    long rval;
    long position;
    long read_len;
    int  bytes;

     short          *sptr =NULL;
     unsigned short *usptr=NULL;
     int 	    *iptr =NULL;
     unsigned int   *uiptr=NULL;
     float	    *fptr =NULL;
     double	    *dptr =NULL; 


    long get_rec_pos    (UNISIPS_HEADER *,int);
    long get_channel_pos(UNISIPS_HEADER *,int,int,int);

    /* Checks for valid record number */

    if ((recnum >= uHead->rows) || (recnum < 0))
    {    
         fprintf(stderr,"\nInvalid Record Number  (%d)\n",recnum);
         return(0);  /*  Failure  */
    }

    /*************************************************
     * Position the file pointer to point to the     *
     * begininng of each record depending on single  *
     * single or multi-channel                       *
     *************************************************/

    position = get_rec_pos(uHead,recnum);

    fseek(inf,position,SEEK_SET);
  
    /*  Read UNISIPS record header */

    if (rval= fread(urechdr,sizeof(UNISIPS_REC_HDR),1,inf) != 1)
    {
	fprintf(stderr,"\n(read_unisips_record) Failure to read record"
		       " header of record %d channel %d  (rval=%d)\n\n",
			recnum,channel,rval);
        return(0);   /* Failure to read record header  */
    }

    /* Read the record subheader,if there is one*/

    if (uHead->sub_header_flag)
    {

      /*The subheader imediately follows the record
        header,no need to position file pointer*/

        if (rval= fread(urec_shdr,sizeof(UNISIPS_REC_SUBHDR),1,inf)!=1)
        {
	 fprintf(stderr,"\n(read_unisips_record)Failure to read record %d subheader\n\n",
		recnum);
         return(0); /*Failure*/
	}

    }
    /*  Byte swap if needed  */
    
    if (swap)
    {
        swap_unisips_rec_hdr(urechdr);

        if(uHead->sub_header_flag)
         swap_unisips_rec_subhdr(urec_shdr);
    }

    /*******************************************
     *  Read data from channel that was passed *
     *******************************************/


    position = get_channel_pos(uHead,recnum,channel,SEEK_CUR);

    fseek(inf,position,SEEK_CUR);

    bytes     = (uHead->bits_per_pixel/BitsPerByte);
    read_len  = uHead->channel_width[channel] * bytes;

      
    if (rval = fread(data,sizeof(unsigned char), read_len,inf) != read_len)
    {

	fprintf(stderr,"\n(read_unisips_header)Read only %d bytes of %d requested on record %d!\n\n",
			rval,read_len,recnum);
        return(0);  /* Failure to read data  */
    }

  /*Byte swap if needed*/

    if (swap && (bytes > 1))
    {

 
    /***********************************************************
     * Here we set all of our data pointers to image->dataptr, *
     * although only one of the pointers will actually be used.*
     ***********************************************************/

      sptr   = (short          *) data;
      usptr  = (unsigned short *) data;
      iptr   = (int	    *)    data;
      uiptr  = (unsigned int   *) data;
      fptr   = (float          *) data;
      dptr   = (double         *) data;


      for (ii=0; ii<uHead->channel_width[channel]; ii++)
      {
        switch (uHead->datatype)
        {
           
 	  case (TYPE_SHORT):

	    sptr[ii] = bytes2short((unsigned char *)&(sptr[ii]),1);
	 
	   break;

          case (TYPE_UNSIGNED_SHORT):

	    usptr[ii] = (unsigned short)bytes2short((unsigned char *)&(sptr[ii]),1);

	   break;
	  case (TYPE_INTEGER):

	    iptr[ii]  = bytes2int((unsigned char *)&(iptr[ii]),1);

          break;
	  case (TYPE_UNSIGNED_INT):

	    uiptr[ii]  = (unsigned int)bytes2int((unsigned char *)&(iptr[ii]),1);

	   break;

	  case (TYPE_FLOAT):

            fptr[ii]  = bytes2float((unsigned char *)&(fptr[ii]),1);
	  
           break;

          case (TYPE_DOUBLE):

            dptr[ii] = bytes2double((unsigned char *)&(dptr[ii]),1);
  	  break;

        }/*switch*/

      }/*for*/


    }/*if swap*/

    return(1);  /*  Successful reading record header and data  */

}
/**********************************************************************************
*
*   Module Name   : read_unisips_rec_all()
*   Author        : Chris Robinson    
*                   Marianne Murphy - copied this routine from read_unisips_record.
*
*   Purpose       : To read one entire record (record header and multiple data 
*                   channels) from a UNISIPS formatted file.  This routine differs
*                   from read_unisips_record in that it copies all data channels.
*
*   Date          : Dec 2001
*
**********************************************************************************/

int read_unisips_rec_all(FILE *inf, UNISIPS_HEADER *uHead, 
		         UNISIPS_REC_HDR *urechdr,UNISIPS_REC_SUBHDR *urec_shdr,
		         int recnum, unsigned char *data, int swap)
{
    /*  Local Variables  */

    register int  ii, ic, iw;

    long rval;
    long position;
    long read_len;
    int  bytes;

     short          *sptr =NULL;
     unsigned short *usptr=NULL;
     int 	    *iptr =NULL;
     unsigned int   *uiptr=NULL;
     float	    *fptr =NULL;
     double	    *dptr =NULL; 


    long get_rec_pos    (UNISIPS_HEADER *,int);
    long get_channel_pos(UNISIPS_HEADER *,int,int,int);

    /* Checks for valid record number */

    if ((recnum >= uHead->rows) || (recnum < 0))
    {    
         fprintf(stderr,"\nInvalid Record Number\n");
         return(0);  /*  Failure  */
    }

    /*************************************************
     * Position the file pointer to point to the     *
     * begininng of each record depending on single  *
     * single or multi-channel                       *
     *************************************************/

    position = get_rec_pos(uHead,recnum);

    fseek(inf,position,SEEK_SET);
  
    /*  Read UNISIPS record header */

    if (rval= fread(urechdr,sizeof(UNISIPS_REC_HDR),1,inf) != 1)
    {
	fprintf(stderr,"\n(read_unisips_record) Failure to read record"
		       " header of record %d (rval=%d)\n\n",
			recnum,rval);
        return(0);   /* Failure to read record header  */
    }

    /* Read the record subheader,if there is one*/

    if (uHead->sub_header_flag)
    {

      /*The subheader imediately follows the record
        header,no need to position file pointer*/

        if (rval= fread(urec_shdr,sizeof(UNISIPS_REC_SUBHDR),1,inf)!=1)
        {
	 fprintf(stderr,"\n(read_unisips_record)Failure to read record %d subheader\n\n",
		recnum);
         return(0); /*Failure*/
	}

    }
    /*  Byte swap if needed  */
    
    if (swap)
    {
        swap_unisips_rec_hdr(urechdr);

        if(uHead->sub_header_flag)
         swap_unisips_rec_subhdr(urec_shdr);
    }


    read_len = 0;
    bytes     = (uHead->bits_per_pixel/BitsPerByte);
    for (ic=0; ic<uHead->total_channels; ic++)
    {
   
      read_len += uHead->channel_width[ic];
    
    }
    read_len = read_len * bytes;

      
    position = get_channel_pos(uHead,recnum,0,SEEK_CUR);
    fseek(inf,position,SEEK_CUR);

    if (rval = fread(data,sizeof(unsigned char), read_len,inf) != read_len)
    {

	fprintf(stderr,"\n(read_unisips_rec_all)Read only %d bytes of %d requested!\n\n",
			rval,read_len);
        return(0);  /* Failure to read data  */
    }

  /*Byte swap if needed*/

    if (swap && (bytes > 1))
    {

 
    /***********************************************************
     * Here we set all of our data pointers to image->dataptr, *
     * although only one of the pointers will actually be used.*
     ***********************************************************/

      sptr   = (short          *) data;
      usptr  = (unsigned short *) data;
      iptr   = (int	    *)    data;
      uiptr  = (unsigned int   *) data;
      fptr   = (float          *) data;
      dptr   = (double         *) data;

    ii = 0;

    for (ic=0; ic<uHead->total_channels; ic++)
    {
      for (iw=0; iw<uHead->channel_width[ic]; iw++)
      {
        switch (uHead->datatype)
        {
           
 	  case (TYPE_SHORT):

	    sptr[ii] = bytes2short((unsigned char *)&(sptr[ii]),1);
	 
	   break;

          case (TYPE_UNSIGNED_SHORT):

	    usptr[ii] = (unsigned short)bytes2short((unsigned char *)&(sptr[ii]),1);

	   break;
	  case (TYPE_INTEGER):

	    iptr[ii]  = bytes2int((unsigned char *)&(iptr[ii]),1);

          break;
	  case (TYPE_UNSIGNED_INT):

	    uiptr[ii]  = (unsigned int)bytes2int((unsigned char *)&(iptr[ii]),1);

	   break;

	  case (TYPE_FLOAT):

            fptr[ii]  = bytes2float((unsigned char *)&(fptr[ii]),1);
	  
           break;

          case (TYPE_DOUBLE):

            dptr[ii] = bytes2double((unsigned char *)&(dptr[ii]),1);
  	  break;

          ii++;
        }/*switch*/
      }/*for*/
    }


    }/*if swap*/

    return(1);  /*  Successful reading record header and data  */

}
    
/**********************************************************************************
*
*   Module Name   : read_boundary_pts()
*   Author        : Chuck Martin
*   Purpose       : To read all boundary points from a UNISIPS formatted file.
*   Date          :

*   MODIFIED      :  CSR  JAN 2000 now pass in a seperate float array for lat and lons
*
**********************************************************************************/

int read_boundary_pts(FILE *inf, UNISIPS_HEADER *uheader, float *pointLat, float *pointLon, int swap)
{
    short pts;
    int   curpos;
    char  infloat[4];

    if (uheader->boundary_pts_loc == 0)
    {
        return (0);
    }

    curpos = ftell(inf);
    fseek(inf, uheader->boundary_pts_loc, SEEK_SET);
    for (pts=0;pts<uheader->num_boundary_pts;pts++)
    {
        fread(infloat, 1, 4, inf);
/**        points[pts][0] = bytes2float((unsigned char *)infloat, swap);**/
        pointLat[pts] = bytes2float((unsigned char *)infloat, swap);
        fread(infloat, 1, 4, inf);
/**        points[pts][1] = bytes2float((unsigned char *)infloat, swap);**/
        pointLon[pts] = bytes2float((unsigned char *)infloat, swap);
    }
    
    fseek(inf, curpos, SEEK_SET);

    return (pts);
}

/**********************************************************************************
***********************************************************************************
*************************** OUTPUT ROUTINES ***************************************
***********************************************************************************
**********************************************************************************/

/**********************************************************************************
*
*   Module Name   : write_unisips_header()
*   Author        : Chris Robinson 
*   Purpose       : To write out the file header to a UNISIPS formatted file.
*   Date          : June 1995 
*
*   Modified By   : Greg Ip
*                   Lockheed Martin Stennis Operations
*                   August 1999
*
*                   Modifications:
*                   Added fseek to seek to the beginning of the file. 
*                   
*
**********************************************************************************/

int write_unisips_header(FILE *outf, UNISIPS_HEADER *uheader, int swap)
{
    /*  Local Variables */

    UNISIPS_HEADER uh;
    int rval; 

    /*  Begin           */

    fseek(outf,0,SEEK_SET);

   /************************************************
    * Make a copy of unisips_header structure that *
    * was passed, just in case we have to byteswap.*
    ************************************************/
    
    memcpy (&uh,uheader,sizeof(UNISIPS_HEADER));

   /***********************************************
    * byte swap if needed before writing out file *
    ***********************************************/
  
    if (swap)
    {
       swap_header(&uh);
    }

   /********************
    * write out header *
    ********************/

    if(rval = fwrite(&uh, sizeof(UNISIPS_HEADER), 1, outf) != 1)
       rval = 0;  /* Failure */
    else 
       rval = 1;  /* Successful */
       
 
    return(rval);    
}

/**********************************************************************************
*
*   Module Name   : write_unisips_rec_hdr()
*   Author        : Chris Robinson 
*   Purpose       : To write one record header to a UNISIPS formatted file.
*   Date          : June 1995
*
*   Modified      : December 1999
*			- Added 'recnum' and 'swap' parameters in support of 
*			  multichannel format.
*			- Added UNISIPS_REC_SUBHDR
*
*
**********************************************************************************/

int write_unisips_rec_hdr(FILE *outf, UNISIPS_HEADER *uhead, UNISIPS_REC_HDR *urechdr,
                          UNISIPS_REC_SUBHDR *urec_shdr,int recnum, int swap)
{

    UNISIPS_REC_HDR urh;
    UNISIPS_REC_SUBHDR ursh;
    
    int rval;
    int filepos;

    long get_rec_pos(UNISIPS_HEADER *,int);

    /* Checks to see if this is a valid record number  */
    if ((recnum >= uhead->rows) || (recnum < 0))
    { 
        printf("Invalid Record Number\n");
        return(0);
    }
    else
    {   
         /* Checks for single or mult-channel file and 
            calculates the record number position   */

         filepos = get_rec_pos(uhead,recnum);
 
         /* Checks if file position is within the valid range */

            if ((uhead->num_boundary_pts > 0) && (filepos >= uhead->boundary_pts_loc))
            {
              fprintf(stderr,"ERROR, that position falls into the boundary points location\n");
              return(0);  /* Failure */
            }

    }
    
   /* Make copies of unisips_rec_hdr and unijsips_rec_subhdr 
      structures, just in case we need to byteswap.	    */

      memcpy(&urh,urechdr,sizeof(UNISIPS_REC_HDR));
      if (uhead->sub_header_flag)
       memcpy(&ursh,urec_shdr,sizeof(UNISIPS_REC_SUBHDR));

   /* Byte swap before writing out -- if needed*/

      if (swap)
      {
        swap_unisips_rec_hdr(&urh);

        if(uhead->sub_header_flag)
         swap_unisips_rec_subhdr(&ursh);
      }  

   /* Seeks to the file position to write out record header */

      fseek(outf, filepos, SEEK_SET);

      if(rval = fwrite(&urh, sizeof(UNISIPS_REC_HDR), 1, outf) != 1)
       return(0);

      if (uhead->sub_header_flag)
      {

       /*The subheader imediately follows the record header,no need to position file pointer*/

        if (rval= fwrite(&ursh,sizeof(UNISIPS_REC_SUBHDR),1,outf)!=1)
         return(0); /*Failure*/

      }

    return(1);  /*  Successful  */
}


/**********************************************************************************
*
*   Module Name   : write_unisips_record()
*   Author        : Chris Robinson    
*   Purpose       : To write one record (record header and data) to a 
*                   UNISIPS formatted file.
*
*   NOTE          : This routine will write out the record header and ONE channel only.
*		    
*   Date          : Nov 98
*
*   Modified      : December 1999 to reflect multichannel data changes
*		     -Added 'recnum' ,'channel'  parameters to indicate which
*		      record and channel to write and 'swap' parameter to 
*                     indicate if data needs to be byteswaped.
*		     -Added UNISIPS_REC_SUBHDR parameter
*
**********************************************************************************/

int write_unisips_record(FILE *outf, UNISIPS_HEADER *uHead, 
			UNISIPS_REC_HDR *urechdr,UNISIPS_REC_SUBHDR *urec_shdr,
		        int recnum,int channel,
			unsigned char *data, int swap)
{
    /*  Local Variables  */

    UNISIPS_REC_HDR urh;
    UNISIPS_REC_SUBHDR ursh;

    long rval;
    long position;
    long write_len;
    int  bytes,ii;
    unsigned char *cdata=NULL;
    
     short          *sptr =NULL;
     unsigned short *usptr=NULL;
     int 	    *iptr =NULL;
     unsigned int   *uiptr=NULL;
     float	    *fptr =NULL;
     double	    *dptr =NULL; 



    long get_rec_pos    (UNISIPS_HEADER *,int);
    long get_channel_pos(UNISIPS_HEADER *,int,int,int);

   /***************************************************
    * Checks to see if this is a valid record number  *
    ***************************************************/
    if ((recnum >= uHead->rows) || (recnum < 0))
    { 
        printf("Invalid Record Number\n");
        return(0);
    }

    /*************************************************
     * Position the file pointer to point to the     *
     * begininng of each record depending on single  *
     * single or multi-channel                       *
     *************************************************/

     position = get_rec_pos(uHead,recnum);

    /* Checks if file position is within the valid range */

       if ((uHead->num_boundary_pts > 0) && (position >= uHead->boundary_pts_loc))
       {
         fprintf(stderr,"ERROR, that position falls into the boundary points location\n");
           return(0);  /* Failure */
       }

    /* Position file pointer*/
       fseek(outf,position,SEEK_SET);

    /* Calculate some needed parameters */
    
       bytes     = (uHead->bits_per_pixel/BitsPerByte);
       write_len = uHead->channel_width[channel] * bytes;

   /* Make copies of unisips_rec_hdr and unijsips_rec_subhdr 
      structures, just in case we need to byteswap.	    */

      memcpy(&urh,urechdr,sizeof(UNISIPS_REC_HDR));
      if (uHead->sub_header_flag)
       memcpy(&ursh,urec_shdr,sizeof(UNISIPS_REC_SUBHDR));
       
   /* Allocate memory for cdata and make copy of 'data' passed in,
      just in case we have to byte swap.			  */    
      
      cdata = (unsigned char *)malloc(sizeof(unsigned char)*write_len);
    
      if (cdata == NULL)
      {
	fprintf(stderr, "\nwrite_unisips_record():Unable to allocate memory for makeing copy ofunisips data record!!\n\n");
	return(0);
      }

      memcpy (cdata,data,write_len);
    
   /* Byte swap before writing out -- if needed*/

      if (swap)
      {
        swap_unisips_rec_hdr(&urh);
        if(uHead->sub_header_flag)
         swap_unisips_rec_subhdr(&ursh);
      }

   /* Write UNISIPS record header */

      if (rval = fwrite(&urh,sizeof(UNISIPS_REC_HDR),1,outf) != 1)
       return(0);   /* Failure to write record header  */

      if (uHead->sub_header_flag)
      {

       /*The subheader imediately follows the record header,no need to position file pointer*/

        if (rval= fwrite(&ursh,sizeof(UNISIPS_REC_SUBHDR),1,outf)!=1)
         return(0); /*Failure*/

      } 
 
    /*******************************************
     *  Write data to channel that was passed  *
     *******************************************/

     position = get_channel_pos(uHead,recnum,channel,SEEK_CUR);

     fseek(outf,position,SEEK_CUR);

  /*Byte swap image data ,if needed*/

    if (swap && (bytes > 1))
    {

    /***********************************************************
     * Here we set all of our data pointers to image->dataptr, *
     * although only one of the pointers will actually be used.*
     ***********************************************************/

      sptr   = (short          *) cdata;
      usptr  = (unsigned short *) cdata;
      iptr   = (int	    *)    cdata;
      uiptr  = (unsigned int   *) cdata;
      fptr   = (float          *) cdata;
      dptr   = (double         *) cdata;


      for (ii=0; ii<uHead->channel_width[channel]; ii++)
      {
        switch (uHead->datatype)
        {
           
 	  case (TYPE_SHORT):

	    sptr[ii] = bytes2short((unsigned char *)&(sptr[ii]),1);
	 
	   break;

          case (TYPE_UNSIGNED_SHORT):

	    usptr[ii] = (unsigned short)bytes2short((unsigned char *)&(sptr[ii]),1);

	   break;
	  case (TYPE_INTEGER):

	    iptr[ii]  = bytes2int((unsigned char *)&(iptr[ii]),1);

          break;
	  case (TYPE_UNSIGNED_INT):

	    uiptr[ii]  = (unsigned int)bytes2int((unsigned char *)&(iptr[ii]),1);

	   break;

	  case (TYPE_FLOAT):

            fptr[ii]  = bytes2float((unsigned char *)&(fptr[ii]),1);
	  
           break;

          case (TYPE_DOUBLE):

            dptr[ii] = bytes2double((unsigned char *)&(dptr[ii]),1);
  	  break;

        }/*switch*/

      }/*for*/
    
    }/*swap*/
  
      
    if (rval = fwrite(cdata,sizeof(unsigned char), write_len,outf) != write_len)
    {
        free(cdata);
        return(0);  /* Failure to write data  */
    }

 
 /*Successfully wrote out record, now free uneeded memory and return*/

    free(cdata);
    return(1);  /*  Successful writing record header and data  */

}

/**********************************************************************************
*
*   Module Name   : write_unisips_rec_all()
*   Author        : Chris Robinson    
*                   Marianne Murphy - copied from write_unisips_record and modified
*                                     it to write multiple data channels 
*   Purpose       : To write one record (record header and data) to a 
*                   UNISIPS formatted file.
*
*   NOTE          : This routine will write out the record header and multiple data
                    channels.
*		    
*   Date          : Dec 2001 - date copied
*
*   Modified      : December 1999 to reflect multichannel data changes
*		     -Added 'recnum' ,'channel'  parameters to indicate which
*		      record and channel to write and 'swap' parameter to 
*                     indicate if data needs to be byteswaped.
*		     -Added UNISIPS_REC_SUBHDR parameter
*
**********************************************************************************/

int write_unisips_rec_all(FILE *outf, UNISIPS_HEADER *uHead, 
			  UNISIPS_REC_HDR *urechdr,UNISIPS_REC_SUBHDR *urec_shdr,
		          int recnum, unsigned char *data, int swap)
{
    /*  Local Variables  */

    register int   ii, ic, iw;

    UNISIPS_REC_HDR urh;
    UNISIPS_REC_SUBHDR ursh;

    long rval;
    long position;
    long write_len;
    int  bytes;
    unsigned char *cdata=NULL;
    
     short          *sptr =NULL;
     unsigned short *usptr=NULL;
     int 	    *iptr =NULL;
     unsigned int   *uiptr=NULL;
     float	    *fptr =NULL;
     double	    *dptr =NULL; 


    long get_rec_pos    (UNISIPS_HEADER *,int);

   /***************************************************
    * Checks to see if this is a valid record number  *
    ***************************************************/
    if ((recnum >= uHead->rows) || (recnum < 0))
    { 
        printf("Invalid Record Number\n");
        return(0);
    }

    /*************************************************
     * Position the file pointer to point to the     *
     * begininng of each record depending on single  *
     * single or multi-channel                       *
     *************************************************/

     position = get_rec_pos(uHead,recnum);


    /* Checks if file position is within the valid range */

       if ((uHead->num_boundary_pts > 0) && (position >= uHead->boundary_pts_loc))
       {
         fprintf(stderr,"ERROR, that position falls into the boundary points location\n");
           return(0);  /* Failure */
       }

    /* Position file pointer*/
       fseek(outf,position,SEEK_SET);

    /* Calculate some needed parameters */
    
       bytes     = (uHead->bits_per_pixel/BitsPerByte);

       write_len = 0;
       for (ic=0; ic<uHead->total_channels; ic++)
       { 
         write_len += uHead->channel_width[ic] * bytes;
       }


   /* Make copies of unisips_rec_hdr and unisips_rec_subhdr 
      structures, just in case we need to byteswap.	    */

      memcpy(&urh,urechdr,sizeof(UNISIPS_REC_HDR));
      if (uHead->sub_header_flag)
       memcpy(&ursh,urec_shdr,sizeof(UNISIPS_REC_SUBHDR));
       
   /* Allocate memory for cdata and make copy of 'data' passed in,
      just in case we have to byte swap.			  */    
      
      cdata = (unsigned char *)malloc(sizeof(unsigned char)*write_len);
    
      if (cdata == NULL)
      {
	fprintf(stderr, "\nwrite_unisips_record():Unable to allocate memory for makeing copy ofunisips data record!!\n\n");
	return(0);
      }

      memcpy (cdata,data,write_len);
    
   /* Byte swap before writing out -- if needed*/

      if (swap)
      {
        swap_unisips_rec_hdr(&urh);
        if(uHead->sub_header_flag)
         swap_unisips_rec_subhdr(&ursh);
      }

   /* Write UNISIPS record header */

      if (rval = fwrite(&urh,sizeof(UNISIPS_REC_HDR),1,outf) != 1)
       return(0);   /* Failure to write record header  */

      if (uHead->sub_header_flag)
      {

       /*The subheader imediately follows the record header,no need to position file pointer*/

        if (rval= fwrite(&ursh,sizeof(UNISIPS_REC_SUBHDR),1,outf)!=1)
         return(0); /*Failure*/

      } 
 

  /*Byte swap image data ,if needed*/

    if (swap && (bytes > 1))
    {

    /***********************************************************
     * Here we set all of our data pointers to image->dataptr, *
     * although only one of the pointers will actually be used.*
     ***********************************************************/

      sptr   = (short          *) cdata;
      usptr  = (unsigned short *) cdata;
      iptr   = (int	    *)    cdata;
      uiptr  = (unsigned int   *) cdata;
      fptr   = (float          *) cdata;
      dptr   = (double         *) cdata;


      ii = 0;
      for (ic=0; ic<uHead->total_channels; ic++)
      {
        for (ii=0; ii<uHead->channel_width[ic]; ii++)
        {
          switch (uHead->datatype)
          {
           
 	    case (TYPE_SHORT):

	      sptr[ii] = bytes2short((unsigned char *)&(sptr[ii]),1);
	 
	     break;

            case (TYPE_UNSIGNED_SHORT):

	      usptr[ii] = (unsigned short)bytes2short((unsigned char *)&(sptr[ii]),1);

	     break;
	    case (TYPE_INTEGER):

	      iptr[ii]  = bytes2int((unsigned char *)&(iptr[ii]),1);

             break;
	    case (TYPE_UNSIGNED_INT):

	      uiptr[ii]  = (unsigned int)bytes2int((unsigned char *)&(iptr[ii]),1);

	     break;

	    case (TYPE_FLOAT):

              fptr[ii]  = bytes2float((unsigned char *)&(fptr[ii]),1);
	  
             break;

            case (TYPE_DOUBLE):

              dptr[ii] = bytes2double((unsigned char *)&(dptr[ii]),1);
  	     break;

            ii++;

          }/*switch*/
        }  /*for*/
      }/*for*/
    
    }/*swap*/
  
      
    if (rval = fwrite(cdata,sizeof(unsigned char), write_len,outf) != write_len)
    {
        free(cdata);
        return(0);  /* Failure to write data  */
    }
 
 /*Successfully wrote out record, now free uneeded memory and return*/

    free(cdata);
    return(1);  /*  Successful writing record header and data  */

}

/* #if 0 msm */
/**********************************************************************************
*
*   Module Name   : write_unisips_data()
*   Author        : Chris Robinson    
*   Purpose       : To write out unisips data (one channel only) to a  
*                   UNISIPS formatted file.
*   
*   NOTE          : Will write out data at the current file pointer location.
*		    
*   Date          : Dec 99
*
*
**********************************************************************************/

int write_unisips_data (FILE *outf, UNISIPS_HEADER *uHead, 
		         int channel,unsigned char *data, int swap)
{
    
    /*  Local Variables  */

    long rval;
    long write_len;
    int  bytes,ii;

     short          *sptr =NULL;
     unsigned short *usptr=NULL;
     int 	    *iptr =NULL;
     unsigned int   *uiptr=NULL;
     float	    *fptr =NULL;
     double	    *dptr =NULL; 


    /*******************************************
     *  Write out data -- byteswap if needed   *
     *******************************************/

  
    bytes     = (uHead->bits_per_pixel/BitsPerByte);
    write_len = uHead->channel_width[channel] * bytes;
		 


  /*Byte swap if needed*/

    if (swap && (bytes > 1))
    {

    /***********************************************************
     * Here we set all of our data pointers to image->dataptr, *
     * although only one of the pointers will actually be used.*
     ***********************************************************/

      sptr   = (short          *) data;
      usptr  = (unsigned short *) data;
      iptr   = (int	    *)    data;
      uiptr  = (unsigned int   *) data;
      fptr   = (float          *) data;
      dptr   = (double         *) data;


      for (ii=0; ii<uHead->channel_width[channel]; ii++)
      {
        switch (uHead->datatype)
        {
           
 	  case (TYPE_SHORT):

	    sptr[ii] = bytes2short((unsigned char *)&(sptr[ii]),1);
	 
	   break;

          case (TYPE_UNSIGNED_SHORT):

	    usptr[ii] = (unsigned short)bytes2short((unsigned char *)&(sptr[ii]),1);

	   break;
	  case (TYPE_INTEGER):

	    iptr[ii]  = bytes2int((unsigned char *)&(iptr[ii]),1);

          break;
	  case (TYPE_UNSIGNED_INT):

	    uiptr[ii]  = (unsigned int)bytes2int((unsigned char *)&(iptr[ii]),1);

	   break;

	  case (TYPE_FLOAT):

            fptr[ii]  = bytes2float((unsigned char *)&(fptr[ii]),1);
	  
           break;

          case (TYPE_DOUBLE):

            dptr[ii] = bytes2double((unsigned char *)&(dptr[ii]),1);
  	  break;

        }/*switch*/

      }/*for*/

    }/*if swap*/
  
      
    if (rval = fwrite(data,sizeof(unsigned char), write_len,outf) != write_len)
    {
        return(0);  /* Failure to write data  */
    }



    return(1);  /*  Successful writing record header and data  */

}
/* #endif msm */

/**********************************************************************************
*
*   Module Name   : write_boundary_pts()
*   Author        : Chuck Martin
*   Purpose       : To write out the boundary points to a UNISIPS formatted file.
*   Date          : 
*
*   MODIFIED      :  CSR  JAN 2000 now pass in a seperate float array for lat and lons
*                   
**********************************************************************************/

int write_boundary_pts(FILE *outf, UNISIPS_HEADER *uheader, float *pointLat, float *pointLon, int swap)
{
    short  pts;
    int    curpos;
    char   infloat[4];
    int    rec_size;
    int    bits_per_byte = 8;


    curpos = ftell(outf);
    fseek(outf,uheader->boundary_pts_loc,SEEK_SET);
 
    for (pts=0;pts<uheader->num_boundary_pts;pts++)
    {
    
         pointLat[pts] = bytes2float((unsigned char *)&(pointLat[pts]), swap);
         pointLon[pts] = bytes2float((unsigned char *)&(pointLon[pts]), swap);
	 fwrite(&(pointLat[pts]), 1, 4, outf);
	 fwrite(&(pointLon[pts]), 1, 4, outf);
	 
  /*     points[pts][0] = bytes2float((unsigned char *)&(points[pts][0]), swap);
         fwrite(&(points[pts][0]), 1, 4, outf);
	 
         points[pts][1] = bytes2float((unsigned char *)&(points[pts][1]), swap);
         fwrite(&(points[pts][1]), 1, 4, outf);
   */
   
         	 
	 
    }
 
    fseek(outf,curpos,SEEK_SET);    

    return(pts);
}


/*****************************************************************************
 *****************************************************************************
 ************************ UTILITY ROUTINES ***********************************
 ***************************************************************************** 
 ****************************************************************************/ 

/*****************************************************************************
*    
*   Module Name   : get_rec_pos()
*   Author        : Greg Ip
*                   Lockheed Martin Stennis Operations 
*     
*   Purpose       : This routine checks for multi-channel or single channel
*                   UNISIPS format files in order to calculate the file pointer
*                   position to the beginning of a particular record header.
*                   The calculated file pointer position is returned.
*
*		    NOTE: Record number passed should be ZERO based.
*			  i.e. the first record would be record number 0.
*                   
*   Date          : August 1999
*
*****************************************************************************/

long get_rec_pos(UNISIPS_HEADER *uhead, int recnum)
{
    int i;
    int position;                    /*  positions file pointer  */
    int total_channel_width = 0;     /*  total width for all channels */
    


    /*  Checks for single or multi-channel file */
    if (uhead->total_channels >= 2)
    {   
        /*  Calculate the total channel width for all channels */
        for (i = 0; i < uhead->total_channels; i++)
        {
             total_channel_width += uhead->channel_width[i];
        }
      
        /*  Check for sub_header record */ 
        if (uhead->sub_header_flag == 1) 
        {

            position = sizeof(UNISIPS_HEADER) + ((sizeof(UNISIPS_REC_HDR) +
                       sizeof(UNISIPS_REC_SUBHDR) + (total_channel_width * 
                       (uhead->bits_per_pixel / 8))) * (recnum)); 
        }
        else
        {
            position = sizeof(UNISIPS_HEADER) + ((sizeof(UNISIPS_REC_HDR) +
                       (total_channel_width * (uhead->bits_per_pixel / 8))) *
                        (recnum)); 
        } 
    }
    else 
    {
       /*Only one channel*/

        if (uhead->sub_header_flag == 1)
	{ 
          /*Has sub header */
     
            position = sizeof(UNISIPS_HEADER) + ((sizeof(UNISIPS_REC_HDR) +
                       sizeof(UNISIPS_REC_SUBHDR) + (uhead->cols * 
                       (uhead->bits_per_pixel / 8))) * (recnum)); 
	}
        else
	{
          /*No sub header */

          position = sizeof(UNISIPS_HEADER) + ((sizeof(UNISIPS_REC_HDR) +
                   (uhead->cols * (uhead->bits_per_pixel / 8))) * (recnum));
        }
    }

	return(position);
}   

/*****************************************************************************
*    
*   Module Name   : get_channel_pos()
*   Author        : Greg Ip
*                   Lockheed Martin Stennis Operations 
*     
*   Purpose       : This routine checks for a mult-channel or single channel
*                   UNISIPS file.  If it is multi-channel, the user is prompt
*                   for a particular channel to read.  Once the channel has 
*                   been entered, the routine calculates the offset to 
*                   position the file pointer to point to that particular 
*                   channel data.  If it is single-channel, then no 
*                   calculation is performed. 
*                   
*   Date          : August 1999
*                   OCT    1999
*			Chris Robinson NAVO
*			Completey changed routine so that it returns the 
*			file position of the channel number passed in
*			(relative to the current position).
*
*		    NOTE:  Determines position relative to current file
*			   position pointer if 'from' is SEEK_CUR.  
*			   The record header should have been read before 
*			   using the position calculated by this routine.
*
*****************************************************************************/ 
long get_channel_pos(UNISIPS_HEADER *uHead,int recnum,int channel,int from)
{
    int channel_flag = 0,i;
    long cpos=0,recpos;

    long get_rec_pos(UNISIPS_HEADER *,int);

    cpos  =0;
    recpos=0;

 
   /*If 'from' is SEEK_SET, then the calculated position should
     include the size and location of the header record.        */

     switch (from)
     {
       case (SEEK_SET):  recpos = get_rec_pos(uHead,recnum);

			 break;

       default:          recpos =0;

     }/*switch*/ 


   /*Determine how many channels to pass over*/

    for (i=0; i<channel; i++)
     cpos += (uHead->channel_width[i] * (uHead->bits_per_pixel / 8));

     


    return ((recpos+cpos));
}



