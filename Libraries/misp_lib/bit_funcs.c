#include "nvutility.h"

/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        bit_set                                             *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Set the specified bit of a given floating-point     *
*                       value to one or zero.                               *
*                                                                           *
*   Inputs:             *value  -   floating point value                    *
*                       bit     -   bit to be set                           *
*                       binary  -   0 or 1                                  *
*                                                                           *
*   Outputs:            None                                                *
*                                                                           *
*   Calling Routines:   Utility routine                                     *
*                                                                           * 
*   Glossary:           data    -   union of an NV_INT32 and NV_FLOAT32     *
*                                                                           *
*   Method:             The floating point value input is stored in a       *
*                       floating point variable that is in a union with an  *
*                       integer.  The bit in the integer is set or unset    *
*                       and then the float from the union is placed in the  *
*                       input float variable.                               *
*                                                                           *
\***************************************************************************/
                                                                            
void bit_set (NV_FLOAT32 *value, NV_INT32 bit, NV_INT32 binary)
{
    union
    {
        NV_U_INT32           intvalue;
        NV_FLOAT32           fltvalue;
    } data;

    data.fltvalue = *value;
    if (binary)
    {
        data.intvalue |= binary << bit;
    }
    else
    {
        data.intvalue &= ~(1 << bit);
    }
    *value = data.fltvalue;

    return;
}


/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        bit_test                                            *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Test the specified bit of a given floating-point    *
*                       value.                                              *
*                                                                           *
*   Inputs:             value   -   floating point value                    *
*                       bit     -   bit to be tested                        *
*                                                                           *
*   Outputs:            NV_INT32     -   1 if bit is set, otherwise 0       *
*                                                                           *
*   Calling Routines:   Utility routine                                     *
*                                                                           * 
*   Glossary:           data    -   union of an NV_INT32 and NV_FLOAT32     *
*                                                                           *
*   Method:             The floating point value input is stored in a       *
*                       floating point variable that is in a union with an  *
*                       integer.  The bit in the integer is tested and the  *
*                       result is returned.                                 *
*                                                                           *
\***************************************************************************/
                                                                            
NV_INT32 bit_test (NV_FLOAT32 value, NV_INT32 bit)
{
    union
    {
        NV_U_INT32           intvalue;
        NV_FLOAT32           fltvalue;
    } data;

    data.fltvalue = value;

    return ((data.intvalue & (1 << bit)));
}
