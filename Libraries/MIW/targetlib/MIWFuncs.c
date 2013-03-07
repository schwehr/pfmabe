/**************************************************************************************************
*
* Module: MIWFuncs
*
* Functions:	
*
* Purpose: The functions in this module are specific functions for adding, editing, removing and/or
*          finding (making current) nodes of the MIWTarget Schema..
*
* History:
*	08/2003 - RWL - Inital Development
***************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>


#include "targetlib.h"
#include "MIW.h"
#include "MIW_version.h"



/***************************************************************************************************

  Function:   getMIWVersion

  Purpose:    Return the MIW library version.

****************************************************************************************************/

char *getMIWVersion ()
{
  static char version[100];

  strcpy (version, MIW_VERSION);

  return (version);
}



/***************************************************************************************************
*
* Function AddTarget
*
* Purpose:  This function adds a Target node to the root element.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*         contactID - string used to Identify the contact.
*                     This is determined by a MOU with COMMINEWARCOM.
*
* Outputs 
*
* Returns status - 0 if successful
*                  -1 - XML Engine not initialized
*                  -2 - DOcument not ready.
*
* Method :
*
*
****************************************************************************************************/

int AddTarget(CNCTDocumentHandle docHandle, const char *contactId)
{
  int rtnval = 0;

  /*see if XML initialized */

  if (!IsXMLInitialized()) return -1;

	
  /*see if valid document handle */
	
  if (!IsValidDoc(docHandle)) return -2;


  /* Add the node to the root element */

  rtnval = AddNodeToRoot(docHandle,"Target");


  /*   Set the attribute Contacid */

  rtnval = SetAttribute(docHandle, "contactId",contactId);

  return rtnval;
}



/***************************************************************************************************
*
* Function FindTarget
*
* Purpose:  This finds a Target Node with the given Contact ID.
*           If found it is made the current node in the tree.  Else the current node is left unchanged.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*         contactID - string used to Identify the contact.
*                     This is determined by a MOU with COMMINEWARCOM.
*
* Outputs:
*
* Returns: 0 - Not Found
*          1 - Found
*
* Method : 
*
****************************************************************************************************/

int FindTarget(CNCTDocumentHandle docHandle, const char *contactId)
{
  return FindNode(docHandle, "Target", "contactId", contactId);
}



/***************************************************************************************************
*
* Function AddElement
*
* Purpose:  This function adds a element node in the node tree named by tag name.  
*           That element then become the new current element.
*           NOTE: This function is implemented as a rudamentary schema validation capability.
*                 The XERCES C++ API has not implemented the Abstract Schema part of the Abstract 
*                 Schema Load And Save W3C Specification as of the building of this library.
*                 Once that implementation is complete, this will become a pass thru to the AddNode function
*                 only.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*         tag - Tag name to name the element 
*
* Outputs none
*
* Returns:  1 - Node added
*           Error Conditions
*          -1 - Can not add node - SCHEMA violation
*          -2 - Can not add node, XML problem
*          -3 - No name for tag
*          -4 - Current Element is not defined in MIW Schema
*
* Method : 
*
****************************************************************************************************/

int AddElement(CNCTDocumentHandle docHandle, const char *tag)
{
  char *curElement=NULL;
  int rtnval = 0;


  /* Get current Node */

  curElement = GetCurrentElementType(docHandle);


  if (tag==NULL) return -3;


  if (curElement== NULL) return -4;


  if (strcmp(curElement,"Targets")==0)
    {
      if (strcmp(tag,"Target")!=0)
	{
	  rtnval = -1;
	}
      else
	{
	  rtnval =  AddNode(docHandle, tag);
	}
    }

  if (strcmp(curElement,"Target")==0)
    {
      if (strcmp(tag,"Object")!=0)
	{
	  rtnval =  -1;
	}
      else
	{
	  rtnval =  AddNode(docHandle, tag);
	}
    }

  if (strcmp(curElement,"Object")==0)
    {
      if (strcmp(tag,"Image")!=0 && strcmp(tag,"Environment")!=0 && strcmp(tag,"Platform")!=0 && strcmp(tag,"Sonar")!=0)
	{
	  rtnval =  -1;
	}
      else
	{
	  rtnval =  AddNode(docHandle, tag);
	}
    }

  if (strcmp(curElement,"Environment")==0)
    {
      if (strcmp(tag,"SSP_Level")!=0)
	{
	  rtnval =  -1;
	}
      else
	{
	  rtnval =  AddNode(docHandle, tag);
	}
    }


  /*if we got here then unknown curelemnt */

  return rtnval;
}



void SetFloatAttribute (CNCTDocumentHandle docHandle, const char *tag, float attr)
{
  char        string[50];


  sprintf (string, "%f", attr);
  SetAttribute (docHandle, tag, string);
}



void SetDoubleAttribute (CNCTDocumentHandle docHandle, const char *tag, double attr)
{
  char        string[50];


  sprintf (string, "%.9f", attr);
  SetAttribute (docHandle, tag, string);
}



void SetIntAttribute (CNCTDocumentHandle docHandle, const char *tag, int attr)
{
  char        string[50];


  sprintf (string, "%d", attr);
  SetAttribute (docHandle, tag, string);
}



/***************************************************************************************************
*
* Function SetRepresentation
*
* Purpose:  This function sets the attributes and vlaue of a  representation node in the 
*           node tree.  It checks to see if current element is Representation, if not, if it is an Object, the 
*           the representation element is set under it.  
*           Note the Representation element then become the new current element.
*           NOTE: This function is implemented as a rudamentary schema validation capability.
*                 The XERCES C++ API has not implemented the Abstract Schema part of the Abstract 
*                 Schema Load And Save W3C Specification as of the building of this library.
*                 Once that implementation is complete, this will become a pass thru to the AddNode function
*                 only.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*         rep - Representation structure to be added. 
*
* Outputs none
*
* Returns:  1 - Node added
*           Error Conditions
*          -1 - Can not add node - SCHEMA violation
*          -2 - Can not add node, XML problem
*          -4 - Current Element is not defined in MIW Schema
*
* Method : 
*          Get current element
*          if Object, then
*             if Repreentation exist move to it
*             else Create representation and move to it
*          if not Representation return error
*          Set Attributes
*          set node value
*             
****************************************************************************************************/

int SetRepresentation(CNCTDocumentHandle docHandle, REPRESENTATION *rep)
{
  char *curElement=NULL;
  int rtnval = 0;
  int i=0;
  char *temp;


  /* Get current Node */

  curElement = GetCurrentElementType(docHandle);


  if (curElement== NULL) return -4;


  if (strcmp(curElement,"Object")!=0)
    {
      rtnval =  -1;
    }
  else
    { 
      /* check to see if representation exists.  if not add else edit */

      rtnval = FindChildNode(docHandle,"Representation",NULL,NULL);  
      if (rtnval == -3) 
	{	      
	  rtnval = AddNode(docHandle, "Representation");


	  /* could not add Representation */

	  if (rtnval < 1) return rtnval;
	}


      /* if we get here the Representation is current node */


      /* Error occured finding Representation */

      if (rtnval <1) return rtnval;


      /* if we got here then Representation is current NODE */
      /* Set attributes */

      SetIntAttribute(docHandle, "representationType", rep->type);
      SetIntAttribute(docHandle, "count", rep->numPoints);


      /* create string for tuples */

      /*  We need to allocate the memory for the string since we have no idea how many points we may have.
	  It should take 11 characters for latitude, 12 characters for longitude, and a space so I'm going to
	  allocate 24 characters per point.  */

      temp = (char *) calloc (rep->numPoints, 24 * sizeof (char));
      if (temp == NULL)
	{
	  perror ("Allocating temp in setRepresentation");
	  exit (-1);
	}

      sprintf(temp, "%0.7lf %0.7lf", rep->lat[0], rep->lon[0]);

      for (i=1;i<rep->numPoints;i++) sprintf(temp, "%s %0.7lf %0.7lf", temp, rep->lat[i], rep->lon[i]);

      rtnval = SetNodeValue(docHandle,temp);

      free (temp);


      /*couldn't set value so remove node */

      if (rtnval < 1)
	{
	  RemoveNode(docHandle);
	  return rtnval;
	}
    }

   return 1;
};



/***************************************************************************************************
*
* Function GetRepresentation
*
* Purpose:  This function gets the attributes and vlaue of a  representation node in the 
*           node tree.  It checks to see if current element is Representation, if not, if it is an Object, the 
*           the first representation element descen dant is read.  
*           Note the Representation element then become the new current element.
*           NOTE: This function is implemented as a rudamentary schema validation capability.
*                 The XERCES C++ API has not implemented the Abstract Schema part of the Abstract 
*                 Schema Load And Save W3C Specification as of the building of this library.
*                 Once that implementation is complete, this will become a pass thru to the AddNode function
*                 only.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*         rep - Representation structure to be added. 
*
* Outputs rep - Structure containing the values in the DOM tree for
*               the representation
*
* Returns:  1 - Representation found and read
*           Error Conditions
*          -1 - No representation exist
*          -2 - Can not readn attributes and or Node value
*
* Method : 
*          Get current element
*          if Object, then
*             if Repreentation exist move to it
*             else return no Representation exist
*          Set Attributes
*          set node value
*             
****************************************************************************************************/
int GetRepresentation(CNCTDocumentHandle docHandle, REPRESENTATION *rep)
{
  char *curElement=NULL;
  int rtnval = 0;
  int i=0, count;
  char *temp = NULL, val[32], *ptrTemp;


  /* Get current Node */

  curElement = GetCurrentElementType(docHandle);


  if (strcmp(curElement,"Object")!=0 && strcmp(curElement,"Representation")!=0)
    {
      rtnval =  -1;
    }
  else
    { 
      if (strcmp(curElement,"Object")==0)
	{
	  /* check to see if representation exist.  if not add else edit */

	  rtnval = FindChildNode(docHandle,"Representation",NULL,NULL);  


	  /* no Representation */

	  if (rtnval == -3) return -1;
	}


      /* if we get here the Representation is current node */
      /* Get attributes */

      if ((rtnval = GetAttribute(docHandle, "representationType", val))>0)
	{
	  rep->type=atoi(val);
	}
      else
	{
	  return rtnval;
	}


      if ((rtnval = GetAttribute(docHandle, "count", val)) > 0)
	{
	  rep->numPoints=atoi(val);
	}
      else
	{
	  return rtnval;
	}


      /*  We need to allocate the memory for the string since we have no idea how many points we may have.
	  It should take 11 characters for latitude, 12 characters for longitude, and a space so I'm going to
	  allocate 24 characters per point.  */

      temp = (char *) calloc (rep->numPoints, 24 * sizeof (char));
      if (temp == NULL)
	{
	  perror ("Allocating temp in getRepresentation");
	  exit (-1);
	}


      /* Get string for tuples */

      rtnval = GetNodeValue(docHandle,temp);


      /*couldn't get value so return with error */

      if (rtnval < 1)
	{
	  free (temp);
	  return rtnval;
	}


      i=0;
      count=0;

      ptrTemp = strtok(temp," ");
      while (ptrTemp!= NULL)
	{
	  if (i % 2 == 0)
	    {
	      if (count==0)
		{
		  rep->lat = calloc(1, sizeof(double));
		}
	      else
		{
		  rep->lat = realloc(rep->lat,(count+1) * sizeof(double));
		}

              if (!rep->lat)
		{
		  free (temp);
		  return -1;
		}

	      rep->lat[count] = atof(ptrTemp);
	    }
	  else
	    {
	      if (count==0)
		{
		  rep->lon = calloc(1, sizeof(double));
		}
	      else
		{
		  rep->lon = realloc(rep->lon,(count+1) * sizeof(double));
		}

              if (!rep->lon)
		{
		  free (temp);
		  return -1;
		}

	      rep->lon[count++] = atof(ptrTemp);
	    }
	  i++;
	  ptrTemp=strtok(NULL," ");
	}
    }

  free (temp);
  return 1;
};



/*****************************************************************************
*
* Function: ReadAllShortTargets
*
* Purpose:  This reads all targets in the XML file, allocates an array of
*           SHORT_TARGET structures, and populates them.  NOTE: The caller is
*           responsible for freeing the array when finished.
*
* Inputs:   CNCTDocHandle docHandle  - Handle for the XML document
*           SHORT_TARGET *target[]   - Pointer to array of SHORT_TARGETs
*
* Returns:  count of targets in file
*
*****************************************************************************/

int ReadAllShortTargets (CNCTDocumentHandle docHandle, SHORT_TARGET *target[])
{
  int    target_count, itmp;
  double dtmp;
  float  ftmp;
  char   string[100];

  target_count = 0;


  MoveRoot (docHandle);

  if (MoveFirstChild (docHandle) == 1)
    {
      do
        {
          *target = (SHORT_TARGET *) realloc (*target, (target_count + 1) * sizeof (SHORT_TARGET));

          GetAttribute (docHandle, "contactId", (*target)[target_count].id);

          MoveFirstChild (docHandle);

          GetAttribute (docHandle, "latitude", string);
          sscanf (string, "%lf", &dtmp);
          (*target)[target_count].lat = dtmp;

          GetAttribute (docHandle, "longitude", string);
          sscanf (string, "%lf", &dtmp);
          (*target)[target_count].lon = dtmp;

          GetAttribute (docHandle, "depth", string);
          sscanf (string, "%f", &ftmp);
          (*target)[target_count].depth = ftmp;

          GetAttribute (docHandle, "confidenceLevel", string);
          sscanf (string, "%d", &itmp);
          (*target)[target_count].conf = itmp;

          GetAttribute (docHandle, "reasonForUpdate", string);
          sscanf (string, "%d", &itmp);
          (*target)[target_count].reason = itmp;

          target_count++;

          MoveUp (docHandle);
        } while (MoveNextSibling (docHandle) == 1);
    }

  return (target_count);
}



/*****************************************************************************
*
* Function: ReadAllMediumTargets
*
* Purpose:  This reads all targets in the XML file, allocates an array of
*           MEDIUM_TARGET structures, and populates them.  NOTE: The caller is
*           responsible for freeing the array when finished.
*
* Inputs:   CNCTDocHandle docHandle  - Handle for the XML document
*           MEDIUM_TARGET *target[]   - Pointer to array of MEDIUM_TARGETs
*
* Returns:  count of targets in file
*
*****************************************************************************/

int ReadAllMediumTargets (CNCTDocumentHandle docHandle, MEDIUM_TARGET *target[])
{
  int    target_count, itmp;
  double dtmp;
  float  ftmp;
  char   string[100];

  target_count = 0;


  MoveRoot (docHandle);

  if (MoveFirstChild (docHandle) == 1)
    {
      do
        {
          *target = (MEDIUM_TARGET *) realloc (*target, (target_count + 1) * sizeof (MEDIUM_TARGET));

          GetAttribute (docHandle, "contactId", (*target)[target_count].id);

          MoveFirstChild (docHandle);

          GetAttribute (docHandle, "latitude", string);
          sscanf (string, "%lf", &dtmp);
          (*target)[target_count].lat = dtmp;

          GetAttribute (docHandle, "longitude", string);
          sscanf (string, "%lf", &dtmp);
          (*target)[target_count].lon = dtmp;

          GetAttribute (docHandle, "depth", string);
          sscanf (string, "%f", &ftmp);
          (*target)[target_count].depth = ftmp;

          GetAttribute (docHandle, "confidenceLevel", string);
          sscanf (string, "%d", &itmp);
          (*target)[target_count].conf = itmp;

          GetAttribute (docHandle, "reasonForUpdate", string);
          sscanf (string, "%d", &itmp);
          (*target)[target_count].reason = itmp;

          strcpy ((*target)[target_count].description, "");
          GetAttribute (docHandle, "description", (*target)[target_count].description);

          strcpy ((*target)[target_count].remarks, "");
          GetAttribute (docHandle, "remarks", (*target)[target_count].remarks);

          target_count++;

          MoveUp (docHandle);
        } while (MoveNextSibling (docHandle) == 1);
    }

  return (target_count);
}



/****************************************************************************************
 *
 * About this module:  
 *      Addendum to the libtarget library.  Need to add this routine
 *      to have a consistent function call to fill in ALL the necessary info
 *      from a contact/target file (far beyond the limited fields of a SHORT_TARGET)
 *
 * Webb McDonald -- Wed Nov 10 18:57:06 2004
 ****************************************************************************************/

/****************************************************************************************
 *
 * Function InitializeLongTarget
 *
 * Purpose: This function initializes the LONG_TARGET structure.  
 *
 * Inputs: tgt - Pointer to a LONG_TARGET. See MIW.h for details.
 *
 * Outputs: tgt - LONG_TARGET with all elements initialized.
 *
 * Return:
 *
 * Method: 
 *
 ****************************************************************************************/

void InitializeLongTarget (LONG_TARGET *tgt)
{
    /*Initialize target's <Object> element vars*/
    tgt->conf = 0.0;
    tgt->height = 0.0;
    tgt->length = 0.0;
    tgt->width = 0.0;
    tgt->depth = 0.0;
    tgt->horiz_orientation = 0.0;
    tgt->vert_orientation = 0.0;
    strcpy (tgt->description, "");
    strcpy (tgt->survey_type, "");
    strcpy (tgt->remarks, "");
    strcpy (tgt->analysis_activity, "");
    /*a default time: highnoon, January 1st, 1996*/
    strcpy (tgt->time, "1996-01-01T12:00:00");


    /*Initialize <Image> element vars */
    tgt->image.scanline_row = 0;
    tgt->image.scanline_col = 0;
    tgt->image.scanline_lat = 0.0;
    tgt->image.scanline_lon = 0.0;
    /* just default to NO IMAGE*/
    strcpy (tgt->image.snippet_name, "NO IMAGE");
    strcpy (tgt->image.scanline_name, "NO IMAGE");

    /*Initialize <Platform> element vars */
    tgt->platform.lat = 0.0;
    tgt->platform.lon = 0.0;
    tgt->platform.heading = 0.0;
    strcpy (tgt->platform.type, "");
    strcpy (tgt->platform.nav_sys, "");

    /*Initialize <Sonar> element vars */
    tgt->sonar.lat = 0.0;
    tgt->sonar.lon = 0.0;
    tgt->sonar.frequency = 0.0;
    tgt->sonar.pulse_length = 0.0;
    strcpy (tgt->sonar.name, "");
    strcpy (tgt->sonar.type, "");

}/*Initialize(LONG_TARGET*)*/



/*****************************************************************************
 *
 * Function ReadAllLongTargets
 *
 * Purpose:  This reads all targets in the XML file, allocates an array of
 *           LONG_TARGET structures, and populates them.  NOTE: The caller is
 *           responsible for freeing the array when finished.
 *
 * Inputs: CNCTDocHandle docHandle  - Handle for the XML document
 *         LONG_TARGET *target[]   - Pointer to array of LONG_TARGETs
 *
 * Outputs: LONG_TARGET *target[]   - Pointer to array of LONG_TARGETs
 *
 * Returns:  Count of targets in the document.
 *
 * Method:
 *
 *****************************************************************************/

int ReadAllLongTargets (CNCTDocumentHandle docHandle, LONG_TARGET *target[])
{
    /* These tmps are sufficient to prevent memory overflow? */
    /* what is their point?  leaving them for now */
    int    target_count, itmp;
    double dtmp;
    float  ftmp;
    char   *curElem, string[512];    


    target_count = 0;

    MoveRoot (docHandle);

    if (MoveFirstChild (docHandle) == 1)
    {
        do
        {
            *target = (LONG_TARGET *) realloc (*target, 
                                               (target_count + 1) * sizeof (LONG_TARGET));
            InitializeLongTarget ( &((*target)[target_count]) );

            /*Get stuff from the "Object" level in this block*/
            GetAttribute (docHandle, "contactId", (*target)[target_count].id);

            MoveFirstChild (docHandle);

            GetAttribute (docHandle, "latitude", string);
            sscanf (string, "%lf", &dtmp);
            (*target)[target_count].lat = dtmp;
            GetAttribute (docHandle, "longitude", string);
            sscanf (string, "%lf", &dtmp);
            (*target)[target_count].lon = dtmp;
            GetAttribute (docHandle, "depth", string);
            sscanf (string, "%f", &ftmp);
            (*target)[target_count].depth = ftmp;
            GetAttribute (docHandle, "confidenceLevel", string);
            sscanf (string, "%d", &itmp);
            (*target)[target_count].conf = itmp;
            GetAttribute (docHandle, "surveyType", 
                          (*target)[target_count].survey_type);
            GetAttribute (docHandle, "analystActivity", 
                          (*target)[target_count].analysis_activity);
            GetAttribute (docHandle, "eventDTG", (*target)[target_count].time);
            GetAttribute (docHandle, "height", string);
            sscanf (string, "%f", &((*target)[target_count].height));
            GetAttribute (docHandle, "length", string);
            sscanf (string, "%f", &((*target)[target_count].length));
            GetAttribute (docHandle, "width", string);
            sscanf (string, "%f", &((*target)[target_count].width));
            GetAttribute (docHandle, "horizontalOrientation", string);
            sscanf (string, "%f", &((*target)[target_count].horiz_orientation));
            GetAttribute (docHandle, "verticalOrientation", string);
            sscanf (string, "%f", &((*target)[target_count].vert_orientation));
            GetAttribute (docHandle, "description", (*target)[target_count].description);
            GetAttribute (docHandle, "remarks", (*target)[target_count].remarks);

            /* We get the subelements' infos next- */
            MoveFirstChild (docHandle);
            while ((curElem = GetCurrentElementType (docHandle)) != NULL)
            {
                if (!strcmp (curElem, "Image"))
                {
                    GetAttribute (docHandle, "snippetFileName", 
                                  (*target)[target_count].image.snippet_name);
                    GetAttribute (docHandle, "UNISIPSName", 
                                  (*target)[target_count].image.scanline_name);
                    GetAttribute (docHandle, "scanlineRow", string);
                    sscanf (string, "%d", &((*target)[target_count].image.scanline_row));
                    GetAttribute (docHandle, "scanlineCol", string);
                    sscanf (string, "%d", &((*target)[target_count].image.scanline_col));
                    GetAttribute (docHandle, "scanlineLatitude", string);
                    sscanf (string, "%lf", &((*target)[target_count].image.scanline_lat));
                    GetAttribute (docHandle, "scanlineLongitude", string);
                    sscanf (string, "%lf", &((*target)[target_count].image.scanline_lon));
                }

                if (!strcmp (curElem, "Sonar"))
                {
                    GetAttribute (docHandle, "pulseLength", string);
                    sscanf (string, "%f", &((*target)[target_count].sonar.pulse_length));
                    GetAttribute (docHandle, "frequency", string);
                    sscanf (string, "%f", &((*target)[target_count].sonar.frequency));
                    GetAttribute (docHandle, "latitude", string);
                    sscanf (string, "%lf", &((*target)[target_count].sonar.lat));
                    GetAttribute (docHandle, "longitude", string);
                    sscanf (string, "%lf", &((*target)[target_count].sonar.lon));
                    GetAttribute (docHandle, "name", (*target)[target_count].sonar.name);
                    GetAttribute (docHandle, "type", (*target)[target_count].sonar.type);
                }

                if (!strcmp (curElem, "Platform"))
                {
                    GetAttribute (docHandle, "latitude", string);
                    sscanf (string, "%lf", &((*target)[target_count].platform.lat));
                    GetAttribute (docHandle, "longitude", string);
                    sscanf (string, "%lf", &((*target)[target_count].platform.lon));
                    GetAttribute (docHandle, "heading", string);
                    sscanf (string, "%lf", &((*target)[target_count].platform.heading));
                    GetAttribute (docHandle, "platformType", (*target)[target_count].platform.type);
                    GetAttribute (docHandle, "navSystem", (*target)[target_count].platform.nav_sys);

                }

                if (MoveNextSibling (docHandle) == -1) 
                    break;

            }/*while*/

            target_count++;

            /*return to <Target> level of xml doc*/
            MoveUp (docHandle);
            MoveUp (docHandle);
        } while (MoveNextSibling (docHandle) == 1);

    }/*if movechild*/

    return (target_count);

}/*ReadAllTargets()*/
