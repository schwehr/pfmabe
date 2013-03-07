/***************************************************************************\
*                                                                           *
*   Module Name:        pfmFeature                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       February 27, 2004                                   *
*                                                                           *
*   Purpose:            Qt wizard program to read a PFM file and produce a  *
*                       feature file for the PFM based on IHO special order *
*                       or order 1.                                         *
*                                                                           *
\***************************************************************************/

#include "pfmFeature.hpp"
#include "version.hpp"


int main (int argc, char **argv)
{
    QApplication a (argc, argv);


    pfmFeature *pf = new pfmFeature (&argc, argv, 0);
    pf->setWindowTitle (VERSION);

    a.setStyle (new QPlastiqueStyle);

    return pf->exec ();
}
