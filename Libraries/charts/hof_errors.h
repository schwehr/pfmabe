#include "nvtypes.h"
#include "nvdef.h"

/*****************************************************************************
 * hof_errors.h   Header
 *
 * Purpose:       This header file holds the horizontal and vertical error 
 *                constants.  These constants are based on information from
 *                Paul LaRoque at Optech, Toronto (29 March 2005).  More 
 *                information is available in the spreadsheet file,
 *                CHARTS_Hydro_Topo_Accuracies_29_Mar_2005.xls
 *          
 *
 * Revision History:
 *
 ****************************************************************************/

#ifndef __HOF_ERRORS_H__
#define __HOF_ERRORS_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "charts.h"


/*  Horizontal Error Constants  */

/**************************************************************************/

/*  CHARTS System Errors  */

#define E_HEIGHT                        0.1
#define E_ROLL                          0.01
#define E_PITCH                         0.01
#define E_YAW                           0.02
#define E_SCAN_ANGLE                    0.02
#define E_ANTENNA                       0.05
#define E_H_CALIBRATION                 0.01
#define E_LASER_POINTING                0.02


/*  Water Parameter Errors  */

#define E_PROPAGATION                   0.015

/****************************************************************************/




/*  Vertical Error Constants  */

/**************************************************************************/

/*  Constant Bias  */

#define E_CONSTANT_BIAS                 0.13


/*  Random Errors  */

#define E_ALTIMETER_TIM                 0.03
#define E_CFD                           0.05
#define E_LOG_AMP_DELAY                 0.01
#define E_WAVE_HEIGHT                   0.10
#define E_PULSE_LOCATION                0.10
#define E_ELLIPSOID_TO_LASER_KGPS       0.10

#define E_ALTIMETER_TIM_2               E_ALTIMETER_TIM * E_ALTIMETER_TIM
#define E_CFD_2                         E_CFD * E_CFD
#define E_LOG_AMP_DELAY_2               E_LOG_AMP_DELAY * E_LOG_AMP_DELAY
#define E_WAVE_HEIGHT_2                 E_WAVE_HEIGHT * E_WAVE_HEIGHT
#define E_PULSE_LOCATION_2              E_PULSE_LOCATION * E_PULSE_LOCATION
#define E_ELLIPSOID_TO_LASER_KGPS_2     E_ELLIPSOID_TO_LASER_KGPS * E_ELLIPSOID_TO_LASER_KGPS


/*  Bias Errors  */

#define E_THERMAL                       0.01
#define E_V_CALIBRATION                 0.02
#define E_PMT_DELAY                     0.01
#define E_SURFACE_ORIGIN                0.05

#define E_THERMAL_2                     E_THERMAL * E_THERMAL
#define E_V_CALIBRATION_2               E_V_CALIBRATION * E_V_CALIBRATION
#define E_PMT_DELAY_2                   E_PMT_DELAY * E_PMT_DELAY
#define E_SURFACE_ORIGIN_2              E_SURFACE_ORIGIN * E_SURFACE_ORIGIN



#ifdef  __cplusplus
}
#endif


#endif
