#include "hawkeyeMonitor.hpp"

void dumpRecord (QTextEdit *listBox, HAWKEYE_RECORD hawkeye_record, HAWKEYE_RECORD_AVAILABILITY available, NV_INT32 pos_format)
{
  NV_INT32 year, day, hour, minute, month, mday;
  NV_FLOAT32 second;
  NV_FLOAT64 deg, min, sec;
  NV_CHAR ltstring[40], lnstring[40], hem;
  QString string;


  listBox->clear ();


  //  Populate the textEdit box.

  if (available.Timestamp)
    {
      cvtime (hawkeye_record.tv_sec, hawkeye_record.tv_nsec, &year, &day, &hour, &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;
      string.sprintf (hawkeyeMonitor::tr ("<b>Date/Time : </b>%d-%02d-%02d (%03d) %02d:%02d:%05.2f<br>").toAscii (), year + 1900, month,
                      mday, day, hour, minute, second);
      listBox->insertHtml (string);

      string.sprintf (hawkeyeMonitor::tr ("<b>Timestamp : </b>%.9f<br>").toAscii (), hawkeye_record.Timestamp);
      listBox->insertHtml (string);
    }

  if (available.Point_Latitude)
    {
      strcpy (ltstring, fixpos (hawkeye_record.Point_Latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
      string.sprintf (hawkeyeMonitor::tr ("<b>Point Latitude : </b>%s<br>").toAscii (), ltstring);
      listBox->insertHtml (string);
    }
  if (available.Point_Longitude)
    {
      strcpy (lnstring, fixpos (hawkeye_record.Point_Longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
      string.sprintf (hawkeyeMonitor::tr ("<b>Point Longitude : </b>%s<br>").toAscii (), lnstring);
      listBox->insertHtml (string);
    }
  if (available.Point_Northing)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Point Northing </b>%f<br>").toAscii (), hawkeye_record.Point_Northing);
      listBox->insertHtml (string);
    }
  if (available.Point_Easting)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Point Easting </b>%f<br>").toAscii (), hawkeye_record.Point_Easting);
      listBox->insertHtml (string);
    }
  if (available.Point_Altitude)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Point Altitude </b>%f<br>").toAscii (), hawkeye_record.Point_Altitude);
      listBox->insertHtml (string);
    }
  if (available.Surface_Altitude)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface Altitude </b>%f<br>").toAscii (), hawkeye_record.Surface_Altitude);
      listBox->insertHtml (string);
    }
  if (available.Interpolated_Surface_Altitude)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Interpolated Surface Altitude </b>%f<br>").toAscii (), hawkeye_record.Interpolated_Surface_Altitude);
      listBox->insertHtml (string);
    }
  if (available.Receiver_Data_Used)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Receiver Data Used </b>%d<br>").toAscii (), hawkeye_record.Receiver_Data_Used);
      listBox->insertHtml (string);
    }
  if (available.Surface_Latitude)
    {
      strcpy (ltstring, fixpos (hawkeye_record.Surface_Latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface Latitude : </b>%s<br>").toAscii (), ltstring);
      listBox->insertHtml (string);
    }

  if (available.Surface_Longitude)
    {
      strcpy (lnstring, fixpos (hawkeye_record.Surface_Longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface Longitude : </b>%s<br>").toAscii (), lnstring);
      listBox->insertHtml (string);
    }

  if (available.Surface_Northing)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface Northing : </b>%f<br>").toAscii (), hawkeye_record.Surface_Northing);
      listBox->insertHtml (string);
    }
  if (available.Surface_Easting)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface Easting </b>%f<br>").toAscii (), hawkeye_record.Surface_Easting);
      listBox->insertHtml (string);
    }
  if (available.Surface_Northing_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface Northing StdDev </b>%f<br>").toAscii (), hawkeye_record.Surface_Northing_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Surface_Easting_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface Easting StdDev </b>%f<br>").toAscii (), hawkeye_record.Surface_Easting_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Surface_Altitude_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface Altitude StdDev </b>%f<br>").toAscii (), hawkeye_record.Surface_Altitude_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Point_Northing_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Point Northing StdDev </b>%f<br>").toAscii (), hawkeye_record.Point_Northing_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Point_Easting_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Point Easting StdDev </b>%f<br>").toAscii (), hawkeye_record.Point_Easting_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Point_Altitude_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Point Altitude StdDev </b>%f<br>").toAscii (), hawkeye_record.Point_Altitude_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Azimuth)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Azimuth </b>%f<br>").toAscii (), hawkeye_record.Azimuth);
      listBox->insertHtml (string);
    }
  if (available.Air_Nadir_Angle)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Air Nadir Angle </b>%f<br>").toAscii (), hawkeye_record.Air_Nadir_Angle);
      listBox->insertHtml (string);
    }
  if (available.Water_Nadir_Angle)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Water Nadir Angle </b>%f<br>").toAscii (), hawkeye_record.Water_Nadir_Angle);
      listBox->insertHtml (string);
    }
  if (available.Slant_range)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Slant range </b>%f<br>").toAscii (), hawkeye_record.Slant_range);
      listBox->insertHtml (string);
    }
  if (available.Slant_Range_Comp_Model)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Slant Range Comp_Model </b>%d<br>").toAscii (), hawkeye_record.Slant_Range_Comp_Model);
      listBox->insertHtml (string);
    }
  if (available.Wave_Height)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Wave Height </b>%f<br>").toAscii (), hawkeye_record.Wave_Height);
      listBox->insertHtml (string);
    }
  if (available.Water_Quality_Correction)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Water Quality Correction </b>%f<br>").toAscii (), hawkeye_record.Water_Quality_Correction);
      listBox->insertHtml (string);
    }
  if (available.Tidal_Correction)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Tidal Correction </b>%f<br>").toAscii (), hawkeye_record.Tidal_Correction);
      listBox->insertHtml (string);
    }
  if (available.Depth)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Depth </b>%f<br>").toAscii (), hawkeye_record.Depth);
      listBox->insertHtml (string);
    }
  if (available.Depth_Amplitude)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Depth Amplitude </b>%f<br>").toAscii (), hawkeye_record.Depth_Amplitude);
      listBox->insertHtml (string);
    }
  if (available.Depth_Class)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Depth Class </b>%d<br>").toAscii (), hawkeye_record.Depth_Class);
      listBox->insertHtml (string);
    }
  if (available.Depth_Confidence)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Depth Confidence </b>%f<br>").toAscii (), hawkeye_record.Depth_Confidence);
      listBox->insertHtml (string);
    }
  if (available.Pixel_Index)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Pixel Index </b>%d<br>").toAscii (), hawkeye_record.Pixel_Index);
      listBox->insertHtml (string);
    }
  if (available.Scanner_Angle_X)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Scanner Angle X </b>%f<br>").toAscii (), hawkeye_record.Scanner_Angle_X);
      listBox->insertHtml (string);
    }
  if (available.Scanner_Angle_Y)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Scanner Angle Y </b>%f<br>").toAscii (), hawkeye_record.Scanner_Angle_Y);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Latitude)
    {
      strcpy (ltstring, fixpos (hawkeye_record.Aircraft_Latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Latitude : </b>%s<br>").toAscii (), ltstring);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Longitude)
    {
      strcpy (lnstring, fixpos (hawkeye_record.Aircraft_Longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Longitude : </b>%s<br>").toAscii (), lnstring);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Northing)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Northing </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Northing);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Easting)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Easting </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Easting);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Altitude)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Altitude </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Altitude);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Roll)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Roll </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Roll);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Pitch)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Pitch </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Pitch);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Heading)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Heading </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Heading);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Northing_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Northing StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Northing_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Easting_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Easting StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Easting_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Altitude_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Altitude StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Altitude_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Roll_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Roll StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Roll_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Pitch_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Pitch StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Pitch_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Aircraft_Heading_StdDev)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Aircraft Heading StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Heading_StdDev);
      listBox->insertHtml (string);
    }
  if (available.Extracted_Waveform_Attributes)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Extracted Waveform Attributes </b>%d<br>").toAscii (), hawkeye_record.Extracted_Waveform_Attributes);
      listBox->insertHtml (string);
    }
  if (available.Manual_Output_Screening_Flags)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Manual Output Screening Flags </b>%x<br>").toAscii (), hawkeye_record.Manual_Output_Screening_Flags);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Amplitude)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Amplitude </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Amplitude);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Classification)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Classification </b>%d<br>").toAscii (), hawkeye_record.Waveform_Peak_Classification);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Contrast)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Contrast </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Contrast);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Debug_Flags)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Debug Flags </b>%x<br>").toAscii (), hawkeye_record.Waveform_Peak_Debug_Flags);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Attributes)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Attributes </b>%d<br>").toAscii (), hawkeye_record.Waveform_Peak_Attributes);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Jitter)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Jitter </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Jitter);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Position)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Position </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Position);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Pulsewidth)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Pulsewidth </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Pulsewidth);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_SNR)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak SNR </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_SNR);
      listBox->insertHtml (string);
    }
  if (available.Scan_Direction_Flag)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Scan Direction Flag </b>%d<br>").toAscii (), hawkeye_record.Scan_Direction_Flag);
      listBox->insertHtml (string);
    }
  if (available.Edge_of_Flightline)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Edge of Flightline </b>%d<br>").toAscii (), hawkeye_record.Edge_of_Flightline);
      listBox->insertHtml (string);
    }
  if (available.Scan_Angle_Rank)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Scan Angle Rank </b>%d<br>").toAscii (), hawkeye_record.Scan_Angle_Rank);
      listBox->insertHtml (string);
    }
  if (available.SelectBottomCase)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>SelectBottomCase </b>%d<br>").toAscii (), hawkeye_record.SelectBottomCase);
      listBox->insertHtml (string);
    }
  if (available.Elevation)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Elevation </b>%f<br>").toAscii (), hawkeye_record.Elevation);
      listBox->insertHtml (string);
    }
  if (available.Return_Number)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Return Number </b>%d<br>").toAscii (), hawkeye_record.Return_Number);
      listBox->insertHtml (string);
    }
  if (available.Number_of_Returns)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Number of Returns </b>%d<br>").toAscii (), hawkeye_record.Number_of_Returns);
      listBox->insertHtml (string);
    }
  if (available.Surface_altitude_interpolated_tide_corrected)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Surface alt. interp. tide cor. </b>%f<br>").toAscii (), hawkeye_record.Surface_altitude_interpolated_tide_corrected);
      listBox->insertHtml (string);
    }
  if (available.IR_polarisation_ratio)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>IR polarisation ratio </b>%f<br>").toAscii (), hawkeye_record.IR_polarisation_ratio);
      listBox->insertHtml (string);
    }
  if (available.Spatial_Quality)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Spatial Quality </b>%f<br>").toAscii (), hawkeye_record.Spatial_Quality);
      listBox->insertHtml (string);
    }
  if (available.IR_AMPLITUDE)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>IR AMPLITUDE </b>%f<br>").toAscii (), hawkeye_record.IR_AMPLITUDE);
      listBox->insertHtml (string);
    }
  if (available.Scan_Position)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Scan Position </b>%d<br>").toAscii (), hawkeye_record.Scan_Position);
      listBox->insertHtml (string);
    }
  if (available.K_Lidar)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>K Lidar </b>%f<br>").toAscii (), hawkeye_record.K_Lidar);
      listBox->insertHtml (string);
    }
  if (available.K_Lidar_Average)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>K Lidar Average </b>%f<br>").toAscii (), hawkeye_record.K_Lidar_Average);
      listBox->insertHtml (string);
    }
  if (available.K_Lidar_Deep_Momentary)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>K Lidar Deep Momentary </b>%f<br>").toAscii (), hawkeye_record.K_Lidar_Deep_Momentary);
      listBox->insertHtml (string);
    }
  if (available.K_Lidar_Shallow_Momentary)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>K Lidar Shallow Momentary </b>%f<br>").toAscii (), hawkeye_record.K_Lidar_Shallow_Momentary);
      listBox->insertHtml (string);
    }
  if (available.Relative_reflectivity)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Relative reflectivity </b>%f<br>").toAscii (), hawkeye_record.Relative_reflectivity);
      listBox->insertHtml (string);
    }
  if (available.Waveform_Peak_Gain)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Waveform Peak Gain </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Gain);
      listBox->insertHtml (string);
    }
  if (available.Adjusted_Amplitude)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Adjusted Amplitude </b>%f<br>").toAscii (), hawkeye_record.Adjusted_Amplitude);
      listBox->insertHtml (string);
    }
  if (available.Water_Travel_Time)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Water Travel Time </b>%f<br>").toAscii (), hawkeye_record.Water_Travel_Time);
      listBox->insertHtml (string);
    }
  if (available.Attenuation_c)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Attenuation c </b>%f<br>").toAscii (), hawkeye_record.Attenuation_c);
      listBox->insertHtml (string);
    }
  if (available.Absorption_a)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Absorption a </b>%f<br>").toAscii (), hawkeye_record.Absorption_a);
      listBox->insertHtml (string);
    }
  if (available.Scattering_s)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Scattering s </b>%f<br>").toAscii (), hawkeye_record.Scattering_s);
      listBox->insertHtml (string);
    }
  if (available.Backscattering_Bb)
    {
      string.sprintf (hawkeyeMonitor::tr ("<b>Backscattering Bb </b>%f<br>").toAscii (), hawkeye_record.Backscattering_Bb);
      listBox->insertHtml (string);
    }
}
