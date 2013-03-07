#ifndef _INGEST_UTILITIES_
#define _INGEST_UTILITIES_

/* Macro definitions for return codes */
#define BAD_PLATFORM_ID -1
#define BAD_FILE_ID     -2
#define BAD_START_TIME  -3
#define MAKE_DIR_FAIL   -4
#define INSUF_MEMORY    -5
#define FILE_EXISTS     -6
#define FOPEN_FAIL      -7

/* Get any necessary header files */
#include "snptime.h"                  /* has struct timespec definition */
#include "gsf.h"
#include "hmpsparm.h"                 /* has HMPS_PARAMETERS definition */

/* Macro definitions for the supported sonar types */
#define EM100            1
#define EM950            2
#define EM1000           3
#define EM12             4
#define EM121            5
#define EM121A           6
#define EM3000           7
#define SEABEAM          8
#define SEAMAP           9
#define SASS            10
#define SEABAT          11

#define EM100_NAME       "EM100" 
#define EM950_NAME       "EM950"
#define EM1000_NAME      "EM1000"
#define EM12_NAME        "EM12"
#define EM121_NAME       "EM121"
#define EM121A_NAME      "EM121A"
#define EM3000_NAME      "EM3000"
#define SEABEAM_NAME     "SEABEAM"
#define SEAMAP_NAME      "SEAMAP"
#define SASS_NAME        "SASS"
#define SEABAT_NAME      "SEABAT"

/* Define a data structure to hold ship installation information */
typedef struct t_ship_data
{
    int sonar_type;
    HMPS_PARAMETERS p;
    char platformID[16]; 
    char sensorFileID[16];
} SHIP_DATA;

/* Function prototypes */

int buildIngestFileName(char *inFile, char *platformID, char *fileID, char *subDir, struct timespec *sTime, char *name, int nSize);
/* Description : This function builds an ISS-60 format filename, but
 *    which includes the hour, minutes, and seconds. 
 *
 * Inputs :
 *    inFile = a pointer to a character string containing the 
 *        path to the input file, from which the output file path
 *        is derived. 
 *    platformID = a pointer to a character string containing the 
 *        two character survey platform identifier.
 *    fileID = a pointer to a character string containing the three
 *        character file identifier.
 *    subDir = a pointer to a character string containing the name of
 *        the directory which will be created across from the raw data
 *        directory
 *    sTime = a pointer to a struct timespec containing the time
 *        from which the file name is to be built.
 *    name = a pointer to memory provided by the caller into which the
 *        file name is loaded.
 *    nSize = the dimension of the caller's memory at name.
 *
 * Returns : This function returns zero if successful, or a negative
 *    number if an error occurs.
 */

void checkBounds (gsfRecords *rec);
/* Description : This is a little function which stores the swath
 *     bathymetry spatial, temporal and depth bounds into the summary
 *     structure of a gsfRecords structure.  It expects the mb_ping
 *     structure to be populated.
 *
 * Inputs :
 *    rec = a pointer to a gsfRecords data structure, the summary substructure
 *        is loaded from data obtained from the mb_ping substructure.
 *
 * Returns : none
 */

int writeHistoryRecord (int argc, char **argv, char *comment, char *gsfFile, int handle, int writeSummary);
/* Description : This function writes a generic history record to the
 *    end of an open gsf data file.  The file must have been open
 *    either UPDATE or CREATE.
 *
 * Inputs :
 *    argc = an integer containing the number of command line arguments
 *        used to execute the program.
 *    argv = an array of pointers to character strings containing the
 *        command line arguments used to execute the program.
 *    comment = a pointer to a program specific character string used
 *        to provide amplifying information
 *    gsfFile = a pointer to a character string containing teh path of the gsf file.
 *    handle = an integer value specifying the handle to the gsf file
 *        to which the history record is added.
 *    writeSummary = an integer flag which if set indicates that the summary file
 *        should be appended with this history record.
 *
 * Returns : This function returns zero if successful, or a negative number
 *    if an error occurs.
 */

int loadShipOffsets (char *fileName, SHIP_DATA *ship);
/* Description : This function loads the relavent information from the
 *    ship offsets file.
 *
 * Inputs :
 *  fileName = a pointer to a character string containing the name of the
 *     ship offsets file to be loaded.
 *  ship = a pointer to a SHIP_DATA structure allocated by the caller into
 *     which the data contained in the file will be loaded.
 *
 * Returns :
 *  This function returns zero if successful, or a negative number if
 *     an error occurs.
 */

void writeSampleOffsets (void);
/* Description : This function will write a sample ship offsets file
 *   for the user to modify.  The offsets file is used by the ingest
 *   programs and by apply correctors.
 *
 * Inputs : none
 *
 * Returns : none
 */

#endif
