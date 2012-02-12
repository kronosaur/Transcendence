//	CMapView.cpp
//
//	Implementation of the CMapView control

#include "Alchemy.h"
#include "NetUtil.h"
#include "Flatland.h"
#include "NullPointClient.h"

ALERROR CMapView::Create (CFView **retpView)
	{
	*retpView = new CMapView;
	if (*retpView == NULL)
		return ERR_MEMORY;

	return NOERROR;
	}

DWORD CMapView::Message (int iMsg, DWORD dwData1, const CString &sData2)
	{
	switch (iMsg)
		{
		case SetNPClient:
			m_pNP = (CNPClient *)dwData1;
			break;
		}

	return 0;
	}

void CMapView::OnCreate (CXMLElement *pDesc)

//	OnCreate
//
//	Called to initialize the subclass

	{
	}

void CMapView::OnPaint (HDC hDC, const RECT &rcUpdateRect)
	{
	gdiFillRect(hDC, &rcUpdateRect, RGB(0, 0, 0));
	}
