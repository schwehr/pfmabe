//
// source:		AxesDlg.hpp
// author:		Gary Morris
//
// This header file supports the Change Scatterplot Axes dialog that allows a 
// user to specify the attribute of choice for the x/y axes.
//

#ifndef _AXES_DLG_H_
#define _AXES_DLG_H_

// include files

#include <QtCore>
#include <QtGui>

#include "nvutility.h"

//
// class:		AxesDlg
//
// This class represents the Change Scatterplot Axes dialog.  It will contain a listing
// of available attributes that may serve as the X or Y axis.  The user may choose any
// attribute and mark it to the X/Y axis.
//

class AxesDlg : public QDialog {

  Q_OBJECT 


    public:

	AxesDlg (QStringList paramList, NV_INT32 xState, NV_INT32 yState, QWidget *parent = 0);
	~AxesDlg ();

	signals:

		void sigAxesApply (NV_INT32 xState, NV_INT32 yState);

	private:	

		QListWidget * parameterList;
		QPushButton * xAxisBtn, * yAxisBtn, * applyBtn, * closeBtn;
		QLineEdit *	xAxisEdit, * yAxisEdit;

		NV_INT32 xState, yState;
		QStringList paramList;
		NV_INT32 currentSelection;

	protected:

		void closeEvent (QCloseEvent * event);
		void AddLayouts ();
	
	protected slots:

		void slotClose ();
		void slotSelection (QListWidgetItem * item);
		void slotXAxisClicked ();
		void slotYAxisClicked ();
		void slotApply ();
};

#endif
