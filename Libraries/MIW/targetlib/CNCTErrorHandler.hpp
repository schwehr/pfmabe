#ifndef __CNCTERRORHANDLER_HPP__
#define __CNCTERRORHANDLER_HPP__

#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <iostream>

using namespace std ;

XERCES_CPP_NAMESPACE_USE

// ---------------------------------------------------------------------------
//  Simple error handler deriviative to install on parser
// ---------------------------------------------------------------------------
class CNCTErrorHandler : public DOMErrorHandler
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  CNCTErrorHandler();
  ~CNCTErrorHandler();


  // -----------------------------------------------------------------------
  //  Getter methods
  // -----------------------------------------------------------------------
  bool getSawErrors() const;


  // -----------------------------------------------------------------------
  //  Implementation of the DOM ErrorHandler interface
  // -----------------------------------------------------------------------
  bool handleError(const DOMError& domError);
  void resetErrors();


private :
  // -----------------------------------------------------------------------
  //  Unimplemented constructors and operators
  // -----------------------------------------------------------------------
  CNCTErrorHandler(const CNCTErrorHandler&);
  void operator=(const CNCTErrorHandler&);


  // -----------------------------------------------------------------------
  //  Private data members
  //
  //  fSawErrors
  //      This is set if we get any errors, and is queryable via a getter
  //      method. Its used by the main code to suppress output if there are
  //      errors.
  // -----------------------------------------------------------------------
  bool    fSawErrors;
};



inline bool CNCTErrorHandler::getSawErrors() const
{
  return fSawErrors;
}

#endif
