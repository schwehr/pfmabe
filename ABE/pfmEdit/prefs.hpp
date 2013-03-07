
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



#ifndef PREFS_H
#define PREFS_H

#include "pfmEditDef.hpp"


class Prefs:public QDialog
{
  Q_OBJECT 


public:

  Prefs (QWidget *parent = 0, OPTIONS *op = NULL, MISC *mi = NULL);
  ~Prefs ();


signals:

  void dataChangedSignal ();
  void hotKeyChangedSignal (NV_INT32 i);


protected:

  ABE_SHARE       mod_share;

  OPTIONS         *options, mod_options;

  MISC            *misc;

  QTabWidget      *prefTab;

  QButtonGroup    *bGrp, *cGrp;

  QDialog         *contoursD, *geotiffD, *hotKeyD, *ancillaryProgramD;

  QCheckBox       *kill;

  nvMap           *geotiff;

  QImage          layer1, layer2;

  QScrollBar      *geotiffScroll;

  NV_INT32        geotiff_w, geotiff_h;

  NV_BOOL         dataChanged;

  QTableWidget    *contoursTable, *hotKeyTable, *ancillaryProgramTable;

  QPushButton     *bWaveColor[MAX_STACK_POINTS], *bContourColor, *bTrackerColor, *bBackgroundColor, *bRefColor, *bFeatureColor,
                  *bFeatureInfoColor, *bFeaturePolyColor, *bHighFeatureColor, *bVerFeatureColor, *bRestoreDefaults, *bAncillaryPrograms;

  QPalette        bWavePalette[MAX_STACK_POINTS], bContourPalette, bTrackerPalette, bBackgroundPalette, bRefPalette, bFeaturePalette,
                  bFeatureInfoPalette, bFeaturePolyPalette, bHighFeaturePalette, bVerFeaturePalette;

  QPushButton     *bContourLevels, *bGeotiff, *bHotKeys;

  QLineEdit       *contourInt, *program[NUMPROGS], *hotkey[NUMPROGS], *actionkey[NUMPROGS];

  QDoubleSpinBox  *offset, *minZ, *rot, *filterSTD, *featureRadius;

  QSpinBox        *slice, *overlap, *undo, *contourSm, *pointSize;

  QComboBox       *ihoOrder, *iconSize, *ZFactor, *Width;

  QCheckBox       *dFilter;

  NV_INT32        current_command;

  QLabel          *geotiff_left;


  void setFields ();


protected slots:

  void slotAncillaryPrograms ();
  void slotApplyAncillaryChanges (const QString &text);
  void slotCloseAncillaryProgram ();
  void slotContourColor ();
  void slotTrackerColor ();
  void slotBackgroundColor ();
  void slotRefColor ();
  void slotFeatureColor ();
  void slotFeatureInfoColor ();
  void slotFeaturePolyColor ();
  void slotHighFeatureColor ();
  void slotVerFeatureColor ();
  void slotColorClicked (int id);
  void slotHelp ();
  void slotRestoreDefaults ();
  void slotPositionClicked (int id);
  void slotContourLevelsClicked ();
  void slotGeotiff ();
  void slotPreRedraw (NVMAP_DEF mapdef);
  void slotGeotiffScrollValueChanged (int value);
  void slotGeotiffScrollSliderMoved (int value);
  void slotCloseGeotiff ();
  void slotHotKeys ();
  void slotMinZValueChanged (double value);
  void slotEgmClicked ();
  void slotCloseHotKeys ();
  void slotCloseContours ();
  void slotApplyPrefs ();
  void slotClosePrefs ();


private:

};

#endif
