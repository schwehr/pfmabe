#ifndef _GSF_FLAGS_H_
#define _GSF_FLAGS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "pfm_nvtypes.h"
#include "pfm.h"
#include "check_flag.h"

void gsf_to_pfm_flags (NV_U_INT32 *pfm_flags, NV_U_BYTE gsf_flags);
void pfm_to_gsf_flags (NV_U_INT32 pfm_flags, NV_U_BYTE *gsf_flags);

#endif
