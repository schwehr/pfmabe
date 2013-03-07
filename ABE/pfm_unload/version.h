
/*********************************************************************************************

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef VERSION

#define     VERSION     "PFM Software - pfm_unload V5.04 - 08/03/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    10/28/99


    Version 2.0
    Jan C. Depner
    02/15/00

    Went to version 2.0 of the PFM library.


    Version 2.1
    Jan C. Depner
    03/29/00

    Added SHOALS output.


    Version 2.2
    Jan C. Depner
    06/22/00

    Moved the history record write to after the file is closed.


    Version 2.3
    Jan C. Depner
    07/06/00

    Unloads merge format.


    Version 2.4
    Jan C. Depner
    09/04/00

    Replaced call to read_depth_record_index with read_depth_array_index.


    Version 2.5
    Jan C. Depner
    12/15/00

    Added undocumented "dump_all" feature.


    Version 2.6
    Jan C. Depner
    02/06/01

    After trying to unload an area that had over 2 billion soundings with 
    110 million edited I had to remove the sort.  It couldn't handle it.  So
    now it just opens and closes the files as it writes out the points.  It 
    will leave open a file until the file number changes though so it's not
    too bad.  There is a noticeable performance hit but it's not terrible
    since this is an unattended function.


    Version 2.61
    Jan C. Depner
    02/21/01

    Trying to catch all of the little bugs in the code.  Check for a handle
    if gsfOpen fails.  This is needed in case the index file is bad or can't
    be opened.


    Version 2.62
    Jan C. Depner
    05/03/01

    Remove the GSF index files before writing out changes.  This gets rid of
    any corrupted GSF index files and then regenerates them on open.  Don't
    know what was causing the corrupted GSF index files.  Apparently some
    other process.


    Version 2.63
    Jan C. Depner
    05/13/01

    Fixed a problem with the gettimeofday call in write_history.c.  Replaced
    it with a call to time.


    Version 2.64
    Jan C. Depner
    06/22/01

    Pass structure args to open_pfm_file.


    Version 3.0
    Jan C. Depner
    07/19/01
 
    4.0 PFM library changes.


    Version 3.1
    Jan C. Depner
    10/25/01
 
    Removed requirement for data to be marked as checked prior to unload.


    Version 3.2
    Jan C. Depner
    06/06/02
 
    Uses PFM_USER flags now (from PFM 4.1).  Maps the GSF flags correctly.


    Version 3.3
    Jan C. Depner
    11/18/02
 
    Added DEMO data type.


    Version 4.0
    Jan C. Depner
    06/12/03
 
    Added UNISIPS depth data type.  Added ability to handle reference masking.


    Version 4.1
    Jan C. Depner
    07/18/03
 
    Removed experimental reference masking and replaced with support for
    PFM_REFERENCE flag.  Went to 32 bits for validity fields.


    Version 4.11
    Jan C. Depner
    08/01/03
 
    Don't unload reference data.


    Version 4.2
    Jan C. Depner
    08/04/03
 
    Support for CHARTS HOF format.


    Version 4.3
    Jan C. Depner
    08/06/03
 
    Support for CHARTS TOF format.


    Version 4.4
    Jan C. Depner
    08/20/03
 
    Added partial unload to be shelled from pfm_view or pfm_edit.


    Version 4.5
    Jan C. Depner
    12/09/03
 
    Fixed computation of percentage on partial unload.


    Version 4.51
    Jan C. Depner
    05/19/04
 
    Changed undocumented unload all feature to -u option.


    Version 4.52
    Jan C. Depner
    10/27/04
 
    Wasn't setting the abdc to a negative for hof and/or tof.


    Version 4.6
    Jan C. Depner
    10/29/04
 
    Shifted PFM numbering of HOF and TOF to start at 1 so that we would be compatible
    with IVS.


    Version 4.61
    Jan C. Depner
    11/19/04
 
    Did the record shifting the correct way - in the libraries.


    Version 4.62
    Jan C. Depner
    12/16/04
 
    Changed Usage message for PFM 4.5 directory input.


    Version 4.63
    Jan C. Depner
    01/19/05
 
    Use |= instead of = when setting DELETED status in hof files.


    Version 4.64
    Jan C. Depner
    02/08/05
 
    Prliminary changes to support C++/Qt pfmView.


    Version 4.65
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 4.66
    Jan C. Depner
    03/04/05

    Fix return from open_existing_pfm_file.


    Version 4.67
    Jan C. Depner
    04/20/05

    Removed remnants of IVS licensing stuff.


    Version 4.68
    Jan C. Depner
    06/29/05

    Removed "Press enter to finish".


    Version 4.69
    Jan C. Depner
    10/26/05

    Changed usage for PFM 4.6 handle file use.


    Version 4.70
    Jan C. Depner
    04/20/06

    Removed "merge" file support.


    Version 4.71
    Jan C. Depner
    08/31/06

    Removed "demo" file support.  Added support for LLZ.


    Version 4.72
    Jan C. Depner
    01/17/07

    Invalidating TOF first returns if the last return is within 5 cm of the first return and the
    last return is invalid.  We don't load first returns in pfmLoad if they are within 5 cm
    of the last return but we want to make sure that they get invalidated on unload if they are
    essentially the same value as the last return.


    Version 4.73
    Jan C. Depner
    02/12/07

    Output error_on_update error message even when running from pfmView (Qt == NVTrue).


    Version 4.74
    Jan C. Depner
    02/14/07

    Don't close the last GSF file until we flush the buffer.


    Version 4.74
    Jan C. Depner
    09/17/07

    Replaced compute_index with compute_index_ptr.


    Version 4.75
    Jan C. Depner
    10/22/07

    Added fflush calls after prints to stderr since flush is not automatic in Windows.


    Version 4.77
    Jan C. Depner
    11/15/07

    Added support for writing zeros back to dted files when data is invalidated.


    Version 4.78
    Jan C. Depner
    03/28/08

    Inverted the nocheck option.  We've had fewer problems with corrupted GSF files lately
    so it's not really a neccesity anymore.  Made the -s and -a options undocumented since -a
    is only used from pfmView and -s is not really needed anymore (GSF corruption issue).


    Version 4.79
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 4.80
    Jan C. Depner
    06/05/08

    Cleaned up the dump_all option.  Since we're not resetting the modified flag in the depth
    record we don't need to check for invalid if we use the -u option.  Cleaned up indentation
    to match xemacs style.


    Version 4.81
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 4.82
    Jan C. Depner
    03/02/09

    Now handles WLF format data.


    Version 4.83
    Jan C. Depner
    06/15/09

    Added support for PFM_CHARTS_HOF_DATA.  Both primary and secondary returns are considered valid
    unless marked otherwise.


    Version 4.84
    Jan C. Depner
    06/25/10

    Now handles HAWKEYE format data.


    Version 4.85
    Jan C. Depner
    10/21/10

    Fix percent spinner when we're only unloading part of the area (like from pfmView).


    Version 4.86
    Jan C. Depner
    02/25/11

    Switched to using PFM_HAWKEYE_HYDRO_DATA and PFM_HAWKEYE_TOPO_DATA to replace the deprecated PFM_HAWKEYE_DATA.


    Version 4.87
    Jan C. Depner
    03/09/11

    Added a check for PFMWDB files so that we can try to unload them.  To unload them they must be moved to the PFMWDB
    directory and the unload operation run from there.  The easiest way to do this would be to use the find command
    like this:           find . -name \*.pfm -print -exec pfm_unload {} \;


    Version 4.90
    Jan C. Depner
    04/28/11

    Now adds 100 to abdc for HOF records that have been manually marked valid but were marked invalid (abdc < 70) by GCS.
    This way, pfmLoad will not automatically mark them as invalid (due to abdc < 70) if we build a new PFM.


    Version 5.00
    Jan C. Depner
    05/05/11

    Reads all modified records into memory and then sorts them by file and record number prior to unloading the edits to the
    input files.  This should save us some major disk thrashing on remote disks.  Keep in mind that if you have modified
    5 million records you will require 80MB of memory to do the unload.  On modern systems that shouldn't be a problem (and we
    usually fon't modify that many records).  Happy Cinco de Mayo!!!


    Version 5.01
    Jan C. Depner
    05/06/11

    Fixed problem with getopt that only happens on Windows.


    Version 5.02
    Jan C. Depner
    06/16/11

    Removed HMPS_SPARE_1 flag check since we don't do swath filtering anymore and it's been removed from hmpsflag.h.


    Version 5.03
    Jan C. Depner
    06/21/11

    Now override the AU_STATUS_DELETED_BIT in HOF records when the user declares data valid.


    Version 5.04
    Jan C. Depner
    08/03/11

    Will now error out if it can't write to the HOF or TOF file.

*/
