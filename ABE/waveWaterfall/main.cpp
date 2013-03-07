#include "waveWaterfall.hpp"
#include <qapplication.h>


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
    QApplication a (argc, argv);

    waveWaterfall *ww = new waveWaterfall (&argc, argv);

    a.setStyle (new QPlastiqueStyle);

    ww->show ();

    return a.exec ();
}
