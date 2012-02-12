//	CLabyrinth.cpp
//
//	Labyrinth is a simple database system optimized to implement
//	the Grail Programming Language.

#include "Alchemy.h"
#include "Labyrinth.h"

CLabyrinth::CLabyrinth (IReportProgress *pProgress) : m_pProgress(pProgress),
		m_GlobalScope(NULL)
	{
	ASSERT(m_pProgress);
	}

CLabyrinth::~CLabyrinth (void)
	{
	}

ALERROR CLabyrinth::Boot (void)
	{
	ALERROR error;
	int i;

	//	Start by loading the grail environment.

	if (error = LoadFile(str_GrailEnvironmentFilename))
		return error;

	//	Get the entry that represents the Grail environment

	if (!m_GlobalScope.FindEntry(str_GEEntry, &m_pEnv))
		return SetErrorMsg(strPatternSubst(CONSTLIT("%s not defined"), str_GEEntry));

	//	Now we need to load all the modules specified by
	//	the environment.

	CLValue *pModules;
	if (!m_pEnv->FindField(str_GEModules, &pModules))
		return SetErrorMsg(strPatternSubst(CONSTLIT("Environment does not define '%s' parameter"), str_GEModules));

	for (i = 0; i < pModules->GetCount(); i++)
		{
		CString sModule = pModules->GetString(i);

		if (error = LoadFile(pModules->GetString(i)))
			return error;
		}

	return NOERROR;
	}

ALERROR CLabyrinth::LoadFile (const CString &sFilename)
	{
	ALERROR error;
	CString sError;

	CFileReadBlock FileToLoad(sFilename);

	if (error = FileToLoad.Open())
		return SetErrorMsg(strPatternSubst(CONSTLIT("Unable to open file: %s"), sFilename));

	Report(strPatternSubst(CONSTLIT("Loading %s"), sFilename));
	char *pPos = FileToLoad.GetPointer(0, -1);
	char *pEndPos = pPos + FileToLoad.GetLength();

	error = ParseEntries(pPos, pEndPos, &sError);

	FileToLoad.Close();
	if (error)
		return SetErrorMsg(strPatternSubst(CONSTLIT("%s%s"), sFilename, sError));

	return NOERROR;
	}

void CLabyrinth::Shutdown (void)
	{
	}
