
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



/*  
    This defines the gsf beam flags as used by NAVO.  
    JCD - 02/01/02  
*/

#ifndef     _BEAM_FLAGS_
    #define _BEAM_FLAGS_

    /* The eight bit beam flag value is divided into two four bit
     * fields. The lower order four bits are used to specify that
     * a beam be ignored, where the value specifies the reason the
     * the beam is to be ignored. The higher order four bits are
     * usedto specify that a beam is selected, where the value specifies
     * the reason why the beam is selected.
     *
     *------------------------------------------------------------
     *
     * Category 00 (HMPS_INFORMATIONAL)
     * xxx1xx00 => This beam does not meet one times IHO accuracy
     *
     * Category 01 (HMPS_IGNORE)
     * 00000001 => This beam is to be ignored, no detection was made by the sonar
     * xxxxx101 => This beam is to be ignored, it has been manually edited
     * xxxx1x01 => This beam is to be ignored, it has been filter edited
     * xx1xxx01 => This beam is to be ignored, since it exceeds two times IHO
     * x1xxxx01 => This beam is to be ignored, since it exceeds the maximum footprint
     * 1xxxxx00 => Spare bit field
     *
     * Category 10 (HMPS_SELECT)
     * 00000010 => Selected sounding, no reason specified
     * xxxxx110 => Selected sounding, it is a least depth
     * xxxx1x10 => Selected sounding, it is a maximum depth
     * xxx1xx10 => Selected sounding, average depth
     * xx1xxx10 => Selected sounding, it has been identified as a side scan contact
     * x1xxxx10 => Selected sounding, spare ....
     * 1xxxxx10 => Selected sounding, spare ....
     */

     /* Definitions for the categories */
     #define HMPS_INFORMATIONAL      1
     #define HMPS_IGNORE	     2
     #define HMPS_SELECT	     3

     /* Definitions for HMPS_INFORMATIONAL category */
     #define HMPS_INFO_NOT_1X_IHO	    0x10
     #define HMPS_INFO_BIT_MASK 	    0x10

     /* Definitions for HMPS_IGNORE category */
     #define HMPS_IGNORE_NULL_BEAM	    0x01  /* 0x00 & 0x01 */
     #define HMPS_IGNORE_MANUALLY_EDITED    0x05  /* 0x04 & 0x01 */
     #define HMPS_IGNORE_FILTER_EDITED	    0x09  /* 0x08 & 0x01 */
     #define HMPS_IGNORE_NOT_2X_IHO	    0x21  /* 0x20 & 0x01 */
     #define HMPS_IGNORE_FOOTPRINT_TOO_BIG  0x41  /* 0x40 & 0x01 */
     #define HMPS_SPARE_1		    0x80
     #define HMPS_IGNORE_BIT_MASK	    0xec  /* combination of reason bits */

     /* Definitions for the HMPS_SELECT category */
     #define HMPS_SELECTED		    0x02  /* 0x00 & 0x02 */
     #define HMPS_SELECTED_LEAST	    0x06  /* 0x04 & 0x02 */
     #define HMPS_SELECTED_MAXIMUM	    0x0a  /* 0x08 & 0x02 */
     #define HMPS_SELECTED_AVERAGE	    0x12  /* 0x10 & 0x02 */
     #define HMPS_SELECTED_CONTACT	    0x22  /* 0x20 & 0x02 */
     #define HMPS_SELECTED_SPARE_1	    0x42  /* 0x40 & 0x02 */
     #define HMPS_SELECTED_SPARE_2	    0x82  /* 0x80 & 0x02 */

#endif
