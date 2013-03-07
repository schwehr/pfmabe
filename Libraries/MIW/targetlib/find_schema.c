#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#ifdef WIN32
    #define SEPARATOR '\\'
    #define PATH_SEPARATOR ';'
#else
    #define SEPARATOR '/'
    #define PATH_SEPARATOR ':'
#endif


/***************************************************************************\
*                                                                           *
*   Module Name:        find_schema (based on find_startup_name)            *
*                                                                           *
*   Programmer(s):      Jan C. Depner (Original)                            *
*                       R. W. Ladner - (modified to look for SCHEMA/        *
*                                       GLOBAL_DATA, then HOME/PATH)        *
*   Date Written:       February 2000                                       *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Search the PATH for and return the full path of the *
*                       specified filename.                                 *
*                                                                           *
*   Arguments:          file_name   -   file name to search for             *
*                                                                           *
*   Return Value:       char *      -   char pointer or NULL if not found   *
*                                                                           *
*   Calling Routines:   Utility routine                                     *
*                                                                           * 
*   Method:             Searches the Envonment variables for a file         *
*                       The order is:                                       *
*                          CWD                                              *
*                          HOME                                             *
*                          GLOBAL_DIR                                      *
*                          SCHEMA_DIR                                       *
*                          PATH.                                            *
*                                                                           *
\***************************************************************************/
                                                                            
char *find_schema (const char *file_name)
{
  char           path[4096];
  static char    file[512], tmp[512];
  FILE           *fp;
  int            i, length, start;


  strcpy (file, file_name);


  /*  File in working directory.  */
    
  if ((fp = fopen (file, "r")) != NULL)
    {
      fclose (fp);
      return (file);
    }


  /*  File in HOME directory.  Assuming MSYS on Windoze.  */
    
  if (getenv ("HOME") != NULL)
    {
      sprintf (file, "%s/%s", getenv ("HOME"), file_name);

      if ((fp = fopen (file, "r")) != NULL)
        {
          fclose (fp);
          return (file);
        }

      sprintf (file, "%s\\%s", getenv ("HOME"), file_name);

      if ((fp = fopen (file, "r")) != NULL)
        {
          fclose (fp);
          return (file);
        }
    }


  /*  File in GLOBAL_DIR directory. */
    
  if (getenv ("GLOBAL_DIR") != NULL)
    {
      sprintf (file, "%s/%s", getenv ("GLOBAL_DIR"), file_name);

      if ((fp = fopen (file, "r")) != NULL)
        {
          fclose (fp);
          return (file);
        }

      sprintf (file, "%s\\%s", getenv ("GLOBAL_DIR"), file_name);

      if ((fp = fopen (file, "r")) != NULL)
        {
          fclose (fp);
          return (file);
        }
    }


  /*  File in SCHEMA_DIR.   */
    
  if (getenv ("SCHEMA_DIR") != NULL)
    {
      strcpy (path, getenv ("SCHEMA_DIR"));

      start = 0;
            
      for (i = 0 ; i < (int) strlen (path) ; i++)
        {
          if (path[i] == PATH_SEPARATOR)
            {
              length = i - start;
              strncpy (file, &path[start], length);
              file[length] = 0;
              start = i + 1;
            
              sprintf (tmp, "%s%1c%s", file, SEPARATOR, file_name);
    
              if ((fp = fopen (tmp, "r")) != NULL)
                {
                  fclose (fp);
                  return (tmp);
                }
            }
        }
    }


  /*  File in path.   */
    
  if (getenv ("PATH") != NULL)
    {
      strcpy (path, getenv ("PATH"));

      start = 0;
            
      for (i = 0 ; i < (int) strlen (path) ; i++)
        {
          if (path[i] == PATH_SEPARATOR)
            {
              length = i - start;
              strncpy (file, &path[start], length);
              file[length] = 0;
              start = i + 1;
            
              sprintf (tmp, "%s%1c%s", file, SEPARATOR, file_name);
    
              if ((fp = fopen (tmp, "r")) != NULL)
                {
                  fclose (fp);
                  return (tmp);
                }
            }
        }
    }


  /*  No file found.  */

  return (NULL);
}
