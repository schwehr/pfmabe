/***************************************************************************************************
*
* Module Initialize: 
*
* Purpose: This module continas the core C++ functions for interacing C applications with the XERCES
*          C++ API.  It interfaces to the DOM Level 3 Abstract Schema Load and Save Specification only.
*          This module does not contain any specific code for the Targets schema.
*
* History: 
*   08/2003 - RWL - Initial Build
*
******************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <wchar.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>


#include "CNCTErrorHandler.hpp"
#include "targetlib.hpp"

bool MIW_XMLinitialized=false;
DOMImplementation* MIW_impl;

/******************************************************************************************************
*
* Function: CNCTNewDoc
*
* Purpose: This function initializes the Xerces XML engine and creates a new DOM node tree for the
*          input schema name and create the root element.
*
* Inputs: fname - Filename to serialize the node tree to at the end.
*         schemaURI - The full, absolute path to the schema to which to validate the document.
*         rootElem - the Tag name for the Root ELement.
*
* Outputs: 
*
* Returns: CNCTDocHandle - Handle for the created XML document.
*                          if psoitive it is a handle for the created XML document.
*                          if negative, an error is returned.
*
*******************************************************************************************************/

CNCTDocumentHandle CNCTNewDoc (const char *fname, const char *schemaURI, const char *rootElem)
{
  // Initialize XERCES

  if (!MIW_XMLinitialized)
    {
      try
        {
          XMLPlatformUtils::Initialize();
        }
      catch(const XMLException& toCatch)
        {
          char *pMsg = XMLString::transcode(toCatch.getMessage());
          cerr << "Error during Xerces-c Initialization.\n"
               << "  Exception Message: "
               << pMsg;
          XMLString::release((XMLCh **)&pMsg);
          return -1;
        }

      /* Get an Implementation with Factory Setttings for both the Load and Save Methods .*/

      static const XMLCh gLS[] = {chLatin_L, chLatin_S, chNull };
      MIW_impl = DOMImplementationRegistry::getDOMImplementation(gLS);
      if (MIW_impl)
        {
          MIW_XMLinitialized=true;
        }
    }

  XMLCh *rootName = XMLString::transcode(rootElem);
  DOMDocument* doc = MIW_impl->createDocument(0,rootName,0);
  XMLString::release(&rootName);


  /* Add some elements in the document tree for later use */
  /* Setup document attributes */

  XMLCh *URIName = XMLString::transcode(fname);
  doc->setDocumentURI(URIName);
  XMLString::release(&URIName);

  XMLCh *temp = XMLString::transcode("UTF-8");
  doc->setEncoding(temp);
  XMLString::release(&temp);

  XMLCh *temp1 = XMLString::transcode("1.0");
  doc->setVersion(temp1);
  XMLString::release(&temp1);


  /* Add the document to the list open */
  CNCTdocuments[CNCTnumDocuments] =  new CNCTDocument;
  CNCTdocuments[CNCTnumDocuments]->setDoc(doc);
  CNCTnumDocuments++;
	
  /* Set current Node for furhter library processing */

  DOMElement* docrootElem = doc->getDocumentElement();
  CNCTdocuments[CNCTnumDocuments-1]->setCurrentNode(docrootElem);

  SetAttribute(CNCTnumDocuments, "xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");

  if (schemaURI) SetAttribute(CNCTnumDocuments, "xsi:noNamespaceSchemaLocation",schemaURI);

  CNCTdocuments[CNCTnumDocuments-1]->setDirty(true);

  return CNCTnumDocuments;
}

/******************************************************************************************************
*
* Function: TerminateXML
*
* Purpose: This function shutdown the Xerces XML engine releaseing all resources back to the system.
*
* Inputs: None
*
* Outputs: None 
*
* Returns: Status 1 - Successful
*                 -1 - Failed
*
*******************************************************************************************************/

int TerminateXML()
{
  // Stop XERCES

  if (!MIW_XMLinitialized)
    {
      return -1;
    }
  else
    {
      XMLPlatformUtils::Terminate();
      return 1;
    }
}

/******************************************************************************************************
*
* Function: CNCTEditDoc
*
* Purpose: This function initializes the Xerces XML engine and creates a new DOM node tree in
*          memory for the XML document specified by fname. It parses and loads an existing XML 
*          document into the DOM node tree.
*
* Inputs: fname - Filename to serialize the node tree to at the end.
*         validate - Flag used to determine whether or not to validate the document against
*         its schema.
*         0 - No Validation 
*         1 - Validate
*
* Outputs: 
*
* Returns: CNCTDocHandle - Handle for the created XML document.
*                          if psoitive it is a handle for the created XML document.
*                          if negative, an error is returned.
*
*******************************************************************************************************/

CNCTDocumentHandle CNCTEditDoc (const char *fname, int validate)
{
  DOMDocument *doc;
  CNCTDocument *myDoc;
  int fred;
	
  if ((fred = ParseDoc(fname, &doc, false, validate)) > 0)
    {
      DOMElement* rootElem = doc->getDocumentElement();

      /* Add the document to the list open */
      myDoc = new CNCTDocument;
      myDoc->setDoc(doc);
      myDoc->setCurrentNode(rootElem);
      CNCTdocuments[CNCTnumDocuments++]=myDoc;
      return CNCTnumDocuments;
    }
  else
    {
      fprintf(stderr,"%s %d Error %d\n",__FILE__,__LINE__,fred);
      return -1;
    }
}

/******************************************************************************************************
*
* Function: ParseDoc
*
* Purpose: This is an internal function to the library.  It passes pointers to objects that 
*          C can't handle.  
*
*          It initializes the XML engine and creates a new DOM node tree for the XML document.
*          It parses and loads an existing XML document into the DOM node tree.  The document is validated 
*          if requested.
*
* Inputs: fname - Filename to serialize the node tree to at the end.
*         parseOnly - Flag used to determine if the document is only parsed.  I.E. no node tree is returned.
*         validate - Flag used to determine whether or not the validate the document against
*                    its schema. (1 = Yes, 0 = No)
*
* Outputs: myDoc - The DOMDocument object or node tree.
*          Messages are also output to the screen.
*
* Returns: Status indicating the status of the Parse.
*              1 - Document Parsed. It is valid.
*             -1 - Document Parsed.  It is not valid for the schema contained.
*             -2 - A DOM Parser error occured.
*             -3 - A XML Parser error occured.
*
*******************************************************************************************************/

int ParseDoc(const char *fname, DOMDocument **myDoc, bool parseOnly, int validate)
{
  DOMDocument *doc = NULL;

  DOMBuilder *myParser;
  bool bFailed = false;
  int rtnval = 1;

  // Initialize XERCES

  if (!MIW_XMLinitialized)
    {
      try
        {
          XMLPlatformUtils::Initialize();
        }
      catch(const XMLException& toCatch)
        {
          char *pMsg = XMLString::transcode(toCatch.getMessage());
          cerr << "Error during Xerces-c Initialization.\n"
               << "  Exception Message: "
               << pMsg;
          XMLString::release(&pMsg);
          return -3;
        }

      /* Get an Implementation with Factory Setttings for both the Lad and Save Methods .*/

      static const XMLCh gLS[] = {chLatin_L, chLatin_S, chNull };
      MIW_impl = DOMImplementationRegistry::getDOMImplementation(gLS);
      if (MIW_impl)
        {
          MIW_XMLinitialized=true;
        }
    }

  myParser = ((DOMImplementationLS*)MIW_impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);

  myParser->setFeature(XMLUni::fgDOMNamespaces, true);
  myParser->setFeature(XMLUni::fgXercesSchema, true);
  myParser->setFeature(XMLUni::fgXercesUserAdoptsDOMDocument, true);
  if (validate)
    {
      myParser->setFeature(XMLUni::fgDOMValidation, true);
      myParser->setFeature(XMLUni::fgXercesSchemaFullChecking, true);
    }

  /* Setup the Error Handler */

  CNCTErrorHandler *errReporter = new CNCTErrorHandler;
  myParser->setErrorHandler(errReporter);

  /* Parse it */

  try
    {
      doc = myParser->parseURI(fname);
    }
  catch (const DOMException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.msg);
      cout << "Exception message is: \n"
           << message << "\n";
      XMLString::release(&message);
      bFailed=true;
      rtnval = -2;
    }
  catch (const XMLException& toCatch)
    {
      char *pMsg = XMLString::transcode(toCatch.getMessage());
      cerr << "Error during Xerces-c Initialization.\n"
           << "  Exception Message: "
           << pMsg;
      XMLString::release(&pMsg);
      bFailed=true;
      rtnval = -3;
    }
  catch (...)
    {
      bFailed=true;
      rtnval = -3;
    }
    
  /* make sure we caught all possibilities */

  if (errReporter->getSawErrors())
    {
      bFailed=true;
      rtnval = -1;
    }

  /* If nore errors occured then save the document if wanted */

  if (!bFailed)
    {
      if (parseOnly)
        {
          if (myDoc) *myDoc = NULL;
        }
      else
        {
          /* The parser owns the parsed document in memory.  We have to release it 
             or terminateXML will not work. */

          *myDoc = doc;
        }

      rtnval = 1;
    }
  else
    {
      if (myDoc) *myDoc = NULL;
    }

  /* release all resources for the parser */

  delete errReporter;
  myParser->release();

  return rtnval;
}


/******************************************************************************************************
*
* Function: CNCTWriteDoc
*
* Purpose: This function serializes a document to disk.  It will validate the document if desired.  If errors
*          are found, they are printed to stderr and a status flag is returned to the calling application.  
*
* Inputs: handle - The document handle for the document to serialize.
*         validate - Flag used to determine whether or not the validate the document against
*                    its schema. (1 = Yes, 0 = No)
*
* Outputs: Messages are also output to the screen.
*
* Returns: Status indicating the status of the Parse.
*              1 - Document Saved as requested. 
*             -1 - Document saved and Parsed.  It is not valid for the schema contained.
*             -2 - A DOM Parser error occured.
*             -3 - A XML Parser error occured.
*
*******************************************************************************************************/
int CNCTWriteDocToFile(CNCTDocumentHandle handle,int validate)
{
  XMLFormatTarget *myFormTarget;

  // Initialize XERCES

  if (!MIW_XMLinitialized)
    {
      try
        {
          XMLPlatformUtils::Initialize();
        }
      catch(const XMLException& toCatch)
        {
          char *pMsg = XMLString::transcode(toCatch.getMessage());
          cerr << "Error during Xerces-c Initialization.\n"
               << "  Exception Message: "
               << pMsg;
          XMLString::release(&pMsg);
          return -3;
        }

      /* Get an Implementation with Factory Setttings for both the Lad and Save Methods .*/

      static const XMLCh gLS[] = {chLatin_L, chLatin_S, chNull };
      MIW_impl = DOMImplementationRegistry::getDOMImplementation(gLS);
      if (MIW_impl)
        {
          MIW_XMLinitialized=true;
        }
    }

  /* Create the DOM Writer */

  DOMWriter* theSerializer = ((DOMImplementationLS*)MIW_impl)->createDOMWriter();

  /* Set some features */
	
  if (theSerializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true))
    theSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);

  if (validate && theSerializer->canSetFeature(XMLUni::fgDOMWRTValidation, true))
    theSerializer->setFeature(XMLUni::fgDOMWRTValidation, true);


  /* Install an error Handler */ 

  DOMErrorHandler* errHandler = new CNCTErrorHandler();
  theSerializer->setErrorHandler(errHandler);

  /* Set the local file output target */
  /* Check to see if a URI is specified.  If so take off protocol */

  DOMDocument *doc = CNCTdocuments[handle-1]->getDoc();
  char *fname =  XMLString::transcode(doc->getDocumentURI());
  char *pdest = strchr(fname,':');
  if (pdest != NULL)
    {
      pdest = pdest+3;  /* get rid of :// */

      /* if windows then strip of leading / and change all / to \ */

#ifdef WIN32

      if (pdest[0] == '/') pdest=pdest + 1;
      char newfname[256];
      memset(newfname,0,256);
      for (unsigned int i=0;i<strlen(pdest);i++)
        {
          if (pdest[i]=='/')
            {
              strcat(newfname,"\\");
            }
          else
            {
              newfname[strlen(newfname)] = pdest[i];
            }
        }
      pdest=newfname;
#endif

      myFormTarget = new LocalFileFormatTarget(XMLString::transcode(pdest)); 
    }
  else
    {
      myFormTarget = new LocalFileFormatTarget(doc->getDocumentURI());
    }
    

  try 
    {
      /* do the serialization */
      DOMNode *node = CNCTdocuments[handle -1]->getDoc();
      theSerializer->writeNode(myFormTarget, *node);
    }
  catch (const XMLException& toCatch) 
    {
      char* message = XMLString::transcode(toCatch.getMessage());
      cout << "Exception message is: \n"
           << message << "\n";
      XMLString::release(&message);
      return -3;
    }
  catch (const DOMException& toCatch) 
    {
      char* message = XMLString::transcode(toCatch.msg);
      cerr << "Exception message is: \n"
           << message << "\n";
      XMLString::release(&message);
      return -2;
    }
  catch (...)
    {
      /*       return -1; */
    }

  theSerializer->release();
  delete errHandler;
  delete myFormTarget;

  /* Note as of this writing, the DOMWriter does not validate.  So if the programmer wants
     parse the serialized doument for validation now */
   if (validate)
     {
       return ParseDoc(XMLString::transcode(doc->getDocumentURI()), NULL, true, validate);
     }
   else
     {
       return 1;
     }

   /*Document is serialized so turn off the Dirty Flag */

   CNCTdocuments[handle-1]->setDirty(false);
}


/******************************************************************************************************
*
* Function: CNCTSaveDocAs
*
* Purpose: This function serializes a document in memeory to a different file name.  It will validate th
*          document if desired.  If errors are found, they are printed to stderr and a status flag 
*          is returned to the calling application. Note, the URI of the document is changed to the new file name.  
*
* Inputs: handle - The document handle for the document to serialize.
*         fname - Filename to serialize the node tree.
*         validate - Flag used to determine whether or not the validate the document against
*                    its schema. (1 = Yes, 0 = No)
*
* Outputs: Messages are also output to the screen.
*
* Returns: Status indicating the status of the Parse.
*              1 - Document Saved as requested. 
*             -1 - Document saved and Parsed.  It is not valid for the schema contained.
*             -2 - A DOM Parser error occured.
*             -3 - A XML Parser error occured.
*
*******************************************************************************************************/
int CNCTSaveDocAs(CNCTDocumentHandle handle, const char *fname, int validate)
{
  /* Change the URI */

  DOMDocument *doc = CNCTdocuments[handle -1]->getDoc();
  doc->setDocumentURI(XMLString::transcode(fname));

  /* Serialize it to the new file name */

  return CNCTWriteDocToFile(handle, validate);
}

/******************************************************************************************************
*
* Function: CNCTCloseDoc
*
* Purpose: This function serializes a document if it has been modified then closes the document. 
*          If validation has been requested then the document will be validated prior to closing. 
*          It releases all resources currently in use by the document. 
*
* Inputs: handle - The document handle for the document to serialize.
*         validate - Flag used to determine whether or not to validate the document against
*                    its schema. (1 = Yes, 0 = No)
*
* Outputs: Messages are also output to the screen.
*
* Returns: Status indicating the status of the Parse.
*              1 - Document Closed as requested. 
*             -1 - Errors occured. Document may not be saved.
*
*******************************************************************************************************/
int CNCTCloseDoc(CNCTDocumentHandle handle, int validate)
{
  DOMNode *myNode;

  if (CNCTdocuments[handle-1]->isDirty() && CNCTWriteDocToFile(handle, validate) < 1) return -1;

  /* Get the root element */
  myNode = (DOMNode *)CNCTdocuments[handle -1]->getDoc();

  try
    {
      myNode->release();
    }
  catch (const DOMException& toCatch) 
    {
      if (toCatch.code == DOMException::INVALID_ACCESS_ERR)
        {
          return 1;
        }
      else
        {
          return 1;
        }
    }
  catch(const XMLException& toCatch)
    {
      char *pMsg = XMLString::transcode(toCatch.getMessage());
      cerr << "Error during Xerces-c Initialization.\n"
           << "  Exception Message: "
           << pMsg;
      XMLString::release(&pMsg);
      return 1;
    }
  catch (...)
    {
      return 1;
    }

  delete CNCTdocuments[handle-1];

  return 1;	
}

/******************************************************************************************************
*
* Function: IsXMLInitialized
*
* Purpose: This function is used to determine if the Xerces XML engine is intialized or not. 
*
* Inputs: None
*
* Outputs: None
*
* Returns: Status indicating the status of the Parse.
*              1 - Yes, the Xerces XML engine is initialized. 
*              0 - No, the Xerces XML engine is not initialized.
*
*******************************************************************************************************/
int IsXMLInitialized(void)
{
  if (MIW_XMLinitialized) return 1;

  return 0;	
}

/******************************************************************************************************
*
* Function: IsValidDoc
*
* Purpose: This function is used to determine if the XML Document pointed to by "handle" is an existing
*          DOM Tree in memory. 
*
* Inputs: handle - The document handle for the document.
*
* Outputs: None
*
* Returns: Status indicating the status of the Parse.
*              1 - Yes, the handle points a DOOM tree in Memory. 
*              0 - No, the handle does not point to a valid document. 
*
*******************************************************************************************************/
int IsValidDoc(CNCTDocumentHandle handle)
{
  if ((handle <= CNCTnumDocuments) && (CNCTdocuments[handle-1])) return 1;

  return 0;	
}

/******************************************************************************************************
*
* Function: AddNodeToRoot
*
* Purpose: This is a convenience function that adds a node to the root element of the DOM Tree pointed 
*          to by handle.  The added element then becomes the currnet element for the document.
*
* Inputs: handle - The document handle for the document.
*         tag - The tag name of the element to add.
*
* Outputs: None
*
* Returns: Status indicating the status of the operation.
*              1 - Succesful 
*              0 - Not Successful
*
*******************************************************************************************************/
int AddNodeToRoot(CNCTDocumentHandle docHandle, const char *tag)
{
  if (!IsValidDoc(docHandle)) return 0;

  if (MoveRoot(docHandle) && AddNode(docHandle,tag))
    {
      CNCTdocuments[docHandle-1]->setDirty(false);
      return 1;
    }

  return 0;
}
	
/******************************************************************************************************
*
* Function: AddNode
*
* Purpose: This function adds a node to the current element of the DOM Tree pointed 
*          to by handle.  The added element then becomes the current element for the document.
*
* Inputs: handle - The document handle for the document.
*         tag - The tag name of the element to add.
*
* Outputs: None
*
* Returns: Status indicating the status of the operation.
*              1 - Succesful 
*              0 - Not Successful
*
*******************************************************************************************************/
int AddNode(CNCTDocumentHandle docHandle, const char *tag)
{
  if (!IsValidDoc(docHandle)) return 0;

  DOMNode *node = CNCTdocuments[docHandle-1]->getCurrentNode();
  DOMDocument *doc = CNCTdocuments[docHandle-1]->getDoc();

  try
    {
      /* Create new element */
      DOMNode *newElem = (DOMNode *)doc->createElement(XMLString::transcode(tag));

      /* add it to the tree */
      node->appendChild(newElem);

      /* Make it the current node */
      CNCTdocuments[docHandle-1]->setCurrentNode(newElem);
    }
  catch (const DOMException& toCatch) 
    {
      char* message = XMLString::transcode(toCatch.msg);
      cout << "Exception message is: \n"
           << message << "\n";
      XMLString::release(&message);
      return 0;
    }
  catch (...)
    {
      return 0;
    }

  /* Got here, must be successful */

  CNCTdocuments[docHandle-1]->setDirty(true);
  return 1;
}

/**********************************************************
*
* Function SetAttribute
*
* Purpose:  This function sets the value of an attribute of the current Element.  
*           If the attribute has already been set then the value is changed.  
*
* Inputs: CNCTDocHandle - Handle for the created XML document.
*         attName - the name of the attribute.
*         attValue - the value of the attribute.
*
* Outputs: Possible error messages to stderr.
*
* Returns: int - status
*          1  - Successful
*          -1 - Current Element undefined
*          -2 - Attribute Error
*
* Method : 
*
****************************************************************************************************/

int SetAttribute(CNCTDocumentHandle docHandle, const char *attName, const char *attValue)
{
  /*Get the document's current Node */
  DOMNode* curNode = CNCTdocuments[docHandle-1]->getCurrentNode();

  /* if the current node is not an element then return error */

  if (curNode->getNodeType() != DOMNode::ELEMENT_NODE) return -1;

  /* Cast the Node object down to an Element object for convenience */
  DOMElement *curElem = (DOMElement *)curNode;

  try 
    {
      /* Set the attribute */

      curElem->setAttribute(XMLString::transcode(attName),XMLString::transcode(attValue));
    }
  catch (const DOMException& toCatch) 
    {
      char* message = XMLString::transcode(toCatch.msg);
      cout << "Exception message is: \n"
           << message << "\n";
      XMLString::release(&message);
      return -2;
    }
  catch (...)
    {
      return -2;
    }

  /* Got here must be successful */
  CNCTdocuments[docHandle-1]->setDirty(true);
  return 1;
}

/*********************************************************************************************
*
*  Function: SetAttributeByType
*
*  Purpose: This function sets the the value of an attribute of the current Element.  
*           If the attribute has already been set then the value is changed.  It accepts 
*           a flag to determine the datatype, and an optional descriptor used to format the
*           value of the attribute.  
*
* Inputs: CNCTDocHandle - Handle for the created XML document.
*         attName - the name of the attribute.
*         attValue - the value of the attribute.
*         datatype - Integer showing the type of specifier to be use for printing.

          descriptor - the actual C printf descriptor for writeing the attribute value.

*
* Outputs: Possible error messages to stderr.
*
* Returns: int - status
*          1  - Successful
*          -1 - Current Element undefined
*          -2 - Attribute Error
*
*  Inputs:
*         CNCTDocHandle - Handle for the created XML document.
*         attName - the name of the attribute. datatype - Integer showing the type of specifier to be use for printing.
*  	      attValue - Pointer to the value to be written.
*         dataType - Code identifying the C type of the attValue
* 	                  INTTYPE =    0 - int
*	                  STRTYPE =    1 - char *
*                     CHARTYPE =   2 - char 
*                     FLOATTYPE =  4 - float
*                     DOUBLETYPE = 5 - double
*                     LONGTYPE =   6 - long
*         descriptor - the actual C printf descriptor for writing the attribute value. If NULL, then the 
*                      default for the datatype is used (i.e. INTTYPE - %d).
*
*  Outputs: Possible error messages to stderr
*
*  Returns: Status
*           1  - Successful
*          -1 - Current Element undefined
*          -2 - Attribute Error
*
*********************************************************************************************/

int SetAttributeByType(CNCTDocumentHandle docHandle, const char *attName, void *attValue, 
                       int datatype, const char *descriptor)
{
  int *intval;
  char *charval;
  float *fltval;
  double *dblval;
  long *lngval;
  char strDesc[2048];


  switch (datatype)
    {
    case INTTYPE:
      intval = (int *)attValue;
      if (descriptor == NULL)
        {
          sprintf(strDesc,"%d",*intval);
        }
      else
        {
          sprintf(strDesc,descriptor,*intval);
        }
      break;

    case STRTYPE:
      if (descriptor == NULL)
        {
          sprintf(strDesc, "%s", (char *) attValue);
        }
      else
        {
          sprintf(strDesc,descriptor,attValue);
        }
      break;

    case CHARTYPE:
      charval = (char *)attValue;
      if (descriptor == NULL)
        {
          sprintf(strDesc,"%c",*charval);
        }
      else
        {
          sprintf(strDesc,descriptor,*charval);
        }
      break;

    case FLOATTYPE:
      fltval = (float *)attValue;
      if (descriptor == NULL)
        {
          sprintf(strDesc, "%f", *fltval);
        }
      else
        {
          sprintf(strDesc,descriptor,*fltval);
        }
      break;

    case DOUBLETYPE:
      dblval = (double *)attValue;
      if (descriptor == NULL)
        {
          sprintf(strDesc,"%f",*dblval);
        }
      else
        {
          sprintf(strDesc,descriptor,*dblval);
        }
      break;

    case LONGTYPE:
      lngval = (long *)attValue;
      if (descriptor == NULL)
        {
          sprintf(strDesc,"%ld",*lngval);
        }
      else
        {
          sprintf(strDesc,descriptor,*lngval);
        }
      break;

    default:
      return -2;
    }

  return SetAttribute(docHandle, attName,strDesc);
}

/************************************************************************************************
*
* Function FindNode
*
* Purpose:  This function finds a node in the node tree defined by the following criteria:
*           tag = a valid tag name, attName = NULL, attValue = NULL. 
*                 The first element with the tag in the node tree is made the current node.
*           tag = a valid tag name, attName = A valide Attribute Name, attValue = a value.
*                 The first element with the tag in the node tree that has the corresponding 
*                 attribute information is made current.
*           If no element meeting the specified criteria is found, then 0 is returned else the 
*           element meeting the criteria is made the current element for the document pointed to 
*           by handle.
*
* Inputs: CNCTDocHandle - Handle for the created XML document.
*         tag - The tag name of the element to find.
*         attName - the name of the attribute the check for.
*         attValue - the value of the attribute to check for.
*
* Outputs: Possible error messages to stderr
*
* Returns: int - status
*          1 = Successful
*          -1 = Error - No tag specified
*          -2 = Bad Document handle
*          -3 = No element meeting the criteria exist
*
****************************************************************************************************/

int FindNode(CNCTDocumentHandle docHandle, const char *tag, const char *attName, const char *attValue)
{
  bool found = false;
  DOMElement *myElem;

  /*  Make sure a tag is specified */
  if (!tag) return -1;

  /* Make sure we are working with a valid document */

  if (!IsValidDoc(docHandle)) return -2;

  /* Create a list of all elements with the tag name */

  DOMDocument* doc = CNCTdocuments[docHandle -1]->getDoc();
  DOMNodeList* mylist = doc->getElementsByTagName(XMLString::transcode(tag));
	
  /* If none exist then return none found */

  if (mylist->getLength() < 1) return -3;

  /* if the attName and attValue not specified, then the first one found becomes the current node */

  if (attName == NULL || attValue == NULL)
    {
      CNCTdocuments[docHandle -1]->setCurrentNode(mylist->item(0));
      return 1;
    }

  /* traverse the list looking for the attName and Value */
  /* If found then make that node the current and return 1 (Success) */

  for (XMLSize_t i=0; i<mylist->getLength();i++)
    {
      myElem = (DOMElement *)mylist->item(i);
      if (myElem->hasAttribute(XMLString::transcode(attName)) && 
          XMLString::compareString(XMLString::transcode(attValue), myElem->getAttribute(XMLString::transcode(attName))) ==0)
        {
          found = true;
          CNCTdocuments[docHandle -1]->setCurrentNode((DOMNode *)myElem);
          break;
        }
    }

  if (found) return 1;

  return -3;
}


/*************************************************************************************************
*
* Function GetAttribute
*
* Purpose:  This function gets the value of an attribute of the current Element.  
*           If the attribute has not been assigned a value
*           then a NULL is returned.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*         attName - the name of the attribute
*
* Outputs char * attValue - the value of the attribute
*
* Returns: int - status
*          1 = Successful
*          -1 = Error - Current Element not defined
*          -2 = Atttribute not defined
*
* Method : 
*
****************************************************************************************************/

int GetAttribute(CNCTDocumentHandle docHandle, const char *attName, char *attValue)
{
  /*Get the document's current Node */
  DOMNode* curNode = CNCTdocuments[docHandle-1]->getCurrentNode();

  /* if the current node is not an element then return error */

  if (curNode->getNodeType() != DOMNode::ELEMENT_NODE) return -1;

  /* Cast the Node object down to an Element object for convenience */
  DOMElement *curElem = (DOMElement *)curNode;

  if (curElem->hasAttribute(XMLString::transcode(attName)))
    {
      const XMLCh *val = curElem->getAttribute(XMLString::transcode(attName));
      strcpy(attValue,XMLString::transcode(val));
      return 1;
    }

  return -2;
}

/***************************************************************************************************
*
* Function GetCurrentElementType
*
* Purpose:  This function returns the tag name of the current element.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*
* Outputs none
*
* Returns: char *tag Name
*
* Method : 
*
****************************************************************************************************/

char *GetCurrentElementType(CNCTDocumentHandle docHandle)
{
  /*Get the document's current Node */
  DOMNode* curNode = CNCTdocuments[docHandle-1]->getCurrentNode();

  /* if the current node is not an element then return error */

  if (curNode->getNodeType() != DOMNode::ELEMENT_NODE) return NULL;

  return XMLString::transcode(curNode->getNodeName());
}

/***************************************************************************************************
*
* Function MoveUp
*
* Purpose:  This function moves the current Node pointer to the parent element in the tree. 
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*
* Outputs none
*
* Returns:  1 - Move OK
*           Error Conditions
*          -1 - Can not move to parent
*
* Method : 
*
****************************************************************************************************/

int MoveUp(CNCTDocumentHandle docHandle)
{
  /* Get current Node */
  DOMNode* curNode = CNCTdocuments[docHandle-1]->getCurrentNode();

  DOMNode* parentNode =  curNode->getParentNode();

  if (parentNode != NULL && parentNode->getNodeType() == DOMNode::ELEMENT_NODE)
    {
      CNCTdocuments[docHandle-1]->setCurrentNode(parentNode);
      return 1;
    }

  return -1;
}

/***************************************************************************************************
*
* Function MoveNextSibling
*
* Purpose:  This function makes the next sibling element of the node tree the current node.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*
* Outputs none
*
* Returns: status = 1 - Move Succesfful
*                   -1 - No move done, no more siblings
*
* Method : 
*
****************************************************************************************************/

int MoveNextSibling(CNCTDocumentHandle docHandle)
{
  bool finish = false;
  int rtnval = -1;

  /* Get current Node */
  DOMNode* nextSiblingNode = CNCTdocuments[docHandle-1]->getCurrentNode();

  while (!finish)
    {
      nextSiblingNode =  nextSiblingNode->getNextSibling();
      if (nextSiblingNode != NULL && nextSiblingNode->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          CNCTdocuments[docHandle-1]->setCurrentNode(nextSiblingNode);
          rtnval = 1;
          finish = true;
        }
      if (nextSiblingNode == NULL) finish = true;
    }

  return rtnval;
}

/***************************************************************************************************
*
* Function MovePreviousSibling
*
* Purpose:  This function makes the previous sibling element of the node tree the current node.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*
* Outputs none
*
* Returns: status = 1 - Move Succesfful
*                   -1 - No move done, no more siblings
*
* Method : 
*
****************************************************************************************************/

int MovePreviousSibling(CNCTDocumentHandle docHandle)
{
  bool finish = false;
  int rtnval = -1;

  /* Get current Node */
  DOMNode* prevSiblingNode = CNCTdocuments[docHandle-1]->getCurrentNode();

  while (!finish)
    {
      prevSiblingNode =  prevSiblingNode->getPreviousSibling();
      if (prevSiblingNode != NULL && prevSiblingNode->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          CNCTdocuments[docHandle-1]->setCurrentNode(prevSiblingNode);
          rtnval = 1;
          finish = true;
        }
      if (prevSiblingNode == NULL) finish = true;
    }

  return rtnval;
}

/***************************************************************************************************
*
* Function MoveFirstChild
*
* Purpose:  This function makes the first child element of the node tree of the current element the current node.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*
* Outputs none
*
* Returns: status = 1 - Move Succesfful
*                   -1 - No move done, no child element exist
*
* Method : 
*
****************************************************************************************************/

int MoveFirstChild(CNCTDocumentHandle docHandle)
{
  bool finish = false;
  int rtnval = -1;

  /* Get current Node */
  DOMNode* Node = CNCTdocuments[docHandle-1]->getCurrentNode();

  if (!Node->hasChildNodes()) return -1;

  /* move to first child element node in tree*/
  Node = Node->getFirstChild();
  while (!finish)
    {
      if (Node != NULL && Node->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          CNCTdocuments[docHandle-1]->setCurrentNode(Node);
          rtnval = 1;
          finish = true;
        }
      Node =  Node->getNextSibling();
      if (Node == NULL) finish = true;
    }
  return rtnval;
}

/***************************************************************************************************
*
* Function MoveLastChild
*
* Purpose:  This function makes the last child element of the node tree of the current element the current node.
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*
* Outputs none
*
* Returns: status = 1 - Move Succesfful
*                   -1 - No move done, no child element exist
*
* Method : 
*
****************************************************************************************************/

int MoveLastChild(CNCTDocumentHandle docHandle)
{
  bool finish = false;
  int rtnval = -1;

  /* Get current Node */
  DOMNode* Node = CNCTdocuments[docHandle-1]->getCurrentNode();

  if (!Node->hasChildNodes()) return -1;

  /* move to first child element node in tree*/
  Node = Node->getLastChild();
  while (!finish)
    {
      if (Node != NULL && Node->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          CNCTdocuments[docHandle-1]->setCurrentNode(Node);
          rtnval = 1;
          finish = true;
        }
      Node =  Node->getPreviousSibling();
      if (Node == NULL) finish = true;
    }
  return rtnval;
}

/***************************************************************************************************
*
* Function MoveRoot
*
* Purpose:  This functions makes the root node of the document the current element. 
*
* Inputs: CNCTDocHandle - Handle for the created XML document
*
* Outputs none
*
* Returns: 1 = Successul
*          otherwise move unsecessful
*
* Method : 
*
****************************************************************************************************/

int MoveRoot(CNCTDocumentHandle docHandle)
{
  /* Set the root element */
  DOMDocument *doc = CNCTdocuments[docHandle-1]->getDoc();
  DOMElement* rootElem = doc->getDocumentElement();

  /*Save the tree location for use elsewhere */
  CNCTdocuments[docHandle-1]->setCurrentNode(rootElem);

  return 1;
}

/***************************************************************************************************
*
* Function RemoveAttribute
*
* Purpose:  This function removes an attribute from the current Element.  The attribute name and value is removed 
*           from the node tree.
*
* Inputs: CNCTDocHandle - Handle for the created XML document.
*         attName - the name of the attribute.
*
* Outputs none
*
* Returns:  1 - Sucessfully removed the attribute
*          -1 - Unknown Error
*          -2 - Attribute did not exist or is readonly 
*
****************************************************************************************************/

int RemoveAttribute(CNCTDocumentHandle docHandle, const char *attName)
{
  CNCTDocument *doc = CNCTdocuments[docHandle-1];
  /*Get the document's current Node */
  DOMNode* curNode = doc->getCurrentNode();

  /* if the current node is not an element then return error */

  if (curNode->getNodeType() != DOMNode::ELEMENT_NODE) return -1;

  /* Cast the Node object down to an Element object for convenience */
  DOMElement *curElem = (DOMElement *)curNode;

  if (curElem->hasAttribute(XMLString::transcode(attName)))
    {
      try
        {
          curElem->removeAttribute(XMLString::transcode(attName));
        }
      catch (DOMException::ExceptionCode  code)
        {
          if (code == DOMException::NO_MODIFICATION_ALLOWED_ERR)
            {
              return -2;
            }
          else 
            {
              return -1;
            }
        }   
      catch (...)
        {
          return -1;
        }

      /* Attribute removed */

      doc->setDirty(true);
      return 1;
    }
  else
    {
      /* Attribute does not exist */
      return -2;
    }
   
  return 1;
}

/***************************************************************************************************
*
* Function RemoveNode
*
* Purpose:  This function removes the current element from the node tree.  The parent of that element 
*           then becomes the current node.  Note, the root node can not be deleted from the node tree.
*
* Inputs: CNCTDocHandle - Handle for the created XML document.
*
* Outputs none
*
* Returns:  1 - Sucessfully removed the attribute
*          -1 - Cannot remove the root element 
*
****************************************************************************************************/

int RemoveNode(CNCTDocumentHandle docHandle)
{
  DOMNode *curNode = CNCTdocuments[docHandle-1]->getCurrentNode();

  if (curNode->isSameNode(CNCTdocuments[docHandle-1]->getDoc())) return -1;

  DOMNode* parentNode =  curNode->getParentNode();

  /* remove the current node from the tree  and release it */

  try
    {
      DOMNode *yourhistory = parentNode->removeChild(curNode);
      yourhistory->release();
    }
  catch (DOMException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.msg);
      cout << "Exception message is: \n"
           << message << "\n";
      XMLString::release(&message);
      return -1;
    }   
  catch (...)
    {
      return -1;
    }

  CNCTdocuments[docHandle-1]->setCurrentNode(parentNode);
  CNCTdocuments[docHandle-1]->setDirty(false);

  return 1;
}

/***************************************************************************************************
*
* Function SetNodeValue
*
* Purpose:  This function set the value the current element. 
*
* Inputs: CNCTDocHandle - Handle for the created XML document.
*
* Outputs none
*
* Returns:  1 - Sucessfully set the value
*          -1 - Could set the value 
*
****************************************************************************************************/

int SetNodeValue(CNCTDocumentHandle docHandle, const char *val)
{
  DOMNode *curNode = CNCTdocuments[docHandle-1]->getCurrentNode();
  if (curNode->getNodeType() != DOMNode::ELEMENT_NODE) return -1;

  /* Set the value */

  const XMLCh *xVal = XMLString::transcode(val);
  try
    {
      curNode->setTextContent(xVal);
    }
  catch (DOMException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.msg);
      cout << "Exception message is: \n"
           << message << "\n";
      XMLString::release(&message);
      return -1;
    }   
  catch (...)
    {
      return -1;
    }

  //   XMLString::release(&xVal);
  CNCTdocuments[docHandle-1]->setDirty(false);

  return 1;
}

/***************************************************************************************************
*
* Function GetNodeValue
*
* Purpose:  This function gets the value the current element. 
*
* Inputs: CNCTDocHandle - Handle for the created XML document.
*
* Outputs val - String of nodal value
*
* Returns:  1 - Sucessfully got the value
*          -1 - Could not get the value 
*
****************************************************************************************************/

int GetNodeValue(CNCTDocumentHandle docHandle, char *val)
{
  const XMLCh *xVal;
  DOMNode *curNode = CNCTdocuments[docHandle-1]->getCurrentNode();
  if (curNode->getNodeType() != DOMNode::ELEMENT_NODE) return -1;

  /* Get the value */
  /* Normalize subtree thenTraverse Child nodes looking for a text Node */

  try
    {
      //	   curNode->normalize();
      //	   if (curNode->hasChildNodes())
      //	   {
      //
      //	   }
      //	   else
      //	   { /* no text node value so return ?? */
      //	   }
      xVal = curNode->getTextContent();
    }
  catch (DOMException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.msg);
      cout << "Exception message is: \n"
           << message << "\n";
      XMLString::release(&message);
      return -1;
    }
  catch (...)
    {
      return -1;
    }

   
  strcpy(val,XMLString::transcode(xVal));

  return 1;
}

/************************************************************************************************
*
* Function FindChildNode
*
* Purpose:  This function finds a node in the node tree descendant to the current element
*           defined by the following criteria:
*           tag = a valid tag name, attName = NULL, attValue = NULL. 
*                 The first element with the tag in the node tree is made the current node.
*           tag = a valid tag name, attName = A valide Attribute Name, attValue = a value.
*                 The first element with the tag in the node tree that has the corresponding 
*                 attribute information is made current.
*           If no element meeting the specified criteria is found, then 0 is returned else the 
*           element meeting the criteria is made the current element for the document pointed to 
*           by handle.
*
* Inputs: CNCTDocHandle - Handle for the created XML document.
*         tag - The tag name of the element to find.
*         attName - the name of the attribute the check for.
*         attValue - the value of the attribute to check for.
*
* Outputs: Possible error messages to stderr
*
* Returns: int - status
*          1 = Successful
*          -1 = Error - No tag specified
*          -2 = Bad Document handle
*          -3 = No element meeting the criteria exist
*
****************************************************************************************************/

int FindChildNode(CNCTDocumentHandle docHandle, const char *tag, const char *attName, const char *attValue)
{
  bool found = false;
  DOMElement *myElem;

  /*  Make sure a tag is specified */
  if (!tag) return -1;

  /* Make sure we are working with a valid document */

  if (!IsValidDoc(docHandle)) return -2;

  /* Create a list of all elements with the tag name */
  /* if the current node is not an element then return error */
  /*Get the document's current Node */
  DOMNode* curNode = CNCTdocuments[docHandle-1]->getCurrentNode();

  if (curNode->getNodeType() != DOMNode::ELEMENT_NODE) return -1;

  /* Cast the Node object down to an Element object for convenience */
  DOMElement *curElem = (DOMElement *)curNode;

  DOMNodeList* mylist = curElem->getElementsByTagName(XMLString::transcode(tag));
	
  /* If none exist then return none found */

  if (mylist->getLength() < 1) return -3;

  /* if the attName and attValue not specified, then the first one found becomes the current node */

  if (attName == NULL || attValue == NULL)
    {
      CNCTdocuments[docHandle -1]->setCurrentNode(mylist->item(0));
      return 1;
    }

  /* traverse the list looking for the attName and Value */
  /* If found then make that node the current and return 1 (Success) */

  for (XMLSize_t i=0; i<mylist->getLength();i++)
    {
      myElem = (DOMElement *)mylist->item(i);
      if (myElem->hasAttribute(XMLString::transcode(attName)) && 
          XMLString::compareString(XMLString::transcode(attValue), 
                                   myElem->getAttribute(XMLString::transcode(attName))) ==0)
        {
          found = true;
          CNCTdocuments[docHandle -1]->setCurrentNode((DOMNode *)myElem);
          break;
        }
    }

  if (found) return 1;

  return -3;
}
