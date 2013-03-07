
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

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include <QtCore>

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACX)
#   include <unistd.h>
#elif defined(Q_OS_MACX)
#   include <mach/mach.h>
#   include <mach/machine.h>
#else
#   include <windows.h>
#endif

#include "nvtypes.h"


/***************************************************************************/
/*!

  - Module Name:        getSystemInfo

  - Programmer(s):      Jan C. Depner

  - Date Written:       October 2011

  - Purpose:            Gets the number of active CPUs and the amount of free
                        memory available on the system.

  - Arguments:
                        - cpu_count    =  Number of active CPUs
                        - memory       =  Amount of free memory

  - Return Value:
                        - void

  - Acknowledgements:   Most of this code was stolen from a post by Stuart
                        Nixon on the qt-interest forum and some stuff from
                        Andrew T on stackoverflow.com.

****************************************************************************/

void getSystemInfo (NV_U_INT32 *cpu_count, NV_U_INT64 *memory)
{
  *cpu_count = 1;
  *memory = -1;

#if defined (Q_OS_UNIX) && !defined (Q_OS_MACX)

  *cpu_count = sysconf (_SC_NPROCESSORS_ONLN);

  NV_INT32 pages = sysconf (_SC_AVPHYS_PAGES);
  NV_INT32 page_size = sysconf (_SC_PAGE_SIZE);
  if (page_size < 0) page_size = sysconf (_SC_PAGESIZE);

  *memory = (NV_U_INT64) pages * (NV_U_INT64) page_size;


  //  Just in case we didn't get a decent answer from sysconf...

  if (pages < 0 || page_size < 0)
    {
      FILE *fp;
      if ((fp = fopen ("/proc/meminfo", "r")) != NULL)
        {
          NV_CHAR string[1024];
          while (fgets (string, sizeof (string), fp) != NULL)
            {
              if (!strncmp (string, "MemFree:", 8))
                {
                  sscanf (string, "MemFree: %lld", memory);

                  if (strstr (string, "kB"))
                    {
                      *memory *= 1024;
                    }
                  else if (strstr (string, "mB"))
                    {
                      *memory *= 1048576;
                    }
                  else if (strstr (string, "gB"))
                    {
                      *memory *= 1073741824;
                    }

                  fclose (fp);
                  break;
                }
            }
        }
    }

      
#elif defined (Q_OS_MACX)

  kern_return_t kr;
  struct host_basic_info hostinfo;
  unsigned int count;

  count = HOST_BASIC_INFO_COUNT;
  kr = host_info (mach_host_self (), HOST_BASIC_INFO, (host_info_t) &hostinfo, &count);
  if (kr == KERN_SUCCESS)
    {
      *cpu_count = hostinfo.avail_cpus;
      *memory = hostinfo.memory_size;
    }

#else

  /*  Unfortunately, this part doesn't work on MinGW

  MEMORYSTATUSEX status;
  status.dwLength = sizeof (status);
  NV_BOOL ret = GlobalMemoryStatusEx (&status);
  if (ret) *memory = status.ullAvailPhys * 1024;

  */


  SYSTEM_INFO si;
  GetSystemInfo (&si);
  *cpu_count = si.dwNumberOfProcessors;

#endif

  if (*cpu_count < 1) *cpu_count = 1;
}
