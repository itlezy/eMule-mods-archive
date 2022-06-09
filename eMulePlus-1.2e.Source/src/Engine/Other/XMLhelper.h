#pragma once

#import "msxml.dll"

#define XmlDoc		MSXML::IXMLDOMDocumentPtr 
#define XmlNodes	MSXML::IXMLDOMNodeListPtr
#define XmlNode		MSXML::IXMLDOMNodePtr
#define XmlElement	MSXML::IXMLDOMElementPtr
#define XmlNodeMap	MSXML::IXMLDOMNamedNodeMapPtr

XmlDoc	XmlLoadDocumentFromFile(CString szFile);
XmlDoc	XmlLoadDocumentFromStr(CString sXml);
XmlDoc	XmlCreateDocument();
bool	XmlInsertProcessingInstruction(XmlDoc spDoc, CString sInstruction, CString sData);
XmlNode	XmlCreateElement(XmlDoc spDoc, XmlNode spParent, CString sElementName);
bool	XmlSetAttribute(XmlElement spElem, CString sAttribName, variant_t vtValue);
CString	XmlGetAttributeStr(XmlElement spElem, CString sAttribute);
long	XmlGetAttributeLong(XmlElement spElem, CString sAttribute, long lDefault = -1);
