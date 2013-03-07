
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
#include <sys/types.h>
#include <errno.h>
#include "nvtypes.h"
#ifndef NVWIN3X
#include <dirent.h>
#else
#include <windows.h>
#endif


/********************************************************************** \
*                                                                       *
*	- FORTRAN callable function.                                    *
*                                                                       *
*       - Calling sequence : fgetarea (areanm, icount, dirnam)          *
*                                                                       *
*       - Author : Jan C. Depner                                        *
*       - Date : 01/15/93                                               *
*                                                                       *
\***********************************************************************/

/*  The _ is required on most machines (not HP).                        */

/*ARGSUSED*/
void fgetarea_ (NV_CHAR *area, NV_INT32 *count, NV_CHAR *dirname, NV_INT32 arealen __attribute__ ((unused)), 
                NV_INT32 dirlen __attribute__ ((unused)))
{
#ifndef NVWIN3X
    struct dirent   *dp;
    DIR             *directory;

    if ((directory = opendir (dirname)) == NULL)
    {
        perror (dirname);
        exit (-1);
    }

    while ((dp = readdir (directory)) != NULL)
    {
        if (strlen (dp->d_name) == 3)
        {
            strcpy (area + *count * 80, dp->d_name);
            (*count)++;
        }
    }
#else
	WIN32_FIND_DATA findData;
	HANDLE hSearchHandle;
	BOOL   validFile;

	hSearchHandle = FindFirstFile( __TEXT( "*" ), &findData );
	validFile = ( hSearchHandle != INVALID_HANDLE_VALUE );
	while ( validFile )
	{
		if (strlen ( (const NV_CHAR *)findData.cFileName) == 3)
		{
			strcpy (area + *count * 80, (const NV_CHAR *)findData.cFileName);
			(*count)++;
		}

		/* Get the next matching filename */
		validFile = FindNextFile( hSearchHandle, &findData );
	}

	if ( hSearchHandle != INVALID_HANDLE_VALUE )
	{
		FindClose( hSearchHandle );
	}
#endif
}

/*ARGSUSED*/
void fgetarea (NV_CHAR *area, NV_INT32 *count, NV_CHAR *dirname, NV_INT32 arealen, NV_INT32 dirlen)
{
    fgetarea_ (area, count, dirname, arealen, dirlen);
}


void area_list (NV_CHAR *area, NV_INT32 *count, NV_CHAR *dirname)
{
    NV_INT32     arealen, dirlen;

    dirlen = 0;
    arealen = 0;

    fgetarea_ (area, count, dirname, arealen, dirlen);
}


