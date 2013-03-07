/*****************************************************************************
 * timestamp.c
 *
 * This file a couple of routines for converting back and forth from/to
 * timestamp and time
 *
 *        Timestamp is now defined as (32 bit integer):
 * 
 *               bits 
 *          31 - 29     28 - 0
 *          ----------|--------- 
 *
 *           date_code  time  
 *	
 *	where:
 *		date_code = 0-7
 *		time = milliseconds since midnight * 2
 *		
 * 	The * 2 part is to give sufficient space between successive
 *	shots.  The old timestamp was just in milliseconds since midnight
 *	but at 400 Hz, it would could by 2.5 per shot which is not
 *	good enough seeing it would be rounded to 2 or 3 being the 
 * 	increment.  Now it will count by 5's for 400 hz and 10's for
 *	200 hz.
 * 
 * Contains:
 *==========
 * time_to_ts()	-convert from time (h,m,s,date_code) to timestamp
 *
 * ts_to_time()	-convert from timestamp to time (h,m,s,date_code) 
 * 
 * dtime_to_ts() - convert from the double time  in milliseconds timestamp
 *
 * ts_to_timestr - convert timestamp to a string
 *
 * timestr_to_ts - convert time (in string format) to timestamp
 *
 * ts_to_dtime() - convert from timestamp to double time
 *****************************************************************************/

#include <stdio.h>
#include "file_globals.h"

/*****************************************************************************
 * time_to_ts 
 *
 * Purpose:	To convert from time (h, m, s, date_code) to timestamp
 * 		using the new timestamp format (400 Hz version)
 *
 * 		Given the inputs of hour, minute, second, milliseconds  
 *		and date code (0-7), will convert to and return
 *		a timestamp value.
 *
 * Modifications:
 * ==============
 * 98/10/10 DR	-wrote it... 
 *
 *****************************************************************************/
int time_to_ts( short hour, short min, short sec, short msec, short d_code )
{
	int	timestamp;

	timestamp = (((hour * 3600000) + (min * 60000) + (sec * 1000) + msec) * 
		2) + (d_code << 28);

	return( timestamp );
}

/*****************************************************************************
 * ts_to_time 
 *
 * Purpose:	To convert from timestamp to time (h, m, s, date_code) 
 * 		using the new timestamp format (400 Hz version)
 *
 * 		Given the inputs of a timestamp, it will 
 *		convert to hours, minutes, seconds, milliseconds and 
 *		date code (0-7).
 *
 * Modifications:
 * ==============
 * 98/10/10 DR	-wrote it... 
 *
 * 99/06/22 DR	-would not convert a midnight timestamp (0:00) with a date
 *		code [ie: 805306368 which is 0:00 with a date code of 3]
 *		Fixed by changing "if (timestamp > offset)" to 
 *		"if (timestamp >= offset)"
 *****************************************************************************/ 
void ts_to_time( int timestamp, short* hour, short* min, short* sec, short* msec, short* date_code )
{
	int	i, offset;

 	*date_code = 0;

	for (i = 7; i > 0; i--) 
	{

		offset = i << 28;
		if (timestamp >= offset) 
		{
			*date_code = i;
			timestamp = timestamp - offset;
			break;
		}
	}

	/* Convert to milliseconds [.5 for rounding] */
	timestamp = (int)(((float)timestamp /2.) + 0.5);
	
	/* Remove hours */
	(*hour) = (int) (timestamp / 3600000.);
	timestamp = timestamp - ((*hour) * 3600000);

	/* Remove minutes */
	(*min) = (int) (timestamp / 60000.);
	timestamp = timestamp - ((*min) * 60000);

	/* Remove seconds */
	(*sec) = (int) (timestamp / 1000.);

	/* Left with milliseconds */
	(*msec) = timestamp - ((*sec)* 1000);


}


/*****************************************************************************
 * ts_to_dtime 
 *
 * Purpose:	To convert from timestamp to dtime (decimal hours)
 * 		using the new timestamp format (400 Hz version)
 *
 * 		Given the inputs of a timestamp, it will 
 *		convert to decimal hours and a date code (0-7).
 *
 * Modifications:
 * ==============
 * 00/07/13 DR	-wrote it... 
 *****************************************************************************/ 
void ts_to_dtime( int timestamp, double *dtime, short* date_code )
{
	int	i, offset;

 	*date_code = 0;

	for (i = 7; i > 0; i--) 
	{

		offset = i << 28;
		if (timestamp >= offset) 
		{
			*date_code = i;
			timestamp = timestamp - offset;
			break;
		}
	}

	/* Convert to milliseconds [.5 for rounding] */
	timestamp = (int)(((float)timestamp /2.) + 0.5);
	
	/* Calc time in hours */
	(*dtime) = (double)timestamp / 3600000.;
}
/*****************************************************************************
 * dtime_to_ts 
 *
 * Purpose:	To convert from (double) laser time (dltime) to  
 * 		the new timestamp format (400 Hz version)
 *
 * 		Given the inputs dltime and a date code, it will 
 *		convert to timestamp.
 *
 * Modifications:
 * ==============
 * 98/10/10 DR	-wrote it... 
 *
 *****************************************************************************/ 
int dtime_to_ts( double dltime, short d_code)
{
	int	timestamp;


	timestamp = (int)(dltime * 2.) + (d_code << 28);

	return( timestamp );
}


/***********************************************************************
 * timestr_to_ts
 *
 * Purpose:	This routine will convert a time to a timestamp given
 *		the time as a string in the form hh:mm:ss.xxx and a date code.
 *
 * Modifications:
 * 
 * 95/03/03 DR	- wrote routine
 **********************************************************************/
void timestr_to_ts( char* time_string, int date_code, int* timestamp)
{
	int	hour, minute, second, msec;
	int	scn_cnt, parse_error;
	char 	error_reason_string[60];

	scn_cnt = sscanf(time_string, "%d:%d:%d.%3d", 
		&hour, &minute, &second, &msec);

 	if(scn_cnt != 4)
	{
		parse_error = 1;
		sprintf(error_reason_string, "String Parsing Error.\n\
			String should be in format HH:MM:SS.mmm");
	}
	else if((hour > 24) || (hour < 1))
	{
		parse_error = 1;
		sprintf(error_reason_string, "Hour %d is out of range.\n\
			Hours range is 1-24.", hour);
	}
	else if((minute > 59) || (minute < 0))
	{
		parse_error = 1;
		sprintf(error_reason_string, "Minute %d is out of range.\n\
			Minutes range is 0-59.", minute);
	}
	else if((second > 59) || (second < 0))
	{
		parse_error = 1;
		sprintf(error_reason_string, "Second %d is out of range.\n\
			Seconds range is 0-59.", second);
	}
	else if((msec > 1000) || (msec < 0))
	{
		parse_error = 1;
		sprintf(error_reason_string, "msec %d is out of range.\n\
			msec range is 0-999.", msec);
	}
 	else
		parse_error = 0;

	if(parse_error)
	{
		printf ("timestr_to_ts: The time\n\
			%s does not convert to a\n\
			legal timestamp.\n\n\
			%s", time_string, error_reason_string);
		
		return;
	}

	*timestamp = (hour * 3600000 + minute * 60000 + 
		second * 1000 + msec) * 2;
 	*timestamp = (*timestamp) + (date_code << 28);


}

/***********************************************************************
 * ts_to_timestr
 *
 * Purpose:	This routine will convert a timestamp to a time and a 
 *		date_code.  The time will be a string in the format  
 *		hh:mm:ss.xxx.
 *
 * Modifications:
 * 
 * 95/03/03 DR	- wrote routine
 *
 * 98/10/14 DR	- moved from wf_ch_shot routine to here.
 *		- divide timestamp by 2 (new 400 Hz timestamp is .5 millisec)
 *		-cleanup/comment
 *		-use ts_to_time() to get time field values...
 **********************************************************************/
void ts_to_timestr(int timestamp,short *date_code,char *time_string )
{
	short	hour, minute, second, msec;

	ts_to_time( timestamp, &hour, &minute, &second, &msec, date_code);

	/* Convert to string */
	sprintf(time_string, "%d:%.2d:%.2d.%.3d\0", hour, 
		minute, second, msec);

}
