#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#ifdef NVWIN3X
    #include <winsock.h>
#endif

#include "nvutility.h"

#include "gsf.h"


NV_INT32 write_history (NV_INT32 argc, NV_CHAR **argv, NV_CHAR *comment, NV_CHAR *gsfFile __attribute__ ((unused)), NV_INT32 handle)
{
    int             i;
    int             rc;
    int             ret;
    int             len;
    extern int      gsfError;
    char            str[1024];
    time_t          t;
    gsfRecords      rec;
    gsfDataID       gsfID;


    memset (&rec, 0, sizeof(rec));


    /* Load the contents of the gsf history record */

    time (&t);
    rec.history.history_time.tv_sec = t;
    rec.history.history_time.tv_nsec = 0;

    str[0] = '\0';
    len = 0;
    for (i = 0 ; i < argc ; i++)
    {
        len += strlen (argv[i]) + 1;
        if ((unsigned int) len >= sizeof(str))
        {
            return(-1);
        }
        strcat (str, argv[i]);
        strcat (str, " ");
    }

    rec.history.command_line = str;

    rc = gethostname (rec.history.host_name, sizeof (rec.history.host_name));

    rec.history.comment = comment;


    /* Seek to the end of the file and write a new history record */

    ret = gsfSeek (handle, GSF_END_OF_FILE);
    if (ret) 
      {
        fprintf(stderr, "gsfSeek error: %d\n", gsfError);
        fflush (stderr);
      }

    memset (&gsfID, 0, sizeof(gsfID));
    gsfID.recordID = GSF_RECORD_HISTORY;
    ret = gsfWrite (handle, &gsfID, &rec);

    if (ret < 0)
    {
        return (gsfError);
    }


    return(0);
}
