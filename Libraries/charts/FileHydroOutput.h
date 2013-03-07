#include "nvtypes.h"

/*****************************************************************************
 * FileHydroOutput.h     Header
 *
 * Purpose:    This header file hold the definition of the hydrographic inputs 
 *             structure for use with the the inputs file.
 *          
 *
 * Revision History:
 *
 * 03/07/02 DR    -added DKS defs..  Were hard coded as 0x01/2/4/8/10 in the 
 *                                   code before.
 *
 * 08/02/03 JD    -modified for NAVO data types
 *
 * 04/08/04 JD    -added text header info as per GCS_file_formats.pdf
 *
 ****************************************************************************/

#ifndef HOFHead
#define HOFHead

#ifdef  __cplusplus
extern "C" {
#endif


#include "charts.h"


/* Full header will be a 16k block.  */

/* 
   Full Output header is 256k:

   Header Text    8k -has WAY more than I can use but keep extra for future
   Binary         8k -Binary - date, start/stop timestamp, planning info (points).... will also contain most of the 
                  items in the "text" block in BIN form
*/

#define    HOF_HEAD_TEXT_BLK_SIZE    (8 * 1024)
#define    HOF_HEAD_BIN_BLK_SIZE     (8 * 1024)
#define    HOF_HEAD_BIN_OFFSET       (HOF_HEAD_TEXT_BLK_SIZE)

#define    HOF_HEAD_SIZE             (HOF_HEAD_TEXT_BLK_SIZE +  HOF_HEAD_BIN_BLK_SIZE)


#define    HOF_NEXT_RECORD           (-1)


/*  DEFINES FOR BITS IN STATUS BYTE IN OUTPUTS (below)...  */

#define    AU_STATUS_DELETED_BIT   0x01        /* B0  */
#define    AU_STATUS_KEPT_BIT      0x02        /* B1  */
#define    AU_STATUS_SWAPPED_BIT   0x04        /* B2  */

/*
    DKS will use the above bit defs (for DELETED/KEPT/SWAPPED) in addition to 
    the bits listed below...
*/

#define    AU_DKS_TOPO_BIT         0x08        /* B3  */
#define    AU_DKS_SLOPE_CHANGE_BIT 0x10        /* B4  */


/*
  DEFINE BITS IN "suspect_status" (in struture below...)
  None  = Not an suspect/feature/outlier
  B0 = Suspect/Anomaly Flag (set in Fledermaus by operator and/or by AU)
  B1 = Feature/Target Flag (set in Fledermaus by operator and/or by AU) 
  B2 = AU detected Flier/outlier flag (set by AU) 
  B3 = AU detected suspect/anomaly (set by AU) [FUTURE USE IN TRACING]
  B4 = AU detected Feature/Target (set by AU) [NOT USED YET] [FUTURE USE IN 
       TRACING]
*/

#define  SUSPECT_STATUS_SUSPECT_BIT    0x01    /*  B0 = suspect/anomaly [FLEDERMAUS CAN CHANGE]  */
#define  SUSPECT_STATUS_FEATURE_BIT    0x02    /*  B1 = feature/target [FLEDERMAUS CAN CHANGE]  */
#define  SUSPECT_STATUS_OUTLIER_BIT    0x04    /*  B2 = outlier/flier  */
#define  SUSPECT_STATUS_AU_SUSPECT_BIT 0x08    /*  B3 = AU suspect/anomaly  */
#define  SUSPECT_STATUS_AU_FEATURE_BIT 0x10    /*  B4 = AU feature/target  */


#define  PMT               0
#define  APD               1
#define  IR                2
#define  RAMAN             3


typedef struct
{
  NV_INT64       start_time;       /* time in microseconds from 01/01/1970 */
  NV_INT64       stop_time;        /* time in microseconds from 01/01/1970 */
  NV_INT64       file_create_time; /* timestamp when file was created...   */
  NV_INT64       file_mod_time;    /* timestamp when file was last modified **TBR**  */

  NV_FLOAT64     min_lat;
  NV_FLOAT64     min_long;
  NV_FLOAT64     max_lat;
  NV_FLOAT64     max_long;

  NV_INT64       fill_i64[6];

  NV_INT32       num_shots;
  NV_FLOAT32     gcs_version;
  NV_FLOAT32     gcs_file_version;

  NV_INT32       header_size;
  NV_INT32       text_block_size;
  NV_INT32       binary_block_size;
  NV_INT32       record_size;

  NV_INT32       ab_system_type;
  NV_INT32       ab_system_number;
  NV_INT32       rep_rate;

  NV_INT16       data_type;        /* 0 = Normal (DGPS), 1 = kinematic (KGPS)  */
  NV_INT16       fill_short;

  NV_INT32       fill_i32[11];

  NV_U_INT16     flightline;       /* Line number is flightline/10 and version is remainder (ie. 13 is 1-3)  */
  NV_INT16       fill_i16[7];
    
  NV_CHAR        UserName[64];     /* Who processed this (whole) line last */
  NV_CHAR        TBD[960];

  NV_CHAR        project[128];
  NV_CHAR        mission[128];
  NV_CHAR        dataset[128];
} HOF_INFO_T;


typedef struct
{
  NV_CHAR        file_type[128];
  NV_BOOL        endian;

  NV_FLOAT32     software_version;
  NV_FLOAT32     file_version;

  NV_INT32       header_size;
  NV_INT32       text_block_size;
  NV_INT32       bin_block_size;
  NV_INT32       record_size;

  NV_BYTE        ab_system_type;
  NV_BYTE        ab_system_number;
  NV_INT16       system_rep_rate;

  NV_CHAR        UserName[64];     /* Who processed this (whole) line last */

  NV_CHAR        project[64];
  NV_CHAR        mission[64];
  NV_CHAR        dataset[64];
  NV_CHAR        flightline_number[64];
  NV_INT16       coded_fl_number;
  NV_CHAR        flight_date[64];
  NV_CHAR        start_time[64];
  NV_CHAR        end_time[64];
  NV_INT64       start_timestamp;  /* time in microseconds from 01/01/1970 */
  NV_INT64       end_timestamp;    /* time in microseconds from 01/01/1970 */
  NV_INT32       number_shots;
  NV_CHAR        dataset_create_date[64];
  NV_CHAR        dataset_create_time[64];
  NV_FLOAT64     line_min_lat;
  NV_FLOAT64     line_max_lat;
  NV_FLOAT64     line_min_lon;
  NV_FLOAT64     line_max_lon;
} HOF_TEXT_T;


typedef struct
{
  /*  8k Text info block */

  HOF_TEXT_T     text;
  NV_CHAR        text_block[(HOF_HEAD_TEXT_BLK_SIZE) - sizeof(HOF_TEXT_T)];


  /* 8k Flightline block */

  HOF_INFO_T     info;
  NV_CHAR        bin_block[(HOF_HEAD_BIN_BLK_SIZE) - sizeof(HOF_INFO_T)];
} HOF_HEADER_T;


typedef struct
{
  NV_INT64       timestamp;        /* time in microseconds from 01/01/1970 */
  NV_INT16       haps_version;
  NV_INT16       position_conf;


  /*
    STATUS DEF (SEE ABOVE FOR BIT SETTINGS)
        bit 0 = deleted    (1); 
        bit 1 = kept       (2); 
        bit 2 = swapped    (4); 
        rest TBD. 
    Only one of kept or swapped may be used at once and they can only be used for shots with a second depth; killed can be used for any shots) 
  */

  NV_CHAR        status;
    

  /*
    Suggested Delete, Keep, Swap value from AU (Theodore's algorithm; SEE ABOVE FOR BIT SETTINGS))
  */

  NV_CHAR        suggested_dks;    
    

  /*
    SUSPECT STATUS (SEE ABOVE FOR BIT SETTINGS)
  */

  NV_CHAR        suspect_status;
  NV_CHAR        tide_status;          /* Lower two bits:                  */
                                       /* 0 - no tide corection applied.   */
                                       /* 1 - predicted tide applied.      */
                                       /* 2 - preliminary tide applied.    */
                                       /* 3 - verified tide applied.       */
                                       /* Other 6 bits are TBD.            */
  NV_FLOAT64     latitude;
  NV_FLOAT64     longitude;
  NV_FLOAT64     sec_latitude;
  NV_FLOAT64     sec_longitude;


  /*
    The following four fields are redundant data that can be used for the loading of the data into the GCS software (as well as Fledermaus...).  It
    is processing mode independent and also specifies the processing mode...
  */

  /*
    IMPORTANT NOTE:  correct_depth and tide_cor_depth are actually elevations.  That is, they are negative depths.
  */

  NV_FLOAT32     correct_depth;       /* Copied from "tide_cor_depth" or "kgps_elevation", UNLESS we did a Shoreline Depth Swap (AKA topo 
                                         depth swap - abdc = 72), this point was "Shallow Water Processed" (abdc = 74), or this is land 
                                         data (abdc 70) in which case it's copied from kgps_topo (with datum shift applied if needed).  */
  NV_FLOAT32     correct_sec_depth;   /* Copied from "sec_depth" or "kgps_sec_elev"  */
  NV_INT16       abdc;                /* from either "ab_dep_conf" or "kgps_abd_conf": 13 - land, unknown elevation < 70 - no good,
                                         >= 70 - good, 70 - land, known elevation, 72 - shoreline depth, 74 - shallow water algorithm.
                                         If this value is over 100 we have added 100 to the original value to override GCS making a point
                                         invalid.  If the value is less than -100 we have manually invalidated a point that was automatically
                                         invalidated by GCS (less than 70) and was manually validated  by a user.*/
  NV_INT16       sec_abdc;            /* Same as above  */
  NV_CHAR        data_type;           /* 0 = normal (DGPS), 1 = kinematic GPS  */
  NV_CHAR        land_mode;           /* Attempt at land detection  */


  /*   Classify status byte (bits defined in GCSCnsts.h)

       bit 0 = classified        (1=classified; 0=not classified)

       bit 1 = non-bare_earth1   (0x02);   

       bit 2 = non-bare_earth2   (0x04);

       bit 3 = water             (0x08);   

       rest TBD.

       If only the first bit is set it is BARE EARTH.

       If value is 0, it has not been classified.

       Yes, this is a weird definition…

       In future the two non bare earth bits will be split into

       buildings/vegetation.  */

  NV_CHAR        classification_status;

  NV_CHAR        tbd[1];


  /* Likely to be used for "CUBE" integration.... */

  NV_FLOAT32     future_use[2];


  /* Traditional mode info... */

  NV_FLOAT32     tide_cor_depth;
  NV_FLOAT32     reported_depth;
  NV_FLOAT32     result_depth;
  NV_FLOAT32     sec_depth;
  NV_FLOAT32     wave_height;
  NV_FLOAT32     elevation;
  NV_FLOAT32     topo;
  NV_FLOAT32     altitude;
    

  /* KGPS processing mode info */

  NV_FLOAT32     kgps_elevation;
  NV_FLOAT32     kgps_res_elev;
  NV_FLOAT32     kgps_sec_elev;
  NV_FLOAT32     kgps_topo;
  NV_FLOAT32     kgps_datum;
  NV_FLOAT32     kgps_water_level;
    

  NV_FLOAT32     k;                   /* K Value TBD      */
  NV_FLOAT32     intensity;           /* Intensity or bottom reflectance TBD      */


  /*  IMPORTANT NOTE : The terms first and second in variable names are used to denote primary and secondary returns.  That is, those returns
      that are accepted as true (primary) and those that are deemed incorrect (secondary).  These do not correspond to first return and second
      return.  To find first return you need to check to see which of bot_bin_first (primary) and bot_bin_second (secondary) has the smallest 
      number (ie, the least number of bins from the surface.  */


  NV_FLOAT32     bot_conf;            /* bottom signal strength for channel of first depth  */
  NV_FLOAT32     sec_bot_conf;        /* bottom signal strength for channel of second depth  */
  NV_FLOAT32     nadir_angle;         /* degrees off nadir (usually around 20 degrees)  */
  NV_FLOAT32     scanner_azimuth;     /* Variable angle (- = left side, + = right side; +- 60)  */
  NV_FLOAT32     sfc_fom_apd;         /* surface signal strength for GAPD channel  */
  NV_FLOAT32     sfc_fom_ir;          /* surface signal strength for IR channel  */
  NV_FLOAT32     sfc_fom_ram;         /* surface signal strength for Raman channel  */


  /* Future items... [Reserve space for them now], No Bottom At   [***TBD***]  */

  NV_FLOAT32     no_bottom_at;
  NV_FLOAT32     no_bottom_at2;

 

  /* Conf (both modes same) */

  NV_INT32       depth_conf;          /* Coded confidence; internal use.. */
  NV_INT32       sec_depth_conf;      /* Coded confidence; internal use.. */

  NV_INT32       warnings;            /* Coded bit warnings...            */
  NV_INT32       warnings2;           /* Coded bit warnings...            */
  NV_INT32       warnings3;           /* Coded bit warnings...[Future use for algorithm]  */

  /* New Algorithm stuff... */

  NV_U_INT16     calc_bfom_thresh_times10[2]; /* MK's Calculated BFOM thresh times by 10;      */
  NV_CHAR        calc_bot_run_required[2];    /* MK's calculated HAP      */


  NV_CHAR        tbd2[2];
    

  /* Bottom/surface stuff (early calcs in AUTO) */

  NV_INT16       bot_bin_first;       /* Bin of the primary return (not necessarily first return) */
  NV_INT16       bot_bin_second;      /* Bin of the secondary return (not necessarily second return) */
                                      /* First return has smallest bin number */
  NV_INT16       bot_bin_used_pmt;    /* These get mixed around if the primary and secondary are on */ 
  NV_INT16       sec_bot_bin_used_pmt;/* different channels  */     
  NV_INT16       bot_bin_used_apd;    /* ditto */ 
  NV_INT16       sec_bot_bin_used_apd;/* ditto */

  NV_U_CHAR      bot_channel;         /* Channel of the primary return (see bot_bin_first) */
  NV_U_CHAR      sec_bot_chan;        /* Channel of the secondary return (see bot_bin_second) */
  NV_U_CHAR      sfc_bin_apd;         
  NV_U_CHAR      sfc_bin_ir;             
  NV_U_CHAR      sfc_bin_ram;         
  NV_U_CHAR      sfc_channel_used;     


  /* Abbreviated confidences (each mode has one for each depth/elevation) */

  NV_CHAR        ab_dep_conf;
  NV_CHAR        sec_ab_dep_conf;
  NV_CHAR        kgps_abd_conf;
  NV_CHAR        kgps_sec_abd_conf;

  NV_CHAR        tbd3[2];
}  HYDRO_OUTPUT_T;                        /* Whole output file definition... */


FILE *open_hof_file (NV_CHAR *path);
NV_INT32 hof_read_header (FILE *fp, HOF_HEADER_T *head);
NV_INT32 hof_read_record (FILE *fp, NV_INT32 num, HYDRO_OUTPUT_T *record);
NV_INT32 hof_write_header (FILE *fp, HOF_HEADER_T head);
NV_INT32 hof_write_record (FILE *fp, NV_INT32 num, HYDRO_OUTPUT_T record);
void hof_get_uncertainty (HYDRO_OUTPUT_T record, NV_FLOAT32 *h_error, NV_FLOAT32 *v_error, NV_FLOAT32 in_depth, NV_INT32 abdc);
void hof_dump_record (HYDRO_OUTPUT_T record);


#ifdef  __cplusplus
}
#endif


#endif
