/********************************************************************
 *
 * Module Name : NPSTRING
 *
 * Author/Date : C. L. Boergesson / 13 Jan 1994
 *
 * Description : This is the header file used to create the npstring
 *     library.  This library provides functions for formatting
 *     (encode) and parsing (decode) the standard portion of
 *     strings received from sensors.  These functions convert
 *     data values between encoded internal representations and
 *     text sequences.
 *
 * Restrictions/Limitations : None
 *
 * Change Descriptions :
 *	 7/21/94  CLB -
 *	     Added definitions for SNP_NULL_LATITUDE and SNP_NULL_LONGITUDE
 *
 * Classification : Unclassified
 *
 * References : None.
 *
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/
#ifndef __npstring_h__
#define __npstring_h__

/*****************************************************************
 * SAIC library includes
 *****************************************************************/
#include "saic.h"     /* timespec definition */
#include "snptime.h"     /* timespec definition */

/*
 * If the encode and decode functions complete successfully, they return
 * a positive integer that is the number of characters encoded/decoded.
 * If the function does not complete successfully, one of these values
 * is returned.
 */
#define  SNP_STR_UNDEFINED_STYLE_CODE  -1
#define  SNP_STR_DATE_OUT_OF_BOUNDS -2
#define  SNP_STR_TIME_OUT_OF_BOUNDS -3
#define  SNP_STR_LAT_OUT_OF_BOUNDS  -4
#define  SNP_STR_LON_OUT_OF_BOUNDS  -5
#define  SNP_STR_BAD_DATA_RECORD -6
#define  SNP_STR_WRITE_LIMIT_EXCEEDED  -7
#define  SNP_STR_COMMENT_RECORD  -8
#define  SNP_STR_DATA_OUT_OF_BOUNDS -9
#define  SNP_STR_SUCCESSFUL	  0

/*
 * SystemError message type definition to be used by any realtime programs
 *  which call determine style
 */
#define  SNP_STR_UNDETERMINED_STYLE "Error determining formatted string style"

#define  SNP_STR_MAX_STANDARD_SIZE   80  /* largest possible length */
		    /* of a standard string  */
/*
 *  possible delimiters when string is encoded.  These are the possible
 *  values for the delimiter element of the t_style structure.
 */
#define SNP_STR_SPACE 0x00000001
#define SNP_STR_COMMA 0x00000002

/*
 * possible formats for date string.  These are the possible values for
 * the date element of the t_style structure.
 */
#define SNP_STR_DATE_01  0x00000001    /* mm/dd/yr   e.g. 04/30/94 */
#define SNP_STR_DATE_02  0x00000002    /* mm,dd,yr   e.g. 04,30,94 */
#define SNP_STR_DATE_03  0x00000003    /* mm dd yr   e.g. 04 30 94 */
#define SNP_STR_DATE_04  0x00000004    /* mm-dd-yr   e.g. 04-30-94 */
#define SNP_STR_DATE_05  0x00000005    /* year/julian	e.g. 1994/075 */
#define SNP_STR_DATE_06  0x00000006    /* year,julian	e.g. 1994,075 */
#define SNP_STR_DATE_07  0x00000007    /* jear julian	e.g. 1994 075 */
#define SNP_STR_DATE_08  0x00000008    /* month day, year e.g. April 30, 1994 */
#define SNP_STR_DATE_09  0x00000009    /* julian     e.g. 075  */
#define SNP_STR_DATE_NA  0x00000010    /* date/time data not available	*/

/*
 * possible formats for time string.  These are the possible values for
 * the time element of the t_style structure.
 */
#define SNP_STR_TIME_01  0x00000001    /* hh:mm:ss.ss  e.g. 14:28:05.32 */
#define SNP_STR_TIME_02  0x00000002    /* hh,mm,ss.ss  e.g. 14,28,05.32 */
#define SNP_STR_TIME_03  0x00000003    /* hh mm ss.ss  e.g. 14 28 05.32 */
#define SNP_STR_TIME_04  0x00000004    /* hhmmi        e.g. 0228p */
#define SNP_STR_TIME_05  0x00000005    /* hhmm	       e.g. 2315  */
#define SNP_STR_TIME_NA  0x00000006    /* date/time data not available	*/

/*
 * possible formats for position string.  These are the possible values for
 * the position element of the t_style structure.
 */
#define SNP_STR_POS_DEGREES  0x00000001  /* dd.dddd	 e.g 41.6384	   */
#define SNP_STR_POS_MINUTES  0x00000002  /* dd mm.mmmm	    e.g. 41 13.7389    */
#define SNP_STR_POS_SECONDS  0x00000003  /* dd mm ss.ssss  e.g. 41 13 04.1273 */
#define SNP_STR_POS_NA	     0x00000004  /* position data not available       */
#define SNP_STR_POS_SIGN     0x00000010  /* this value is or'ed with the      */
		/* above values to define if the     */
		/* hemisphere designation is by +    */
		/* and -, or by N, S, E and W	      */
		/* if set, sign is +/-; else N,S,E,W */
#define SNP_STR_HEMISPHERE   0x00000100  /* this value is or'ed with the      */
		/* above values to define if there   */
		/* is a delimiter between the	      */
		/* position value and the hemisphere */
		/* designator.	If set, there is a   */
		/* delimiter; if not, the hemisphere */
		/* designator immediately follows    */
		/* the position value:		*/
		/* 23.1583N vs. 23.1583,N	*/
#define SNP_NULL_LATITUDE  0.0	  /* these values will be used when */
#define SNP_NULL_LONGITUDE 0.0	  /* the RealTimeMerge CSU fails to */
				  /* supply a lat and long	    */

/*
 * possible precision values for time and position parameters. These values
 * will be or'ed with the above values to define a parameter's precision.
 * e.g. a position format of 0x00020002 will be in degrees and minutes, with
 * minutes having a precision of 1/100 - dd mm.mm
 */
#define SNP_STR_DECIMAL00  0x00000000  /* precision in whole numbers */
#define SNP_STR_DECIMAL01  0x00010000  /* precision to 1/10	 */
#define SNP_STR_DECIMAL02  0x00020000  /* precision to 1/100	    */
#define SNP_STR_DECIMAL03  0x00040000  /* etc.		   */
#define SNP_STR_DECIMAL04  0x00080000
#define SNP_STR_DECIMAL05  0x00100000
#define SNP_STR_DECIMAL06  0x00200000
#define SNP_STR_DECIMAL07  0x00400000
#define SNP_STR_DECIMAL08  0x00800000

/*****************************************************************************
 *  local data types
 ****************************************************************************/

typedef struct t_dtp {	  /* internal representation of date-time-position */
   struct timespec time;
   double   latitude;	     /* signed radians */
   double   longitude;	     /* signed radians */
} date_time_posn;

typedef struct t_style {
   unsigned long  delimiter;  /* either space or comma */
   unsigned long  date;    /* one of the above possible date formats */
   unsigned long  time;    /* one of the above possible time formats */
	    /* or'ed with one of the precision values */
   unsigned long  position;   /* one of the above possible position    */
} dtp_style;	     /* formats or'ed with a precision value   */
	    /* and possibly the POS_SIGN bit   */

/***************************************************************************
 * Relevant macros
 * These macros are used to supply the caller with the string length of each
 * format type.  These macros are used when the caller wants to advance the
 * pointer to the next data field.  This could be done with the decode
 * functions, but these macros eliminate the data parsing that goes along
 * with the decode functions.  In each of these macros, the input is the
 * appropriate value from the dtp_style structure.
 ***************************************************************************/

#define STD_DATE_SIZE(date_code, date_size) \
{\
   switch (date_code) {\
      case SNP_STR_DATE_01:\
      case SNP_STR_DATE_02:\
      case SNP_STR_DATE_03:\
      case SNP_STR_DATE_04:\
      case SNP_STR_DATE_05:\
      case SNP_STR_DATE_06:\
      case SNP_STR_DATE_07:\
    date_size = 8;\
    break;\
      case SNP_STR_DATE_08:\
    date_size = 18;\
    break;\
      case SNP_STR_DATE_09:\
    date_size = 3;\
    break;\
      case SNP_STR_DATE_NA:\
    date_size = 1;\
    break;\
      default:\
    date_size = SNP_STR_UNDEFINED_STYLE_CODE;\
   }\
}

#define STD_TIME_SIZE(time_code, time_size) \
{\
   int	  len1, len2;\
   long  style;\
   style = time_code & 0x7;\
   switch (style) {\
      case (SNP_STR_TIME_01):\
      case (SNP_STR_TIME_02):\
      case (SNP_STR_TIME_03):\
    len1 = 8;\
    break;\
      case (SNP_STR_TIME_04):\
    len1 = 5;\
    break;\
      case (SNP_STR_TIME_05):\
    len1 = 4;\
    break;\
      case (SNP_STR_TIME_NA):\
      default:\
    len1 = 0;\
   }\
   PRECISION_SIZE (time_code, len2);\
   if (len2 == 0)\
      time_size = len1;\
   else\
      time_size = len1 + len2 + 1;     /* add 1 for the decimal point */\
}

#define STD_DATE_TIME_SIZE(date_code, time_code, dt_size) \
{\
   int	  dlen, tlen;\
   if (date_code == SNP_STR_DATE_NA)\
       dt_size = 1;\
   else {\
      STD_DATE_SIZE (date_code, dlen);\
      STD_TIME_SIZE (time_code, tlen);\
      dt_size = dlen + tlen + 1;    /* add 1 for space between date and time */\
   }\
}

#define STD_LAT_SIZE(pos_code, lat_size) \
{\
   int	  len1, len2;\
   long  code;\
   code = pos_code & 0x7;\
   switch (code) {\
      case SNP_STR_POS_DEGREES:\
    len1 = 2;\
    break;\
      case SNP_STR_POS_MINUTES:\
    len1 = 5;\
    break;\
      case SNP_STR_POS_SECONDS:\
    len1 = 8;\
    break;\
      case SNP_STR_POS_NA:\
    len1 = 0;\
    break;\
      default:\
    len1 = SNP_STR_UNDEFINED_STYLE_CODE;\
   }\
   if (pos_code & SNP_STR_HEMISPHERE)  /* there is a delimiter between */\
      len1 += 2;	/* lat and hemisphere indicator */\
   else\
      len1 += 1;\
   PRECISION_SIZE(pos_code, len2);\
   if (len2 == 0)\
      lat_size = len1;\
   else\
      lat_size = len1 + len2 + 1;      /* add 1 for decimal point */\
}

#define STD_LON_SIZE(pos_code, lon_size) \
{\
   if (pos_code == SNP_STR_POS_NA)\
      lon_size = 0;\
   else {\
       STD_LAT_SIZE (pos_code, lon_size);\
       lon_size += 1;	       /* add 1 because lat degrees are */\
   }		   /* 2 digits, lon degrees are 3   */\
}

#define PRECISION_SIZE(code, length) \
{\
   length = 0;\
   if (code & SNP_STR_DECIMAL01)\
      length = 1;\
   if (code & SNP_STR_DECIMAL02)\
      length = 2;\
   if (code & SNP_STR_DECIMAL03)\
      length = 3;\
   if (code & SNP_STR_DECIMAL04)\
      length = 4;\
   if (code & SNP_STR_DECIMAL05)\
      length = 5;\
   if (code & SNP_STR_DECIMAL06)\
      length = 6;\
   if (code & SNP_STR_DECIMAL07)\
      length = 7;\
   if (code & SNP_STR_DECIMAL08)\
      length = 8;\
}

/*****************************************************************************
 * NPSTRING Library Function Prototypes
 ****************************************************************************/

/*
 * EncodeStandardString - creates a character string (buf) of date, time and
 *    position using the date, time and position values in the dtp struct and
 *    the format specified in style_code struct.  Upon success, it returns
 *    the number of characters encoded.  Returning a value < 0 indicates
 *    failure.
 */
int OPTLK EncodeStandardString
      (dtp_style *style_code, date_time_posn *dtp, int max_length, char *buf);

/*
 * DecodeStandardString - populates the date, time and position values in the
 *    dtp struct by reading the values from a character string and parsing
 *    the string based on the format specified in the style_code struct.  It
 *    sets a pointer (buf) to the first byte of the string following the
 *    standard portion.  Upon success, it returns the number of characters
 *    decoded. Returning a value < 0 indicates failure.
 */
int OPTLK DecodeStandardString
      (char *string, dtp_style *style_code, date_time_posn *dtp);

/*
 * EncodeTimeString - creates a character string (buf) containing date and time
 *    from the date and time values in the tp struct based on the date and time
 *    formats specified in the style_code struct.  Upon success, it returns
 *    the number of characters encoded.  Returning a value < 0 indicates failure.
 */
int OPTLK EncodeTimeString
      (struct timespec *tp, dtp_style *style_code, int max_length, char *buf);

/*
 * DecodeTimeString - populates the tp struct with the time and date from a
 *     character string.  It parses the string based on the time and date
 *     formats in the style_code struct.  Upon success, it returns the number
 *     of characters decoded.  Returning a value < 0 indicates failure.
 */
int OPTLK DecodeTimeString
      (char *string, dtp_style *style_code, struct timespec *tp);

/*
 * EncodeLatString - creates a character string (buf) containing latitude
 *    from the latitude value based on the position format specified in the
 *    the style_code struct.  Upon success, it returns the number of
 *    characters encoded.  Returning a value < 0 indicates failure.
 */
int OPTLK EncodeLatString
      (double latitude, dtp_style *style_code, int max_length, char *buf);

/*
 * DecodeLatString - creates the latitude value in signed radians from a
 *    character string.  It parses the string based on the position format
 *    in the style_code struct.  Upon success, it returns the number of
 *    characters decoded.  Returning a value < 0 indicates failure.
 */
int OPTLK DecodeLatString (char *string, dtp_style *style_code, double *lat);

/*
 * EncodeLonString - creates a character string (buf) containing longitude
 *    from the longitude value based on the position format specified in the
 *    the style_code struct.  Upon success, it returns the number of
 *    characters encoded.  Returning a value < 0 indicates failure.
 */
int OPTLK EncodeLonString
      (double longitude, dtp_style *style_code, int max_length, char *buf);

/*
 * DecodeLonString - creates the longitude value in signed radians from a
 *    character string.  It parses the string based on the position format
 *    in the style_code struct.  Upon success, it returns the number of
 *    characters decoded.  Returning a value < 0 indicates failure.
 */
int OPTLK DecodeLonString (char *string, dtp_style *style_code, double *lon);

/*
 * DetermineStyle - populates the style_code struct for a given input string.
 *    Upon success, it returns a 0.  Returning a value < 0 indicates failure.
 */
int OPTLK DetermineStyle (char *string, dtp_style *style_code);

/*
 * ConfirmStandardBounds - confirms that time and position fall within their
 *    allowable bounds.  Upon success, it returns a 0.	 Returning a value
 *    < 0 indicates failure.
 */
int OPTLK ConfirmStandardBounds
      (struct timespec *tp, double latitude, double longitude);

/*
 * GetYear - parses year from an ISS filename of format XXYY_DDD.XXX and
 * initializes a timespec structure accordingly. This function should only
 * be called when working with a date style code of SNP_STR_DATE_09.
 * The timespec pointer passed to this function should be the same as the
 * pointer passed to any subsequent calls to DecodeTimeString or
 * DecodeStandardString.
 */
int OPTLK GetYear (char *filename, struct timespec *tp);
#endif
