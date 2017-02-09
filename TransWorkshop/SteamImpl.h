//  SteamImpl.h
//
//  Implementation of Steam objects
//  Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "steam/steam_api.h"

class CSteamCtx
    {
    public:
        CSteamCtx (void) :
                m_bConnected(false)
            { }
        ~CSteamCtx (void);

        bool Connect (CString *retsError = NULL);
        bool CreateItem (PublishedFileId_t *retFileId, bool *retbAgreementRequired, CString *retsError = NULL);
        bool GetOrCreateItem (DWORD dwUNID, EExtensionTypes iType, PublishedFileId_t *retFileId, bool *retbCreated, CString *retsError = NULL);
        inline AccountID_t GetUserAccountID (void) const { return SteamUser()->GetSteamID().GetAccountID(); }
        bool GetUserContent (const TSortMap<CString, CString> &Tags, TArray<SteamUGCDetails_t> &Results, CString *retsError = NULL) const;
        inline const CString &GetUsername (void) const { return m_sUsername; }
        bool UpdateItem (UGCUpdateHandle_t Update, const CString &sChangeNote, CString *retsError = NULL);
        UGCUpdateHandle_t UpdateItemStart (PublishedFileId_t FileId) const;

    private:
        inline CString ComposeUNID (DWORD dwUNID) const { return strPatternSubst(CONSTLIT("Transcendence:%08x"), dwUNID); }
        bool ValidateConnected (CString *retsError = NULL) const;

        bool m_bConnected;
        CString m_sUsername;
    };

//	Call Thunks ----------------------------------------------------------------

template <class HANDLERTYPE, class RESULTTYPE> class TCallContext
	{
	protected:
		bool Invoke (SteamAPICall_t hSteamAPICall, CString *retsError = NULL)
			{
			m_CallResult.Set(hSteamAPICall, (HANDLERTYPE *)this, &TCallContext::Handler);

			while (m_CallResult.IsActive())
				{
				::Sleep(50);
				SteamAPI_RunCallbacks();
				}

            if (!m_bSuccess && retsError)
                *retsError = m_sError;

			return m_bSuccess;
			}

        void Handler (RESULTTYPE *pResult, bool bIOFailure)
            {
            if (bIOFailure)
                {
                m_bSuccess = false;
                m_sError = CONSTLIT("Unable to communicate with Steam service.");
                }
            else if (pResult->m_eResult != k_EResultOK)
                {
                m_bSuccess = false;
                switch (pResult->m_eResult)
                    {
                    case k_EResultInsufficientPrivilege:
                        m_sError = CONSTLIT("Sorry, you are not authorized to do that.");
                        break;

                    case k_EResultTimeout:
                        m_sError = CONSTLIT("Sorry, the operation timed out.");
                        break;

                    case k_EResultNotLoggedOn:
                        m_sError = CONSTLIT("Please log on to Steam and try again.");
                        break;

                    default:
                        m_sError = CONSTLIT("Unable to comply.");
                        break;
                    }
                }
            else
                {
                m_bSuccess = true;
                ((HANDLERTYPE *)this)->OnResult(pResult);
                }
            }

		CCallResult<HANDLERTYPE, RESULTTYPE> m_CallResult;
		bool m_bSuccess;
        CString m_sError;
	};

class CUGCCreateItem : public TCallContext<CUGCCreateItem, CreateItemResult_t>
	{
	public:
		bool Call (AppId_t nAppId, EWorkshopFileType eFileType, PublishedFileId_t *retFileId, bool *retbAgreementRequired, CString *retsError = NULL)
			{
            m_retFileId = retFileId;
            m_retbAgreementRequired = retbAgreementRequired;

            return Invoke(SteamUGC()->CreateItem(nAppId, eFileType), retsError);
			}

	private:
		void OnResult (CreateItemResult_t *pResult)
			{
            *m_retFileId = pResult->m_nPublishedFileId;
            *m_retbAgreementRequired = pResult->m_bUserNeedsToAcceptWorkshopLegalAgreement;
			}

        PublishedFileId_t *m_retFileId;
        bool *m_retbAgreementRequired;

    friend TCallContext;
	};

class CUGCUpdateItem : public TCallContext<CUGCUpdateItem, SubmitItemUpdateResult_t>
    {
    public:
        bool Call (UGCUpdateHandle_t Update, const CString &sChangeNote, CString *retsError)
            {
            return Invoke(SteamUGC()->SubmitItemUpdate(Update, (LPSTR)sChangeNote), retsError);
            }

    private:
        void OnResult (SubmitItemUpdateResult_t *pResult)
            {
            }

    friend TCallContext;
    };

class CUGCGetUserContent : public TCallContext<CUGCGetUserContent, SteamUGCQueryCompleted_t>
    {
    public:
        bool Call (AppId_t nAppId, AccountID_t unAccountID, EUserUGCList eListType, const TSortMap<CString, CString> &Tags, TArray<SteamUGCDetails_t> &Results, CString *retsError = NULL)
            {
            int i;

            //  Store results here

            Results.DeleteAll();
            m_pResults = &Results;

            //  Loop until we've got all results
            
            m_bDone = false;
            int iPage = 1;
            while (!m_bDone)
                {
                m_Query = SteamUGC()->CreateQueryUserUGCRequest(unAccountID, 
                        eListType, 
                        k_EUGCMatchingUGCType_Items, 
                        k_EUserUGCListSortOrder_CreationOrderDesc, 
                        nAppId, 
                        nAppId, 
                        iPage);

                //  Add required tags

                for (i = 0; i < Tags.GetCount(); i++)
                    SteamUGC()->AddRequiredKeyValueTag(m_Query, Tags.GetKey(i), Tags[i]);

                //  Invoke

                if (!Invoke(SteamUGC()->SendQueryUGCRequest(m_Query), retsError))
                    {
                    SteamUGC()->ReleaseQueryUGCRequest(m_Query);
                    return false;
                    }

                SteamUGC()->ReleaseQueryUGCRequest(m_Query);
                iPage++;
                }

            //  Done

            return true;
            }

    private:
        void OnResult (SteamUGCQueryCompleted_t *pResult)
            {
            DWORD i;

            if (m_pResults->GetCount() == 0)
                m_pResults->GrowToFit(pResult->m_unTotalMatchingResults);

            for (i = 0; i < pResult->m_unNumResultsReturned; i++)
                {
                SteamUGCDetails_t *pDetails = m_pResults->Insert();
                if (!SteamUGC()->GetQueryUGCResult(m_Query, i, pDetails))
                    {
                    //  LATER: Mark as an error.
                    m_bDone = true;
                    return;
                    }
                }

            //  If we've got all details, then we're done

            m_bDone = ((DWORD)m_pResults->GetCount() >= pResult->m_unTotalMatchingResults);
            }

        TArray<SteamUGCDetails_t> *m_pResults;
        UGCQueryHandle_t m_Query;
        bool m_bDone;

    friend TCallContext;
    };