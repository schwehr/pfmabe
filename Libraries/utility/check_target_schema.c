
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



#include "check_target_schema.h"
#include "find_startup_name.h"


/***************************************************************************/
/*!

  - Function        check_target_schema

  - Synopsis        check_target_schema (target_path);
                    - NV_CHAR *target_path - target file name

  - Description     Tries to open the schema file designated in the file
                    target_path (unless target_path = NONE).  If the schema
                    can't be opened we try to find the file "MIWTarget.xsd"
                    somewhere in either the home directory, current
                    directory, or the PATH.  If we find "MIWTarget.xsd" we
                    modify the target_path file to point to the one we
                    found.

  - Returns         NVTrue if the target file is OK, NVFalse otherwise

  - Author          Jan C. Depner

****************************************************************************/ 
 
 
NV_BOOL check_target_schema (const NV_CHAR *target_path)
{
  FILE *tfp, *cfp, *ofp;
  NV_CHAR string[8192], *ptr, fname[512], tmp_fname[512], blank = ' ';
  NV_BOOL eos, ret;
  NV_INT32 i = 0, loc, size;


  ret = NVFalse;


  /*  If target_path is "NONE" we don't have a target file.  */

  if (strcmp (target_path, "NONE"))
    {
      if ((tfp = fopen (target_path, "r+")) != NULL)
        {
	  /*  Get the file size of the target file.  */

	  fseek (tfp, 0, SEEK_END);
	  size = ftell (tfp);
	  fseek (tfp, 0, SEEK_SET);


          /*  Look for the schema file name.  */

          while (fgets (string, sizeof (string), tfp) != NULL)
            {
              ptr = strstr (string, "xsi:noNamespaceSchemaLocation=");
              if (ptr)
                {
                  eos = NVFalse;
                  for (loc = 0 ; loc < 8000 ; loc++)
                    {
                      if (ptr[loc + 30] == '"')
                        {
                          if (eos)
                            {
                              fname[i] = 0;
                              break;
                            }
                          eos = NVTrue;
                        }
                      else
                        {
                          fname[i] = ptr[loc + 30];
                          i++;
                        }
                    }


                  /*  Try to open the schema file.  */

                  if ((cfp = fopen (fname, "r")) == NULL)
                    {
                      /*  We were unable to open the schema file so now we want to look for a copy of the schema file
                          somewhere in the HOME directory, current working directory, or the PATH.  */

                      if (find_startup_name ("MIWTarget.xsd") == NULL)
                        {
                          fclose (tfp);
                          break;
                        }


                      /*  If we got here we found a schema file so now we want to replace the schema file name in the
                          target XML file.  */

                      sprintf (tmp_fname, "temp_target_%d.xml", getpid ());
                      if ((ofp = fopen (tmp_fname, "w")) == NULL)
                        {
                          fclose (tfp);
                          break;
                        }

                      fseek (tfp, 0, SEEK_SET);


                      /*  Read from the target file and write to the temporary output file.  */

                      eos = NVFalse;
                      while (fgets (string, sizeof (string), tfp) != NULL)
                        {
                          if (!eos)
                            {
                              fprintf (ofp, 
                                       "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><Targets xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"%s\">\n", 
                                       find_startup_name ("MIWTarget.xsd"));
                              eos = NVTrue;
                            }
                          else
                            {
                              if (!strstr (string, "xsi:noNamespaceSchemaLocation=")) fprintf (ofp, "%s", string);
                            }
                        }


                      /*  Using rename causes problems in Windows so we'll just write over the target file.  */

                      fclose (ofp);


                      if ((ofp = fopen (tmp_fname, "r")) == NULL)
                        {
                          fclose (tfp);
                          break;
                        }

                      fseek (tfp, 0, SEEK_SET);
                      while (fgets (string, sizeof (string), ofp) != NULL)
                        {
                          fprintf (tfp, "%s", string);
                        }


		      /*  If the new file is shorter than the original we need to clear the end.  I don't think the 
			  truncate function works in Windows.  */

		      while (ftell (tfp) < size) fwrite (&blank, 1, 1, tfp);


                      fclose (tfp);

                      remove (tmp_fname);

                      ret = NVTrue;
                    }
                  else
                    {
                      fclose (tfp);
                      fclose (cfp);

                      ret = NVTrue;
                    }
                  break;
                }
            }
        }
    }


  return (ret);
}
