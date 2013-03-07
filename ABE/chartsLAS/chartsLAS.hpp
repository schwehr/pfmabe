#ifndef CHARTSLAS_H
#define CHARTSLAS_H

#include "chartsLASDef.hpp"
#include "startPage.hpp"
#include "areaInputPage.hpp"
#include "fileInputPage.hpp"
#include "runPage.hpp"
#include "version.hpp"


#define DEG2RAD  (PI / 180.0)


class chartsLAS : public QWizard
{
  Q_OBJECT


public:

  chartsLAS (QWidget *parent = 0);
  ~chartsLAS ();


protected:

  void initializePage (int id);
  void cleanupPage (int id);

  NV_BOOL writeLASHeader (NV_CHAR *file, TOF_HEADER_T tof_header, AREA_DEFINITION *area_def);
  void writeLASRecord (TOPO_OUTPUT_T tof, AREA_DEFINITION *area_def, NV_INT32 k);
  void writeTOFASCRecord (TOPO_OUTPUT_T tof, NV_INT32 k, AREA_DEFINITION *area_def);
  void writeHOFASCRecord (HYDRO_OUTPUT_T hof, AREA_DEFINITION *area_def);

  void envin ();
  void envout ();


  NV_INT32         input_area_count, input_file_count, window_x, window_y, window_width, window_height, datum;

  NV_BOOL          geoid03, ver_dep_flag;

  AREA_DEFINITION  *input_area_def;

  FILE_DEFINITION  *input_file_def;

  RUN_PROGRESS     progress;

  QTextEdit        *inputAreas, *inputFiles;

  QString          inputAreaFilter, inputFileFilter, filePrefix, outputDir;

  QStringList      input_areas, input_files;

  QProcess         *loadProc;

  QLabel           *totalConvertLabel, *fileConvertLabel;


protected slots:

  void slotCustomButtonClicked (int id);
  void slotHelpClicked ();


};

#endif
