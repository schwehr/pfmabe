/*****************************************************************************
 *
 * File:	file_globals.h
 *
 * Purpose:	Contains a bunch of file constatns.
 *
 *		 
 * Revision History:   
 *
 * 99/05/03 DR	-increased NUM_LINE_FILE_TYPES from 9 to 11 (for INS files)
 * 		-added prototypes for ins routines 
 *
 * 99/12/03 DR	-added header
 *		-brought up to date with 2.30 unix version.
 *****************************************************************************/
#ifndef FILE_GLOBALS_H
#define FILE_GLOBALS_H

#ifdef  __cplusplus
extern "C" {
#endif


#define	 FILE_HAPPY		1
#define	 FILE_OK		0
#define	 FILE_NOT_FOUND		-1
#define	 FILE_EOF		-2
#define	 FILE_READ_ERR		-3
#define	 FILE_WRITE_ERR		-4
#define	 FILE_PERM_ERR		-5
#define	 FILE_LOCKED		-6
#define	 FILE_OPEN_ERR		-7
#define	 FILE_ALREADY_EXISTS	-8	
#define	 FILE_CREATE_ERR	-9	
#define	 FILE_ERROR		-10	/* Other error */


#define FILE_FIRST_RECORD	-999

#define	 NUM_MISSION_FILE_TYPES 10
#define	 NUM_LINE_FILE_TYPES 11

void Exit(int error);

int	file_create_mission_files(char *db_name);
int file_delete_mission_files( const char *db_name);

int	file_create_line_files(char *db_name, int flightline);
int file_delete_line_files( const char *db_name, int flightline );

char	*file_get_GB_DATA_DIR();
char 	*file_get_GROUNDBASE();
int	file_guess_record_location(FILE *handle, char *db_name, short flightline, int timestamp, int rec_size);
int	file_guess_record_location_ub(int handle, char *db_name, short flightline, int timestamp, int rec_size);
int	file_delete_record(char *filename, int number, int rec_size);

#ifdef  __cplusplus
}
#endif


#endif /*file_globals_h*/
