
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



QString fileText = 
  globalPage::tr ("Enter the maximum number of input files that will be loaded.  Remember that this number will include "
                  "files that will be appended later.  This number determines the number of bits used to store the file "
                  "number field in the index file.  The number will be adjusted upward to the next higher power of 2 "
                  "in order to maximize the use of the bits.<br><br>"
                  "<b>DO NOT adjust this value unless you know with certainty what the final number of input files will "
                  "be!</b>");

QString lineText = 
  globalPage::tr ("Enter the maximum number of input lines that will be loaded.  Remember that this number will include "
                  "lines that will be appended later.  This number determines the number of bits used to store the line "
                  "number field in the index file.  The number will be adjusted upward to the next higher power of 2 "
                  "in order to maximize the use of the bits.<br><br>"
                  "<b>DO NOT adjust this value unless you know with certainty what the final number of input lines will "
                  "be!</b>");

QString pingText = 
  globalPage::tr ("Enter the maximum number of records that will be in any of the input files.  Remember that this number "
                  "will include data from files that will be appended later.  This number determines the number of bits "
                  "used to store the record number field in the index file.  The number will be adjusted upward to the "
                  "next higher power of 2 in order to maximize the use of the bits.<br><br>"
                  "<b>DO NOT adjust this value unless you know with certainty what the largest number of records in any "
                  "input file will be!</b>");

QString beamText = 
  globalPage::tr ("Enter the maximum number of subrecords that will be in any input record.  Remember that this number "
                  "will include data from files that will be appended later.  The subrecord number equates to beam "
                  "numbers for multibeam sonar records.  This number determines the number of bits used to store the "
                  "subrecord number field in the index file.  The number will be adjusted upward to the next higher power "
                  "of 2 in order to maximize the use of the bits.<br><br>"
                  "<b>DO NOT adjust this value unless you know with certainty what the final number of input files will "
                  "be!</b><br<br>"
                  "<b>IMPORTANT NOTE: This field is used for other things when loading LIDAR data.  Do not set to less "
                  "than 127 for LIDAR data.</b>");

QString hofOldText = 
  globalPage::tr ("Selecting this button will cause all HOF data to be loaded in the old PFM_SHOALS_1K_DATA form.  "
                  "This is compatible with GCS.  Only the primary return will be loaded into the PFM.  If this button "
                  "is not checked then the data will be loaded in the new PFM_CHARTS_HOF_DATA form which includes both "
                  "the primary and secondary return.  <b>Only check this button if you are planning to do flight line "
                  "reprocessing in GCS.</b>");

QString hofNullText = 
  globalPage::tr ("Selecting this button will cause all HOF or CZMIL null values to be loaded as "
                  "null depths.  This is useful for interfacing with the OPTECH GCS or DPS software.  <b>Only check this button if "
                  "you are planning to do flight line reprocessing of null values in GCS or DPS.</b>");

QString subPathText = 
  globalPage::tr ("Selecting this button will cause all Linux paths to be converted to their Windoze equivalents.  First, "
                  "some background :<br><br>"
                  "Windoze equivalent paths are defined in a file called .pfm_cfg that is somewhere in the user's PATH.  "
                  "Normally this will be in /usr/local/bin but may also be in the current directory to override settings "
                  "in one that is elsewhere in the PATH.  The syntax of the substitute path line in the .pfm_cfg file "
                  "is:<br><br>"
                  "[SUBSTITUTE PATH] = N:,/data1/datasets<br><br>"
                  "There may be up to 10 substitute path lines in the .pfm_cfg file.  What this is used for is to allow "
                  "Windoze systems to access PFM files that are stored on a Linux SAMBA server.  In the above example a "
                  "Windoze system has mapped /data1/datasets on the SAMBA server to N: on the Windoze system.  If you "
                  "select the Invert Substitute Paths button all Linux paths will be converted into Windoze paths in the "
                  ".pfm file.  Using the above as an example, "
                  "/data1/datasets/Palau_D/02DS04026_006_041012_2306_A_07488.hof would become "
                  "N:\\Palau_D\\02DS04026_006_041012_2306_A_07488.hof.  The PFM library on the Linux system will use the "
                  ".pfm_cfg file to convert these back to Linux paths when processing.  The equivalent behavior is not "
                  "yet available on Windoze.<br><br>"
                  "<b>WARNING: multiple substitute paths for the same directory or part of the same directory will "
                  "confuse the system!</b>");

QString invLandText = 
  globalPage::tr ("Selecting this button will cause all HOF land values (ABDC 70) to be invalidated.  This is useful if "
                  "you have full TOF coverage over land.");

QString invSecText = 
  globalPage::tr ("Selecting this button will cause all HOF secondary return values to be invalidated.  Secondary returns "
                  "should not be confused with second (or last) returns.  The primary return is the one of a possible two "
                  "returns that Optech's GCS has decided is the correct bottom.  This will depend on how the real-time system "
                  "was set up.  If it was set to use first return then the secondary return will most likely be the second "
                  "return.  If it was set to use strongest return then the secondary return will most likely be the first "
                  "return.  If you are looking for hazards to navigation <b>NEVER</b> set this flag.");

QString lidAttrText = 
  globalPage::tr ("Selecting this button will cause all HOF and TOF files loaded into a new PFM structure to have the "
                  "IVS standard bin attributes set.  This is useful if you are planning on running GCS using this PFM.  "
                  "The attributes are Shallow Water Algorithm used, Shoreline Depth Swap used, Land flag set, and "
                  "second depth present.  If you are planning on loading both LIDAR and sonar data, load the LIDAR data "
                  "first with this flag set, then append the sonar data.");

QString refDataText = 
  globalPage::tr ("Selecting this button will cause all data that is loaded in this run to be marked as reference data.  "
                  "That is, it will be loaded but will not be used to compute the surfaces, statistics, or used for "
                  "selecting soundings.  This is especially useful for data types that are not very accurate but that "
                  "are useful for reference.  A perfect example is UNISIPS depth data.  Normally UNISIPS data is loaded "
                  "as reference data.  Lead line data is another example.  NOTE: If you are appending to a pre-4.3 PFM "
                  "file this button is meaningless.");

QString fileCheckText = 
  globalPage::tr ("Selecting this button will cause all input data files to be checked prior to loading.  This helps to "
                  "prevent crashes due to problems with the input files.  This is mostly useful for GSF and Charts data.  "
                  "If you have a large number of files for input this step may take some time but it could also "
                  "save you from having to reload all of the files if you have one or more bad files near the end of your "
                  "input file list.");

QString tofRefText = 
  globalPage::tr ("Selecting this button will cause all TOF first return values to be loaded as <b><i>reference</i></b> "
                  "values.  These points will not be used for computing the PFM surfaces.  Normally you will want to "
                  "leave this unchecked.  To differentiate between first and second returns in PFM structures you can "
                  "check the subrecord.  Subrecord 0 indicates a first return.  Subrecord 1 is a second return.");

QString hofFlagText = 
  globalPage::tr ("Selecting this button will cause all HOF data points to have the standard (IVS) user flags set in "
                  "the data points.  The flags are<br><br>"
                  "<ul>"
                  "<li>Shallow Water Process!</li>"
                  "<li>Shoreline Depth Swapped!</li>"
                  "<li>Land!</li>"
                  "<li>Second Depth Present!</li>"
                  "</ul>");

QString gsfNominalText = 
  globalPage::tr ("Selecting this button will cause the nominal depths (computed with a fixed sound velocity of "
                  "1500 m/sec) to be loaded.  If nominal depth is not available then true depth will be used and an "
                  "advisory message will be placed in the error listing.");

QString srtmBestText = 
  globalPage::tr ("Load the highest resolution Shuttle Radar Topography Mission (SRTM) data available for the selected "
                  "area.  In order for this option to be available the SRTM_DATA environment variable must point to the "
                  "directory containing the SRTM .cte directories (srtm1_topo.cte, srtm3_topo.cte, and srtm30_topo.cte).  "
                  "The SRTM 1 data is only available for the continental US, Hawaii, and Alaska.  The SRTM 3 data is "
                  "available for the entire world between 56S and 60N latitude.  The SRTM 30 data is available for the "
                  "entire world (this is the equivalent of GTOPO30).  SRTM data is loaded as PFM_NAVO_ASCII_DATA and "
                  "cannot be unloaded.  It can however be edited in the PFM structure.<br><br>"
                  "<b>IMPORTANT NOTE: If you've already loaded SRTM data in a PFM and you want to append to it make sure "
                  "that you turn the load STRM data flag off since it will be loaded again!</b><br><br>"
                  "<b><i>WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING<br>"
                  "If you have the limited distribution SRTM2 data on your system, your PFM file will be limited "
                  "distribution if your PFM contains areas outside of the continental US, Hawaii, and Alaska.<br>"
                  "WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING</i></b>");

QString srtmOneText = 
  globalPage::tr ("Load the one second resolution Shuttle Radar Topography Mission (SRTM) data for the selected "
                  "area.  In order for this option to be available the SRTM_DATA environment variable must point to the "
                  "directory containing the SRTM .cte directories (srtm1_topo.cte, srtm3_topo.cte, and srtm30_topo.cte).  "
                  "The SRTM 1 data is only available for the continental US, Hawaii, and Alaska.  SRTM data is loaded as "
                  "PFM_NAVO_ASCII_DATA and cannot be unloaded.  It can however be edited in the PFM structure.<br><br>"
                  "<b>IMPORTANT NOTE: If you've already loaded SRTM data in a PFM and you want to append to it make sure "
                  "that you turn the load STRM data flag off since it will be loaded again!</b><br><br>"
                  "<b><i>WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING<br>"
                  "If you have the limited distribution SRTM2 data on your system, your PFM file will be limited "
                  "distribution if your PFM contains areas outside of the continental US, Hawaii, and Alaska.<br>"
                  "WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING</i></b>");

QString srtmThreeText = 
  globalPage::tr ("Load the three second resolution Shuttle Radar Topography Mission (SRTM) data for the selected "
                  "area.  In order for this option to be available the SRTM_DATA environment variable must point to the "
                  "directory containing the SRTM .cte directories (srtm1_topo.cte, srtm3_topo.cte, and srtm30_topo.cte).  "
                  "The SRTM 3 data is available for the entire world between 56S and 60N latitude.  SRTM data is loaded "
                  "as PFM_NAVO_ASCII_DATA and cannot be unloaded.  It can however be edited in the PFM structure.<br><br>"
                  "<b>IMPORTANT NOTE: If you've already loaded SRTM data in a PFM and you want to append to it make sure "
                  "that you turn the load STRM data flag off since it will be loaded again!</b>");

QString srtmThirtyText = 
  globalPage::tr ("Load the thirty second resolution Shuttle Radar Topography Mission (SRTM) data for the selected "
                  "area.  In order for this option to be available the SRTM_DATA environment variable must point to the "
                  "directory containing the SRTM .cte directories (srtm1_topo.cte, srtm3_topo.cte, and srtm30_topo.cte).  "
                  "The SRTM 30 data is available for the entire world (this is the equivalent of GTOPO30).  SRTM data is "
                  "loaded as PFM_NAVO_ASCII_DATA and cannot be unloaded.  It can however be edited in the PFM "
                  "structure.<br><br>"
                  "<b>IMPORTANT NOTE: If you've already loaded SRTM data in a PFM and you want to append to it make sure "
                  "that you turn the load STRM data flag off since it will be loaded again!</b>");

QString srtmRefText = 
  globalPage::tr ("If this box is checked, all Shuttle Radar Topography Mission (SRTM) data that is loaded will be marked "
                  "as <b>reference</b> data.  That is, it will not be used to compute the bin surfaces and will only be "
                  "visible in the editor portion of PFM_ABE (and then only if the <b>Display Reference</b> button is "
                  "selected).");

QString srtm2ExText = 
  globalPage::tr ("If this box is checked, NGA's limited distribution Shuttle Radar Topography Mission (SRTM2) data "
                  "will not be used in the generation of your PFM(s).");

QString cubeAttributesText = 
  globalPage::tr ("Selecting this button will cause the program to allocate storage for the six standard CUBE attributes in "
		  "the PFM structure.  These slots can be filled by selecting <b>Run CUBE</b> so that the CUBE process will "
		  "run after the PFM has completed building, by running <b>navo_pfm_cube</b> after the PFM has been built, or by using "
		  "the CUBE button <img source=\":/icons/cube.xpm\"> in pfmView.  There is no requirement to run CUBE on load "
		  "and, in most cases, it is more effective to just allocate the storage and use the normal tools to clean the "
		  "data.<br><br>"
		  "<b>WARNING: If you do not select this button you will have to rebuild the PFM if you want to run the CUBE "
		  "process on the data.  It is strongly suggested that you select this box especially when loading <i>ANY</i> "
		  "GSF sonar, HOF LIDAR, or WLF LIDAR data.</b>");

QString cubeLoadText = 
  globalPage::tr ("Selecting this button will cause the program to run navo_pfm_cube as a last step in the build process.  "
		  "If you select this then <b>Insert CUBE attributes</b> will be automatically set.  If the "
		  "navo_pfm_cube program is not available then this option will be disabled.");
		  
QString horizontalText = 
  globalPage::tr ("Set the default value for horizontal error (meters).  This value is used if no error value is available "
		  "from the input file.  This is the 95% confidence level and should be set optimistically.  The default values are "
		  "appropriate for data in the 0-40 meter range.<br><br>"
		  "<b>IMPORTANT NOTE: At this time GSF files contain horizontal and vertical error estimates and HOF/WLF files have "
		  "them computed on-the-fly.</b>");

QString verticalText = 
  globalPage::tr ("Set the default value for vertical error (meters).  This value is used if no error value is available "
		  "from the input file.  This is the 95% confidence level and should be set optimistically.  The default values are "
		  "appropriate for data in the 0-40 meter range.<br><br>"
		  "<b>IMPORTANT NOTE: At this time GSF files contain horizontal and vertical error estimates and HOF/WLF files have "
		  "them computed on-the-fly.</b>");

QString cubeBoxText = 
  globalPage::tr ("Unless you really know what you're doing you should leave all of these settings as they are.  The only one that "
		  "might be user modifiable without disastrous results is the IHO order selection.  Because these are so "
		  "little understood they are not saved for subsequent runs or in the pfmLoad parameters file.  They will "
		  "always return to the defaults.");

QString timeCheckText = 
  globalPage::tr ("If this box is checked, the time in minutes from the epoch (01/01/1970) will be inserted as a "
                  "depth attribute for all data types that have time (GSF, HOF, TOF, WLF, LLZ, CZMIL, BAG, HAWKEYE).  This data type is "
                  "decoded in pfmView, pfmEdit, and pfmEdit3D so that you will see the date and time to the nearest minute.<br><br>"
                  "<b>IMPORTANT NOTE: It is highly recommended that you always select this option.</b>");

QString gsfText = 
  globalPage::tr ("Press this button to bring up a menu of possible GSF attributes that you might want to include in "
                  "the index file along with each data point.  You may combine attributes from different data types but "
                  "only 10 total are allowed.  Note that some attributes may not be available from some files.  For example, "
                  "<b>Mean relative amp.</b> is not available for older SIMRAD GSF files.  If attributes are not "
                  "available but are selected they will be set to 0.0.<br><br>"
                  "<b>IMPORTANT NOTE: The ability to select attributes to be included in the PFM file is only available "
                  "when creating a new PFM file.  If you are appending to an existing file you cannot add or subtract "
                  "attributes.</b>");

QString hofText = 
  globalPage::tr ("Press this button to bring up a menu of possible HOF attributes that you might want to include with each "
                  "data point.  You may combine attributes from different data types but "
                  "only 10 total are allowed.  Note that some attributes may not be available from some files.  For example, "
                  "<b>Mean relative amp.</b> is not available for older SIMRAD GSF files.  If attributes are not "
                  "available but are selected they will be set to 0.0.<br><br>"
                  "<b>IMPORTANT NOTE: The ability to select attributes to be included in the PFM file is only available "
                  "when creating a new PFM file.  If you are appending to an existing file you cannot add or subtract "
                  "attributes.</b>");

QString tofText = 
  globalPage::tr ("Press this button to bring up a menu of possible TOF attributes that you might want to include with each "
                  "data point.  You may combine attributes from different data types but "
                  "only 10 total are allowed.  Note that some attributes may not be available from some files.  For example, "
                  "<b>Mean relative amp.</b> is not available for older SIMRAD GSF files.  If attributes are not "
                  "available but are selected they will be set to 0.0.<br><br>"
                  "<b>IMPORTANT NOTE: The ability to select attributes to be included in the PFM file is only available "
                  "when creating a new PFM file.  If you are appending to an existing file you cannot add or subtract "
                  "attributes.</b>");

QString wlfText = 
  globalPage::tr ("Press this button to bring up a menu of possible WLF attributes that you might want to include with each "
                  "data point.  You may combine attributes from different data types but "
                  "only 10 total are allowed.  Note that some attributes may not be available from some files.  For example, "
                  "<b>Mean relative amp.</b> is not available for older SIMRAD GSF files.  If attributes are not "
                  "available but are selected they will be set to 0.0.<br><br>"
                  "<b>IMPORTANT NOTE: The ability to select attributes to be included in the PFM file is only available "
                  "when creating a new PFM file.  If you are appending to an existing file you cannot add or subtract "
                  "attributes.</b>");

QString czmilText = 
  globalPage::tr ("Press this button to bring up a menu of possible CZMIL attributes that you might want to include with each "
                  "data point.  You may combine attributes from different data types but "
                  "only 10 total are allowed.  Note that some attributes may not be available from some files.  For example, "
                  "<b>Mean relative amp.</b> is not available for older SIMRAD GSF files.  If attributes are not "
                  "available but are selected they will be set to 0.0.<br><br>"
                  "<b>IMPORTANT NOTE: The ability to select attributes to be included in the PFM file is only available "
                  "when creating a new PFM file.  If you are appending to an existing file you cannot add or subtract "
                  "attributes.</b>");

QString bagText = 
  globalPage::tr ("Press this button to bring up a menu of possible BAG attributes that you might want to include with each "
                  "data point.  You may combine attributes from different data types but "
                  "only 10 total are allowed.  Note that some attributes may not be available from some files.  For example, "
                  "<b>Mean relative amp.</b> is not available for older SIMRAD GSF files.  If attributes are not "
                  "available but are selected they will be set to 0.0.<br><br>"
                  "<b>IMPORTANT NOTE: The ability to select attributes to be included in the PFM file is only available "
                  "when creating a new PFM file.  If you are appending to an existing file you cannot add or subtract "
                  "attributes.</b>");

QString hawkeyeText = 
  globalPage::tr ("Press this button to bring up a menu of possible HAWKEYE attributes that you might want to include with each "
                  "data point.  You may combine attributes from different data types but "
                  "only 10 total are allowed.  Note that some attributes may not be available from some files.  For example, "
                  "<b>Mean relative amp.</b> is not available for older SIMRAD GSF files.  If attributes are not "
                  "available but are selected they will be set to 0.0.<br><br>"
                  "<b>IMPORTANT NOTE: The ability to select attributes to be included in the PFM file is only available "
                  "when creating a new PFM file.  If you are appending to an existing file you cannot add or subtract "
                  "attributes.</b>");
