//	BackgroundTasks.h
//
//	Transcendence background tasks
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

class CInitModelTask : public IHITask
	{
	public:
		CInitModelTask (CHumanInterface &HI, CTranscendenceModel &Model) : IHITask(HI), m_Model(Model) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Model.InitBackground(retsResult); }

	private:
		CTranscendenceModel &m_Model;
	};

class CPostRecordTask : public IHITask
	{
	public:
		CPostRecordTask (CHumanInterface &HI, CCloudService &Service, const CGameRecord &Record) : IHITask(HI), m_Service(Service), m_Record(Record) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.PostGameRecord(pProcessor, m_Record, retsResult); }

	private:
		CCloudService &m_Service;
		CGameRecord m_Record;
	};

class CPostStatsTask : public IHITask
	{
	public:
		CPostStatsTask (CHumanInterface &HI, CCloudService &Service, const CGameStats &Stats) : IHITask(HI), m_Service(Service), m_Stats(Stats) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.PostGameStats(pProcessor, m_Stats, retsResult); }

	private:
		CCloudService &m_Service;
		CGameStats m_Stats;
	};

class CRegisterUserTask : public IHITask
	{
	public:
		CRegisterUserTask (CHumanInterface &HI, CCloudService &Service, const CString &sUsername, const CString &sPassword) : IHITask(HI), m_Service(Service), m_sUsername(sUsername), m_sPassword(sPassword) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.RegisterUser(pProcessor, m_sUsername, m_sPassword, retsResult); }

	private:
		CCloudService &m_Service;
		CString m_sUsername;
		CString m_sPassword;
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
		CSignInUserTask (CHumanInterface &HI, CCloudService &Service, const CString &sUsername, const CString &sPassword) : IHITask(HI), m_Service(Service), m_sUsername(sUsername), m_sPassword(sPassword) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Service.SignInUser(pProcessor, m_sUsername, m_sPassword, retsResult); }

	private:
		CCloudService &m_Service;
		CString m_sUsername;
		CString m_sPassword;
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

class CStartNewGameTask : public IHITask
	{
	public:
		CStartNewGameTask (CHumanInterface &HI, CTranscendenceModel &Model) : IHITask(HI), m_Model(Model) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Model.StartNewGameBackground(retsResult); }

	private:
		CTranscendenceModel &m_Model;
	};