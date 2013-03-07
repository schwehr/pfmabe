/*****************************************************************************
 *
 * File:	shoals_icon.h
 *
 * Purpose:	Contains some defs for ICON use in shoals and function
 *		prototypes
 *		 
 * Revision History:   
 * 98/08/18 DR	-wrote it
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


void attach_standard_icon( );	/* ( Frame frame, *window_text);
void attach_gg_auto_win_icon();	/* ( Frame frame); */
void attach_main_win_icon();	/* ( Frame frame); */
void attach_icon();		/* ( Frame frame, int icon_number); */


/* For use in call to attach_icon */
#define	 ICON_FL_WIN		0
#define	 ICON_MAP_WIN		1
#define	 ICON_POLYGON_WIN	2
#define	 ICON_REGION_WIN	3
#define	 ICON_ZOOM_WIN		4
#define	 ICON_LIMITS_WIN	5
#define	 ICON_WF_WIN		6
#define	 ICON_XYZ_MAP_WIN	7
#define	 ICON_STANDARD_WIN	8

#ifdef  __cplusplus
}
#endif
