#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <mosaic_funcs.h>
#include <mosaic_io.h>
#include <bytes2func.h>

short num_contrib_files;

void swap_mosaic_hdr(MOSAIC_HEADER *mheader)
{
    mheader->head_size        = bytes2short((unsigned char *)&(mheader->head_size),        1);
    mheader->file_size        = bytes2int((unsigned char *)&(mheader->file_size),          1);
    mheader->s_latitude       = bytes2float((unsigned char *)&(mheader->s_latitude),       1);
    mheader->w_longitude      = bytes2float((unsigned char *)&(mheader->w_longitude),      1);
    mheader->n_latitude       = bytes2float((unsigned char *)&(mheader->n_latitude),       1);
    mheader->e_longitude      = bytes2float((unsigned char *)&(mheader->e_longitude),      1);
    mheader->central_meridian = bytes2float((unsigned char *)&(mheader->central_meridian), 1);
    mheader->grid_size        = bytes2short((unsigned char *)&(mheader->grid_size),        1);
    mheader->x_range          = bytes2float((unsigned char *)&(mheader->x_range),          1);
    mheader->y_range          = bytes2float((unsigned char *)&(mheader->y_range),          1);
    mheader->rows             = bytes2int((unsigned char *)&(mheader->rows),               1);
    mheader->columns          = bytes2int((unsigned char *)&(mheader->columns),            1);
 /* mheader->projection is an unsigned char and doesn't need swapping      */
    mheader->num_files        = bytes2short((unsigned char *)&(mheader->num_files),        1);
    mheader->file_location    = bytes2int((unsigned char *)&(mheader->file_location),      1);
 /* mheader->comment is a char array and doesn't need swapping             */
}

void read_mosaic_hdr(FILE *inf, MOSAIC_HEADER *mheader, int swap)
{
    int fpos;
    
    /*fpos = ftell(inf);*/
    fseek(inf, 0, SEEK_SET);
    
    fread(mheader, 1, sizeof(MOSAIC_HEADER), inf);
    
    if(swap)
      swap_mosaic_hdr(mheader);

    /*fseek(inf, fpos, SEEK_SET);*/
}

void write_mosaic_hdr(FILE *outf, MOSAIC_HEADER mheader, int swap)
{
    int fpos;
    
    /*fpos = ftell(outf);*/
    fseek(outf, 0, SEEK_SET);
    
    if(swap)
      swap_mosaic_hdr(&mheader);

    fwrite(&mheader, 1, sizeof(MOSAIC_HEADER), outf);

    /*fseek(outf, fpos, SEEK_SET);*/
}

void print_mosaic_hdr(MOSAIC_HEADER mheader)
{
    int i;

    printf("Header size      = %d bytes\n", mheader.head_size);
    printf("Data Size        = %d bytes\n", (mheader.file_size - mheader.head_size));
    printf("South Latitude   = %11.6f\n", mheader.s_latitude);
    printf("West Longitude   = %11.6f\n", mheader.w_longitude);
    printf("North Latitude   = %11.6f\n", mheader.n_latitude);
    printf("East Longitude   = %11.6f\n", mheader.e_longitude);
    printf("Central Meridian = %11.6f\n", mheader.central_meridian);
    printf("Grid Cell Size   = %04.2f meters\n", (mheader.grid_size / 100.0));
    printf("X Range          = %11.6f meters\n", mheader.x_range);
    printf("Y Range          = %11.6f meters\n", mheader.y_range);
    printf("Rows             = %d\n", mheader.rows);
    printf("Columns          = %d\n", mheader.columns);
    printf("Projection       = %d\n", mheader.projection);
    printf("Number of Files  = %d\n", mheader.num_files);
    printf("File Location    = %d\n", mheader.file_location);
    if (mheader.comment[0] != 0)
    {
        for (i=0;i<MOSAIC_COMMENT_SIZE;i++)
        {
            if (isprint(mheader.comment[i]))
            {
                printf("%c", mheader.comment[i]);
            }
            else
            {
                printf(" %02X ", mheader.comment[i]);
            }
        }
        printf("\n");
    }
}

int find_contrib_file(int path, CONTRIB_LIST *list, char *adding_file)
{
    CONTRIB_LIST *cur;
    int          value = 0, done = 0, i = 0;
    char         namestr[MIO_STRLEN];
    
    namestr[0] = '\0';

    if (path == PATH_RELATIVE)
    {
        i = 0;
        if ((adding_file[0] != '/') && (adding_file[0] != '.'))
        {
            strcpy(namestr, "./");
        }
    }
    else if (path == PATH_ABSOLUTE)
    {
        i = 0;
    }
    else if (adding_file[0] != '/')
    {
        strcpy(namestr, "../../");
        i = 0;
        while ((adding_file[i] == '.') || (adding_file[i] == '/'))
        {
            i++;
        }
    }
    else
    {
        i = 0;
    }
    strcat(namestr, &(adding_file[i]));

/*printf("searching list for %s\n", namestr);*/

    cur = list;
    while ((cur != (CONTRIB_LIST *)NULL) && (!done))
    {
        if (!strcmp(cur->contrib_file,namestr))
        {
            value = 1;
            done  = 1;
        }
        else
        {
            cur = cur->next;
        }
    }

    return(value);
}

int add_to_contrib_list(int path, char *file_name, CONTRIB_LIST **list)
{
    CONTRIB_LIST *cur = (CONTRIB_LIST *)(NULL);
    int value, i;
    char outstr[MIO_STRLEN];

    value = 0;

    outstr[0] = '\0';
    if (path == PATH_RELATIVE)
    {
        /*printf("Maintaining the exact path name.\n");*/
        i = 0;
        if ((file_name[0] != '/') && (file_name[0] != '.'))
        {
            strcpy(outstr, "./");
        }
    }
    else if (path == PATH_STRUCTURED)
    {
        strcpy(outstr, "../../");
        i = 0;
        while ((file_name[i] == '.') || (file_name[i] == '/'))
        {
            i++;
        }
    }
    else /* if (path == PATH_ABSOLUTE) */
    {
        /*printf("Warning: Absolute path name!!\n");*/
        i = 0;
    }

    strcat(outstr, &(file_name[i]));

    if (!find_contrib_file(path, *list, file_name))
    {
        if (*list == (CONTRIB_LIST *)(NULL))
        {
            /* printf("Contrib list is null\n"); */
            *list = (CONTRIB_LIST *)malloc(sizeof(CONTRIB_LIST));
            cur = *list;
            cur->next = (CONTRIB_LIST *)NULL;
            strcpy(cur->contrib_file, outstr);
        }
        else
        {
            cur = *list;
            while (cur->next != (CONTRIB_LIST *)(NULL))
            {
                cur = cur->next;
            }
            cur->next = (CONTRIB_LIST *)malloc(sizeof(CONTRIB_LIST));
            cur = cur->next;
            cur->next = (CONTRIB_LIST *)NULL;
            strcpy(cur->contrib_file, outstr);
        }
        value = 1;
        /* printf("Added %s to contrib file list; %X\n", cur->contrib_file, cur); */
    }
    return (value);
}

short read_contrib_list(FILE *inf, MOSAIC_HEADER header, CONTRIB_LIST **list)
{
    char  instr[MIO_STRLEN];
    short i, done, num_read_in;
    int   fpos;
    
    fpos = ftell(inf);

    if (header.file_location == (sizeof(MOSAIC_HEADER)+(header.rows * header.columns)))
    {
        fseek(inf, header.file_location, 0);
        done = 0;
        num_read_in = 0;
        while ((!done) && (!feof(inf)))
        {
            for(i=0;i<MIO_STRLEN;i++)
            {
                instr[i] = '\0';
            }
            i = 0;
            fread(&(instr[i]), 1, 1, inf);
            while((instr[i] != ',') && (!feof(inf)))
            {
                instr[i+1] = '\0';
                i++;
                fread(&(instr[i]), 1, 1, inf);
            }
            if (instr[i] == ',')
            {
                instr[i] = '\0';
            }
            if (instr[0] != '\0')
            {
                num_read_in++;
                if (add_to_contrib_list(PATH_ABSOLUTE, instr, list))
                {
                    /* added file to list */
                    num_contrib_files++;
                }
            }
            if ((num_read_in == header.num_files) || (feof(inf)))
            {
                done = 1;
            }
        }
        /* printf(" Added %hd files\n", num_contrib_files); */
    }
    fseek(inf, fpos, SEEK_SET);
    return (num_read_in);
}

void write_contrib_list(FILE *outf, CONTRIB_LIST *list)
{
    CONTRIB_LIST *cur;
    char outstr[MIO_STRLEN+1];
    short i=0;
    
    cur = list;
    while (cur != (CONTRIB_LIST *)NULL)
    {
        strcpy(outstr, cur->contrib_file);
        strcat(outstr, ",");
        /* printf("%s\n", outstr); */
        fwrite(outstr,1,strlen(outstr),outf);
        cur = cur->next;
    }
}

void print_contrib_list(CONTRIB_LIST *list)
{
    CONTRIB_LIST *cur;
    
    cur = list;
    while (cur != (CONTRIB_LIST *)NULL)
    {
        printf("%s\n", cur->contrib_file);
        cur = cur->next;
    }
}

void free_contrib_list(CONTRIB_LIST **list)
{
    CONTRIB_LIST *cur, *next;

    if (*list == (CONTRIB_LIST *)NULL)
    /* nothing to free */
    {
        return;
    }

    cur = *list;
    next = cur->next;

    while(cur != (CONTRIB_LIST *)NULL)
    {
        free(cur);
        cur = next;
        if (cur != (CONTRIB_LIST *)NULL)
        {
            next = cur->next;
        }
    }
    *list = (CONTRIB_LIST *)NULL;
}
