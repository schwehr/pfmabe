#ifndef PREFS_H
#define PREFS_H

#include "geoSwath3DDef.hpp"


class Prefs:public QDialog
{
  Q_OBJECT 


    public:

  Prefs (QWidget *parent = 0, OPTIONS *op = NULL, MISC *mi = NULL, POINT_DATA *da = NULL);
  ~Prefs ();


 signals:

  void dataChangedSignal ();
  void hotKeyChangedSignal (NV_INT32 i);


 protected:

  ABE_SHARE       mod_share;

  POINT_DATA      *data;

  OPTIONS         *options, mod_options;

  MISC            *misc;

  QButtonGroup    *bGrp, *cGrp;

  QDialog         *hotKeyD, *ancillaryProgramD;

  QCheckBox       *kill, *scale;

  QImage          layer1, layer2;

  NV_BOOL         dataChanged;

  QTableWidget    *hotKeyTable, *ancillaryProgramTable;

  QPushButton     *bWaveColor[MAX_STACK_POINTS], *bTrackerColor, *bBackgroundColor, *bScaleColor, *bRestoreDefaults, *bAncillaryPrograms;

  QPalette        bWavePalette[MAX_STACK_POINTS], bTrackerPalette, bBackgroundPalette, bScalePalette;

  QPushButton     *bHotKeys;

  QLineEdit       *program[NUMPROGS], *hotkey[NUMPROGS], *actionkey[NUMPROGS];

  QDoubleSpinBox  *offset, *minZ, *rot, *fSize, *exag, *filterSTD;

  QSpinBox        *undo, *pointSize, *pointLimit, *zoomPercent;

  QComboBox       *ihoOrder, *iconSize, *ZFactor, *Width;

  QCheckBox       *dFilter;

  NV_INT32        current_command;


  void setFields ();


  protected slots:

  void slotAncillaryPrograms ();
  void slotApplyAncillaryChanges (const QString &text);
  void slotCloseAncillaryProgram ();
  void slotTrackerColor ();
  void slotBackgroundColor ();
  void slotScaleColor ();
  void slotColorClicked (int id);
  void slotHelp ();
  void slotRestoreDefaults ();
  void slotPositionClicked (int id);
  void slotHotKeys ();
  void slotMinZValueChanged (double value);
  void slotEgmClicked ();
  void slotCloseHotKeys ();
  void slotApplyPrefs ();
  void slotClosePrefs ();

 private:
};

#endif
