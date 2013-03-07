
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



#include "pfmWDBSecurity.hpp"

/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBLoadSecurity

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Reads the security.cfg file in the $PFMWDB directory and loads the 
                         security keys into an array in memory.

   - Arguments:
                         - security       =  The PFMWDB_SECURITY structure array into which
                                             the keys are loaded.

   - Return Value:
                         - NV_BOOL        =  NVFalse on error, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBLoadSecurity (PFMWDB_SECURITY *security)
{
  NV_CHAR security_file[512], pfmwdb[512];
  FILE *fp;


  memset (security, 0, PFMWDB_MAX_SECURITY_KEY * sizeof (PFMWDB_SECURITY));


  //  Check for the PFMWDB environment variable

  if (getenv ("PFMWDB") == NULL)
    {
      fprintf (stderr, "The PFMWDB environment variable is not set.\nThis must point to the folder that contains the one-degree PFM files.");
      fflush (stderr);
      return (NVFalse);
    }


  strcpy (pfmwdb, getenv ("PFMWDB"));


  sprintf (security_file, "%s%1csecurity.cfg", pfmwdb, SEPARATOR);


  NV_CHAR security_string[8192];


  if ((fp = fopen (security_file, "r")) != NULL)
    {
      NV_INT32 cnt = 0;

      while (ngets (security_string, 8192, fp))
        {
          //  Check the first line (field definition).

          if (!cnt)
            {
              if (strncmp (security_string, "SECURITY_KEY,HANDLING_INSTRUCTIONS,DOWNGRADE_AUTHORITY,DISTRIBUTION_LIMITATIONS,DISTRIBUTION_LIMIT_REASON,CLASSIFICATION_REF_ENCLOSURE,EXEMPTION_CATEGORY,CLASSIFICATION,FULL_DISTRIBUTION_CODE,DISTRIBUTION_STATEMENT_PARTA,DISTRIBUTION_STATEMENT_PARTB", 249))
                {
                  fprintf (stderr, "Security file header in %s has changed.\nPlease modify pfmWDB_loadSecurity.cpp to match the new file.\n",
                           security_file);
                  return (NVFalse);
                }
            }
          else
            {
              //  We have to replace commas with ^ where they occur within quoted strings so that QString::section will work properly.
              //  After parsing out the sections we'll replace the ^ with a comma in the sections.  We also want to get rid of the quotes.  
              //  What a PITA!

              NV_BOOL quote_start = NVFalse;
              for (NV_U_INT32 i = 0 ; i < strlen (security_string) ; i++)
                {
                  if (quote_start)
                    {
                      if (security_string[i] == '"')
                        {
                          quote_start = NVFalse;
                        }
                      else
                        {
                          if (security_string[i] == ',') security_string[i] = '^';
                        }
                    }
                  else
                    {
                      if (security_string[i] == '"') quote_start = NVTrue;
                    }
                }


              QString securityString = QString (security_string);

              NV_INT32 ndx = securityString.section (',', 0, 0).toInt ();

              security[ndx].populated = NVTrue;
              security[ndx].handling_instructions = new QString (securityString.section (',', 1, 1));
              security[ndx].downgrade_authority = new QString (securityString.section (',', 2, 2));
              security[ndx].distribution_limitations = new QString (securityString.section (',', 3, 3));
              security[ndx].distribution_limit_reason = new QString (securityString.section (',', 4, 4));
              security[ndx].classification_ref_enclosure = new QString (securityString.section (',', 5, 5));
              security[ndx].exemption_category = new QString (securityString.section (',', 6, 6));
              security[ndx].classification = new QString (securityString.section (',', 7, 7));
              security[ndx].full_distribution_code = new QString (securityString.section (',', 8, 8));
              security[ndx].distribution_statement_part_a = new QString (securityString.section (',', 9, 9));
              security[ndx].distribution_statement_part_b = new QString (securityString.section (',', 10, 10));


              //  Replace ^ with , and remove "

              security[ndx].handling_instructions->replace ('^', ',');
              security[ndx].handling_instructions->remove ('"');
              security[ndx].downgrade_authority->replace ('^', ',');
              security[ndx].downgrade_authority->remove ('"');
              security[ndx].distribution_limitations->replace ('^', ',');
              security[ndx].distribution_limitations->remove ('"');
              security[ndx].distribution_limit_reason->replace ('^', ',');
              security[ndx].distribution_limit_reason->remove ('"');
              security[ndx].classification_ref_enclosure->replace ('^', ',');
              security[ndx].classification_ref_enclosure->remove ('"');
              security[ndx].exemption_category->replace ('^', ',');
              security[ndx].exemption_category->remove ('"');
              security[ndx].classification->replace ('^', ',');
              security[ndx].classification->remove ('"');
              security[ndx].full_distribution_code->replace ('^', ',');
              security[ndx].full_distribution_code->remove ('"');
              security[ndx].distribution_statement_part_a->replace ('^', ',');
              security[ndx].distribution_statement_part_a->remove ('"');
              security[ndx].distribution_statement_part_b->replace ('^', ',');
              security[ndx].distribution_statement_part_b->remove ('"');

              if (security[ndx].classification->contains ("P")) security[ndx].class_flags = 0x01;
              if (security[ndx].classification->contains ("U")) security[ndx].class_flags = 0x02;
              if (security[ndx].classification->contains ("C")) security[ndx].class_flags = 0x04;
              if (security[ndx].classification->contains ("S")) security[ndx].class_flags = 0x08;
              if (security[ndx].classification->contains ("T")) security[ndx].class_flags = 0x10;
            }
          cnt++;
        }
      fclose (fp);

      return (NVTrue);
    }

  fprintf (stderr, "Unable to open security configuration file %s\n", security_file);
  return (NVFalse);
}
