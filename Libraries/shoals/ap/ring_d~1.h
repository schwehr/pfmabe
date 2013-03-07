/************************************************************
 ** Define the maximum number of position buffers and the  **
 ** size of each buffer.  Same stuff for the GPS ring.     **
 ************************************************************/
#ifndef RING_DEFS_H
#define RING_DEFS_H

#define MAX_RING_BUF_NUM (400*60+2)
#define RING_BUF_SIZE	sizeof(RING_BUF_T)

#define MAX_GPS_BUF_NUM	2
#define GPS_BUF_SIZE	sizeof(GPS_T)

#endif