
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



#include "swap_bytes.h"

/***************************************************************************/
/*!

  - Module Name:        swap_int

  - Programmer(s):      Jan C. Depner

  - Date Written:       July 1992

  - Purpose:            This function swaps bytes in a four byte int.

  - Arguments:          word                -   pointer to the int

****************************************************************************/

void swap_int (NV_INT32 *word)
{
    NV_U_INT32    temp[4];

    temp[0] = *word & 0x000000ff;

    temp[1] = (*word & 0x0000ff00) >> 8;

    temp[2] = (*word & 0x00ff0000) >> 16;

    temp[3] = (*word & 0xff000000) >> 24;

    *word = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];
}



/***************************************************************************/
/*!

  - Module Name:        swap_float

  - Programmer(s):      Jan C. Depner

  - Date Written:       July 1992

  - Purpose:            This function swaps bytes in a four byte float.

  - Arguments:          word                -   pointer to the float

****************************************************************************/

void swap_float (NV_FLOAT32 *word)
{
    NV_U_INT32          temp[4];

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



/***************************************************************************/
/*!

  - Module Name:        swap_double

  - Programmer(s):      Jan C. Depner

  - Date Written:       July 1992

  - Purpose:            This function swaps bytes in an eight byte double.

  - Arguments:          word                -   pointer to the double

****************************************************************************/

void swap_double (NV_FLOAT64 *word)
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



/***************************************************************************/
/*!

  - Module Name:        swap_short

  - Programmer(s):      Jan C. Depner

  - Date Written:       July 1992

  - Purpose:            This function swaps bytes in a two byte int.

  - Arguments:          word                -   pointer to the int

****************************************************************************/

void swap_short (NV_INT16 *word)
{
    NV_INT16   temp;
#ifdef NVWIN3X
  #ifdef __MINGW64__
    swab((char *)word, (char *)&temp, 2);
  #else
    _swab((char *)word, (char *)&temp, 2);
  #endif
#else
    swab (word, &temp, 2);
#endif
    *word = temp;

    return;
}
