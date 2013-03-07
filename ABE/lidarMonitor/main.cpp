#include "lidarMonitor.hpp"
#include <qapplication.h>


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
    QApplication a (argc, argv);

    lidarMonitor *lm = new lidarMonitor (&argc, argv);

    a.setStyle (new QPlastiqueStyle);

    lm->show ();

    return a.exec ();
}
