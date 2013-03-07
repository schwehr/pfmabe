//
// source:		AttPrefs.cpp
// author:		Gary Morris
//
// This source file contains code dealing with the Preferences dialog box for the Attribute Viewer.
//

// include files

#include "attPrefs.hpp"

//
// construct:	AttPrefs
//
// This constructor will set the window title, bring back settings from the last session, and
// organize the look and feel of the GUI.
//

AttPrefs::AttPrefs (QWidget * parent): QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose) {


    setWindowTitle (tr ("CZMIL Attribute Viewer Preferences"));
	
	RecallSettings (settings);

	AddLayouts ();


}


//
// method:		AddLayouts
//
// This method will create the layout of the Preferences GUI consisting of 3 group boxes with
// various widgets inside.  Default values for histogram intervals, histogram colors, table
// styles and sizes are provided.  All widget signal/slot connections are defined in here as
// well.

void AttPrefs::AddLayouts (void) {

	// set the main layout up

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setMargin (5);
	vbox->setSpacing (5);

	// add histogram interval layout

	QGroupBox * histIntBox = new QGroupBox (tr ("Histogram"), this);
	QVBoxLayout * histIntBoxLayout = new QVBoxLayout;
	histIntBox->setLayout (histIntBoxLayout);

	histIntSpin = new QDoubleSpinBox (histIntBox);
	histIntSpin->setRange (0.01f, 10.0f);
	histIntSpin->setSingleStep (0.1f);
	histIntSpin->setDecimals (2);
	histIntSpin->setValue (settings.histInterval);
	histIntSpin->setToolTip (tr ("Change the histogram interval"));
	histIntBoxLayout->addWidget (histIntSpin);
	connect (histIntSpin, SIGNAL (valueChanged (double)), this, SLOT (slotHistIntervalChanged (double)));


	CreateColorButton( pbHistBarColor					, tr("Bar Color")					, tr("Change Histogram Bar Color") 						, settings.histBarColor);
	CreateColorButton( pbHistBarOutlineColor	, tr("Bar Outline Color")	, tr("Change Histogram Bar Outline Color")		, settings.histBarOutlineColor );
	CreateColorButton( pbHistNullColor				, tr("Null Color")				, tr("Change Histogram Null Indicator Color")	, settings.histNullColor);


	histIntBoxLayout->addWidget(pbHistBarColor);
	histIntBoxLayout->addWidget(pbHistBarOutlineColor);
	histIntBoxLayout->addWidget(pbHistNullColor);	

	/* added a Table Style group box to the right of the previous preferences which */
	/* allows the user to specify the type of table style for the Attribute Viewer. */

	QGroupBox * tableBox = new QGroupBox (tr ("Table Style"), this);
	QVBoxLayout * tableBoxLayout = new QVBoxLayout;
	tableBox->setLayout (tableBoxLayout);

	QButtonGroup * styleGrp = new QButtonGroup (tableBox);
	connect (styleGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotTableStyleClicked (int)));

	listingBtn = new QRadioButton ("Listing of All Entries", tableBox);
	neighborBtn = new QRadioButton ("Spatial Neighbor Table", tableBox);

	styleGrp->addButton (listingBtn, LISTING);
	styleGrp->addButton (neighborBtn, NEIGHBOR);

	tableBoxLayout->addWidget (listingBtn);
	tableBoxLayout->addWidget (neighborBtn);

	QGroupBox * spatialNeighborBox = new QGroupBox (tr ("Spatial Table Size"), this);
	QVBoxLayout * spatialNeighborLayout = new QVBoxLayout;
	spatialNeighborBox->setLayout (spatialNeighborLayout);

	sizeCombo = new QComboBox (this);
	
	sizeCombo->addItem ("3 x 3");
	sizeCombo->addItem ("5 x 5");
	sizeCombo->addItem ("7 x 7");

	sizeCombo->setCurrentIndex (sizeComboSelect);
	
	connect (sizeCombo, SIGNAL (currentIndexChanged (int)), this, SLOT (slotSpatialNeighborSizeChanged (int)));

	spatialNeighborLayout->addWidget (sizeCombo);

	QVBoxLayout * vBox2 = new QVBoxLayout;
	vBox2->addWidget (tableBox);
	vBox2->addWidget (spatialNeighborBox);

	QHBoxLayout * hBox = new QHBoxLayout;
	hBox->addWidget (histIntBox);

	hBox->addLayout (vBox2);
	
	vbox->addLayout (hBox);	

	QHBoxLayout *actions = new QHBoxLayout (0);
	vbox->addLayout (actions);

	QPushButton *closeButton = new QPushButton (tr ("Close"), this);
	closeButton->setToolTip (tr ("Close dialog"));
	
	connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClosePrefs ()));
	actions->addWidget (closeButton);

}


//
// deconstruct:	~AttPrefs
//
// The deconstructor does nothing.

AttPrefs::~AttPrefs ()
{
}


//
// slot:		slotHistIntervalChanged
//
// This slot is called when the histogram interval spinner box is changed.  This value
// determines the interval that each histogram bar represents.  Here we save the value
// in our settings structure for recall.
//

void AttPrefs::slotHistIntervalChanged (double histInterval) {

	settings.histInterval = (NV_FLOAT32) histInterval;
}



//
// slot:			slotTableStyleClicked
//
// This slot will be fired when the user selects one of the table style
// radio buttons within the Preferences dialog box.  The table style is
// recorded.
//

void AttPrefs::slotTableStyleClicked (int style) {

	settings.tStyle = (TableStyle)style;
}


//
// slot:		slotClosePrefs
//
// This slot is tied to the "Close" button and will close the Preferences dialog.
// The close() method will cause the closeEvent event to fire as well.
//

void AttPrefs::slotClosePrefs () {

	close ();
}


//
// event:		closeEvent
//
// This event is automatically called when the Preferences dialog is destroyed.
//

void AttPrefs::closeEvent (QCloseEvent * event __attribute__ ((unused))) {

	SaveSettings (settings);
}


//
// method:		SaveSettings
//
// This method will save all of the values of the preferences .
//

void AttPrefs::SaveSettings (AttPrefSettings settings) {

	QSettings avPreferences ("CHL", "CZMIL Attribute Viewer Prefs");
	avPreferences.beginGroup ("CZMIL Attribute Viewer Prefs");

	avPreferences.setValue (tr("histogram interval"), settings.histInterval);
	avPreferences.setValue (tr ("hist bar color"),					pbHistBarColor->palette().color(QPalette::Normal, QPalette::Button).rgba() );
	avPreferences.setValue (tr ("hist bar outline color"),	pbHistBarOutlineColor->palette().color(QPalette::Normal, QPalette::Button).rgba() );
	avPreferences.setValue (tr ("hist null color"),					pbHistNullColor->palette().color(QPalette::Normal, QPalette::Button).rgba() );

	/* save status of size comboBox */
	avPreferences.setValue (tr ("size combo box index"), sizeCombo->currentIndex());
	
	avPreferences.endGroup ();
}


//
// method:		RecallSettings
//
// This method will recall previously saved settings.
//

void AttPrefs::RecallSettings (AttPrefSettings &settings) {	
	
	QSettings avPreferences ("CHL", "CZMIL Attribute Viewer Prefs");
	avPreferences.beginGroup ("CZMIL Attribute Viewer Prefs");

	settings.histInterval = (NV_FLOAT32) avPreferences.value ("histogram interval", 0.01).toDouble ();
	settings.histBarColor.setRgba        ( avPreferences.value( tr("hist bar color")        , QColor::fromRgbF(  .0f, .0f, 1.0f,  .4f  ).rgba() ).toUInt() );
	settings.histBarOutlineColor.setRgba ( avPreferences.value( tr("hist bar outline color"), QColor::fromRgbF(  .0f, .0f, 1.0f,  .65f ).rgba() ).toUInt() );
	settings.histNullColor.setRgba       ( avPreferences.value( tr("hist null color")       , QColor::fromRgbF( 1.0f, .0f,  .0f, 1.0f  ).rgba() ).toUInt() );

	sizeComboSelect = avPreferences.value (tr("size combo box index"), 1).toInt();
	
	avPreferences.endGroup ();
}


//
// slot:		slotColorButtonClicked
//
// This slot will be fired when either the "Bar Color", "Bar Outline Color", or "Null Color"
// buttons are pressed.  The button that sent the signal is evaluated and the color dialog
// is presented.  The button color and what it represents is changed to the color that was
// selected.
//

void AttPrefs::slotColorButtonClicked () {

	bool ok;
	QPushButton *pb = (QPushButton *)sender();

	QRgb color = QColorDialog::getRgba (pb->palette().color(QPalette::Normal, QPalette::Button).rgba(), &ok, this);

	if (ok) SetColorButtonColor(pb, QColor::fromRgba(color));

}


//
// method:		CreateColorButton
//
// This method will create a color button complete with the caption, tooltip, and underlying
// color.  This is used for the "Bar Color", "Bar Outline Color", and "Null Color" buttons.
//

 void AttPrefs::CreateColorButton(QPushButton *&pb, QString caption, QString toolTip, QColor color) {

	pb = new QPushButton (caption, this);

    pb->setToolTip (toolTip);
    pb->setWhatsThis (toolTip);
	SetColorButtonColor( pb, color);

	connect (pb, SIGNAL (clicked ()), this, SLOT (slotColorButtonClicked ()));
}


//
// method:		SetColorButtonColor
//
// Incoming is the pushbutton and a color that will be applied to the button.  The value()
// method of the color (brightness) determines whether the caption of the button is white
// or black.
//

void AttPrefs::SetColorButtonColor( QPushButton *pb, QColor color) {


	QPalette pal = pb->palette();
	QColor captionColor = (color.value() < 128)? Qt::white : Qt::black;

	pal.setColor (QPalette::Normal, QPalette::ButtonText, captionColor);
	pal.setColor (QPalette::Inactive, QPalette::ButtonText, captionColor);
    pal.setColor (QPalette::Normal, QPalette::Button, color);
    pal.setColor (QPalette::Inactive, QPalette::Button, color);
	
	pb->setPalette(pal);

}


//
// slot:			slotSpatialNeighborSizeChanged
//
// This slot is fired when the combo box is changed which controls the size
// of the spatial neighbor table.  Here, we will simply emit a signal that 
// the parent will handle.
//

void AttPrefs::slotSpatialNeighborSizeChanged (int index) {

	emit spatialNeighborSizeSig (index);
}
