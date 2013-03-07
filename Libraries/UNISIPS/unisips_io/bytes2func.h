#ifndef BYTES2FUNC_H
#define BYTES2FUNC_H

#ifdef  __cplusplus
extern "C" {
#endif

#define ENDIAN_BIG     1
#define ENDIAN_LITTLE  0

long   bytes2long   (unsigned char *, int);
int    bytes2int    (unsigned char *, int);
short  bytes2short  (unsigned char *, int);
float  bytes2float  (unsigned char *, int);
double bytes2double (unsigned char *, int);
int    endian();

#ifdef  __cplusplus
}
#endif

#endif
