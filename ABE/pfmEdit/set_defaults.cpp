
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



#include "pfmEdit.hpp"


/*!
  Set defaults for all of the program's user defined options.  This called before envin and may be called from 
  prefs.cpp if requested.
*/

void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore)
{
  if (!restore)
    {
      misc->draw_area_width = 950;
      misc->draw_area_height = 950;
      misc->drawing_canceled = NVFalse;
      misc->feature = NULL;
      memset (&misc->bfd_header, 0, sizeof (BFDATA_HEADER));
      misc->feature_mod = NVFalse;
      misc->resized = NVTrue;
      misc->area_drawn = NVFalse;
      misc->poly_count = 0;
      misc->rotate_angle = 0;
      misc->save_view = 0;
      misc->maxd = 10;
      misc->num_lines = 0;
      misc->reference_flag = NVFalse;
      misc->slice = NVFalse;
      misc->hotkey = -1;
      for (NV_INT32 pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++)
        {
          misc->shifted[pfm] = NVFalse;
          for (NV_INT32 i = 0 ; i < 4096 ; i++) misc->line_shift[pfm][i] = 0.0;
        }
      misc->GeoTIFF_init = NVTrue;
      misc->map_border = 5;
      misc->busy = NVFalse;
      misc->visible_feature_count = 0;
      misc->view = 0;
      misc->html_help_tag = "#edit_pfm";
      misc->marker_mode = 0;
      misc->frozen_point = -1;
      misc->mask_active = NVFalse;
      misc->filter_mask = NVFalse;
      misc->filter_kill_list = NULL;
      misc->filter_kill_count = 0;
      misc->filtered = NVFalse;
      misc->av_dist_list = NULL;
      misc->av_dist_count = 0;
      misc->hydro_lidar_present = NVFalse;
      misc->lidar_present = NVFalse;
      misc->gsf_present = NVFalse;
      misc->highlight_count = 0;
      misc->highlight = NULL;


      //  Tooltip text for the buttons that have editable accelerators

      misc->buttonText[SAVE_EXIT_KEY] = pfmEdit::tr ("Save changes and exit");
      misc->buttonText[SAVE_EXIT_MASK_KEY] = pfmEdit::tr ("Save changes, exit, and filter mask in pfmView");
      misc->buttonText[NO_SAVE_EXIT_KEY] = pfmEdit::tr ("Exit without saving changes");
      misc->buttonText[RESET_KEY] = pfmEdit::tr ("Reset to original view");
      misc->buttonText[REDRAW_KEY] = pfmEdit::tr ("Redraw");
      misc->buttonText[TOGGLE_CONTOUR_KEY] = pfmEdit::tr ("Toggle contour drawing");
      misc->buttonText[DELETE_POINT_MODE_KEY] = pfmEdit::tr ("Select delete subrecord/record edit mode");
      misc->buttonText[DELETE_RECTANGLE_MODE_KEY] = pfmEdit::tr ("Select delete rectangle edit mode");
      misc->buttonText[DELETE_POLYGON_MODE_KEY] = pfmEdit::tr ("Select delete polygon edit mode");
      misc->buttonText[FILTER_KEY] = pfmEdit::tr ("Run the statistical filter");
      misc->buttonText[ATTR_FILTER_KEY] = pfmEdit::tr ("Run the attribute filter");
      misc->buttonText[RECTANGLE_FILTER_MASK_KEY] = pfmEdit::tr ("Select rectangle filter mask mode");
      misc->buttonText[POLYGON_FILTER_MASK_KEY] = pfmEdit::tr ("Select polygon filter mask mode");
      misc->buttonText[RUN_HOTKEY_POLYGON_MODE_KEY] = pfmEdit::tr ("Select run hotkey program in polygon mode");
      misc->buttonText[EDIT_FEATURE_MODE_KEY] = pfmEdit::tr ("Select edit feature mode");
      misc->buttonText[UNDO_KEY] = pfmEdit::tr ("Undo last edit operation");
      misc->buttonText[DISPLAY_MULTIPLE_KEY] = pfmEdit::tr ("Select display multiple lines mode");
      misc->buttonText[DISPLAY_ALL_KEY] = pfmEdit::tr ("Display all lines");
      misc->buttonText[CLEAR_HIGHLIGHT_KEY] = pfmEdit::tr ("Clear all highlighted/marked points");
      misc->buttonText[HIGHLIGHT_POLYGON_MODE_KEY] = pfmEdit::tr ("Select highlight/mark points in polygon mode");
      misc->buttonText[CLEAR_POLYGON_MODE_KEY] = pfmEdit::tr ("Select clear all highlighted/marked points in polygon mode");
      misc->buttonText[COLOR_BY_DEPTH_ACTION_KEY] = pfmEdit::tr ("Color by depth");
      misc->buttonText[COLOR_BY_LINE_ACTION_KEY] = pfmEdit::tr ("Color by line");
      misc->buttonText[AV_DISTANCE_THRESHOLD_KEY] = pfmEdit::tr ("CZMIL LIDAR attribute viewer distance threshold tool");


      //  Toolbutton icons for the buttons that have editable accelerators

      misc->buttonIcon[SAVE_EXIT_KEY] = QIcon (":/icons/exit_save.xpm");
      misc->buttonIcon[SAVE_EXIT_MASK_KEY] = QIcon (":/icons/exit_mask.xpm");
      misc->buttonIcon[NO_SAVE_EXIT_KEY] = QIcon (":/icons/exit_no_save.xpm");
      misc->buttonIcon[RESET_KEY] = QIcon (":/icons/reset_view.xpm");
      misc->buttonIcon[REDRAW_KEY] = QIcon (":/icons/redraw.xpm");
      misc->buttonIcon[TOGGLE_CONTOUR_KEY] = QIcon (":/icons/contour.xpm");
      misc->buttonIcon[DELETE_POINT_MODE_KEY] = QIcon (":/icons/delete_point.xpm");
      misc->buttonIcon[DELETE_RECTANGLE_MODE_KEY] = QIcon (":/icons/delete_rect.xpm");
      misc->buttonIcon[DELETE_POLYGON_MODE_KEY] = QIcon (":/icons/delete_poly.xpm");
      misc->buttonIcon[FILTER_KEY] = QIcon (":/icons/filter.xpm");
      misc->buttonIcon[ATTR_FILTER_KEY] = QIcon (":/icons/attr_filter.xpm");
      misc->buttonIcon[RECTANGLE_FILTER_MASK_KEY] = QIcon (":/icons/filter_mask_rect.xpm");
      misc->buttonIcon[POLYGON_FILTER_MASK_KEY] = QIcon (":/icons/filter_mask_poly.xpm");
      misc->buttonIcon[RUN_HOTKEY_POLYGON_MODE_KEY] = QIcon (":/icons/hotkey_poly.xpm");
      misc->buttonIcon[EDIT_FEATURE_MODE_KEY] = QIcon (":/icons/editfeature.xpm");
      misc->buttonIcon[UNDO_KEY] = QIcon (":/icons/undo.png");
      misc->buttonIcon[DISPLAY_MULTIPLE_KEY] = QIcon (":/icons/displaylines.xpm");
      misc->buttonIcon[DISPLAY_ALL_KEY] = QIcon (":/icons/displayall.xpm");
      misc->buttonIcon[CLEAR_HIGHLIGHT_KEY] = QIcon (":/icons/clear_highlight.xpm");
      misc->buttonIcon[HIGHLIGHT_POLYGON_MODE_KEY] = QIcon (":/icons/highlight_polygon.xpm");
      misc->buttonIcon[CLEAR_POLYGON_MODE_KEY] = QIcon (":/icons/clear_polygon.xpm");
      misc->buttonIcon[COLOR_BY_DEPTH_ACTION_KEY] = QIcon (":/icons/color_by_depth.xpm");
      misc->buttonIcon[COLOR_BY_LINE_ACTION_KEY] = QIcon (":/icons/color_by_line.xpm");
      misc->buttonIcon[AV_DISTANCE_THRESHOLD_KEY] = QIcon(":/icons/distance_threshold.xpm");


      //  NULL out the buttons and actions so we can tell them apart.

      for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
        {
          misc->button[i] = NULL;
          misc->action[i] = NULL;
        }


      //  Set up the sine and cosine values for rotation of view.

      for (NV_INT32 i = 0 ; i < 3600 ; i++)
        {
          misc->sin_array[i] = sinf (((NV_FLOAT32) i / 10.0) * NV_DEG_TO_RAD);
          misc->cos_array[i] = cosf (((NV_FLOAT32) i / 10.0) * NV_DEG_TO_RAD);
        }


      misc->abe_share->key = 0;
      misc->abe_share->modcode = 0;
      misc->displayed_area.min_y = -91.0;

      misc->qsettings_org = pfmEdit::tr ("navo.navy.mil");
      misc->qsettings_app = pfmEdit::tr ("pfmEdit");

      misc->add_feature_index = -1;
      misc->nearest_feature_point = -1;

      misc->bfd_open = NVFalse;
      misc->dateline = NVFalse;
      misc->bounds_changed = NVFalse;

      misc->undo = NULL;
      misc->undo_count = 0;
      misc->time_attr = -1;
      misc->datum_attr = -1;
      misc->ellipsoid_attr = -1;


      //  This is a special case for an option.  We don't want to reset the undo levels if we called for a restore of defaults from the prefs dialog.

      options->undo_levels = 100;


#ifdef NVWIN3X

      misc->help_browser = "C:\\Program Files\\Mozilla Firefox\\firefox.exe";

      QFileInfo br (misc->help_browser);

      if (!br.exists () || !br.isExecutable ()) misc->help_browser = "C:\\Program Files\\Internet Explorer\\iexplore.exe";

#else

      misc->help_browser = "firefox";

#endif
    }


  /*

    Ancillary program command line substitutions:


    [MOSAIC_FILE] - associated mosaic file name
    [TARGET_FILE] - associated feature (target) file name
    [PFM_FILE] - PFM list or handle file name
    [BIN_FILE] - PFM bin directory name
    [INDEX_FILE] - PFM index directory name
    [INPUT_FILE] - input data file associated with the current point
    [SHARED_MEMORY_ID] - ABE shared memory ID (some programs like chartsPic require this)
    [LINE] - line name associated with the current point
    [Z_VALUE] - Z value of the current point
    [X_VALUE] - X value (usually longitude) associated with the current point
    [Y_VALUE] - Y value (usually latitude) associated with the current point
    [MIN_Y] - minimum Y value in the currently displayed area
    [MIN_X] - minimum X value in the currently displayed area
    [MAX_Y] - maximum Y value in the currently displayed area
    [MAX_X] - maximum X value in the currently displayed area
    [FILE_NUMBER] - PFM input file number associated with the current point
    [LINE_NUMBER] - PFM input line number associated with the current point
    [VALIDITY] - PFM validity word for the current point
    [RECORD] - input file record number associated with the current point
    [SUBRECORD] - input file subrecord (usually beam) number associated with the current point
    [DATA_TYPE] - PFM data type of the current point

    [CL] - run this program as a command line program (creates a dialog for output)


    Note: the commands used to be options (i.e. user modifiable but it became too complicated).
    If you change these you must update the documentation in hotkeysHelp.cpp.

    Note: Polygon eligible commands cannot have an associated button.  Associated buttons are set in pfmEdit.cpp.

    If you want to add or subtract programs it needs to be done here, in pfmEditDef.hpp, and add or subtract buttons
    in pfmEdit.cpp.

  */


  options->kill_and_respawn = NVFalse;

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      options->kill_switch[i] = KILL_SWITCH_OFFSET + i;

      for (NV_INT32 j = 0 ; j < PFM_DATA_TYPES ; j++) options->data_type[i][j] = NVFalse;

      switch (i)
        {
        case EXAMGSF:
          options->prog[i] = "examGSF --file [INPUT_FILE] --record [RECORD]";
          options->name[i] = "examGSF";
          options->description[i] = pfmEdit::tr ("GSF non-graphical data viewer.  Displays data for nearest point.");
          options->data_type[i][PFM_GSF_DATA] = NVTrue;
          options->hotkey[i] = "e";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;

        case GSFMONITOR:
          options->prog[i] = "gsfMonitor [SHARED_MEMORY_KEY]";
          options->name[i] = "gsfMonitor";
          options->description[i] = pfmEdit::tr ("GSF non-graphical data viewer.  Displays ping data for nearest point.");
          options->data_type[i][PFM_GSF_DATA] = NVTrue;
          options->hotkey[i] = "g";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;

        case WAVEFORMMONITOR:
          options->prog[i] = "waveformMonitor [SHARED_MEMORY_KEY]";
          options->name[i] = "waveformMonitor";
          options->description[i] = pfmEdit::tr ("HOF/WLF waveform monitor (PMT, APD, IR, Raman).  Displays waveform for nearest point.");
          options->data_type[i][PFM_SHOALS_1K_DATA] = NVTrue;
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->data_type[i][PFM_WLF_DATA] = NVTrue;
          options->hotkey[i] = "w";
          options->action[i] = "1,2,3,4";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;

        case UNISIPS:
          options->prog[i] = "unisips [INPUT_FILE] -record [RECORD]";
          options->name[i] = "unisips";
          options->description[i] = 
            pfmEdit::tr ("UNISIPS image file graphical data viewer.  Displays image data in line containing the nearest point.");
          options->data_type[i][PFM_UNISIPS_DEPTH_DATA] = NVTrue;
          options->hotkey[i] = "u";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;


          //  For historical reasons we are using "x" to kick this off.  It used to be the "extended" information
          //  for the waveformMonitor program.

        case LIDARMONITOR:
          options->prog[i] = "lidarMonitor [SHARED_MEMORY_KEY]";
          options->name[i] = "lidarMonitor";
          options->description[i] = pfmEdit::tr ("HOF, TOF, WLF, and HAWKEYE textual data viewer.  Displays entire record for nearest point.");
          options->data_type[i][PFM_SHOALS_1K_DATA] = NVTrue;
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->data_type[i][PFM_SHOALS_TOF_DATA] = NVTrue;
          options->data_type[i][PFM_HAWKEYE_HYDRO_DATA] = NVTrue;
          options->data_type[i][PFM_HAWKEYE_TOPO_DATA] = NVTrue;
          options->data_type[i][PFM_CZMIL_DATA] = NVTrue;
          options->hotkey[i] = "x";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;

        case HAWKEYEMONITOR:
          options->prog[i] = "hawkeyeMonitor [SHARED_MEMORY_KEY]";
          options->name[i] = "hawkeyeMonitor";
          options->description[i] = pfmEdit::tr ("HAWKEYE graphic waveform and text data viewer.  Displays data for nearest point.");
          options->data_type[i][PFM_HAWKEYE_HYDRO_DATA] = NVTrue;
          options->hotkey[i] = "h";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;

        case CHARTSPIC:
          options->prog[i] = "chartsPic [SHARED_MEMORY_KEY]";
          options->name[i] = "chartsPic";
          options->description[i] = pfmEdit::tr ("HOF and TOF down-looking image viewer.  Displays nearest down-looking photo.");
          options->data_type[i][PFM_SHOALS_1K_DATA] = NVTrue;
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->data_type[i][PFM_SHOALS_TOF_DATA] = NVTrue;
          options->data_type[i][PFM_WLF_DATA] = NVTrue;
          options->data_type[i][PFM_CZMIL_DATA] = NVTrue;
          options->hotkey[i] = "i";
          options->action[i] = "r,t";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;

          //  Don't move mosaicView from 8 unless you change the zoom check in the keypress event in pfmEdit.cpp
          //  to match the new number.

        case MOSAICVIEW:
          options->prog[i] = "mosaicView [SHARED_MEMORY_KEY] [MOSAIC_FILE]";
          options->name[i] = "mosaicView";
          options->description[i] = pfmEdit::tr ("GeoTIFF viewer for photo mosaics or other GeoTIFF files (scanned maps, etc.).");
          options->data_type[i][PFM_UNDEFINED_DATA] = NVTrue;
          options->hotkey[i] = "m";
          options->action[i] = "z,+,-";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;

        case WAVEWATERFALL_APD:
          options->prog[i] = "waveWaterfall [SHARED_MEMORY_KEY] -a";
          options->name[i] = "waveWaterfall (APD)";
          options->description[i] = 
            pfmEdit::tr ("HOF/WLF waveform waterfall display (APD waveform).  Displays APD waveforms for geographically nearest 9 records.");
          options->data_type[i][PFM_SHOALS_1K_DATA] = NVTrue;
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->data_type[i][PFM_WLF_DATA] = NVTrue;
          options->data_type[i][PFM_CZMIL_DATA] = NVTrue;
          options->hotkey[i] = "a";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;

        case WAVEWATERFALL_PMT:
          options->prog[i] = "waveWaterfall [SHARED_MEMORY_KEY] -p";
          options->name[i] = "waveWaterfall (PMT)";
          options->description[i] = 
            pfmEdit::tr ("HOF/WLF waveform waterfall display (PMT waveform).  Displays PMT waveforms for geographically nearest 9 records.");
          options->data_type[i][PFM_SHOALS_1K_DATA] = NVTrue;
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->data_type[i][PFM_WLF_DATA] = NVTrue;
          options->data_type[i][PFM_CZMIL_DATA] = NVTrue;
          options->hotkey[i] = "p";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;

        case CZMILWAVEMONITOR:
          options->prog[i] = "CZMILwaveMonitor [SHARED_MEMORY_KEY]";
          options->name[i] = "CZMILwaveMonitor";
          options->description[i] = pfmEdit::tr ("CZMIL waveform monitor.  Displays waveform(s) for nearest point(s).");
          options->data_type[i][PFM_CZMIL_DATA] = NVTrue;
          options->hotkey[i] = "Alt+w";
          options->action[i] = "n,1,2,3,4,5";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;

        case HOFRETURNKILL:
          options->prog[i] = "hofReturnKill [SHARED_MEMORY_KEY]";
          options->name[i] = "hofReturnKill (invalidate low slope returns)";
          options->description[i] = pfmEdit::tr ("Kill HOF returns based on fore/back slope and amplitude.");
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->hotkey[i] = "!";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVTrue;
          options->hk_poly_filter[i] = 1;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;

        case HOFRETURNKILL_SWA:
          options->prog[i] = "hofReturnKill -s [SHARED_MEMORY_KEY]";
          options->name[i] = "hofReturnKill (invalidate shallow water algorithm data)";
          options->description[i] = pfmEdit::tr ("Kill HOF Shallow Water Algorithm or Shoreline Depth Swap data.");
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->hotkey[i] = "$";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVTrue;
          options->hk_poly_filter[i] = 1;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;


        case ATTRIBUTEVIEWER:
          options->prog[i] = "attributeViewer [SHARED_MEMORY_KEY]";
          options->name[i] = "attributeViewer";
          options->description[i] = pfmEdit::tr ("CZMIL LIDAR attribute viewer.  Analyze/edit key parameters of CZMIL LIDAR shot(s).");
          options->data_type[i][PFM_CZMIL_DATA] = NVTrue;
          options->hotkey[i] = "Alt+a";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVTrue;
          options->state[i] = 1;
          break;


        case RMSMONITOR:
          options->prog[i] = "rmsMonitor [SHARED_MEMORY_KEY]";
          options->name[i] = "rmsMonitor";
          options->description[i] = pfmEdit::tr ("CHARTS HOF and TOF navigation RMS textual data viewer.  Displays entire record for nearest point.");
          options->data_type[i][PFM_SHOALS_1K_DATA] = NVTrue;
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->data_type[i][PFM_SHOALS_TOF_DATA] = NVTrue;
          options->hotkey[i] = "r";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 1;
          break;


        case HOFWAVEFILTER:
          options->prog[i] = "hofWaveFilter [SHARED_MEMORY_KEY]";
          options->name[i] = "hofWaveFilter";
          options->description[i] = pfmEdit::tr ("CHARTS HOF waveform filter.  Checks adjacent waveforms for isolated points.");
          options->data_type[i][PFM_CHARTS_HOF_DATA] = NVTrue;
          options->hotkey[i] = "Ctrl+w";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 2;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;


          //  IMPORTANT NOTE: The following six functions are not ancillary programs.  If you have to add new ancillary programs,
          //  add them above these just for esthetics.

        case INVALIDATE_FEATURES:
          options->prog[i] = "INVALIDATE_FEATURES";
          options->name[i] = "INVALIDATE FEATURES";
          options->description[i] = pfmEdit::tr ("Invalidate all features included in the hotkey polygon.  This is an internal function.");
          options->data_type[i][PFM_UNDEFINED_DATA] = NVTrue;
          options->hotkey[i] = "v";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVTrue;
          options->hk_poly_filter[i] = 1;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;


        case ACCEPT_FILTER_HIGHLIGHTED:
          options->prog[i] = "DELETE FILTER HIGHLIGHTED";
          options->name[i] = "DELETE FILTER HIGHLIGHTED";
          options->description[i] = pfmEdit::tr ("Delete points marked by the filter or highlighted.  This is an internal function.");
          options->data_type[i][PFM_UNDEFINED_DATA] = NVTrue;
          options->hotkey[i] = "Del";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;


        case REJECT_FILTER_HIGHLIGHTED:
          options->prog[i] = "CLEAR FILTER HIGHLIGHTED";
          options->name[i] = "CLEAR FILTER HIGHLIGHTED";
          options->description[i] = pfmEdit::tr ("Clear points marked by the filter/highlighted or restore highlighted invalid points.  This is an internal function.");
          options->data_type[i][PFM_UNDEFINED_DATA] = NVTrue;
          options->hotkey[i] = "Ins";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;


        case DELETE_SINGLE_POINT:
          options->prog[i] = "DELETE SINGLE POINT";
          options->name[i] = "DELETE SINGLE POINT";
          options->description[i] = pfmEdit::tr ("Delete single point nearest cursor in DELETE_POINT mode.  This is an internal function.");
          options->data_type[i][PFM_UNDEFINED_DATA] = NVTrue;
          options->hotkey[i] = "d";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;


          //  IMPORTANT NOTE: The following two functions affect external ancillary programs.  They are used for freezing the marker
          //  or freezing all of the multi-markers.

         case FREEZE:
          options->prog[i] = "FREEZE";
          options->name[i] = "FREEZE MAIN BOX CURSOR";
          options->description[i] = pfmEdit::tr ("Toggle freezing of the box cursor.  This is an internal function that affects ancillary programs.");
          options->data_type[i][PFM_UNDEFINED_DATA] = NVTrue;
          options->hotkey[i] = "f";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;

        case FREEZE_ALL:
          options->prog[i] = "FREEZE ALL";
          options->name[i] = "FREEZE ALL BOX CURSORS";
          options->description[i] = pfmEdit::tr ("Toggle freezing of all of the box cursors.  This is an internal function that affects ancillary programs.");
          options->data_type[i][PFM_UNDEFINED_DATA] = NVTrue;
          options->hotkey[i] = "Alt+F";
          options->action[i] = "";
          options->progButton[i] = NULL;
          options->hk_poly_eligible[i] = NVFalse;
          options->hk_poly_filter[i] = 0;
          options->hk_planview_only[i] = NVFalse;
          options->state[i] = 0;
          break;
        }
    }


  options->min_window_size = 2.0;
  options->iho_min_window = 0;
  options->display_man_invalid = NVFalse;
  options->display_flt_invalid = NVFalse;
  options->display_contours = NVFalse;
  options->display_null = NVFalse;
  options->display_feature = 0;
  options->display_children = NVFalse;
  options->display_reference = NVFalse;
  options->display_feature_info = NVFalse;
  options->display_feature_poly = NVFalse;
  options->display_GeoTIFF = NVTrue;
  options->auto_redraw = NVTrue;
  options->function = DELETE_POLYGON;
  options->slice_percent = 5;
  options->slice_alpha = 16;
  options->rotation_increment = 10.0;
  options->GeoTIFF_alpha = 128;
  options->contour_width = 2;
  options->smoothing_factor = 0;
  options->point_size = 4;
  options->contour_color = QColor (255, 255, 255);
  options->ref_color[0] = QColor (255, 255, 255, 255);
  options->tracker_color = QColor (255, 255, 0);
  options->background_color = QColor (0, 0, 0);
  options->feature_color = QColor (255, 255, 255);
  options->feature_info_color = QColor (255, 255, 255);
  options->feature_poly_color = QColor (255, 255, 0, 255);
  options->feature_highlight_color = QColor (255, 0, 0, 255);
  options->verified_feature_color = QColor (0, 255, 0, 255);
  options->unload_prog = "pfm_unload";
  options->auto_unload = NVFalse;
  options->plan_view_by_line = NVFalse;
  options->alt_view_by_line = NVTrue;
  options->last_rock_feature_desc = 4;
  options->last_offshore_feature_desc = 17;
  options->last_light_feature_desc = 9;
  options->last_lidar_feature_desc = 0;
  options->last_feature_description = "";
  options->last_feature_remarks = "";
  options->overlap_percent = 5;
  options->display_grid = NVFalse;
  options->main_button_icon_size = 24;
  options->color_index = 0;
  options->flag_index = PRE_USER - 1;
  options->screenshot_delay = 2;
  options->filterSTD = 2.4;
  options->deep_filter_only = NVFalse;
  options->feature_radius = 20.0;
  options->distThresh = 4.0;
  options->hofWaveFilter_search_radius = 2.0;
  options->hofWaveFilter_search_width = 8;
  options->hofWaveFilter_rise_threshold = 5;
  options->hofWaveFilter_pmt_ac_zero_offset_required = 10;
  options->hofWaveFilter_apd_ac_zero_offset_required = 5;


  //  Default to red-blue, unlocked, 0.0 for all color bands.

  for (NV_INT32 i = 0 ; i < NUM_HSV ; i++)
    {
      options->min_hsv_color[i] = 0;
      options->max_hsv_color[i] = 240;
      options->min_hsv_locked[i] = NVFalse;
      options->max_hsv_locked[i] = NVFalse;
      options->min_hsv_value[i] = 0.0;
      options->max_hsv_value[i] = 0.0;
    }


  //  Now change the color by depth default to red-magenta.

  options->max_hsv_color[0] = 315;


  // 
  // CHL Addition:        Set the multi-waveform cursor colors.  MAX_STACK_POINTS is 9 so we manually define 9 colors.
  //                                If MAX_STACK_POINTS gets changed you should add/subtrack.
  //

  // white

  misc->abe_share->mwShare.multiColors[0].r = 255;
  misc->abe_share->mwShare.multiColors[0].g = 255;
  misc->abe_share->mwShare.multiColors[0].b = 255;
  misc->abe_share->mwShare.multiColors[0].a = 255;


  // red 

  misc->abe_share->mwShare.multiColors[1].r = 255;
  misc->abe_share->mwShare.multiColors[1].g = 0;
  misc->abe_share->mwShare.multiColors[1].b = 0;
  misc->abe_share->mwShare.multiColors[1].a = 255;


  // dark green 

  misc->abe_share->mwShare.multiColors[2].r = 0;
  misc->abe_share->mwShare.multiColors[2].g = 128;
  misc->abe_share->mwShare.multiColors[2].b = 0;
  misc->abe_share->mwShare.multiColors[2].a = 255;


  // blue 

  misc->abe_share->mwShare.multiColors[3].r = 0;
  misc->abe_share->mwShare.multiColors[3].g = 0;
  misc->abe_share->mwShare.multiColors[3].b = 255;
  misc->abe_share->mwShare.multiColors[3].a = 255;


  // magenta 

  misc->abe_share->mwShare.multiColors[4].r = 255;
  misc->abe_share->mwShare.multiColors[4].g = 0;
  misc->abe_share->mwShare.multiColors[4].b = 255;
  misc->abe_share->mwShare.multiColors[4].a = 255;


  // dark yellow 

  misc->abe_share->mwShare.multiColors[5].r = 128;
  misc->abe_share->mwShare.multiColors[5].g = 128;
  misc->abe_share->mwShare.multiColors[5].b = 0;
  misc->abe_share->mwShare.multiColors[5].a = 255;


  // gray

  misc->abe_share->mwShare.multiColors[6].r = 160;
  misc->abe_share->mwShare.multiColors[6].g = 160;
  misc->abe_share->mwShare.multiColors[6].b = 164;
  misc->abe_share->mwShare.multiColors[6].a = 255;


  // cyan 

  misc->abe_share->mwShare.multiColors[7].r = 0;
  misc->abe_share->mwShare.multiColors[7].g = 255;
  misc->abe_share->mwShare.multiColors[7].b = 255;
  misc->abe_share->mwShare.multiColors[7].a = 255;


  // dark magenta 

  misc->abe_share->mwShare.multiColors[8].r = 128;
  misc->abe_share->mwShare.multiColors[8].g = 0;
  misc->abe_share->mwShare.multiColors[8].b = 128;
  misc->abe_share->mwShare.multiColors[8].a = 255;


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      options->waveColor[i].setRgb (misc->abe_share->mwShare.multiColors[i].r,
                                    misc->abe_share->mwShare.multiColors[i].g,
                                    misc->abe_share->mwShare.multiColors[i].b);
      options->waveColor[i].setAlpha (misc->abe_share->mwShare.multiColors[i].a);
    }



  options->buttonAccel[SAVE_EXIT_KEY] = "Ctrl+s";
  options->buttonAccel[SAVE_EXIT_MASK_KEY] = "Ctrl+f";
  options->buttonAccel[NO_SAVE_EXIT_KEY] = "Ctrl+q";
  options->buttonAccel[RESET_KEY] = "Ctrl+r";
  options->buttonAccel[REDRAW_KEY] = "Alt+r";
  options->buttonAccel[TOGGLE_CONTOUR_KEY] = "Alt+c";
  options->buttonAccel[DELETE_POINT_MODE_KEY] = "F3";
  options->buttonAccel[DELETE_RECTANGLE_MODE_KEY] = "F4";
  options->buttonAccel[DELETE_POLYGON_MODE_KEY] = "F5";
  options->buttonAccel[FILTER_KEY] = "F6";
  options->buttonAccel[ATTR_FILTER_KEY] = "Ctrl+a";
  options->buttonAccel[RECTANGLE_FILTER_MASK_KEY] = "F7";
  options->buttonAccel[POLYGON_FILTER_MASK_KEY] = "F8";
  options->buttonAccel[RUN_HOTKEY_POLYGON_MODE_KEY] = "F9";
  options->buttonAccel[EDIT_FEATURE_MODE_KEY] = "F10";
  options->buttonAccel[UNDO_KEY] = "Ctrl+z";
  options->buttonAccel[DISPLAY_MULTIPLE_KEY] = "F11";
  options->buttonAccel[DISPLAY_ALL_KEY] = "F12";
  options->buttonAccel[CLEAR_HIGHLIGHT_KEY] = "Ctrl+h";
  options->buttonAccel[HIGHLIGHT_POLYGON_MODE_KEY] = "Ctrl+p";
  options->buttonAccel[CLEAR_POLYGON_MODE_KEY] = "Ctrl+c";
  options->buttonAccel[COLOR_BY_DEPTH_ACTION_KEY] = "Ctrl+d";
  options->buttonAccel[COLOR_BY_LINE_ACTION_KEY] = "Ctrl+l";
  options->buttonAccel[AV_DISTANCE_THRESHOLD_KEY] = "Alt+d";
}
