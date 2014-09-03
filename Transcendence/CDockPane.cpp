//	CDockPane.cpp
//
//	CDockPane class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

CDockPane::CDockPane (void) :
		m_pPaneDesc(NULL),
		m_pExtension(NULL),
		m_pData(NULL),
		m_pScreen(NULL),
		m_pContainer(NULL),
		m_bInShowPane(false)

//	CDockPane constructor

	{
	}

CDockPane::~CDockPane (void)

//	CDockPane destructor

	{
	CleanUp();
	}

void CDockPane::CleanUp (AGScreen *pScreen)

//	CleanUp
//
//	Clean up the pane

	{
	//	If we're still inside a screen, clean up our container

	if (pScreen && m_pContainer)
		pScreen->DestroyArea(m_pContainer);

	//	Clean Up

	m_Actions.CleanUp();
	m_Controls.DeleteAll();

	//	No need to free these fields because we don't own them

	m_pPaneDesc = NULL;
	m_pExtension = NULL;
	m_pData = NULL;
	m_pScreen = NULL;
	m_pContainer = NULL;
	m_bInShowPane = false;
	}

ALERROR CDockPane::InitPane (AGScreen *pScreen, CXMLElement *pPaneDesc, CExtension *pExtension, ICCItem *pData)

//	InitPane
//
//	Initializes the pane.

	{

	return NOERROR;
	}
