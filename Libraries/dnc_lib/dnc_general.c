#include "table_header.h"


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/*  We need the library type (HARBOR, APPROACH, COASTAL, or GENERAL) because the tables are formatted
    differently for different library types (brain dead, if you ask me)  */

static NV_BYTE library_type = -1;
static NV_CHAR library_name[20];
static NV_CHAR library_path[512];



/********************************************************************

  Function:    trim_read

  Purpose:     Read an ASCII record and trim off trailing spaces.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   string         -    The text string
               length         -    number of characters to read
               fp             -    file pointer

  Returns:     N/A

********************************************************************/

void trim_read (NV_CHAR *string, NV_INT32 length, FILE *fp)
{
  NV_INT32 i;


  fread (string, length, 1, fp);


  string[length] = 0;


  for (i = length - 1 ; i >= 0 ; i--)
    {
      if (string[i] != ' ') 
        {
          break;
        }
      else
        {
          string[i] = 0;
        }
    }
}


/********************************************************************

  Function:    swap_triplet

  Purpose:     Byte swap the VPF triplet format values

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   triplet        -    The triplet record

  Returns:     N/A

********************************************************************/

void swap_triplet (TRIPLET *triplet)
{
  NV_INT32 i, type[3];
  NV_INT16 type2;


  /*
    The first byte of a triplet defines how much data follows.  The byte is broken up into
    four, two-bit sections.  The first two bits define how many bytes will be in the first section 
    of the triplet.  The next two bits, the second.  The third two bits, the third.  The last two
    bits are TBD.  If the value in a two bit section is 0 there will be no bytes in the corresponding
    data section.  If it is 1 there will be one byte, 2 for two, 3 for four.  Some examples:

      type byte = 1000         1 byte value                                  size = 2 bytes
      type byte = 3000         4 byte value                                  size = 5 bytes
      type byte = 0230         2 byte value then a 4 byte value              size = 7 bytes
      type byte = 3230         4 byte value, 2 byte value, 4 byte value      size = 11 bytes
      type byte = 3330         4 byte value, 4 byte value, 4 byte value      size = 13 bytes
  */

  type[0] = (triplet->type & 0xc0) > 6;
  type[1] = (triplet->type & 0x30) > 4;
  type[2] = (triplet->type & 0x0c) > 2;


  for (i = 0 ; i < 3 ; i++)
    {
      switch (type[i])
        {
        case 0:
        case 1:
          break;

        case 2:
          type2 = triplet->field[i];
          swap_short (&type2);
          triplet->field[i] = type2;
          break;

        case 3:
          swap_int (&triplet->field[i]);
          break;
        }
    }
}



/********************************************************************

  Function:    read_triplet

  Purpose:     Read a VPF triplet format record from the file pointed 
               to by fp.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   triplet        -    The returned triplet record
               fp             -    File pointer

  Returns:     N/A

********************************************************************/

void read_triplet (TRIPLET *triplet, FILE *fp)
{
  NV_INT32 i, type[3], type3;
  NV_U_BYTE type1;
  NV_INT16 type2;


  triplet->field[0] = 0;
  triplet->field[1] = 0;
  triplet->field[2] = 0;


  /*
    The first byte of a triplet defines how much data follows.  The byte is broken up into
    four, two-bit sections.  The first two bits define how many bytes will be in the first section 
    of the triplet.  The next two bits, the second.  The third two bits, the third.  The last two
    bits are TBD.  If the value in a two bit section is 0 there will be no bytes in the corresponding
    data section.  If it is 1 there will be one byte, 2 for two, 3 for four.  Some examples:

      type byte = 1000         1 byte value                                  size = 2 bytes
      type byte = 3000         4 byte value                                  size = 5 bytes
      type byte = 0230         2 byte value then a 4 byte value              size = 7 bytes
      type byte = 3230         4 byte value, 2 byte value, 4 byte value      size = 11 bytes
      type byte = 3330         4 byte value, 4 byte value, 4 byte value      size = 13 bytes
  */

  fread (&triplet->type, sizeof (NV_U_BYTE), 1, fp);
  type[0] = (triplet->type & 0xc0) >> 6;
  type[1] = (triplet->type & 0x30) >> 4;
  type[2] = (triplet->type & 0x0c) >> 2;

  for (i = 0 ; i < 3 ; i++)
    {
      switch (type[i])
        {
        case 0:
          break;

        case 1:
          fread (&type1, sizeof (NV_U_BYTE), 1, fp);
          triplet->field[i] = (NV_INT32) type1;
          break;

        case 2:
          fread (&type2, sizeof (NV_INT16), 1, fp);
          triplet->field[i] = (NV_INT32) type2;
          break;

        case 3:
          fread (&type3, sizeof (NV_INT32), 1, fp);
          triplet->field[i] = type3;
          break;
        }
    }
}



/********************************************************************

  Function:    set_library_type

  Purpose:     Given the path to the DNC library directory determine
               which type of library is currently being used.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   library_path   -    The path to the library directory

  Returns:     N/A

********************************************************************/

void set_library_type (NV_CHAR *path)
{
  strcpy (library_path, path);
  strcpy (library_name, gen_basename (path));


  if (tolower (library_name[0]) == 'h')
    {
      library_type = HARBOR;
    }
  else if (tolower (library_name[0]) == 'a')
    {
      library_type = APPROACH;
    }
  else if (tolower (library_name[0]) == 'c')
    {
      library_type = COASTAL;
    }
  else if (tolower (library_name[0]) == 'g')
    {
      library_type = GENERAL;
    }
  else if (tolower (library_name[0]) == 't')
    {
      library_type = TOD2;
    }
  else
    {
      fprintf (stderr, "\nUnknown library type for path %s\n\n", library_path);
      exit (-1);
    }
}



/********************************************************************

  Function:    get_library_type

  Purpose:     Return the current library type (HARBOR, APPROACH,
               COASTAL, GENERAL).

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   N/A

  Returns:     NV_BYTE        -    HARBOR, APPROACH, COASTAL, GENERAL

********************************************************************/

NV_BYTE get_library_type ()
{
  if (library_type < 0)
    {
      fprintf (stderr, "\nLibrary type has not been set.\n");
      fprintf (stderr, "You must call set_library_type prior to accessing a library.\n\n");
      exit (-1);
    }

  return (library_type);
}



/********************************************************************

  Function:    get_library_name

  Purpose:     Return the current library name

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   N/A

  Returns:     NV_BYTE        -    HARBOR, APPROACH, COASTAL, GENERAL

********************************************************************/

NV_CHAR *get_library_name ()
{
  if (library_type < 0)
    {
      fprintf (stderr, "\nLibrary name has not been set.\n");
      fprintf (stderr, "You must call set_library_type prior to accessing a library.\n\n");
      exit (-1);
    }

  return (library_name);
}
