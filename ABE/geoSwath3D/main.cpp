/*  geoSwath3D  */

/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#include "geoSwath3D.hpp"


int
main (int argc, char **argv)
{
  QApplication a (argc, argv);


  a.setStyle (new QPlastiqueStyle);

  geoSwath3D *gs;

  gs = new geoSwath3D (&argc, argv);

  return a.exec ();
}
