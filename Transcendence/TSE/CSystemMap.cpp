//	CSystemMap.cpp
//
//	CSystemMap class

#include "PreComp.h"

#define DEFINE_ZONE_TAG						CONSTLIT("DefineZone")
#define NODE_TAG							CONSTLIT("Node")
#define RANDOM_TOPOLOGY_TAG					CONSTLIT("RandomTopology")
#define ROOT_NODE_TAG						CONSTLIT("RootNode")
#define STARGATE_TAG						CONSTLIT("Stargate")
#define STARGATES_TAG						CONSTLIT("Stargates")
#define SYSTEM_DISTRIBUTION_TAG				CONSTLIT("SystemTypes")
#define SYSTEM_TOPOLOGY_TAG					CONSTLIT("SystemTopology")
#define TOPOLOGY_CREATOR_TAG				CONSTLIT("TopologyCreator")
#define TOPOLOGY_PROCESSOR_TAG				CONSTLIT("TopologyProcessor")
#define USES_TAG							CONSTLIT("Uses")

#define BACKGROUND_IMAGE_ATTRIB				CONSTLIT("backgroundImage")
#define DEBUG_SHOW_ATTRIBUTES_ATTRIB		CONSTLIT("debugShowAttributes")
#define PRIMARY_MAP_ATTRIB					CONSTLIT("displayOn")
#define GRADIENT_RANGE_ATTRIB				CONSTLIT("gradientRange")
#define ID_ATTRIB							CONSTLIT("id")
#define ID_PREFIX_ATTRIB					CONSTLIT("idPrefix")
#define INITIAL_SCALE_ATTRIB				CONSTLIT("initialScale")
#define MAX_SCALE_ATTRIB					CONSTLIT("maxScale")
#define MIN_SCALE_ATTRIB					CONSTLIT("minScale")
#define NAME_ATTRIB							CONSTLIT("name")
#define NODE_COUNT_ATTRIB					CONSTLIT("nodeCount")
#define PATTERN_ATTRIB						CONSTLIT("pattern")
#define STARGATE_PREFIX_ATTRIB				CONSTLIT("stargatePrefix")
#define STARTING_MAP_ATTRIB					CONSTLIT("startingMap")
#define UNID_ATTRIB							CONSTLIT("unid")
#define X_ATTRIB							CONSTLIT("x")
#define Y_ATTRIB							CONSTLIT("y")
#define ZONE_ATTRIB							CONSTLIT("zone")

#define CIRCULAR_PATTERN					CONSTLIT("circular")

const int MAX_NODE_PLACEMENT_LOOPS =		100;

CSystemMap::~CSystemMap (void)

//	CSystemMap destructor

	{
	int i;

	for (i = 0; i < m_Creators.GetCount(); i++)
		delete m_Creators[i];

	for (i = 0; i < m_Processors.GetCount(); i++)
		delete m_Processors[i];

	for (i = 0; i < m_Annotations.GetCount(); i++)
		m_Annotations[i].pPainter->Delete();
	}

void CSystemMap::AddAnnotation (CEffectCreator *pEffect, int x, int y, int iRotation, DWORD *retdwID)

//	AddAnnotation
//
//	Adds an annotation to the map. We take ownership of the painter

	{
	SMapAnnotation *pAnnotation = m_Annotations.Insert();
	pAnnotation->dwID = g_pUniverse->CreateGlobalID();
	pAnnotation->pPainter = pEffect->CreatePainter(CCreatePainterCtx());
	pAnnotation->xOffset = x;
	pAnnotation->yOffset = y;
	pAnnotation->iTick = 0;
	pAnnotation->iRotation = iRotation;

	//	Done

	if (retdwID)
		*retdwID = pAnnotation->dwID;
	}

ALERROR CSystemMap::AddFixedTopology (CTopology &Topology, CString *retsError)

//	AddFixedTopology
//
//	Adds all the nodes in its fixed topology

	{
	ALERROR error;
	int i;

	//	If we already added this map, then we're done

	if (m_bAdded)
		return NOERROR;

	//	Mark this map as added so we don't recurse back here when we
	//	process all the Uses statments.

	m_bAdded = true;

	//	Load all the maps that this map requires

	for (i = 0; i < m_Uses.GetCount(); i++)
		{
		if (error = m_Uses[i]->AddFixedTopology(Topology, retsError))
			return error;
		}

	//	Iterate over all creators and execute them

	CTopologyNodeList NodesAdded;
	STopologyCreateCtx Ctx;
	Ctx.pMap = GetDisplayMap();
	Ctx.pNodesAdded = &NodesAdded;

	//	We need to include any maps that we use.

	Ctx.Tables.Insert(&m_FixedTopology);
	for (i = 0; i < m_Uses.GetCount(); i++)
		Ctx.Tables.Insert(&m_Uses[i]->m_FixedTopology);

	for (i = 0; i < m_Creators.GetCount(); i++)
		{
		if (error = ExecuteCreator(Ctx, Topology, m_Creators[i]))
			{
			*retsError = strPatternSubst(CONSTLIT("SystemMap (%x): %s"), GetUNID(), Ctx.sError);
			return error;
			}
		}

	//	Add any additional nodes marked as "root" (this is here only for backwards compatibility)
	//	NOTE: This call only worries about the first table (Ctx.Tables[0])

	if (error = Topology.AddTopology(Ctx))
		{
		*retsError = strPatternSubst(CONSTLIT("SystemMap (%x): %s"), GetUNID(), Ctx.sError);
		return error;
		}

	//	Apply any topology processors (in order) on all the newly added nodes

	for (i = 0; i < m_Processors.GetCount(); i++)
		{
		//	Make a copy of the node list because each call will destroy it

		CTopologyNodeList NodeList = NodesAdded;

		//	Process

		if (error = m_Processors[i]->Process(this, Topology, NodeList, retsError))
			{
			*retsError = strPatternSubst(CONSTLIT("SystemMap (%x): %s"), GetUNID(), *retsError);
			return error;
			}
		}

	//	Make sure every node added has a system UNID

	for (i = 0; i < NodesAdded.GetCount(); i++)
		if (NodesAdded[i]->GetSystemTypeUNID() == 0)
			{
			*retsError = strPatternSubst(CONSTLIT("SystemMap (%x): NodeID %s: No system specified"), GetUNID(), NodesAdded[i]->GetID());
			return ERR_FAIL;
			}

	return NOERROR;
	}

CG16bitImage *CSystemMap::CreateBackgroundImage (void)

//	CreateBackgroundImage
//
//	Creates an image containing the map.
//	Caller is responsible for freeing the returned image.

	{
	int i;

	CG16bitImage *pImage = g_pUniverse->GetLibraryBitmapCopy(m_dwBackgroundImage);
	if (pImage == NULL)
		return NULL;

	int xCenter = pImage->GetWidth() / 2;
	int yCenter = pImage->GetHeight() / 2;

	//	Paint all annotations

	if (m_Annotations.GetCount() > 0)
		{
		SViewportPaintCtx Ctx;

		for (i = 0; i < m_Annotations.GetCount(); i++)
			{
			SMapAnnotation *pAnnotation = &m_Annotations[i];

			Ctx.iTick = pAnnotation->iTick;
			Ctx.iRotation = pAnnotation->iRotation;

			pAnnotation->pPainter->Paint(*pImage, xCenter + pAnnotation->xOffset, yCenter - pAnnotation->yOffset, Ctx);
			}
		}

#ifdef DEBUG
#if 0
	//	Test noise function
	CG16bitImage Alpha;
	Alpha.CreateBlankAlpha(pImage->GetWidth(), pImage->GetHeight());
	DrawNebulosity8bit(Alpha, 0, 0, pImage->GetWidth(), pImage->GetHeight(), 256, 0, 255);

	pImage->Fill(0, 0, pImage->GetWidth(), pImage->GetHeight(), 0);
	pImage->FillMask(0, 0, pImage->GetWidth(), pImage->GetHeight(), Alpha, CG16bitImage::RGBValue(255, 255, 255), 0, 0);
#endif
#endif

	return pImage;
	}

ALERROR CSystemMap::ExecuteCreator (STopologyCreateCtx &Ctx, CTopology &Topology, CXMLElement *pCreator)

//	ExecuteCreator
//
//	Runs a specific creator

	{
	ALERROR error;
	int i;

	//	If this is a root node tag then we add it and all its connections.

	if (strEquals(pCreator->GetTag(), ROOT_NODE_TAG))
		{
		if (error = Topology.AddTopologyNode(Ctx, pCreator->GetAttribute(ID_ATTRIB)))
			return error;
		}

	//	Otherwise we process the creator element

	else
		{
		for (i = 0; i < pCreator->GetContentElementCount(); i++)
			{
			CXMLElement *pDirective = pCreator->GetContentElement(i);

			if (strEquals(pDirective->GetTag(), NODE_TAG))
				{
				if (error = Topology.AddTopologyNode(Ctx, pDirective->GetAttribute(ID_ATTRIB)))
					return error;
				}
			else if (strEquals(pDirective->GetTag(), STARGATE_TAG) || strEquals(pDirective->GetTag(), STARGATES_TAG))
				{
				if (error = Topology.AddStargateFromXML(Ctx, pDirective))
					return error;
				}
			else
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unknown TopologyCreator directive: %s."), pDirective->GetTag());
				return ERR_FAIL;
				}
			}
		}

	return NOERROR;
	}

void CSystemMap::GetBackgroundImageSize (int *retcx, int *retcy)

//	GetBackgroundImageSize
//
//	Returns the size of the background image

	{
	CG16bitImage *pImage = g_pUniverse->GetLibraryBitmap(m_dwBackgroundImage);
	if (pImage)
		{
		*retcx = pImage->GetWidth();
		*retcy = pImage->GetHeight();
		}
	else
		{
		*retcx = 0;
		*retcy = 0;
		}
	}

ALERROR CSystemMap::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind the design

	{
	ALERROR error;
	int i;

	if (error = m_pPrimaryMap.Bind(Ctx))
		return error;

	for (i = 0; i < m_Uses.GetCount(); i++)
		if (error = m_Uses[i].Bind(Ctx))
			return error;

	if (error = m_FixedTopology.BindDesign(Ctx))
		return error;

	for (i = 0; i < m_Processors.GetCount(); i++)
		if (error = m_Processors[i]->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

ALERROR CSystemMap::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML

	{
	ALERROR error;
	int i;

	//	Load some basic info

	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_dwBackgroundImage = pDesc->GetAttributeInteger(BACKGROUND_IMAGE_ATTRIB);
	if (error = m_pPrimaryMap.LoadUNID(Ctx, pDesc->GetAttribute(PRIMARY_MAP_ATTRIB)))
		return error;

	m_bStartingMap = pDesc->GetAttributeBool(STARTING_MAP_ATTRIB);

	//	If we have a primary map, then add it to the Uses list.

	if (m_pPrimaryMap.GetUNID() != 0)
		m_Uses.Insert(m_pPrimaryMap);

	//	Scale information

	m_iInitialScale = pDesc->GetAttributeIntegerBounded(INITIAL_SCALE_ATTRIB, 10, 1000, 100);
	m_iMaxScale = pDesc->GetAttributeIntegerBounded(MAX_SCALE_ATTRIB, 100, 1000, 200);
	m_iMinScale = pDesc->GetAttributeIntegerBounded(MIN_SCALE_ATTRIB, 10, 100, 50);

	//	Generate an UNID

	CString sUNID = strPatternSubst(CONSTLIT("%d"), GetUNID());

	//	Keep track of root nodes

	TArray<CString> RootNodes;

	//	Iterate over all child elements and process them

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (strEquals(pItem->GetTag(), TOPOLOGY_CREATOR_TAG)
				|| strEquals(pItem->GetTag(), ROOT_NODE_TAG))
			{
			m_Creators.Insert(pItem->OrphanCopy());

			if (strEquals(pItem->GetTag(), ROOT_NODE_TAG))
				RootNodes.Insert(pItem->GetAttribute(ID_ATTRIB));
			}
		else if (strEquals(pItem->GetTag(), TOPOLOGY_PROCESSOR_TAG))
			{
			ITopologyProcessor *pNewProc;
			CString sProcessorUNID = strPatternSubst(CONSTLIT("%d:p%d"), GetUNID(), m_Processors.GetCount());

			if (error = ITopologyProcessor::CreateFromXMLAsGroup(Ctx, pItem, sProcessorUNID, &pNewProc))
				return error;

			m_Processors.Insert(pNewProc);
			}
		else if (strEquals(pItem->GetTag(), SYSTEM_TOPOLOGY_TAG))
			{
			if (error = m_FixedTopology.LoadFromXML(Ctx, pItem, this, sUNID, true))
				return error;
			}
		else if (strEquals(pItem->GetTag(), USES_TAG))
			{
			CSystemMapRef *pRef = m_Uses.Insert();

			if (error = pRef->LoadUNID(Ctx, pItem->GetAttribute(UNID_ATTRIB)))
				return error;
			}
		else
			{
			//	If it's none of the above, see if it is a node descriptor

			if (error = m_FixedTopology.LoadNodeFromXML(Ctx, pItem, this, sUNID))
				return error;
			}
		}

	//	Mark all the root nodes.
	//
	//	We need to do this for backwards compatibility because the old technique
	//	of having a root node with [Prev] for a stargate requires this. This was
	//	used by Huaramarca.

	for (i = 0; i < RootNodes.GetCount(); i++)
		if (error = m_FixedTopology.AddRootNode(Ctx, RootNodes[i]))
			return error;

	//	Init

	m_bAdded = false;

	//	Debug info

	m_bDebugShowAttributes = pDesc->GetAttributeBool(DEBUG_SHOW_ATTRIBUTES_ATTRIB);

	return NOERROR;
	}

CEffectCreator *CSystemMap::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Find the effect. We start after the map UNID.
//
//	{unid}/{nodeID}
//	{unid}:p{processorIndex}
//		  ^

	{
	//	We start after the map class UNID

	char *pPos = sUNID.GetASCIIZPointer();

	//	If we have a slash, then dive into the topology nodes

	if (*pPos == '/')
		{
		pPos++;
		return m_FixedTopology.FindEffectCreator(CString(pPos));
		}

	//	A colon means that it is some other element

	else if (*pPos == ':')
		{
		pPos++;

		//	'p' is a processor

		if (*pPos == 'p')
			{
			pPos++;

			//	Get the processor index

			int iIndex = ::strParseInt(pPos, -1, &pPos);
			if (iIndex < 0 || iIndex >= m_Processors.GetCount())
				return NULL;

			//	Let the processor handle the effect

			return m_Processors[iIndex]->FindEffectCreator(CString(pPos));
			}

		//	Otherwise, we don't know

		else
			return NULL;
		}

	//	Else, we don't know what it is

	else
		return NULL;
	}

void CSystemMap::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read the variable parts
//
//	DWORD		No. of annotations
//	DWORD		annotation: dwID
//	IEffectPainter	annotation: pPainter
//	DWORD		annotation: xOffset
//	DWORD		annotation: yOffset
//	DWORD		annotation: iTick
//	DWORD		annotation: iRotation

	{
	int i;
	DWORD dwLoad;

	if (Ctx.dwVersion < 9)
		return;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_Annotations.InsertEmpty(dwLoad);

	for (i = 0; i < m_Annotations.GetCount(); i++)
		{
		Ctx.pStream->Read((char *)&m_Annotations[i].dwID, sizeof(DWORD));

		//	To load the painter we need to cons up an SLoadCtx. Fortunately.
		//	we have the system version saved in the universe load ctx.

		SLoadCtx SystemCtx;
		SystemCtx.dwVersion = Ctx.dwSystemVersion;
		SystemCtx.pStream = Ctx.pStream;
		SystemCtx.pSystem = NULL;

		m_Annotations[i].pPainter = CEffectCreator::CreatePainterFromStream(SystemCtx);

		//	Load remaining fields

		Ctx.pStream->Read((char *)&m_Annotations[i].xOffset, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_Annotations[i].yOffset, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_Annotations[i].iTick, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_Annotations[i].iRotation, sizeof(DWORD));
		}
	}

void CSystemMap::OnReinit (void)

//	OnReinit
//
//	Reinitialize for a new game

	{
	m_bAdded = false;
	m_Annotations.DeleteAll();
	}

void CSystemMap::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write variable parts
//
//	DWORD		No. of annotations
//	DWORD		annotation: dwID
//	IEffectPainter	annotation: pPainter
//	DWORD		annotation: xOffset
//	DWORD		annotation: yOffset
//	DWORD		annotation: iTick
//	DWORD		annotation: iRotation

	{
	int i;
	DWORD dwSave;

	dwSave = m_Annotations.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_Annotations.GetCount(); i++)
		{
		pStream->Write((char *)&m_Annotations[i].dwID, sizeof(DWORD));
		CEffectCreator::WritePainterToStream(pStream, m_Annotations[i].pPainter);
		pStream->Write((char *)&m_Annotations[i].xOffset, sizeof(DWORD));
		pStream->Write((char *)&m_Annotations[i].yOffset, sizeof(DWORD));
		pStream->Write((char *)&m_Annotations[i].iTick, sizeof(DWORD));
		pStream->Write((char *)&m_Annotations[i].iRotation, sizeof(DWORD));
		}
	}

int KeyCompare (const CSystemMap::SSortEntry &Key1, const CSystemMap::SSortEntry &Key2)
	{
	if (Key1.iSort > Key2.iSort)
		return 1;
	else if (Key1.iSort < Key2.iSort)
		return -1;
	else
		return 0;
	}
