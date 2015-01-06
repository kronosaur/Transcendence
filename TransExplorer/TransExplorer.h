#pragma once

#include "resource.h"
#include "..\TSE\TSE.h"

//	Data Model -----------------------------------------------------------------

class CExplorerModel
	{
	public:
		CExplorerModel (CHumanInterface &HI) : m_HI(HI) { }
		~CExplorerModel (void) { }

		void CleanUp (void) { }
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

//	Controller -----------------------------------------------------------------

class CExplorerController : public IHIController
	{
	public:
		CExplorerController (CHumanInterface &HI) : IHIController(HI),
				m_Model(HI)
			{ }

		//	IHICommand virtuals
		virtual ALERROR OnBoot (char *pszCommandLine, SHIOptions &Options);
		virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (void);

	private:
		CExplorerModel m_Model;
	};

//	Background Tasks -----------------------------------------------------------

class CLoadUniverseTask : public IHITask
	{
	public:
		CLoadUniverseTask (CHumanInterface &HI, CExplorerModel &Model) : IHITask(HI), m_Model(Model) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Model.LoadUniverse(retsResult); }

	private:
		CExplorerModel &m_Model;
	};

