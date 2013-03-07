#include "nvtypes.h"

/***************************************************************************\
*                                                                           *
*   Module Name:        swap_NV_INT32                                       *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       July 1992                                           *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function swaps bytes in a four byte int.       *
*                                                                           *
*   Arguments:          word                -   pointer to the NV_INT32     *
*                                                                           *
\***************************************************************************/

void swap_NV_INT32 (NV_INT32 *word)
{
    NV_U_INT32    temp[4];

    temp[0] = *word & 0x000000ff;

    temp[1] = (*word & 0x0000ff00) >> 8;

    temp[2] = (*word & 0x00ff0000) >> 16;

    temp[3] = (*word & 0xff000000) >> 24;

    *word = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];
}



/***************************************************************************\
*                                                                           *
*   Module Name:        swap_NV_FLOAT32                                     *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       July 1992                                           *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function swaps bytes in a four byte float.     *
*                                                                           *
*   Arguments:          word                -   pointer to the NV_FLOAT32   *
*                                                                           *
\***************************************************************************/

void swap_NV_FLOAT32 (NV_FLOAT32 *word)
{
    NV_U_INT32    temp[4];

    union
    {
        NV_U_INT32    iword;
        NV_FLOAT32    fword;
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



/***************************************************************************\
*                                                                           *
*   Module Name:        swap_NV_FLOAT64                                     *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function swaps bytes in an eight byte double.  *
*                                                                           *
*   Arguments:          word                -   pointer to the NV_FLOAT64   *
*                                                                           *
\***************************************************************************/

void swap_NV_FLOAT64 (NV_FLOAT64 *word)
{
    NV_INT32    i;
    NV_CHAR     temp;
    union
    {
        NV_CHAR     bytes[8];
        NV_FLOAT64  d;
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

void swap_NV_INT64 (NV_INT64 *word)
{
    union
    {
      NV_FLOAT64 d;
      NV_U_INT64 i;
    } eq;

    eq.i = *word;
    swap_NV_FLOAT64 (&eq.d);

    *word = eq.i;
}


/***************************************************************************\
*                                                                           *
*   Module Name:        swap_NV_INT16                                       *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 1996                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function swaps bytes in a two byte int.        *
*                                                                           *
*   Arguments:          word                -   pointer to the NV_INT16     *
*                                                                           *
\***************************************************************************/

void swap_NV_INT16 (NV_INT16 *word)
{
    NV_INT16   temp;

    swab (word, &temp, 2);

    *word = temp;

    return;
}
