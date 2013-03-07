#ifndef UNISIPS_IO_H
#define UNISIPS_IO_H
/*
 * Header file for defining the new UNISIPS header and record structures.
 * UNISIPS Format Version 4.0
 */

#if defined(__cplusplus)
extern "C"
{

#endif

#define MAX_SYS_ID     30
#define MAX_FORMAT_ID  10
#define MAX_PLATFORM_ID 40
#define MAX_EXTRA_ID   (204 - MAX_SYS_ID - MAX_FORMAT_ID - MAX_PLATFORM_ID)
#define EMPTY_VAL      -999
#define VERSION_NUMBER 4.00
#define MAX_CHANNELS  5
#define METERS  1
#define FEET    2
#define FATHOMS 3
#define SIDESCAN  0
#define SUBBOTTOM 1
#define BATHY     2
#define TYPE_UNSIGNED_CHAR   0
#define TYPE_CHAR            1
#define TYPE_SHORT           2
#define TYPE_UNSIGNED_SHORT  3
#define TYPE_INTEGER         4
#define TYPE_UNSIGNED_INT    5
#define TYPE_FLOAT           6
#define TYPE_DOUBLE          7

#define BitsPerByte     8
/* to set 	|=  LAB
   to turn off	&= ~LAB */
   
#define LAYBACK_APPLIED_BIT 0x8000

typedef struct
{
 int    rows,                    /* number of rows in image file */
        cols;                    /* number of cols in image file */
   
 float  pixel_res;               /* pixel resolution (meters) 
 				    (for compatiability with old format)  */

 char   sys_id     [MAX_SYS_ID];  /* Type system data came from (SIMRAD,etc) */
 char   format_id  [MAX_FORMAT_ID];   /*Always UNISIPS*/
 char   platform_id[MAX_PLATFORM_ID]; /*Name of platform used to collect data*/
 char   spare1     [MAX_EXTRA_ID];    /* Spare Unused Space */

 double max_lat,                 /* Minimum and maximum lat and lon */
        min_lat,
        max_lon,
        min_lon;

 float  version;                 /* Version number of UNISIPS used to create file */
 char   creation_date[16];       /* Date that file was created  */

 short  sub_header_flag;         /* Indicates sub_header_record is present*/
 short  bathy_units;             /* Indicates the units of a bathy channel
					1 - METERS
					2 - FEET
					3 - FATHOMS */                   
 
 short  total_channels;          /* Total number of channels in the file */
 short  datatype;                /* Indicates the type of data being stored       
                                          0 -- Indicates unsigned char
                                          1 -- Indicates char
                                          2 -- Indicates short 
                                          3 -- Indicates unsigned short
                                          4 -- Indicates integer
                                          5 -- Indicates unsigned integer
                                          6 -- Indicates float 
                                          7 -- Indicates double */

 short  channel_type[MAX_CHANNELS];  /* Indicates the type of each channel 
					  0 - SIDESCAN/ACOUSTIC IMAGERY
					  1 - SUBBOTTOM
					  2 - BATHY*/ 

 unsigned short channel_width[MAX_CHANNELS]; /* Indicates the width for each channel */

 float  pulse_length,
        frequency,               /* Hull mounted/single frequency system */
        port_frequency,          /* 0 if single frequency */
        starboard_frequency,     /* 0 if single frequency */
        band_width;              /* for chirp system */

 float  channel_frequency[MAX_CHANNELS]; /* Indicates the frequency for each channel 
                                                      frequency is in KHz  */
 
 int    processing_history[2];   /* bitmask to show what corrections were 
                                    done to file.  64 bits are availiable, 
                                    only 6 are used for now, they are:

                    BIT    CORRECTION
                    ---    ----------
                     0     slant range
                     1     TVG
                     2     Angle Varying Gain
                     3     Histogram Stretch
                     4     Histogram Equalization
                     5     Speckle Noise Removal

                  When a bit is set, it indicates that the
                  corresponding correction was made to file.
                                 */

 short  bits_per_pixel;    
 short  num_boundary_pts;
 int    boundary_pts_loc;      
 int    endian;                  /* flag to tell if data needs to be 
				    byte swapped.
				     (0x00010203) -  no byte swapping 
				     (0x3020100)  -  needs byte swapping
				  */
				  
 float  channel_resolution[MAX_CHANNELS]; /*Resolution for each channel (meters)*/

 unsigned char extra_space[144]; /* Unused space, will be used for adding to
                                    the header at a later date.  */
} UNISIPS_HEADER;

typedef struct
{
 int            year;
 unsigned short tide_v;          /* 10 bit tide level (scaled * 10, offset   */
                                 /* by +512, range is -511 to 511, 0 is no   */
                                 /* level) and 6 bit validity for cb_depth   */
                                 /* value.  Low order bit is manually        */
                                 /* invalid.  Next bit is filter invalid.    */
                                 /* Others are TBD.                          */
 unsigned short layback;         /* was cable_out, changed 11 May 1999 - CBM */
 				 /* if very first bit is set, this layback   */
				 /* has been applied.  If that bit is 0, the */
				 /* layback has NOT been applied.            */
				 /* Layback is stored in decimeters.         */

 double         time;            /* Total seconds since beginning of year */

 float          roll,            /* Roll in radians */
                pitch,           /* Pitch in radians */
                heading;         /* Heading in radians */

 float          fish_spd;        /* fish speed in m/s */

 double         lat,             /* Latitude in decimal degrees */
                lon;             /* Longitude in decimal degrees */

 float          cb_depth;        /* Center beam depth: fish to bottom (meters) */
 float          tow_depth;       /* Towfish only, surface to fish (meters) */
} UNISIPS_REC_HDR;

typedef struct
{
  float 	Pgain;		/*Port Gain in db			*/
  float		Sgain;		/*Starboard Gain in db			*/
  float		Pulse_length;	/*Width of sensor pulse in micro secs   */
  			        /* 0 if chirp is used.			*/
  float 	Bandwidth;	/*Bandwidth of chirp, 0 if chirp not used*/
  float		Ping_period;	/*Period of ping in secs.		*/
  float		Temperature;	/*Water temp in deg C			*/
  float		Sound_vel;	/*Velocity of sound in meters/sec	*/
  double	Ship_lat;       /*Ship's Latitude in degrees		*/
  double	Ship_lon;	/*Ship's Longitude in degrees		*/
  float		Prange;		/*Sensor port range in meters		*/
  float		Srange;		/*Sensor starboard range in meters	*/
  float		Samp_freq;	/*Sample frequency of system, samps/sec */	
} UNISIPS_REC_SUBHDR;

typedef struct
{
  unsigned char *data;           /* image data */
} UNISIPS_REC;



/* 
 * +-------------------------+
 * + I/O Function Prototypes +
 * +-------------------------+
 */
 
 int swap_header(UNISIPS_HEADER *);
 int swap_unisips_rec_hdr(UNISIPS_REC_HDR *);

 int read_unisips_header  (FILE *,UNISIPS_HEADER *, int *);
 int read_unisips_rec_hdr (FILE *, UNISIPS_HEADER *,UNISIPS_REC_HDR *, UNISIPS_REC_SUBHDR *,int, int);
 int read_unisips_record  (FILE *,UNISIPS_HEADER *,UNISIPS_REC_HDR *,UNISIPS_REC_SUBHDR *,int,int,unsigned char *,int);
 int read_boundary_pts    (FILE *,UNISIPS_HEADER *,float *, float *,int);

 int write_unisips_header (FILE *,UNISIPS_HEADER *, int);
 int write_unisips_rec_hdr(FILE *,UNISIPS_HEADER *, UNISIPS_REC_HDR *,UNISIPS_REC_SUBHDR *, int, int);
 int write_unisips_record (FILE *,UNISIPS_HEADER *, UNISIPS_REC_HDR *,UNISIPS_REC_SUBHDR *,int,int,unsigned char *,int);
 int write_unisips_data   (FILE *,UNISIPS_HEADER *, int,unsigned char *,int);
 int write_boundary_pts   (FILE *,UNISIPS_HEADER *, float *, float *,int); 

#if defined(__cplusplus)
}
#endif


#endif
