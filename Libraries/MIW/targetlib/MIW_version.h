
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef MIW_VERSION

#define     MIW_VERSION     "PFM Software - MIW library V1.10 - 04/30/10"

#endif


/*

    Version 1.00
    08/01/03
    Wade Ladner

    First version.


    Version 1.01
    05/20/08
    Jan C. Depner

    Fixed a plethora of static allocation problems in MIWFuncs.c.  Also, cleaned up the code in that one a bit.


    Version 1.02
    06/16/08
    Jan C. Depner

    Changed XMLinitialized and impl to MIW_XMLinitialized and MIW_impl because they conflicted
    with the BAG library.


    Version 1.03
    01/29/09
    Jan C. Depner

    Cleaned up code.  Added reasonForUpdate to short and medium targets.  Fixed OS dependencies in find_schema.c.


    Version 1.10
    04/30/10
    Jan C. Depner

    Replaced most occurrences of "char *" arguments with "const char *" arguments so that gcc 4.4 wouldn't freak out about
    passing character constants (i.e. "contactId").

*/
