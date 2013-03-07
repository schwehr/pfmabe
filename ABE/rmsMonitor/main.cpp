#include "rmsMonitor.hpp"
#include <qapplication.h>


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
    QApplication a (argc, argv);

    rmsMonitor *rm = new rmsMonitor (&argc, argv);

    a.setStyle (new QPlastiqueStyle);

    rm->show ();

    return a.exec ();
}
