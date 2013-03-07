/******************************************************************************************************
* Class CNCTErrorHandler
*
*******************************************************************************************************/

#include <xercesc/dom/DOM.hpp>
#include "CNCTErrorHandler.hpp"


CNCTErrorHandler::CNCTErrorHandler() :
  fSawErrors(false)
{
}

CNCTErrorHandler::~CNCTErrorHandler()
{
}


// ---------------------------------------------------------------------------
//  DOMCountHandlers: Overrides of the DOM ErrorHandler interface
// ---------------------------------------------------------------------------
bool CNCTErrorHandler::handleError(const DOMError& domError)
{
  fSawErrors = true;
  if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
    {
      cerr << "\nWarning at file ";
    }
  else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR)
    {
      cerr << "\nError at file ";
    }
  else
    {
      cerr << "\nFatal Error at file ";
    }

  cerr << XMLString::transcode(domError.getLocation()->getURI())
       << ", line " << domError.getLocation()->getLineNumber()
       << ", char " << domError.getLocation()->getColumnNumber()
       << "\n  Message: " << XMLString::transcode(domError.getMessage()) << endl;

  return true;
}

void CNCTErrorHandler::resetErrors()
{
  fSawErrors = false;
}
