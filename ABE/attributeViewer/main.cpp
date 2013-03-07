#include "attributeViewer.hpp"
#include <qapplication.h>


NV_INT32
main (NV_INT32 argc, NV_CHAR **argv)
{
DPRINT
	QApplication a (argc, argv);

	attributeViewer attributeViewer (&argc, argv);

	a.setStyle (new QPlastiqueStyle);

	attributeViewer.show ();

	return a.exec ();

}
