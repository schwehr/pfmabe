
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifndef NVWIN3X
#include <unistd.h>
#endif

#ifdef NVWIN3X
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#endif

#include "huge_io.h"

#ifdef NVWIN3X     /* Windows Operating System */

#define HUGE_SLASH '\\'
#define HUGE_SLASH_STR "\\"

#else            /* Unix operating system */

#define HUGE_SLASH '/'
#define HUGE_SLASH_STR "/"

#endif

#define LAZY_HFSEEK
/*#undef LAZY_HFSEEK*/


#define NUM_HUGE_FILES          384
#define NUM_SUB_FILES           128
#define FILE_NAME_SIZE          256

static NV_CHAR                  file_context[NUM_HUGE_FILES];
static FILE                     *cfg_file_pnt[NUM_HUGE_FILES];
static FILE                     *file_pnt[NUM_HUGE_FILES][NUM_SUB_FILES];
static NV_INT32                 num_files[NUM_HUGE_FILES];
static NV_INT32                 cur_file[NUM_HUGE_FILES];
static NV_CHAR                  file_mode[NUM_HUGE_FILES][10];
static NV_CHAR                  file_path[NUM_HUGE_FILES][FILE_NAME_SIZE];
static NV_CHAR                  directory_path[NUM_HUGE_FILES][FILE_NAME_SIZE];
static NV_INT32                 file_handle[NUM_HUGE_FILES] = {
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1};
static NV_INT64                 cur_pos[NUM_HUGE_FILES];
static NV_INT64                 eof_huge[NUM_HUGE_FILES];




/***************************************************************************/
/*!

  - Module Name:        open_sub_file

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Opens a sub-file for the specified huge file handle.

  - Arguments:
                        - path            =   See fopen
                        - mode            =   See fopen
                        - handle          =   huge file handle
                        - file            =   sub-file number
                        - op              =   operation (0 - read, 1 - write)

  - Return Value:
                        - Possible error status :
                            - 0           =   success
                            - -1          =   failure, check errno for reason

  - Caveats:            This function is only used internal to the library.

****************************************************************************/

static NV_INT32 open_sub_file (NV_CHAR *path, NV_CHAR *mode, NV_INT32 handle, NV_INT32 file, NV_INT32 op)
{
    NV_CHAR           filename[FILE_NAME_SIZE];
    static NV_INT32   max_bin_file_num = -1;
    static NV_INT32   max_ndx_file_num = -1;
    static NV_INT32   max_hyp_file_num = -1;

    /*  Generate the sub-file name. */

    sprintf (filename, "%s%shugefile.%03d", directory_path[handle], HUGE_SLASH_STR, file);

    if (file >= NUM_SUB_FILES) {
      fprintf (stderr, "\n\nCan't open %s ...\n", filename);
      fprintf (stderr, "MAXIMUM Number of subfiles has been EXCEEDED!\nPlease reduce size of PFM or split into smaller sections.\n\n");
      exit (-1);
    }

    /*  If a previous sub-file was opened for write or update, flush the
        buffer.  */

    if (cur_file[handle] >= 0 && (strchr (mode, 'w') != NULL || strchr (mode,
        '+') != NULL))
    {
        fflush (file_pnt[handle][cur_file[handle]]);
    }


    /*  Save the new current file number.  */

    cur_file[handle] = file;



    /*  Open the sub-file.  */

    if ((file_pnt[handle][file] = fopen (filename, file_mode[handle])) ==
        NULL)
    {
        /*  If we're in read/update mode and we're appending we might have to
            create a new file.  */

        if (strchr (mode, 'r') != NULL && strchr (mode, '+') != NULL && op)
        {
            if ((file_pnt[handle][file] = fopen (filename, "w+")) == NULL)
            {
                return (-1);
            }
            else
            {
                fclose (file_pnt[handle][file]);
                file_pnt[handle][file] = fopen (filename, file_mode[handle]);
            }
        }
        else
        {
            return (-1);
        }
    }

    /* By keeping track of these static max sub_file numbers, you will see them all when loading but
       only once in SABER when opening, querying, etc. because the library stays linked as long
       as SABER is open.
    */

    if ((strstr(filename, ".pfm.bin")) && (file > max_bin_file_num)) {
      max_bin_file_num = file;
      /*fprintf (stderr, "Opening BIN sub_file:  %s ...\n", filename);*/
    }
    else if ((strstr(filename, ".pfm.ndx")) && (file > max_ndx_file_num)) {
      max_ndx_file_num = file;
      /*fprintf (stderr, "Opening INDEX sub_file:  %s ...\n", filename);*/
    }
    else if ((strstr(filename, ".pfm.hyp")) && (file > max_hyp_file_num)) {
      max_hyp_file_num = file;
      /*fprintf (stderr, "Opening HYP sub_file:  %s ...\n", filename);*/
    }

    /*  Set the number of files if this file number is larger than the count.
        This is used when writing a record to a new sub-file or when reading a
        record from a sub-file that has not been previously opened. */

    if (num_files[handle] <= file) num_files[handle] = file + 1;


    /*  Initialize the file context.  */

    file_context[handle] = ' ';


    /*  Return the handle.  */

    return (handle);
}




/***************************************************************************/
/*!

  - Module Name:        get_file_pos

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Gets the sub-file number and position within the
                        sub-file based on the handle and the double
                        precision offset.

  - Arguments:
                        - offset          =   See fseek (same except long 
                                              long), position, or end of file
                        - whence          =   See fseek
                        - handle          =   huge file handle
                        - file            =   sub-file number
                        - pos             =   position within sub-file (from
                                              beginning of file)

  - Return Value:
                        - Position offset from beginning of huge file (to be
                          used as current position within file)

  - Caveats:            This function is only used internal to the library.

****************************************************************************/

static NV_INT64 get_file_pos (NV_INT64 offset, NV_INT32 whence, NV_INT32 handle, NV_INT32 *file, NV_INT32 *pos)
{
    NV_INT64            position;


    /*  Compute the position based on whence.  The position is the offset from
        the beginning of the huge file. */

    switch (whence)
    {
        case SEEK_SET:
            position = offset;
            break;

        case SEEK_CUR:
            position = cur_pos[handle] + offset;
            break;

        case SEEK_END:
            position = eof_huge[handle] + offset;
            break;

        default:
            fprintf (stderr, "\n\n Invalid mode for seek.\n\n");
            exit (-1);
    }


    /*  Compute the sub-file number and the position within that sub-file.  */

    *file = position / (NV_INT64) MAX_SUB_FILE_SIZE;
    *pos = position % (NV_INT64) MAX_SUB_FILE_SIZE;


    /*  Return the offset from the beginning of the huge file.  */

    return (position);
}




/***************************************************************************/
/*!

  - Module Name:        hfseek

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Positions to offset within the huge file.

  - Arguments:
                        - handle          =   huge file handle
                        - offset          =   See fseek (same except long long)
                        - whence          =   See fseek

  - Return Value:
                        - 0               =   success
                        - -1              =   failure, see errno for reason

****************************************************************************/

NV_INT32 hfseek (NV_INT32 handle, NV_INT64 offset, NV_INT32 whence)
{
    NV_INT32                    file, pos;


#ifdef LAZY_HFSEEK

    /*  Check if a seek is required or not.  */
    /*  NOTE:  SEEK_CUR is used in the case that a seek is really required!
     *         do NOT short circuit SEEK_CUR.  */

    if ((( whence == SEEK_SET ) && ( hftell( handle ) == offset )) ||
        (( whence == SEEK_END ) && ( hftell( handle ) == ( eof_huge[handle] + offset ))))
      {
        /*  The file pointer is at the desired location, so save a seek.  */

        return 0;
      }
#endif


    /*  Get the sub-file number and position within the sub-file.  Also, save
        the current position within the huge file.  */

    cur_pos[handle] = get_file_pos (offset, whence, handle, &file, &pos);

    /*  If this sub-file has not been previously opened, open it.   */

    if (file_pnt[handle][file] == NULL)
    {
        if (open_sub_file (file_path[handle], file_mode[handle], handle, file, 0) == -1) return (-1);
    }


    /*  Seek to the computed position within the sub-file and return the
        status.    */

    return (fseek (file_pnt[handle][file], pos, SEEK_SET));
}




/***************************************************************************/
/*!

  - Module Name:        hfopen

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Opens a huge file (actually just assigns the
                        handle, opens the first sub-file, and does some
                        setup).

  - Arguments:
                        - path            =   See fopen (actual files created
                                              are path, path.000, path.001,
                                              etc)
                        - mode            =   See fopen (append not supported)

  - Return Value:
                        - file handle (0 or higher)
                        - -1 on failure

****************************************************************************/

NV_INT32 hfopen (NV_CHAR *path, NV_CHAR *mode)
{
    NV_INT32                    i, handle = -1;
    NV_CHAR                     filename[FILE_NAME_SIZE];
    NV_CHAR                     tempString[512];


    /*  We don't support 'append' mode yet. */

    if (strchr (mode, 'a') != NULL)
    {
        fprintf (stderr, "\n\n Append mode not yet supported!!\n\n");
        exit (-1);
    }


    /*  Check for an available handle.  */

    for (i = 0 ; i < NUM_HUGE_FILES ; i++)
    {
        if (file_handle[i] == -1)
        {
            handle = i;
            file_handle[handle] = i;
            break;
        }
    }


    /*  No free handles available.  */

    if ( handle == -1 ) return (-1);


    /*  Get the directory path from the file name.  */

    strcpy (directory_path[handle], "");
    for (i = strlen (path) ; i >= 0 ; i--)
    {
        if (path[i] == HUGE_SLASH)
        {
            strncpy (directory_path[handle], path, i + 1);
            directory_path[handle][i + 1] = 0;


            /*  Save the path name.  */

            strcpy (file_path[handle], &path[i + 1]);

            strcat (directory_path[handle], &path[i + 1]);
            break;
        }
    }

    if (!strcmp (directory_path[handle], ""))
    {
        /*  Save the path name.  */

        strcpy (file_path[handle], path);

        strcat (directory_path[handle], path);
    }


    /*  Save the mode.   */

    strcpy (file_mode[handle], mode);


    /*  Try to create the directory.  If it already exists we'll fail but
        we don't really care as file opens will trap any major problems.  */

#ifdef NVWIN3X
  #ifdef __MINGW64__
        mkdir( directory_path[handle] );
  #else
        _mkdir( directory_path[handle] );
  #endif
#else
        mkdir( directory_path[handle], 00777 );
#endif


    /*  Create the config file name (this file holds the EOF position for the
        huge file, maybe more info later).   */

    strcpy (filename, path);
    sprintf( tempString, "%sconfig", HUGE_SLASH_STR );
    strcat (filename, tempString);


    /*  Set the EOF for the huge file to 0.   */

    eof_huge[handle] = 0;


    /*  Open the config file based on the mode. */

    if (strchr (mode, 'w') != NULL || strchr (mode, '+') != NULL)
    {
        /*  Try to open for read update.  If not, open for write update.    */

        if ((cfg_file_pnt[handle] = fopen (filename, "rb+")) == NULL)
        {
            if ((cfg_file_pnt[handle] = fopen (filename, "wb+")) == NULL)
            {
                file_handle[handle] = -1;
                return (-1);
            }
        }
        else
        {
            /*  If file opened for read update, read the EOF from the file. */

            fscanf (cfg_file_pnt[handle], NV_INT64_SPECIFIER, &eof_huge[handle]);  /* GS: need different int64 specifier for windows */
        }
    }
    else
    {
        /*  If file opened for read, read EOF from the file. */

        if ((cfg_file_pnt[handle] = fopen (filename, "rb")) == NULL)
        {
            file_handle[handle] = -1;
            return (-1);
        }

        fscanf (cfg_file_pnt[handle], NV_INT64_SPECIFIER, &eof_huge[handle]);  /* GS: need different int64 specifier for windows */
    }




    /*  Set the number of sub-files to 0, the current position to 0, and
        the current file to -1.   */

    num_files[handle] = 0;
    cur_pos[handle] = 0;
    cur_file[handle] = -1;


    /*  Set all of the sub-file pointers for this handle to NULL (they don't
        default that way on some systems).  */

    for (i = 0 ; i < NUM_SUB_FILES ; i++)
        file_pnt[handle][i] = NULL;


    /*  Open the first sub-file.    */

    if (open_sub_file (file_path[handle], mode, handle, 0, 0) == -1)
    {
        file_handle[handle] = -1;
        return (-1);
    }


    /*  Return the file handle.    */

    return (handle);
}




/***************************************************************************/
/*!

  - Module Name:        hfread

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Reads data from the huge file pointed to by handle.

  - Arguments:
                        - ptr             =   See fread
                        - size            =   See fread
                        - nmemb           =   See fread
                        - handle          =   handle of huge file

  - Return Value:
                        - Number of items read
                        - 0 on failure

\***************************************************************************/

size_t hfread (void *ptr, size_t size, size_t nmemb, NV_INT32 handle)
{
    NV_INT32                    file, pos, buffer_size, partial_buffer_size;
    NV_U_BYTE                   *t_ptr;

#ifdef LAZY_HFSEEK

    /*  Note if the file context switched or not.  */
    if (file_context[handle] != 'r')
    {
        file_context[handle] = 'r';
        hfseek( handle, 0, SEEK_CUR );
    }
#endif

    /*  Set the temporary pointer to the pointer that was passed in.    */

    t_ptr = ptr;


    /*  Get the sub-file number and the position within the sub-file.   */

    get_file_pos (cur_pos[handle], SEEK_SET, handle, &file, &pos);


    /*  If this sub-file hasn't been opened yet, open it.   */

    if (file_pnt[handle][file] == NULL)
    {
        if (open_sub_file (file_path[handle], file_mode[handle], handle, file, 0) == -1) return (0);
    }


    /*  Set the buffer size.    */

    buffer_size = nmemb * size;


    /*  Fix for a very rare bug that was occurring with the hyp file.  Found by Jeff Parker (SAIC).
        I can't wait to move to PFM 6.0 and stop using this stuff except for legacy files - JCD.  */

    if (ftell (file_pnt[handle][file]) != pos)
      {
        /*printf ("In hfread:  Forcing fseek from position %ld to %d in file %d ...\n", ftell(file_pnt[handle][file]), pos, file);*/
        fflush (stdout);
        fseek (file_pnt[handle][file], pos, SEEK_SET);
      }


    /*  If the read is going to overlap sub-files, read a partial buffer from
        each of the two sub-files.   */

    if (((NV_INT64) pos + (NV_INT64) buffer_size) > MAX_SUB_FILE_SIZE)
    {
        /*  Compute the partial buffer size for the first part. */

        partial_buffer_size = MAX_SUB_FILE_SIZE - pos;


        /*  Read the first partial buffer and put it at the beginning of the
            full buffer. */

        if (!fread (t_ptr, partial_buffer_size, 1, file_pnt[handle][file]))
           return (0);


        /*  Set the current position.   */

        cur_pos[handle] += partial_buffer_size;


        /*  Get the sub-file number and the position within the sub-file
            (should be 0) for the second partial buffer.  */

        get_file_pos (cur_pos[handle], SEEK_SET, handle, &file, &pos);


        /*  If this sub-file hasn't been opened yet, open it.   */

        if (file_pnt[handle][file] == NULL)
        {
            if (open_sub_file (file_path[handle], file_mode[handle], handle, file, 0) == -1) return (0);
        }


        /*  Set the temporary pointer to the correct location in the full
            buffer.   */

        t_ptr += partial_buffer_size;


        /*  Compute the second partial buffer size. */

        partial_buffer_size = buffer_size - partial_buffer_size;


        /*  Seek to the position (should be 0) since this file may have been
            opened already.  */

        fseek (file_pnt[handle][file], pos, SEEK_SET);


        /*  Read the second partial buffer. */

        if (!fread (t_ptr, partial_buffer_size, 1, file_pnt[handle][file]))
            return (0);


        /*  Set the current position.   */

        cur_pos[handle] += partial_buffer_size;
    }
    else
    {
        /*  Read the buffer.    */

        if (!fread (t_ptr, buffer_size, 1, file_pnt[handle][file])) return (0);


        /*  Set the current position.   */

        cur_pos[handle] += buffer_size;
    }


    /*  Return the number of items read.    */

    return (nmemb);
}




/***************************************************************************/
/*!

  - Module Name:        hfwrite

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Writes data to the huge file pointed to by handle.

  - Arguments:
                        - ptr             =   See fwrite
                        - size            =   See fwrite
                        - nmemb           =   See fwrite
                        - handle          =   handle of huge file

  - Return Value:
                        - Number of items written
                        - 0 on failure

****************************************************************************/

size_t hfwrite (void *ptr, size_t size, size_t nmemb, NV_INT32 handle)
{
    NV_INT32                    file, pos, buffer_size, partial_buffer_size;
    NV_U_BYTE                   *t_ptr;


#ifdef LAZY_HFSEEK

    /*  Note if the file context switched or not.  */
    if (file_context[handle] != 'w')
    {
        file_context[handle] = 'w';
        hfseek( handle, 0, SEEK_CUR );
    }
#endif


    /*  Set the temporary pointer to the pointer that was passed in.    */

    t_ptr = ptr;


    /*  Get the sub-file number and the position within the sub-file.   */

    get_file_pos (cur_pos[handle], SEEK_SET, handle, &file, &pos);


    /*  If this sub-file hasn't been opened yet, open it.   */

    if (file_pnt[handle][file] == NULL)
    {
        if (open_sub_file (file_path[handle], file_mode[handle], handle, file, 1) == -1) return (0);
    }


    /*  Set the buffer size.    */

    buffer_size = nmemb * size;


    /*  Fix for a very rare bug that was occurring with the hyp file.  Found by Jeff Parker (SAIC).
        I can't wait to move to PFM 6.0 and stop using this stuff except for legacy files - JCD.  */

    if (ftell (file_pnt[handle][file]) != pos)
      {
        printf ("In hfwrite: Forcing fseek from position %ld to %d in file %d ...\n", ftell(file_pnt[handle][file]), pos, file);
        fflush(stdout);
        fseek (file_pnt[handle][file], pos, SEEK_SET);
      }


    /*  If the write is going to overlap sub-files, write a partial buffer to
        each of the two sub-files.   */

    if (((NV_INT64) pos + (NV_INT64) buffer_size) > MAX_SUB_FILE_SIZE)
    {
        /*  Compute the partial buffer size for the first part. */

        partial_buffer_size = MAX_SUB_FILE_SIZE - pos;


        /*  Write the first partial buffer from the beginning of the full
            buffer.   */

        if (!fwrite (t_ptr, partial_buffer_size, 1, file_pnt[handle][file]))
            return (0);


        /*  Set the current position.   */

        cur_pos[handle] += partial_buffer_size;


        /*  Get the sub-file number and the position within the sub-file
            (should be 0) for the second partial buffer.  */

        get_file_pos (cur_pos[handle], SEEK_SET, handle, &file, &pos);


        /*  If this sub-file hasn't been opened yet, open it.   */

        if (file_pnt[handle][file] == NULL)
        {
            if (open_sub_file (file_path[handle], file_mode[handle], handle, file, 1) == -1) return (0);
        }


        /*  Set the temporary pointer to the correct location in the full
            buffer.   */

        t_ptr += partial_buffer_size;


        /*  Compute the second partial buffer size. */

        partial_buffer_size = buffer_size - partial_buffer_size;


        /*  Seek to the position (should be 0) since this file may have been
            opened already.  */

        fseek (file_pnt[handle][file], pos, SEEK_SET);


        /*  Write the second partial buffer. */

        if (!fwrite (t_ptr, partial_buffer_size, 1, file_pnt[handle][file]))
            return (0);


        /*  Set the current position.   */

        cur_pos[handle] += partial_buffer_size;
    }
    else
    {
        /*  Write the buffer.    */

        if (!fwrite (t_ptr, buffer_size, 1, file_pnt[handle][file]))
            return (0);


        /*  Set the current position.   */

        cur_pos[handle] += buffer_size;
    }


    /*  If the current position is larger than the EOF, set the EOF to the
        current position.   */

    if (cur_pos[handle] > eof_huge[handle]) eof_huge[handle] = cur_pos[handle];


    /*  Return the number of items read.    */

    return (nmemb);
}




/***************************************************************************/
/*!

  - Module Name:        hftell

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Get the current position within a huge file.

  - Arguments:
                        - handle          =   handle of huge file

  - Return Value:
                        - Current position within the huge file

****************************************************************************/

NV_INT64 hftell (NV_INT32 handle)
{
    return (cur_pos[handle]);
}




/***************************************************************************/
/*!

  - Module Name:        hftruncate

  - Programmer(s):      Jan C. Depner

  - Date Written:       September 2001

  - Purpose:            Truncates the file to "length" bytes.

  - Arguments:
                        - handle          =   handle of huge file
                        - length          =   length

  - Return Value:
                        - 0               =   success
                        - -1              =   failure, see errno for reason

****************************************************************************/

NV_INT32 hftruncate (NV_INT32 handle, NV_INT64 length)
{
    NV_INT32                    i, file, pos, fd, stat;
    NV_CHAR                     filename[FILE_NAME_SIZE];


    /*  Get the sub-file number and position within the sub-file.  Also, save
        the current position within the huge file.  */

    get_file_pos (length, SEEK_SET, handle, &file, &pos);


    /*  If this sub-file has been previously opened, close it.   */

    if (file_pnt[handle][file] != NULL)
    {
        fclose (file_pnt[handle][file]);
        file_pnt[handle][file] = NULL;
    }


    /*  Generate the sub-file name. */

    sprintf (filename, "%s%shugefile.%03d", directory_path[handle],
        HUGE_SLASH_STR, file);


    /*  Truncate the file.  */

    fd = open (filename, O_RDWR);

    if (fd < 0) return (fd);


#ifdef NVWIN3X
  #ifdef __MINGW64__
    stat = _chsize (fd, pos);
  #else
    stat = _chsize (fd, pos);
  #endif
#else
    stat = ftruncate (fd, pos);
#endif

    if (stat) return (stat);

    close (fd);


    /*  Write the EOF value to the config file.  */

    fseek (cfg_file_pnt[handle], 0, SEEK_SET);

    eof_huge[handle] = length;

    fprintf (cfg_file_pnt[handle], NV_INT64_SPECIFIER "\n", eof_huge[handle]); /* GS: need different int64 specifier for windows */

    fflush (cfg_file_pnt[handle]);

    fseek (cfg_file_pnt[handle], 0, SEEK_SET);


    /*  Loop through and remove later sub-files.   */

    for (i = pos + 1 ; i < num_files[handle] ; i++)
    {
        /*  If the sub-file had been opened, close it.  */

        if (file_pnt[handle][i] != NULL)
        {
            sprintf (filename, "%s%shugefile.%03d", directory_path[handle],
                HUGE_SLASH_STR, file);


            fclose (file_pnt[handle][i]);

            remove (filename);

            file_pnt[handle][i] = NULL;
        }
    }

    num_files[handle] = file + 1;

    return (0);
}




/***************************************************************************/
/*!

  - Module Name:        hfclose

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Closes a huge file (actually closes all open
                        sub-files).

  - Arguments:
                        - handle          =   handle of huge file

  - Return Value:
                        - 0               =   success
                        - EOF             =   failure

\***************************************************************************/

NV_INT32 hfclose (NV_INT32 handle)
{
    NV_INT32            i, status;
    NV_CHAR             filename[FILE_NAME_SIZE];
    NV_INT64            tmp_huge;


    /*  If the file was opened for write or update, write the EOF to the
        config file.    */

    if (strchr (file_mode[handle], 'w') != NULL ||
        strchr (file_mode[handle], '+') != NULL)
    {
        /* read the EOF currently on disk... */

        i = fseek (cfg_file_pnt[handle], 0, SEEK_SET);

        i = fscanf (cfg_file_pnt[handle], NV_INT64_SPECIFIER "\n", &tmp_huge);

        /*
           If the EOF on disk is larger than what is known here, leave it alone.
           We were observing a corruption otherwise.
           When one program is accessing a PFM, and then separately someone runs
           the loader, adding data.  The pfm_loader closes and updates the EOF
           in the ndx config.  Now a bit later, after closing the accessing program
           (MVE in our case) we'd overwrite the updated config file with the old EOF.
           The corruption would only be exposed after subsequent loads - and was very
           subtle to discover and diagnose.

           Webb McDonald -- Mon Jun 27 14:08:58 2005

        */

        if (i > 0 && tmp_huge > eof_huge[handle])
        {
            /*
               Kinda pointless, since we're closing the file,
               this should get set to the same on the next open,
               but at least at this point we have the most current EOF now
            */
            eof_huge[handle] = tmp_huge;
        }
        else
        {
            i = fseek (cfg_file_pnt[handle], 0, SEEK_SET);

            i = fprintf (cfg_file_pnt[handle], NV_INT64_SPECIFIER "\n", eof_huge[handle]);  /* GS: need different int64 specifier for windows */
        }

        i = fclose (cfg_file_pnt[handle]);
    }


    /*  Loop through and close the sub-files.   */

    status = 0;
    for (i = 0 ; i < num_files[handle] ; i++)
    {
        /*  If the sub-file had been opened, close it.  */

        if (file_pnt[handle][i] != NULL)
        {
            /*  Check for error on close.   */

            if (fclose (file_pnt[handle][i]))
            {
                sprintf (filename, "%s%s%s.%03d", directory_path[handle],
                    HUGE_SLASH_STR, file_path[handle], i);
                perror (filename);
                status = EOF;
            }

            file_pnt[handle][i] = NULL;
        }
    }


    /*  Set the number of sub-files to 0, the current position to 0, and
        the current file to -1.   */

    num_files[handle] = 0;
    cur_pos[handle] = 0;
    cur_file[handle] = -1;


    /*  Make this handle available again.  */

    file_handle[handle] = -1;


    /*  Return the status.  */

    return (status);
}



/***************************************************************************/
/*!

  - Module Name:        hrewind

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Rewinds a huge file (positions to beginning of the
                        first sub-file).

  - Arguments:
                        - handle          =   handle of huge file

  - Return Value:
                        - void

****************************************************************************/

void hrewind (NV_INT32 handle)
{
    hfseek (handle, 0, SEEK_SET);
}



/***************************************************************************/
/*!

  - Module Name:        hfflush

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1998

  - Purpose:            Flushes the buffer for the opened sub-file.

  - Arguments:
                        - handle          =   handle of huge file

  - Return Value:
                        - void

****************************************************************************/

void hfflush (NV_INT32 handle)
{
    fflush (file_pnt[handle][cur_file[handle]]);
}

