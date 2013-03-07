/********************************************************************
 *
 * Module Name : ing_util
 *
 * Author/Date : JSB November 11, 1995
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 *
 *
 * Classification : Unclassified
 *
 * References :
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

/* Standard C library includes */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <limits.h>

/* Unix specific include files */
#include <sys/stat.h>

/* Get the necessary header files */
#include "snptime.h"
#include "snputil.h"
#include "hmpsutil.h"
#include "hmpsflag.h"
#include "gsf.h"

/********************************************************************
 *
 * Function Name : buildIngestFileName
 *
 * Description : This function builds an ISS-60 format filename, but
 *    which includes the hour, minutes, and seconds. If the path to the
 *    file does not exist, then it is created here.
 *
 * Inputs :
 *    inputFile = a pointer to a character string containing the
 *        path to the input file, from which the output file path
 *        is derived.
 *    platformID = a pointer to a character string containing the
 *        two character survey platform identifier.
 *    fileID = a pointer to a character string containing the three
 *        character file identifier.
 *    subDir = a pointer to a character string containing the name of
 *        the directory which will be created across from the raw data
 *        directory
 *    start_time = a pointer to a struct timespec containing the time
 *        from which the file name is to be built.
 *    name = a pointer to memory provided by the caller into which the
 *        file name is loaded.
 *    name_size = the dimension of the caller's memory at name.
 *
 * Returns : This function returns zero if successful, or a negative
 *    number if an error occurs.
 *
 * Error Conditions :
 *
 ********************************************************************/

int
buildIngestFileName(char *inputFile, char *platformID, char *fileID, char *subDir, struct timespec *start_time, char *name, int name_size)
{
    int             i;
    int             len;
    int             first_dir = 0;
    int             second_dir = 0;
    char           *ptr;
    char            name_str[64];
    char            path_str[64];
    struct tm      *t;
    struct stat     stat_buf;

    /* platformID must be two characters long */
    if (strlen(platformID) != 2)
    {
        return (BAD_PLATFORM_ID);
    }

    /* fileID must be three characters long */
    if (strlen(fileID) != 3)
    {
        return (BAD_FILE_ID);
    }

    /* Convert the three character file id to all upper case */
    for (i = 0; i < strlen(fileID); i++)
    {
        fileID[i] = toupper(fileID[i]);
    }

    /* Make sure we are on Greenwich time zone */
    SET_TZ_GMT;

    /* Convert the specified timespec to a struct tm */
    t = gmtime(&start_time->tv_sec);
    if (t == (struct tm *) NULL)
    {
        return (BAD_START_TIME);
    }

    /* Build the file name into local memory */
    sprintf(name_str, "%s%s%02d%03d%02d%02d.d01",
        platformID,
        fileID,
        t->tm_year,
        t->tm_yday + 1,
        t->tm_hour,
        t->tm_min);

    /* Build the output directory path */
    strncpy(path_str, inputFile, sizeof(path_str));
    len = strlen(path_str);
    ptr = path_str + len;
    for (i = 0; i < len; i++)
    {
        ptr--;
        if (*ptr == '/')
        {
            if (first_dir)
            {
                sprintf(ptr, "/%s", subDir);
                second_dir++;
                break;
            }
            first_dir++;
        }
    }

    if (!first_dir)
    {
        sprintf(path_str, "../%s", subDir);
    }
    else if (!second_dir)
    {
        sprintf(path_str, "%s", subDir);
    }


    /* If this directory is not here then create it */
    if (stat(path_str, &stat_buf))
    {
        if (MakePath(path_str))
        {
            return (MAKE_DIR_FAIL);
        }
    }

    /* Ensure caller has sufficent memory at their pointer */
    if ((strlen(name_str) + strlen(path_str)) >= name_size)
    {
        return (INSUF_MEMORY);
    }

    strcpy(name, path_str);
    strcat(name, "/");
    strcat(name, name_str);

    /* Test to ensure that this file doesn't already exist */
    if (stat(name, &stat_buf) == 0)
    {
        return (FILE_EXISTS);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : checkBounds
 *
 * Description : This is a little function which stores the swath
 *     bathymetry spatial, temporal and depth bounds into the summary
 *     structure of a gsfRecords structure.  It expects the mb_ping
 *     structure to be populated.
 *
 * Inputs :
 *    rec = a pointer to a gsfRecords data structure, the summary substructure
 *        is loaded from data obtained from the mb_ping substructure.
 *
 * Returns : none
 *
 * Error Conditions :
 *
 ********************************************************************/

void
checkBounds(gsfRecords * rec)
{
    int             i;
    static int      first = 1;
    double          diff;

    if (first)
    {
        rec->summary.min_latitude = GSF_NULL_LATITUDE;
        rec->summary.min_longitude = GSF_NULL_LONGITUDE;
        rec->summary.max_latitude = -1.0 * GSF_NULL_LATITUDE;
        rec->summary.max_longitude = -1.0 * GSF_NULL_LONGITUDE;
        rec->summary.start_time.tv_sec = LONG_MAX;
        rec->summary.min_depth = 1.0e6;
        rec->summary.max_depth = -1.0e6;
        first = 0;
    }

    /* Store the time of the first and the the time of the last ping */
    SubtractTimes(&rec->mb_ping.ping_time, &rec->summary.start_time, &diff);
    if (diff < 0)
    {
        rec->summary.start_time = rec->mb_ping.ping_time;
    }

    SubtractTimes(&rec->mb_ping.ping_time, &rec->summary.end_time, &diff);
    if (diff > 0)
    {
        rec->summary.end_time = rec->mb_ping.ping_time;
    }

    /* Store the min and max valide latitudes */
    if (rec->mb_ping.latitude != GSF_NULL_LATITUDE)
    {
        if (rec->mb_ping.latitude < rec->summary.min_latitude)
        {
            rec->summary.min_latitude = rec->mb_ping.latitude;
        }
        if (rec->mb_ping.latitude > rec->summary.max_latitude)
        {
            rec->summary.max_latitude = rec->mb_ping.latitude;
        }
    }

    /* Store the min and max valide longitudes */
    if (rec->mb_ping.longitude != GSF_NULL_LONGITUDE)
    {
        if (rec->mb_ping.longitude < rec->summary.min_longitude)
        {
            rec->summary.min_longitude = rec->mb_ping.longitude;
        }
        if (rec->mb_ping.longitude > rec->summary.max_longitude)
        {
            rec->summary.max_longitude = rec->mb_ping.longitude;
        }
    }

    /* Store the min and max valid depths */
    for (i = 0; i < rec->mb_ping.number_beams; i++)
    {
        if (rec->mb_ping.beam_flags != (unsigned char *) NULL)
        {
            if (rec->mb_ping.beam_flags[i] & HMPS_IGNORE_NULL_BEAM)
            {
                continue;
            }
            if (rec->mb_ping.depth[i] < rec->summary.min_depth)
            {
                rec->summary.min_depth = rec->mb_ping.depth[i];
            }
            if (rec->mb_ping.depth[i] > rec->summary.max_depth)
            {
                rec->summary.max_depth = rec->mb_ping.depth[i];
            }
        }
    }
    return;
}
/********************************************************************
 *
 * Function Name : writeHistoryRecord
 *
 * Description : This function writes a generic history record to the 
 *    end of an open gsf data file.  The file must have been open 
 *    either UPDATE or CREATE. 
 *
 * Inputs :
 *    argc = an integer containing the number of command line arguments
 *        used to execute the program.
 *    argv = an array of pointers to character strings containing the
 *        command line arguments used to execute the program.
 *    comment = a pointer to a application specific character string used
 *        to provide amplifying information
 *    gsfFile = a pointer to a character string containing the path to the gsf file.
 *    handle = an integer value specifying the handle to the gsf file
 *        to which the history record is added.
 *    writeSummary = an integer flag which if set indicates that the summary
 *        file should be appended with this history record 
 *
 * Returns : This function returns zero if successful, or a negative number 
 *    if an error occurs.
 *
 * Error Conditions :
 *
 ********************************************************************/

int
writeHistoryRecord (int argc, char **argv, char *comment, char *gsfFile, int handle, int writeSummary)
{
    int             i;
    int             rc;
    int             ret;
    int             len;
    extern int      gsfError;
    char            str[1024];
    char            datasum_str[128];
    char            t_str[64];
    char            sumFile[128];
    time_t          t;
    FILE           *sumFD;
    gsfRecords      rec;
    gsfDataID       gsfID;
    struct stat     stat_buf;

    memset (&rec, 0, sizeof(rec));

    /* Load the contents of the gsf history record */

    time (&t);
    rec.history.history_time.tv_sec = t;
    rec.history.history_time.tv_nsec = 0;

    str[0] = '\0';
    len = 0;
    for (i=0; i<argc; i++)
    {
        len += strlen(argv[i]) + 1;
        if (len >= sizeof(str))
        {
            return(INSUF_MEMORY);
        }
        strcat(str, argv[i]);
        strcat(str, " ");
    }

    rec.history.command_line = str;

    rc = gethostname(rec.history.host_name, sizeof(rec.history.host_name));

    rec.history.comment = comment;

    /* Seek to the end of the file and write a new history record */
    ret = gsfSeek (handle, GSF_END_OF_FILE);
    memset(&gsfID, 0, sizeof(gsfID));
    gsfID.recordID = GSF_RECORD_HISTORY;
    ret = gsfWrite(handle, &gsfID, &rec);
    if (ret < 0)
    {
        return(gsfError); 
    }

    /* Update (or create) the summary file if desired */ 
    if (writeSummary)
    {
        strncpy(sumFile, gsfFile, sizeof(sumFile));
        sumFile[strlen(sumFile) - 3] = 's';

        /* If the summary file is not here then create it, and run datasumm to it */
        if (stat(sumFile, &stat_buf))
        {
            fprintf(stdout, "Creating summary file: %s\n", sumFile);
            sprintf(datasum_str, "datasumm -f %s > %s", gsfFile, sumFile);
            system (datasum_str);
        } 
   
        /* Now add the history record to the summary file
         * Entry will look like:
         * ------------------------------------------
         * | HISTORY RECORD | hh:mm:ss mmm dd, yyyy |
         * ------------------------------------------
         * Host Name: <hostname>
         * Command Line: <program command line>
         * Comment: <application specific comment>
         */
         sumFD = fopen(sumFile, "a+");
         if (sumFD == (FILE *) NULL)
         {
             return(FOPEN_FAIL);
         }
       
         t = gmtime(&rec.history.history_time.tv_sec);
         strftime(t_str, sizeof(t_str), "%H:%M:%S %b %d, %Y", t);
         fprintf(sumFD, "\n");
         fprintf(sumFD, "------------------------------------------\n");
         fprintf(sumFD, "| HISTORY RECORD | %s |\n", t_str);
         fprintf(sumFD, "------------------------------------------\n");
         fprintf(sumFD, "Host Name: %s\n", rec.history.host_name);
         fprintf(sumFD, "Command Line: %s\n", rec.history.command_line); 
         if ((rec.history.comment != (char *) NULL) && (strlen(rec.history.comment) > 1))
         {
             fprintf(sumFD, "Comment: %s\n", rec.history.comment); 
         }
         fprintf(sumFD, "\n");
 
         fclose(sumFD);
    } 

    return(0);
}

/*********************************************************************
 *
 * Function Name : loadShipOffsets
 *
 * Description : This function loads the relavent information from the
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
 *
 * Error Conditions :
 *
 ********************************************************************/

int 
loadShipOffsets (char *fileName, SHIP_DATA *ship)
{
    int             i;
    int             len;
    char           *ptr;
    char           *tptr;
    char            key[128];
    char            name[64]; 
    FILE           *offF;

    /* Try to open the user specified offsets file */
    offF = fopen (fileName, "r");
    if (offF == (FILE *) NULL)
    {
        fprintf(stdout, "Error opening ship offsets file: %s\n", fileName);
        exit(1);
    }

    /* Load the platform ID for this data */
    ptr = GetEnvVal("PLATFORM_ID", offF);
    if (ptr)
    {
        tptr = ptr;
        len = strlen(ptr);
        ship->platformID[0] = '\0'; 
        for (i=0; i<len; i++)
        {
            if (*tptr == ' ')
            {
                tptr++;
            }
            else
            {
                break;
            }
        }
        if (i < len)
        {
            strncpy(ship->platformID, tptr, 2);
        }
    }
    else
    {
        fprintf(stdout, "Error reading PLATFORM_ID\n");
        exit(1);
    }

    /* Load the position x offset */
    ship->p.to_apply.position_offset.x = 0.0;
    ptr = GetEnvVal("POSITION_OFFSET_X", offF);
    if (ptr)
    {
        if (!strstr(ptr, "NA"))
        {
            ship->p.to_apply.position_offset.x = atof(ptr);
        }
    }
    else
    {
        fprintf(stdout, "Error reading POSITION_OFFSET_X\n");
        exit(1);
    }

    /* Load the position y offset */
    ship->p.to_apply.position_offset.y = 0.0;
    ptr = GetEnvVal("POSITION_OFFSET_Y", offF);
    if (ptr)
    {
        if (!strstr(ptr, "NA"))
        {
            ship->p.to_apply.position_offset.y = atof(ptr);
        }
    }
    else
    {
        fprintf(stdout, "Error reading POSITION_OFFSET_Y\n");
        exit(1);
    }


    /* Load the position z offset */
    ship->p.to_apply.position_offset.z = 0.0;
    ptr = GetEnvVal("POSITION_OFFSET_Z", offF);
    if (ptr)
    {
        if (!strstr(ptr, "NA"))
        {
            ship->p.to_apply.position_offset.z = atof(ptr);
        }
    }
    else
    {
        fprintf(stdout, "Error reading POSITION_OFFSET_Z\n");
        exit(1);
    }

    /* Build the parameter keyword based on the sonar type */
    switch(ship->sonar_type)
    {
        case EM100:
            strcpy(name, EM100_NAME);
            break;
       
        case EM950:
            strcpy(name, EM950_NAME);
            break;
       
        case EM1000:
            strcpy(name, EM1000_NAME);
            break;
       
        case EM12:
            strcpy(name, EM12_NAME);
            break;
       
        case EM121:
            strcpy(name, EM121_NAME);
            break;
       
        case EM121A:
            strcpy(name, EM121A_NAME);
            break;
       
        case EM3000:
            strcpy(name, EM3000_NAME);
            break;
 
        case SEABEAM:
            strcpy(name, SEABEAM_NAME);
            break;
 
        case SEAMAP:
            strcpy(name, SEAMAP_NAME);
            break;
 
        case SASS:
            strcpy(name, SASS_NAME);
            break;
 
        case SEABAT:
            strcpy(name, SEABAT_NAME);
            break;
 
        default:
            fprintf(stdout, "Error unsupported sonar type\n");
            return (-1);
    }

    /* Load the sensor ID for this data */
    sprintf(key, "%s_SENSOR_ID", name);
    ptr = GetEnvVal(key, offF);
    if (ptr)
    {
        tptr = ptr;
        len = strlen(ptr);
        ship->sensorFileID[0] = '\0';
        for (i=0; i<len; i++)
        {
            if (*tptr == ' ')
            {
                tptr++;
            }
            else
            {
                break;
            }
        }
        if (i < len)
        {
            strncpy(ship->sensorFileID, tptr, 3);
        }
    }
    else
    {
        fprintf(stdout, "Error reading: %s\n", key);
        exit(1);
    }

    /* Load the transducer X offsets */
    sprintf(key, "%s_TRANSDUCER_X", name);
    ship->p.to_apply.transducer_offset[0].x = 0.0;
    ship->p.to_apply.transducer_offset[1].x = 0.0;
    ptr = GetEnvVal(key, offF);
    if (ptr)
    {
        if (!strstr(ptr, "NA"))
        {
            /* Need to handle more than one offset here */ 
            sscanf(ptr, "%lf,%lf",
                &ship->p.to_apply.transducer_offset[0].x,
                &ship->p.to_apply.transducer_offset[1].x);
        }
    }
    else
    {
        fprintf(stdout, "Error reading: %s\n", key);
        exit(1);
    }

    /* Load the transducer Y offsets */
    sprintf(key, "%s_TRANSDUCER_Y", name);
    ship->p.to_apply.transducer_offset[0].y = 0.0;
    ship->p.to_apply.transducer_offset[1].y = 0.0;
    ptr = GetEnvVal(key, offF);
    if (ptr)
    {
        if (!strstr(ptr, "NA"))
        {
            /* Need to handle more than one offset here */
            sscanf(ptr, "%lf,%lf",
                &ship->p.to_apply.transducer_offset[0].y, 
                &ship->p.to_apply.transducer_offset[1].y);
        }
    }
    else
    {
        fprintf(stdout, "Error reading: %s\n", key);
        exit(1);
    }

    /* Load the transducer Z offsets */
    sprintf(key, "%s_TRANSDUCER_Z", name);
    ship->p.to_apply.transducer_offset[0].z = 0.0;
    ship->p.to_apply.transducer_offset[1].z = 0.0;
    ptr = GetEnvVal(key, offF);
    if (ptr)
    {
        if (!strstr(ptr, "NA"))
        {
            /* Need to handle more than one offset here */
            sscanf(ptr, "%lf,%lf",
                &ship->p.to_apply.transducer_offset[0].z,
                &ship->p.to_apply.transducer_offset[1].z);
        }
    }
    else
    {
        fprintf(stdout, "Error reading: %s\n", key);
        exit(1);
    }

    /* Load the DRAFT for this data */
    sprintf(key, "%s_DRAFT", name);
    ship->p.to_apply.draft[0] = 0.0;
    ship->p.to_apply.draft[1] = 0.0;
    ptr = GetEnvVal(key, offF);
    if (ptr)
    {
        if (!strstr(ptr, "NA"))
        {
            /* Need to handle more than one offset here */
            sscanf(ptr, "%lf,%lf",
                &ship->p.to_apply.draft[0], 
                &ship->p.to_apply.draft[1]);
        }
    }
    else
    {
        fprintf(stdout, "Error reading: %s\n", key);
        exit(1);
    }
    return (0);
}

/*********************************************************************
 *
 * Function Name : writeSampleOffsetsFile 
 *
 * Description : This function will write a sample ship offsets file
 *   for the user to modify.  The offsets file is used by the ingest
 *   programs and by apply correctors.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions :
 *
 ********************************************************************/

void
writeSampleOffsetsFile(void)
{
    FILE *offF;

    offF = fopen("sample.off", "w");
    if (offF == (FILE *) NULL)
    {
        fprintf(stdout, "Error write sample ship offsets file \n");
        return;
    }

    /* Write the sample ship offsets file */
    fprintf(offF, "#\n");
    fprintf(offF, "# Sample ship offsets file used by ingest programs and apply correctors\n");
    fprintf(offF, "# Coordinate system is: x positive forward\n");
    fprintf(offF, "#                       y positive starboard\n");
    fprintf(offF, "#                       z positive down\n");
    fprintf(offF, "#\n");
    fprintf(offF, "PLATFORM_ID=99          # two character platform ID for file names\n");
    fprintf(offF, "EM100_SENSOR_ID=MBD     # three character sensor ID for file names\n");
    fprintf(offF, "EM100_TRANSDUCER_X=NA   # meters NA means no correction to be made\n");
    fprintf(offF, "EM100_TRANSDUCER_Y=NA   # meters\n");
    fprintf(offF, "EM100_TRANSDUCER_Z=NA   # meters\n");
    fprintf(offF, "EM100_DRAFT=NA          # meters\n");
    fprintf(offF, "EM950_SENSOR_ID=MBC     # three character sensor ID for file names\n");
    fprintf(offF, "EM950_TRANSDUCER_X=NA   # meters\n");
    fprintf(offF, "EM950_TRANSDUCER_Y=NA   # meters\n");
    fprintf(offF, "EM950_TRANSDUCER_Z=NA   # meters\n");
    fprintf(offF, "EM950_DRAFT=NA          # meters\n");
    fprintf(offF, "EM1000_SENSOR_ID=MBB    # three character sensor ID for file names\n");
    fprintf(offF, "EM1000_TRANSDUCER_X=NA  # meters\n");
    fprintf(offF, "EM1000_TRANSDUCER_Y=NA  # meters\n");
    fprintf(offF, "EM1000_TRANSDUCER_Z=NA  # meters\n");
    fprintf(offF, "EM1000_DRAFT=NA         # meters\n");
    fprintf(offF, "EM121A_SENSOR_ID=MBA    # three character sensor ID for file names\n");
    fprintf(offF, "EM121A_TRANSDUCER_X=NA  # meters\n");
    fprintf(offF, "EM121A_TRANSDUCER_Y=NA  # meters\n");
    fprintf(offF, "EM121A_TRANSDUCER_Z=NA  # meters\n");
    fprintf(offF, "EM121A_DRAFT=NA         # meters\n");
    fprintf(offF, "POSITION_OFFSET_X=0.0   # meters\n");
    fprintf(offF, "POSITION_OFFSET_Y=0.0   # meters\n");
    fprintf(offF, "POSITION_OFFSET_Z=0.0   # meters\n");
    fclose (offF);

    fprintf(stdout, "Wrote sample ship offsets file: sample.off\n");

    return;
}
