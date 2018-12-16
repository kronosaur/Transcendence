//	BackgroundTasks.h
//
//	Transcendence background tasks
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

class CChangePasswordTask : public IHITask
	{
	public:
		CChangePasswordTask (CHumanInterface &HI, CCloudService &Service, const CString &sUsername, const CString &sOldPassword, const CString &sNewPassword) : IHITask(HI), m_Service(Service), m_sUsername(sUsername), m_sOldPassword(sOldPassword), m_sNewPassword(sNewPassword) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.ChangePassword(pProcessor, m_sUsername, m_sOldPassword, m_sNewPassword, retsResult); }

	private:
		CCloudService &m_Service;
		CString m_sUsername;
		CString m_sOldPassword;
		CString m_sNewPassword;
	};

class CInitAdventureTask : public IHITask
	{
	public:
		CInitAdventureTask (CHumanInterface &HI, CTranscendenceModel &Model, const SAdventureSettings &Settings) : IHITask(HI), m_Model(Model), m_Settings(Settings) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Model.InitAdventure(m_Settings, retsResult); }

	private:
		CTranscendenceModel &m_Model;
		SAdventureSettings m_Settings;
	};

class CInitModelTask : public IHITask
	{
	public:
		CInitModelTask (CHumanInterface &HI, 
						CTranscendenceModel &Model,
						const CGameSettings &Settings,
						const CString &sCollectionFolder, 
						const TArray<CString> &ExtensionFolders) : IHITask(HI), 
				m_Model(Model),
				m_Settings(Settings),
				m_sCollectionFolder(sCollectionFolder),
				m_ExtensionFolders(ExtensionFolders)
			{ }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Model.InitBackground(m_Settings, m_sCollectionFolder, m_ExtensionFolders, retsResult); }

	private:
		CTranscendenceModel &m_Model;
		const CGameSettings &m_Settings;
		CString m_sCollectionFolder;
		TArray<CString> m_ExtensionFolders;
	};

class CLoadExtensionTask : public IHITask
	{
	public:
		CLoadExtensionTask (CHumanInterface &HI, const CHexarcDownloader::SStatus &Status) : IHITask(HI), 
				m_Status(Status)
			{ }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult)
			{
			//	Status

			m_HI.HIPostCommand(CONSTLIT("serviceStatus"), new CString(strPatternSubst(CONSTLIT("Loading %s..."), pathGetFilename(m_Status.sFilespec))));

			//	Load the extension, but lock out the UI thread so that we don't
			//	screw up the data structures.
			//
			//	NOTE: LoadNewExtension is protected by try/catch, so it will always return 
			//	correctly.

			g_pUniverse->GetSem().Lock();
			ALERROR error = g_pUniverse->LoadNewExtension(m_Status.sFilespec, m_Status.FileDigest, retsResult);
			g_pUniverse->GetSem().Unlock();

			if (error)
				{
				m_HI.HIPostCommand(CONSTLIT("serviceError"), new CString(*retsResult));
				return ERR_FAIL;
				}

			m_HI.HIPostCommand(CONSTLIT("serviceStatus"), NULL);
			m_HI.HIPostCommand(CONSTLIT("serviceExtensionLoaded"), NULL);

			return NOERROR;
			}

	private:
		CHexarcDownloader::SStatus m_Status;
	};

class CLoadGameTask : public IHITask
	{
	public:
		CLoadGameTask (CHumanInterface &HI, CTranscendenceModel &Model, const CString &sUsername, const CString &sFilespec) : IHITask(HI), m_Model(Model), m_sUsername(sUsername), m_sFilespec(sFilespec) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Model.LoadGame(m_sUsername, m_sFilespec, retsResult); }

	private:
		CTranscendenceModel &m_Model;
		CString m_sUsername;
		CString m_sFilespec;
	};

class CLoadGameWithSignInTask : public IHITask
	{
	public:
		CLoadGameWithSignInTask (CHumanInterface &HI, CCloudService &Service, CTranscendenceModel &Model, const CString &sFilespec) : IHITask(HI), m_Service(Service), m_Model(Model), m_sFilespec(sFilespec) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult);

	private:
		CCloudService &m_Service;
		CTranscendenceModel &m_Model;
		CString m_sFilespec;
	};

class CLoadNewsTask : public IHITask
	{
	public:
		CLoadNewsTask (CHumanInterface &HI, CCloudService &Service, CMultiverseModel &Multiverse, const SFileVersionInfo &AppVersion, const CString &sCacheFilespec) : IHITask(HI), 
				m_Service(Service), 
				m_Multiverse(Multiverse),
				m_AppVersion(AppVersion),
				m_sCacheFilespec(sCacheFilespec)
			{ }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult)
			{
			::kernelDebugLogPattern("Loading news.");
			return m_Service.LoadNews(pProcessor, m_Multiverse, m_AppVersion, m_sCacheFilespec, retsResult); 
			}

	private:
		CCloudService &m_Service;
		CMultiverseModel &m_Multiverse;
		SFileVersionInfo m_AppVersion;
		CString m_sCacheFilespec;
	};

class CLoadUserCollectionTask : public IHITask
	{
	public:
		CLoadUserCollectionTask (CHumanInterface &HI, CCloudService &Service, CMultiverseModel &Multiverse, CExtensionCollection &Extensions) : IHITask(HI),
				m_Service(Service), 
				m_Multiverse(Multiverse),
				m_Extensions(Extensions)
			{ }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult)
			{
			CSmartLock Lock(g_pUniverse->GetSem());
			::kernelDebugLogPattern("Loading user collection.");
			return m_Service.LoadUserCollection(pProcessor, m_Extensions, m_Multiverse, retsResult); 
			}

	private:
		CCloudService &m_Service;
		CExtensionCollection &m_Extensions;
		CMultiverseModel &m_Multiverse;
	};

class CPostCrashReportTask : public IHITask
	{
	public:
		CPostCrashReportTask (CHumanInterface &HI, CCloudService &Service, const CString &sCrash) : IHITask(HI), m_Service(Service), m_sCrash(sCrash) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.PostCrashReport(pProcessor, m_sCrash, retsResult); }

	private:
		CCloudService &m_Service;
		CString m_sCrash;
	};

class CPostRecordTask : public IHITask
	{
	public:
		CPostRecordTask (CHumanInterface &HI, CCloudService &Service, const CGameRecord &Record, const CGameStats &Stats) : IHITask(HI), m_Service(Service), m_Record(Record), m_Stats(Stats) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.PostGameRecord(pProcessor, m_Record, m_Stats, retsResult); }

	private:
		CCloudService &m_Service;
		CGameRecord m_Record;
		CGameStats m_Stats;
	};

class CProcessDownloadsTask : public IHITask
	{
	public:
		CProcessDownloadsTask (CHumanInterface &HI, CCloudService &Service) : IHITask(HI), m_Service(Service) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.ProcessDownloads(pProcessor, retsResult); }

	private:
		CCloudService &m_Service;
	};

class CReadHighScoreListTask : public IHITask
	{
	public:
		CReadHighScoreListTask (CHumanInterface &HI, CCloudService &Service, const CAdventureHighScoreList::SSelect &Select) : IHITask(HI), 
				m_Service(Service), 
				m_Select(Select)
			{ }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult)
			{
			ALERROR error;

			CAdventureHighScoreList *pHighScoreList = new CAdventureHighScoreList;
			if (error = m_Service.ReadHighScoreList(pProcessor, m_Select.dwAdventure, pHighScoreList, retsResult))
				{
				m_HI.HIPostCommand(CONSTLIT("serviceHighScoreListError"));
				delete pHighScoreList;
				return error;
				}

			pHighScoreList->SetSelection(m_Select.sUsername, m_Select.iScore);
			m_HI.HIPostCommand(CONSTLIT("serviceHighScoreListLoaded"), pHighScoreList);

			return NOERROR;
			}

	private:
		CCloudService &m_Service;
		CAdventureHighScoreList::SSelect m_Select;
	};

class CRegisterUserTask : public IHITask
	{
	public:
		CRegisterUserTask (CHumanInterface &HI, CCloudService &Service, const CString &sUsername, const CString &sPassword, const CString &sEmail, bool bAutoSignIn) : IHITask(HI), m_Service(Service), m_sUsername(sUsername), m_sPassword(sPassword), m_sEmail(sEmail), m_bAutoSignIn(bAutoSignIn) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.RegisterUser(pProcessor, m_sUsername, m_sPassword, m_sEmail, m_bAutoSignIn, retsResult); }

	private:
		CCloudService &m_Service;
		CString m_sUsername;
		CString m_sPassword;
		CString m_sEmail;
		bool m_bAutoSignIn;
	};

class CServiceHousekeepingTask : public IHITask
	{
	public:
		CServiceHousekeepingTask (CHumanInterface &HI, CCloudService &Service) : IHITask(HI), m_Service(Service) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.Housekeeping(pProcessor); }

	private:
		CCloudService &m_Service;
	};

class CSignInUserTask : public IHITask
	{
	public:
		CSignInUserTask (CHumanInterface &HI, CCloudService &Service, const CString &sUsername, const CString &sPassword, bool bAutoSignIn) : IHITask(HI), m_Service(Service), m_sUsername(sUsername), m_sPassword(sPassword), m_bAutoSignIn(bAutoSignIn) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.SignInUser(pProcessor, m_sUsername, m_sPassword, m_bAutoSignIn, retsResult); }

	private:
		CCloudService &m_Service;
		CString m_sUsername;
		CString m_sPassword;
		bool m_bAutoSignIn;
	};

class CSignOutUserTask : public IHITask
	{
	public:
		CSignOutUserTask (CHumanInterface &HI, CCloudService &Service) : IHITask(HI), m_Service(Service) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.SignOutUser(pProcessor, retsResult); }

	private:
		CCloudService &m_Service;
	};

class CStartGameTask : public IHITask
	{
	public:
		CStartGameTask (CHumanInterface &HI, CTranscendenceModel &Model, bool bNewGame) : IHITask(HI), m_Model(Model), m_bNewGame(bNewGame) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { m_Model.StartGame(m_bNewGame); return NOERROR; }

	private:
		CTranscendenceModel &m_Model;
		bool m_bNewGame;
	};

class CStartNewGameTask : public IHITask
	{
	public:
		CStartNewGameTask (CHumanInterface &HI, CTranscendenceModel &Model, const SNewGameSettings &Settings) : IHITask(HI), m_Model(Model), m_NewGame(Settings) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Model.StartNewGameBackground(m_NewGame, retsResult); }

	private:
		CTranscendenceModel &m_Model;
		SNewGameSettings m_NewGame;
	};

class CTravelThroughStargate : public IHITask
	{
	public:
		CTravelThroughStargate (CHumanInterface &HI, CTranscendenceModel &Model) : IHITask(HI), m_Model(Model) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { m_Model.OnPlayerTraveledThroughGate(); return NOERROR; }

	private:
		CTranscendenceModel &m_Model;
	};

class CUpgradeProgram : public IHITask
	{
	public:
		CUpgradeProgram (CHumanInterface &HI, CCloudService &Service, const CString &sUpgradeURL) : IHITask(HI), 
				m_Service(Service),
				m_sUpgradeURL(sUpgradeURL)
			{ }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) 
			{
			::kernelDebugLogPattern("Upgrading Transcendence.");
			return m_Service.DownloadUpgrade(pProcessor, m_sUpgradeURL, retsResult); 
			}

	private:
		CCloudService &m_Service;
		CString m_sUpgradeURL;
	};
