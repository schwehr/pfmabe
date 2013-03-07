#ifndef WAVEFORMS_H
#define WAVEFORMS_H
/*****************************************************************************
 * waveforms.h 	Header
 *
 * Purpose:  	This header file hold some structures that are used in the
 *		waveform file processing parts of the system 
 *		Processing.
 *              
 *
 * Revision History:
 * 93		DR	Created
 *
 * 98/02/05 DR	-400 Hz cleanup.
 *		-removed "LINKS" stuff...
 *		-change so .ra and .ch are no longer used.  They are just 
 *		 leftovers from the databse which needed blobs to be chars..
 *		-changed name from WF_FILE_ST to WAVEFORMS_T and saved
 *		 to "waveforms.h" instead of "file.h"
 ***************************************************************************/
 
#ifdef  __cplusplus
extern "C" {
#endif


#include "cnsts.h"

struct WAVEFORMS
{
	int	time_stamp;

     	short   pmt_raw_waveform_ra[PMT_LEN+1];
     	short   apd_raw_waveform_ra[APD_LEN+1];
     	short   ir_raw_waveform_ra[IR_LEN+1];
     	short   raman_raw_waveform_ra[RAMAN_LEN+1];
};
typedef struct WAVEFORMS WAVEFORMS_T;


typedef struct
{
    unsigned short  size;
    int             start_bits[4];
    int             ref_bits[4];
    int             offset_bits[4];
    int             delta_bits[4];
    short           start[4];
    short           reference[4];
    short           offset[4];
    short           pmt[PMT_LEN];
    short           apd[APD_LEN];
    short           ir[IR_LEN];
    short           raman[RAMAN_LEN];
} PACK_DATA;


FILE *file_open_wave( char *db_name, int flightline);
int	file_read_wave( FILE *wave_file, WAVEFORMS_T *wave_p);
int	file_write_wave( FILE *wave_file, WAVEFORMS_T *wave_p);
int file_get_wave( char *db_name, short flightline, int timestamp, WAVEFORMS_T *wave_p);
FILE *file_open_wavc( char *db_name, int flightline, int *compression_flag, int *numrecs);
int file_read_wavc (FILE *wave_file, WAVEFORMS_T *wave_p, int record);
int pack_wave (WAVEFORMS_T wave, unsigned char *data);
void unpack_wave (WAVEFORMS_T *wave, unsigned char *data);


#ifdef  __cplusplus
}
#endif


#endif /*waveforms_h*/
