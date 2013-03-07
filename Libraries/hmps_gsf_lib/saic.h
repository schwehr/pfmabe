#ifndef    __SAIC_H__
   #define __SAIC_H__

   #ifdef __OS2__
      #ifndef OPTLK
    #define OPTLK _Optlink
      #endif
   #else
      #ifndef OPTLK
    #define OPTLK
      #endif
   #endif

   /*
    * Define the clock to be used by realtime programs.
    *  The default clock will be IRIG-B. (as specified by CLOCK_SNP_AITG)
    */
   #define PROJECT_CLOCK CLOCK_SNP_IRIGB

   /*
    * The logged data SAIC standard string style code is defined here.
    * If a project requires a style different than this it must provide
    * a telegram containing the new style.
    */
   #define DELIMITER_STYLE SNP_STR_SPACE
   #define DATE_STYLE	   SNP_STR_DATE_05
   #define TIME_STYLE	   SNP_STR_TIME_01 | SNP_STR_DECIMAL02
   #define POSITION_STYLE  SNP_STR_POS_MINUTES | SNP_STR_DECIMAL05 | SNP_STR_HEMISPHERE

   #define SET_SAIC_STYLE(x) \
   { \
      (x).delimiter = DELIMITER_STYLE; \
      (x).date = DATE_STYLE; \
      (x).position = POSITION_STYLE; \
      (x).time = TIME_STYLE; \
   }

/* The following defines are used in the ISS and SMI programs
 * to identify display units requested by the operator. These
 * values are set into the SYSTEM_S_PRM telegram by system control
 * and are used by the GUI programs to format the display information
 */

#define DISTANCE_METERS 0x01
#define DISTANCE_YARDS	0x02
#define DISTANCE_FEET	0x03
#define DISTANCE_MILES	0x04

#define POSITION_DECMIN 0x01
#define POSITION_DECSEC 0x02
#define POSITION_XY  0x03

#define VELOCITY_KNOTS	  0x01
#define VELOCITY_METERSEC 0x02

#define DEPTH_FEET    0x01
#define DEPTH_METERS  0x02
#define DEPTH_FATHOMS 0x03

#endif	  /* __SAIC_H__ */
