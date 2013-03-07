QString pfm_fileText = 
  startPage::tr ("Use the browse button to select an input PFM list or handle file.  You cannot modify the text in the "
                 "<b>PFM File</b> text window.  The reason for this is that the file must exist in order for the "
                 "program to run.  Note also that the <b>Next</b> button will not work until you select an input "
                 "PFM file.  When one is selected the default output file name will be supplied in the <b>Output "
                 "Feature File</b> text window.  The feature file name can be edited since it may be a new file or a "
                 "pre-existing file.");

QString pfm_fileBrowseText = 
  startPage::tr ("Use this button to select the input PFM list or handle file");

QString output_fileText = 
  startPage::tr ("You may enter a new file name to be used for the output feature file or modify the default file name "
                 "that was placed here when you selected the input PFM file.");

QString output_fileBrowseText = 
  startPage::tr ("Use this button to select a pre-existing feature file to be overwritten.");

QString area_fileText = 
  startPage::tr ("This is the optional area file name for the file that will be used to limit the area of the PFM file from "
                 "which to select features.  Use the <b>Browse</b> button to select the file.  The area file may be in ISS-60 "
                 "format (*.ARE), generic format (*.are), or USACE format (*.afs).");

QString area_fileBrowseText = 
  startPage::tr ("Use this button to select an optional area file.");

QString ex_fileText = 
  startPage::tr ("This is the optional area file name for the file that will be used to define an area to exclude when creating "
                 "features.  This is useful if you have already selected a small area at special order and then want to select "
                 "a surrounding or adjacent area at order 1.  Use the <b>Browse</b> button to select the file.  The area "
                 "file may be in ISS-60 format (*.ARE), generic format (*.are), or USACE format (*.afs).");

QString ex_fileBrowseText = 
  startPage::tr ("Use this button to select an optional exclusion area file.");

QString ridiculousText = 
  startPage::tr ("<br><center><b>Ridiculously Long Explanation Of Feature Selection Algorithm</b></center><br><br>"
                 "The feature selection algorithm uses a simple moving bin/window with overlap technique.  The first feature "
                 "selection pass is done with a 3 meter bin size using a 1 meter overlap in both X and Y directions.  "
                 "We search each 3 meter bin for the shoalest point.  This point may fall in a single bin, two bins if it is "
                 "on the edge of the bin or, four bins if it is in one of the corners.  For each shoal point we set an octant "
                 "number for each of the points in all nine bins.  These octants are divided at 0, 45, 90, 135, 180, 225, 270, "
                 "and 315 degrees using the shoal point as the center (where 0-45 is octant 0 and 315-360 is octant 7).  We "
                 "then check to see if there is a point in each octant that is at least IHO special order or order one criteria "
                 "deeper than the shoal point.  If this is the case then the shoal point is flagged as a possible feature and added "
                 "to the 3 meter bin feature list.  If we don't have a point in all 8 octants we check for points that fall in "
                 "octants 7 or 0, 1 or 2, 3 or 4, and 5 or 6 that meet the criteria.  We then check for points in octants "
                 "0 or 1, 2 or 3, 4 or 5, and 6 or 7 that meet the criteria.  If we have hits in all 8 of these larger (90 degree "
                 "slice) quadrants we mark the shoal point as a possible feature.  This allows ridges to be flagged as features since "
                 "one or more of the smaller (45 degree slice) octants did not have a point that was deep enough to cause a hit.  "
                 "Subsequent passes are done at 6 meter, 12 meter, and, for order 1, 24 meter bin sizes using a 1/3 bin size "
                 "overlap.<br><br>"
                 "The next step is to deconflict the features.  The first pass is to deconflict the N meter bin features with all of "
                 "the other N meter bin features (N being either 3, 6, 12, or 24).  This is done by sorting all of the features by "
                 "depth and then checking to see if any deeper features fall within the diagonal of the bin size from each feature "
                 "(the 3 meter pass uses 2.83 meters for order 1 and 1.414 meters for special order since these are the diagonals of a "
                 "2 meter and 1 meter square respectively).  If there is a feature within that radius that is deeper it is marked "
                 "as invalid.  The second pass is to deconflict each of the N meter bin features with all of the features of a different "
                 "bin size.  We compare the smaller bin size features against the larger bin size features using the bin diagonal (or the "
                 "special sizes for 3 meter bins) of the smaller bin size.  If we find a feature and it is deeper than the smaller bin "
                 "feature we invalidate the deeper (i.e. larger bin) feature.  If we find a feature that is shoaler than the smaller bin "
                 "feature we invalidate the smaller bin feature.<br><br>"
                 "The last step is to output all of the features to a NAVO standard feature file.  It should be intuitively obvious to "
                 "the most casual observer that the first step will take much longer than the last two steps.<br>");
