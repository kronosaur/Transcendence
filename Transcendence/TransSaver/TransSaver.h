//	TransSaver.h
//
//	Main header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#ifndef INCL_TRANS_SAVER
#define INCL_TRANS_SAVER

//	Data Model -----------------------------------------------------------------

class CTransSaverModel
	{
	public:
		CTransSaverModel (CHumanInterface &HI) : m_HI(HI) { }
		~CTransSaverModel (void) { }

		void CleanUp (void);
		inline const CString &GetCopyright (void) { return m_Version.sCopyright; }
		inline const CString &GetProductName (void) { return m_Version.sProductName; }
		inline CUniverse &GetUniverse (void) { return m_Universe; }
		inline const CString &GetVersion (void) { return m_Version.sProductVersion; }
		ALERROR Init (void);
		ALERROR LoadUniverse (CString *retsError = NULL);

	private:
		CHumanInterface &m_HI;

		SFileVersionInfo m_Version;
		CUniverse m_Universe;
	};

//	Main Window Handler --------------------------------------------------------

class CTransSaverController : public IHIController
	{
	public:
		CTransSaverController (CHumanInterface &HI) : IHIController(HI),
				m_Model(HI) { }

		//	IHICommand virtuals
		virtual ALERROR OnBoot (char *pszCommandLine, SHIOptions &Options);
		virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);

	private:
		CTransSaverModel m_Model;
	};

//	Include concrete sessions and tasks

#include "Sessions.h"
#include "BackgroundTasks.h"

#endif