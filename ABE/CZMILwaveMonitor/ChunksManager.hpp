#ifndef _CHUNKS_MANAGER_H_
#define _CHUNKS_MANAGER_H_

#include "nvtypes.h"
#include <QtCore>
#include <QtGui>

#define MAX_FLT_LINES_SUPPORTED		5

struct Chunk {

	QString name;
	NV_INT32 numChunks;
	NV_INT64 * maxRecord;
	NV_INT64 * minRecord;
};

class ChunkManager {

public:

	ChunkManager ();
	~ChunkManager ();

	void SetMaxFltLinesSupported (NV_INT32 toSupport);
	Chunk * RegisterFlightline (QString name);

private:

	NV_INT32	maxFltlinesSupported;
	NV_INT32	fltInUse;
	Chunk		chunks[MAX_FLT_LINES_SUPPORTED];

	NV_INT32 FindFlightLineIndex (QString name);
	Chunk * FillChunk (QString name);
	NV_BOOL ProcessWFFile (QString fileName, Chunk * chunk);
	void AddFlightlineChunk (Chunk * chunk);
};

#endif
