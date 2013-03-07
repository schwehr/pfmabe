/********************************************************************
 *
 * Module Name : HMPSPARAM
 *
 * Author/Date : J. S. Byrne / 23 Jul 1995
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who	when	  what
 * ---	----	  ----
 *
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/
#ifndef _HMPS_PARAM_
#define _HMPS_PARAM_

#ifdef  __cplusplus
extern "C" {
#endif


#include "gsf.h"

/* Macro definitions for this module */
#define MAX_TXRX_PAIRS    2
#define COMPENSATED       1
#define UNCOMPENSATED     0
#define TRUE_DEPTHS       1
#define DEPTHS_RE_1500_MS 2

/* Macro definitions for known geoids 
 * These are here to record with the data the geoid to which latitude and longitude
 * are relative.  In general, ingest applications are storing gsf data with latitude
 * and longitude relative to the WGS-84 geoid.
 */
#define UNKNOWN_GEOID        0
#define WGS_1984             1
#define WGS_1972             2
#define WGS_1966             3
#define CLARK_1866           4
#define CLARK_1880           5
#define EVEREST_1830         6 
#define BESSEL_1841          7 
#define INTERNATIONAL_1909   8 
#define AUSTRALIAN_NATIONAL  9  
#define AIRY                10 
#define FISCHER_1960        11 
#define FISCHER_1968        12 

/* Macro definitions for tidal datums */
#define UNKNOWN_DATUM        0 
#define MLLW                 1     /* Mean Lower Low Water */
#define MLW                  2     /* Mean Low Water */

/* Define a data structure to hold the position offsets */
typedef struct t_hmps_offsets
{
   double x;    /* meters, positive forward */
   double y;    /* meters, positive starboard */  
   double z;    /* meters, positive down */
} HMPS_OFFSETS;

/* Define a data structure of correctors we need to track */
typedef struct t_hmps_correctors
{
    double	     draft[MAX_TXRX_PAIRS];          /* meters */
    double	     roll_bias[MAX_TXRX_PAIRS];      /* degrees */
    double	     pitch_bias[MAX_TXRX_PAIRS];     /* degrees */
    double	     gyro_bias[MAX_TXRX_PAIRS];      /* degrees */
    HMPS_OFFSETS position_offset;          
    HMPS_OFFSETS transducer_offset[MAX_TXRX_PAIRS];
} HMPS_CORRECTORS;

/* Define a data structure to store the parameters in internal form */
typedef struct t_hmps_params
{
    /* These text parameters define parameter reference points */
    char start_of_epoch[64];
    int geoid;
    int tidal_datum;

    /* These parameters define correctors that need to be applied */
    int roll_compensated;	/* has the data been corrected for roll */
    int pitch_compensated;	/* has the data been corrected for pitch */
    int heave_compensated;	/* has the data been corrected for heave */
    int tide_compensated;	/* has the data been corrected for tide */
    int depth_calculation;	/* is the depth field true depth or relative to a fixed sound speed */
    int ray_tracing;	        /* are angle/travel time pairs compensated for ray tracing */

    /* These parameters define the values that need to be applied */
    HMPS_CORRECTORS to_apply;

    /* These parameters define the valued that have already been applied */
    HMPS_CORRECTORS applied;
} HMPS_PARAMETERS;

void getHMPSParams(gsfRecords *rec, HMPS_PARAMETERS *p, int *numArrays);
/* getHMPSParams loads an HMPS_PARAMETERS data structure, given a
 * gsfRecords data structure with a populated gsfProcessingParameters
 * data structure.
 */

void putHMPSParams(HMPS_PARAMETERS *p, gsfRecords *rec, int numArrays);
/* putHMPSParams loads the gsfProcessingParameters substructure of a
 * gsfRecords data structure given a populated HMPS_PARAMETERS data
 * structure.
*/


#ifdef  __cplusplus
}
#endif


#endif

