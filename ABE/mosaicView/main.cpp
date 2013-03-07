#include "mosaicView.hpp"


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
  QApplication a (argc, argv);


  mosaicView *mv = new mosaicView (argc, argv);

  a.setStyle (new QPlastiqueStyle);

  mv->show ();

  mv->redrawMap ();

  return a.exec ();
}
