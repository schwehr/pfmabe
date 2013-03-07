#ifndef MOSAIC_FUNCS_H
#define MOSAIC_FUNCS_H

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <unisips_io.h>
#include <mosaic_io.h>

#define PATH_ABSOLUTE   'A'
#define PATH_RELATIVE   'R'
#define PATH_STRUCTURED 'S'

#define GRID_INDEPENDENT 'I'
#define GRID_STRUCTURED  'S'

void  swap_mosaic_hdr(MOSAIC_HEADER *mheader);
void  read_mosaic_hdr (FILE *, MOSAIC_HEADER *, int);
void  write_mosaic_hdr(FILE *, MOSAIC_HEADER,   int);
void  print_mosaic_hdr(MOSAIC_HEADER);
int   find_contrib_file(int, CONTRIB_LIST *, char *);
int   add_to_contrib_list(int, char *, CONTRIB_LIST **);
short read_contrib_list(FILE *, MOSAIC_HEADER, CONTRIB_LIST **);
void  write_contrib_list(FILE *, CONTRIB_LIST *);
void  print_contrib_list(CONTRIB_LIST *);
void  free_contrib_list(CONTRIB_LIST **);

#ifdef  __cplusplus
}
#endif

#endif
