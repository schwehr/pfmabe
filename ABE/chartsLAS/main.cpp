/***************************************************************************\
*                                                                           *
*   Module Name:        chartsLAS                                           *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       February 24, 2006                                   *
*                                                                           *
*   Purpose:            Qt wizard program to split HOF and TOF files by     *
*                       area and output them as ASCII text and LAS format   *
*                       (TOF only) files.                                   *
*                                                                           *
\***************************************************************************/

#include "chartsLAS.hpp"
#include "version.hpp"


int main (int argc, char **argv)
{
    QApplication a (argc, argv);


    QString parm_file = "";

    chartsLAS *cl = new chartsLAS (0);
    cl->setWindowTitle (VERSION);

    a.setStyle (new QPlastiqueStyle);

    return cl->exec ();
}
