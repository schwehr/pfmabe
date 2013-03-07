//
// header:		UserScaleDlg.hpp
// author:		Gary Morris
//
// This header file supports the Change Min/Max Scales which is a dialog that allows
// the user to set their own min/max scales that will be used for profiling or scatterplot.
// The profile Y axis, scatterplot X axis, and scatterplot Y axis are all independent.
//

#ifndef _USER_SCALE_DLG_H_
#define _USER_SCALE_DLG_H_

// include files

#include <QtCore>
#include <QtGui>

#include "nvutility.h"
#include "nvtypes.h"

//
// class:		UserScaleDlg
//
// This class represents the Change Min/Max Scales dialog.  It will emit a signal
// when the Apply button is pressed and it will send back all of the ranges for
// the profile axis and both scatterplot axes.
//

class UserScaleDlg : public QDialog {

  Q_OBJECT 

    public:

	UserScaleDlg (NV_FLOAT32 min, NV_FLOAT32 max, 
					NV_FLOAT32 min2, NV_FLOAT32 max2,
					NV_FLOAT32 min3, NV_FLOAT32 max3,QWidget *parent = 0);
	~UserScaleDlg ();

	signals:

		void sigUserScaleDefined (NV_FLOAT32 min, NV_FLOAT32 max,
									NV_FLOAT32 min2, NV_FLOAT32 max2,
									NV_FLOAT32 min3, NV_FLOAT32 max3);

	private:	

		QDoubleSpinBox * minSpin, * maxSpin;
		QDoubleSpinBox * minSpin2, * maxSpin2;
		QDoubleSpinBox * minSpin3, * maxSpin3;
		QPushButton * applyBtn, * closeBtn;	
		NV_FLOAT32 savedMinVal, savedMaxVal;
		NV_FLOAT32 savedScatXMinVal, savedScatXMaxVal;
		NV_FLOAT32 savedScatYMinVal, savedScatYMaxVal;
		
	protected:

		void closeEvent (QCloseEvent * event);
		void AddLayouts (NV_FLOAT32 savedMinVal, NV_FLOAT32 savedMaxVal,
								NV_FLOAT32 savedScatXMinVal, NV_FLOAT32 savedScatXMaxVal,
								NV_FLOAT32 savedScatYMinVal, NV_FLOAT32 savedScatYMaxVal);
		NV_BOOL errorCheck ();
	
	protected slots:

		void slotClose ();		
		void slotApply ();
};

#endif


