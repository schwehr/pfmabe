#include "check_flag.h"


/********************************************************************
*
* Function Name :  check_flag
*
* Description :    Checks the specified HMPS flag in a GSF beam flag 
*                  field
*
* Inputs :         field    -   beam flag
*                  flag     -   HMPS flag
*
* Returns :        False if this field is not set to the flag, or
*                  True if it is.
*
********************************************************************/

unsigned char check_flag (unsigned char field, unsigned char flag)
{
  unsigned char        ret_value;


  ret_value = 0;

  if (flag == HMPS_IGNORE_NULL_BEAM)
    {
      if (!(field ^ flag)) ret_value = 1;
    }
  else if (flag == HMPS_INFO_NOT_1X_IHO)
    {
      if (field & flag) ret_value = 1;
    }
  else
    {
      if ((field & flag) == flag) ret_value = 1;
    }

  return (ret_value);
}



/********************************************************************
*
* Function Name :  set_flag
*
* Description :    Sets the specified HMPS flag in a GSF beam flag 
*                  field
*
* Inputs :         field    -   beam flag
*                  flag     -   HMPS flag
*
********************************************************************/

void set_flag (unsigned char *field, unsigned char flag)
{
  int               category;


  category = (flag & 0x03) + 1;

  switch (category)
    {
    case (HMPS_INFORMATIONAL):
      *field |= flag;

      /*  Clear the selected bit or it will get confused with HMPS_SELECTED_AVERAGE.  */

      /*  Blew this off because we're not using selected average anyway.   JCD  07/14/00  */

      /**field |= ~HMPS_SELECTED;*/
      break;

    case (HMPS_IGNORE):

      /*  If this sounding has been selected, clear the selection flags, and then set the ignore flags.  */

      if (*field & HMPS_SELECTED) *field = 0;

      *field |= flag;
      break;

    case (HMPS_SELECT):

      /*  The beam must not be set ignore, or have the informational bit set to be selected.  */

      if (((*field & 0x03) != 1) && (*field != HMPS_INFO_NOT_1X_IHO))
        {
          *field |= flag;
        }
      break;
    }
}



/********************************************************************
*
* Function Name :  clear_flag
*
* Description :    Clears the specified HMPS flag in a GSF beam flag 
*                  field
*
* Inputs :         field    -   beam flag
*                  flag     -   HMPS flag
*
********************************************************************/

void clear_flag (unsigned char *field, unsigned char flag)
{
  int                category;
  unsigned char      cMask, reasons, clear_bit;


  category = (flag & 0x03) + 1;


  /*  Switch on the category for this flag.  */

  switch (category)
    {
    case (HMPS_INFORMATIONAL):
      *field &= ~flag;
      break;

    case (HMPS_IGNORE):

      /*  Save the category mask, and check to see if more than one of the ignore bits is set, 
          if so don't clear the category mask.  */

      cMask = *field & 0x01;
      reasons = *field & HMPS_IGNORE_BIT_MASK;
      clear_bit = flag & ~(0x01);
      reasons &= ~clear_bit;
      if (reasons)
        {
          *field &= ~flag;
          *field |= cMask;
        }
      else
        {
          *field &= ~flag;
        }
      break;

    case (HMPS_SELECT):

      /*  Save the category mask, and check to see if more than one of the select bits is set, 
          if so don't clear the category mask.  */

      cMask = *field & 0x02;
      reasons = *field & 0xfc;
      clear_bit = flag & ~(0x02);
      reasons &= ~clear_bit;
      if (reasons)
        {
          *field &= ~flag;
          *field |= cMask;
        }
      else
        {
          *field &= ~flag;
        }
      break;
    }
}
