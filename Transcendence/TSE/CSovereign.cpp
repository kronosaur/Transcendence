//	CSovereign.cpp
//
//	CSovereign class

#include "PreComp.h"

#define LANGUAGE_TAG							CONSTLIT("Language")
#define RELATIONSHIPS_TAG						CONSTLIT("Relationships")
#define RELATIONSHIP_TAG						CONSTLIT("Relationship")

#define ALIGNMENT_ATTRIB						CONSTLIT("alignment")
#define ID_ATTRIB								CONSTLIT("id")
#define TEXT_ATTRIB								CONSTLIT("text")
#define SOVEREIGN_ATTRIB						CONSTLIT("sovereign")
#define DISPOSITION_ATTRIB						CONSTLIT("disposition")
#define MUTUAL_ATTRIB							CONSTLIT("mutual")

#define CONSTRUCTIVE_CHAOS_ALIGN				CONSTLIT("constructive chaos")
#define CONSTRUCTIVE_ORDER_ALIGN				CONSTLIT("constructive order")
#define NEUTRAL_ALIGN							CONSTLIT("neutral")
#define DESTRUCTIVE_ORDER_ALIGN					CONSTLIT("destructive order")
#define DESTRUCTIVE_CHAOS_ALIGN					CONSTLIT("destructive chaos")

#define DISP_FRIEND								CONSTLIT("friend")
#define DISP_NEUTRAL							CONSTLIT("neutral")
#define DISP_ENEMY								CONSTLIT("enemy")

#define FIELD_NAME								CONSTLIT("name")

static char *g_DefaultText[] =
	{
	//	0
	"",

	//	msgAttack
	"Attack target",

	//	msgDestroyBroadcast
	"",

	//	msgHitByFriendlyFire
	"",

	//	msgQueryEscortStatus
	"",

	//	msgQueryFleetStatus
	"",

	//	msgEscortAttacked
	"",

	//	msgEscortReportingIn
	"",

	//	msgWatchTargets
	"Watch your targets!",

	//	msgNiceShooting
	"Nice shooting!",

	//	msgFormUp
	"Form up!",

	//	msgBreakAndAttack
	"Break & attack",

	//	msgQueryCommunications
	"",

	//	msgAbortAttack
	"Cancel attack"
	};

#define DEFAULT_TEXT_COUNT					(sizeof(g_DefaultText) / sizeof(g_DefaultText[0]))

static CString *g_pDefaultText = NULL;

static char g_UNIDAttrib[] = "UNID";
static char g_NameAttrib[] = "name";

CSovereign::Disposition g_DispositionTable[5][5] =
	{
		//	WE are constructive chaos...
		{
			CSovereign::dispFriend,			//	THEY are constructive chaos
			CSovereign::dispFriend,			//	THEY are constructive order
			CSovereign::dispNeutral,		//	THEY are neutral
			CSovereign::dispEnemy,			//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},

		//	WE are constructive order...
		{
			CSovereign::dispNeutral,		//	THEY are constructive chaos
			CSovereign::dispFriend,			//	THEY are constructive order
			CSovereign::dispNeutral,		//	THEY are neutral
			CSovereign::dispEnemy,			//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},

		//	WE are neutral...
		{
			CSovereign::dispNeutral,		//	THEY are constructive chaos
			CSovereign::dispNeutral,		//	THEY are constructive order
			CSovereign::dispNeutral,		//	THEY are neutral
			CSovereign::dispNeutral,		//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},

		//	WE are destructive order...
		{
			CSovereign::dispEnemy,			//	THEY are constructive chaos
			CSovereign::dispEnemy,			//	THEY are constructive order
			CSovereign::dispNeutral,		//	THEY are neutral
			CSovereign::dispNeutral,		//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},

		//	WE are destructive chaos...
		{
			CSovereign::dispEnemy,			//	THEY are constructive chaos
			CSovereign::dispEnemy,			//	THEY are constructive order
			CSovereign::dispEnemy,			//	THEY are neutral
			CSovereign::dispEnemy,			//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},
	};

const DWORD ALIGN_FLAG_CONSTRUCTIVE =		0x00000001;
const DWORD ALIGN_FLAG_DESTRUCTIVE =		0x00000002;
const DWORD ALIGN_FLAG_COMMUNITY =			0x00000004;
const DWORD ALIGN_FLAG_INDEPENDENCE =		0x00000008;
const DWORD ALIGN_FLAG_KNOWLEDGE =			0x00000010;
const DWORD ALIGN_FLAG_SPIRITUALITY =		0x00000020;
const DWORD ALIGN_FLAG_EVOLUTION =			0x00000040;
const DWORD ALIGN_FLAG_TRADITION =			0x00000080;

struct SAlignData
	{
	int iNameLen;
	char pszName[32];
	DispositionClasses iDispClass;
	DWORD dwFlags;
	};

static SAlignData ALIGN_DATA[CSovereign::alignCount] =
	{
		{	CONSTDEF("republic"),		alignConstructiveChaos,		ALIGN_FLAG_CONSTRUCTIVE | ALIGN_FLAG_COMMUNITY | ALIGN_FLAG_KNOWLEDGE | ALIGN_FLAG_EVOLUTION },
		{	CONSTDEF("federation"),		alignConstructiveOrder,		ALIGN_FLAG_CONSTRUCTIVE | ALIGN_FLAG_COMMUNITY | ALIGN_FLAG_KNOWLEDGE | ALIGN_FLAG_TRADITION },
		{	CONSTDEF("uplifter"),		alignConstructiveChaos,		ALIGN_FLAG_CONSTRUCTIVE | ALIGN_FLAG_COMMUNITY | ALIGN_FLAG_SPIRITUALITY | ALIGN_FLAG_EVOLUTION },
		{	CONSTDEF("foundation"),		alignNeutral,				ALIGN_FLAG_CONSTRUCTIVE | ALIGN_FLAG_COMMUNITY | ALIGN_FLAG_SPIRITUALITY | ALIGN_FLAG_TRADITION },
		{	CONSTDEF("competitor"),		alignNeutral,				ALIGN_FLAG_CONSTRUCTIVE | ALIGN_FLAG_INDEPENDENCE | ALIGN_FLAG_KNOWLEDGE | ALIGN_FLAG_EVOLUTION },
		{	CONSTDEF("archivist"),		alignConstructiveOrder,		ALIGN_FLAG_CONSTRUCTIVE | ALIGN_FLAG_INDEPENDENCE | ALIGN_FLAG_KNOWLEDGE | ALIGN_FLAG_TRADITION },
		{	CONSTDEF("seeker"),			alignConstructiveChaos,		ALIGN_FLAG_CONSTRUCTIVE | ALIGN_FLAG_INDEPENDENCE | ALIGN_FLAG_SPIRITUALITY | ALIGN_FLAG_EVOLUTION },
		{	CONSTDEF("hermit"),			alignConstructiveOrder,		ALIGN_FLAG_CONSTRUCTIVE | ALIGN_FLAG_INDEPENDENCE | ALIGN_FLAG_SPIRITUALITY | ALIGN_FLAG_TRADITION },

		{	CONSTDEF("collective"),		alignDestructiveChaos,		ALIGN_FLAG_DESTRUCTIVE | ALIGN_FLAG_COMMUNITY | ALIGN_FLAG_KNOWLEDGE | ALIGN_FLAG_EVOLUTION },
		{	CONSTDEF("empire"),			alignDestructiveOrder,		ALIGN_FLAG_DESTRUCTIVE | ALIGN_FLAG_COMMUNITY | ALIGN_FLAG_KNOWLEDGE | ALIGN_FLAG_TRADITION },
		{	CONSTDEF("sterilizer"),		alignDestructiveChaos,		ALIGN_FLAG_DESTRUCTIVE | ALIGN_FLAG_COMMUNITY | ALIGN_FLAG_SPIRITUALITY | ALIGN_FLAG_EVOLUTION },
		{	CONSTDEF("corrector"),		alignDestructiveOrder,		ALIGN_FLAG_DESTRUCTIVE | ALIGN_FLAG_COMMUNITY | ALIGN_FLAG_SPIRITUALITY | ALIGN_FLAG_TRADITION },
		{	CONSTDEF("megalomaniac"),	alignDestructiveChaos,		ALIGN_FLAG_DESTRUCTIVE | ALIGN_FLAG_INDEPENDENCE | ALIGN_FLAG_KNOWLEDGE | ALIGN_FLAG_EVOLUTION },
		{	CONSTDEF("cryptologue"),	alignDestructiveOrder,		ALIGN_FLAG_DESTRUCTIVE | ALIGN_FLAG_INDEPENDENCE | ALIGN_FLAG_KNOWLEDGE | ALIGN_FLAG_TRADITION },
		{	CONSTDEF("perversion"),		alignDestructiveChaos,		ALIGN_FLAG_DESTRUCTIVE | ALIGN_FLAG_INDEPENDENCE | ALIGN_FLAG_SPIRITUALITY | ALIGN_FLAG_EVOLUTION },
		{	CONSTDEF("solipsist"),		alignDestructiveOrder,		ALIGN_FLAG_DESTRUCTIVE | ALIGN_FLAG_INDEPENDENCE | ALIGN_FLAG_SPIRITUALITY | ALIGN_FLAG_TRADITION },

		{	CONSTDEF("unorganized"),	alignNeutral,				0 },
		{	CONSTDEF("subsapient"),		alignNeutral,				0 },
		{	CONSTDEF("predator"),		alignDestructiveChaos,		0 },
	};

CSovereign::CSovereign (void) : 
		m_pEnemyObjectsSystem(NULL),
		m_pFirstRelationship(NULL),
		m_pInitialRelationships(NULL),
		m_bSelfRel(false)

//	CSovereign constructor

	{
	}

CSovereign::~CSovereign (void)

//	CSovereign destructor

	{
	if (m_pInitialRelationships)
		delete m_pInitialRelationships;

	DeleteRelationships();
	}

bool CSovereign::CalcSelfRel (void)

//	CalcSelfRel
//
//	Returns TRUE if there is a relationship that mentions ourselves

	{
	SRelationship *pRel = m_pFirstRelationship;
	while (pRel && pRel->pSovereign != this)
		pRel = pRel->pNext;

	return (pRel != NULL);
	}

void CSovereign::DeleteRelationships (void)

//	DeleteRelationships
//
//	Delete all relationships

	{
	SRelationship *pRel = m_pFirstRelationship;
	while (pRel)
		{
		SRelationship *pDelete = pRel;
		pRel = pRel->pNext;
		delete pDelete;
		}

	m_pFirstRelationship = NULL;
	m_bSelfRel = false;
	}

bool CSovereign::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	if (strEquals(sField, FIELD_NAME))
		*retsValue = m_sName;
	else
		return false;

	return true;
	}

CSovereign::SRelationship *CSovereign::FindRelationship (CSovereign *pSovereign, bool bCheckParent)

//	FindRelationship
//
//	Finds a specific relationship with the given sovereign (or NULL
//	if none is found)

	{
	SRelationship *pRel = m_pFirstRelationship;
	while (pRel && pRel->pSovereign != pSovereign)
		pRel = pRel->pNext;

	//	If we didn't find a relationship and we want to check our parent, then
	//	we see if they have a relationship.

	CSovereign *pInheritFrom;
	if (pRel == NULL 
			&& bCheckParent
			&& (pInheritFrom = CSovereign::AsType(GetInheritFrom())))
		pRel = pInheritFrom->FindRelationship(pSovereign, true);

	//	Done

	return pRel;
	}

CSovereign::Disposition CSovereign::GetDispositionTowards (CSovereign *pSovereign, bool bCheckParent)

//	GetDispositionTowards
//
//	Returns how this sovereign feels about the given sovereign

	{
	//	Optimize checks against ourselves

	if (!m_bSelfRel && pSovereign == this)
		return dispFriend;

	//	See if we have a specific relationship. If so, then return the
	//	disposition there.

	SRelationship *pRel = FindRelationship(pSovereign, bCheckParent);
	if (pRel)
		return pRel->iDisp;

	//	Get the alignment of the other. We treat NULL as Neutral

	DispositionClasses iOurDispClass = ALIGN_DATA[GetAlignment()].iDispClass;
	DispositionClasses iTheirDispClass = (pSovereign ? ALIGN_DATA[pSovereign->GetAlignment()].iDispClass : alignNeutral);

	//	Consult the table

	return g_DispositionTable[iOurDispClass][iTheirDispClass];
	}

CString CSovereign::GetText (MessageTypes iMsg)

//	GetText
//
//	Get the text associated with the given message

	{
	//	Look up the message in our data table

	CString sString;
	if (TranslateText(NULL, strFromInt(iMsg), NULL, &sString))
		return sString;

	//	If we don't already have it, load the default string array

	if (g_pDefaultText == NULL)
		{
		g_pDefaultText = new CString [DEFAULT_TEXT_COUNT];
		for (int i = 0; i < DEFAULT_TEXT_COUNT; i++)
			g_pDefaultText[i] = CString(g_DefaultText[i], lstrlen(g_DefaultText[i]), true);
		}

	//	Get default text

	if ((int)iMsg > 0 && (int)iMsg < DEFAULT_TEXT_COUNT)
		return g_pDefaultText[iMsg];
	else
		return g_pDefaultText[0];
	}

void CSovereign::InitEnemyObjectList (CSystem *pSystem)

//	InitEnemyObjectList
//
//	Compiles and caches a list of enemy objects in the system

	{
	int i;

	if (m_pEnemyObjectsSystem != pSystem)
		{
		m_EnemyObjects.SetAllocSize(pSystem->GetObjectCount());

		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);

			if (pObj 
					&& pObj->ClassCanAttack()
					&& !pObj->IsDestroyed()
					&& IsEnemy(pObj->GetSovereign()))
				m_EnemyObjects.FastAdd(pObj);
			}

		m_pEnemyObjectsSystem = pSystem;
		}
	}

void CSovereign::InitRelationships (void)

//	InitRelationships
//
//	Initialize relationships from XML element

	{
	DEBUG_TRY

	int i;

	if (m_pInitialRelationships)
		{
		for (i = 0; i < m_pInitialRelationships->GetContentElementCount(); i++)
			{
			CXMLElement *pRelDesc = m_pInitialRelationships->GetContentElement(i);
			CSovereign *pTarget = g_pUniverse->FindSovereign(pRelDesc->GetAttributeInteger(SOVEREIGN_ATTRIB));
			if (pTarget)
				{
				CString sDisposition = pRelDesc->GetAttribute(DISPOSITION_ATTRIB);
				Disposition iDisp;
				if (strEquals(sDisposition, DISP_FRIEND))
					iDisp = dispFriend;
				else if (strEquals(sDisposition, DISP_NEUTRAL))
					iDisp = dispNeutral;
				else if (strEquals(sDisposition, DISP_ENEMY))
					iDisp = dispEnemy;
				else
					iDisp = dispNeutral;

				SetDispositionTowards(pTarget, iDisp);

				//	If the disposition is mutual, set the target's disposition
				//	towards us

				if (pRelDesc->GetAttributeBool(MUTUAL_ATTRIB)
						&& pTarget != this)
					{
					Disposition iReverseDisp = pTarget->GetDispositionTowards(this, false);
					if (iReverseDisp != iDisp)
						{
						//	Check to see if we already have an explicit relationship
						//	If we don't, then set the reverse relationship;
						//	If the reverse disposition is neutral, then we override
						//	If the reverse disposition is friendly, then we override if we're enemies

						SRelationship *pRel = pTarget->FindRelationship(this);
						if (pRel == NULL
								|| iReverseDisp == dispNeutral
								|| (iReverseDisp == dispFriend && iDisp == dispEnemy))
							{
							pTarget->SetDispositionTowards(this, iDisp);
							}
						}
					}
				}
			}
		}

	DEBUG_CATCH
	}

void CSovereign::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Add types used by the sovereign.

	{
	int i;

	//	We only add explicit relationships

	if (m_pInitialRelationships)
		{
		for (i = 0; i < m_pInitialRelationships->GetContentElementCount(); i++)
			{
			CXMLElement *pRelDesc = m_pInitialRelationships->GetContentElement(i);
			CSovereign *pTarget = g_pUniverse->FindSovereign(pRelDesc->GetAttributeInteger(SOVEREIGN_ATTRIB));
			if (pTarget)
				retTypesUsed->SetAt(pTarget->GetUNID(), true);
			}
		}
	}

ALERROR CSovereign::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	InitRelationships();
	return NOERROR;
	}

ALERROR CSovereign::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	//	Initialize

	m_sName = pDesc->GetAttribute(CONSTLIT(g_NameAttrib));

	//	Alignment

	CString sAlignment = pDesc->GetAttribute(ALIGNMENT_ATTRIB);
	m_iAlignment = ParseAlignment(sAlignment);

	//	For backwards compatibility see if we have the old alignments

	if (m_iAlignment == alignUnknown)
		{
		if (strEquals(sAlignment, CONSTRUCTIVE_CHAOS_ALIGN))
			m_iAlignment = alignRepublic;
		else if (strEquals(sAlignment, CONSTRUCTIVE_ORDER_ALIGN))
			m_iAlignment = alignFederation;
		else if (strEquals(sAlignment, NEUTRAL_ALIGN))
			m_iAlignment = alignCompetitor;
		else if (strEquals(sAlignment, DESTRUCTIVE_ORDER_ALIGN))
			m_iAlignment = alignEmpire;
		else if (strEquals(sAlignment, DESTRUCTIVE_CHAOS_ALIGN))
			m_iAlignment = alignMegalomaniac;
		else
			return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unknown alignment: %s"), sAlignment));
		}

	//	Load relationships

	m_pInitialRelationships = pDesc->GetContentElementByTag(RELATIONSHIPS_TAG);
	if (m_pInitialRelationships)
		m_pInitialRelationships = m_pInitialRelationships->OrphanCopy();

	//	Done

	return NOERROR;
	}

ALERROR CSovereign::OnPrepareBindDesign (SDesignLoadCtx &Ctx)

//	OnPrepareBindDesign
//
//	Delete all our relationships.

	{
	DeleteRelationships();

	return NOERROR;
	}

void CSovereign::OnPrepareReinit (void)

//	OnPrepareReinit
//
//	Delete all our relationships. We need to do this as a first pass
//	because sometimes we add relationships from a different sovereign's init
//	(because of "mutual")

	{
	DeleteRelationships();
	}

void CSovereign::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	For each relationship:
//	DWORD		Unid of target relationship (or NULL if no more)
//	DWORD		Disposition
//	DWORD		Flags

	{
	DWORD dwUNID;
	DWORD dwLoad;

	DeleteRelationships();
	SRelationship **pNext = &m_pFirstRelationship;

	//	Keep reading until there are no more

	Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
	while (dwUNID)
		{
		SRelationship *pRel = new SRelationship;
		pRel->pSovereign = g_pUniverse->FindSovereign(dwUNID);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		pRel->iDisp = (Disposition)dwLoad;

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

		//	Add to list

		if (pRel->pSovereign)
			{
			(*pNext) = pRel;
			pNext = &pRel->pNext;
			}
		else
			delete pRel;

		//	Next

		Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
		}

	//	Terminate

	(*pNext) = NULL;

	m_bSelfRel = CalcSelfRel();
	}

void CSovereign::OnReinit (void)

//	OnReinit
//
//	Reinitialize

	{
	InitRelationships();
	}

void CSovereign::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes out the variable portions of the item type
//
//	For each relationship:
//	DWORD		Unid of target relationship (or NULL if no more)
//	DWORD		Disposition
//	DWORD		Flags

	{
	DWORD dwSave;

	SRelationship *pRel = m_pFirstRelationship;
	while (pRel)
		{
		dwSave = pRel->pSovereign->GetUNID();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		dwSave = pRel->iDisp;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		//	Flags
		dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pRel = pRel->pNext;
		}

	//	Write out end

	dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

CSovereign::Alignments CSovereign::ParseAlignment (const CString &sAlign)

//	ParseAlignment
//
//	Parse the string into an alignment

	{
	int i;

	for (i = 0; i < alignCount; i++)
		if (strEquals(sAlign, CString(ALIGN_DATA[i].pszName, ALIGN_DATA[i].iNameLen, true)))
			return (Alignments)i;

	return alignUnknown;
	}

void CSovereign::SetDispositionTowards (CSovereign *pSovereign, Disposition iDisp)

//	SetDispositionTowards
//
//	Sets the disposition towards the given sovereign

	{
	SRelationship *pRel = FindRelationship(pSovereign);
	if (pRel == NULL)
		{
		pRel = new SRelationship;
		pRel->pSovereign = pSovereign;
		pRel->pNext = m_pFirstRelationship;
		m_pFirstRelationship = pRel;
		}

	pRel->iDisp = iDisp;

	//	Flush cache of enemy objects

	FlushEnemyObjectCache();

	if (pSovereign == this)
		m_bSelfRel = true;
	}

