/*****************************************************************************
 * auto.h 	Header
 *
 * Purpose:  	This header file hold some structures from Automated
 *		Processing.
 *              
 *
 * Revision History:
 * 93		DR	Created
 *
 * 98/02/04 DR	-added "auto_output_type" to AU_PROCESS_T.  It will
 *		 have the value of 0 (xyz), 1 (xyz+results) or 
 *		 2 (xyz+results+diagnostics).  This is done as there
 *		 will only be one auto exec (instead of three) and
 *		 this flag will be used within auto to determine which
 *		 outputs files will be written...
 *
 * 98/02/05 DR	-400 Hz Cleanup
 * 
 * 98/06/12 DR	-added POS_T for early postion stuff used in auto.  Used
 *		 to be in inputs but now use the input file's structure in
 * 		 auto.
 *****************************************************************************/
#ifndef _AUTO
#define _AUTO

typedef struct
{
	int	start_time;
	int	stop_time;
	short	line_num;
	short	hap_ver;
} AU_TIMES_T;

typedef struct
{
	short	number_of_lines;
	AU_TIMES_T	*au_line;
} AU_LINES_T;

typedef struct
{
	short	auto_pro_type;
	short	auto_output_type;
	char	db_name[40];
 	AU_LINES_T lines;
} AU_PROCESS_T;

typedef struct
{
	short	auto_pro_type;
	short	fill;
 	AU_TIMES_T	line;
 } AU_SELECT_T;
 
typedef struct 
{                        
	float		scanner_az;
	float		nadir_angle;
	short		nadir_angle_flag;
} POS_T; 			 

#endif