
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H


//  IMPORTANT NOTE: Don't make the descriptions longer than 29 characters or you'll blow out the PFM attribute name length.


QString time_attribute_name = "Time (POSIX minutes)";

NV_FLOAT32 time_attribute_def[3] = {PFMWDB_NULL_TIME, -PFMWDB_NULL_TIME, 1.0};


//  To add to these you must change the ???_ATTRIBUTES definitions in pfmLoadDef.hpp.  Names must not exceed
//  29 characters.  Because I always forget this - NEVER USE SLASHES (/) IN ANY OF THESE STRINGS!  If you 
//  read the documentation for QSettings you'll understand why.

QString gsf_attribute_name[GSF_ATTRIBUTES] = 
  {"GSF Heading", "GSF Pitch", "GSF Roll", "GSF Heave",
   "GSF Course", "GSF Speed", "GSF Height above ellipsoid", "GSF Ellipsoid datum distance", "GSF Tide correction",
   "GSF Tide type", "GSF Across track distance", "GSF Along track distance", "GSF Travel time", "GSF Beam angle",
   "GSF Beam angle forward", "GSF Mean calibrated amp.", "GSF Mean relative amp.", "GSF Echo width", "GSF Simrad Quality factor",
   "GSF Signal to noise", "GSF Receive heave", "GSF Depth error", "GSF Across track error", "GSF Along track error",
   "GSF Reson quality flag", "GSF Beam flag", "GSF Beam angle forward", "GSF Transit sector", "GSF Detection info",
   "GSF Incident beam adj.", "GSF System cleaning", "GSF Doppler correction"};

NV_FLOAT32 gsf_attribute_def[GSF_ATTRIBUTES][3] = 
  {{0.0, 360.0, 100.0}, {-30.0, 30.0, 100.0}, {-50.0, 50.0, 100.0}, {-25.0, 25.0, 100.0},
   {0.0, 360.0, 100.0}, {0.0, 30.0, 100.0}, {-200.0, 200.0, 1000.0}, {-200.0, 200.0, 1000.0}, {-20.0, 20.0, 1000.0},
   {0.0, 4.0, 1.0}, {-50000.0, 50000.0, 100.0}, {-20000.0, 20000.0, 100.0}, {0.0, 20.0, 1000.0}, {-180.0, 180.0, 100.0},
   {-180.0, 180.0, 100.0}, {-200.0, 200.0, 100.0}, {-200.0, 200.0, 100.0}, {-200.0, 200.0, 100.0}, {0.0, 1000.0, 10.0},
   {0.0, 100.0, 100.0}, {-100.0, 100.0, 100.0}, {-1000.0, 1000.0, 100.0}, {-5000.0, 5000.0, 100.0}, {-5000.0, 5000.0, 100.0},
   {0.0, 255.0, 1.0}, {0.0, 255, 1.0}, {-90.0, 90.0, 100.0}, {0.0, 65535.0, 1.0}, {0.0, 65535.0, 1.0},
   {-90.0, 90.0, 100.0}, {0.0, 65535.0, 1.0}, {-5000.0, 5000.0, 100.0}};


QString hof_attribute_name[HOF_ATTRIBUTES] = 
  {"HOF Haps version", "HOF Position confidence", "HOF Status", "HOF Suggested DKS",
   "HOF Suspect status", "HOF Tide status", "HOF Abbreviated depth conf.", "HOF Abbreviated sec dep conf.", "HOF Data type",
   "HOF Land mode", "HOF Classification status", "HOF Wave height", "HOF Elevation", "HOF Topo value",
   "HOF Altitude", "HOF KGPS topo value", "HOF KGPS datum value", "HOF KGPS water level", "HOF K value (TBD)",
   "HOF Intensity (TBD)", "HOF Bottom confidence", "HOF Secondary depth conf.", "HOF Nadir angle", "HOF Scanner azimuth",
   "HOF Surface APD FOM", "HOF Surface IR FOM", "HOF Surface Raman FOM", "HOF Coded depth conf.", "HOF Coded secondary depth conf.",
   "HOF Warnings (1)", "HOF Warnings (2)", "HOF Warnings (3)", "HOF First calculated bfom*10", "HOF Second calculated bfom*10",
   "HOF First bottom run req.", "HOF Second bottom run req.", "HOF Primary return bin", "HOF Secondary return bin", "HOF PMT bottom bin",
   "HOF PMT secondary bottom bin", "HOF APD bottom bin", "HOF APD secondary bottom bin", "HOF Bottom channel", "HOF Secondary bottom channel",
   "HOF Surface bin from APD", "HOF Surface bin from IR", "HOF Surface bin from Raman", "HOF Surface channel used"};

NV_FLOAT32 hof_attribute_def[HOF_ATTRIBUTES][3] = 
  {{0.0, 10.0, 1.0}, {-100.0, 100.0, 1.0}, {0.0, 126.0, 1.0}, {0.0, 126.0, 1.0},
   {0.0, 126.0, 1.0}, {0.0, 4.0, 1.0}, {-100.0, 100.0, 1.0}, {-100.0, 100.0, 1.0}, {0.0, 1.0, 1.0},
   {0.0, 1.0, 1.0}, {0.0, 4096.0, 1.0}, {-25.0, 25.0, 100.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {0.0, 1000.0, 100.0}, {-1000.0, 1000.0, 100.0}, {-1000.0, 1000.0, 100.0}, {-1000.0, 1000.0, 100.0}, {0.0, 126.0, 1.0},
   {0.0, 1000.0, 1.0}, {0.0, 126.0, 1.0}, {0.0, 126.0, 1.0}, {-45.0, 45.0, 100.0}, {-180.0, 180.0, 100.0},
   {0.0, 1000.0, 1000.0}, {0.0, 1000.0, 1000.0}, {0.0, 1000.0, 1000.0}, {0.0, 1000000.0, 1.0}, {0.0, 1000000.0, 1.0},
   {0.0, 32766.0, 1.0}, {0.0, 32766.0, 1.0}, {0.0, 32766.0, 1.0}, {0.0, 1022.0, 1.0}, {0.0, 1022.0, 1.0},
   {0.0, 15.0, 1.0}, {0.0, 15.0, 1.0}, {0.0, 510.0, 1.0}, {0.0, 510.0, 1.0}, {0.0, 510.0, 1.0},
   {0.0, 510.0, 1.0}, {0.0, 510.0, 1.0}, {0.0, 510.0, 1.0}, {0.0, 2.0, 1.0}, {0.0, 2.0, 1.0},
   {0.0, 510.0, 1.0}, {0.0, 510.0, 1.0}, {0.0, 510.0, 1.0}, {0.0, 2.0, 1.0}};


QString tof_attribute_name[TOF_ATTRIBUTES] = 
  {"TOF Classification status", "TOF Altitude", "TOF First return intensity", "TOF Last return intensity",
   "TOF First return confidence", "TOF Last return confidence", "TOF Nadir angle", "TOF Scanner azimuth", "TOF Position confidence"};

NV_FLOAT32 tof_attribute_def[TOF_ATTRIBUTES][3] = 
  {{0.0, 4094.0, 1.0}, {0.0, 1000.0, 100.0}, {0.0, 126.0, 1.0}, {0.0, 126.0, 1.0},
   {0.0, 126.0, 1.0}, {0.0, 126.0, 1.0}, {-45.0, 45.0, 100.0}, {-180.0, 180.0, 100.0}, {0.0, 126.0, 1.0}};


QString wlf_attribute_name[WLF_ATTRIBUTES] = 
  {"WLF Water surface", "WLF Z offset", "WLF Number of returns", "WLF Return number",
   "WLF Point source ID", "WLF Scanner angle", "WLF Edge of flightline flag", "WLF Intensity value", "WLF RGB red value",
   "WLF RGB green value", "WLF RGB blue value", "WLF Reflectance value", "WLF Classification"};

NV_FLOAT32 wlf_attribute_def[WLF_ATTRIBUTES][3] = 
  {{-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {0.0, 32.0, 1.0}, {0.0, 32.0, 1.0},
   {0.0, 262143.0, 1.0}, {-180.0, 180.0, 100.0}, {0.0, 2.0, 1.0}, {0.0, 1000.0, 100.0}, {0.0, 16777214.0, 1.0},
   {0.0, 16777214.0, 1.0}, {0.0, 16777214.0, 1.0}, {0.0, 20000.0, 100.0}, {0.0, 4094.0, 1.0}};


QString czmil_attribute_name[CZMIL_ATTRIBUTES] = 
  {"CZMIL Deep Elev[1]", "CZMIL Deep Elev[2]", "CZMIL Deep Elev[3]", "CZMIL Deep Classification", 
   "CZMIL Shallow 1 Elev[1]", "CZMIL Shallow 1 Elev[2]", "CZMIL Shallow 1 Elev[3]", "CZMIL Shallow 1 Classification",
   "CZMIL Shallow 2 Elev[1]", "CZMIL Shallow 2 Elev[2]", "CZMIL Shallow 2 Elev[3]", "CZMIL Shallow 2 Classification",
   "CZMIL Shallow 3 Elev[1]", "CZMIL Shallow 3 Elev[2]", "CZMIL Shallow 3 Elev[3]", "CZMIL Shallow 3 Classification",
   "CZMIL Shallow 4 Elev[1]", "CZMIL Shallow 4 Elev[2]", "CZMIL Shallow 4 Elev[3]", "CZMIL Shallow 4 Classification",
   "CZMIL Shallow 5 Elev[1]", "CZMIL Shallow 5 Elev[2]", "CZMIL Shallow 5 Elev[3]", "CZMIL Shallow 5 Classification",
   "CZMIL Shallow 6 Elev[1]", "CZMIL Shallow 6 Elev[2]", "CZMIL Shallow 6 Elev[3]", "CZMIL Shallow 6 Classification",
   "CZMIL Shallow 7 Elev[1]", "CZMIL Shallow 7 Elev[2]", "CZMIL Shallow 7 Elev[3]", "CZMIL Shallow 7 Classification",
   "CZMIL IR Elev[1]", "CZMIL IR Elev[2]", "CZMIL IR Elev[3]", "CZMIL IR Classification"};

NV_FLOAT32 czmil_attribute_def[CZMIL_ATTRIBUTES][3] = 
  {{-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0},
   {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}, {-1000.0, 1000.0, 1000.0}};


QString bag_attribute_name[BAG_ATTRIBUTES] = 
  {"BAG Uncertainty"};

NV_FLOAT32 bag_attribute_def[BAG_ATTRIBUTES][3] = 
  {{0.0, 1001.0, 1000.0}};


QString hawkeye_attribute_name[HAWKEYE_ATTRIBUTES] = 
  {"HWK Surface Altitude", "HWK Interp. Surface Altitude", "HWK Surface Northing StdDev", "HWK Surface Easting StdDev",
   "HWK Surface Altitude StdDev", "HWK Point Altitude", "HWK Point Northing StdDev", "HWK Point Easting StdDev",
   "HWK Point Altitude StdDev", "HWK Azimuth", "HWK Air Nadir Angle", "HWK Water Nadir Angle",
   "HWK Slant range", "HWK Slant Range Comp Model", "HWK Wave Height", "HWK Water Quality Correction",
   "HWK Tidal Correction", "HWK Depth Amplitude", "HWK Depth Class", "HWK Depth Confidence",
   "HWK Pixel Index", "HWK Scanner Angle X", "HWK Scanner Angle Y", "HWK Aircraft Altitude",
   "HWK Aircraft Roll", "HWK Aircraft Pitch", "HWK Aircraft Heading", "HWK Aircraft Northing StdDev",
   "HWK Aircraft Easting StdDev", "HWK Aircraft Altitude StdDev", "HWK Aircraft Roll StdDev", "HWK Aircraft Pitch StdDev",
   "HWK Aircraft Heading StdDev", "HWK Extracted Waveform Attr.", "HWK Receiver Data Used", "HWK Manual Screening Flags",
   "HWK Waveform Peak Amplitude", "HWK Waveform Peak Class.", "HWK Waveform Peak Contrast", "HWK Waveform Peak Debug Flags",
   "HWK Waveform Peak Attributes", "HWK Waveform Peak Jitter", "HWK Waveform Peak Position", "HWK Waveform Peak Pulsewidth",
   "HWK Waveform Peak SNR", "HWK Scan Direction Flag", "HWK Edge of Flightline", "HWK Scan Angle Rank",
   "HWK SelectBottomCase", "HWK Return Number", "HWK Number of Returns", "HWK Srfc alt. intrp. tid cor.",
   "HWK IR polarisation ratio", "HWK Spatial Quality", "HWK IR AMPLITUDE", "HWK Scan Position",
   "HWK K Lidar", "HWK K Lidar Average", "HWK K Lidar Deep Momentary", "HWK K Lidar Shallow Momentary",
   "HWK Relative reflectivity", "HWK Waveform Peak Gain", "HWK Adjusted Amplitude", "HWK Water Travel Time",
   "HWK Attenuation c", "HWK Absorption a", "HWK Scattering s", "HWK Backscattering Bb"};

NV_FLOAT32 hawkeye_attribute_def[HAWKEYE_ATTRIBUTES][3] = 
  {{0.0, 1000.0, 100.0}, {0.0, 1000.0, 100.0}, {0.0, 126.0, 100.0}, {0.0, 126.0, 100.0},
   {0.0, 126.0, 100.0}, {0.0, 1000.0, 100.0}, {0.0, 126.0, 100.0}, {0.0, 126.0, 100.0},
   {0.0, 126.0, 100.0}, {0.0, 360.0, 100.0}, {-100.0, 100.0, 100.0}, {-100.0, 100.0, 100.0},
   {0.0, 1000.0, 100.0}, {0.0, 65534.0, 1.0}, {0.0, 20.0, 100.0}, {-1000.0, 1000.0, 100.0},
   {-1000.0, 1000.0, 100.0}, {0.0, 1023.0, 100.0}, {0.0, 65534.0, 1.0}, {0.0, 100000.0, 100.0},
   {0.0, 256.0, 1.0}, {0.0, 360.0, 100.0}, {0.0, 360.0, 100.0}, {0.0, 1000.0, 100.0},
   {-40.0, 40.0, 100.0}, {-40.0, 40.0, 100.0}, {0.0, 360.0, 100.0}, {0.0, 126.0, 100.0},
   {0.0, 126.0, 100.0}, {0.0, 126.0, 100.0}, {0.0, 126.0, 100.0}, {0.0, 126.0, 100.0},
   {0.0, 126.0, 100.0}, {-32766.0, 32766.0, 1.0}, {-32766.0, 32766.0, 1.0}, {0.0, 256.0, 1.0}, 
   {0.0, 100000.0, 100.0}, {0.0, 65534.0, 1.0}, {-10000.0, 10000.0, 100.0}, {-32766.0, 32766.0, 1.0},
   {-32766.0, 32766.0, 1.0}, {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0},
   {-10000.0, 10000.0, 100.0}, {-127.0, 127.0, 1.0}, {0.0, 1.0, 1.0}, {-127.0, 127.0, 1.0},
   {0.0, 65534.0, 1.0}, {0.0, 65534.0, 1.0}, {0.0, 65534.0, 1.0}, {-1000.0, 1000.0, 100.0},
   {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {0.0, 65534.0, 1.0},
   {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0},
   {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {0.0, 1000000.0, 100.0},
   {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}, {-10000.0, 10000.0, 100.0}};


#endif
