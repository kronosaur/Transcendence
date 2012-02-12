//	BackgroundTasks.h
//
//	Background tasks
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

class CLoadUniverseTask : public IHITask
	{
	public:
		CLoadUniverseTask (CHumanInterface &HI, CTransSaverModel &Model) : IHITask(HI), m_Model(Model) { }

		//	IHITask virtuals
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return m_Model.LoadUniverse(retsResult); }

	private:
		CTransSaverModel &m_Model;
	};

