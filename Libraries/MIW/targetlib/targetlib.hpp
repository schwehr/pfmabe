#ifndef __TARGETLIB_HPP__
#define __TARGETLIB_HPP__

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>

#include <iostream>

#include "targetlib.h"

#include "CNCTDocument.hpp"

using namespace std;

XERCES_CPP_NAMESPACE_USE

// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of char* data to XMLCh data.
// ---------------------------------------------------------------------------
class XStr
{
public :
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  XStr(const char* const toTranscode)
  {
    // Call the private transcoding method
    fUnicodeForm = XMLString::transcode(toTranscode);
  }

  ~XStr()
  {
    XMLString::release(&fUnicodeForm);
  }


  // -----------------------------------------------------------------------
  //  Getter methods
  // -----------------------------------------------------------------------
  const XMLCh* unicodeForm() const
  {
    return fUnicodeForm;
  }

private :
  // -----------------------------------------------------------------------
  //  Private data members
  //
  //  fUnicodeForm
  //      This is the Unicode XMLCh format of the string.
  // -----------------------------------------------------------------------
  XMLCh*   fUnicodeForm;
};

#define X(str) XStr(str).unicodeForm()


CNCTDocument *CNCTdocuments[1000];


int CNCTnumDocuments;


/* internal prototyes */

int ParseDoc(const char *fname, DOMDocument **myDoc, bool parseOnly, int validate);

#endif
