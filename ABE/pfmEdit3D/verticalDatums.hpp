
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



#include "pfmEdit3D.hpp"


//!  Abbreviated vertical datum names.

QString shortVerticalDatum[15] = {
  pfmEdit3D::tr ("UNK"),
  pfmEdit3D::tr ("UNK"),
  pfmEdit3D::tr ("MLLW"),
  pfmEdit3D::tr ("MLW"),
  pfmEdit3D::tr ("ALAT"),
  pfmEdit3D::tr ("ESLW"),
  pfmEdit3D::tr ("ISLW"),
  pfmEdit3D::tr ("LAT"),
  pfmEdit3D::tr ("LLW"),
  pfmEdit3D::tr ("LNLW"),
  pfmEdit3D::tr ("LWD"),
  pfmEdit3D::tr ("MLHW"),
  pfmEdit3D::tr ("MLLWS"),
  pfmEdit3D::tr ("MLWN"),
  pfmEdit3D::tr ("MSL")
};


//!  Full vertical datum names.

QString verticalDatum[15] = {
  pfmEdit3D::tr ("Unknown vertical datum"),
  pfmEdit3D::tr ("Unknown vertical datum"),
  pfmEdit3D::tr ("Mean lower low water"),
  pfmEdit3D::tr ("Mean low water"),
  pfmEdit3D::tr ("Aprox Lowest Astronomical Tide"),
  pfmEdit3D::tr ("Equatorial Springs Low Water"),
  pfmEdit3D::tr ("Indian Springs Low Water"),
  pfmEdit3D::tr ("Lowest Astronomical Tide"),
  pfmEdit3D::tr ("Lowest Low Water"),
  pfmEdit3D::tr ("Lowest Normal Low Water"),
  pfmEdit3D::tr ("Low Water Datum"),
  pfmEdit3D::tr ("Mean Lower High Water"),
  pfmEdit3D::tr ("Mean Lower Low Water Springs"),
  pfmEdit3D::tr ("Mean Low Water Neap"),
  pfmEdit3D::tr ("Mean Sea Level")
};
