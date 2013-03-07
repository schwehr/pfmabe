#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/trackLineWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("trackLine is a tool for building an ASCII decimated trackline file from GSF, POS/SBET, "
                                  "WLF, or HAWKEYE data.  The resulting file is viewable in areaCheck or as an overlay in "
                                  "pfmView.  Help is available by clicking on the Help button and then clicking on the item "
                                  "for which you want help.  Click <b>Next</b> to continue or <b>Cancel</b> to exit."));

  label->setWordWrap(true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);



  QGroupBox *oBox = new QGroupBox (tr ("trackline output file"), this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);
  oBoxLayout->setSpacing (10);

  outputFile = new QLineEdit (this);
  outputFile->setWhatsThis (outputFileText);
  oBoxLayout->addWidget (outputFile, 10);

  QPushButton *output_file_browse = new QPushButton (tr ("Browse..."), this);
  output_file_browse->setWhatsThis (output_fileBrowseText);
  connect (output_file_browse, SIGNAL (clicked ()), this, SLOT (slotOutputFileBrowse ()));
  oBoxLayout->addWidget (output_file_browse, 1);


  vbox->addWidget (oBox);


  registerField ("outputFile*", outputFile);
}



void 
startPage::slotOutputFileBrowse ()
{
  QFileDialog fd (this, tr ("trackLine Output file"));
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, options->output_dir);


  QStringList filters;
  filters << tr ("trackLine file (*.trk)");


  fd.setFilters (filters);
  fd.setFileMode (QFileDialog::AnyFile);


  QStringList files;


  if (fd.exec () == QDialog::Accepted) 
    {
      files = fd.selectedFiles ();

      QString file = files.at (0);

      if (!file.isEmpty ())
        {
          outputFile->setText (file);
        }
    }

  options->output_dir = fd.directory ().absolutePath ();
}
