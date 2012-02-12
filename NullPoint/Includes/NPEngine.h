//	NPEngine.h
//
//	Header file for the NullPoint engine

#ifndef INCL_NPENGINE
#define INCL_NPENGINE

#ifndef INCL_NETUTIL
#include "NetUtil.h"
#endif

#ifndef INCL_XMLUTIL
#include "XMLUtil.h"
#endif

//	Universal constants

const int SecondsPerOro =				54;
const int OrosPerDia =					16;
const int DiasPerAnno =					100;

//	Errors

const ALERROR NPERROR_FLEET_IN_TRANSIT =			ERR_APPL + 1;	//	Fleet is in transit
const ALERROR NPERROR_FLEET_NO_HYPERDRIVE =			ERR_APPL + 2;	//	Fleet has no hyperdrive ships

//	Forwards

class CNPAssetList;
class CNPNullPoint;
class CNPSovereign;
class CNPUniverse;

//	UNIDs
//
//	UNIDs are used to identify all objects in the universe.

enum UNIDType
	{
	unidNullPoint = 0x00,
	unidLink = 0x01,
	unidWorld = 0x02,
	unidFleet = 0x03,
	unidSovereign = 0x04,
	unidTrait = 0x05,
	unidResourceClass = 0x06,
	unidUnitClass = 0x07,

	unidReserved = 0xff
	};

const DWORD NullUNID = 0xffffffff;

inline DWORD MakeUNID (UNIDType dwType, DWORD dwOrdinal)
	{ return (dwType << 24) | dwOrdinal; }
inline DWORD GetUNIDType (DWORD dwUNID)
	{ return (dwUNID >> 24); }
inline DWORD GetUNIDOrdinal (DWORD dwUNID)
	{ return (dwUNID & 0x00ffffff); }

template <UNIDType t_utype, class t_entryclass> class CEntryTable
	{
	public:
		CEntryTable (void) : m_IDTable(true, false), m_dwNextID(1)
			{
			}

		bool FindEntry (DWORD dwUNID, t_entryclass **retpObj)
			{
			CObject *pObj;

			if (m_IDTable.Lookup(dwUNID, &pObj) != NOERROR)
				return false;

			*retpObj = (t_entryclass *)pObj;
			return true;
			}

		inline int GetCount (void) { return m_IDTable.GetCount(); }

		inline t_entryclass *GetEntry (DWORD dwUNID)
			{
			t_entryclass *pEntry;
			if (FindEntry(dwUNID, &pEntry))
				return pEntry;
			else
				return NULL;
			}

		inline t_entryclass *GetEntryByIndex (int iIndex)
			{ return (t_entryclass *)m_IDTable.GetValue(iIndex); }

		inline DWORD RegisterEntry (void)
			{ return MakeUNID(t_utype, m_dwNextID++); }

		inline void RemoveEntry (DWORD dwUNID)
			{ m_IDTable.RemoveEntry(dwUNID, NULL); }

		inline void SetEntry (DWORD dwUNID, t_entryclass *pObj)
			{ m_IDTable.ReplaceEntry(dwUNID, pObj, true, NULL); }

	private:
		CIDTable m_IDTable;
		DWORD m_dwNextID;
	};

class CEntryNameTable
	{
	public:
		CEntryNameTable (void);

		CString GetEntryName (DWORD dwUNID);
		void NameEntry (DWORD dwUNID, const CString &sName);
		DWORD TranslateName (const CString &sName);

	private:
		CIDTable m_IDTable;
	};

//	Abstract concepts

enum NPTechLevels
	{
	techNone			= 0,
	techAgricultural	= 1,
	techSteam			= 2,
	techCyber			= 3,
	techBio				= 4,
	techFusion			= 5,
	techAI				= 6,
	techPteracnium		= 7,
	techNano			= 8,
	techDyson			= 9,
	techQuantum			= 10,
	techGraviton		= 11,
	techDarkMatter		= 12,

	techCount			= 13
	};

enum DefaultTraits
	{
	traitReservedCapital		= 0,	//	[WORLD]: May be turned into a capital
	traitCapital				= 1,	//	[WORLD]: World is a capital
	traitNeothals				= 2,	//	[WORLD]: World has neothal population
	traitLowGravity				= 3,	//	[WORLD]: Low gravity world
	traitGasGiantMoon			= 4,	//	[WORLD]: Moon of a gas giant
	traitFertile				= 5,	//	[WORLD]: Fertile lands
	traitRings					= 6,	//	[WORLD]: Ring system
	traitTropical				= 7,	//	[WORLD]: Hot, humid, jungle
	traitVolcanic				= 8,	//	[WORLD]: Heavy volcanic activity
	traitDesert					= 9,	//	[WORLD]: Desert
	traitHighGravity			= 10,	//	[WORLD]: High gravity world
	traitIonized				= 11,	//	[WORLD]: Area is ionized
	traitPermafrost				= 12,	//	[WORLD]: Snow, glaciers
	traitOcean					= 13,	//	[WORLD]: Ocean world
	traitAirless				= 14,	//	[WORLD]: No atmosphere
	traitRadioactive			= 15,	//	[WORLD]: Radioactive
	traitFreighter				= 16,	//	[UNIT]: Ship is a cargo ship
	traitPlanetfall				= 17,	//	[UNIT]: Ship can land on planet
	traitInfantry				= 18,	//	[UNIT]: Unit is an infantry regiment
	traitSpaces					= 19,	//	[UNIT]: Unit is a spacer regiment
	traitImperial				= 20,	//	[UNIT,STRUCT]: Unit is an imperial design
	traitAutonomous				= 21,	//	[UNIT]: Unit is unmanned
	traitOrbital				= 22,	//	[STRUCT]: Structure orbits the planet
	};

class CNPResource
	{
	public:
		CNPResource (DWORD dwUNID, int iAmount);

		inline DWORD GetClassUNID (void) { return MakeUNID(unidResourceClass, (m_dwData >> 24)); }
		inline int GetAmount (void) { return (int)((m_dwData & 0x00ffffff) >> 8); }
		int Modify (int iChange);

	private:
		CNPResource (void) { }
		static CNPResource CreateFromInt (int iData);
		inline int GetAsInt (void) { return (int)m_dwData; }

		//	|    8-bits   |      24-bits       |
		//	------------------------------------
		//	|    class    |       amount       |
		//	|   ordinal   |                    |
		//	|   (1-255)   |   (0-16,777,215)   |

		DWORD m_dwData;

	friend class CNPAssetList;
	};

class CNPUnit
	{
	public:
		CNPUnit (DWORD dwUNID, int iStrength, int iExperience);

		inline DWORD GetClassUNID (void) { return MakeUNID(unidUnitClass, (m_dwData >> 20)); }

	private:
		CNPUnit (void) { }
		static CNPUnit CreateFromInt (int iData);
		inline int GetAsInt (void) { return (int)m_dwData; }

		//	|    12-bits   | 4-bits  | 4-bits  | 4-bits  |1|  7-bits  |
		//	-----------------------------------------------------------
		//	|     class    | inf str | exprnce | resrvd  | |  resrvd  |
		//	|    ordinal   | OR damg |         |         | |          |
		//	|   (1-4095)   | (0-15)  |  (0-15) |         | |          |
		//                                                ^
		//                                                |
		//                                                +- Disabled

		DWORD m_dwData;

	friend class CNPAssetList;
	};

class CNPAssetList
	{
	public:
		enum AssetType
			{
			assetResource,
			assetUnit
			};

		CNPAssetList (void) : m_iResourceCount(0) { }

		inline int GetCount (void) const { return m_Assets.GetCount(); }
		inline AssetType GetAssetType (int iIndex) const { return (iIndex < m_iResourceCount ? assetResource : assetUnit); }
		DWORD GetAssetUNID (int iIndex) const;
		inline bool IsResource (int iIndex) const { return (iIndex < m_iResourceCount); }
		inline bool IsUnit (int iIndex) const { return (iIndex >= m_iResourceCount); }

		inline void AddUnit (CNPUnit NewUnit) { m_Assets.AppendElement(NewUnit.GetAsInt(), NULL); }
		inline CNPUnit GetUnitByIndex (int iIndex) const { return CNPUnit::CreateFromInt(m_Assets.GetElement(m_iResourceCount + iIndex)); }

		int GetResource (DWORD dwUNID) const;
		int ModifyResource (DWORD dwUNID, int iChange);
		inline CNPResource GetResourceByIndex (int iIndex) const { return CNPResource::CreateFromInt(m_Assets.GetElement(iIndex)); }

		ALERROR Load (const CDataPackStruct &Data);
		ALERROR Save (CDataPackStruct *pData) const;

	private:
		int FindResource (DWORD dwUNID, CNPResource *retpRes) const;

		int m_iResourceCount;
		CIntArray m_Assets;					//	Array of resources and units
											//	(resources first)
	};

class CNPStructureList
	{
	private:
		CIntArray m_Structures;
	};

//	Classes

class CNPUnitClass : public CObject
	{
	public:
		CNPUnitClass (void);

		inline void EnumTraits (CIntArray *pList) const { m_Traits.EnumElements(pList); }
		inline NPTechLevels GetBuildTech (void) const { return m_iBuildTech; }
		inline const CNPAssetList &GetBuildRes (void) const { return m_BuildRes; }
		inline int GetBuildTime (void) const { return m_iBuildTime; }
		inline int GetArmor (void) const { return m_iArmor; }
		inline int GetFirepower (void) const { return m_iFirepower; }
		inline int GetFireRate (void) const { return m_iFireRate; }
		inline int GetHull (void) const { return m_iHull; }
		inline int GetHyperdrive (void) const { return m_iHyperdrive; }
		inline CString GetName (void) const { return m_sName; }
		inline int GetScanners (void) const { return m_iScanners; }
		inline int GetStealth (void) const { return m_iStealth; }
		inline DWORD GetUNID (void) const { return m_dwUNID; }
		inline bool HasTrait (int iTrait) const { return m_Traits.HasElement(iTrait); }

		ALERROR Load (const CDataPackStruct &Data);
		ALERROR LoadFromXML (CXMLElement *pElement, CSymbolTable &Symbols);
		ALERROR Save (CDataPackStruct *pData) const;

	private:
		DWORD m_dwUNID;
		CString m_sName;
		CString m_sClassName;

		int m_iHull;						//	Hull class 1-10
		int m_iFirepower;					//	Firepower 1-10
		int m_iFireRate;					//	Fire rate
		int m_iArmor;						//	Armor 1-10
		int m_iStealth;						//	Stealth 1-10
		int m_iScanners;					//	Scanners 1-10

		int m_iHyperdrive;					//	Oros to travel one link

		NPTechLevels m_iBuildTech;			//	Tech required to build
		CNPAssetList m_BuildRes;			//	Resources required to build
		int m_iBuildTime;					//	Oros to build
		CIntSet m_Traits;					//	Traits
	};

//	Fleets

class CNPFleet : public CObject
	{
	public:
		CNPFleet (void);

		static ALERROR Create (DWORD dwUNID,
							   DWORD dwLocation,
							   CNPSovereign *pSovereign,
							   CNPFleet **retpFleet);

		inline CNPAssetList &GetAssetList (void) { return m_Assets; }
		inline DWORD GetDest (void) { return m_dwDest; }
		int GetHyperdrive (CNPUniverse *pUniv);
		inline int GetInTransit (void) { return m_iInTransit; }
		inline DWORD GetLocation (void) { return m_dwLocation; }
		inline CNPFleet *GetNextFleet (void) { return m_pNextFleet; }
		inline CNPSovereign *GetSovereign (void) { return m_pSovereign; }
		inline DWORD GetUNID (void) { return m_dwUNID; }
		inline bool InHyperspace (void) { return (GetInTransit() > 0); }
		inline void SetDest (CNPNullPoint *pDest);
		inline void SetLocation (CNPNullPoint *pPos);
		inline void SetInTransit (int iInTransit) { m_iInTransit = iInTransit; }

		//	Should only be called by CNPNullPoint
		inline void SetNextFleet (CNPFleet *pFleet) { m_pNextFleet = pFleet; }

	private:
		DWORD m_dwUNID;				//	Unique ID for this fleet
		DWORD m_dwLocation;			//	UNID of NullPoint or Link
		CNPSovereign *m_pSovereign;	//	Sovereign commanding this fleet

		DWORD m_dwDest;				//	Destination
		int m_iInTransit;			//	Turns left in transit

		CNPFleet *m_pNextFleet;		//	Next fleet at same null point

		CNPAssetList m_Assets;		//	Ships and resources carried by the fleet
	};

//	Worlds

class CNPWorld : public CObject
	{
	public:
		CNPWorld (void);

		static ALERROR Create (DWORD dwUNID,
							   DWORD dwLocation,
							   CNPSovereign *pSovereign,
							   CNPWorld **retpWorld);

		inline void EnumTraits (CIntArray *pList) { m_Traits.EnumElements(pList); }
		inline int GetDestiny (void) { return m_iDestiny; }
		inline int GetEfficiency (void) { return m_iEfficiency; }
		inline DWORD GetLocation (void) { return m_dwLocation; }
		NPTechLevels GetMinTech (void);
		inline int GetPopulation (void) { return m_iPopulation; }
		inline CNPSovereign *GetSovereign (void) { return m_pSovereign; }
		inline NPTechLevels GetTechLevel (void) { return m_iTechLevel; }
		inline DWORD GetUNID (void) { return m_dwUNID; }
		inline bool HasTrait (int iTrait) { return m_Traits.HasElement(iTrait); }
		inline void RemoveTrait (int iTrait) { m_Traits.RemoveElement(iTrait); }
		inline void SetEfficiency (int iEfficiency) { m_iEfficiency = iEfficiency; }
		inline void SetPopulation (int iPopulation) { m_iPopulation = iPopulation; }
		inline void SetSovereign (CNPSovereign *pSovereign) { m_pSovereign = pSovereign; }
		inline void SetTechLevel (NPTechLevels iTechLevel) { m_iTechLevel = iTechLevel; }
		inline void SetTrait (int iTrait) { m_Traits.AddElement(iTrait); }

	private:
		DWORD m_dwUNID;				//	Unique ID for this world
		DWORD m_dwLocation;			//	UNID of NullPoint
		CNPSovereign *m_pSovereign;	//	Sovereign commanding this world

		int m_iPopulation;			//	Population (in thousands)
		NPTechLevels m_iTechLevel;	//	Technological level
		int m_iEfficiency;			//	Efficiency (0-100)
		CIntSet m_Traits;			//	Traits
		int m_iDestiny;				//	Random variable (0-3599)

		CNPAssetList m_Assets;		//	Resources and units
		CNPStructureList m_Structures;	//	Structures on this world
	};

//	Null points and links

class CNPNullPoint : public CObject
	{
	public:
		CNPNullPoint (void);
		~CNPNullPoint (void);

		static ALERROR Create (DWORD dwUNID, int x, int y, CNPNullPoint **retpNullPoint);

		ALERROR AddWorld (CNPWorld *pWorld);
		inline int GetWorldCount (void) { if (m_pWorld) return 1; else return 0; }
		inline CNPWorld *GetWorld (int iIndex) { return m_pWorld; }

		ALERROR CreateLinkTo (CNPNullPoint *pDest);
		bool IsLinkedTo (CNPNullPoint *pDest);

		inline int GetLinkCount (void) { return m_Links.GetCount(); }
		inline CNPNullPoint *GetLinkDest (int iLink) { return (CNPNullPoint *)m_Links.GetObject(iLink); }
		inline int GetX (void) { return m_x; }
		inline int GetY (void) { return m_y; }
		inline DWORD GetUNID (void) { return m_dwUNID; }

		inline CNPFleet *GetFirstFleet (void) { return m_pFirstFleet; }
		void PlaceFleet (CNPFleet *pFleet);
		void RemoveFleet (CNPFleet *pFleet);

	private:
		DWORD m_dwUNID;				//	Unique ID for this null point
		int m_x, m_y;				//	Coordinates
		CObjectArray m_Links;		//	Links to other null points
		CNPWorld *m_pWorld;			//	World at this null point (may be NULL)

		CNPFleet *m_pFirstFleet;	//	First fleet here
	};

//	Sovereigns

class CNPSovereign : public CObject
	{
	public:
		CNPSovereign (void);

		static ALERROR Create (DWORD dwUNID, CString sName, CNPSovereign **retpSovereign);

		inline DWORD GetCapital (void) { return m_dwCapital; }
		inline const CString &GetName (void) { return m_sName; }
		inline DWORD GetUNID (void) { return m_dwUNID; }
		inline bool KnowsAbout (DWORD dwUNID) { return m_KnownObjects.HasElement(dwUNID); }
		inline void SetCapital (DWORD dwUNID) { m_dwCapital = dwUNID; }
		inline void SetKnowledge (DWORD dwUNID) { m_KnownObjects.AddElement(dwUNID); }

	private:
		DWORD m_dwUNID;				//	Unique ID for this sovereign
		CString m_sName;			//	Name of sovereign entity

		DWORD m_dwCapital;			//	UNID of capital

		CEntryNameTable m_Names;		//	Names for objects in universe
		CIntSet m_KnownObjects;			//	Known objects in the universe
	};

//	The Universe

class CNPUniverse : public CObject
	{
	public:
		CNPUniverse (void);

		static ALERROR CreateUniverse (const CString &sTemplate,
				CNPUniverse **retpUniv,
				CString *retsError);

		ALERROR CreateSovereign (CString sName, CNPWorld *pCapital, CXMLElement *pSovTemplate, CNPSovereign **retpSovereign);
		ALERROR CreateFleet (CNPNullPoint *pNP, CNPSovereign *pSovereign, CNPFleet **retpFleet);

		inline bool CycleTime (int iDestiny, int iCycle) { return (((DWORD)m_iTurn + (DWORD)iDestiny) % (DWORD)iCycle) == 0; }

		inline CNPFleet *GetFleet (DWORD dwUNID) { return m_Fleets.GetEntry(dwUNID); }
		inline DWORD GetIndependentUNID (void) { return m_dwIndependent; }
		inline CNPNullPoint *GetNullPoint (DWORD dwUNID) { return m_NullPoints.GetEntry(dwUNID); }
		inline CNPNullPoint *GetNullPointByIndex (int iIndex) { return m_NullPoints.GetEntryByIndex(iIndex); }
		inline int GetNullPointCount (void) { return m_NullPoints.GetCount(); }
		inline DWORD GetOverlordUNID (void) { return m_dwOverlord; }
		inline CNPSovereign *GetSovereign (DWORD dwUNID) { return m_Sovereigns.GetEntry(dwUNID); }
		inline int GetSovereignCount (void) { return m_Sovereigns.GetCount(); }
		inline CXMLElement *GetPlayerTemplate (void) { return m_pPlayerTemplate; }
		CString GetStardate (void);
		inline int GetTurn (void) { return m_iTurn; }
		inline CNPUnitClass *GetUnitClass (DWORD dwUNID) { return m_UnitClasses.GetEntry(dwUNID); }
		inline CNPWorld *GetWorld (DWORD dwUNID) { return m_Worlds.GetEntry(dwUNID); }

		ALERROR EnterHyperspace (CNPFleet *pFleet, CNPNullPoint *pDest);
		void ScanNullPoint (CNPSovereign *pSovereign, CNPNullPoint *pNP, int iScannerLevel);

		bool Update (void);

	private:
		enum TraitType
			{
			ttMajorAdvantage = 1,
			ttMinorAdvantage = 2,
			ttMinorDisadvantage = 3,
			ttMajorDisadvantage = 4,
			};

		struct CreateCtx
			{
			CXMLElement *pTemplate;
			CSymbolTable *pSymbols;
			CString sError;
			};

		struct TechTableEntry
			{
			CString sName;
			int iProductionFactor;			//	Production adjustment
			int iStdLivingFactor;			//	Standard of living expected
			int iPollutionFactor;			//	Pollution factor
			};

		//	Internal creation functions
		ALERROR AddRandomTrait (CreateCtx &Ctx,
								CNPWorld *pWorld, 
								TraitType iSection, 
								int iCount, 
								CXMLElement *pTraitTable);
		ALERROR CreateNullPointNetwork (int iNullPoints);
		ALERROR CreateRandomWorld (CreateCtx &Ctx, CNPNullPoint *pNP, CNPSovereign *pSovereign);
		ALERROR CreateRandomWorlds (CreateCtx &Ctx);
		ALERROR InitFromXML (CXMLElement *pTemplate, CSymbolTable &Symbols, CString *retsError);
		ALERROR LoadClassesFromXML (CXMLElement *pTemplate, CSymbolTable &Symbols, CString *retsError);

		//	Internal update functions
		void UpdateWorld (CNPWorld *pWorld);

		//	Universe classes
		CXMLElement *m_pPlayerTemplate;							//	Template for new player sovereigns
		CEntryTable<unidUnitClass, CNPUnitClass> m_UnitClasses;	//	List of unit classes
		TechTableEntry m_TechTable[techCount];					//	Technolog table

		//	Universe objects
		CEntryTable<unidNullPoint, CNPNullPoint> m_NullPoints;	//	List of null points (indexed by UNID)
		CEntryTable<unidWorld, CNPWorld> m_Worlds;				//	List of worlds (indexed by UNID)
		CEntryTable<unidSovereign, CNPSovereign> m_Sovereigns;	//	List of sovereigns in the universe (indexed by UNID)
		CEntryTable<unidFleet, CNPFleet> m_Fleets;				//	List of fleets (indexed by UNID)

		//	State
		DWORD m_dwNextTurn;										//	Time at which to update (in milliseconds since system start)
		int m_iTurn;											//	Oros since universe creation
		int m_iBaseAnno;										//	Base anno for the scenario
		int m_iMaxPlayers;
		DWORD m_dwOverlord;										//	UNID of Overlord sovereign
		DWORD m_dwIndependent;									//	UNID of independent sovereign
	};

//	Inlines

inline void CNPFleet::SetDest (CNPNullPoint *pDest) { m_dwDest = pDest->GetUNID(); }
inline void CNPFleet::SetLocation (CNPNullPoint *pPos) { m_dwLocation = pPos->GetUNID(); }

//	INTERNAL DEFINES

int GetSymbolicAttribute (CSymbolTable &Symbols, CXMLElement *pElement, const CString &sAttribute);

#endif