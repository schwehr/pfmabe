
/*********************************************************************************************

    This module is public domain software that was developed by the U.S. Naval Oceanographic
    Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.

*********************************************************************************************/


/*********************************************************************************************

    This library is used to read and (optionally) update an AHAB HAWKEYE CSS Generic Binary
    Output Format (.bin) file.  There is no option to create or append to files.  This will be
    used to edit CSS .bin files using the PFM Area-Based Editor (ABE).

*********************************************************************************************/



#ifndef __HAWKEYE_H__
#define __HAWKEYE_H__


#ifdef  __cplusplus
extern "C" {
#endif


  /*  Preparing for language translation using GNU gettext at some point in the future.  */

#define _(String) (String)
#define N_(String) String

  /*
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
  */


#include "hawkeye_nvtypes.h"


  /*  Windoze insanity.  Note that there may be nothing defined for any of these DLL values.  When building statically on Windows I
      strip out the __declspec(dllexport) and __declspec(dllimport) definitions before I copy the .h file to the central include
      location.  That's so I don't have to modify a bunch of application Makefiles to check for static build adn set the _STATIC
      option.  Of course, on Linux/UNIX we don't have to do anything to these ;-)  */

#ifdef HAWKEYE_DLL_EXPORT
#  define HAWKEYE_DLL __declspec(dllexport)
#else
#  ifdef NVWIN3X
#    ifdef HAWKEYE_STATIC
#      define HAWKEYE_DLL
#    else
#      define HAWKEYE_DLL __declspec(dllimport)
#    endif
#  else
#    define HAWKEYE_DLL
#  endif
#endif


  /*  Various constants (hopefully these are intuitively obvious to the most casual observer).  */

#define HAWKEYE_MAX_FILES                          32
#define HAWKEYE_NEXT_RECORD                        -1
#define HAWKEYE_MAX_TOTAL_WAVEFORMS_BYTES          10000
#define HAWKEYE_NUMBER_OF_TOPO_PIXELS              8
#define HAWKEYE_NUMBER_OF_TOPO_REFLEXES            3


  /*  Hawkeye bin file data types (for _HD.bin and _TD.bin files).  */

#define HAWKEYE_HYDRO_BIN_DATA                     1
#define HAWKEYE_TOPO_BIN_DATA                      2


  /*  CSS bin file header descriptor and tag lengths.  */

#define HAWKEYE_CSSOUT_DATAFIELD_DESCR_LENGTH      128
#define HAWKEYE_CSSOUT_DATAFIELD_TAG_LENGTH        32


  /*  Manual Output Screening Flags definitions.  */

#define HAWKEYE_VALID                   0       /* 0000 0000 0000 */
#define HAWKEYE_INVALID                 4       /* 0000 0000 0100 */


  /*  File open modes.  */

#define HAWKEYE_UPDATE                             0
#define HAWKEYE_READONLY                           1


  /*  Error conditions.  */

#define       HAWKEYE_SUCCESS                             0
#define       HAWKEYE_META_HEADER_READ_FSEEK_ERROR       -1
#define       HAWKEYE_CONTENTS_HEADER_READ_FSEEK_ERROR   -2
#define       HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR       -3
#define       HAWKEYE_TOO_MANY_OPEN_FILES                -4
#define       HAWKEYE_OPEN_UPDATE_ERROR                  -5
#define       HAWKEYE_OPEN_READONLY_ERROR                -6
#define       HAWKEYE_POINT_RECORD_BUFFER_CALLOC_ERROR   -7
#define       HAWKEYE_META_HEADER_WRONG_VERSION_ERROR    -8
#define       HAWKEYE_CLOSE_ERROR                        -9
#define       HAWKEYE_INVALID_RECORD_NUMBER_ERROR        -10
#define       HAWKEYE_READ_FSEEK_ERROR                   -11
#define       HAWKEYE_READ_ERROR                         -12
#define       HAWKEYE_UPDATE_RECORD_FSEEK_ERROR          -13
#define       HAWKEYE_UPDATE_RECORD_WRITE_ERROR          -14
#define       HAWKEYE_NOT_HAWKEYE_FILE_ERROR             -15
#define       HAWKEYE_SFM_FILE_OPEN_ERROR                -16
#define       HAWKEYE_SFM_READ_FSEEK_ERROR               -17
#define       HAWKEYE_SFM_READ_ERROR                     -18
#define       HAWKEYE_SFM_EOF_ERROR                      -19
#define       HAWKEYE_DAT_FILE_OPEN_ERROR                -20
#define       HAWKEYE_DAT_READ_FSEEK_ERROR               -21
#define       HAWKEYE_DAT_READ_HEADER_ERROR              -22
#define       HAWKEYE_NOT_SHOT_DATA_ERROR                -23
#define       HAWKEYE_DAT_READ_SHOT_ERROR                -24
#define       HAWKEYE_DAT_READ_WAVE_ERROR                -25


  typedef enum
    {
      HawkeyeCSSOutCHAR                = 0,   /* 1 byte character */
      HawkeyeCSSOutBOOL,                      /* 1 byte boolean true/false */
      HawkeyeCSSOutFLOAT32,                   /* 4 byte/32 bit float, little endian */
      HawkeyeCSSOutFLOAT64,                   /* 8 byte/64 bit float, little endian */
      HawkeyeCSSOutINT16,                     /* 2 byte short integer */
      HawkeyeCSSOutINT32,                     /* 4 byte long integer */
      HawkeyeCSSOutUINT16,                    /* 2 byte short unsigned integer */
      HawkeyeCSSOutUINT32,                    /* 4 byte long unsigned integer */
      HawkeyeCSSOutNbrOfValidTypes            /* Counter of # of valid types */
    } E_HawkeyeCSSOutputDataFieldTypes;


  typedef enum
    {
      msgId_shotDataHydro              = 20010, 
      msgId_measurementData            = 20020, 
      msgId_navigationData             = 20030, 
      msgId_storeGenericLog            = 20040, 
      msgId_initGenericLog             = 20050, 
      msgId_locationMission            = 20060, 
      msgId_locationFlightline         = 20070, 
      msgId_flightlineNotSet           = 20072, 
      msgId_topoShotData               = 20110, 
      msgId_acknowledge                = 20120, 
      msgId_sensorSystemCommand        = 20130,
      msgId_sensorSystemStatus         = 20140,
      msgId_sensorSystemSettingCommand = 20150,
      msgId_scannerData                = 20161,
      msgId_scannerData_0              = 20160,
      msgId_shotDataTopo               = 20170,
      msgId_health                     = 20180,
      msgId_topoEchoStatistic          = 20190,
      msgId_timeSync                   = 20200,
      msgId_initGenericLogMulti        = 20210,
      msgId_storageReport              = 20220,
      msgId_picUEye                    = 20230	
    } E_HawkeyeMsgIdType;


  typedef enum
    {
      hydroChannelID_GrnShlw1          = 0, 
      hydroChannelID_GrnDeep1, 
      hydroChannelID_GrnShlw2, 
      hydroChannelID_GrnDeep2, 
      hydroChannelID_GrnShlw3, 
      hydroChannelID_GrnDeep3, 
      hydroChannelID_GrnShlw4, 
      hydroChannelID_GrnDeep4, 
      hydroChannelID_IR1, 
      hydroChannelID_IR2, 
      hydroChannelID_GainMonitorShlw, 
      hydroChannelID_GainMonitorDeep, 
      hydroChannelID_nrOfChannels
    } E_HawkeyeHydroChannelIDType;



  /*  Since many of the fields in a Hawkeye record are optional we have this structure of booleans to indicate
      the presence or absence of each field.  This gets populated when we open the file and read the field tags
      from the header.  */

  typedef struct
  {
    NV_BOOL      Timestamp;
    NV_BOOL      Surface_Latitude;
    NV_BOOL      Surface_Longitude;
    NV_BOOL      Surface_Northing;
    NV_BOOL      Surface_Easting;
    NV_BOOL      Surface_Altitude;
    NV_BOOL      Interpolated_Surface_Altitude;
    NV_BOOL      Surface_Northing_StdDev;
    NV_BOOL      Surface_Easting_StdDev;
    NV_BOOL      Surface_Altitude_StdDev;
    NV_BOOL      Point_Latitude;
    NV_BOOL      Point_Longitude;
    NV_BOOL      Point_Northing;
    NV_BOOL      Point_Easting;
    NV_BOOL      Point_Altitude;
    NV_BOOL      Point_Northing_StdDev;
    NV_BOOL      Point_Easting_StdDev;
    NV_BOOL      Point_Altitude_StdDev;
    NV_BOOL      Azimuth;
    NV_BOOL      Air_Nadir_Angle;
    NV_BOOL      Water_Nadir_Angle;
    NV_BOOL      Slant_range;
    NV_BOOL      Slant_Range_Comp_Model;
    NV_BOOL      Wave_Height;
    NV_BOOL      Water_Quality_Correction;
    NV_BOOL      Tidal_Correction;
    NV_BOOL      Depth;
    NV_BOOL      Depth_Amplitude;
    NV_BOOL      Depth_Class;
    NV_BOOL      Depth_Confidence;
    NV_BOOL      Pixel_Index;
    NV_BOOL      Scanner_Angle_X;
    NV_BOOL      Scanner_Angle_Y;
    NV_BOOL      Aircraft_Latitude;
    NV_BOOL      Aircraft_Longitude;
    NV_BOOL      Aircraft_Northing;
    NV_BOOL      Aircraft_Easting;
    NV_BOOL      Aircraft_Altitude;
    NV_BOOL      Aircraft_Roll;
    NV_BOOL      Aircraft_Pitch;
    NV_BOOL      Aircraft_Heading;
    NV_BOOL      Aircraft_Northing_StdDev;
    NV_BOOL      Aircraft_Easting_StdDev;
    NV_BOOL      Aircraft_Altitude_StdDev;
    NV_BOOL      Aircraft_Roll_StdDev;
    NV_BOOL      Aircraft_Pitch_StdDev;
    NV_BOOL      Aircraft_Heading_StdDev;
    NV_BOOL      Extracted_Waveform_Attributes;
    NV_BOOL      Receiver_Data_Used;
    NV_BOOL      Manual_Output_Screening_Flags;
    NV_BOOL      Waveform_Peak_Amplitude;
    NV_BOOL      Waveform_Peak_Classification;
    NV_BOOL      Waveform_Peak_Contrast;
    NV_BOOL      Waveform_Peak_Debug_Flags;
    NV_BOOL      Waveform_Peak_Attributes;
    NV_BOOL      Waveform_Peak_Jitter;
    NV_BOOL      Waveform_Peak_Position;
    NV_BOOL      Waveform_Peak_Pulsewidth;
    NV_BOOL      Waveform_Peak_SNR;
    NV_BOOL      Scan_Direction_Flag;
    NV_BOOL      Edge_of_Flightline;
    NV_BOOL      Scan_Angle_Rank;
    NV_BOOL      SelectBottomCase;
    NV_BOOL      Elevation;
    NV_BOOL      Return_Number;
    NV_BOOL      Number_of_Returns;
    NV_BOOL      Surface_altitude_interpolated_tide_corrected;
    NV_BOOL      IR_polarisation_ratio;
    NV_BOOL      Spatial_Quality;
    NV_BOOL      IR_AMPLITUDE;
    NV_BOOL      Scan_Position;
    NV_BOOL      K_Lidar;
    NV_BOOL      K_Lidar_Average;
    NV_BOOL      K_Lidar_Deep_Momentary;
    NV_BOOL      K_Lidar_Shallow_Momentary;
    NV_BOOL      Relative_reflectivity;
    NV_BOOL      Waveform_Peak_Gain;
    NV_BOOL      Adjusted_Amplitude;
    NV_BOOL      Water_Travel_Time;
    NV_BOOL      Attenuation_c;
    NV_BOOL      Absorption_a;
    NV_BOOL      Scattering_s;
    NV_BOOL      Backscattering_Bb;
  } HAWKEYE_RECORD_AVAILABILITY;


  typedef struct
  {
    time_t          tv_sec;                                        /*  POSIX seconds from 01/01/1970 created from Timestamp, Survey_Start_Year,
                                                                       and Survey_Start_Day  */
    long            tv_nsec;                                       /*  POSIX nano seconds of the second created from Timestamp, Survey_Start_Year,
                                                                       and Survey_Start_Day  */
    NV_FLOAT64      Timestamp;                                     /*  Onboard Inertial/GPS Navigation System Timestamp. The unit is
                                                                       seconds from the latest GPS time reset  */
    NV_FLOAT64      Surface_Latitude;                              /*  Survey Point Surface Latitude  */
    NV_FLOAT64      Surface_Longitude;                             /*  Survey Point Surface Longitude  */
    NV_FLOAT64      Surface_Northing;                              /*  Survey Point Surface Northing  */
    NV_FLOAT64      Surface_Easting;                               /*  Survey Point Surface Northing  */
    NV_FLOAT64      Surface_Altitude;                              /*  Survey Point Instantaneous Surface Altitude  */
    NV_FLOAT64      Interpolated_Surface_Altitude;                 /*  Survey Point Interpolated Surface Altitude  */
    NV_FLOAT64      Surface_Northing_StdDev;                       /*  Estimated Standard Deviation of Survey Point Surface Northing  */
    NV_FLOAT64      Surface_Easting_StdDev;                        /*  Estimated Standard Deviation of Survey Point Surface Easting  */
    NV_FLOAT64      Surface_Altitude_StdDev;                       /*  Estimated Standard Deviation of Survey Point Surface Altitude  */
    NV_FLOAT64      Point_Latitude;                                /*  Survey Point Surface Northing  */
    NV_FLOAT64      Point_Longitude;                               /*  Survey Point Surface Northing  */
    NV_FLOAT64      Point_Northing;                                /*  Survey Point Northing  */
    NV_FLOAT64      Point_Easting;                                 /*  Survey Point Easting  */
    NV_FLOAT64      Point_Altitude;                                /*  Survey Point Altitude  */
    NV_FLOAT64      Point_Northing_StdDev;                         /*  Estimated Standard Deviation of Survey Point Northing  */
    NV_FLOAT64      Point_Easting_StdDev;                          /*  Estimated Standard Deviation of Survey Point Easting  */
    NV_FLOAT64      Point_Altitude_StdDev;                         /*  Estimated Standard Deviation of Survey Point Altitude  */
    NV_FLOAT64      Azimuth;                                       /*  Scanner Mirror Azimuth Angle at Aircraft Direction of Movement  */
    NV_FLOAT64      Air_Nadir_Angle;                               /*  Scanner Mirror Nadir Angle  */
    NV_FLOAT64      Water_Nadir_Angle;                             /*  Estimated Laser Beam Nadir Angle in Water Column  */
    NV_FLOAT64      Slant_range;                                   /*  Surface slant range  */
    NV_U_INT16      Slant_Range_Comp_Model;                        /*  Enumerator for the computational model used to produce the slant range  */
    NV_FLOAT64      Wave_Height;                                   /*  Estimated Wave Height  */
    NV_FLOAT64      Water_Quality_Correction;                      /*  The value subtracted to produce a wave height corrected surface altitude  */
    NV_FLOAT64      Tidal_Correction;                              /*  The value subtracted to produce a tide corrected surface altitude  */
    NV_FLOAT64      Depth;                                         /*  The calculated depth for this position (positive below water surface)  */
    NV_FLOAT64      Depth_Amplitude;                               /*  The amplitude of the waveform peak attributed to this survey point  */
    NV_U_INT16      Depth_Class;                                   /*  The classification of this survey point  */
    NV_FLOAT64      Depth_Confidence;                              /*  The estimated confidence of the depth or altitude value for this survey point  */
    NV_CHAR         Pixel_Index;                                   /*  Index of the receiver pixel whose data was used to produce this survey point  */
    NV_FLOAT64      Scanner_Angle_X;                               /*  Scanner angle at X-axis  */
    NV_FLOAT64      Scanner_Angle_Y;                               /*  Scanner angle at Y-axis  */
    NV_FLOAT64      Aircraft_Latitude;                             /*  Onboard Inertial/GPS Navigation System Latitude  */
    NV_FLOAT64      Aircraft_Longitude;                            /*  Onboard Inertial/GPS Navigation System Longitude  */
    NV_FLOAT64      Aircraft_Northing;                             /*  Onboard Inertial/GPS Navigation System Northing  */
    NV_FLOAT64      Aircraft_Easting;                              /*  Onboard Inertial/GPS Navigation System Easting  */
    NV_FLOAT64      Aircraft_Altitude;                             /*  Onboard Inertial/GPS Navigation System Altitude  */
    NV_FLOAT64      Aircraft_Roll;                                 /*  Onboard Inertial/GPS Navigation System Roll Angle  */
    NV_FLOAT64      Aircraft_Pitch;                                /*  Onboard Inertial/GPS Navigation System Pitch Angle  */
    NV_FLOAT64      Aircraft_Heading;                              /*  Onboard Inertial/GPS Navigation System Heading  */
    NV_FLOAT64      Aircraft_Northing_StdDev;                      /*  Onboard Inertial/GPS Navigation System Northing Standard Deviation  */
    NV_FLOAT64      Aircraft_Easting_StdDev;                       /*  Onboard Inertial/GPS Navigation System Easting Standard Deviation  */
    NV_FLOAT64      Aircraft_Altitude_StdDev;                      /*  Onboard Inertial/GPS Navigation System Altitude Standard Deviation  */
    NV_FLOAT64      Aircraft_Roll_StdDev;                          /*  Onboard Inertial/GPS Navigation System Roll Angle Standard Deviation  */
    NV_FLOAT64      Aircraft_Pitch_StdDev;                         /*  Onboard Inertial/GPS Navigation System Pitch Angle Standard Deviation  */
    NV_FLOAT64      Aircraft_Heading_StdDev;                       /*  Onboard Inertial/GPS Navigation System Heading Standard Deviation  */
    NV_INT16        Extracted_Waveform_Attributes;                 /*  Attributes extracted/estimated during waveform processing kept as bitwise flags  */
    NV_INT16        Receiver_Data_Used;                            /*  Enumerator for the receiver whose data was used to produce the survey point  */
    NV_CHAR         Manual_Output_Screening_Flags;                 /*  A byte bitwise interpreted as flags assigned during manual screening of post
                                                                       processing output  */
    NV_FLOAT32      Waveform_Peak_Amplitude;                       /*  The amplitude of the waveform processing detected peak used for this survey point  */
    NV_U_INT16      Waveform_Peak_Classification;                  /*  The classification enumerator of the waveform processing detected peak used for
                                                                       this survey point  */
    NV_FLOAT32      Waveform_Peak_Contrast;                        /*  The contrast of the waveform processing detected peak used for this survey point  */
    NV_INT16        Waveform_Peak_Debug_Flags;                     /*  The debug flags for the waveform processing detected peak used for this survey
                                                                       point (two bytes of bitwise flags)  */
    NV_INT16        Waveform_Peak_Attributes;                      /*  The extracted attributes of the waveform processing detected peak used for this
                                                                       survey point (two bytes of bitwise flags)  */
    NV_FLOAT32      Waveform_Peak_Jitter;                          /*  The jitter of the waveform processing detected peak used for this survey point  */
    NV_FLOAT32      Waveform_Peak_Position;                        /*  The waveform sample index of the waveform processing detected peak used for this
                                                                       survey point  */
    NV_FLOAT32      Waveform_Peak_Pulsewidth;                      /*  The pulsewidth of the waveform processing detected peak used for this survey point  */
    NV_FLOAT32      Waveform_Peak_SNR;                             /*  The signal-to-noise ratio of the waveform processing detected peak used for this
                                                                       survey point  */
    NV_CHAR         Scan_Direction_Flag;                           /*  Scan Direction Flag indicating left-to-right with >0 right-to-left with <0 and
                                                                       non-computable with 0  */
    NV_BOOL         Edge_of_Flightline;                            /*  A boolean flag indicating the scanning direction having shifted  */
    NV_CHAR         Scan_Angle_Rank;                               /*  The angle of the laser output at the nadir rounded to nearest integer  */
    NV_U_INT16      SelectBottomCase;                              /*  SelectBottom Case is for development purposes states which case in SeletBottoms
                                                                       that is used  */
    NV_FLOAT64      Elevation;                                     /*  The calculated elevation for this position (positive above water surface)  */
    NV_U_INT16      Return_Number;                                 /*  The index into the array of returns extracted from the current waveform  */
    NV_U_INT16      Number_of_Returns;                             /*  The total number of returns extracted from the current waveform  */
    NV_FLOAT64      Surface_altitude_interpolated_tide_corrected;  /*  Mean surface altitude adjusted with tide correction  */
    NV_FLOAT64      IR_polarisation_ratio;                         /*  Amplitude ratio orthogonal/parallell polarisaion for infrared  */
    NV_FLOAT32      Spatial_Quality;                               /*  The spatial quality of a measurement  */
    NV_FLOAT64      IR_AMPLITUDE;                                  /*  The amplitude of the parallell infrared pulse in Volts  */
    NV_U_INT16      Scan_Position;                                 /*    */
    NV_FLOAT64      K_Lidar;                                       /*  An estimate of the water column system diffuse attenuation coefficient  */
    NV_FLOAT32      K_Lidar_Average;                               /*    */
    NV_FLOAT32      K_Lidar_Deep_Momentary;                        /*    */
    NV_FLOAT32      K_Lidar_Shallow_Momentary;                     /*    */
    NV_FLOAT64      Relative_reflectivity;                         /*  Relative reflectivity  */
    NV_FLOAT64      Waveform_Peak_Gain;                            /*  Gain at point of peak  */
    NV_FLOAT32      Adjusted_Amplitude;                            /*  Peak amplitude adjusted for gain distance and pulsewidth  */
    NV_FLOAT32      Water_Travel_Time;                             /*  Travel time for pulse through water  */
    NV_FLOAT32      Attenuation_c;                                 /*  Attenuation coefficient c /m  */
    NV_FLOAT32      Absorption_a;                                  /*  Absorption coefficient a /m  */
    NV_FLOAT32      Scattering_s;                                  /*  Scattering coefficient s /m  */
    NV_FLOAT32      Backscattering_Bb;                             /*  Backscattering coefficient Bb /m  */
  } HAWKEYE_RECORD;


  /*  Meta header structure.  */

  typedef struct
  {
    NV_CHAR         FileSignature[6];                              /*  A string identifying the file as a CSS output file.  This should always be "CSSOUT".  */
    NV_INT32        VersionMajor;                                  /*  Header major version number, currently 1  */
    NV_INT32        VersionMinor;                                  /*  Header minor version number, currently 3  */
    NV_INT32        MetaHeaderSize;                                /*  The size of the Meta Header, always 24 bytes  */
    NV_INT32        ContentsHeaderSize;                            /*  The size of the variable size block following the Meta Header, the so-called
                                                                       Contents Header  */
  } HAWKEYE_META_HEADER;


  /*  Contents header structure.  */

  typedef struct
  {
    NV_CHAR         GeneratingSoftware[32];                        /*  Generating software  */
    NV_CHAR         SystemIdentifier[32];                          /*  System ID  */
    NV_INT32        NbrOfPointDataFields;                          /*  Number of point data fields per record  */
    NV_U_INT32      NbrOfPointRecords;                             /*  Number of point data records  */
    NV_INT32        OffsetToPointData;                             /*  Offset from beginning of file to the point data in bytes  */
    NV_INT32        PointRecordSize;                               /*  Point record size in bytes  */
    NV_CHAR         **PointDataFieldDescr;                         /*  Descriptive text for each point data field  */
    NV_CHAR         **PointDataFieldTags;                          /*  Short form point data field tags  */
    NV_U_BYTE       *PointDataMax;                                 /*  Max values for each point data field, stored in the same manner as the point data
                                                                       records.  Must be allocated to be PointRecordSize bytes.  */
    NV_U_BYTE       *PointDataMin;                                 /*  Min values for each point data field, stored in the same manner as the point data
                                                                       records.  Must be allocated to be PointRecordSize bytes.  */
    HAWKEYE_RECORD  MaxValues;                                     /*  Max values for each point data field broken out into a HAWKEYE_RECORD structure  */
    HAWKEYE_RECORD  MinValues;                                     /*  Min values for each point data field broken out into a HAWKEYE_RECORD structure  */
    E_HawkeyeCSSOutputDataFieldTypes *PointDataFieldTypes;         /*  Data types for each point data field. Must allocate NbrOfPointDataFields entries.  */
    NV_CHAR         SoftwareVersion[16];                           /*  Software version  */
    NV_U_INT16      SurveyYear;                                    /*  Year of the beginning of the survey  */
    NV_U_INT16      SurveyDay;                                     /*  Julian day (day of year) of the beginning of the survey  */
    NV_U_INT16      ProcessingYear;                                /*  Year the data was processed  */
    NV_U_INT16      ProcessingDay;                                 /*  Julian day (day of year) the data was processed  */
    NV_CHAR         UTMDatum[16];                                  /*  A character array identifying the datum for the UTM system used. Possible values
                                                                       are e.g. "NAD27", "NAD83" or "WGS84".  */
    NV_U_INT16      UTMZone;                                       /*  UTM zone from 1 to 60  */
    NV_INT32        UTMSouth;                                      /*  Indicates northern (0) or southern (1) hemisphere  */
    NV_CHAR         UTMUnits[16];                                  /*  A char array identifying the units of the UTM system. If this is an empty string,
                                                                       the units shall default to "m".  */
    HAWKEYE_RECORD_AVAILABILITY available;                         /*  Availability flags derived from PointDataFieldFlags  */
    NV_BYTE         data_type;                                     /*  Either HAWKEYE_HYDRO_BIN_DATA or HAWKEYE_TOPO_BIN_DATA.  So we don't have to keep
                                                                       parsing the file name in applications.  */
  } HAWKEYE_CONTENTS_HEADER;


  /*  SFM = Shot File Map  */

  #define HAWKEYE_SFM_RECORD_SIZE         20

  typedef struct
  {
    NV_FLOAT64      Timestamp;                                     /*  Onboard Inertial/GPS Navigation System Timestamp. The unit is
                                                                       seconds from the latest GPS time reset.  It should match the
                                                                       .dat record Timestamp.  */
    NV_U_INT32      FilePosIntermediate;                           /*  Offset to intermediate file record (TBD).  */
    NV_U_INT32      FilePosOriginal;                               /*  Offset to beginning of record in .dat file.  */
    NV_INT32        FileIndex;                                     /*  File number (FL004_2871181_Flightline_1_HD.sfm has records for
                                                                       HD_FL004_2871181_Flightline_1_000.dat and HD_FL004_2871181_Flightline_1_001.dat,
                                                                       if the index is 1 the record is in the second file).  */
  } HAWKEYE_SFM_RECORD;


  #define HAWKEYE_COMMON_HEADER_SIZE      8

  typedef struct
  {
    NV_U_INT16      msgId;
    NV_U_INT16      msgSize;                                       /*  Number of bytes in the msg (header and data).  */
    NV_U_INT32      senderSeqMsgId;
  } HAWKEYE_COMMON_HEADER;


  #define HAWKEYE_CHANNEL_INFO_SIZE       16

  typedef struct
  {
    NV_U_BYTE       dataValid; 
    NV_U_BYTE       waveformFormat;
    NV_U_BYTE       nbrBytesInOneSample; 
    NV_U_BYTE       padding;
    NV_U_INT32      pulseLength;                                   /*  Number of bytes.  */
    NV_U_INT32      reflexLength;                                  /*  Number of bytes.  */
    NV_U_INT32      intermissionLength;
  } HAWKEYE_CHANNEL_INFO;


  /*

  WTF, over!?

  <dGainQuotaShallow type ="double" value="1.0651"/>
  <dGainQuotaShallow type ="double" value="0.9760"/>
  <dGainQuotaShallow type ="double" value="0.7928"/>
  <dGainQuotaShallow type ="double" value="1.1661"/>
  <dGainQuotaDeep type ="double" value="0.6589"/>
  <dGainQuotaDeep type ="double" value="0.9244"/>
  <dGainQuotaDeep type ="double" value="1.3436"/>
  <dGainQuotaDeep type ="double" value="1.0731"/>

  */

  #define HAWKEYE_SHOT_DATA_HYDRO_SIZE    36 + hydroChannelID_nrOfChannels * HAWKEYE_CHANNEL_INFO_SIZE

  /*  The above value is the size of the HAWKEYE_SHOT_DATA_HYDRO structure minus the size of the waveforms.  The waveforms are NOT stored in the order of the 
      HAWKEYE_CHANNEL_INFO structure.  That is:

      IR parallel (invert)
      IR ortho (invert)
      Gain Monitor Shallow
      Gain Monitor Deep
      Green Shallow 0
      Green Deep 0
      Green Shallow 1 (invert ?)
      Green Deep 1
      Green Shallow 2
      Green Deep 2
      Green Shallow 3
      Green Deep 3
  */

  typedef struct
  {
    NV_FLOAT64      pcTimestamp;
    NV_FLOAT64      Timestamp;
    NV_FLOAT64      scannerTimestamp;
    NV_U_INT32      seqShotID;
    NV_FLOAT32      scannerXaxisAngle;
    NV_FLOAT32      scannerYaxisAngle;
    HAWKEYE_CHANNEL_INFO channelInfo[hydroChannelID_nrOfChannels];
    NV_U_BYTE       wave_pulse[hydroChannelID_nrOfChannels][500];
    NV_U_BYTE       wave_reflex[hydroChannelID_nrOfChannels][500];
  } HAWKEYE_SHOT_DATA_HYDRO;


  /*  We're not using this just yet.  */

  typedef struct
  {
    NV_FLOAT64      tdcTimestamp;
    NV_FLOAT64      pcTimestamp;
    NV_FLOAT64      Timestamp;
    NV_U_INT32      seqShotID;
    NV_U_INT32      dataValid;
    NV_U_INT32      scannerXaxisAngle;
    NV_FLOAT32      scannerYaxisAngle;
    NV_FLOAT32      reflexTime[HAWKEYE_NUMBER_OF_TOPO_PIXELS][HAWKEYE_NUMBER_OF_TOPO_REFLEXES];
  } HAWKEYE_SHOT_DATA_TOPO;


  HAWKEYE_DLL NV_INT32 hawkeye_open_file (const NV_CHAR *path, HAWKEYE_META_HEADER **MetaHeader, HAWKEYE_CONTENTS_HEADER **ContentsHeader, NV_INT32 mode);
  HAWKEYE_DLL NV_INT32 hawkeye_close_file (NV_INT32 hnd);
  HAWKEYE_DLL NV_INT32 hawkeye_read_record (NV_INT32 hnd, NV_INT32 recnum, HAWKEYE_RECORD *hawkeye_record);
  HAWKEYE_DLL NV_INT32 hawkeye_update_record (NV_INT32 hnd, NV_INT32 recnum, HAWKEYE_RECORD hawkeye_record);
  HAWKEYE_DLL NV_INT32 hawkeye_read_sfm_record (NV_INT32 hnd, NV_INT32 recnum, HAWKEYE_SFM_RECORD *hawkeye_sfm_record);
  HAWKEYE_DLL NV_INT32 hawkeye_read_shot_data_hydro (NV_INT32 hnd, NV_INT32 recnum, HAWKEYE_SHOT_DATA_HYDRO *hawkeye_shot_data_hydro);
  HAWKEYE_DLL NV_INT32 hawkeye_get_errno ();
  HAWKEYE_DLL NV_CHAR *hawkeye_strerror ();
  HAWKEYE_DLL void hawkeye_perror ();
  HAWKEYE_DLL void hawkeye_dump_record (NV_INT32 hnd, HAWKEYE_RECORD hawkeye_record);


#ifdef  __cplusplus
}
#endif

#endif
