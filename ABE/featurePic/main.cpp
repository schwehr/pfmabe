#include "featurePic.hpp"
#include <qapplication.h>


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
  QApplication a (argc, argv);


  featurePic *fp = new featurePic (&argc, argv);

  a.setStyle (new QPlastiqueStyle);

  fp->show ();

  fp->redrawPic ();

  return a.exec ();
}
