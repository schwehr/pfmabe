#ifndef __MIW_H__
#define __MIW_H__

/* ****************************************************************************************
*
* File: MIW.h 
*
* Purpose: The include for definitions and types and prototypes for interfacing to C (not c++)
*          This include is specific to the MIWTarget Schema.
*
* History: 
*  08/2003 - RWL Development
*
*******************************************************************************************/

/* Prototypes */

/* if linking from C compiler then map C naming conventions to C++ (library source) */

#ifdef __cplusplus
extern "C"
{
#endif


#define MIW_SECONDARY_OBJECT        0x40000000   /*  Bit 30 set  */


  typedef struct
  {
    double          *lat;
    double          *lon;
    int             numPoints;  /* Number of lat/lon tuples in the list */
    int             type;       /* Code for the type of feature 
				   0 - line (last and first not connected
			           1 - polygon (last and first will be connected */
  } REPRESENTATION;
	
typedef struct
{
    char            snippet_name[512];
    char            scanline_name[512];
    int             scanline_row;
    int             scanline_col;
    double          scanline_lat;
    double          scanline_lon;
    
} IMAGE_TARGET_ELEMENT;

typedef struct
{
    double          lat;
    double          lon;
    float           pulse_length;
    float           frequency;
    char            name[512];
    char            type[32];

} SONAR_TARGET_ELEMENT;

typedef struct
{
    double          lat;
    double          lon;
    double          heading;
    char            type[32];
    char            nav_sys[512];

} PLATFORM_TARGET_ELEMENT;

typedef struct
{
    /*these are all the attributes of the <Object> element*/
    double          lat;
    double          lon;
    float           depth;
    float           height;
    float           length;
    float           width;
    float           horiz_orientation;
    float           vert_orientation;
    char            conf;
    char            id[15];
    char            description[512];
    char            remarks[512];
    char            analysis_activity[512];
    char            survey_type[32];
    char            time[64];

    /*these are child elements of a <Target> element's <Object>*/
    IMAGE_TARGET_ELEMENT     image;
    SONAR_TARGET_ELEMENT     sonar;
    PLATFORM_TARGET_ELEMENT  platform;
    
} LONG_TARGET;


  typedef struct
  {
    double          lat;
    double          lon;
    float           depth;
    char            conf;
    char            id[15];
    char            UNISIPSName[257];
    int             row;
    int             col;
    int             reason;            /*  Bit 30 is set if this is a secondary object of a target with many objects.  */
  } SHORT_TARGET;


  typedef struct
  {
    double          lat;
    double          lon;
    float           depth;
    char            conf;
    char            id[15];
    char            UNISIPSName[257];
    int             row;
    int             col;
    char            description[100];
    char            remarks[100];
    int             reason;            /*  Bit 30 is set if this is a secondary object of a target with many objects.  */
  } MEDIUM_TARGET;



  char *getMIWVersion ();

  int AddTarget(CNCTDocumentHandle handle, const char *contactid);
  int FindTarget(CNCTDocumentHandle handle, const char *contactid);
  int RemoveTarget(CNCTDocumentHandle handle, const char *contactid);

  int AddElement(CNCTDocumentHandle docHandle, const char *tag);

  void SetFloatAttribute (CNCTDocumentHandle docHandle, const char *tag, float attr);
  void SetDoubleAttribute (CNCTDocumentHandle docHandle, const char *tag, double attr);
  void SetIntAttribute (CNCTDocumentHandle docHandle, const char *tag, int attr);
  int ReadAllShortTargets (CNCTDocumentHandle docHandle, SHORT_TARGET *target[]);
  int ReadAllMediumTargets (CNCTDocumentHandle docHandle, MEDIUM_TARGET *target[]);
  int ReadAllLongTargets (CNCTDocumentHandle docHandle, LONG_TARGET *target[]);

  char *find_schema (const char*);

  int SetRepresentation(CNCTDocumentHandle docHandle, REPRESENTATION *rep);
  int GetRepresentation(CNCTDocumentHandle docHandle, REPRESENTATION *rep);

#ifdef __cplusplus
}
#endif

#endif
