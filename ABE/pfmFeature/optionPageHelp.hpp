QString orderText = 
  optionPage::tr ("Select the IHO order that will be used to make the feature selections.  IHO special order is "
                  "based on a one meter cube object in less than 40 meters of water and 5 percent of depth over "
                  "40 meters.  IHO order 1 is based on a two meter cube object in less than 40 meters of water "
                  "and 10 percent of depth over 40 meters.");

QString zeroText = 
  optionPage::tr ("Set this if you want to select objects above the zero level of the data.  Note that if you "
                  "haven't corrected the data to the final datum this could be meaningless.");

QString hpcText = 
  optionPage::tr ("Set this if you want to compute the confidence value of selected features using the "
                  "<b>Hockey Puck of Confidence (TM)</b>.  This will set the confidence level of the feature based on the "
                  "number of points within a hockey puck shaped region around the feature point whose dimensions are twice the horizontal "
                  "uncertainty of the feature point in X/Y and twice the vertical uncertainty of the feature point in Z.  The criteria "
                  "is that there must be at least 2 other points in the HPC area and the points must be from more than one "
                  "line to set the confidence to 5 (verified).  Otherwise, the confidence level is set to 3.<br><br>"
                  "<b>IMPORTANT NOTE: If you do not have multiple line coverage in an area this is just a waste of computation time.</b>");

QString descriptionText = 
  optionPage::tr ("This is the default description used for all of the selected features.  You may leave this "
                  "blank, choose one of the standard descriptions using the buttons, or set a description manually.  "
                  "<b>All</b> of the features will have this description but they can be individually modified in "
                  "pfmView or pfmEdit(3D).");

QString typeBoxText = 
  optionPage::tr ("Select an item then click on OK to use the selected item for the feature description.");

QString offsetText = 
  optionPage::tr ("Set an offset value for the input depth values.  This is useful if you have not applied a final datum shift to your "
                  "data but want to use pfmFeature to create a feature file to help in editing the data.<br><br>"
                  "<b>IMPORTANT NOTE: The offset value is <i>added</i> to the Z value.</b>");

QString egmText = 
  optionPage::tr ("Press this button to retrieve the EGM08 model value of the ellipsoid to geoid (approximate mean sea "
                  "level) datum offset for the point nearest the center of the minimum bounding rectangle of the displayed "
                  "area.  This will be placed in the <b>Offset</b> field to be used to shift the input depth values.");

