//	CXMLElement.cpp
//
//	CXMLElement class

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"

static CObjectClass<CXMLElement>g_Class(OBJID_CXMLELEMENT, NULL);

CXMLElement::CXMLElement (void) : CObject(&g_Class),
		m_pParent(NULL),
		m_Attributes(TRUE, FALSE)

//	CXMLElement constructor

	{
	}

CXMLElement::CXMLElement (const CXMLElement &Obj) : CObject(&g_Class)

//	CXMLElement constructor

	{
	int i;

	m_sTag = Obj.m_sTag;
	m_pParent = Obj.m_pParent;
	m_Attributes = Obj.m_Attributes;
	m_ContentText = Obj.m_ContentText;

	m_ContentElements.InsertEmpty(Obj.m_ContentElements.GetCount());
	for (i = 0; i < m_ContentElements.GetCount(); i++)
		m_ContentElements[i] = new CXMLElement(*Obj.m_ContentElements[i]);
	}

CXMLElement::CXMLElement (const CString &sTag, CXMLElement *pParent) : CObject(&g_Class),
		m_sTag(sTag),
		m_pParent(pParent),
		m_Attributes(TRUE, FALSE)

//	CXMLElement constructor

	{
	}

CXMLElement &CXMLElement::operator= (const CXMLElement &Obj)

//	CXMLElement operator=

	{
	int i;

	CleanUp();

	CObject::operator=(Obj);
	m_sTag = Obj.m_sTag;
	m_pParent = Obj.m_pParent;
	m_Attributes = Obj.m_Attributes;
	m_ContentText = Obj.m_ContentText;

	m_ContentElements.InsertEmpty(Obj.m_ContentElements.GetCount());
	for (i = 0; i < m_ContentElements.GetCount(); i++)
		m_ContentElements[i] = new CXMLElement(*Obj.m_ContentElements[i]);

	return *this;
	}

ALERROR CXMLElement::AddAttribute (const CString &sAttribute, const CString &sValue)

//	AddAttribute
//
//	Add the given attribute to our table

	{
	ALERROR error;
	CString *pValue;

	pValue = new CString(sValue);
	if (pValue == NULL)
		return ERR_MEMORY;

	if (error = m_Attributes.AddEntry(sAttribute, pValue))
		{
		delete pValue;
		return error;
		}

	return NOERROR;
	}

ALERROR CXMLElement::AppendContent (const CString &sContent, int iIndex)

//	AppendContent
//
//	Appends some content

	{
	//	Always append to the last content element

	int iCount = m_ContentText.GetCount();
	if (iCount == 0)
		m_ContentText.Insert(sContent);
	else if (iIndex >= 0 && iIndex < iCount)
		m_ContentText[iIndex].Append(sContent);
	else
		m_ContentText[iCount - 1].Append(sContent);

	return NOERROR;
	}

ALERROR CXMLElement::AppendSubElement (CXMLElement *pElement, int iIndex)

//	AppendSubElement
//
//	Append a sub element. We take ownership of pElement.

	{
	//	Are we appending to the end?

	if (iIndex < 0 || iIndex >= m_ContentElements.GetCount())
		{
		//	If this element was previously empty, we have to add
		//	a content text item. [Because we optimize the case where
		//	there are no children.]

		if (m_ContentText.GetCount() == 0)
			m_ContentText.Insert(NULL_STR);

		//	Append the element

		m_ContentElements.Insert(pElement);

		//	We always add a new content text value at the end

		m_ContentText.Insert(NULL_STR);
		}

	//	Otherwise, we're inserting at a position.

	else
		{
		//	If this element was previously empty, we have to add
		//	a content text item. [Because we optimize the case where
		//	there are no children.]

		if (m_ContentText.GetCount() == 0)
			m_ContentText.Insert(NULL_STR);

		m_ContentElements.Insert(pElement, iIndex);
		m_ContentText.Insert(NULL_STR, iIndex + 1);
		}

	return NOERROR;
	}

bool CXMLElement::AttributeExists (const CString &sName)

//	AttributeExists
//
//	Returns TRUE if the attribute exists in the element

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		return true;
	else
		return false;
	}

void CXMLElement::CleanUp (void)

//	CleanUp
//
//	Free

	{
	int i;

	for (i = 0; i < m_ContentElements.GetCount(); i++)
		delete m_ContentElements[i];

	m_ContentElements.DeleteAll();
	}

CString CXMLElement::ConvertToString (void)

//	StreamToString
//
//	Convert to a string.

	{
	CMemoryWriteStream Stream;
	if (Stream.Create() != NOERROR)
		return NULL_STR;

	WriteToStream(&Stream);

	return CString(Stream.GetPointer(), Stream.GetLength());
	}

ALERROR CXMLElement::DeleteSubElement (int iIndex)

//	DeleteSubElement
//
//	Deletes the given sub-element

	{
	if (iIndex >= m_ContentElements.GetCount())
		return ERR_FAIL;

	delete m_ContentElements[iIndex];
	m_ContentElements.Delete(iIndex);

	m_ContentText[iIndex].Append(m_ContentText[iIndex + 1]);
	m_ContentText.Delete(iIndex + 1);

	return NOERROR;
	}

bool CXMLElement::FindAttribute (const CString &sName, CString *retsValue)

//	FindAttribute
//
//	If the attribute exists, returns TRUE and the attribute value.
//	Otherwise, returns FALSE

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		{
		if (retsValue)
			*retsValue = *pValue;
		return true;
		}
	else
		return false;
	}

bool CXMLElement::FindAttributeBool (const CString &sName, bool *retbValue)

//	FindAttributeBool
//
//	If the attribute exists, returns TRUE and the value
//	Otherwise, returns FALSE

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		{
		if (retbValue)
			*retbValue = IsBoolTrueValue(*pValue);
		return true;
		}
	else
		return false;
	}

bool CXMLElement::FindAttributeDouble (const CString &sName, double *retrValue)

//	FindAttributeDouble
//
//	Finds an attribute.

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		{
		if (retrValue)
			*retrValue = strToDouble(*pValue, 0.0);
		return true;
		}
	else
		return false;
	}

bool CXMLElement::FindAttributeInteger (const CString &sName, int *retiValue)

//	FindAttributeInteger
//
//	If the attribute exists, returns TRUE and the attribute value.
//	Otherwise, returns FALSE

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		{
		if (retiValue)
			*retiValue = strToInt(*pValue, 0, NULL);
		return true;
		}
	else
		return false;
	}

CString CXMLElement::GetAttribute (const CString &sName)

//	GetAttribute
//
//	Returns the attribute

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		return *pValue;
	else
		return CString();
	}

bool CXMLElement::GetAttributeBool (const CString &sName)

//	GetAttributeBool
//
//	Returns TRUE or FALSE for the attribute

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		return IsBoolTrueValue(*pValue);
	else
		return false;
	}

double CXMLElement::GetAttributeDouble (const CString &sName)

//	GetAttributeDouble
//
//	Returns a double attribute.

	{
	return strToDouble(GetAttribute(sName), 0.0);
	}

double CXMLElement::GetAttributeDoubleBounded (const CString &sName, double rMin, double rMax, double rNull)

//	GetAttributeDoubleBounded
//
//	Returns a double, insuring that it is in range

	{
	CString sValue;
	if (FindAttribute(sName, &sValue))
		{
		bool bFailed;
		double rValue = strToDouble(sValue, rNull, &bFailed);
		if (bFailed)
			return rNull;

		//	The null value is always valid

		if (rValue == rNull)
			return rValue;

		//	If iMax is less than iMin, then there is no maximum

		else if (rMax < rMin)
			return Max(rValue, rMin);

		//	Bounded

		else
			return Max(Min(rValue, rMax), rMin);
		}
	else
		return rNull;
	}

float CXMLElement::GetAttributeFloat (const CString &sName)

//	GetAttributeFloat
//
//	Returns a floating point attribute

	{
	return (float)strToDouble(GetAttribute(sName), 0.0);
	}

int CXMLElement::GetAttributeInteger (const CString &sName)

//	GetAttributeInteger
//
//	Returns an integer attribute

	{
	return strToInt(GetAttribute(sName), 0, NULL);
	}

int CXMLElement::GetAttributeIntegerBounded (const CString &sName, int iMin, int iMax, int iNull)

//	GetAttributeIntegerBounded
//
//	Returns an integer, insuring that it is in range

	{
	CString sValue;
	if (FindAttribute(sName, &sValue))
		{
		bool bFailed;
		int iValue = strToInt(sValue, iNull, &bFailed);
		if (bFailed)
			return iNull;

		//	The null value is always valid

		if (iValue == iNull)
			return iValue;

		//	If iMax is less than iMin, then there is no maximum

		else if (iMax < iMin)
			return Max(iValue, iMin);

		//	Bounded

		else
			return Max(Min(iValue, iMax), iMin);
		}
	else
		return iNull;
	}

ALERROR CXMLElement::GetAttributeIntegerList (const CString &sName, CIntArray *pList)

//	GetAttributeIntegerList
//
//	Appends a list of integers separated by commas

	{
	return ParseAttributeIntegerList(GetAttribute(sName), pList);
	}

CXMLElement *CXMLElement::GetContentElementByTag (const CString &sTag) const

//	GetContentElementByTag
//
//	Returns a sub element of the given tag

	{
	for (int i = 0; i < GetContentElementCount(); i++)
		{
		CXMLElement *pElement = GetContentElement(i);

		if (strCompareAbsolute(sTag, pElement->GetTag()) == 0)
			return pElement;
		}

	return NULL;
	}

void CXMLElement::MergeFrom (CXMLElement *pElement)

//	MergeFrom
//
//	Merges the given element into ours.
//
//	Attributes are replaced if they duplicate; child elements are appended to the end.

	{
	int i;

	//	Copy all attributes (replacing in case of duplication)

	for (i = 0; i < pElement->GetAttributeCount(); i++)
		SetAttribute(pElement->GetAttributeName(i), pElement->GetAttribute(i));

	//	If we have at least one text item then append it to our last
	//	text item.

	if (pElement->m_ContentText.GetCount() > 0)
		{
		if (m_ContentText.GetCount() == 0)
			m_ContentText.Insert(pElement->m_ContentText[0]);
		else
			m_ContentText[m_ContentText.GetCount() - 1].Append(pElement->m_ContentText[0]);
		}

	//	Now add each of the child elements

	for (i = 0; i < pElement->GetContentElementCount(); i++)
		{
		m_ContentElements.Insert(pElement->GetContentElement(i)->OrphanCopy());

		//	If we have no text items then we need to insert one (it goes before
		//	this new element). NOTE: This shouldn't happen if pElement is
		//	properly formed.

		if (m_ContentText.GetCount() == 0)
			m_ContentText.Insert(NULL_STR);

		//	Now insert the text that goes AFTER the element that we just
		//	inserted.

		if (i + 1 < pElement->m_ContentText.GetCount())
			m_ContentText.Insert(pElement->m_ContentText[i + 1]);
		else
			m_ContentText.Insert(NULL_STR);
		}
	}

CXMLElement *CXMLElement::OrphanCopy (void)

//	OrphanCopy
//
//	Creates a copy of the element and makes it top-level

	{
	CXMLElement *pCopy = new CXMLElement(*this);
	pCopy->m_pParent = NULL;
	return pCopy;
	}

ALERROR CXMLElement::SetAttribute (const CString &sName, const CString &sValue)

//	SetAttribute
//
//	Sets an attribute on the element.

	{
	ALERROR error;
	CString *pValue;

	pValue = new CString(sValue);
	if (pValue == NULL)
		return ERR_MEMORY;

	if (error = m_Attributes.ReplaceEntry(sName, pValue, TRUE, NULL))
		{
		delete pValue;
		return error;
		}

	return NOERROR;
	}

ALERROR CXMLElement::SetContentText (const CString &sContent, int iIndex)

//	SetContentText
//
//	Sets the content

	{
	//	Always append to the last content element

	int iCount = m_ContentText.GetCount();
	if (iCount == 0)
		m_ContentText.Insert(sContent);
	else if (iIndex >= 0 && iIndex < iCount)
		m_ContentText[iIndex] = sContent;
	else
		m_ContentText[iCount - 1] = sContent;

	return NOERROR;
	}

ALERROR CXMLElement::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Streams out to text XML

	{
	int i;

	//	Open tag

	pStream->Write("<", 1);
	pStream->Write(m_sTag.GetASCIIZPointer(), m_sTag.GetLength());

	//	If we have attributes, write them out

	if (GetAttributeCount() > 0)
		{
		for (i = 0; i < GetAttributeCount(); i++)
			{
			CString sName = GetAttributeName(i);
			CString sValue = MakeAttribute(GetAttribute(i));

			pStream->Write(" ", 1);
			pStream->Write(sName.GetASCIIZPointer(), sName.GetLength());
			pStream->Write("=\"", 2);
			pStream->Write(sValue.GetASCIIZPointer(), sValue.GetLength());
			pStream->Write("\"", 1);
			}
		}

	//	If we don't have any sub elements, then we're done.

	if (m_ContentText.GetCount() == 0)
		{
		pStream->Write("/>", 2);
		return NOERROR;
		}

	//	End of open tag

	pStream->Write(">", 1);

	//	Output sub-elements and text

	for (i = 0; i < m_ContentElements.GetCount(); i++)
		{
		//	Write the text before this sub-element

		CString sXMLText = strToXMLText(m_ContentText[i], true);
		pStream->Write(sXMLText.GetASCIIZPointer(), sXMLText.GetLength());

		//	Write the element

		m_ContentElements[i]->WriteToStream(pStream);
		}

	//	Output any trailing text

	CString sXMLText = strToXMLText(m_ContentText[i], true);
	pStream->Write(sXMLText.GetASCIIZPointer(), sXMLText.GetLength());

	//	Close the element

	pStream->Write("</", 2);
	pStream->Write(m_sTag.GetASCIIZPointer(), m_sTag.GetLength());
	pStream->Write(">", 1);

	//	Done

	return NOERROR;
	}
