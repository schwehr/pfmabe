
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




/*  C/Motif pfm_load replaced by C++/Qt pfmLoadM on 04/27/05.  All comments for pfm_load
    have been left in for historical (hysterical ???) purposes.  In addition, the pfm_loader
    program has been combined with pfmLoadM so the comments for pfm_loader have been included
    here.  JCD  */


#ifndef VERSION

#define     VERSION     "PFM Software - pfmLoadM V4.74 - 10/06/11"

#endif

/*!< <pre>


******************* pfm_load comments *********************


    Version 1.0
    Jan C. Depner
    10/13/99


    Version 2.0
    Jan C. Depner
    02/15/00

    Went to version 2.0 of the PFM library.


    Version 3.0
    Jan C. Depner
    04/19/00

    Completely rewritten.  Looks much better doesn't it?


    Version 3.01
    Jan C. Depner
    04/27/00

    Force the .pfm extension on file creation.


    Version 3.02
    Jan C. Depner
    07/17/00

    Allowed ASCII area files.


    Version 3.03
    Jan C. Depner
    08/20/00

    Added area filter parameters.  Changed SHOALS .out file input section.


    Version 3.04
    Jan C. Depner
    02/21/01

    Passing scale to open_pfm_file as a pointer.


    Version 4.0
    Jan C. Depner
    07/19/01
 
    4.0 PFM library changes.


    Version 4.1
    Jan C. Depner
    09/30/01
 
    Fixed bug in file_cbs.c that was causing the script to be incorrect.


    Version 4.11
    Jan C. Depner
    10/23/01
 
    Changed the run script file name to the first list file name with a
    suffix of .load_run.  Changed size of definitoin page (taller).


    Version 4.12
    Jan C. Depner
    10/30/01
 
    Increased allowable standard deviation filter range to 6.0 from 4.0.


    Version 4.2
    Jan C. Depner
    06/06/02
 
    Removed the swath filter stuff.


    Version 4.3
    Jan C. Depner
    06/15/03
 
    Added ability to use "reference masking".


    Version 4.4
    Jan C. Depner
    07/18/03
 
    Removed experimental "reference masking" and replaced with PFM_REFERENCE
    flagging.  Went to 32 bits for validity fields.


    Version 4.5
    Jan C. Depner
    02/24/04
 
    Changed target file mask to *.xml to support new XML format target files.


    Version 4.6
    Jan C. Depner
    10/11/04
 
    Added [HOF Load Null Flag] and [Invert Substitute Paths Flag] options.
    Added ability to get all files under a directory that match the Filter
    mask in the input multiple file selection box (uses a shelled "find"
    command).


    Version 4.7
    Jan C. Depner
    10/22/04
 
    Added [Invalidate HOF Land Flag] option.


    Version 4.71
    Jan C. Depner
    11/12/04
 
    Bug fix in file_cbs.c for new FC3 compiler.


    Version 4.8
    Jan C. Depner
    12/15/04
 
    Handle the new (PFM 4.5) way that the library creates PFM files - a directory
    with hard-wired filenames.


    Version 4.9
    Jan C. Depner
    01/19/05
 
    Button for loading IVS standard attributes with HOF/TOF data.


    Version 4.91
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 4.92
    Jan C. Depner
    03/04/05

    Fix return from open_existing_pfm_file.



******************* pfm_loader comments *********************



#ifndef VERSION

#define     VERSION     "PFM_Software - pfm_loader V6.52 - 03/23/05"

#endif


    Version 1.0
    Jan C. Depner
    05/28/99


    Version 2.0
    Jan C. Depner
    02/15/00

    Went to version 2.0 of the PFM library.


    Version 2.1
    Jan C. Depner
    03/29/00

    Uses version 2.1 of the pfm library.


    Version 3.0
    Jan C. Depner
    04/20/00

    Changed the input to come from stdin and uses [key] = syntax.


    Version 3.1
    Jan C. Depner
    07/06/00

    Loads SHOALS xy2 (11 column) format and merge format.


    Version 3.11
    Jan C. Depner
    07/17/00

    Now able to use standard ASCII area files in addition to ISS-60 .ARE files.


    Version 3.12
    Jan C. Depner
    08/18/00

    Added area filter parameters to inputs.  Added function to clear edits
    around known targets.


    Version 3.2
    Jan C. Depner
    08/27/00

    Finally, the damn thing will handle multiple PFM files!


    Version 3.3
    Jan C. Depner
    09/04/00

    Fixed bugs in the swath filter.  It wouldn't unload the last window if
    it wasn't full.  Also, it wouldn't flag the swath filtered data as 
    PFM_MODIFIED.


    Version 3.4
    Jan C. Depner
    09/27/00

    Fixed the memory leak in the swath filter.  Removed redundant 
    recompute_bin_values from main.c.


    Version 4.0
    Jan C. Depner
    09/29/00

    Replaced along-row running average and STD with nine cell average.  See
    PFM_Filter.c for explanation of algorithm.


    Version 4.1
    Jan C. Depner
    10/12/00

    Modified slightly to work with the more efficient pfm_lib V2.6.


    Version 4.2
    Jan C. Depner
    12/12/00

    Made PFM_Filter buffer up three rows of bin and depth data so that we 
    could compute the average filtered depth and standard deviation on the fly.
    This allowed us to remove the recompute bin loop in the main program when
    filtering data.


    Version 4.21
    Jan C. Depner
    02/21/01

    Trying to catch all of the little bugs in the code.  Check for a handle
    if gsfOpen fails.  This is needed in case the index file is bad or can't
    be opened.  Also, fixed the looping in the scroll bars.


    Version 4.22
    Jan C. Depner
    02/23/01

    Added an error log file.


    Version 4.23
    Jan C. Depner
    02/28/01

    Fixed problem trying to open nonexistent GSF file.


    Version 4.24
    Jan C. Depner
    05/03/01

    Remove the GSF index files before reading them in.  This gets rid of
    any corrupted GSF index files and then regenerates them on open.  Don't
    know what was causing the corrupted GSF index files.  Apparently some
    other process.


    Version 5.0
    Jan C. Depner
    07/19/01
 
    4.0 PFM library changes.


    Version 5.02
    Jan C. Depner
    10/31/01
 
    Changed the name of the errors file to something that makes more sense.
    Made SASS specific records use "nominal" depth since there is no "true"
    depth for SASS.


    Version 5.03
    Jan C. Depner
    11/23/01
 
    No longer errors out on pre-check.  Outputs an error message and goes
    on with load of other files.


    Version 5.04
    Jan C. Depner
    12/03/01
 
    For some unknown reason some GSF files may have all records with beam
    flags except one.  This was bombing the load.  Now I check the beam_flags
    array prior to looking at the data.


    Version 5.05
    Jan C. Depner / Jeff Parker
    05/01/01
 
    Added Jeff's changes to fix projected data problems.


    Version 5.1
    Jan C. Depner
    06/06/02
 
    Removed the swath filter stuff.  PFM 4.1 changes (attributes/classes).


    Version 5.11
    Jeff Parker (SAIC)
    07/12/02
 
    Added correction to get_area_mbr.c to handle RECTANGLE and ELLIPSE
    area files.


    Version 5.12
    Jan C. Depner
    07/23/02
 
    Added correction for filename array.


    Version 5.2
    Jan C. Depner
    11/18/02
 
    Added DEMO data type.


    Version 5.21
    Jan C. Depner
    12/12/02
 
    Close and reopen the PFM files between reading data and recomputing.


    Version 5.3
    Jan C. Depner
    06/15/03
 
    Added UNISIPS depth data type.  Added ability to reference mask the input
    data.


    Version 5.4
    Jan C. Depner
    07/18/03
 
    Removed experimental reference masking and replaced with support for
    PFM_REFERENCE flagging.  Went to 32 bits for validity fields.


    Version 5.5
    Jan C. Depner
    08/03/03
 
    Added support for SHOALS/CHARTS HOF format.


    Version 5.6
    Jan C. Depner
    08/06/03
 
    Added support for SHOALS/CHARTS TOF format.


    Version 5.7
    Jan C. Depner
    08/24/03
 
    Fixed problems with loading TOF values set to -998.0.  Also fixed insidious
    elevation limit problem.


    Version 5.8
    Jan C. Depner
    10/23/03
 
    Decreased storage for filenames.  Changed variable name in load_file.c
    to be more clear.


    Version 5.9
    Jan C. Depner
    12/22/03
 
    Added support for generic YXZ files, signed degrees decimal, space 
    delimited.


    Version 6.0
    Jan C. Depner
    02/24/04
 
    Added support for the new XML target file format.  Removed support for
    the old HYDRO .tgt format.


    Version 6.1
    Jan C. Depner
    04/12/04
 
    Sorting data by row/col index prior to loading to decrease the load time
    and disk thrashing.  Also fixed real stupid thing in clear_targets - don't
    ask, you don't have a need to know ;-)


    Version 6.2
    Jan C. Depner
    09/01/04
 
    Saving -998.0 hof depths as null depths with PFM_USER_04 set to match IVS
    load.


    Version 6.3
    Jan C. Depner
    10/11/04
 
    Made saving -998.0 hof depths as null depths optional [HOF Load Null Flag].
    Added support for inverting Windoze substitute paths 
    [Invert Substitute Paths Flag].


    Version 6.4
    Jan C. Depner
    10/29/04
 
    Shifted PFM numbering of HOF and TOF to start at 1 so that we would be compatible
    with IVS.  Made the HOF and TOF precheck read the header of the files to look at
    the file type string.


    Version 6.41
    Jan C. Depner
    11/17/04
 
    Minor bug fix in hof loader.


    Version 6.42
    Jan C. Depner
    12/10/04
 
    Turn on dynamic_reload to be compatible with IVS pfm_direct and Optech GCS.


    Version 6.43
    Jan C. Depner
    01/14/05
 
    Added support for IVS ASCII xyz format (PFM_ASCXYZ_DATA).


    Version 6.5
    Jan C. Depner
    01/19/05
 
    Now loading IVS standard attributes for LIDAR data.


    Version 6.51
    Jan C. Depner
    03/04/05
 
    Fix return from open_pfm_file.


    Version 6.52
    Jan C. Depner
    03/23/05
 
    Don't set HOF LIDAR nulls to PFM_INVALID or PFM_MODIFIED so GCS can
    deal with them properly if we want to reprocess them.



******************* pfmLoadM comments *********************



    Version 1.0
    Jan C. Depner
    04/27/05

    First version of C++/Qt pfmLoadM.


    Version 1.01
    Jan C. Depner
    05/23/05

    Added TOF first return reference flag and modified the PFM Browse functionality 
    a bit.


    Version 1.02
    Jan C. Depner
    06/06/05

    Fix bug reading in parameters file - no setField call after input files are being read.
    Changed HOF PFM_USER flags to Second Depth Present, Shoreline Depth Swapped, Land, and 
    NBR.


    Version 1.03
    Jan C. Depner
    06/26/05

    Now checks for SHOALS 3000TH data types in HOF and TOF file checks.  Flags with Bare 
    Earth! instead of NBR!


    Version 1.04
    Jan C. Depner
    07/21/05

    Added Deep Filter Only option since deep flyers have become a problem with the new SHOALS3000THE LIDAR data.


    Version 1.05
    Jan C. Depner
    08/15/05

    Removed support for merge and demo formats.


    Version 1.06
    Jan C. Depner
    10/26/05

    Now uses the PFM 4.6 handle file or list file instead of the PFM directory name.


    Version 1.07
    Jan C. Depner
    11/08/05

    Added load GSF nominal depth flag.


    Version 1.08
    Jan C. Depner
    12/30/05

    Fixed hypack loader bug.


    Version 1.1
    Jan C. Depner
    04/10/06

    Replaced QVBox, QHBox, QVGroupBox, QHGroupBox with QVBoxLayout, QHBoxLayout, QGroupBox to prepare for
    Qt 4.  Changed help button to WhatsThis icon.


    Version 1.11
    Jan C. Depner
    05/10/06

    Minor bug fix for rebuild PFM - checks to see if X and Y bin sizes in degrees are identical.  If so, it
    populates the "minutes" bin size instead of "meters".


    Version 1.12
    Jan C. Depner
    05/18/06

    Changed appearance of file input page.


    Version 1.13
    Jan C. Depner
    05/31/06

    Now opens files to check and see if they're UNISIPS since ISS-60 doesn't use the proper file extension.


    Version 1.2
    Jan C. Depner
    06/30/06

    Added ability to shell areaCheck in order to make or modify the area file.


    Version 1.21
    Jan C. Depner
    08/03/06

    Removed the percent checks in load_file.cpp.


    Version 1.3
    Jan C. Depner
    08/04/06

    Append summary data to end of PFM handle file as comments.


    Version 1.31
    Jan C. Depner
    08/07/06

    Changed max depth from 10000 to 12000... DOH!  Changed GSF file mask to include *.gsf.  Load true depth if
    nominal depth is requested but not available.


    Version 1.32
    Jan C. Depner
    08/25/06

    Doesn't error out on add_depth_record out of area.


    Version 1.33
    Jan C. Depner
    08/31/06

    Added support for NAVO LLZ data format.


    Version 1.34
    Jan C. Depner
    10/06/06

    Now loading ALL tof values even if first and last are identical.


    Version 1.35
    Jan C. Depner
    01/05/07

    Flag auto shallow processed data in HOF files (confidence = 74).


    Version 1.36
    Jan C. Depner
    01/17/07

    If the difference between the first and last TOF returns is less than 5 cm we will not load the first 
    return and will give it the last return validity on unload.


    Version 1.4
    Jan C. Depner
    02/05/07

    Added ability to load Shuttle Radar Topography Mission (SRTM) data if available.  Loads as PFM_NAVO_ASCII_DATA.


    Version 1.41
    Jan C. Depner
    02/09/07

    Oops!  I can't count!  DOH!!!


    Version 1.42
    Jan C. Depner
    02/13/07

    Fix one lat grid cell offset in read of SRTM topo data.


    Version 1.43
    Jan C. Depner
    02/19/07

    Fix point count when reading only SRTM data.


    Version 1.5
    Jan C. Depner
    03/23/07

    Fixed bug in clear_targets.  Sets the SRTM flags instead of trying to load the fake SRTM files.


    Version 1.51
    Jan C. Depner
    03/25/07

    Wasn't picking up the global options.


    Version 1.52
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 1.53
    Jan C. Depner
    09/17/07

    Replaced compute_index with compute_index_ptr.


    Version 1.54
    Jan C. Depner
    10/01/07

    Fixed some small GUI bugs.


    Version 1.55
    Jan C. Depner
    10/31/07

    Fixed "off by one" error in longitude in SRTM read code.  Happy Halloween!


    Version 1.56
    Jan C. Depner
    11/06/07

    Got rid of extra data reads in SRTM read code.


    Version 2.00
    Jan C. Depner
    12/03/07

    Switched to cached_io PFM calls.  Which I've probably screwed up royally ;-)
    Added fflush calls after prints to stderr since flush is not automatic in Windows.
    Added SRTM2 (limdis) data input support.
    Added DTED data input support and removed SHOALS .out and .xy2 support.


    Version 2.01
    Jan C. Depner
    12/21/07

    Fixed PFM file Browse dialog so that you could define a new name in a new directory.


    Version 2.02
    Jan C. Depner
    02/07/08

    Allow definition of an area by extracting it from a pre-existing PFM structure.


    Version 2.03
    Jan C. Depner
    03/10/08

    Came up with reasonable value for max horizontal error (I guess ;-)  Also, fixed the directory browse function
    so that you don't have to navigate around if you want the current working directory.


    Version 2.04
    Jan C. Depner
    03/14/08

    Disable the deep filter, std, and target radius when apply filter is unselected.  Load horizontal and vertical
    errors from GSF.


    Version 2.05
    Jan C. Depner
    03/28/08

    Added NSEW button to area definition section of pfmPage (Paul Marin request).


    Version 2.06
    Jan C. Depner
    04/02/08

    Fixed the CUBE parameters stuff (added the two new SAIC parameters).


    Version 2.07
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 2.08
    Jan C. Depner
    04/08/08

    Corrected the directory browse location problem.


    Version 3.00
    Jan C. Depner
    04/21/08

    Added NAVO PFM CUBE processing options.


    Version 3.01
    Jan C. Depner
    04/23/08

    Added NAVO PFM CUBE settings options in the Global Parameters page.  Removed Along Track Filters and Expert Only stuff.


    Version 3.02
    Jan C. Depner
    05/14/08

    Make sure that the default H/V errors aren't 0.0 if the CUBE attributes are to be loaded.  Also, make sure we can read
    input files prior to checking for type.


    Version 3.03
    Jan C. Depner
    05/27/08

    Switched to Abbreviated Depth Confidence as the phony beam number for HOF data.  Also check it for being
    out of bounds.


    Version 3.04
    Jan C. Depner
    06/02/08

    Computing HOF horizontal and vertical errors in the CHARTS library instead of setting to the default values.


    Version 3.05
    Jan C. Depner
    06/05/08

    Check for comments (#) in .pfm_cfg file when doing "invert paths";


    Version 3.06
    Jan C. Depner
    10/21/08

    Allow .afs area files.


    Version 3.07
    Jan C. Depner
    11/24/08

    Corrected loading of PFM_USER flags for HOF data.


    Version 3.08
    Jan C. Depner
    12/08/08

    Added ability to load CHRTR data (with no unload capability at present).


    Version 3.09
    Jan C. Depner
    03/02/09

    Added ability to load WLF data.


    Version 4.00
    Jan C. Depner
    03/23/09

    Added selectable attributes by data type for GSF, HOF, TOF, and WLF.


    Version 4.01
    Jan C. Depner
    04/09/09

    Added the HOF user flags back in... DOH!


    Version 4.02
    Jan C. Depner
    04/13/09

    Fixed bug that caused checkboxes to uncheck if you set them prior to setting attributes.


    Version 4.03
    Jan C. Depner
    04/14/09

    Fixed bug that caused TOF to not uncheck (cut and paste error).  Switched to Binary Feature Data (BFD)
    from NAVO standard XML target files.


    Version 4.04
    Jan C. Depner
    04/27/09

    Make sure we send just the handle name to navo_pfm_cube instead of the ctl file name.


    Version 4.05
    Jan C. Depner
    05/07/09

    Fix invert paths bug introduced when fixing last bug ;-)


    Version 4.06
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 4.07
    Jan C. Depner
    06/12/09

    Fix the dateline crossing problem.


    Version 4.10
    Jan C. Depner
    06/15/09

    Replaced type of PFM_SHOALS_1K_DATA with PFM_CHARTS_HOF_DATA for Optech HOF files.


    Version 4.11
    Jan C. Depner
    06/16/09

    Cleaned up the GUI for the run page.


    Version 4.12
    Jan C. Depner
    06/24/09

    Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


    Version 4.13
    Jan C. Depner
    07/29/09

    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 4.14
    Jan C. Depner
    07/31/09

    Added the ability to load HOF data in either the new PFM_CHARTS_HOF_DATA form or the old, GCS compatible, 
    PFM_SHOALS_1K_DATA form.


    Version 4.15
    Jan C. Depner
    08/03/09

    Wasn't invalidating secondary HOF records when "Invalidate HOF Secondary Values" was selected.  Fixed
    bug that caused error message to keep re-appearing if the feature file was gone on a rebuild.


    Version 4.16
    Jan C. Depner
    09/24/09

    Change the window title to the first PFM name when the user presses the Run button.


    Version 4.17
    Jan C. Depner
    10/22/09

    Allow loading of HOF data with ABDC = 0 if we're loading NULL HOF points.


    Version 4.18
    Jan C. Depner
    12/17/09

    Fixed one beam multibeam GSF load error.


    Version 4.19
    Jan C. Depner
    12/29/09

    Stopped the filter from filtering points when there is not enough surrounding valid data.


    Version 4.20
    Jan C. Depner
    01/07/10

    Fixed an SRTM load bug.  Also fixed another single-beam GSF load bug.


    Version 4.21
    Jan C. Depner
    01/11/10

    Fixed SRTM load bug at equator.


    Version 4.22
    Jan C. Depner
    01/15/10

    Yet another one-beam multibeam loading bug.


    Version 4.23
    Jan C. Depner
    02/05/10

    Line names now contain start date and time of line (GSF, HOF, TOF, WLF).


    Version 4.24
    Jan C. Depner
    02/26/10

    Forgot to set PFM_MODIFIED when using "invalidate" options.


    Version 4.25
    Jan C. Depner
    03/04/10

    Added multiple directory selection to the directory browse option in inputPage.cpp.


    Version 4.26
    Jan C. Depner
    03/11/10

    Added ability to store date/time in POSIX minutes (minutes from 01/01/1970) as an attribute for
    GSF, HOF, TOF, and WLF data.


    Version 4.27
    Jan C. Depner
    03/17/10

    Added memset to 0 of all of the OPEN_ARGS structures.  This hasn't been an issue but we've probably just been lucky.
    Happy Saint Patrick's Day!


    Version 4.28
    Jan C. Depner
    04/15/10

    Added ability to load time from LLZ as an attribute.
    Happy Tax Day!


    Version 4.29
    Jan C. Depner
    04/30/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 4.30
    Jan C. Depner
    07/06/10

    Removed incipient projected PFM support.  I was never going to finish it anyway ;-)


    Version 4.31
    Jan C. Depner
    08/30/10

    Changed HOF loader to check the absolute value of the abdc so that points that were originally above
    70 but marked as invalid by negating (i.e. -93) will be marked as manually invalid instead of filter invalid.


    Version 4.32
    Jan C. Depner
    09/21/10

    Added changes to support CZMIL test data.


    Version 4.33
    Jan C. Depner
    09/22/10

    Outputs the polygon points to the .prm file instead of the area file name.  This way we can lose of change the
    area file without effecting the .prm file.  This makes more sense anyway since we were depending on an external
    file in what was essentially a script.  We still support the old .prm files with the [Area File] field.


    Version 4.34
    Jan C. Depner
    10/20/10

    Time as a depth attribute is now set across all input files that have time.  Speeded the whole thing up quite a bit
    by making the progress bars only update at 5% intervals.


    Version 4.35
    Jan C. Depner
    11/15/10

    No longer removes GSF index files and regenerates them.  Corrupted GSF index files used to be a problem but
    this has gone away in recent years.  If you do have a problem you can always manually remove the index files.
    Added support for ingesting BAG data.


    Version 4.36
    Jan C. Depner
    12/13/10

    Fixed bug when setting HOF standard user flags.  I forgot that abdc of -70 is land just as abdc of 70 is.


    Version 4.37
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 4.40
    Jan C. Depner
    01/17/11

    Added support for CHRTR2 data as we phase out the old CHRTR format.  Still using PFM_CHRTR_DATA flag.  Check the 
    file name to determine the actual type (for now).  Also, removed rebuild and parameter files from start page.
    These options are seldom used and you can essentailly do a rebuild using a pre-existing PFM just by changing the
    name.  The parameter file input is available by adding the parameter file to the command line.  This makes the
    interface much cleaner.


    Version 4.50
    Jan C. Depner
    02/08/11

    Added support for AHAB Hawkeye attributes.


    Version 4.51
    Jan C. Depner
    02/16/11

    Now converts path separators to native separators (\ on Windows) before writing the file name to the list file.


    Version 4.52
    Jan C. Depner
    02/23/11

    Shortened HAWKEYE attribute names because they were blowing out the PFM attribute name fields.


    Version 4.53
    Jan C. Depner
    02/25/11

    Now loads both PFM_HAWKEYE_HYDRO_DATA and PFM_HAWKEYE_TOPO_DATA.


    Version 4.54
    Jan C. Depner
    03/03/11

    Added "Clear" button to attribute dialogs.


    Version 4.55
    Jan C. Depner
    03/08/11

    Added warning when trying to use GCS compatible mode.


    Version 4.56
    Jan C. Depner
    03/15/11

    Fixed relative path bug in pfmPage.cpp.


    Version 4.60
    Jan C. Depner
    04/14/11

    Added ability to load from file list (*.lst) files.


    Version 4.61
    Jan C. Depner
    04/25/11

    Removed the "shoreline depth swapped" HOF lidar user flag and combined it with the "shallow water processed" HOF
    lidar user flag.  Replaced the "shoreline depth swapped" user flag with an "APD" user flag.  Since "shoreline depth swapped"
    (the old shallow water processing method) is never used with "shallow water processed" data processing this should
    cause no problems.


    Version 4.62
    Jan C. Depner
    06/16/11

    Removed HMPS_SPARE_1 flag check since we don't do swath filtering anymore and it's been removed from hmpsflag.h.


    Version 4.63
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


    Version 4.70
    Jan C. Depner
    07/21/11

    Added support for DRAFT CZMIL data.


    Version 4.71
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.


    Version 4.72
    Jan C. Depner
    08/15/11

    Added in all of the currently available CZMIL attributes.  Many more to come.


    Version 4.73
    Jan C. Depner
    09/21/11

    Added ability to create an "update" parameter file (.upr) for use by pfmLoadMer.
    Replaced bin_inside calls with bin_inside_ptr calls.
    Added U.S. Government diclaimer comments to all files.


    Version 4.74
    Jan C. Depner
    10/06/11

    Fixed bug in TOF loader.  I was under the impression that if the last return was bad (-998.0)
    then the first return must be bad as well.  This is not the case.

</pre>*/
