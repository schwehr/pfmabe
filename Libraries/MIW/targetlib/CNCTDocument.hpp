#ifndef __CNCTDOCUMENT_HPP__
#define __CNCTDOCUMENT_HPP__


#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_USE

class CNCTDocument 
{
public:	

  CNCTDocument();
  ~CNCTDocument();

  bool isDirty(void);
  void setDirty(bool);
  void setCurrentNode(DOMNode *);
  DOMNode *getCurrentNode(void);
  void setDoc(DOMDocument *);
  DOMDocument *getDoc(void);


protected:
private:
  bool needsWriting;
  DOMNode *currentNode;
  DOMDocument *CNCTDoc;
};

#endif
