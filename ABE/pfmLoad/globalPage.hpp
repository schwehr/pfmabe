
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



#ifndef GLOBALPAGE_H
#define GLOBALPAGE_H

#include "pfmLoadDef.hpp"


class globalPage:public QWizardPage
{
  Q_OBJECT 


public:

  globalPage (QWidget *prnt = 0, PFM_GLOBAL *pfm_glb = NULL, FLAGS *fl = NULL, GLOBAL_DIALOG *gd = NULL, NV_BOOL cube_available = NVFalse);

  void setFields (PFM_GLOBAL *pfm_glb, FLAGS *fl);


signals:


protected:

  FLAGS            *flags;

  PFM_GLOBAL       *pfm_global;

  GLOBAL_DIALOG    *global_dialog;

  QWidget          *parent;

  NV_BOOL          srtm1, srtm2, srtm3, srtm30, cube_available;

  QDoubleSpinBox   *horizontal, *vertical, *capture, *horiz, *distance, *minContext, *maxContext;

  QSpinBox         *maxFiles, *maxLines, *maxPings, *maxBeams, *queue;

  QComboBox        *ihoOrder, *std2conf, *disambiguation;

  QCheckBox        *hofOld, *hofNull, *subPath, *invLand, *invSec, *lidAttr, *refData, *fileCheck, *tofRef, *hofFlag, *gsfNominal,  *srtmBest, 
                   *srtmOne, *srtmThree, *srtmThirty, *srtmRef, *srtm2Ex, *timeCheck, *cubeLoad, *cubeAttributes;

  QPalette         gsfPalette, hofPalette, tofPalette, wlfPalette, czmilPalette, bagPalette, hawkeyePalette;

  QColor           widgetBackgroundColor;

  QButtonGroup     *optGrp, *gsfGrp, *hofGrp, *tofGrp, *wlfGrp, *czmilGrp, *bagGrp, *hawkeyeGrp;

  QCheckBox        *gsf_attr[GSF_ATTRIBUTES], *hof_attr[HOF_ATTRIBUTES], *tof_attr[TOF_ATTRIBUTES], *wlf_attr[WLF_ATTRIBUTES], 
    *czmil_attr[CZMIL_ATTRIBUTES], *bag_attr[BAG_ATTRIBUTES], *hawkeye_attr[HAWKEYE_ATTRIBUTES];

  QPushButton      *gsf, *hof, *tof, *wlf, *czmil, *bag, *hawkeye;


  void cubeDialog ();
  void move_and_decrement (NV_INT32 selected);


protected slots:

  void slotHofOldClicked (int state);
  void slotTimeCheckClicked (bool state);
  void slotGSFClicked ();
  void slotGSFAttrClicked (int index);
  void slotClearGSF ();
  void slotCloseGSF ();
  void slotHOFClicked ();
  void slotHOFAttrClicked (int index);
  void slotClearHOF ();
  void slotCloseHOF ();
  void slotTOFClicked ();
  void slotTOFAttrClicked (int index);
  void slotClearTOF ();
  void slotCloseTOF ();
  void slotWLFClicked ();
  void slotWLFAttrClicked (int index);
  void slotClearWLF ();
  void slotCloseWLF ();
  void slotCZMILClicked ();
  void slotCZMILAttrClicked (int index);
  void slotClearCZMIL ();
  void slotCloseCZMIL ();
  void slotBAGClicked ();
  void slotBAGAttrClicked (int index);
  void slotClearBAG ();
  void slotCloseBAG ();
  void slotHAWKEYEClicked ();
  void slotHAWKEYEAttrClicked (int index);
  void slotClearHAWKEYE ();
  void slotCloseHAWKEYE ();
  void slotOptClicked (int id);
  void slotIhoOrderChanged (int index);
  void slotCaptureChanged (double value);
  void slotQueueChanged (int value);
  void slotHorizChanged (double value);
  void slotDistanceChanged (double value);
  void slotMinContextChanged (double value);
  void slotMaxContextChanged (double value);
  void slotStd2confChanged (int index);
  void slotDisambiguationChanged (int index);
  void slotCloseCube ();

private:
};

#endif
