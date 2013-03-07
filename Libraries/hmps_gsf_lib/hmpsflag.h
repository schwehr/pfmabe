/********************************************************************
 *
 * Module Name : HMPSFLAGS
 *
 * Author/Date : J. S. Byrne / 13 Mar 1995
 *
 * Description : This file contains the ping and beam bit flag definitions
 *    used by the HMPS processing software when dealing with GSF data files.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * DHG  2004/08/18 Assigned "GSF_PING_USER_FLAG_12" to indicate that delayed heave
 *                 was applied to each ping where it is set, and added the
 *                 prototype for "setHMPSDelayedHeaveFlag"
 * DHG  2005/12/07 Added the "HMPS_IGNORE_TPE_EXCEEDS_IHO" macro definition to the
 *                 beam flags.
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/
#ifndef     _HMPS_FLAGS_
    #define _HMPS_FLAGS_

    #include "gsf.h"

    /* There are 16 bits for the ping flags.  The definitions follow: */

    /* If BAD_PING is set set and nothing else is set then the whole ping
     * had no bottom detections made, other wise decode the other bit(s) set
     * to determine why this is a bad ping.
     */
    #define HMPS_IGNORE_PING            GSF_IGNORE_PING
    #define HMPS_OFF_LINE_PING          GSF_PING_USER_FLAG_01
    #define HMPS_BAD_TIME               GSF_PING_USER_FLAG_02
    #define HMPS_BAD_POSITION           GSF_PING_USER_FLAG_03
    #define HMPS_BAD_HEADING            GSF_PING_USER_FLAG_04
    #define HMPS_BAD_ROLL               GSF_PING_USER_FLAG_05
    #define HMPS_BAD_PITCH              GSF_PING_USER_FLAG_06
    #define HMPS_BAD_HEAVE              GSF_PING_USER_FLAG_07
    #define HMPS_BAD_DEPTH_CORRECTOR    GSF_PING_USER_FLAG_08
    #define HMPS_BAD_TIDE_CORRECTOR     GSF_PING_USER_FLAG_09
    #define HMPS_BAD_SVP                GSF_PING_USER_FLAG_10
    #define HMPS_NO_POSITION            GSF_PING_USER_FLAG_11
    #define HMPS_DELAYED_HEAVE_APPLIED  GSF_PING_USER_FLAG_12  /* DHG Indicates that the delayed heave correction was applied */
    #define HMPS_GPS_VERTICAL_CONTROL   GSF_PING_USER_FLAG_13

    #define HMPS_BAD_PING_TEST          (unsigned)(0x0002 | 0x0004 | 0x0008 | 0x0010 | 0x0020 | 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800)

   /*
    * GSF_PING_USER_FLAG_14 and GSF_PING_USER_FLAG_15 are used to describe the
    *  tide correctors that have been applied to a ping.  The setHMPSTideFlag()
    *  and testHMPSTideFlag() functions are used to modify and test these flags,
    *  which have the following meanings:
    *
    *   Bit: 15  14    Tide Corrector Type
    *       +-------+  -------------------
    *       | 0 | 0 |    Unknown
    *       | 0 | 1 |    Predicted
    *       | 1 | 0 |    Observed Preliminary
    *       | 1 | 1 |    Observed Verified
    *       +-------+
    */
     #define HMPS_UNKNOWN_TC            1
     #define HMPS_PREDICTED_TC          2
     #define HMPS_OBSERVED_PRELIM_TC    3
     #define HMPS_OBSERVED_VERIFIED_TC  4
     #define HMPS_ERS_TC                5



    /* There are 8 bits for each beam for the beam flags. The definitions follow: */

    /* The eight bit beam flag value is divided into two four bit
     * fields. The lower order four bits are used to specify that
     * a beam be ignored, where the value specifies the reason the
     * the beam is to be ignored. The higher order four bits are
     * usedto specify that a beam is selected, where the value specifies
     * the reason why the beam is selected.
     *
     *------------------------------------------------------------
     *
     * Category 00 (HMPS_INFORMATIONAL)
     * xxx1xx00 => This beam does not meet one times IHO accuracy
     *
     * Category 01 (HMPS_IGNORE)
     * 00000001 => This beam is to be ignored, no detection was made by the sonar
     * xxxxx101 => This beam is to be ignored, it has been manually edited
     * xxxx1x01 => This beam is to be ignored, it has been filter edited
     * xx1xxx01 => This beam is to be ignored, since it exceeds two times IHO
     * x1xxxx01 => This beam is to be ignored, since it exceeds the maximum footprint
     * 1xxxxx00 => This beam is to be ignored, since the horizontal error or vertical
     *             error tolerance  is exceeded
     *
     * Category 10 (HMPS_SELECT)
     * 00000010 => Selected sounding, no reason specified
     * xxxxx110 => Selected sounding, it is a least depth
     * xxxx1x10 => Selected sounding, it is a maximum depth
     * xx1xxx10 => Selected sounding, it has been identified as a side scan contact
     * x1xxxx10 => Selected sounding, spare ....
     * 1xxxxx10 => Selected sounding, spare ....
     */

     /* Definitions for the categories */
     #define HMPS_INFORMATIONAL      1
     #define HMPS_IGNORE             2
     #define HMPS_SELECT             3

     /* Definitions for HMPS_INFORMATIONAL category */
     #define HMPS_INFO_NOT_1X_IHO           0x10
     #define HMPS_INFO_BIT_MASK             0x10

     /* Definitions for HMPS_IGNORE category */
     #define HMPS_IGNORE_NULL_BEAM          0x01  /* 0x00 & 0x01 */
     #define HMPS_IGNORE_MANUALLY_EDITED    0x05  /* 0x04 & 0x01 */
     #define HMPS_IGNORE_FILTER_EDITED      0x09  /* 0x08 & 0x01 */
     #define HMPS_IGNORE_NOT_2X_IHO         0x21  /* 0x20 & 0x01 */
     #define HMPS_IGNORE_FOOTPRINT_TOO_BIG  0x41  /* 0x40 & 0x01 */
     #define HMPS_IGNORE_TPE_EXCEEDS_IHO    0x81  /* 0x80 & 0x01 */   /*DHG 2005/12/07 */
     #define HMPS_IGNORE_BIT_MASK           0xec  /* combination of reason bits */

     /* Definitions for the HMPS_SELECT category */
     #define HMPS_SELECTED                  0x02  /* 0x00 & 0x02 */
     #define HMPS_SELECTED_LEAST            0x06  /* 0x04 & 0x02 */
     #define HMPS_SELECTED_MAXIMUM          0x0a  /* 0x08 & 0x02 */
     #define HMPS_SELECTED_CONTACT          0x22  /* 0x20 & 0x02 */
     #define HMPS_SELECTED_SPARE_1          0x42  /* 0x40 & 0x02 */
     #define HMPS_SELECTED_DESIGNATED       0x82  /* 0x80 & 0x02 */

     int testHMPSBeamFlag(gsfSwathBathyPing *p, int beam, int flag);
     int setHMPSBeamFlag(gsfSwathBathyPing *p, int beam, int flag);
     int clearHMPSBeamFlag(gsfSwathBathyPing *p, int beam, int category, int flag);

     int testHMPSTideFlag(gsfSwathBathyPing *p, int flag);
     int setHMPSTideFlag(gsfSwathBathyPing *p, int flag);

     int setHMPSDelayedHeaveFlag (gsfSwathBathyPing *p, int flag);   /*DHG 1 = delayed heave applied, 0 = not */
     int setHMPS_GPSZFlag(gsfSwathBathyPing *p, int flag);
     int setHMPS_DSFlag(gsfSwathBathyPing *p, int beam, int flag);
#endif
