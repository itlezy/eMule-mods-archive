#include "stdafx.h"
#include "XMLHelper.h"

XmlDoc XmlLoadDocumentFromFile(CString szFile)
{
	try
	{
	XmlDoc spDoc;
	spDoc.CreateInstance(__uuidof(DOMDocument));
	if(spDoc->load(variant_t(szFile)) == VARIANT_TRUE)
		return spDoc;
	}
	catch(...){ }
	return NULL;
}

XmlDoc XmlLoadDocumentFromStr(CString sXml)
{
	try
	{
	XmlDoc spDoc;
	spDoc.CreateInstance(__uuidof(DOMDocument));
	if(spDoc->loadXML(bstr_t(sXml)) == VARIANT_TRUE)
		return spDoc;
	}
	catch(...){ }
	return NULL;
}

XmlDoc XmlCreateDocument()
{
	try
	{
	XmlDoc spDoc;
	spDoc.CreateInstance(__uuidof(DOMDocument));
	spDoc->async = VARIANT_FALSE;
	return spDoc;
	}
	catch(...){ }
	return NULL;
}

// for example, 'xml', 'version="1.0"'
//			or, 'xml-stylesheet', 'type="text/xsl" href="stylesheet.xsl"'
bool XmlInsertProcessingInstruction(XmlDoc spDoc, CString sInstruction, CString sData)
{
	try
	{
		MSXML::IXMLDOMProcessingInstructionPtr spPI = 
			spDoc->createProcessingInstruction(bstr_t(sInstruction), bstr_t(sData));
		if(spPI != NULL)
			return SUCCEEDED(spDoc->appendChild(spPI));
	}
	catch(...){ }
	return false;
}

XmlNode XmlCreateElement(XmlDoc spDoc, XmlNode spParent, CString sElementName)
{
	try
	{
		XmlNode spNode = spDoc->createElement(bstr_t(sElementName));
		if(spNode != NULL)
		{
			if(spParent !=NULL)
			{
				if(SUCCEEDED(spParent->appendChild(spNode)))
					return spNode;
			}
			else
			{
				if(SUCCEEDED(spDoc->appendChild(spNode)))
					return spNode;
			}
		}
	}
	catch(...){ }
	return NULL;
}

bool XmlSetAttribute(XmlElement spElem, CString sAttribName, variant_t vtValue)
{
	try
	{
	return SUCCEEDED(spElem->setAttribute(bstr_t(sAttribName), vtValue));
	}
	catch(...){ }
	return false;
}

CString XmlGetAttributeStr(XmlElement spElem, CString sAttribute)
{
	try
	{
	XmlNodeMap spAttrMap = spElem->attributes;
	if(spAttrMap != NULL)
	{
		XmlNode spNode = spAttrMap->getNamedItem(bstr_t(sAttribute));
		if(spNode)
			return (LPCTSTR)spNode->text;
	}
	}
	catch(...){ }
	return _T("");
}

long XmlGetAttributeLong(XmlElement spElem, CString sAttribute, long lDefault/* = -1*/)
{
	try
	{
	XmlNodeMap spAttrMap = spElem->attributes;
	if(spAttrMap != NULL)
	{
		XmlNode spNode = spAttrMap->getNamedItem(bstr_t(sAttribute));
		if(spNode)
			return _ttol((LPCTSTR)spNode->text);
	}
	}
	catch(...){ }
	return lDefault;
}
