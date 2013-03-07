
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



#ifndef BEAMRECS_H
#define BEAMRECS_H


#include "examGSFDef.hpp"


#define NUM_ARRAYS 25
#define NUM_ROWS NUM_ARRAYS + 1


class beamRecs:public QDialog
{
  Q_OBJECT 


public:

  beamRecs (QWidget *parent = 0, NV_INT32 gsfHandle = 0, NV_INT32 num_ping = 1, OPTIONS *op = NULL, MISC *mi = NULL);
  ~beamRecs ();


signals:


protected:

  NV_INT32        gsf_handle, ping, sensor_type, beams, start, end;

  OPTIONS         *options;

  MISC            *misc;

  gsfRecords      gsf_record;

  gsfDataID       gsf_data_id;

  nvMap           *data_profile;

  NVMAP_DEF       mapdef;

  NV_BOOL         data_flag[NUM_ARRAYS];

  NV_F64_COORD2   *data;

  BOUNDS          data_bounds;

  QPushButton     *profileButton[62];

  QStatusBar      *dataBar;

  QLabel          *dataLabel, *beamLabel;

  QTableWidget    *beamTable;

  QHeaderView     *vheader;

  QButtonGroup    *profileGrp;

  QDialog         *profileD;

  QString         rowLabel[27];


  void populateTable ();
  void closeEvent (QCloseEvent *e);


protected slots:

  void slotCloseBeamRecs ();
  void slotHelp ();
  void slotProfile (int logicalIndex);
  void slotDataProfileMouseMove (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y);
  void slotPlotProfile (NVMAP_DEF l_mapdef);
  void slotProfileFinished (int result);
  void slotCloseProfile ();
  void slotResize (QResizeEvent *e);


private:
};

#endif
