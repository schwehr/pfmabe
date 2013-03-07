/***************************************************************************\
*                                                                           *
*   Module Name:        trackLine                                           *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       April 19, 2011                                      *
*                                                                           *
*   Purpose:            Qt wizard program to generate trackline files from  *
*                       GSF, POS/SBET, HAWKEYE, and WLF data.               *
*                                                                           *
\***************************************************************************/

#include "trackLine.hpp"
#include "version.hpp"
#include <qapplication.h>


int main (int argc, char **argv)
{
    QApplication a (argc, argv);


    trackLine *cg = new trackLine (0);

    cg->setWindowTitle (VERSION);

    a.setStyle (new QPlastiqueStyle);

    return cg->exec ();
}
