
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



#include "abe.hpp"


//  Set defaults for all of the program's user defined options.

void set_defaults (MISC *misc, OPTIONS *options)
{
  //  Tooltip text for the buttons that have editable accelerators.  If you want to change the order of the buttons
  //  change the define statements in abeDef.hpp

#ifdef NVWIN3X
  misc->progName[TRACKLINE] = "trackLine.exe";
  misc->progName[AREACHECK] = "areaCheck.exe";
  misc->progName[PFMLOAD] = "pfmLoad.exe";
  misc->progName[PFMVIEW] = "pfmView.exe";
  misc->progName[GEOSWATH3D] = "geoSwath3D.exe";
  misc->progName[EXAMGSF] = "examGSF.exe";
  misc->progName[PFMFEATURE] = "pfmFeature.exe";
  misc->progName[PFMBAG] = "pfmBag.exe";
  misc->progName[PFMEXTRACT] = "pfmExtract.exe";
  misc->progName[PFMGEOTIFF] = "pfmGeotiff.exe";
  misc->progName[PFMCHARTSIMAGE] = "pfmChartsImage.exe";
  misc->progName[CHARTS2LAS] = "charts2LAS.exe";
  misc->progName[BAGGEOTIFF] = "bagGeotiff.exe";
  misc->progName[BAGVIEWER] = "bagViewer.exe";
#else
  misc->progName[TRACKLINE] = "trackLine";
  misc->progName[AREACHECK] = "areaCheck";
  misc->progName[PFMLOAD] = "pfmLoad";
  misc->progName[PFMVIEW] = "pfmView";
  misc->progName[GEOSWATH3D] = "geoSwath3D";
  misc->progName[EXAMGSF] = "examGSF";
  misc->progName[PFMFEATURE] = "pfmFeature";
  misc->progName[PFMBAG] = "pfmBag";
  misc->progName[PFMEXTRACT] = "pfmExtract";
  misc->progName[PFMGEOTIFF] = "pfmGeotiff";
  misc->progName[PFMCHARTSIMAGE] = "pfmChartsImage";
  misc->progName[CHARTS2LAS] = "charts2LAS";
  misc->progName[BAGGEOTIFF] = "bagGeotiff";
  misc->progName[BAGVIEWER] = "bagViewer";
#endif

  misc->buttonIcon[TRACKLINE] = QIcon (":/icons/trackLine.xpm");
  misc->buttonIcon[AREACHECK] = QIcon (":/icons/areaCheck.xpm");
  misc->buttonIcon[PFMLOAD] = QIcon (":/icons/pfmLoad.xpm");
  misc->buttonIcon[PFMVIEW] = QIcon (":/icons/pfmView.xpm");
  misc->buttonIcon[GEOSWATH3D] = QIcon (":/icons/geoSwath3D.png");
  misc->buttonIcon[EXAMGSF] = QIcon (":/icons/examGSF.xpm");
  misc->buttonIcon[PFMFEATURE] = QIcon (":/icons/pfmFeature.xpm");
  misc->buttonIcon[PFMBAG] = QIcon (":/icons/pfmBag.xpm");
  misc->buttonIcon[PFMEXTRACT] = QIcon (":/icons/pfmExtract.xpm");
  misc->buttonIcon[PFMGEOTIFF] = QIcon (":/icons/pfmGeotiff.xpm");
  misc->buttonIcon[PFMCHARTSIMAGE] = QIcon (":/icons/pfmChartsImage.xpm");
  misc->buttonIcon[CHARTS2LAS] = QIcon (":/icons/charts2LAS.xpm");
  misc->buttonIcon[BAGGEOTIFF] = QIcon (":/icons/bagGeotiff.xpm");
  misc->buttonIcon[BAGVIEWER] = QIcon (":/icons/bagViewer.xpm");


  misc->buttonWhat[AREACHECK] =
    abe::tr ("The areaCheck program is used to display and edit ISS60 and generic area and zone files, "
             "Army Corps area files.  It can also be used to display track files generated by the trackLine "
             "program, GeoTIFF files, ESRI polygon shape files, Binary Feature Data (.bfd) files, LLZ files, "
             "YXZ files, CHRTR .fin files, and BAG (Linux only) files.");
  misc->buttonWhat[PFMLOAD] =
    abe::tr ("The pfmLoad program is used to create, rebuild, or append to PFM structures.");
  misc->buttonWhat[PFMVIEW] =
    abe::tr ("The pfmView program is used to display geographically binned data that is stored in a PFM structure.  "
             "Subsequent to displaying a portion of the area, 2D or 3D editing of the contributing data points "
             "is done.  This program is what is most often referred to as the PFM Area-Based Editor.");
  misc->buttonWhat[GEOSWATH3D] =
    abe::tr ("The geoSwath3D program is used to display and edit data points in data files.  The currently "
             "available file types are GSF, HOF, TOF, and WLF.");
  misc->buttonWhat[EXAMGSF] =
    abe::tr ("examGSF is a program that is used to display data in a GSF file.  The data is displayed in a text form.  "
             "This program is useful for debugging systematic errors with sonar systems since every single piece of "
             "information in the GSF file can be displayed.");
  misc->buttonWhat[PFMFEATURE] =
    abe::tr ("pfmFeature is a tool for selecting features in a PFM structure.  "
             "The selected features will be written to a new or pre-existing "
             "feature file.  Feature selection criteria may be either IHO special order or order 1.");
  misc->buttonWhat[PFMBAG] =
    abe::tr ("pfmBag is a tool for creating a Bathymetric Attributed Grid (BAG) file "
             "that represents a best fit surface from a PFM structure.");
  misc->buttonWhat[PFMEXTRACT] =
    abe::tr ("The pfmExtract program is a tool for reading a PFM file and writing out a series of "
             "files (in ASCII, LLZ, or RDP) for input to CARIS or other software packages.");
  misc->buttonWhat[PFMGEOTIFF] =
    abe::tr ("pfmGeotiff is a tool for creating a GeoTIFF file that represents one of "
             "the filtered surfaces of a PFM structure.  It can optionally write out "
             "multiple files that represent the data type coverages.");
  misc->buttonWhat[PFMCHARTSIMAGE] =
    abe::tr ("pfmChartsImage is a tool for extracting downlooking images from .img files "
             "associated with CHARTS LIDAR .hof or .tof files.  It uses the associated "
             ".pos or SBET files to get the aircraft attitude for each image.  A Digital "
             "Elevation Model (DEM) is created at a user defined grid spacing for the "
             "specified area using the Minimum Curvature Spline Interpolation (MISP) "
             "library.  An ASCII position and attitude file is also generated.  Camera "
             "biases and other offsets must be supplied in order to correctly position "
             "the photos.  This program creates a directory in the current working "
             "directory of the form <b>PFM_FILENAME__AREA_FILENAME_image_files</b>.  "
             "The images, the DEM file, and the attitude file will be stored in that "
             "directory.  Once the directory and files are created Ray Seyfarth's mosaic "
             "program is run to generate a GeoTIFF mosaic file from the images.");
  misc->buttonWhat[TRACKLINE] =
    abe::tr ("The trackLine program is used to create a decimated TRACK file from GSF, POS/SBET, WLF, "
             "or CZMIL data files.  The TRACK file can be used in areaCheck to define an area for "
             "a PFM.");
  misc->buttonWhat[CHARTS2LAS] =
    abe::tr ("The charts2LAS program is used to convert CHARTS HOF and/or TOF files to LAS format.");
  misc->buttonWhat[BAGGEOTIFF] =
    abe::tr ("bagGeotiff is a tool for creating a GeoTIFF file from a Bathymetric Attributed "
             "Grid (BAG) file.");
  misc->buttonWhat[BAGVIEWER] =
    abe::tr ("The bagViewer program is used for viewing ONS BAG surfaces in 3D.  It can also display GeoTIFF "
             "files as opaque or translucent overlays.");


  misc->buttonFile[TRACKLINE] = NVFalse;
  misc->buttonFile[AREACHECK] = NVTrue;
  misc->buttonFile[PFMLOAD] = NVFalse;
  misc->buttonFile[PFMVIEW] = NVTrue;
  misc->buttonFile[GEOSWATH3D] = NVTrue;
  misc->buttonFile[EXAMGSF] = NVTrue;
  misc->buttonFile[PFMFEATURE] = NVTrue;
  misc->buttonFile[PFMBAG] = NVTrue;
  misc->buttonFile[PFMEXTRACT] = NVTrue;
  misc->buttonFile[PFMGEOTIFF] = NVTrue;
  misc->buttonFile[PFMCHARTSIMAGE] = NVTrue;
  misc->buttonFile[CHARTS2LAS] = NVFalse;
  misc->buttonFile[BAGGEOTIFF] = NVTrue;
  misc->buttonFile[BAGVIEWER] = NVTrue;


  options->buttonAccel[TRACKLINE] = "t";
  options->buttonAccel[AREACHECK] = "a";
  options->buttonAccel[PFMLOAD] = "l";
  options->buttonAccel[PFMVIEW] = "v";
  options->buttonAccel[GEOSWATH3D] = "g";
  options->buttonAccel[EXAMGSF] = "e";
  options->buttonAccel[PFMFEATURE] = "f";
  options->buttonAccel[PFMBAG] = "b";
  options->buttonAccel[PFMEXTRACT] = "F3";
  options->buttonAccel[PFMGEOTIFF] = "F4";
  options->buttonAccel[PFMCHARTSIMAGE] = "F5";
  options->buttonAccel[CHARTS2LAS] = "F6";
  options->buttonAccel[BAGGEOTIFF] = "F7";
  options->buttonAccel[BAGVIEWER] = "F8";


  misc->qsettings_org = abe::tr ("navo.navy.mil");
  misc->qsettings_app = abe::tr ("abe");

  options->window_x = 0;
  options->window_y = 0;

  options->main_button_icon_size = 48;
}


