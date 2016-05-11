//	CSteamCtx.cpp
//
//	CSteamCtx class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

const AppId_t TRANSCENDENCE_APP_ID = 364510;

CSteamCtx::~CSteamCtx (void)

//  CSteamCtx destructor

    {
	if (m_bConnected)
		SteamAPI_Shutdown();
    }

bool CSteamCtx::Connect (CString *retsError)

//  Connect
//
//  Connect to Steam.

    {
    if (m_bConnected)
        return true;

	//	Boot up steam

	if (!SteamAPI_Init())
		{
        if (retsError) *retsError = CONSTLIT("Unable to initialize Steam API.");
		m_bConnected = false;
		return false;
		}

    //  We're enabled

	m_sUsername = CString(CString::csUTF8, SteamFriends()->GetPersonaName());
    m_bConnected = true;
    return true;
    }

bool CSteamCtx::CreateItem (PublishedFileId_t *retFileId, bool *retbAgreementRequired, CString *retsError)

//  CreateItem
//
//  Creates an item.

    {
    if (!ValidateConnected(retsError))
        return false;

    CUGCCreateItem Thunk;
    return Thunk.Call(TRANSCENDENCE_APP_ID, k_EWorkshopFileTypeCommunity, retFileId, retbAgreementRequired, retsError);
    }

bool CSteamCtx::ValidateConnected (CString *retsError)

//  ValidateConnected
//
//  Make sure we are connected

    {
    if (!m_bConnected)
        {
        if (retsError) *retsError = CONSTLIT("Not connected to Steam service.");
        return false;
        }

    return true;
    }
