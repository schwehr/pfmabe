/***************************************************************************\
*                                                                           *
*   Module Name:        pfmExtract                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       February 27, 2004                                   *
*                                                                           *
*   Purpose:            Qt wizard program to read a PFM file and produce a  *
*                       output files of the selected data source in ASCII,  *
*                       LLZ, or RDP format.                                 *
*                                                                           *
\***************************************************************************/

#include "pfmExtract.hpp"
#include "version.hpp"


int main (int argc, char **argv)
{
    QApplication a (argc, argv);


    pfmExtract *pe = new pfmExtract (&argc, argv, 0);
    pe->setWindowTitle (VERSION);


    a.setStyle (new QPlastiqueStyle);


    return pe->exec ();
}
