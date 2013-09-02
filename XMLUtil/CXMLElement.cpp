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

ALERROR CXMLElement::AppendContent (const CString &sContent)

//	AppendContent
//
//	Appends some content

	{
	//	Always append to the last content element

	int iCount = m_ContentText.GetCount();
	if (iCount == 0)
		m_ContentText.Insert(sContent);
	else
		m_ContentText[iCount - 1].Append(sContent);

	return NOERROR;
	}

ALERROR CXMLElement::AppendSubElement (CXMLElement *pElement)

//	AppendSubElement
//
//	Append a sub element

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
