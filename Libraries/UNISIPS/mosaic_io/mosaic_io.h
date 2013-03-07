#ifndef MOSAIC_IO_H
#define MOSAIC_IO_H

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * Header file for defining the file header structure 
 * of MOSAIC formatted files.
 */

#define MIO_STRLEN          256
#define MOSAIC_COMMENT_SIZE 460

typedef struct
{ 
    short          head_size;         /* bytes (512)            */
    char           pad1[2];
    int            file_size;         /* bytes including header */
    float          s_latitude;        /* decimal degrees +-  90 */
    float          w_longitude;       /* decimal degrees +- 180 */
    float          n_latitude;
    float          e_longitude;
    float          central_meridian;
    short          grid_size;         /* centimeters            */
    char           pad2[2];
    float          x_range;           /* meters (total size)    */
    float          y_range;
    int            rows;
    int            columns;
    unsigned char  projection;        /* 1 = UTM  2 = Mercator  */
    unsigned char  classification;    /* 0=unk, 1=uncl, 2=conf, 3=sec, 4=ts */
    short          num_files;
    int            file_location;
    char           comment[MOSAIC_COMMENT_SIZE];
}MOSAIC_HEADER;

typedef struct contrib_list
{
    char contrib_file[MIO_STRLEN];
    struct contrib_list *next;
} CONTRIB_LIST;

#ifdef  __cplusplus
}
#endif

#endif
