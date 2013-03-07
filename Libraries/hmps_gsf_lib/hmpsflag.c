/********************************************************************
 *
 * Module Name : HMPSFLAG
 *
 * Author/Date : J. S. Byrne / 25 Apr 1995
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * DHG  2004/08/19 Added "setHMPSDelayedHeaveFlag" which uses "GSF_PING_USER_FLAG_12"
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

/* Get the necessary include files */
#include "gsf.h"
#include "hmpsflag.h"

/********************************************************************
 *
 * Function Name : testHMPSBeamFlag
 *
 * Description :
 *
 * Inputs :
 *
 * Returns : zero if this beam is not set to the flag, or
 *           1 if it is.
 *
 *
 * Error Conditions :
 *
 ********************************************************************/
int
testHMPSBeamFlag(gsfSwathBathyPing *p, int beam, int flag)
{
    /* Make sure the beam flags array is allocated */
    if (p->beam_flags == (unsigned char *) NULL)
    {
        return (0);
    }

    /* Make sure the beam number is valid */
    if ((beam >= p->number_beams) || (beam < 0))
    {
        return (0);
    }

    if (flag == HMPS_IGNORE_NULL_BEAM)
    {
        if ((p->beam_flags[beam] & flag) == flag)
        {
            /*
            if (!(p->beam_flags[beam] & ~flag)) return (1);
             */
            /* only test to make sure none of the reason bits are set.
             *  this way, a beam that has the HMPS_IGNORE_NULL_BEAM
             *  set and also has the info bit set, it will still
             *  be considered a null beam. bac, 06-04-02
             */
            if (!(p->beam_flags[beam] & HMPS_IGNORE_BIT_MASK)) return (1);
        }
    }
    else
    {
        if ((p->beam_flags[beam] & flag) == flag) return (1);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : setHMPSBeamFlag
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/
int
setHMPSBeamFlag (gsfSwathBathyPing *p, int beam, int flag)
{
    int category;

    /* Make sure the beam flags array is allocated */
    if (p->beam_flags == (unsigned char *) NULL)
    {
        return(-1);
    }

    /* Make sure the beam number is valid */
    if ((beam >= p->number_beams) || (beam < 0))
    {
        return(-1);
    }

    category = (flag & 0x03) + 1;
    switch (category)
    {
        case (HMPS_INFORMATIONAL):
            p->beam_flags[beam] |= flag;
            break;

        case (HMPS_IGNORE):
            /* If this sounding has been selected, clear the selection
             * flags, and then set the ignore flags.
             */
            if (p->beam_flags[beam] & HMPS_SELECTED)
            {
                p->beam_flags[beam] = 0;
            }
            p->beam_flags[beam] |= flag;
            break;

        case (HMPS_SELECT):
            /* The beam must not be set ignore, or have the informational
             * bit set to be selected.
             */
            if ((p->beam_flags[beam] & 0x01) != 1)
            {
                /* Set the appropiate bit mask */
                p->beam_flags[beam] |= flag;
            }
            else
            {
                return(-1);
            }
            break;
    }
    return(0);
}

/********************************************************************
 *
 * Function Name : clearHMPSBeamFlag
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/
int
clearHMPSBeamFlag (gsfSwathBathyPing *p, int beam, int category, int flag)
{
    unsigned char cMask;
    unsigned char reasons;
    unsigned char clear_bit;

    /* Make sure the beam flags array is allocated */
    if (p->beam_flags == (unsigned char *) NULL)
    {
        return(-1);
    }

    /* Make sure the beam number is valid */
    if ((beam >= p->number_beams) || (beam < 0))
    {
        return(-1);
    }

    /* switch on the category for this flag */
    switch (category)
    {
        case (HMPS_INFORMATIONAL):
            p->beam_flags[beam] &= ~flag;
            break;

        case (HMPS_IGNORE):
            /* Save the category mask, and check to see if more than one
             * of the ignore bits is set, if so don't clear the category
             * mask.
             */
            cMask = p->beam_flags[beam] & 0x01;
            if (cMask)
            {
                reasons = p->beam_flags[beam] & HMPS_IGNORE_BIT_MASK;
                clear_bit = flag & ~(0x01);
                reasons &= ~clear_bit;
                if (reasons)
                {
                    p->beam_flags[beam] &= ~flag;
                    p->beam_flags[beam] |= cMask;
                }
                else
                {
                    p->beam_flags[beam] &= ~flag;
                }
                /* If this is a null beam then leave the ignore flag set */
                if (p->depth[beam] == 0.0)
                {
                    setHMPSBeamFlag (p, beam, HMPS_IGNORE_NULL_BEAM);
                }
            }
            break;

        case (HMPS_SELECT):
            /* Save the category mask, and check to see if more than one
             * of the reason bits is set, if so don't clear the category
             * mask.
             */
            cMask = p->beam_flags[beam] & 0x02;
            if (cMask)
            {
                /* the HMPS_IGNORE_BIT_MASK can be used here also to check if any
                 *  of the selected sounding reason bits are set. bac, 03-23-04
                 */
                reasons = p->beam_flags[beam] & HMPS_IGNORE_BIT_MASK;
                clear_bit = flag & ~(0x02);
                reasons &= ~clear_bit;
                if (reasons)
                {
                    p->beam_flags[beam] &= ~flag;
                    p->beam_flags[beam] |= cMask;
                }
                else
                {
                    p->beam_flags[beam] &= ~flag;
                }
            }
            break;
    }
    return(0);
}

/********************************************************************
 *
 * Function Name : testHMPSTideFlag
 *
 * Description :  This function tests the tide bits in the
 *                ping_flags field of a ping structure.
 *
 * Inputs :     gsfSwathBathyPing p -- the ping structure whose ping_flags
 *                                     are to be tested
 *              int flag -- the type of tide corrector that the ping_flags
 *                          field should be tested against
 *
 * Returns :  1 -- the "flag" argument matched the tide corrector type
 *                 in the ping structure
 *            0 -- the "flag" argument did not match the type of tide
 *                 corrector in the ping structure
 *
 ********************************************************************/
int testHMPSTideFlag(gsfSwathBathyPing *p, int flag){
    switch (flag) {

        case (HMPS_UNKNOWN_TC):
            /* True only when both bits 14 and 15 are off. */
            return (!(p->ping_flags & GSF_PING_USER_FLAG_14)
                    && !(p->ping_flags & GSF_PING_USER_FLAG_15));

        case (HMPS_PREDICTED_TC):
            /* True only when bit 14 is on and bit 15 is off. */
            return ((p->ping_flags & GSF_PING_USER_FLAG_14) &&
                    !(p->ping_flags & GSF_PING_USER_FLAG_15));

        case (HMPS_OBSERVED_PRELIM_TC):
            /* True only when bit 14 is off and bit 15 is on. */
            return ( !(p->ping_flags & GSF_PING_USER_FLAG_14) &&
                    (p->ping_flags & GSF_PING_USER_FLAG_15));

        case (HMPS_OBSERVED_VERIFIED_TC):
            /* True only when both bits are on. */
            return ((p->ping_flags & GSF_PING_USER_FLAG_14) &&
                    (p->ping_flags & GSF_PING_USER_FLAG_15));

        case (HMPS_ERS_TC):
            /* True only when both bits 14 and 15 are off and bit 13 is set. */
            return (!(p->ping_flags & GSF_PING_USER_FLAG_14) &&
                    !(p->ping_flags & GSF_PING_USER_FLAG_15) &&
                     (p->ping_flags & GSF_PING_USER_FLAG_13));

      default:
            return (0);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : setHMPSTideFlag
 *
 * Description :  This function sets the tide flag bits in the
 *                ping_flags field of a ping structure.
 *
 * Inputs :     gsfSwathBathyPing p -- the ping structure whose ping_flags
 *                                     are to be set
 *              int flag -- the type of tide corrector that the ping_flags
 *                          field should be set to
 *
 * Returns :  0 -- success
 *           -1 -- error due to an unrecognized "flag" argument
 *
 ********************************************************************/
int setHMPSTideFlag(gsfSwathBathyPing *p, int flag){

    switch (flag) {
        case (HMPS_UNKNOWN_TC):
            p->ping_flags &= ~(GSF_PING_USER_FLAG_14 | GSF_PING_USER_FLAG_15); /* Clear both. */
            break;
        case (HMPS_PREDICTED_TC):
            p->ping_flags &= ~(GSF_PING_USER_FLAG_15);                         /* Clear bit 15 */
            p->ping_flags |= (GSF_PING_USER_FLAG_14) ;                         /* Set bit 14 */
            break;
        case (HMPS_OBSERVED_PRELIM_TC):
            p->ping_flags &= ~(GSF_PING_USER_FLAG_14);                         /* Clear bit 14 */
            p->ping_flags |= (GSF_PING_USER_FLAG_15) ;                         /* Set bit 15 */
            break;
        case (HMPS_OBSERVED_VERIFIED_TC):
            p->ping_flags |= (GSF_PING_USER_FLAG_14 | GSF_PING_USER_FLAG_15);  /* Set both. */
            break;
        case (HMPS_ERS_TC):
            p->ping_flags &= ~(GSF_PING_USER_FLAG_14 | GSF_PING_USER_FLAG_15); /* Clear both 14 and 15. */
            p->ping_flags |= (GSF_PING_USER_FLAG_13) ;                         /* Set bit 13 */
            break;
      default:
            return (-1);
    }
    return (0);
}



/********************************************************************
 *
 * Function Name : setHMPSDelayedHeaveFlag
 *
 * Description :  This function sets the true heave applied flag bit in the
 *                ping_flags field of a ping structure.
 *
 * Inputs :     gsfSwathBathyPing p -- the ping structure whose ping_flags
 *                                     are to be set
 *              int flag -- the true heave applied status that the ping_flags
 *                          field should be set to
 *
 * Returns :  0 -- success
 *           -1 -- error due to an unrecognized "flag" argument
 *
 ********************************************************************/
int setHMPSDelayedHeaveFlag(gsfSwathBathyPing *p, int flag){

    switch (flag) {
        case (0):
            p->ping_flags &= ~(GSF_PING_USER_FLAG_12);                         /* Clear bit 12 */
            break;
        case (1):
            p->ping_flags |= (GSF_PING_USER_FLAG_12) ;                         /* Set bit 12 */
            break;
        default:
            return (-1);
    }
    return (0);
}

/********************************************************************
 *
 * Function Name : setHMPS_GPSZFlag
 *
 * Description :  This function sets the GPS vertical control flag bit in the
 *                ping_flags field of a ping structure.
 *
 * Inputs :     gsfSwathBathyPing p -- the ping structure whose ping_flags
 *                                     are to be set
 *              int flag -- the GPS vertical control status that the ping_flags
 *                          field should be set to
 *
 * Returns :  0 -- success
 *           -1 -- error due to an unrecognized "flag" argument
 *
 ********************************************************************/
int setHMPS_GPSZFlag(gsfSwathBathyPing *p, int flag){

  switch (flag) {
    case (0):
      p->ping_flags &= ~(GSF_PING_USER_FLAG_13);                         /* Clear bit 13 */
      break;
    case (1):
      p->ping_flags |= (GSF_PING_USER_FLAG_13) ;                         /* Set bit 13 */
      break;
    default:
      return (-1);
  }
  return (0);
}

/********************************************************************
 *
 * Function Name : setHMPS_DSFlag
 *
 * Description :  This function sets the Designated Sounding flag bit in the
 *                ping_flags field of a ping structure.
 *
 * Inputs :     gsfSwathBathyPing p -- the ping structure whose beam_flags
 *                                     are to be set
 *              int beam -- the beam index of the beam that the flags should
 *                          be updated in.
 *              int flag -- the Designated Sounding status that the beam_flags
 *                          field should be set to
 *
 * Returns :  0 -- success
 *           -1 -- error due to an unrecognized "flag" argument
 *
 ********************************************************************/
int setHMPS_DSFlag(gsfSwathBathyPing *p, int beam, int flag){

  switch (flag) {
    case (0):
      p->beam_flags[beam] &= ~(HMPS_SELECTED_DESIGNATED);                         /* Clear bit 7 */
      break;
    case (1):
      p->beam_flags[beam] |= (HMPS_SELECTED_DESIGNATED) ;                         /* Set bit 7 */
      break;
    default:
      return (-1);
  }
  return (0);
}
