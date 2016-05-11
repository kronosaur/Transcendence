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
        inline const CString &GetUsername (void) const { return m_sUsername; }

    private:
        bool ValidateConnected (CString *retsError = NULL);

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

