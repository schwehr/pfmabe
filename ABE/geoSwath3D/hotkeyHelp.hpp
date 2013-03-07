#ifndef HOTKEYHELP_H
#define HOTKEYHELP_H

#include "geoSwath3DDef.hpp"


class hotkeyHelp:public QDialog
{
  Q_OBJECT 


public:

  hotkeyHelp (QWidget *parent = 0, OPTIONS *op = NULL, MISC *mi = NULL);
  ~hotkeyHelp ();


signals:


protected:

  OPTIONS         *options;

  MISC            *misc;

  QTabWidget      *hotkeyTabWidget;



protected slots:

  void slotClose ();


private:
};

#endif
