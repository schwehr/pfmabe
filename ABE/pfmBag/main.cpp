/***************************************************************************\
*                                                                           *
*   Module Name:        pfmBag                                              *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       April 29, 2008                                      *
*                                                                           *
*   Purpose:            Qt wizard program to read a PFM file and produce a  *
*                       Bathymetric Attributed Grid (BAG) of the data.      *
*                                                                           *
\***************************************************************************/

#include "pfmBag.hpp"
#include "version.hpp"


int main (int argc, char **argv)
{
  QApplication a (argc, argv);


  pfmBag *pb = new pfmBag (&argc, argv, 0);
  pb->setWindowTitle (VERSION);

  a.setStyle (new QPlastiqueStyle);

  return pb->exec ();
}
