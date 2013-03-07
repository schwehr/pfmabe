
#include "CNCTDocument.hpp"

CNCTDocument::CNCTDocument() 
{
  needsWriting = false;
}

CNCTDocument::~CNCTDocument()
{
  //  CNCTDoc->release();
}


bool CNCTDocument::isDirty()
{
  return needsWriting;
}

void CNCTDocument::setDirty(bool newFlag)
{
  needsWriting = newFlag;
}


void CNCTDocument::setCurrentNode(DOMNode *node)
{
  currentNode = node;
}

DOMNode *CNCTDocument::getCurrentNode()
{
  return currentNode;
}

DOMDocument *CNCTDocument::getDoc(void)
{
  return CNCTDoc;
};

void CNCTDocument::setDoc(DOMDocument *newDoc)
{
  CNCTDoc = newDoc;
};
