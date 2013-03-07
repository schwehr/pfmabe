
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



QString defChartScaleText = 
  definePolygon::tr ("Set the chart scale that will be used to determine feature circle radius for use in defining "
                     "feature polygons.  The value will be the denominator of the scale fraction with the numerator "
                     "always set to 1.  That is, if you enter 5000 then the scale will be 1:5000.  Feature polygons "
                     "may be defined by using the <b>Define Feature Polygon</b> entry in the <b>Utilities</b> menu.");

QString grpFeaturesText = 
  definePolygon::tr ("Check this box to automatically make sub-features of all features that fall within the defined "
                     "polygon.  The master feature, the one associated with the polygon, will always be the shoalest "
                     "feature inside the polygon.");

QString defPolygonText = 
  definePolygon::tr ("Push this button to define a polygonal area for a group of features.  When selected the "
                     "define feature polygon dialog will be hidden and the cursor will become the pencil cursor.  "
                     "Left clicking on a location will cause that point to be the first polygon vertex.  Moving the cursor will "
                     "cause a line to appear.  Select the next vertex by left clicking on a location again.  "
                     "Select as many points as needed.  To select the last point double click on its location "
                     "or right click to bring up the popup menu and select <b>Accept polygon</b>.  You can "
                     "discard the polygon at any time by either clicking the center mouse button or selecting "
                     "<b>Discard polygon</b> from the right-click popup menu.  After either accepting or "
                     "discarding the polygon the feature dialog will reappear.");

QString cancelDefinePolygonText = 
  definePolygon::tr ("Click this button to cancel the polygon definition Define Feature Polygon dialog.");

QString applyDefinePolygonText = 
  definePolygon::tr ("Click this button to accept the polygon and group the sub-features under the shoalest feature.  "
                     "The dilaog will be closed upon completion of grouping.");
