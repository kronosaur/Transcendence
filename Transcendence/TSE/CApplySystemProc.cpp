//	CApplySystemProc.cpp
//
//	CApplySystemProc class

#include "PreComp.h"

CApplySystemProc::~CApplySystemProc (void)

//	CApplySystemProc destructor

	{
	if (m_pSystemDesc)
		delete m_pSystemDesc;
	}

ALERROR CApplySystemProc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnInitFromXML
//
//	Initialize from XML element

	{
	//	Treat the content as a system definition

	m_pSystemDesc = pDesc->OrphanCopy();

	return NOERROR;
	}

ALERROR CApplySystemProc::OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError)

//	OnProcess
//
//	Process on topology

	{
	ALERROR error;
	int i;

	//	Apply system properties to all nodes in list

	for (i = 0; i < NodeList.GetCount(); i++)
		{
		if (error = NodeList.GetAt(i)->InitFromAdditionalXML(m_pSystemDesc, retsError))
			return error;
		}

	//	Remove from the original node list

	NodeList.DeleteAll();

	return NOERROR;
	}
