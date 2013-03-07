#include "pfmLoaderDef.hpp"

NV_BOOL update_upr_file (QString parameter_file, FILE_DEFINITION *input_file_def, NV_INT32 input_file_count)
{
  NV_CHAR file[1024];
  strcpy (file, parameter_file.toAscii ());
  FILE *fp;

  if ((fp = fopen (file, "r+")) == NULL)
    {
      perror (file);
      fflush (stderr);
      return (NVFalse);
    }


  NV_CHAR string[1024];
  NV_INT32 pos = 0;
  QString qstring;


  while (fgets (string, sizeof (string), fp) != NULL)
    {
      qstring = string;


      //  Look for the end of the .upr input files section.

      if (qstring.contains ("End Update Parameter File Input Files"))
        {
          //  Move the file pointer to just before the end sentinel.

          fseek (fp, pos, SEEK_SET);


          //  Now add all of the new files to the list.

          for (NV_INT32 i = 0 ; i < input_file_count ; i++)
            {
              if (input_file_def[i].status)
                {
                  strcpy (file, input_file_def[i].name.toAscii ());

                  fprintf (fp, "%s\n", file);
                }
            }

          fprintf (fp, "**********  End Update Parameter File Input Files  **********\n");
        }

      pos = ftell (fp);
    }

  return (NVTrue);
}
