#ifndef _CHECK_FLAG_H_
#define _CHECK_FLAG_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include "hmpsflag.h"


  unsigned char check_flag (unsigned char field, unsigned char flag);
  void set_flag (unsigned char *field, unsigned char flag);
  void clear_flag (unsigned char *field, unsigned char flag);


#ifdef  __cplusplus
}
#endif

#endif
