/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/



/*****************************************************************************\

                           IMPORTANT NOTE

    The double_bit_pack and double_bit_unpack routines have been shown
    to have inconsistencies in some rare cases.  Unfortunately, I built the
    SRTM files using these so we have to keep them around.  Due to the fact
    that hardly any of the files exceed 2GB I doubt that this will cause
    any problems.  If it did it would be in the area of the north pole as that 
    is the last data that is loaded in to each file.  So far I have not seen
    any problems.  JCD

\*****************************************************************************/



#ifndef _SRTM_BIT_PACK_H_
#define _SRTM_BIT_PACK_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include "nvtypes.h"


  void srtm_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT32 value) ;
  NV_U_INT32 srtm_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits);
  void srtm_double_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT64 value);
  NV_INT64 srtm_double_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits);


#ifdef  __cplusplus
}
#endif

#endif
