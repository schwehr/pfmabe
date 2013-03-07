
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



/********************************************************************************************/
/*!

   - Module Name:        binaryFeatureData_ngets

   - Programmer(s):      Jan C. Depner

   - Date Written:       May 1999

   - Purpose:            This is an implementation of fgets that strips the line feed (or
                         carriage return/line feed) off of the end of the string if present.

   - Arguments:          See fgets

   - Return Value:       See fgets

*********************************************************************************************/

static NV_CHAR *binaryFeatureData_ngets (NV_CHAR *s, NV_INT32 size, FILE *stream)
{
  if (fgets (s, size, stream) == NULL) return (NULL);

  while (strlen(s) > 0 && (s[strlen(s) - 1] == '\n' || s[strlen(s) - 1] == '\r')) s[strlen(s) - 1] = '\0';

  if (s[strlen (s) - 1] == '\n') s[strlen (s) - 1] = 0;


  return (s);
}



/********************************************************************************************/
/*!

  - Module Name:        swap_int

  - Programmer(s):      Jan C. Depner

  - Date Written:       July 1992

  - Purpose:            This function swaps bytes in a four byte int.

  - Arguments:          word                -   pointer to the int

*********************************************************************************************/

static void binaryFeatureData_swap_int (NV_INT32 *word)
{
    NV_U_INT32    temp[4];

    temp[0] = *word & 0x000000ff;

    temp[1] = (*word & 0x0000ff00) >> 8;

    temp[2] = (*word & 0x00ff0000) >> 16;

    temp[3] = (*word & 0xff000000) >> 24;

    *word = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];
}



/********************************************************************************************/
/*!

  - Module Name:        swap_float

  - Programmer(s):      Jan C. Depner

  - Date Written:       July 1992

  - Purpose:            This function swaps bytes in a four byte float.

  - Arguments:          word                -   pointer to the float

*********************************************************************************************/

static void binaryFeatureData_swap_float (NV_FLOAT32 *word)
{
    unsigned int    temp[4];

    union
    {
        NV_U_INT32      iword;
        NV_FLOAT32      fword;
    } eq;

    eq.fword = *word;

    temp[0] = eq.iword & 0x000000ff;

    temp[1] = (eq.iword & 0x0000ff00) >> 8;

    temp[2] = (eq.iword & 0x00ff0000) >> 16;

    temp[3] = (eq.iword & 0xff000000) >> 24;

    eq.iword = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];

    *word = eq.fword;

    return;
}



/********************************************************************************************/
/*!

  - Module Name:        swap_double

  - Programmer(s):      Jan C. Depner

  - Date Written:       January 2000

  - Purpose:            This function swaps bytes in an eight byte double.

  - Arguments:          word                -   pointer to the double

*********************************************************************************************/

static void binaryFeatureData_swap_double (NV_FLOAT64 *word)
{
    NV_INT32         i;
    NV_U_BYTE        temp;
    union
    {
        NV_U_BYTE    bytes[8];
        NV_FLOAT64   d;
    }eq;
    
    memcpy (&eq.bytes, word, 8);

    for (i = 0 ; i < 4 ; i++)
    {
        temp = eq.bytes[i];
        eq.bytes[i] = eq.bytes[7 - i];
        eq.bytes[7 - i] = temp;
    }

    *word = eq.d;
}



static NV_INT32 binaryFeatureData_big_endian ()
{
    union
    {
        NV_INT32        word;
        NV_U_BYTE       byte[4];
    } a;

    a.word = 0x00010203;
    return ((NV_INT32) a.byte[3]);
}



/********************************************************************************************/
/*!

  - Module Name:        get_string

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1994

  - Purpose:            Parses the input string for the equals sign and
                        returns everything to the right.

  - Arguments:
                        - *in     =   Input string
                        - *out    =   Output string

  - Return Value:
                        - NVFalse on error
                        - NVTrue

*********************************************************************************************/

static NV_BOOL binaryFeatureData_get_string (const NV_CHAR *in, NV_CHAR *out)
{
    NV_INT32            i, start, length;
    NV_CHAR             *ptr;


    start = 0;
    length = 0;


    if (!strchr (in, '=')) return (NVFalse);


    ptr = strchr (in, '=') + 1;


    /*  Search for first non-blank character.   */

    for (i = 0 ; i < (NV_INT32) strlen (ptr) ; i++)
    {
        if (ptr[i] != ' ')
        {
            start = i;
            break;
        }
    }


    /*  Search for last non-blank character.    */

    for (i = (NV_INT32) strlen (ptr) ; i >= 0 ; i--)
    {
        if (ptr[i] != ' ' && ptr[i] != 0)
        {
            length = (i + 1) - start;
            break;
        }
    }

    strncpy (out, &ptr[start], length);
    out[length] = 0;

    return (NVTrue);
}



/********************************************************************************************/
/*!

  - Module Name:        gen_basename

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 2004

  - Purpose:            Generic replacement for POSIX basename.  One
                        advantage to this routine over the POSIX one is
                        that it doesn't destroy the input path.  This
                        works on Windoze even when using MSYS (both types
                        of specifiers).

  - Arguments:          path        =   path to parse

  - Return Value:
                        - basename of the file, for example
                              - path           dirname        basename
                              - "/usr/lib"     "/usr"         "lib"
                              - "/usr/"        "/"            "usr"
                              - "usr"          "."            "usr"
                              - "/"            "/"            "/"
                              - "."            "."            "."
                              - ".."           "."            ".."

*********************************************************************************************/
                                                                            
static NV_CHAR *binaryFeatureData_gen_basename (const NV_CHAR *path)
{
  static NV_CHAR  basename[512];
  NV_INT32        i, j, start = 0, len;


  strcpy (basename, path);

  len = strlen (path);

  if (path[len - 1] == '/' || path[len - 1] == '\\') len--;

  start = 0;
  for (i = len - 1 ; i >= 0 ; i--)
    {
      if (path[i] == '/' || path[i] == '\\')
        {
          start = i + 1;
          break;
        }
    }

  if (!start) return (basename);

  for (j = start ; j < len ; j++) basename[j - start] = path[j];
  basename[len - start] = 0;

  return (basename);
}
