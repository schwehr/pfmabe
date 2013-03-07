//
// source:		UserScaleDlg.cpp
// author:		Gary Morris
//
// This source file supports the Change Min/Max Scales which is a dialog that allows
// the user to set their own min/max scales that will be used for profiling or scatterplot.
// The profile Y axis, scatterplot X axis, and scatterplot Y axis are all independent.
//

// include files

#include "UserScaleDlg.hpp"

//
// construct:		UserScaleDlg
//
// This constructor inherits QT's dialog class.  It will set all of the min/max values
// for profiling and scatterplot by the incoming arguments.  Lastly, it will set up the
// layouts to build the dialog.
//

UserScaleDlg::UserScaleDlg (NV_FLOAT32 min, NV_FLOAT32 max, 
							NV_FLOAT32 min2, NV_FLOAT32 max2,
							NV_FLOAT32 min3, NV_FLOAT32 max3, QWidget * parent)
: QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose) {

	setWindowTitle (tr ("Change Min/Max Scales"));
	
	savedMinVal = min;
	savedMaxVal = max;
	savedScatXMinVal = min2;
	savedScatXMaxVal = max2;
	savedScatYMinVal = min3;
	savedScatYMaxVal = max3;

	AddLayouts (savedMinVal, savedMaxVal, savedScatXMinVal,
				savedScatXMaxVal, savedScatYMinVal, savedScatYMaxVal);
}

//
// method:		AddLayouts
//
// The layout consists of 2 buttons, 6 spinner boxes, and multiple labels.
//

void UserScaleDlg::AddLayouts (NV_FLOAT32 savedMinVal, NV_FLOAT32 savedMaxVal,
								NV_FLOAT32 savedScatXMinVal, NV_FLOAT32 savedScatXMaxVal,
								NV_FLOAT32 savedScatYMinVal, NV_FLOAT32 savedScatYMaxVal) {		
		
	applyBtn = new QPushButton (tr ("Apply"));
	connect (applyBtn, SIGNAL (clicked ()), this, SLOT (slotApply ()));

	closeBtn = new QPushButton (tr ("Close"));
	connect (closeBtn, SIGNAL (clicked ()), this, SLOT (slotClose ()));

	// set the main layout up

	QVBoxLayout * vBox = new QVBoxLayout (this);
	QHBoxLayout * hBox1 = new QHBoxLayout;
	QHBoxLayout * hBox2 = new QHBoxLayout (this);
	QHBoxLayout * hBox3 = new QHBoxLayout (this);
	QHBoxLayout * hBox4 = new QHBoxLayout (this);
	
	QGroupBox * grpBox1 = new QGroupBox (tr ("Profile"), this);
	grpBox1->setLayout (hBox1);
	
	minSpin = new QDoubleSpinBox (grpBox1);
	minSpin->setSingleStep (1.0f);
	minSpin->setRange (-9999.0, 9999.0);
	
	maxSpin = new QDoubleSpinBox (grpBox1);
	maxSpin->setSingleStep (1.0f);
	maxSpin->setRange (-9999.0, 9999.0);
	
	// first time ever running the application, user-defined values 
	// for min and max will be -500 to 500
	
	minSpin->setValue(savedMinVal);
	maxSpin->setValue(savedMaxVal);
	
	hBox1->addWidget (new QLabel ("Min:", grpBox1));
	hBox1->addWidget (minSpin);
	hBox1->addWidget (new QLabel (" to Max: ", grpBox1));
	hBox1->addWidget (maxSpin);	
	
	QGroupBox * grpBox2 = new QGroupBox (tr ("Scatter Plot X Axis"), this);
	grpBox2->setLayout (hBox2);
	
	minSpin2 = new QDoubleSpinBox (grpBox2);
	minSpin2->setSingleStep (1.0f);
	minSpin2->setRange (-9999.0, 9999.0);
	
	maxSpin2 = new QDoubleSpinBox (grpBox2);
	maxSpin2->setSingleStep (1.0f);
	maxSpin2->setRange (-9999.0, 9999.0);
	
	minSpin2->setValue(savedScatXMinVal);
	maxSpin2->setValue(savedScatXMaxVal);
	
	hBox2->addWidget (new QLabel ("Min:", grpBox2));
	hBox2->addWidget (minSpin2);
	hBox2->addWidget (new QLabel (" to Max: ", grpBox2));
	hBox2->addWidget (maxSpin2);
	
	QGroupBox * grpBox3 = new QGroupBox (tr ("Scatter Plot Y Axis"), this);
	grpBox3->setLayout (hBox3);
	
	minSpin3 = new QDoubleSpinBox (grpBox3);
	minSpin3->setSingleStep (1.0f);
	minSpin3->setRange (-9999.0, 9999.0);
	
	maxSpin3 = new QDoubleSpinBox (grpBox3);
	maxSpin3->setSingleStep (1.0f);
	maxSpin3->setRange (-9999.0, 9999.0);
	
	minSpin3->setValue(savedScatYMinVal);
	maxSpin3->setValue(savedScatYMaxVal);
	
	hBox3->addWidget (new QLabel ("Min:", grpBox3));
	hBox3->addWidget (minSpin3);
	hBox3->addWidget (new QLabel (" to Max: ", grpBox3));
	hBox3->addWidget (maxSpin3);

	hBox4->addWidget (applyBtn);
	hBox4->addWidget (closeBtn);

	vBox->addWidget (grpBox1);
	vBox->addWidget (grpBox2);
	vBox->addWidget (grpBox3);
	vBox->addLayout (hBox4);
	
	setLayout (vBox);
}


//
// method:			errorCheck
//
// This method will check to see if the the minimum is less than the maximum.
//

NV_BOOL UserScaleDlg::errorCheck () {

	NV_BOOL pass = NVTrue;
	
	if (minSpin->value() >= maxSpin->value() ||
		minSpin2->value() >= maxSpin2->value() ||
		minSpin3->value() >= maxSpin3->value()) {
		QMessageBox::information (this, "Invalid Range", "The min/max values are invalid.");
		pass = NVFalse;
	}
	
	return pass;
}
	

//
// deconstruct:		~UserScaleDlg
//
// The deconstructor does nothing.
//

UserScaleDlg::~UserScaleDlg ()
{
}


//
// event:		closeEvent
//
// This event fires when the close() method is called or the window is destroyed.
// Currently, it does nothing.
//

void UserScaleDlg::closeEvent (QCloseEvent * event __attribute__ ((unused))) {

	//SaveSettings (settings);
}


//
// slot:		slotClose
//
// This slot is fired when the Close button is pressed.
//

void UserScaleDlg::slotClose () {

	close ();
}


//
// slot:		slotApply
//
// This slot is fired when the Apply button is pressed.  If no errors exist, a user-defined
// signal is emitted that will be caught by the Attribute Viewer so it can update it's 
// graph properties for the user-defined scale range.
//

void UserScaleDlg::slotApply () {

	if (errorCheck ()) {	
		emit sigUserScaleDefined ((NV_FLOAT32)(minSpin->value()), 
									(NV_FLOAT32)(maxSpin->value()),
									(NV_FLOAT32)(minSpin2->value()),
									(NV_FLOAT32)(maxSpin2->value()),
									(NV_FLOAT32)(minSpin3->value()),
									(NV_FLOAT32)(maxSpin3->value()));
	}
	else {
	
		minSpin->setValue (savedMinVal);
		maxSpin->setValue (savedMaxVal);
		
		minSpin2->setValue (savedScatXMinVal);
		maxSpin2->setValue (savedScatXMaxVal);
		
		minSpin3->setValue (savedScatYMinVal);
		maxSpin3->setValue (savedScatYMaxVal);
	}
}

