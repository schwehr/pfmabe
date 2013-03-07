#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "nvtypes.h"
#include "hawkeye.h"


/* gcc -O -ansi -Wall -D_LARGFILE64_SOURCE hawkeye_waveform_test.c -lhawkeye -lm -o tst  */


/*  .sfm file (waveform index)

    header is CSSSFMHD or CSSSFMTD followed by 4 byte number of records
    in file followed by 4 bytes (version major), followed by 4 bytes (version minor).
    Then records:
    8 byte Timestamp = matches .dat record timestamp
    4 byte FilePosIntermediate = offset to ???
    4 byte FilePosOriginal = offset to beginning of record in .dat (waveform) file
    4 byte FileIndex = file number (FL004_2871181_Flightline_1_HD.sfm has records for HD_FL004_2871181_Flightline_1_000.dat and
                       HD_FL004_2871181_Flightline_1_001.dat, if the index is 1 the record is in the second file)



    .dat file (waveforms)

    header is 256 bytes long, ASCII, line feed delimited, space filled

    #DATE 20100410
    #TIME 134503
    #GPSTIME 567899.546
    #DATAVERSION 00103


    Each record starts with the CommonHeader (note the packing - single byte aligned) :


#ifdef WIN32
#pragma pack(push, 1)
#endif
typedef struct
{
	Uint16				msgId;
	Uint16				msgSize;// nbr Bytes in the msg (header and data)
	Uint32				senderSeqMsgId;				
	}CommonHeaderType
#ifdef __GNUC__
__attribute__ ((packed, aligned(1))); 
#elif defined WIN32
;
#pragma pack(pop)
#else
#error "Struct packing not implemented for this compiler"
#endif;



    Then the ShotHydroData (which includes the ChannelInfoType data) for each channel:



enum MsgIdType {
    msgId_shotDataHydro					= 20010, 
    msgId_measurementData				= 20020, 
    msgId_navigationData				= 20030, 
    msgId_storeGenericLog				= 20040, 
    msgId_initGenericLog				= 20050, 
    msgId_locationMission				= 20060, 
    msgId_locationFlightline			= 20070, 
    msgId_flightlineNotSet				= 20072, 
    msgId_topoShotData					= 20110, 
    msgId_acknowledge					= 20120, 
    msgId_sensorSystemCommand			= 20130,
    msgId_sensorSystemStatus			= 20140,
    msgId_sensorSystemSettingCommand	= 20150,
    msgId_scannerData					= 20161,
	msgId_scannerData_0					= 20160,
    msgId_shotDataTopo					= 20170,
    msgId_health						= 20180,
    msgId_topoEchoStatistic				= 20190,
	msgId_timeSync						= 20200,
	msgId_initGenericLogMulti			= 20210,
	msgId_storageReport					= 20220,
	msgId_picUEye						= 20230	
};

#define MAX_TOTAL_WAVEFORMS_BYTES		10000


// HYDRO CHANNEL ID
enum HydroChannelIDType {
    hydroChannelID_GrnShlw1, 
    hydroChannelID_GrnDeep1, 
    hydroChannelID_GrnShlw2, 
    hydroChannelID_GrnDeep2, 
    hydroChannelID_GrnShlw3, 
    hydroChannelID_GrnDeep3, 
    hydroChannelID_GrnShlw4, 
    hydroChannelID_GrnDeep4, 
    hydroChannelID_IR1, 
    hydroChannelID_IR2, 
    hydroChannelID_GainMonitorShlw, 
    hydroChannelID_GainMonitorDeep, 
    hydroChannelID_nrOfChannels
};

typedef struct
{
        Uint8 			dataValid; 
	Uint8 			waveformFormat;
	Uint8 			nbrBytesInOneSample; 
	Uint8			padding;
	Uint32 			pulseLength;     // Nr of bytes
	Uint32 			reflexLength;	 // Nr of bytes
  Uint32 			intermissionLength;
} ChannelInfoType

typedef struct
{
        Float64 				pcTimestamp;
	Float64 				timestamp;
	Float64					scannerTimestamp;
	Uint32					seqShotID;
	Float32					scannerXaxisAngle;
	Float32					scannerYaxisAngle;
	ChannelInfoType			        channelInfo[hydroChannelID_nrOfChannels];
	Uint8					waveforms[MAX_TOTAL_WAVEFORMS_BYTES];		
} ShotDataHydroType


    We do not read all MAX_TOTAL_WAVEFORMS_BYTES of waveforms data.  We use the CommonHeaderType.msgSize minus the size of everything that 
    precedes the waveforms (including the CommonHeader) to compute the size of the waveforms.

    Take the ChannelInfoType.pulseLength plus the ChannelInfoType.reflexLength (returnLength in Hawkeye code) to compute waveform length.

    Waveforms are in the order specified in the HYDRO CHANNEL ID above.

*/

NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_INT32                    hawkeye_handle, rec, i, j, k, m, wave_size, total_bytes, count, numrecs;
  NV_CHAR                     bin_file[512];
  HAWKEYE_RECORD              hawkeye;
  HAWKEYE_META_HEADER         *meta_header;
  HAWKEYE_CONTENTS_HEADER     *contents_header;
  HAWKEYE_SFM_RECORD          hawkeye_sfm_record;
  HAWKEYE_SHOT_DATA_HYDRO     hawkeye_shot_data_hydro;


  if (argc < 3)
    {
      fprintf (stderr, "\nUsage: %s HAWKEYE_BIN_FILE RECORD_NUMBER\n\n", argv[0]);
      exit (-1);
    }


  strcpy (bin_file, argv[1]);


  sscanf (argv[2], "%d", &rec);


  numrecs = 1;
  if (argc > 3) sscanf (argv[3], "%d", &numrecs);


  if ((hawkeye_handle = hawkeye_open_file (bin_file, &meta_header, &contents_header, HAWKEYE_READONLY)) >= 0)
    {
      fprintf (stderr, "\n\nNumber of point data fields = %d\n\n",contents_header->NbrOfPointDataFields);

      count = 0;
      for (i = 0 ; i < contents_header->NbrOfPointDataFields ; i++) fprintf (stderr, "%02d  -  %s\n", i, contents_header->PointDataFieldDescr[i]);

      fprintf (stderr, "\n");

      count = 0;
      total_bytes = 0;
      for (i = 0 ; i < contents_header->NbrOfPointDataFields ; i++)
        {
          fprintf (stderr, "%02d,%02d  -  %s\n", i, contents_header->PointDataFieldTypes[i], contents_header->PointDataFieldTags[i]);

          switch (contents_header->PointDataFieldTypes[i])
            {
            case HawkeyeCSSOutCHAR:
            case HawkeyeCSSOutBOOL:
              total_bytes++;
              break;

            case HawkeyeCSSOutINT16:
            case HawkeyeCSSOutUINT16:
              total_bytes += 2;
              break;

            case HawkeyeCSSOutFLOAT32:
            case HawkeyeCSSOutINT32:
            case HawkeyeCSSOutUINT32:
              total_bytes += 4;
              break;

            case HawkeyeCSSOutFLOAT64:
              total_bytes += 8;
              break;

            case HawkeyeCSSOutNbrOfValidTypes:
              break;
            }
        }

      fprintf (stderr, "\n\nOffset to point records = %x\n", contents_header->OffsetToPointData);
      fprintf (stderr, "\nPoint record size from header = %d\n", contents_header->PointRecordSize);
      fprintf (stderr, "\nPoint record size from field tags = %d\n", total_bytes);
      fprintf (stderr, "\nNumber of point records = %d\n\n\n", contents_header->NbrOfPointRecords);


      for (m = rec ; m < rec + numrecs ; m++)
        {
          if (!hawkeye_read_record (hawkeye_handle, m, &hawkeye))
            {
              hawkeye_dump_record (hawkeye_handle, hawkeye);

              if (contents_header->data_type == HAWKEYE_HYDRO_BIN_DATA)
                {
                  if (hawkeye_read_sfm_record (hawkeye_handle, m, &hawkeye_sfm_record))
                    {
                      hawkeye_perror ();
                      exit (-1);
                    }

                  if (hawkeye_read_shot_data_hydro (hawkeye_handle, m, &hawkeye_shot_data_hydro))
                    {
                      hawkeye_perror ();
                      exit (-1);
                    }

                  /*
                  for (i = 0 ; i < hydroChannelID_nrOfChannels ; i++)
                    {
                      if (hawkeye_shot_data_hydro.channelInfo[i].dataValid)
                        {
                          if (hawkeye_shot_data_hydro.channelInfo[i].pulseLength)
                            {
                              wave_size = hawkeye_shot_data_hydro.channelInfo[i].pulseLength;

                              k = 0;
                              for (j = 0 ; j < wave_size ; j++)
                                {
                                  if (!(k % 20)) fprintf (stderr, "\n[%02d/%03d] ", i, j);
                                  k++;

                                  fprintf (stderr, "%03d ", hawkeye_shot_data_hydro.wave_pulse[i][j]);
                                }

                              fprintf (stderr, "\n\n");
                            }

                          if (hawkeye_shot_data_hydro.channelInfo[i].reflexLength)
                            {
                              wave_size = hawkeye_shot_data_hydro.channelInfo[i].reflexLength;

                              k = 0;
                              for (j = 0 ; j < wave_size ; j++)
                                {
                                  if (!(k % 20)) fprintf (stderr, "\n[%02d/%03d] ", i, j);
                                  k++;

                                  fprintf (stderr, "%03d ", hawkeye_shot_data_hydro.wave_reflex[i][j]);
                                }

                              fprintf (stderr, "\n\n");
                            }
                        }
                    }
                  */
                }
            }
        }
      hawkeye_close_file (hawkeye_handle);
    }
  else
    {
      hawkeye_perror ();
    }

  return (0);
}



/*  A different type of test.  */

/*
NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_INT32 handle, i, j, count, total_bytes;
  HAWKEYE_META_HEADER *MetaHeader;
  HAWKEYE_CONTENTS_HEADER *contents_header;
  HAWKEYE_RECORD hawkeye_record;


  handle = hawkeye_open_file (argv[1], &MetaHeader, &contents_header, HAWKEYE_READONLY);
  if (handle < 0)
    {
      hawkeye_perror ();
      exit (-1);
    }


  printf ("\n\nOffset to point records = %x\n", contents_header->OffsetToPointData);

  printf ("\nPoint record size from header = %d\n", contents_header->PointRecordSize);

  printf ("\nPoint record size from field tags = %d\n", total_bytes);

  printf ("\nNumber of point records = %d\n", contents_header->NbrOfPointRecords);

  printf ("\n\nMinimum values\n\n");


  hawkeye_dump_record (handle, contents_header->MinValues);

  printf ("\n\nMaximum values\n\n");

  hawkeye_dump_record (handle, contents_header->MaxValues);


  printf ("\n\n");

  for (i = 0 ; i < 10 ; i += 2)
    {
      if (!hawkeye_read_record (handle, i, &hawkeye_record))
        {
          printf ("Hawkeye record # %d\n\n", i);
          hawkeye_dump_record (handle, hawkeye_record);
          printf ("\n\n");
        }
    }


  hawkeye_close_file (handle);

  return (0);
}
*/
