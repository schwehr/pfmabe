#ifndef __TARGETLIB_H__
#define __TARGETLIB_H__

/* ****************************************************************************************
*
* File: targetlib.h 
*
* Purpose: The include for for definitions and types and prototypes for interfacing to C (not c++)
*          This file is not specific to the MIWTarget schema.
*
* History: 
*  08/2003 - RWL Development
*
*******************************************************************************************/

#define CNCTDocumentHandle int


/* Prototypes */

/* if linking from C compiler then map C naming conventions to C++ (library source) */

#ifdef __cplusplus
extern "C"
{
#endif

#define INTTYPE 0
#define STRTYPE 1
#define CHARTYPE 2
#define FLOATTYPE 3
#define DOUBLETYPE 4
#define LONGTYPE 5


CNCTDocumentHandle  CNCTNewDoc(const char *fname, const char *schema, const char *rootElem);
CNCTDocumentHandle  CNCTEditDoc(const char *fname, int validate);
int CNCTCloseDoc(CNCTDocumentHandle handle, int validate);
int CNCTWriteDocToFile(CNCTDocumentHandle handle,int validate);
int CNCTSaveDocAs(CNCTDocumentHandle handle, const char *fname, int validate);

int IsValidDoc(CNCTDocumentHandle handle);
int IsXMLInitialized(void);
int TerminateXML();


int AddNodeToRoot(CNCTDocumentHandle docHandle, const char* tag);
int SetAttribute(CNCTDocumentHandle docHandle, const char *attName, const char * attValue);
int SetAttributeByType(CNCTDocumentHandle docHandle, const char *attName, void *attValue, int datatype, const char *descriptor);

int AddNode(CNCTDocumentHandle docHandle, const char *tag);

char *GetCurrentElementType(CNCTDocumentHandle docHandle);
int GetAttribute(CNCTDocumentHandle docHandle, const char *attName, char *attValue);

int FindNode(CNCTDocumentHandle docHandle, const char *tag, const char *attName, const char *attValue);
int MoveUp(CNCTDocumentHandle docHandle);
int MovePreviousSibling(CNCTDocumentHandle docHandle);
int MoveNextSibling(CNCTDocumentHandle docHandle);
int MoveFirstChild(CNCTDocumentHandle docHandle);
int MoveLastChild(CNCTDocumentHandle docHandle);
int MoveRoot(CNCTDocumentHandle docHandle);

int RemoveNode(CNCTDocumentHandle docHandle);
int RemoveAttribute(CNCTDocumentHandle docHandle, const char *attName);

int SetNodeValue(CNCTDocumentHandle docHandle, const char *val);
int GetNodeValue(CNCTDocumentHandle docHandle, char *val);
int FindChildNode(CNCTDocumentHandle docHandle, const char *tag, const char *attName, const char *attValue);



#ifdef __cplusplus
}
#endif

#endif
