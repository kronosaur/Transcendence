//	CSteamCtx.cpp
//
//	CSteamCtx class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define FIELD_TYPE                              CONSTLIT("type")
#define FIELD_UNID                              CONSTLIT("unid")

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

bool CSteamCtx::GetOrCreateItem (DWORD dwUNID, EExtensionTypes iType, PublishedFileId_t *retFileId, bool *retbCreated, CString *retsError)

//  GetOrCreateItem
//
//  Looks for the given item by the current user and returns a fileId for it.
//  If necessary, we create it.

    {
    if (!ValidateConnected(retsError))
        return false;

    CString sUNID = ComposeUNID(dwUNID);

    //  Get a list of all items with the given UNID (there should only be one).

    TSortMap<CString, CString> Tags;
    Tags.Insert(FIELD_UNID, sUNID);

    TArray<SteamUGCDetails_t> List;
    if (!GetUserContent(Tags, List, retsError))
        return false;

    //  If we found some content, get the FileID

    PublishedFileId_t FileId;
    if (List.GetCount() > 0)
        {
        FileId = List[0].m_nPublishedFileId;
        if (retbCreated)
            *retbCreated = false;
        }

    //  Otherwise, we need to create a new item

    else
        {
        //  Create the item

        bool bAgreementRequired;
        if (!CreateItem(&FileId, &bAgreementRequired, retsError))
            return false;

        //  Associate an UNID with this item

        UGCUpdateHandle_t Update = SteamUGC()->StartItemUpdate(TRANSCENDENCE_APP_ID, FileId);
        if (!SteamUGC()->AddItemKeyValueTag(Update, (LPSTR)FIELD_UNID, (LPSTR)sUNID))
            return false;

		//	Set the type

		switch (iType)
			{
			case extAdventure:
				SteamUGC()->AddItemKeyValueTag(Update, (LPSTR)FIELD_TYPE, "transcendenceAdventure");
				break;

			case extExtension:
				SteamUGC()->AddItemKeyValueTag(Update, (LPSTR)FIELD_TYPE, "transcendenceExpansion");
				break;

			case extLibrary:
				SteamUGC()->AddItemKeyValueTag(Update, (LPSTR)FIELD_TYPE, "transcendenceLibrary");
				break;

			default:
				SteamUGC()->AddItemKeyValueTag(Update, (LPSTR)FIELD_TYPE, "transcendenceExtension");
				break;
			}

		//	Update

        if (!UpdateItem(Update, CONSTLIT("Extension created."), retsError))
            return false;

        if (retbCreated)
            *retbCreated = true;
        }

    //  Done

    if (retFileId)
        *retFileId = FileId;

    return true;
    }

bool CSteamCtx::GetUserContent (const TSortMap<CString, CString> &Tags, TArray<SteamUGCDetails_t> &Results, CString *retsError) const

//  GetUserContent
//
//  Returns a list of Transcendence workshop items for the currently signed in
//  user.

    {
    if (!ValidateConnected(retsError))
        return false;

    CUGCGetUserContent Thunk;
    return Thunk.Call(TRANSCENDENCE_APP_ID, GetUserAccountID(), k_EUserUGCList_Published, Tags, Results, retsError);
    }

bool CSteamCtx::UpdateItem (UGCUpdateHandle_t Update, const CString &sChangeNote, CString *retsError)

//  UpdateItem
//
//  Updates the item.

    {
    if (!ValidateConnected(retsError))
        return false;

    CUGCUpdateItem Thunk;
    return Thunk.Call(Update, sChangeNote, retsError);
    }

UGCUpdateHandle_t CSteamCtx::UpdateItemStart (PublishedFileId_t FileId) const

//  UpdateItemStart
//
//  Creates an update handle for the given item.

    {
    return SteamUGC()->StartItemUpdate(TRANSCENDENCE_APP_ID, FileId);
    }

bool CSteamCtx::ValidateConnected (CString *retsError) const

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
