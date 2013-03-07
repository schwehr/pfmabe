
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



QString lengthText = 
  editFeature::tr ("Set the length of the feature in meters.  Changes to this will not be placed into the "
                   "feature file until the <b>Apply</b> button is pressed.");

QString widthText = 
  editFeature::tr ("Set the width of the feature in meters.  Changes to this will not be placed into the "
                   "feature file until the <b>Apply</b> button is pressed.");

QString orientationText = 
  editFeature::tr ("Set the orientation of the feature in degrees.  Changes to this will not be placed into the "
                   "feature file until the <b>Apply</b> button is pressed.");

QString polygonText = 
  editFeature::tr ("Push this button to define a polygonal area for this feature.  When selected the feature "
                   "dialog will be hidden and the cursor will become the arrow cursor.  Left clicking on a "
                   "location will cause that point to be the first polygon vertex.  Moving the cursor will "
                   "cause a line to appear.  Select the next vertex by left clicking on a location again.  "
                   "Select as many points as needed.  To select the last point double click on its location "
                   "or right click to bring up the popup menu and select <b>Accept polygon</b>.  You can "
                   "discard the polygon at any time by either clicking the center mouse button or selecting "
                   "<b>Discard polygon</b> from the right-click popup menu.  After either accepting or "
                   "discarding the polygon the feature dialog will reappear.  If you have accepted the polygon "
                   "the length and orientation will be recomputed using the long axis of the polygonal area.  "
                   "If you have discarded the polygon these fields will remain unchanged.");

QString cFeatureText = 
  editFeature::tr ("Push this button to consolidate all of the features that fall inside of the defined polygonal "
                   "area into a single feature.  When this button is clicked a dialog will appear that will allow "
                   "you to select the current feature or the shoalest feature in the polygonal area.  Note that "
                   "you will not visually see the consolidation until you select the <b>Apply</b> button.  Also, "
                   "even if all of the feature polygon is not displayed on the screen, features within the polygon "
                   "will be consolidated into a single feature.  The unselected features within the polygon will be "
                   "set to invalid so you can view them by viewing any of the invalid surfaces.");

QString confidenceText = 
  editFeature::tr ("Set the confidence value for this feature.  The value goes from 0 to 5.  A confidence value "
                   "of 0 indicates that the feature is invalid.  As a general rule, unverified features are set "
                   "to a confidence value of 3.  Once they have been verified they may be set to 4.  Changes to "
                   "this will not be placed into the feature file until the <b>Apply</b> button is pressed.");

QString descriptionText = 
  editFeature::tr ("This is the description field for the feature.  Normally this value is set from the IHO Chart 1 "
                   "descriptions from the <b>Feature type</b> buttons.  If you need to elaborate on the description use "
                   "the <b>Remarks</b> field.  Changes to this field will not be placed into the feature file until the "
                   "<b>Apply</b> button is pressed.");

QString typeBoxText = 
  editFeature::tr ("Select an item then click on OK to use the selected item for the feature description.");

QString remarksText = 
  editFeature::tr ("This is the remarks field for the feature.  This is where we usually put a brief note about "
                   "what we believe the object to be.  Changes to this will not be placed into the feature file "
                   "until the <b>Apply</b> button is pressed.");

QString viewText = 
  editFeature::tr ("This button will bring up a picture of the associated snippet (sidescan or photo) if "
                   "available.");

QString assignText = 
  editFeature::tr ("This button will allow you to set or change the associated snippet (sidescan or photo).  "
                   "When assigning a snippet (sidescan or photo) to a feature the general rule is to use the "
                   "feature file name concatenated with the record number with an extension of .jpg (or other "
                   "type of image extension).  Changes to this will not be placed into the feature file until "
                   "the <b>Apply</b> button is pressed.");

QString cancelEditFeatureText = 
  editFeature::tr ("Click this button to cancel changes to the current feature and close the Edit Feature dialog.  If"
                   "this was a new feature it will not be added to the feature file.");

QString applyEditFeatureText = 
  editFeature::tr ("Click this button to apply changes that have been made to the feature fields to the feature "
                   "file and close the Edit Feature dialog.");

QString screenText = 
  editFeature::tr ("Click this button to grab a copy of the current screen to be used as the associated image for the "
                   "current feature.  There will be a delay prior to grabbing the screen.  The delay time is set in the "
                   "screenshot dialog (default 2 seconds).  If you do not like the screenshot you may rearrange the "
                   "windows and grab a new screenshot by pressing the <b>New Screenshot</b> button in the screenshot "
                   "dialog.  If you do not like the screenshot you may discard it by pressing the <b>Quit</b> button "
                   "in the screenshot dialog.");
