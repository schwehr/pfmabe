/*****************************************************************************
 * warnings.h 	Header
 *
 * Purpose:  	This header file hold the definitions of all the Automated
 *		Processing warnings names and values associated with them.
 *              
 *
 * Revision History:
 *...
 * 
 * 98/11/03 DR	-added AU2_INTERLEAVE_FLAG (39) and AU2_TIM1_FLAG (40)
 * 
 * 98/11/11 DR	-AU2_DATA_OFFSET_ZERO_FLAG
 * 
 * 98/11/25 DR	-added 3 new SCANNER flags (42-44)
 * 
 * 98/12/09 DR	-added 2 new SCANNER flags (45-46)
 *
 * 99/01/21 DR	-changed NO_IR_SFC_FLAG to NO_HARD_OR_SOFT_SFC_FLAG
 *		-added 2 new flags STRETCHED_IR_FLAG and 
 *		 STRETCHED_RAMAN_FLAG (47 and 48)
 *
 * 99/06/11 DR	-added 2 new flags; GPS and INS (49, 50).  They will be
 *		set when there are too many missing records OR the 
 *		INS/GPS files end too soon (before shots) OR starts too late
 *		(after shots strart)
 *
 * 99/11/09 DR	-shorted a bunch of the warning names (the strings...) 
 *
 * 99/11/16 DR	-added new warn AU2_WAVEHEIGHT_FLAG 
 *
 * 99/12/03 DR	-brought up to date with 2.30 unix version.
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


#define  AU_NUM_WARN_TYPES 			51

#define  AU_INTERPOLATED_FLAG			1 
#define  AU_NADIR_ANGLE_FLAG			2 
#define	 AU_LAND_FLAG				3 
#define  AU_SFC_TRACKER_BUFFER_FLAG		4 
#define  AU_ST_SURFING_FLAG			5 
#define  AU_ST_JERK_FLAG			6 
#define  AU_STRETCHED_BOT_FLAG			7 
#define  AU_GREEN_GREEN_DEPTH_DIF_FLAG		8 
#define  AU_LATE_HDWR_SFC_FLAG			9 
#define	 AU_SATURATED_APD_BOT_FLAG		10 
#define  AU_NO_HARD_OR_SOFT_SFC_FLAG		11 
#define  AU_EARLY_HDWR_SFC_FLAG			12 
#define  AU_NOISY_HEIGHT_FLAG			13 
#define  AU_SOFT_SFC_DIF_12_FLAG 		14 
#define  AU_SOFT_SFC_DIF_13_FLAG		15
#define  AU_SOFT_SFC_DIF_23_FLAG		16 
#define  AU_HARD_SOFT_SFC_DIF_FLAG		17 
#define  AU_PEAK_FIFTY_ERROR_FLAG		18 
#define  AU_ACCEL_BIAS_CHANGE_FLAG		19 
#define	 AU_RAMAN_BOTTOM_FLAG			20 
#define  AU_GLINT_ERROR_FLAG			21 
#define  AU_LAND_AND_RAMAN_FLAG			22 
#define  AU_NO_LAND_NO_RAMAN_FLAG		23 
#define  AU_STRETCHED_SFC_FLAG			24
#define  AU_SATURATED_SFC_FLAG			25
#define  AU_NO_SFC_DATA_FLAG			26 
#define  AU_SHOAL_PEAK_FLAG			27
#define  AU_LESSER_PULSE_SELECTED_FLAG		28
#define	 AU_NO_OTF_DATUM_ZONE_FLAG		29
#define  AU_OTF_ALT_FLAG			30

#define  AU2_NO_TIDE_ZONE_FLAG			31
#define  AU2_DLESSONE_FLAG			32
#define  AU2_D2LESSONE_FLAG			33
#define  AU2_SEC_RAMAN_BOTTOM_FLAG		34
#define  AU2_SEC_PEAK_FIFTY_ERROR_FLAG		35
#define  AU2_SEC_SAT_APD_BOT_FLAG		36
#define  AU2_SEC_STRETCHED_BOT_FLAG		37
#define  AU2_SEC_SHOAL_PEAK_FLAG		38
#define  AU2_INTERLEAVE_FLAG			39
#define  AU2_TIM1_FLAG				40
#define  AU2_DATA_OFFSET_ZERO_FLAG		41
#define  AU2_INTERPOLATED_EDGE_FLAG		42
#define  AU2_PINNED_EDGE_FLAG			43
#define	 AU2_SCANNER_Y_PINNED_FLAG		44
#define	 AU2_SCANNER_EDGE_NOT_FOUND_FLAG	45
#define	 AU2_SCANNER_EDGE_BAD_FLAG		46
#define	 AU2_STRETCHED_IR_FLAG			47
#define	 AU2_STRETCHED_RAMAN_FLAG		48
#define	 AU2_INS_FLAG				49
#define	 AU2_GPS_FLAG				50
#define	 AU2_WAVEHEIGHT_FLAG			51
/*
 * The mask values are computed as 2 to the (x-1) where x is the below number 
 *that is commented out beside the value (and reflects the numbers above) 
 */
#define  AU_INTERPOLATED_FLAG_MASK		1		/* 1  */
#define  AU_NADIR_ANGLE_FLAG_MASK		2		/* 2  */
#define  AU_LAND_FLAG_MASK			4		/* 3  */
#define  AU_SFC_TRACKER_BUFFER_FLAG_MASK	8		/* 4  */
#define  AU_ST_SURFING_FLAG_MASK		16		/* 5  */
#define  AU_ST_JERK_FLAG_MASK			32		/* 6  */
#define  AU_STRETCHED_BOT_FLAG_MASK		64		/* 7  */
#define  AU_GREEN_GREEN_DEPTH_DIF_FLAG_MASK	128		/* 8  */
#define  AU_LATE_HDWR_SFC_FLAG_MASK		256		/* 9  */
#define  AU_SATURATED_APD_BOT_FLAG_MASK		512		/* 10 */
#define  AU_NO_HARD_OR_SOFT_SFC_FLAG_MASK	1024 		/* 11 */
#define  AU_EARLY_HDWR_SFC_FLAG_MASK		2048		/* 12 */
#define  AU_NOISY_HEIGHT_FLAG_MASK		4096		/* 13 */
#define  AU_SOFT_SFC_DIF_12_FLAG_MASK		8192		/* 14 */
#define  AU_SOFT_SFC_DIF_13_FLAG_MASK		16384		/* 15 */
#define  AU_SOFT_SFC_DIF_23_FLAG_MASK		32768		/* 16 */
#define  AU_HARD_SOFT_SFC_DIF_FLAG_MASK		65536		/* 17 */
#define  AU_PEAK_FIFTY_ERROR_FLAG_MASK		131072		/* 18 */
#define  AU_ACCEL_BIAS_CHANGE_FLAG_MASK		262144		/* 19 */
#define  AU_RAMAN_BOTTOM_FLAG_MASK		524288		/* 20 */
#define  AU_GLINT_ERROR_FLAG_MASK		1048576		/* 21 */
#define  AU_LAND_AND_RAMAN_FLAG_MASK		2097152		/* 22 */
#define  AU_NO_LAND_NO_RAMAN_FLAG_MASK		4194304		/* 23 */
#define  AU_STRETCHED_SFC_FLAG_MASK		8388608		/* 24 */
#define  AU_SATURATED_SFC_FLAG_MASK		16777216 	/* 25 */
#define  AU_NO_SFC_DATA_FLAG_MASK		33554432 	/* 26 */
#define	 AU_SHOAL_PEAK_FLAG_MASK		67108864 	/* 27 */
#define	 AU_LESSER_PULSE_SELECTED_FLAG_MASK	134217728 	/* 28 */
#define	 AU_NO_OTF_DATUM_ZONE_FLAG_MASK		268435456	/* 29 */
#define  AU_OTF_ALT_FLAG_MASK			536870912	/* 30 */

/* 30 left intentionally blank.  This is done so warning handling code
 * can work with the multiple words as one (ie warnings summary program)
 * The warnings in the second work will go from at 31-60) */

#define  AU2_NO_TIDE_ZONE_FLAG_MASK		1 		/* 31 */
#define  AU2_DLESSONE_FLAG_MASK			2		/* 32 */
#define  AU2_D2LESSONE_FLAG_MASK		4		/* 33 */
#define  AU2_SEC_RAMAN_BOTTOM_FLAG_MASK		8		/* 34 */
#define  AU2_SEC_PEAK_FIFTY_ERROR_FLAG_MASK	16		/* 35 */
#define  AU2_SEC_SAT_APD_BOT_FLAG_MASK		32		/* 36 */
#define  AU2_SEC_STRETCHED_BOT_FLAG_MASK	64		/* 37 */
#define  AU2_SEC_SHOAL_PEAK_FLAG_MASK		128		/* 38 */
#define  AU2_INTERLEAVE_FLAG_MASK		256		/* 39 */
#define  AU2_TIM1_FLAG_MASK			512		/* 40 */
#define  AU2_DATA_OFFSET_ZERO_FLAG_MASK		1024		/* 41 */
#define  AU2_INTERPOLATED_EDGE_FLAG_MASK	2048		/* 42 */
#define  AU2_PINNED_EDGE_FLAG_MASK		4096		/* 43 */
#define  AU2_SCANNER_Y_PINNED_FLAG_MASK		8192		/* 44 */
#define  AU2_SCANNER_EDGE_NOT_FOUND_FLAG_MASK	16384		/* 45 */
#define  AU2_SCANNER_EDGE_BAD_FLAG_MASK		32768		/* 46 */
#define	 AU2_STRETCHED_IR_FLAG_MASK		65536		/* 47 */
#define	 AU2_STRETCHED_RAMAN_FLAG_MASK		131072		/* 48 */
#define  AU2_INS_FLAG_MASK			262144		/* 49 */
#define  AU2_GPS_FLAG_MASK			524288		/* 50 */
#define  AU2_WAVEHEIGHT_FLAG_MASK		1048576		/* 51 */




/*
 * STATUS VALUES WERE CHANGED TO FLAGS FROM MASKS SO THEY COULD BE QUERIED
 * IN THE DATABASE...(SIMPLER)
 */
#define  STATUS_KEPT_MASK			1		/* 1  */
#define  STATUS_KILLED_MASK			2		/* 2  */
#define  STATUS_SWAPPED_MASK			3		/* 3  */

#ifndef WARN_MESSAGES

#define WARN_MESSAGES


static char warning_mess[AU_NUM_WARN_TYPES][50] =
{
	"1  *_INTERPOLATED_[K]",
	"2  *_NADIR_ANGLE_[K]",
	"3  *_LAND_[K]",
	"4  SFC_TRACKER_BUFFER[10]",
	"5  SURFING_[calc]",
	"6  *_JERK_[K,]",
	"7  (*)_STR_BOT_[K_or_10]",
	"8  *_GREEN_GREEN_DEPTH_DIF_[K]",
	"9  LATE_HARDWARE_SFC_[5]",
	"10 *_SATURATED_APD_BOT_[K]",
	"11 *_NO_HARD_OR_SOFT_SFC_[K]",
	"12 EARLY_HARDWARE_SFC_[15]",
	"13 NOISY_HEIGHT_[10]",
	"14 SOFT_SFC_DIF_12_[0]",
	"15 SOFT_SFC_DIF_13_[0]",
	"16 SOFT_SFC_DIF_32_[0]",
	"17 HARD_SOFT_SFC_DIF_[10]",
	"18 PEAK_FIFTY_ERROR_[15]",
	"19 ACCEL_BIAS_CHANGE_[20]",
	"20 RAMAN_BOT_[20]",
	"21 GLINT_ERROR_[10]",
	"22 LAND_AND_RAMAN_[0]",
	"23 NO_LAND_NO_RAMAN_[0]",
	"24 STR_SFC_[10]",
	"25 SATURATED_SFC_[0]",
	"26 NO_SFC_DATA_[0]",
	"27 SHOAL_PEAK_[10]",
	"28 LESSER_PULSE_SELECTED_[10]",
	"29 NO_OTF_DATUM_ZONE_[0]",
	"30 (*)_OTF_ALT[OTF_only]",
	"31 NO_TIDE_ZONE_[0]",
	"32 *_DEP_LESS_ONE_[K]",
	"33 *_SEC_DEP_LESS_ONE_[K]",
	"34 SEC_RAMAN_BOT_[20]",
	"35 SEC_PEAK_FIFTY_ERROR_[15]",
	"36 *_SEC_SATURATED_APD_BOT_[K]",
	"37 (*)_SEC_STR_BOT_[K_or_10]",
	"38 SEC_SHOAL_PEAK_[10]",
	"39 (*)_INTERLEAVE_[K_or_0]",
	"40 (*)_TIM1_[K_or_0]",
	"41 *_DATA_OFFSET_ZERO_[K]",
	"42 INTERPOLATED_EDGE_[10_var]",
	"43 PINNED_EDGE_[10_var]",
	"44 (*)_SCAN_Y_PINNED_[K]",
	"45 (*)_SCAN_EDGE_NOT_FOUND_[K]",
	"46 (*)_SCAN_EDGE_BAD_[K]",
	"47 STR_IR_[0]",
	"48 STR_RAMAN_[0]",
	"49 (*)_INS_[0]",
	"50 (*)_GPS_[K]",
	"51 *_WAVEHEIGHT_[K]"};
#endif

#ifdef  __cplusplus
}
#endif
