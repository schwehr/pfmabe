#include "hawkeyeMonitor.hpp"
#include <qapplication.h>


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
    QApplication a (argc, argv);

    hawkeyeMonitor *wm = new hawkeyeMonitor (&argc, argv);

    a.setStyle (new QPlastiqueStyle);

    wm->show ();

    return a.exec ();
}
