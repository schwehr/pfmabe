
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



/***********************************************************************/
/*!

  - FORTRAN callable function.

  - fgetsrv presents a menu of all of the surveys being processed.

  - Calling sequence : fgetsrv (drctry, areanm, archiv)

  - Author : Jan C. Depner
  - Date : 09/03/92

************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "nvtypes.h"
#include "ngets.h"


/*  The _ is required on most machines (not HP).                        */

/*ARGSUSED*/
void fgetsrv_ (NV_CHAR *directory, NV_CHAR *areaname, NV_CHAR *archive, NV_INT32 dirlen __attribute__ ((unused)),
               NV_INT32 arealen __attribute__ ((unused)), NV_INT32 arclen __attribute__ ((unused)))
{
  NV_CHAR    srv_file[132], string[132], dirs[21][132], areas[21][50], archives[21][10];
  FILE    *srv;
  NV_INT32     j, selection, manual, count;

  sprintf (srv_file, "%s%1csurveys", getenv ("GLOBAL_DIR"), (NV_CHAR) SEPARATOR);

        
  if ((srv = fopen (srv_file, "r")) == NULL)
    {
      perror (srv_file);
      printf ("\n Please correct the problem and rerun\n");
      exit (-1);
    }

  fflush (stdout);

  count = 0;
    
  do
    {
      printf ("\n\n\n");

      while (1)
        {
          if (fgets (string, 132, srv) == NULL)
            {
              manual = count + 1;
              break;
            }

          string[strlen (string) - 1] = 0;

          if (string[0] != ';')
            {
              for (j = (NV_INT32) strlen (string) ; j >= 0 ; j--)
                {
                  if (string[j] != ' ') break;
                  string[j] = 0;
                }

              if (!strncmp (string, "AREANAME", 8)) strcpy (areas[count], &string[16]);
              if (!strncmp (string, "DIRECTORY", 9)) strcpy (dirs[count], &string[16]);
              if (!strncmp (string, "ARCHIVE", 7))
                {
                  strcpy (archives[count], &string[16]);
                  count++;
                }
            }
        }

      for (j = 0 ; j < count ; j++)
        {
          printf ("   (%2d)   -   %s    %s\n", j + 1, areas[j], archives[j]);
          printf ("              %s\n", dirs[j]);
        }
        
      count++;

      printf ("   (%2d)   -   Manual directory entry\n\n\n\n", manual);

      printf (" Select a survey : ");

      ngets (string, sizeof (string), stdin);

      sscanf (string, "%d", &selection);

    } while (selection < 1 || selection > count);

  if (selection == manual)
    {
      printf ("\n\n Enter working directory name.\n");
      printf (" Enter . for current working directory : ");

      ngets (string, sizeof (string), stdin);

      strcpy (directory, string);
      strcpy (areaname, "TEST");
      strcpy (archive, "NONE");

      for (j = 0 ; j < manual ; j++)
        {
          if (! (strncmp (string, dirs[j], strlen (string))))
            {
              strcpy (areaname, areas[j]);
              strcpy (archive, archives[j]);
              break;
            }
        }
    }
  else
    {
      strcpy (directory, dirs[selection - 1]);
      strcpy (areaname, areas[selection - 1]);
      strcpy (archive, archives[selection - 1]);
    }

  fflush (stdout);
}

/*ARGSUSED*/
void fgetsrv (NV_CHAR *directory, NV_CHAR *areaname, NV_CHAR *archive, NV_INT32 dirlen,
              NV_INT32 arealen, NV_INT32 arclen)
{
  fgetsrv_ (directory, areaname, archive, dirlen, arealen, arclen);
}


/***********************************************************************/
/*!

  - get_survey

  - presents a menu of all of the surveys being processed.

  - Calling sequence : get_survey (directory, areaname, archive)

  - Author : Jan C. Depner
  - Date : 09/03/92

************************************************************************/

void get_survey (NV_CHAR *directory, NV_CHAR *areaname, NV_CHAR *archive)
{
  NV_INT32     dirlen, arealen, arclen;

  dirlen = 0;
  arealen = 0;
  arclen = 0;

  fgetsrv_ (directory, areaname, archive, dirlen, arealen, arclen);
}
