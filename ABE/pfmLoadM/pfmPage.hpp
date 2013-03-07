
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef PFMPAGE_H
#define PFMPAGE_H

#include "pfmLoadMDef.hpp"


class pfmPage:public QWizardPage
{
  Q_OBJECT 


public:

  pfmPage (QWidget *parent = 0, PFM_DEFINITION *pfmDef = NULL, PFM_GLOBAL *pfmg = NULL, NV_INT32 page_num = 0);

  void setFields (PFM_DEFINITION *pfmDef);

  int nextId () const;


signals:


protected:

  bool validatePage ();

  PFM_GLOBAL      *pfm_global;

  QLabel          *pfm_file_label, *area_label, *mosaic_label, *feature_label;

  QLineEdit       *pfm_file_edit, *area_edit, *mosaic_edit, *feature_edit, *north, *south, *east, *west;

  QDialog         *nsewD;

  QPushButton     *pfm_file_browse, *area_browse, *area_map, *area_pfm, *area_nsew, *mosaic_browse, *feature_browse;

  QCheckBox       *applyFilter, *deepFilter;

  QDoubleSpinBox  *mBinSize, *gBinSize, *minDepth, *maxDepth, *stdSpin, *featureRadius;

  QComboBox       *precision;

  PFM_DEFINITION  *pfm_def;

  QString         temp_area_name, pfmIndex, pfm_file_edit_field, area_edit_field, mBinSizeField, gBinSizeField,
                  minDepthField, maxDepthField, precisionField, mosaic_edit_field, feature_edit_field, applyFilterField,
                  deepFilterField, stdSpinField, featureRadiusField;

  QProcess        *mapProc;

  NV_INT32        l_page_num;

  NV_FLOAT64      prev_gbin, prev_mbin;


protected slots:

  void slotPFMFileEdit (const QString &string);
  void slotPFMFileBrowse ();
  void slotAreaFileBrowse ();
  void slotMosaicFileBrowse ();
  void slotFeatureFileBrowse ();
  void slotApplyFilterStateChanged (int state);
  void slotMBinSizeChanged (double value);
  void slotGBinSizeChanged (double value);
  void slotMapReadyReadStandardError ();
  void slotMapReadyReadStandardOutput ();
  void slotMapError (QProcess::ProcessError error);
  void slotMapDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotAreaMap ();
  void slotAreaPFM ();
  void slotAreaNSEW ();
  void slotHelp ();
  void slotApplyNSEW ();
  void slotCloseNSEW ();

private:
};

#endif
