/*  areaCheck  */

/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/



#include "areaCheck.hpp"


int
main (int argc, char **argv)
{
    QApplication a (argc, argv);

    areaCheck *ac = new areaCheck (&argc, argv);

    ac->setWindowTitle (VERSION);

    a.setStyle (new QPlastiqueStyle);

    ac->show ();

    ac->initializeMaps ();

    return a.exec ();
}
