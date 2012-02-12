//	CS1Controller.cpp
//
//	CS1Controller class

#include "stdafx.h"
#include "S1.h"

#define NAME_ATTRIB									CONSTLIT("name")

bool CS1Controller::CmdEscape (void)
	{
	CXMLElement *pParent = m_pPos->GetParentElement();
	if (pParent == NULL)
		return false;

	m_pPos = pParent;
	return true;
	}

void CS1Controller::CmdSelectMenuItem (int iIndex)
	{
	m_pPos = m_pPos->GetContentElement(iIndex);
	}

int CS1Controller::GetMenuCount (void)
	{
	if (m_pPos)
		return m_pPos->GetContentElementCount();
	else
		return 0;
	}

CString CS1Controller::GetMenuTitle (void)
	{
	if (m_pPos)
		return m_pPos->GetAttribute(NAME_ATTRIB);
	else
		return NULL_STR;
	}

CString CS1Controller::GetMenuItemLabel (int iIndex)
	{
	if (m_pPos)
		return m_pPos->GetContentElement(iIndex)->GetAttribute(NAME_ATTRIB);
	else
		return NULL_STR;
	}

ALERROR CS1Controller::LoadGuide (void)

//	LoadGuide
//
//	Loads the guide from disk

	{
	ALERROR error;

	CString sError;
	CFileReadBlock Input(CONSTLIT("Guide.xml"));
	if (error = CXMLElement::ParseXML(&Input, &m_pGuide, &sError))
		return error;

	m_pPos = m_pGuide;

	return NOERROR;
	}

void CS1Controller::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up the object

	{
	if (m_pGuide)
		delete m_pGuide;
	}

ALERROR CS1Controller::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	return NOERROR;
	}

ALERROR CS1Controller::OnInit (void)

//	OnInit
//
//	Handle initialization

	{
	ALERROR error;

	if (error = LoadGuide())
		return error;

	if (error = m_HI.ShowSession(new CMenuSession(m_HI)))
		return error;

	return NOERROR;
	}

