#include "waveformMonitor.hpp"
#include <qapplication.h>


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
    QApplication a (argc, argv);

    waveformMonitor *wm = new waveformMonitor (&argc, argv);

    a.setStyle (new QPlastiqueStyle);

    wm->show ();

    return a.exec ();
}
