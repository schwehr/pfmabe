//
// source:		AxesDlg.cpp
// author:		Gary Morris
//
// This source file supports the Change Scatterplot Axes dialog that allows a 
// user to specify the attribute of choice for the x/y axes.
//

// include files

#include "AxesDlg.hpp"

//
// construct:		AxesDlg
//
// This constructor initializes the x and y axis indices.  It also takes in all
// of the available attributes as strings and stores them.  Lastly, it builds
// the layout for the dialog.
//

AxesDlg::AxesDlg (QStringList paramList, NV_INT32 xState, NV_INT32 yState, QWidget * parent)
: QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose) {


	setWindowTitle (tr ("Change Scatterplot Axes"));

	this->xState = xState;
	this->yState = yState;
	this->paramList = paramList;

	currentSelection = -1;

	AddLayouts ();
}


//
// method:		AddLayouts
//
// This method builds the layout for the Change Scatterplot Axes dialog.  It 
// consists of a list widget, some push buttons, and read-only text fields.  All
// signal/slot relationships are connected here.
//

void AxesDlg::AddLayouts () {

	parameterList = new QListWidget ();
	connect (parameterList, SIGNAL (itemClicked (QListWidgetItem *)), this, SLOT (slotSelection (QListWidgetItem *)));

	for (int i = 0; i < paramList.count(); i++)
		parameterList->addItem (paramList.at(i));
	
	xAxisBtn = new QPushButton (tr ("X Axis ->"));
	connect (xAxisBtn, SIGNAL (clicked ()), this, SLOT (slotXAxisClicked ()));

	yAxisBtn = new QPushButton (tr ("Y Axis ->"));
	connect (yAxisBtn, SIGNAL (clicked ()), this, SLOT (slotYAxisClicked ()));

	applyBtn = new QPushButton (tr ("Apply"));
	applyBtn->setEnabled (NVFalse);
	connect (applyBtn, SIGNAL (clicked ()), this, SLOT (slotApply ()));

	closeBtn = new QPushButton (tr ("Close"));
	connect (closeBtn, SIGNAL (clicked ()), this, SLOT (slotClose ()));

	xAxisEdit = new QLineEdit (paramList.at(xState));
	yAxisEdit = new QLineEdit (paramList.at(yState));
	xAxisEdit->setEnabled (NVFalse);
	yAxisEdit->setEnabled (NVFalse);

	// set the main layout up

	QHBoxLayout * hBox = new QHBoxLayout (this);
	hBox->addWidget (parameterList);

	QVBoxLayout * vBox = new QVBoxLayout (this);
	QHBoxLayout * hBox2 = new QHBoxLayout (this);
	QHBoxLayout * hBox3 = new QHBoxLayout (this);
	QHBoxLayout * hBox4 = new QHBoxLayout (this);

	hBox2->addWidget (xAxisBtn);
	hBox2->addWidget (xAxisEdit);

	hBox3->addWidget (yAxisBtn);
	hBox3->addWidget (yAxisEdit);

	hBox4->addWidget (applyBtn);
	hBox4->addWidget (closeBtn);

	vBox->addLayout (hBox2);
	vBox->addLayout (hBox3);
	vBox->addSpacing (15);
	vBox->addLayout (hBox4);

	hBox->addLayout (vBox);
}


//
// deconstruct:		~AxesDlg
//
// The deconstructor does nothing.
//

AxesDlg::~AxesDlg ()
{
}


//
// event:		closeEvent
//
// This event is called before the window is destroyed.  Nothing is implemented
// currently.
//

void AxesDlg::closeEvent (QCloseEvent * event __attribute__ ((unused))) {

	//SaveSettings (settings);
}

//
// slot:		slotClose
//
// This slot is called when the Close button is pressed.  
//

void AxesDlg::slotClose () {

	close ();
}

//
// slot:		slotSelection
//
// When an item is selected from the list, this slot is fired.  We look up the
// name of the selection and store the index of the row since it correlates to
// the indices of the attributes.
//

void AxesDlg::slotSelection (QListWidgetItem * item) {	

	currentSelection = parameterList->row (item);
}


// 
// slot:		slotXAxisClicked
//
// This slot will fire when the X Axis-> button is pressed.  The xState
// variable will be set to the index of the attribute that will correspond
// to the X axis.
//

void AxesDlg::slotXAxisClicked () {

	if (currentSelection != -1 && currentSelection != xState) {

		xAxisEdit->setText (paramList.at (currentSelection));
		xState = currentSelection;
		applyBtn->setEnabled  (NVTrue);
	}
}


// 
// slot:		slotYAxisClicked
//
// This slot will fire when the Y Axis-> button is pressed.  The yState
// variable will be set to the index of the attribute that will correspond
// to the Y axis.
//

void AxesDlg::slotYAxisClicked () {

	if (currentSelection != -1 && currentSelection != yState) {

		yAxisEdit->setText (paramList.at (currentSelection));
		yState = currentSelection;
		applyBtn->setEnabled (NVTrue);
	}
}


//
// slot:		slotApply
//
// This slot will fire when the Apply button is pressed.  A user-defined signal
// will be emitted that will be caught by the Attribute Viewer so it can update
// the scatterplot axes definitions.
//

void AxesDlg::slotApply () {
	
	emit sigAxesApply (xState, yState);
	applyBtn->setEnabled (NVFalse);
}
