/********************************************************************
 *
 * Module Name : HMPSPARM
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmpsparm.h"

/* Macro definitions for this module */
#define MAX_PROCESS_PARAMETERS	32
#define PROCESS_PARAM_SIZE     128

/* Definitions for static global data */
static char process_params[MAX_PROCESS_PARAMETERS][PROCESS_PARAM_SIZE];

/* Prototypes for local functions */
static int numberParams(char *param);

/********************************************************************
 *
 * Function Name : getHMPSParams
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/
void
getHMPSParams(gsfRecords *rec, HMPS_PARAMETERS *p, int *numArrays)
{
    int i;
    char str[64];

    for (i=0; i<rec->process_parameters.number_parameters; i++)
    {
	if (strstr(rec->process_parameters.param[i], "REFERENCE TIME"))
	{
	    memset(p->start_of_epoch, 0, sizeof(p->start_of_epoch));
	    strncpy(p->start_of_epoch, rec->process_parameters.param[i], sizeof(p->start_of_epoch));
	}
	else if (strstr(rec->process_parameters.param[i], "ROLL_COMPENSATED"))
	{
	    if (strstr(rec->process_parameters.param[i], "YES"))
	    {
		p->roll_compensated = COMPENSATED;
	    }
	    else
	    {
		p->roll_compensated = UNCOMPENSATED;
	    }
	}
	else if (strstr(rec->process_parameters.param[i], "PITCH_COMPENSATED"))
	{
	    if (strstr(rec->process_parameters.param[i], "YES"))
	    {
		p->pitch_compensated = COMPENSATED;
	    }
	    else
	    {
		p->pitch_compensated = UNCOMPENSATED;
	    }
	}
	else if (strstr(rec->process_parameters.param[i], "HEAVE_COMPENSATED"))
	{
	    if (strstr(rec->process_parameters.param[i], "YES"))
	    {
		p->heave_compensated = COMPENSATED;
	    }
	    else
	    {
		p->heave_compensated = UNCOMPENSATED;
	    }
	}
	else if (strstr(rec->process_parameters.param[i], "TIDE_COMPENSATED"))
	{
	    if (strstr(rec->process_parameters.param[i], "YES"))
	    {
		p->tide_compensated = COMPENSATED;
	    }
	    else
	    {
		p->tide_compensated = UNCOMPENSATED;
	    }
	}
	else if (strstr(rec->process_parameters.param[i], "DEPTH_CALCULATION"))
	{
	    if (strstr(rec->process_parameters.param[i], "CORRECTED"))
	    {
		p->depth_calculation = TRUE_DEPTHS;
	    }
	    else if (strstr(rec->process_parameters.param[i], "CALCULATED_RE_1500_MS"))
	    {
		p->depth_calculation = DEPTHS_RE_1500_MS;
	    }
	}

	/* This parameter indicates whether the angle travel time
	 * pairs have been corrected for ray tracing.
	 */
	else if (strstr(rec->process_parameters.param[i], "RAY_TRACING"))
	{
	    if (strstr(rec->process_parameters.param[i], "YES"))
	    {
		p->ray_tracing = COMPENSATED;
	    }
	    else
	    {
		p->ray_tracing = UNCOMPENSATED;
	    }
	}
	else if (strstr(rec->process_parameters.param[i], "DRAFT_TO_APPLY"))
	{
	    memset(&p->to_apply.draft, 0, sizeof(p->to_apply.draft));
	    sscanf (rec->process_parameters.param[i], "DRAFT_TO_APPLY=%lf,%lf",
		&p->to_apply.draft[0], &p->to_apply.draft[1]);
	}
	else if (strstr(rec->process_parameters.param[i], "PITCH_TO_APPLY"))
	{
	    memset(&p->to_apply.pitch_bias, 0, sizeof(p->to_apply.pitch_bias));
	    sscanf (rec->process_parameters.param[i], "PITCH_TO_APPLY=%lf,%lf",
		&p->to_apply.pitch_bias[0], &p->to_apply.pitch_bias[1]);
	}
	else if (strstr(rec->process_parameters.param[i], "ROLL_TO_APPLY"))
	{
	    memset(&p->to_apply.roll_bias, 0, sizeof(p->to_apply.roll_bias));
	    sscanf (rec->process_parameters.param[i], "ROLL_TO_APPLY=%lf,%lf",
		&p->to_apply.roll_bias[0], &p->to_apply.roll_bias[1]);
	}
	else if (strstr(rec->process_parameters.param[i], "GYRO_TO_APPLY"))
	{
	    memset(&p->to_apply.gyro_bias, 0, sizeof(p->to_apply.gyro_bias));
	    sscanf (rec->process_parameters.param[i], "GYRO_TO_APPLY=%lf,%lf",
		&p->to_apply.gyro_bias[0], &p->to_apply.gyro_bias[1]);
	}
	/* The POSITION_OFFSET_TO_APPLY parameter is place holder for a new
	 * position reference offset to be applied in postprocessing. The
	 * String contains an x,y,z triplet which locates the vessel's
	 * navigation reference point.
	 */
	else if (strstr(rec->process_parameters.param[i], "POSITION_OFFSET_TO_APPLY"))
	{
	    memset (&p->to_apply.position_offset, 0, sizeof(p->to_apply.position_offset));
	    sscanf (rec->process_parameters.param[i], "POSITION_OFFSET_TO_APPLY=%lf,%lf,%lf",
		&p->to_apply.position_offset.x,
		&p->to_apply.position_offset.y,
		&p->to_apply.position_offset.z);
	}
	else if (strstr(rec->process_parameters.param[i], "TRANSDUCER_OFFSET_TO_APPLY"))
	{
	    memset (&p->to_apply.transducer_offset, 0, sizeof(p->to_apply.transducer_offset));
	    sscanf (rec->process_parameters.param[i], "TRANSDUCER_OFFSET_TO_APPLY=%lf,%lf,%lf,%lf,%lf,%lf",
		&p->to_apply.transducer_offset[0].x,
		&p->to_apply.transducer_offset[0].y,
		&p->to_apply.transducer_offset[0].z,
		&p->to_apply.transducer_offset[1].x,
		&p->to_apply.transducer_offset[1].y,
		&p->to_apply.transducer_offset[1].z);
	}
	else if (strstr(rec->process_parameters.param[i], "APPLIED_DRAFT"))
	{
	    memset (&p->applied.draft, 0, sizeof(p->applied.draft));
	    sscanf (rec->process_parameters.param[i], "APPLIED_DRAFT=%lf,%lf",
		&p->applied.draft[0], &p->applied.draft[1]);
	    /* Use this parameter to decide how many Transmit/Receive array
	     * pairs this data has.
	     */
	    *numArrays = numberParams(rec->process_parameters.param[i]);
	}
	else if (strstr(rec->process_parameters.param[i], "APPLIED_PITCH_BIAS"))
	{
	    memset (&p->applied.pitch_bias, 0, sizeof(p->applied.pitch_bias));
	    sscanf (rec->process_parameters.param[i], "APPLIED_PITCH_BIAS=%lf,%lf",
		&p->applied.pitch_bias[0], &p->applied.pitch_bias[1]);
	}
	else if (strstr(rec->process_parameters.param[i], "APPLIED_ROLL_BIAS"))
	{
	    memset (&p->applied.roll_bias, 0, sizeof(p->applied.roll_bias));
	    sscanf (rec->process_parameters.param[i], "APPLIED_ROLL_BIAS=%lf,%lf",
		&p->applied.roll_bias[0], &p->applied.roll_bias[1]);
	}
	else if (strstr(rec->process_parameters.param[i], "APPLIED_GYRO_BIAS"))
	{
	    memset (&p->applied.gyro_bias, 0, sizeof(p->applied.gyro_bias));
	    sscanf (rec->process_parameters.param[i], "APPLIED_GYRO_BIAS=%lf,%lf",
		&p->applied.gyro_bias[0], &p->applied.gyro_bias[1]);
	}
	/* The APPLIED_POSITION_OFFSET parameter defines the x,y,z position in
	 * ship coordinates to which the lat lons are relative.
	 */
	else if (strstr(rec->process_parameters.param[i], "APPLIED_POSITION_OFFSET"))
	{
	    memset (&p->to_apply.position_offset, 0, sizeof(p->applied.position_offset));
	    sscanf (rec->process_parameters.param[i], "APPLIED_POSITION_OFFSET=%lf,%lf,%lf",
		&p->applied.position_offset.x,
		&p->applied.position_offset.y,
		&p->applied.position_offset.z);
	}
	/* The APPLIED_TRANSDUCER_OFFSET parameter defines the x,y,z offsets
	 * in ship coordinates to which have been applied to refer the x,y,z
	 * beam values to the ship reference point.
	 */
	else if (strstr(rec->process_parameters.param[i], "APPLIED_TRANSDUCER_OFFSET"))
	{
	    memset (&p->to_apply.position_offset, 0, sizeof(p->applied.transducer_offset));
	    sscanf (rec->process_parameters.param[i], "APPLIED_TRANSDUCER_OFFSET=%lf,%lf,%lf,%lf,%lf,%lf",
		&p->applied.transducer_offset[0].x,
		&p->applied.transducer_offset[0].y,
		&p->applied.transducer_offset[0].z,
		&p->applied.transducer_offset[1].x,
		&p->applied.transducer_offset[1].y,
		&p->applied.transducer_offset[1].z);
	}
	/* The GEOID parameter defines the geoid on which the latitude
	 * and longitude values are referenced.
	 */
	else if (strstr(rec->process_parameters.param[i], "GEOID"))
	{
	    sscanf (rec->process_parameters.param[i], "GEOID=%s", str);
            if (strstr(str, "WGS-84"))
            {
                p->geoid = WGS_1984;
            }
            else if (strstr(str, "WGS-72")) 
            {
                p->geoid = WGS_1972;
            }
            else if (strstr(str, "WGS-66")) 
            {
                p->geoid = WGS_1966;
            }
            else if (strstr(str, "CLK-66")) 
            {
                p->geoid = CLARK_1866;
            }
            else if (strstr(str, "CLK-80")) 
            {
                p->geoid = CLARK_1880;
            }
            else if (strstr(str, "EVT-30")) 
            {
                p->geoid = EVEREST_1830;
            }
            else if (strstr(str, "BSL-41")) 
            {
                p->geoid = BESSEL_1841;
            }
            else if (strstr(str, "INT-09")) 
            {
                p->geoid = INTERNATIONAL_1909;
            }
            else if (strstr(str, "AUS-NT")) 
            {
                p->geoid = INTERNATIONAL_1909;
            }
            else if (strstr(str, "AIRY")) 
            {
                p->geoid = AIRY;
            }
            else if (strstr(str, "FSH-60")) 
            {
                p->geoid = FISCHER_1960;
            }
            else if (strstr(str, "FSH-68")) 
            {
                p->geoid = FISCHER_1968;
            }
	}

	/* The TIDAL_DATUM paremeter defines the reference datum for tide
	 * corrections
	 */
	else if (strstr(rec->process_parameters.param[i], "TIDAL_DATUM"))
	{
	    sscanf (rec->process_parameters.param[i], "TIDAL_DATUM=%s",
		str);
    
            if (strstr(str, "MLLW"))
            {
                p->tidal_datum = MLLW;
            } 
            else if (strstr(str, "MLW"))
            {
                p->tidal_datum = MLW;
            }
            else
            {
                p->tidal_datum = UNKNOWN_DATUM;
            }
        }
    }
             
    return;
}

/********************************************************************
 *
 * Function Name : putHMPSParams
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/
void
putHMPSParams(HMPS_PARAMETERS *p, gsfRecords *rec, int numArrays)
{
    /* Load the text descriptor for the beginning of time */
    sprintf(process_params[0], "REFERENCE TIME=1970/001 00:00:00");
    rec->process_parameters.param[0] = process_params[0];
    rec->process_parameters.param_size[0] = PROCESS_PARAM_SIZE;

    /* This parameter indicates whether the data has been roll compensated */
    if (p->roll_compensated == COMPENSATED)
    {
	sprintf(process_params[1], "ROLL_COMPENSATED=YES");
    }
    else
    {
	sprintf(process_params[1], "ROLL_COMPENSATED=NO ");
    }
    rec->process_parameters.param[1] = process_params[1];
    rec->process_parameters.param_size[1] = PROCESS_PARAM_SIZE;

    /* This parameter indicates whether the data has been pitch compensated */
    if (p->pitch_compensated == COMPENSATED)
    {
	sprintf(process_params[2], "PITCH_COMPENSATED=YES");
    }
    else
    {
	sprintf(process_params[2], "PITCH_COMPENSATED=NO ");
    }
    rec->process_parameters.param[2] = process_params[2];
    rec->process_parameters.param_size[2] = PROCESS_PARAM_SIZE;

    /* This parameter indicates whether the data has been heave compensated */
    if (p->heave_compensated == COMPENSATED)
    {
	sprintf(process_params[3], "HEAVE_COMPENSATED=YES");
    }
    else
    {
	sprintf(process_params[3], "HEAVE_COMPENSATED=NO ");
    }
    rec->process_parameters.param[3] = process_params[3];
    rec->process_parameters.param_size[3] = PROCESS_PARAM_SIZE;

    /* This parameter indicates whether the data has been tide compensated */
    if (p->tide_compensated == COMPENSATED)
    {
	sprintf(process_params[4], "TIDE_COMPENSATED=YES");
    }
    else
    {
	sprintf(process_params[4], "TIDE_COMPENSATED=NO ");
    }
    rec->process_parameters.param[4] = process_params[4];
    rec->process_parameters.param_size[4] = PROCESS_PARAM_SIZE;

    /* The depth field of the swath bathy ping data structure is true depth,
     * meaning depth is computed by indegrating travel time through the sound
     * speed profile. As opposed to DEPTH CALCULATION=UNCORRECTED
     */
    if (p->depth_calculation == TRUE_DEPTHS)
    {
	sprintf(process_params[5], "DEPTH_CALCULATION=CORRECTED");
    }
    else if (p->depth_calculation == DEPTHS_RE_1500_MS)
    {
	sprintf(process_params[5], "DEPTH_CALCULATION=RELATIVE_TO_1500_MS");
    }
    else
    {
	sprintf(process_params[5], "DEPTH_CALCULATION=UNKNOWN");
    }
    rec->process_parameters.param[5] = process_params[5];
    rec->process_parameters.param_size[5] = PROCESS_PARAM_SIZE;

    /* This parameter indicates whether the angle travel time pairs have been
     * corrected for ray tracing.
     */
    if (p->ray_tracing == COMPENSATED)
    {
	sprintf(process_params[6], "RAY_TRACING=YES");
    }
    else
    {
	sprintf(process_params[6], "RAY_TRACING=NO");
    }
    rec->process_parameters.param[6] = process_params[6];
    rec->process_parameters.param_size[6] = PROCESS_PARAM_SIZE;

    /* The DRAFT_TO_APPLY parameter is a place holder for a new draft
     * value to be applied in postprocessing.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[7], "DRAFT_TO_APPLY=%+06.2f,%+06.2f",
	    p->to_apply.draft[0], p->to_apply.draft[1]);
    }
    else if (numArrays == 1)
    {
	sprintf(process_params[7], "DRAFT_TO_APPLY=%+06.2f",
	    p->to_apply.draft[0]);
    }
    rec->process_parameters.param[7] = process_params[7];
    rec->process_parameters.param_size[7] = PROCESS_PARAM_SIZE;

    /* The PITCH_BIAS_TO_APPLY parameter is place holder for a
     * picth bias value to be applied in postprocessing.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[8], "PITCH_TO_APPLY=%+06.2f,%+06.2f",
	    p->to_apply.pitch_bias[0], p->to_apply.pitch_bias[1]);
    }
    else if (numArrays == 1)
    {
	sprintf(process_params[8], "PITCH_TO_APPLY=%+06.2f",
	    p->to_apply.pitch_bias[0]);
    }
    rec->process_parameters.param[8] = process_params[8];
    rec->process_parameters.param_size[8] = PROCESS_PARAM_SIZE;

    /* The ROLL_BIAS_TO_APPLY parameter is place holder for a new roll bias value
     * to be applied in postprocessing.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[9], "ROLL_TO_APPLY=%+06.2f,%+06.2f",
	    p->to_apply.roll_bias[0], p->to_apply.roll_bias[1]);
    }
    else
    {
	sprintf(process_params[9], "ROLL_TO_APPLY=%+06.2f",
	    p->to_apply.roll_bias[0]);
    }
    rec->process_parameters.param[9] = process_params[9];
    rec->process_parameters.param_size[9] = PROCESS_PARAM_SIZE;

    /* The GYRO_BIAS_TO_APPLY parameter is place holder for a new gyro value
     * to be applied in postprocessing.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[10], "GYRO_TO_APPLY=%+06.2f,%+06.2f",
	    p->to_apply.gyro_bias[0], p->to_apply.gyro_bias[1]);
    }
    else
    {
	sprintf(process_params[10], "GYRO_TO_APPLY=%+06.2f",
	    p->to_apply.gyro_bias[0]);
    }
    rec->process_parameters.param[10] = process_params[10];
    rec->process_parameters.param_size[10] = PROCESS_PARAM_SIZE;

    /* The POSITION_OFFSET_TO_APPLY parameter is place holder for a new
     * position reference offset to be applied in postprocessing.
     */
    sprintf(process_params[11], "POSITION_OFFSET_TO_APPLY=%+06.2f,%+06.2f,%+06.2f",
	p->to_apply.position_offset.x,
	p->to_apply.position_offset.y,
	p->to_apply.position_offset.z);
    rec->process_parameters.param[11] = process_params[11];
    rec->process_parameters.param_size[11] = PROCESS_PARAM_SIZE;

    /* The TRANSDUCER_OFFSET_TO_APPLY parameter is place holder for a new
     * transduer reference offset to be applied in postprocessing.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[12], "TRANSDUCER_OFFSET_TO_APPLY=%+06.2f,%+06.2f,%+06.2f,%+06.2f,%+06.2f,%+06.2f",
	   p->to_apply.transducer_offset[0].x,
	   p->to_apply.transducer_offset[0].y,
	   p->to_apply.transducer_offset[0].z,
	   p->to_apply.transducer_offset[1].x,
	   p->to_apply.transducer_offset[1].y,
	   p->to_apply.transducer_offset[1].z);
    }
    else if (numArrays == 1)
    {
	sprintf(process_params[12], "TRANSDUCER_OFFSET_TO_APPLY=%+06.2f,%+06.2f,%+06.2f",
	   p->to_apply.transducer_offset[0].x,
	   p->to_apply.transducer_offset[0].y,
	   p->to_apply.transducer_offset[0].z);
    }
    rec->process_parameters.param[12] = process_params[12];
    rec->process_parameters.param_size[12] = PROCESS_PARAM_SIZE;

    /* The APPLIED_DRAFT parameter defines the transducer draft value
     * previously applied to the depths.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[13], "APPLIED_DRAFT=%+06.2f,%+06.2f",
	    p->applied.draft[0], p->applied.draft[1]);
    }
    else if (numArrays == 1)
    {
	sprintf(process_params[13], "APPLIED_DRAFT=%+06.2f",
	    p->applied.draft[0]);
    }
    rec->process_parameters.param[13] = process_params[13];
    rec->process_parameters.param_size[13] = PROCESS_PARAM_SIZE;

    /* The APPLIED_PITCH_BIAS parameter defines the pitch bias previously
     * applied.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[14], "APPLIED_PITCH_BIAS=%+06.2f,%+06.2f",
	    p->applied.pitch_bias[0], p->applied.pitch_bias[1]);
    }
    else if (numArrays == 1)
    {
	sprintf(process_params[14], "APPLIED_PITCH_BIAS=%+06.2f",
	    p->applied.pitch_bias[0]);
    }
    rec->process_parameters.param[14] = process_params[14];
    rec->process_parameters.param_size[14] = PROCESS_PARAM_SIZE;

    /* The APPLIED_ROLL_BIAS parameter defines the roll bias previously
     * applied to the data.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[15], "APPLIED_ROLL_BIAS=%+06.2f,%+06.2f",
	    p->applied.roll_bias[0], p->applied.roll_bias[1]);
    }
    else if (numArrays == 1)
    {
	sprintf(process_params[15], "APPLIED_ROLL_BIAS=%+06.2f",
	    p->applied.roll_bias[0]);
    }
    rec->process_parameters.param[15] = process_params[15];
    rec->process_parameters.param_size[15] = PROCESS_PARAM_SIZE;

    /* The APPLIED_GYRO_BIAS parameter defines the gyro bias previously
     * applied to the data.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[16], "APPLIED_GYRO_BIAS=%+06.2f,%+06.2f",
	    p->applied.gyro_bias[0], p->applied.gyro_bias[1]);
    }
    else if (numArrays == 1)
    {
	sprintf(process_params[16], "APPLIED_GYRO_BIAS=%+06.2f",
	    p->applied.gyro_bias[0]);
    }
    rec->process_parameters.param[16] = process_params[16];
    rec->process_parameters.param_size[16] = PROCESS_PARAM_SIZE;

    /* The APPLIED_POSITION_OFFSET parameter defines the x,y,z position in
     * ship coordinates to which the lat lons are relative.
     */
    sprintf(process_params[17], "APPLIED_POSITION_OFFSET=%+06.2f,%+06.2f,%+06.2f",
	p->applied.position_offset.x,
	p->applied.position_offset.y,
	p->applied.position_offset.z);
    rec->process_parameters.param[17] = process_params[17];
    rec->process_parameters.param_size[17] = PROCESS_PARAM_SIZE;

    /* The APPLIED_TRANSDUCER_OFFSET parameter defines the x,y,z offsets
     * in ship coordinates to which have been applied to refer the x,y,z
     * beam values to the ship reference point.
     */
    if (numArrays == 2)
    {
	sprintf(process_params[18], "APPLIED_TRANSDUCER_OFFSET=%+06.2f,%+06.2f,%+06.2f,%+06.2f,%+06.2f,%+06.2f",
	    p->applied.transducer_offset[0].x,
	    p->applied.transducer_offset[0].y,
	    p->applied.transducer_offset[0].z,
	    p->applied.transducer_offset[1].x,
	    p->applied.transducer_offset[1].y,
	    p->applied.transducer_offset[1].z);
    }
    else if (numArrays == 1)
    {
	sprintf(process_params[18], "APPLIED_TRANSDUCER_OFFSET=%+06.2f,%+06.2f,%+06.2f",
	    p->applied.transducer_offset[0].x,
	    p->applied.transducer_offset[0].y,
	    p->applied.transducer_offset[0].z);
    }
    rec->process_parameters.param[18] = process_params[18];
    rec->process_parameters.param_size[18] = PROCESS_PARAM_SIZE;

    /* The GEOID parameter defines the geoid on which the latitude longitude
     * values are referenced.
     */
    switch (p->geoid)
    {
        case (WGS_1984):
            sprintf(process_params[19], "GEOID=WGS-84");
            break;

        case (WGS_1972):
            sprintf(process_params[19], "GEOID=WGS-72");
            break;

        case (WGS_1966):
            sprintf(process_params[19], "GEOID=WGS-66");
            break;

        case (CLARK_1866):
            sprintf(process_params[19], "GEOID=CLK-66");
            break;

        case (CLARK_1880):
            sprintf(process_params[19], "GEOID=CLK-80");
            break;

        case (EVEREST_1830):
            sprintf(process_params[19], "GEOID=EVT-30");
            break;

        case (BESSEL_1841):
            sprintf(process_params[19], "GEOID=BSL-41");
            break;

        case (INTERNATIONAL_1909):
            sprintf(process_params[19], "GEOID=INT-09");
            break;

        case (AUSTRALIAN_NATIONAL):
            sprintf(process_params[19], "GEOID=AUS-NT");
            break;

        case (AIRY):
            sprintf(process_params[19], "GEOID=AIRY  ");
            break;

        case (FISCHER_1960):
            sprintf(process_params[19], "GEOID=FSH-60");
            break;

        case (FISCHER_1968):
            sprintf(process_params[19], "GEOID=FSH-68");
            break;

        default:
            sprintf(process_params[19], "GEOID=UNKNWN");
            break;
        
    } 
    rec->process_parameters.param[19] = process_params[19];
    rec->process_parameters.param_size[19] = PROCESS_PARAM_SIZE;

    /* The TIDAL_DATUM paremeter defines the reference datum for tide
     * corrections
     */
    switch (p->tidal_datum)
    {
        case (MLLW):
            sprintf(process_params[20], "TIDAL_DATUM=MLLW   ");
            break;

        case (MLW):
            sprintf(process_params[20], "TIDAL_DATUM=MLW    ");
            break;

        default:
            sprintf(process_params[20], "TIDAL_DATUM=UNKNOWN");
            break;
    }
             
    rec->process_parameters.param[20] = process_params[20];
    rec->process_parameters.param_size[20] = PROCESS_PARAM_SIZE;

    rec->process_parameters.number_parameters = 21;

    return;
}

/********************************************************************
 *
 * Function Name : numberParams
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/
static int
numberParams(char *param)
{
    int number;
    char *p;
    char tmp[128];

    strncpy (tmp, param, sizeof(tmp));
    p = strtok (tmp, ",");

    if (p == NULL)
    {
	return (0);
    }
    else
    {
	number = 1;
    }

    while ((p=strtok(NULL, ",")))
    {
	number++;
    }

    return (number);
}
