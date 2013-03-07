#include "chartsPic.hpp"


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
  QApplication a (argc, argv);


  chartsPic *cp = new chartsPic (&argc, argv);

  a.setStyle (new QPlastiqueStyle);

  cp->show ();

  cp->redrawPic ();

  return a.exec ();
}
