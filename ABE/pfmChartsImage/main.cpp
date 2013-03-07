/***************************************************************************\
*                                                                           *
*   Module Name:        pfmChartsImage                                      *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October, 26, 2004                                   *
*                                                                           *
*   Ported to Qt4:      July 3, 2007                                        *
*                                                                           *
*   Purpose:            Qt wizard program to extract image files and plane  *
*                       attitude/position for use in building photo mosaics *
*                       in either ERDAS or Ray Seyfarth's mosaic program.   *
*                       A Digital Elevation Model (DEM) is also created     *
*                       from the input PFM file.                            *
*                                                                           *
\***************************************************************************/

#include "pfmChartsImage.hpp"
#include "version.hpp"


int main (int argc, char **argv)
{
    QApplication a (argc, argv);


    pfmChartsImage *pci = new pfmChartsImage (&argc, argv, 0);
    pci->setWindowTitle (VERSION);

    a.setStyle (new QPlastiqueStyle);

    return pci->exec ();
}
