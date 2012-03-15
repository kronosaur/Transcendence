//	TSE.h
//
//	Transcendence Space Engine

#ifndef INCL_TSE
#define INCL_TSE

#ifndef INCL_EUCLID
#include "Euclid.h"
#endif

#ifndef INCL_LINGUA
#include "Lingua.h"
#endif

#ifndef INCL_INTERNETS
#include "Internets.h"
#endif

#ifndef INCL_CRYPTO
#include "Crypto.h"
#endif

#ifndef INCL_JPEGUTIL
#include "JPEGUtil.h"
#endif

#ifndef INCL_XMLUTIL
#include "XMLUtil.h"
#endif

#ifndef INCL_JSONUTIL
#include "JSONUtil.h"
#endif

#ifndef INCL_DIRECTXUTIL
#include "DirectXUtil.h"
#endif

#ifndef INCL_REANIMATOR
#include "Reanimator.h"
#endif

#ifndef INCL_TSE_GEOMETRY
#include "Geometry.h"
#endif

#define LEVEL_ROMAN_NUMERALS

//	Define some debugging symbols

#ifdef DEBUG
//#define DEBUG_SOURCE_LOAD_TRACE
//#define DEBUG_SHIP
//#define DEBUG_HENCHMAN
//#define DEBUG_LOAD
//#define DEBUG_COMBAT
//#define DEBUG_ALL_ITEMS
//#define DEBUG_ALL_NODES
//#define DEBUG_ATTACK_TARGET
#define DEBUG_VECTOR
//#define DEBUG_PERFORMANCE
//#define DEBUG_NEBULA_PAINTING
//#define DEBUG_ASTAR_PERF
//#define DEBUG_ASTAR_PATH
//#define DEBUG_NAV_PATH
//#define DEBUG_AVOID_POTENTIAL
//#define DEBUG_DOCK_PORT_POS
//#define DEBUG_STATION_EXCLUSION_ZONE
//#define DEBUG_BOUNDING_RECT
//#define DEBUG_FIRE_ON_OPPORTUNITY
#endif

//	We leave this defined because we want to get traces in the field in case
//	of a crash.
#define DEBUG_PROGRAMSTATE

//	If ITEM_REFERENCE is defined, then the player doesn't see the
//	stats for an item until they install it (or get reference info)
//#define ITEM_REFERENCE

//	Forward references

class CArmorClass;
class CBoundaryMarker;
class CDeviceClass;
class CDockScreenType;
class CEffectCreator;
class CEnergyField;
class CEnergyFieldType;
class CInstalledDevice;
class CItem;
class CItemListManipulator;
class CItemTable;
class CItemType;
class CMarker;
class CMissile;
class CParticleEffect;
class CPower;
class CRandomEntryResults;
class CResourceDb;
class CShip;
class CShipClass;
class CShipTable;
class CSovereign;
class CSpaceEnvironmentType;
class CSpaceObject;
class CStation;
class CStationType;
class CSystem;
class CTimedEvent;
class CTopologyNode;
class CTradingDesc;
class CUniverse;
class CVector;
class CWeaponClass;
class CWeaponFireDesc;
class IEffectPainter;
class IShipController;
class IShipGenerator;

extern CUniverse *g_pUniverse;

//	Include other TSE files

#ifndef INCL_TSE_TYPES
#include "TSETypes.h"
#endif

#ifndef INCL_TSE_UTIL
#include "TSEUtil.h"
#endif

#ifndef INCL_TSE_DESIGN
#include "TSEDesign.h"
#endif

//	CResourceDb

class CResourceDb
	{
	public:
		CResourceDb (const CString &sDefFilespec, CResourceDb *pMainDb = NULL, bool bExtension = false);
		~CResourceDb (void);

		const CString &GetFilespec (void) const { return m_sFilespec; }
		CString GetRootTag (void);
		bool ImageExists (const CString &sFolder, const CString &sFilename);
		bool IsUsingExternalGameFile (void) const { return !m_bGameFileInDb; }
		bool IsUsingExternalResources (void) const { return !m_bResourcesInDb; }
		ALERROR LoadEntities (CString *retsError, CExternalEntityTable **retEntities = NULL);
		ALERROR LoadGameFile (CXMLElement **retpData, CExternalEntityTable *pEntities, CString *retsError, CExternalEntityTable *ioEntityTable = NULL);
		ALERROR LoadImage (const CString &sFolder, const CString &sFilename, HBITMAP *rethImage, EBitmapTypes *retiImageType = NULL);
		ALERROR LoadModule (const CString &sFolder, const CString &sFilename, CXMLElement **retpData, CString *retsError);
		ALERROR LoadSound (CSoundMgr &SoundMgr, const CString &sFolder, const CString &sFilename, int *retiChannel);
		ALERROR Open (DWORD dwFlags = 0);
		void SetEntities (CExternalEntityTable *pEntities, bool bFree = false);

		CString GetResourceFilespec (int iIndex);
		int GetResourceCount (void);
		inline int GetVersion (void) { return m_iVersion; }
		ALERROR ExtractMain (CString *retsData);
		ALERROR ExtractResource (const CString sFilespec, CString *retsData);

	private:
		ALERROR OpenDb (void);

		int m_iVersion;
		bool m_bGameFileInDb;
		bool m_bResourcesInDb;

		//	If we're just using the file system
		CString m_sFilespec;
		CString m_sRoot;
		CString m_sGameFile;

		//	If we're using a TDB
		CDataFile *m_pDb;
		CSymbolTable *m_pResourceMap;
		int m_iGameFile;

		CExternalEntityTable *m_pEntities;			//	Entities to use in parsing
		bool m_bFreeEntities;						//	If TRUE, we own m_pEntities;
		CResourceDb *m_pMainDb;						//	Main file db (only if loading extension)
	};

class CAStarPathFinder
	{
	public:
		CAStarPathFinder (void);
		~CAStarPathFinder (void);

		void AddObstacle (const CVector &vUR, const CVector &vLL);
		int FindPath (const CVector &vStart, const CVector &vEnd, CVector **retPathList);

	private:
		struct SObstacle
			{
			CVector vUR;
			CVector vLL;
			};

		struct SNode
			{
			int x;
			int y;

			CVector vPos;
			int iCostFromStart;
			int iHeuristic;
			int iTotalCost;

			SNode *pParent;
			SNode *pPrev;
			SNode *pNext;
			};

		typedef SNode *SNodeRoot;

		void AddToClosedList (SNode *pNew);
		void AddToOpenList (SNode *pNew);
		int CalcHeuristic (const CVector &vPos, const CVector &vDest);
		void CollapsePath (TArray<SNode *> &Path, int iStart, int iEnd);
		void CreateInOpenList (const CVector &vEnd, SNode *pCurrent, int xDir, int yDir);
		bool IsInClosedList (int x, int y);
		bool IsPathClear (const CVector &vStart, const CVector &vEnd);
		bool IsPointClear (const CVector &vPos);
		bool LineIntersectsRect (const CVector &vStart, const CVector &vEnd, const CVector &vUR, const CVector &vLL);
		int OptimizePath (const CVector &vEnd, SNode *pFinal, CVector **retPathList);
		void Reset (void);

		void AddToList (SNodeRoot &pRoot, SNode *pNew, SNode *pAddAfter = NULL);
		void DeleteFromList (SNodeRoot &pRoot, SNode *pDelete);
		void DeleteList (SNodeRoot &pRoot);

		TArray<SObstacle> m_Obstacles;
		SNodeRoot m_pOpenList;
		SNodeRoot m_pClosedList;
		CTileMap *m_pClosedMap;

#ifdef DEBUG_ASTAR_PERF
		int m_iCallsToIsPathClear;
		int m_iClosedListCount;
		int m_iOpenListCount;
#endif
	};

//	Paint Utilities

void CreateBlasterShape (int iAngle, int iLength, int iWidth, SPoint *Poly);
void DrawItemTypeIcon (CG16bitImage &Dest, int x, int y, CItemType *pType);
void DrawLightning (CG16bitImage &Dest,
					int xFrom, int yFrom,
					int xTo, int yTo,
					WORD wColor,
					int iPoints,
					Metric rChaos);
void DrawParticle (CG16bitImage &Dest,
				   int x, int y,
				   WORD wColor,
				   int iSize,
				   DWORD byOpacity);

//	Helper functions

CString ComposeDamageAdjReference (int *AdjRow, int *StdRow);
CString ComposeNounPhrase (const CString &sNoun, 
						   int iCount, 
						   const CString &sModifier, 
						   DWORD dwNounFlags, 
						   DWORD dwComposeFlags);
void ComputePercentages (int iCount, int *pTable);
CString ParseNounForm (const CString &sNoun, DWORD dwNounFlags, bool bPluralize, bool bShortName);

#define VISUAL_RANGE_INDEX		6
#define RANGE_INDEX_COUNT		9
Metric RangeIndex2Range (int iIndex);

CString ReactorPower2String (int iPower);

int NLCompare (TArray<CString> &Input, TArray<CString> &Pattern);
void NLSplit (const CString &sPhrase, TArray<CString> *retArray);

void ReportCrashObj (CString *retsMessage, CSpaceObject *pCrashObj = NULL);

struct SExplosionType
	{
	CWeaponFireDesc *pDesc;				//	Explosion type
	int iBonus;							//	Bonus damage
	DestructionTypes iCause;			//	Cause
	};

class CWeaponFireDesc
	{
	public:
		enum ECachedHandlers
			{
			evtOnDamageArmor			= 0,
			evtOnDamageOverlay			= 1,
			evtOnDamageShields			= 2,
			evtOnFragment				= 3,

			evtCount					= 4,
			};

		struct SFragmentDesc
			{
			CWeaponFireDesc *pDesc;			//	Data for fragments
			DiceRange Count;				//	Number of fragments
			bool bMIRV;						//	Fragments seek independent targets

			SFragmentDesc *pNext;
			};

		CWeaponFireDesc (void);
		CWeaponFireDesc (const CWeaponFireDesc &Desc);
		~CWeaponFireDesc (void);

		inline bool CanAutoTarget (void) const { return m_bAutoTarget; }
		inline bool CanHitFriends (void) const { return !m_bNoFriendlyFire; }
		void CreateHitEffect (CSystem *pSystem, SDamageCtx &DamageCtx);
		CEffectCreator *FindEffectCreator (const CString &sUNID);
		bool FindEventHandler (const CString &sEvent, SEventHandlerDesc *retEvent = NULL) const;
		inline bool FindEventHandler (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }
		CWeaponFireDesc *FindWeaponFireDesc (const CString &sUNID, char **retpPos = NULL);
		static CWeaponFireDesc *FindWeaponFireDescFromFullUNID (const CString &sUNID);
		bool FireOnDamageArmor (SDamageCtx &Ctx);
		bool FireOnDamageOverlay (SDamageCtx &Ctx, CEnergyField *pOverlay);
		bool FireOnDamageShields (SDamageCtx &Ctx, int iDevice);
		bool FireOnFragment (const CDamageSource &Source, CSpaceObject *pShot, const CVector &vHitPos, CSpaceObject *pNearestObj, CSpaceObject *pTarget);
		inline CItemType *GetAmmoType (void) const { return m_pAmmoType; }
		inline int GetAreaDamageDensity (void) const { return m_AreaDamageDensity.Roll(); }
		inline int GetAveParticleCount (void) const { return m_ParticleCount.GetAveValue(); }
		inline int GetBeamType (void) const { return m_iBeamType; }
		CItemType *GetWeaponType (CItemType **retpLauncher = NULL) const;
		inline CEffectCreator *GetEffect (void) const { return m_pEffect; }
		inline ICCItem *GetEventHandler (const CString &sEvent) const { SEventHandlerDesc Event; if (!FindEventHandler(sEvent, &Event)) return NULL; return Event.pCode; }
		inline Metric GetExpansionSpeed (void) const { return (m_ExpansionSpeed.Roll() * LIGHT_SPEED / 100.0); }
		inline SExtensionDesc *GetExtension (void) const { return m_pExtension; }
		inline CEffectCreator *GetFireEffect (void) const { return m_pFireEffect; }
		inline CEffectCreator *GetHitEffect (void) const { return m_pHitEffect; }
		inline int GetInitialDelay (void) const { return m_InitialDelay.Roll(); }
		Metric GetInitialSpeed (void) const;
		inline int GetIntensity (void) const { return m_iIntensity; }
		inline int GetInteraction (void) const { return m_iInteraction; }
		inline FireTypes GetFireType (void) const { return m_iFireType; }
		inline SFragmentDesc *GetFirstFragment (void) const { return m_pFirstFragment; }
		inline int GetHitPoints (void) const { return m_iHitPoints; }
		inline int GetLifetime (void) const { return m_Lifetime.Roll(); }
		inline int GetManeuverRate (void) const { return m_iManeuverRate; }
		inline int GetMaxLifetime (void) const { return m_Lifetime.GetMaxValue(); }
		inline int GetMaxParticleCount (void) const { return m_ParticleCount.GetMaxValue(); }
		inline Metric GetMaxRadius (void) const { return m_rMaxRadius; }
		inline Metric GetMinRadius (void) const { return m_rMinRadius; }
		inline int GetParticleCount (void) const { return m_ParticleCount.Roll(); }
		inline int GetParticleEmitTime (void) const { return m_ParticleEmitTime.Roll(); }
		inline int GetParticleMissChance (void) const { return m_iMissChance; }
		inline int GetParticleSplashChance (void) const { return m_iSplashChance; }
		inline int GetParticleSpreadAngle (void) const { return m_iParticleSpread; }
		inline int GetParticleSpreadWidth (void) const { return m_iParticleSpreadWidth; }
		inline int GetPassthrough (void) const { return m_iPassthrough; }
		inline WORD GetPrimaryColor (void) const { return m_wPrimaryColor; }
		inline int GetProximityFailsafe (void) const { return m_iProximityFailsafe; }
		inline Metric GetRatedSpeed (void) const { return m_rMissileSpeed; }
		inline WORD GetSecondaryColor (void) const { return m_wSecondaryColor; }
		inline WORD GetVaporTrailColor (void) const { return m_wVaporTrailColor; }
		inline int GetVaporTrailLength (void) const { return m_iVaporTrailLength; }
		inline int GetVaporTrailWidth (void) const { return m_iVaporTrailWidth; }
		inline int GetVaporTrailWidthInc (void) const { return m_iVaporTrailWidthInc; }
		inline bool HasEvents (void) const { return !m_Events.IsEmpty(); }
		inline bool HasFragments (void) const { return m_pFirstFragment != NULL; }
		inline bool HasOnFragmentEvent (void) const { return m_CachedEvents[evtOnFragment].pCode != NULL; }

		void InitFromDamage (DamageDesc &Damage);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, bool bDamageOnly = false);
		inline bool IsTracking (void) { return m_iManeuverability != 0; }
		inline bool IsTrackingTime (int iTick) { return (m_iManeuverability > 0 && (iTick % m_iManeuverability) == 0); }
		void LoadImages (void);
		void MarkImages (void);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		ALERROR OverrideDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		inline bool ProximityBlast (void) const { return m_bProximityBlast; }

	public:
		CString m_sUNID;					//	Identification. The format is
											//		uuuuu/n/fi/e
											//
											//		uuuuu = weapon UNID
											//		n = shot data ordinal
											//		fi = fragment, i is index (optional)
											//		e = enhanced (optional)

		CItemTypeRef m_pAmmoType;			//	item type for this ammo

		FireTypes m_iFireType;				//	beam or missile
		DamageDesc m_Damage;				//	Damage per shot
		Metric m_rMaxEffectiveRange;		//	Max effective range of weapon
		int m_iContinuous;					//	repeat for this number of frames
		int m_iFireSound;					//	Sound when weapon is fired (-1 == no sound)

		//	Missile stuff (m_iFireType == ftMissile)
		CObjectImageArray m_Image;			//	Image for missile
		int m_iAccelerationFactor;			//	% increase in speed per 10 ticks
		Metric m_rMaxMissileSpeed;			//	Max speed.
		bool m_bDirectional;				//	True if different images for each direction
		bool m_bFragment;					//	True if this is a fragment of a proximity blast
		bool m_bCanDamageSource;			//	TRUE if we can damage the source
		int m_iStealth;						//	Missile stealth

		//	Missile exhaust
		int m_iExhaustRate;					//	Ticks per exhaust creation (0 if no exhaust)
		CObjectImageArray m_ExhaustImage;	//	Image for exhaust
		int m_iExhaustLifetime;				//	Ticks that each exhaust particle lasts
		Metric m_rExhaustDrag;				//	Coefficient of drag for exhaust particles

		//	Miscellaneous
		CWeaponFireDesc *m_pEnhanced;		//	Data when weapon is enhanced

		//	Flags
		DWORD m_fVariableInitialSpeed:1;	//	TRUE if initial speed is random
		DWORD m_dwSpare:31;

	private:
		SExtensionDesc *m_pExtension;		//	Extension that defines the weaponfiredesc

		//	Basic properties
		Metric m_rMissileSpeed;				//	Speed of missile
		DiceRange m_MissileSpeed;			//	Speed of missile (if random)
		DiceRange m_Lifetime;				//	Lifetime of fire in seconds
		DiceRange m_InitialDelay;			//	Delay for n ticks before starting

		bool m_bAutoTarget;					//	TRUE if we can acquire new targets after launch
		bool m_bNoFriendlyFire;				//	TRUE if we cannot hit our friends
		int m_iPassthrough;					//	Chance that the missile will continue through target

		//	Effects
		CEffectCreatorRef m_pEffect;		//	Effect for the actual bullet/missile/beam
		CEffectCreatorRef m_pHitEffect;		//	Effect when we hit/explode
		CEffectCreatorRef m_pFireEffect;	//	Effect when we fire (muzzle flash)
		int m_iVaporTrailWidth;				//	Width of vapor trail (0 = none)
		int m_iVaporTrailLength;			//	Number of segments
		int m_iVaporTrailWidthInc;			//	Width increment in 100ths of a pixel
		WORD m_wVaporTrailColor;			//	Color of vapor trail
		WORD m_wSpare;

		//	Missile stuff (m_iFireType == ftMissile)
		int m_iHitPoints;					//	HP before disipating (0 = destroyed by any hit)
		int m_iInteraction;					//	Interaction opacity (0-100)
		int m_iManeuverability;				//	Tracking maneuverability (0 = none)
		int m_iManeuverRate;				//	Angle turned at each maneuverability point

		//	Particles (m_iFireType == ftParticles)
		DiceRange m_ParticleCount;			//	Number of new particles per tick
		DiceRange m_ParticleEmitTime;		//	Emit new particles for this number of ticks (default = 1)
		int m_iParticleSpreadWidth;			//	Width of spread
		int m_iParticleSpread;				//	Angle of spread
		int m_iSplashChance;				//	Chance that particles will splash
		int m_iMissChance;					//	Chance that particles will miss

		//	Area stuff (m_iFireType == ftArea)
		DiceRange m_ExpansionSpeed;			//	Speed of expansion (% of lightspeed)
		DiceRange m_AreaDamageDensity;		//	Number of points along the edge

		//	Radius stuff (m_iFireType == ftRadius)
		Metric m_rMinRadius;				//	All objects inside this radius take full damage
		Metric m_rMaxRadius;				//	Damage inside this radius is decreased by inverse square law

		//	Beam stuff (m_iFireType == ftBeam)
		BeamTypes m_iBeamType;				//	If a beam...
		WORD m_wPrimaryColor;				//	Primary beam color
		WORD m_wSecondaryColor;				//	Secondary (usually halo) color
		int m_iIntensity;					//	Variation depends on beam type

		//	Fragmentation
		SFragmentDesc *m_pFirstFragment;	//	Pointer to first fragment desc (or NULL)
		bool m_bProximityBlast;				//	This is TRUE if we have fragments or if we have
											//		and OnFragment event.
		int m_iProximityFailsafe;			//	Min ticks before proximity is active

		//	Events
		CEventHandler m_Events;				//	Events
		SEventHandlerDesc m_CachedEvents[evtCount];
	};

//	Trading

class CTradingDesc
	{
	public:
		CTradingDesc (void);
		~CTradingDesc (void);

		inline void AddBuyOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj)
			{ AddOrder(pType, sCriteria, iPriceAdj, FLAG_BUYS); }
		inline void AddSellOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj)
			{ AddOrder(pType, sCriteria, iPriceAdj, FLAG_SELLS); }
		bool Buys (CSpaceObject *pObj, const CItem &Item, int *retiPrice = NULL, int *retiMaxCount = NULL);
		int Charge (CSpaceObject *pObj, int iCharge);
		inline CEconomyType *GetEconomyType (void) { return m_pCurrency; }
		inline int GetMaxCurrency (void) { return m_iMaxCurrency; }
		inline int GetReplenishCurrency (void) { return m_iReplenishCurrency; }
		bool Sells (CSpaceObject *pObj, const CItem &Item, int *retiPrice = NULL);
		void SetEconomyType (CEconomyType *pCurrency) { m_pCurrency.Set(pCurrency); }
		void SetMaxCurrency (int iMaxCurrency) { m_iMaxCurrency = iMaxCurrency; }
		void SetReplenishCurrency (int iReplenishCurrency) { m_iReplenishCurrency = iReplenishCurrency; }

		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CTradingDesc **retpTrade);
		void OnCreate (CSpaceObject *pObj);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		void OnUpdate (CSpaceObject *pObj);
		void ReadFromStream (SLoadCtx &Ctx);
		void RefreshInventory (CSpaceObject *pObj, int iPercent = 100);
		void WriteToStream (IWriteStream *pStream);

	private:
		enum Flags
			{
			FLAG_SELLS =			0x00000001,	//	TRUE if station sells this item type
			FLAG_BUYS =				0x00000002,	//	TRUE if station buys this item type
			FLAG_ACTUAL_PRICE =		0x00000004,	//	TRUE if we compute actual price
			FLAG_INVENTORY_ADJ =	0x00000008,	//	TRUE if we adjust the inventory
			};

		struct SCommodityDesc
			{
			CString sID;						//	ID of order

			CItemTypeRef pItemType;				//	Item type
			CItemCriteria ItemCriteria;			//	If ItemType is NULL, this is the criteria

			CFormulaText PriceAdj;				//	Price adjustment
			CFormulaText InventoryAdj;			//	% of item count found at any one time

			DWORD dwFlags;						//	Flags
			};

		void AddOrder (CItemType *pItemType, const CString &sCriteria, int iPriceAdj, DWORD dwFlags);
		CString ComputeID (DWORD dwUNID, const CString &sCriteria, DWORD dwFlags);
		int ComputeMaxCurrency (CSpaceObject *pObj);
		int ComputePrice (CSpaceObject *pObj, const CItem &Item, const SCommodityDesc &Commodity);
		bool Matches (const CItem &Item, const SCommodityDesc &Commodity);
		bool SetInventoryCount (CSpaceObject *pObj, SCommodityDesc &Desc, CItemType *pItemType);

		CEconomyTypeRef m_pCurrency;
		int m_iMaxCurrency;
		int m_iReplenishCurrency;

		TArray<SCommodityDesc> m_List;
	};

//	Space environment

//	CNavigationPath

class CNavigationPath : public TSEListNode<CNavigationPath>
	{
	public:
		CNavigationPath (void);
		~CNavigationPath (void);

		static void Create (CSystem *pSystem, CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd, CNavigationPath **retpPath);

		Metric ComputePathLength (CSystem *pSystem) const;
		CVector ComputePointOnPath (CSystem *pSystem, Metric rDist) const;
		static CString DebugDescribe (CSpaceObject *pObj, CNavigationPath *pNavPath);
		void DebugPaintInfo (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		inline DWORD GetID (void) const { return m_dwID; }
		inline int GetNavPointCount (void) const { return m_iWaypointCount; }
		CVector GetNavPoint (int iIndex) const;
		bool Matches (CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd);
		void OnReadFromStream (SLoadCtx &Ctx);
		void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream) const;

	private:
		static int ComputePath (CSystem *pSystem, CSovereign *pSovereign, const CVector &vFrom, const CVector &vTo, CVector **retpPoints);
//		static int ComputePath (CSystem *pSystem, CSovereign *pSovereign, const CVector &vFrom, const CVector &vTo, int iDepth, CVector **retpPoints);
		static bool PathIsClear (CSystem *pSystem, CSovereign *pSovereign, const CVector &vFrom, const CVector &vTo, CSpaceObject **retpEnemy, CVector *retvAway);

		DWORD m_dwID;							//	ID of path
		CSovereign *m_pSovereign;				//	Sovereign that will use (NULL if all)
		DWORD m_iStartIndex;					//	Index of start object
		DWORD m_iEndIndex;						//	Index of end object

		int m_iSuccesses;						//	Count of ships that successfully reached end
		int m_iFailures;						//	Count of ships that were destroyed

		CVector m_vStart;						//	Start position
		int m_iWaypointCount;					//	Number of waypoints (excludes start)
		CVector *m_Waypoints;					//	Array of waypoints
	};

typedef TSEListNode<CNavigationPath> CNavigationPathNode;

//	CSystemEventHandler

class CSystemEventHandler : public TSEListNode<CSystemEventHandler>
	{
	public:
		CSystemEventHandler (void);

		static void Create (CSpaceObject *pObj, Metric rMaxRange, CSystemEventHandler **retpHandler);

		inline CSpaceObject *GetObj (void) const { return m_pHandler; }
		bool InRange (const CVector &vPos) const;
		void OnObjDestroyed (CSpaceObject *pObjDestroyed, bool *retbRemoveNode);
		void OnReadFromStream (SLoadCtx &Ctx);
		void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream) const;
		inline void SetRange (Metric rMaxRange) { m_rMaxRange2 = rMaxRange * rMaxRange; }

	private:
		CSpaceObject *m_pHandler;				//	Event handler
		Metric m_rMaxRange2;					//	Beyond this range from handler, no events seen
	};

typedef TSEListNode<CSystemEventHandler> CSystemEventHandlerNode;

//	System

enum SpaceEnvironmentAreaTypes
	{
	seaNone	=					0,
	seaNebula =					1,
	seaDarkNebula =				2,
	seaRadiationField =			3,
	seaIonStorm =				4,
	seaPlasmaStorm =			5,

	seaTileSize =				512,	//	tile size in pixels
	seaArraySize =				16,		//	each level is 16x16 tiles
	seaScale =					1,		//	2 levels (levels = scale + 1)
	};

class COrbit
	{
	public:
		COrbit (void);
		COrbit (const CVector &vCenter, Metric rRadius, Metric rPos = 0.0);
		COrbit (const CVector &vCenter, Metric rSemiMajorAxis, Metric rEccentricity, Metric rRotation, Metric rPos = 0.0);

		inline const Metric &GetEccentricity (void) const { return m_rEccentricity; }
		inline const CVector &GetFocus (void) const { return m_vFocus; }
		inline const Metric &GetObjectAngle (void) const { return m_rPos; }
		inline CVector GetObjectPos (void) const { return GetPoint(m_rPos); }
		CVector GetPoint (Metric rAngle) const;
		CVector GetPointAndRadius (Metric rAngle, Metric *retrRadius) const;
		CVector GetPointCircular (Metric rAngle) const;
		inline const Metric &GetRotation (void) const { return m_rRotation; }
		inline const Metric &GetSemiMajorAxis (void) const { return m_rSemiMajorAxis; }
		void Paint (CG16bitImage &Dest, const ViewportTransform &Trans, COLORREF rgbColor);

	private:
		CVector m_vFocus;				//	Focus of orbit
		Metric m_rEccentricity;			//	Ellipse eccentricity
		Metric m_rSemiMajorAxis;		//	Semi-major axis
		Metric m_rRotation;				//	Angle of rotation (radians)

		Metric m_rPos;					//	Obj position in orbit (radians)
	};

class CSystemCreateStats
	{
	public:
		CSystemCreateStats (void);
		~CSystemCreateStats (void);

		void AddLabel (const CString &sAttributes);
		inline int GetLabelAttributesCount (void) { return m_LabelAttributeCounts.GetCount(); }
		void GetLabelAttributes (int iIndex, CString *retsAttribs, int *retiCount);
		inline int GetTotalLabelCount (void) { return m_iLabelCount; }

	private:
		struct SLabelAttributeEntry
			{
			CString sAttributes;
			int iCount;
			};

		void AddLabelExpansion (const CString &sAttributes, const CString &sPrefix = NULL_STR);

		int m_iLabelCount;
		CSymbolTable m_LabelAttributeCounts;
	};

class CSystemCreateEvents
	{
	public:
		void AddDeferredEvent (CSpaceObject *pObj, SExtensionDesc *pExtension, CXMLElement *pEventCode);
		ALERROR FireDeferredEvent (const CString &sEvent, CString *retsError);

	private:
		struct SEventDesc
			{
			CSpaceObject *pObj;
			SExtensionDesc *pExtension;
			CXMLElement *pEventCode;
			};

		TArray<SEventDesc> m_Events;
	};

struct SLabelDesc
	{
	SLabelDesc (void) : 
			bDelete(false) { }

	COrbit OrbitDesc;
	CString sAttributes;
	bool bDelete;
	};

struct SSystemCreateCtx
	{
	SSystemCreateCtx (void) : 
			pExtension(NULL),
			pStats(NULL),
			pStation(NULL), 
			dwLastObjID(0) 
		{ }

	CTopologyNode *pTopologyNode;			//	Topology node
	CSystem *pSystem;						//	System that we're creating
	CXMLElement *pLocalTables;				//	Lookup tables

	SExtensionDesc *pExtension;				//	Extension from which the current desc came

	CSystemCreateStats *pStats;				//	System creation stats (may be NULL)
	CSystemCreateEvents Events;				//	System deferred events

	CString sError;							//	Creation error
	CString sLocationAttribs;				//	Current location attribs
	CSpaceObject *pStation;					//	Root station when creating satellites
	DWORD dwLastObjID;						//	Object created in last call
											//	NOTE: This is an ID in case the object gets deleted.
	};

//	CMoveCtx is currently unused; it was part of an experiment to see
//	if I could improve the moving algorithms, but it proved too time-consuming

class CMoveCtx
	{
	public:
		CMoveCtx (int iAlloc);
		~CMoveCtx (void);

		void AddObj (CSpaceObject *pObj);
		void BounceObj (int iIndex);
		inline int GetCount (void) { return m_iCount; }
		inline CSpaceObject *GetObj (int iIndex) { return m_pArray[iIndex].pObj; }

	private:
		struct SEntry
			{
			CSpaceObject *pObj;

			CVector vOldPos;				//	Starting position of obj
			CVector vNewPos;				//	Ending position of obj

			CVector vBoundsUR;				//	Bounds for object across motion
			CVector vBoundsLL;

			bool bMobile;					//	TRUE if obj is capable of moving
			bool bHasMoved;					//	TRUE if obj moved this tick
			bool bBarrier;					//	TRUE if obj is a barrier
			};

		SEntry *m_pArray;
		int m_iCount;
		int m_iAlloc;
	};

class CLocationDef
	{
	public:
		CLocationDef (void);

		inline const CString &GetAttributes (void) { return m_sAttributes; }
		inline DWORD GetObjID (void) { return m_dwObjID; }
		inline const COrbit &GetOrbit (void) { return m_OrbitDesc; }
		inline bool HasAttribute (const CString &sAttrib) { return ::HasModifier(m_sAttributes, sAttrib); }
		inline bool IsBlocked (void) { return m_bBlocked; }
		inline bool IsEmpty (void) { return (m_dwObjID == 0 && !m_bBlocked); }
		inline void SetAttributes (const CString &sAttribs) { m_sAttributes = sAttribs; }
		inline void SetBlocked (bool bBlocked = true) { m_bBlocked = bBlocked; }
		inline void SetID (const CString &sID) { m_sID = sID; }
		inline void SetObjID (DWORD dwObjID) { m_dwObjID = dwObjID; }
		inline void SetOrbit (const COrbit &Orbit) { m_OrbitDesc = Orbit; }
		void ReadFromStream (SLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

	private:
		CString m_sID;						//	May be blank
		COrbit m_OrbitDesc;
		CString m_sAttributes;

		DWORD m_dwObjID;					//	Object created at this location (or 0)

		bool m_bBlocked;					//	If TRUE, this location is too close to another
	};

class CLocationList
	{
	public:
		CLocationList (void);

		void FillCloseLocations (void);
		inline int GetCount (void) { return m_List.GetCount(); }
		bool GetEmptyLocations (TArray<int> *retList);
		inline CLocationDef *GetLocation (int iIndex) { return &m_List[iIndex]; }
		CLocationDef *Insert (const CString &sID = NULL_STR);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetBlocked (int iIndex) { m_List[iIndex].SetBlocked(); }
		inline void SetObjID (int iIndex, DWORD dwObjID) { m_List[iIndex].SetObjID(dwObjID); }
		void WriteToStream (IWriteStream *pStream);

	private:
		TArray<CLocationDef> m_List;

		bool m_bMinDistCheck;				//	If TRUE, then we've checked all locations for min distance
	};

class CTerritoryDef
	{
	public:
		CTerritoryDef (void);

		void AddAttributes (const CString &sAttribs);
		void AddRegion (const COrbit &Orbit, Metric rMinRadius, Metric rMaxRadius);
		static ALERROR CreateFromXML (CXMLElement *pDesc, const COrbit &OrbitDesc, CTerritoryDef **retpTerritory);
		inline const CString &GetCriteria (void) { return m_sCriteria; }
		bool HasAttribute (const CString &sAttrib);
		bool HasAttribute (const CVector &vPos, const CString &sAttrib);
		inline bool IsMarked (void) { return m_bMarked; }
		bool PointInTerritory (const CVector &vPos);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetCriteria (const CString &sCriteria) { m_sCriteria = sCriteria; }
		inline void SetMarked (bool bMarked = true) { m_bMarked = bMarked; }
		void WriteToStream (IWriteStream *pStream);

	private:
		struct SRegion
			{
			COrbit OrbitDesc;
			Metric rMinRadius;
			Metric rMaxRadius;

			Metric rMinRadius2;				//	Computed
			Metric rMaxRadius2;				//	Computed
			};

		bool MatchesCriteria (TArray<int> &Exclude, const CVector &vPos, const CString &sCriteria);

		CString m_sID;
		TArray<SRegion> m_Regions;
		CString m_sCriteria;
		CString m_sAttributes;

		bool m_bMarked;						//	Temporary mark
	};

class CTerritoryList
	{
	public:
		~CTerritoryList (void);

		inline int GetCount (void) { return m_List.GetCount(); }
		bool HasAttribute (const CVector &vPos, const CString &sAttrib);
		inline void Insert (CTerritoryDef *pTerritory) { m_List.Insert(pTerritory); }
		void ReadFromStream (SLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

	private:
		TArray<CTerritoryDef *> m_List;
	};

class CSystem : public CObject
	{
	public:
		//	Other defines

		enum LayerEnum
			{
			layerFirst = 0,

			layerBackground = 0,
			layerSpace = 1,
			layerStations = 2,
			layerShips = 3,
			layerEffects = 4,

			layerCount = 5
			};

		enum Flags
			{
			//	CreateWeaponFire flags
			CWF_WEAPON_FIRE =				0x00000001,	//	Creating a shot from a weapon
			CWF_FRAGMENT =					0x00000002,	//	Creating a fragment
			CWF_EXPLOSION =					0x00000004,	//	Creating an explosion (or fragment of an explosion)
			CWF_EJECTA =					0x00000008,	//	Creating ejecta (or fragments of ejecta)

			//	IsAreaClear flags
			IAC_FIXED_RADIUS =				0x00000001,	//	Don't use station exclusion zone
			IAC_INCLUDE_NON_ENEMIES =		0x00000002,	//	Non-enemies count
			IAC_INCLUDE_NON_ATTACKING =		0x00000004,	//	Include objects that cannot attack
			};

		//	System methods

		static ALERROR CreateEmpty (CUniverse *pUniv, CTopologyNode *pTopology, CSystem **retpSystem);
		static ALERROR CreateFromStream (CUniverse *pUniv, 
										 IReadStream *pStream, 
										 CSystem **retpSystem,
										 DWORD dwObjID = OBJID_NULL,
										 CSpaceObject **retpObj = NULL,
										 CSpaceObject *pPlayerShip = NULL);
		static ALERROR CreateFromXML (CUniverse *pUniv, 
									  CSystemType *pType, 
									  CTopologyNode *pTopology, 
									  CSystem **retpSystem,
									  CString *retsError = NULL,
									  CSystemCreateStats *pStats = NULL);
		virtual ~CSystem (void);

		ALERROR CreateMarker (CXMLElement *pDesc, const COrbit &oOrbit, CMarker **retpObj);
		ALERROR CreateParticles (CXMLElement *pDesc, const COrbit &oOrbit, CParticleEffect **retpObj);
		ALERROR CreateRandomEncounter (IShipGenerator *pTable, 
									   CSpaceObject *pBase,
									   CSovereign *pBaseSovereign,
									   CSpaceObject *pTarget,
									   CSpaceObject *pGate = NULL);
		ALERROR CreateShip (DWORD dwClassID,
							IShipController *pController,
							CDesignType *pOverride,
							CSovereign *pSovereign,
							const CVector &vPos,
							const CVector &vVel,
							int iRotation,
							CSpaceObject *pExitGate,
							SShipGeneratorCtx *pCtx,
							CShip **retpShip);
		ALERROR CreateShipwreck (CShipClass *pClass,
								 const CVector &vPos,
								 const CVector &vVel,
								 CSovereign *pSovereign,
								 CStation **retpWreck);
		ALERROR CreateStargate (CStationType *pType,
								CVector &vPos,
								const CString &sStargateID,
								const CString &sDestNodeID,
								const CString &sDestStargateID,
								CSpaceObject **retpStation = NULL);
		ALERROR CreateStation (CStationType *pType,
							   CVector &vPos,
							   CSpaceObject **retpStation = NULL);
		ALERROR CreateStation (SSystemCreateCtx *pCtx, 
							   CStationType *pType, 
							   const CVector &vPos,
							   const COrbit &OrbitDesc,
							   bool bCreateSatellites,
							   CXMLElement *pExtraData,
							   CSpaceObject **retpStation = NULL);
		ALERROR CreateWeaponFire (CWeaponFireDesc *pDesc,
								  int iBonus,
								  DestructionTypes iCause,
								  const CDamageSource &Source,
								  const CVector &vPos,
								  const CVector &vVel,
								  int iDirection,
								  CSpaceObject *pTarget,
								  DWORD dwFlags,
								  CSpaceObject **retpShot);
		ALERROR CreateWeaponFragments (CWeaponFireDesc *pDesc,
									   int iBonus,
									   DestructionTypes iCause,
									   const CDamageSource &Source,
									   CSpaceObject *pTarget,
									   const CVector &vPos,
									   const CVector &vVel,
									   CSpaceObject *pMissileSource);

		ALERROR AddTimedEvent (CTimedEvent *pEvent);
		inline void AddToDeleteList (CSpaceObject *pObj) { m_DeletedObjects.FastAdd(pObj); }
		ALERROR AddToSystem (CSpaceObject *pObj, int *retiIndex);
		int CalculateLightIntensity (const CVector &vPos, CSpaceObject **retpStar = NULL);
		COLORREF CalculateSpaceColor (CSpaceObject *pPOV);
		void CancelTimedEvent (CSpaceObject *pSource, const CString &sEvent, bool bInDoEvent = false);
		void CancelTimedEvent (CDesignType *pSource, const CString &sEvent, bool bInDoEvent = false);
		inline void EnumObjectsInBoxStart (SSpaceObjectGridEnumerator &i, const CVector &vUR, const CVector &vLL, DWORD dwFlags = 0) { m_ObjGrid.EnumStart(i, vUR, vLL, dwFlags); }
		inline void EnumObjectsInBoxStart (SSpaceObjectGridEnumerator &i, const CVector &vPos, Metric rRange, DWORD dwFlags = 0)
			{
			CVector vRange = CVector(rRange, rRange);
			CVector vUR = vPos + vRange;
			CVector vLL = vPos - vRange;
			m_ObjGrid.EnumStart(i, vUR, vLL, dwFlags);
			}
		inline bool EnumObjectsInBoxHasMore (SSpaceObjectGridEnumerator &i) { return i.bMore; }
		inline CSpaceObject *EnumObjectsInBoxGetNext (SSpaceObjectGridEnumerator &i) { return m_ObjGrid.EnumGetNext(i); }
		inline CSpaceObject *EnumObjectsInBoxGetNextFast (SSpaceObjectGridEnumerator &i) { return m_ObjGrid.EnumGetNextFast(i); }
		inline CSpaceObject *EnumObjectsInBoxPointGetNext (SSpaceObjectGridEnumerator &i) { return m_ObjGrid.EnumGetNextInBoxPoint(i); }
		CSpaceObject *FindObject (DWORD dwID);
		bool FindObjectName (CSpaceObject *pObj, CString *retsName = NULL);
		bool FireOnObjJumpPosAdj (CSpaceObject *pPos, CVector *iovPos);
		void FireOnSystemExplosion (CSpaceObject *pExplosion, CWeaponFireDesc *pDesc, const CDamageSource &Source);
		void FireOnSystemObjAttacked (SDamageCtx &Ctx);
		void FireOnSystemObjDestroyed (SDestroyCtx &Ctx);
		void FireOnSystemWeaponFire (CSpaceObject *pShot, CWeaponFireDesc *pDesc, const CDamageSource &Source);
		inline CSpaceObject *GetDestroyedObject (int iIndex) { return m_DeletedObjects.GetObj(iIndex); }
		inline int GetDestroyedObjectCount (void) { return m_DeletedObjects.GetCount(); }
		inline DWORD GetID (void) { return m_dwID; }
		inline int GetLastUpdated (void) { return m_iLastUpdated; }
		int GetLevel (void);
		CSpaceObject *GetNamedObject (const CString &sName);
		inline const CString &GetName (void) { return m_sName; }
		CNavigationPath *GetNavPath (CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd);
		CNavigationPath *GetNavPathByID (DWORD dwID);
		CSpaceObject *GetObject (int iIndex) { return (CSpaceObject *)m_AllObjects.GetObject(iIndex); }
		int GetObjectCount (void) { return m_AllObjects.GetCount(); }
		inline void GetObjectsInBox (const CVector &vPos, Metric rRange, CSpaceObjectList &Result)
			{
			CVector vRange = CVector(rRange, rRange);
			CVector vUR = vPos + vRange;
			CVector vLL = vPos - vRange;
			m_ObjGrid.GetObjectsInBox(vUR, vLL, Result);
			}
		inline void GetObjectsInBox (const CVector &vUR, const CVector &vLL, CSpaceObjectList &Result) { m_ObjGrid.GetObjectsInBox(vUR, vLL, Result); }
		CSpaceObject *GetPlayer (void) const;
		static DWORD GetSaveVersion (void);
		inline Metric GetSpaceScale (void) const { return m_rKlicksPerPixel; }
		inline int GetTick (void) { return m_iTick; }
		inline Metric GetTimeScale (void) const { return m_rTimeScale; }
		inline CTopologyNode *GetTopology (void) { return m_pTopology; }
		inline CSystemType *GetType (void) { return m_pType; }
		CSpaceEnvironmentType *GetSpaceEnvironment (int xTile, int yTile);
		CSpaceEnvironmentType *GetSpaceEnvironment (const CVector &vPos, int *retxTile = NULL, int *retyTile = NULL);
		CTopologyNode *GetStargateDestination (const CString &sStargate, CString *retsEntryPoint);
		inline CUniverse *GetUniverse (void) const { return g_pUniverse; }
		bool HasAttribute (const CVector &vPos, const CString &sAttrib);
		inline bool IsCreationInProgress (void) const { return (m_fInCreate ? true : false); }
		bool IsStationInSystem (CStationType *pType);
		inline bool IsTimeStopped (void) { return (m_iTimeStopped != 0); }
		void LoadImages (void);
		void MarkImages (void);
		void NameObject (const CString &sName, CSpaceObject *pObj);
		CVector OnJumpPosAdj (CSpaceObject *pObj, const CVector &vPos);
		void PaintViewport (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, bool bEnhanced);
		void PaintViewportGrid (CG16bitImage &Dest, const RECT &rcView, const ViewportTransform &Trans, const CVector &vCenter, Metric rGridSize);
		void PaintViewportObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj);
		void PaintViewportLRS (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, bool *retbNewEnemies);
		void PaintViewportMap (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rMapScale);
		void PaintViewportMapObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj);
		void PlaceInGate (CSpaceObject *pObj, CSpaceObject *pGate);
		void PlayerEntered (CSpaceObject *pPlayer);
		void ReadObjRefFromStream (SLoadCtx &Ctx, CSpaceObject **retpObj);
		void ReadSovereignRefFromStream (SLoadCtx &Ctx, CSovereign **retpSovereign);
		void RegisterEventHandler (CSpaceObject *pObj, Metric rRange);
		inline void RegisterForOnSystemCreated (CSpaceObject *pObj) { m_DeferredOnCreate.FastAdd(pObj); }
		void RemoveObject (SDestroyCtx &Ctx);
		void RemoveTimersForObj (CSpaceObject *pObj);
		void RestartTime (void);
		ALERROR SaveToStream (IWriteStream *pStream);
		inline void SetID (DWORD dwID) { m_dwID = dwID; }
		void SetLastUpdated (void);
		void SetObjectSovereign (CSpaceObject *pObj, CSovereign *pSovereign);
		void SetPOVLRS (CSpaceObject *pCenter);
		void SetSpaceEnvironment (int xTile, int yTile, CSpaceEnvironmentType *pEnvironment);
		ALERROR StargateCreated (CSpaceObject *pGate, const CString &sStargateID, const CString &sDestNodeID, const CString &sDestEntryPoint);
		void StopTime (const CSpaceObjectList &Targets, int iDuration);
		void StopTimeForAll (int iDuration, CSpaceObject *pExcept);
		void TransferObjEventsIn (CSpaceObject *pObj, CTimedEventList &ObjEvents);
		void TransferObjEventsOut (CSpaceObject *pObj, CTimedEventList &ObjEvents);
		void UnnameObject (CSpaceObject *pObj);
		void UnregisterEventHandler (CSpaceObject *pObj);
		void Update (Metric rSecondsPerTick, bool bForceEventFiring = false);
		void UpdateExtended (const CTimeSpan &ExtraTime);
		void WriteObjRefToStream (CSpaceObject *pObj, IWriteStream *pStream, CSpaceObject *pReferrer = NULL);
		void WriteSovereignRefToStream (CSovereign *pSovereign, IWriteStream *pStream);

		//	Locations & Territories
		ALERROR AddTerritory (CTerritoryDef *pTerritory);
		inline void BlockOverlappingLocations (void) { m_Locations.FillCloseLocations(); }
		int CalcLocationWeight (int iLocID, const CAttributeCriteria &Criteria);
		ALERROR CreateLocation (const CString &sID, const COrbit &Orbit, const CString &sAttributes, CLocationDef **retpLocation = NULL);
		bool FindRandomLocation (const CAttributeCriteria &Criteria, DWORD dwFlags, CStationType *pStationToPlace, int *retiLocID);
		int GetEmptyLocationCount (void);
		inline bool GetEmptyLocations (TArray<int> *retList) { return m_Locations.GetEmptyLocations(retList); }
		inline CLocationDef *GetLocation (int iLocID) { return m_Locations.GetLocation(iLocID); }
		inline int GetLocationCount (void) { return m_Locations.GetCount(); }
		bool IsAreaClear (const CVector &vPos, Metric rRadius, DWORD dwFlags, CStationType *pType = NULL);
		inline void SetLocationObjID (int iLocID, DWORD dwObjID) { m_Locations.SetObjID(iLocID, dwObjID); }

	private:
		struct CStar
			{
			int x;
			int y;
			WORD wColor;
			WORD wDistance;

			bool bBrightStar;
			WORD wSpikeColor;
			};

		CSystem (void);
		CSystem (CUniverse *pUniv, CTopologyNode *pTopology);

		void ComputeMapLabels (void);
		void ComputeRandomEncounters (void);
		void ComputeStars (void);
		static void ConvertSpaceEnvironmentToPointers (CTileMap &UNIDs, CTileMap **retpPointers);
		static void ConvertSpaceEnvironmentToUNIDs (CTileMap &Pointers, CTileMap **retpUNIDs);
		ALERROR CreateStarField (int cxFieldWidth, int cyFieldHeight);
		ALERROR CreateStationInt (CStationType *pType,
								  const CVector &vPos,
								  const CVector &vVel,
								  CXMLElement *pExtraData,
								  CSpaceObject **retpStation,
								  CString *retsError = NULL);
		void FlushEnemyObjectCache (void);
		inline int GetTimedEventCount (void) { return m_TimedEvents.GetCount(); }
		inline CTimedEvent *GetTimedEvent (int iIndex) { return m_TimedEvents.GetEvent(iIndex); }
		void PaintDestinationMarker (CG16bitImage &Dest, int x, int y, CSpaceObject *pObj, CSpaceObject *pCenter);
		void PaintStarField(CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rKlicksPerPixel, COLORREF rgbSpaceColor);
		void ResetStarField (void);
		void UpdateRandomEncounters (void);

		//	Game instance data

		DWORD m_dwID;							//	System ID
		CSystemType *m_pType;					//	System type definition
		CString m_sName;						//	Name of system
		CTopologyNode *m_pTopology;				//	Topology descriptor

		CObjectArray m_AllObjects;				//	Array of CSpaceObject
		TSortMap<CString, CSpaceObject *> m_NamedObjects;			//	Indexed array of named objects (CSpaceObject *)

		CTimedEventList m_TimedEvents;			//	Array of CTimedEvent
		CTileMap *m_pEnvironment;				//	Nebulas, etc.
		CSystemEventHandlerNode m_EventHandlers;	//	List of system event handler
		CNavigationPathNode m_NavPaths;			//	List of navigation paths
		CLocationList m_Locations;				//	List of point locations
		CTerritoryList m_Territories;			//	List of defined territories

		int m_iTick;							//	Ticks since beginning of time
		int m_iNextEncounter;					//	Time of next random encounter
		int m_iTimeStopped;						//	Ticks until time restarts
		Metric m_rKlicksPerPixel;				//	Space scale
		Metric m_rTimeScale;					//	Time scale
		int m_iLastUpdated;						//	Tick on which system was last updated (-1 = never)

		DWORD m_fNoRandomEncounters:1;			//	TRUE if no random encounters in this system
		DWORD m_fInCreate:1;					//	TRUE if system in being created
		DWORD m_fEncounterTableValid:1;			//	TRUE if m_pEncounterObj is valid
		DWORD m_fUseDefaultTerritories:1;		//	TRUE if we should use defaults for innerZone, lifeZone, outerZone
		DWORD m_fSpare:28;

		//	Support structures

		CStructArray m_StarField;				//	Star field
		CSpaceObjectList m_EncounterObjs;		//	List of objects that generate encounters
		CSpaceObjectList m_BarrierObjects;		//	List of barrier objects
		CSpaceObjectList m_Stars;				//	List of stars in the system
		CSpaceObjectGrid m_ObjGrid;				//	Grid to help us hit test
		CSpaceObjectList m_DeletedObjects;		//	List of objects deleted in the current update
		CSpaceObjectList m_LayerObjs[layerCount];	//	List of objects by layer
		CSpaceObjectList m_EnhancedDisplayObjs;	//	List of objects to show in viewport periphery
		CSpaceObjectList m_BackgroundObjs;		//	List of background objects to paint in viewport
		CSpaceObjectList m_ForegroundObjs;		//	List of foreground objects to paint in viewport
		CSpaceObjectList m_DeferredOnCreate;	//	Ordered list of objects that need an OnSystemCreated call

		static const Metric g_MetersPerKlick;

	friend CObjectClass<CSystem>;
	};

//	Miscellaneous Structures & Classes

class CParticleArray
	{
	public:
		struct SParticle
			{
			CVector Pos;						//	Position. Valid if we use real coordinates
			CVector Vel;						//	Velocity. Valid if we use real coordinates
												//		NOTE: In Km per tick (unlike normal velocities)

			int x;								//	Offset from center of particle cloud
			int y;								//		(screen-coords, in 256ths of pixels)
												//		(valid in all cases)
			int xVel;							//	Velocity relative to particle cloud
			int yVel;							//		(screen-coords, in 256ths of pixels per tick)
												//		(not valid if using real coordinates)

			int iLifeLeft;						//	Ticks of life left
			int iDestiny;						//	Random number from 1-360
			int iRotation;						//	Particle rotation
			DWORD dwData;						//	Miscellaneous data for particle

			DWORD fAlive:1;						//	TRUE if particle is alive
			DWORD dwSpare:31;					//	Spare
			};

		CParticleArray (void);
		~CParticleArray (void);

		void AddParticle (const CVector &vPos, const CVector &vVel, int iLifeLeft = -1, int iRotation = -1, int iDestiny = -1, DWORD dwData = 0);
		const RECT &GetBounds (void) const { return m_rcBounds; }
		void GetBounds (CVector *retvUR, CVector *retvLL);
		inline int GetCount (void) const { return m_iCount; }
		inline SParticle *GetArray (int *retiCount = NULL) const { if (retiCount) *retiCount = m_iCount; return m_pArray; }
		inline const CVector &GetOrigin (void) const { return m_vOrigin; }
		void Init (int iMaxCount, const CVector &vOrigin = NullVector);
		void Paint (CG16bitImage &Dest,
					int xPos,
					int yPos,
					SViewportPaintCtx &Ctx,
					SParticlePaintDesc &Desc);
		void Paint (CG16bitImage &Dest,
					int xPos,
					int yPos,
					SViewportPaintCtx &Ctx,
					IEffectPainter *pPainter);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetOrigin (const CVector &vOrigin) { m_vOrigin = vOrigin; }
		void Update (SEffectUpdateCtx &Ctx);
		void UpdateMotionLinear (bool *retbAlive = NULL, CVector *retvAveragePos = NULL);
		void UpdateRingCohesion (Metric rRadius, Metric rMinRadius, Metric rMaxRadius, int iCohesion, int iResistance);
		void UpdateTrackTarget (CSpaceObject *pTarget, int iManeuverRate, Metric rMaxSpeed);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		void CleanUp (void);
		void PaintFireAndSmoke (CG16bitImage &Dest, 
								int xPos, 
								int yPos, 
								SViewportPaintCtx &Ctx, 
								int iLifetime, 
								int iMinWidth,
								int iMaxWidth,
								int iCore,
								int iFlame,
								int iSmoke,
								int iSmokeBrightness);
		void PaintGaseous (CG16bitImage &Dest,
						   int xPos,
						   int yPos,
						   SViewportPaintCtx &Ctx,
						   int iMaxLifetime,
						   int iMinWidth,
						   int iMaxWidth,
						   WORD wPrimaryColor,
						   WORD wSecondaryColor);
		void PaintImage (CG16bitImage &Dest, int xPos, int yPos, SViewportPaintCtx &Ctx, SParticlePaintDesc &Desc);
		void PaintLine (CG16bitImage &Dest,
						int xPos,
						int yPos,
						SViewportPaintCtx &Ctx,
						WORD wPrimaryColor);
		void PosToXY (const CVector &xy, int *retx, int *rety);
		void UseRealCoords (void);
		CVector XYToPos (int x, int y);

		int m_iCount;
		SParticle *m_pArray;
		RECT m_rcBounds;						//	Bounding box in pixels relative to center
		CVector m_vOrigin;						//	Origin position
		CVector m_vCenterOfMass;				//	Center of mass
		CVector m_vUR;							//	Bounds
		CVector m_vLL;							//	Bounds

		int m_iLastAdded;						//	Index of last particle added
		bool m_bUseRealCoords;					//	If TRUE, we keep real (instead of int) coordinates
	};

class CShockwaveHitTest
	{
	public:
		void Init (int iSegments, int iLives);
		inline bool IsEmpty (void) const { return (m_Segments.GetCount() == 0); }
		void ReadFromStream (SLoadCtx &Ctx);
		void Update (SEffectUpdateCtx &Ctx, const CVector &vPos, Metric rMinRadius, Metric rMaxRadius);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		void CalcObjBounds (CSpaceObject *pObj, const CVector &vPos, Metric *retrStartAngle, Metric *retrSizeAngle, Metric *retrStartRadius, Metric *retrEndRadius);

		TArray<int> m_Segments;
	};

//	Topology

class CTopology
	{
	public:
		CTopology (void);
		~CTopology (void);

		ALERROR AddTopology (STopologyCreateCtx &Ctx);
		ALERROR AddTopologyDesc (STopologyCreateCtx &Ctx, CTopologyDesc *pNode, CTopologyNode **retpNewNode = NULL);
		void DeleteAll (void);
		CTopologyNode *FindTopologyNode (const CString &sID);
		int GetDistance (const CString &sSourceID, const CString &sDestID);
		inline CTopologyNodeList &GetTopologyNodeList (void) { return m_Topology; }
		inline CTopologyNode *GetTopologyNode (int iIndex) { return m_Topology.GetAt(iIndex); }
		inline int GetTopologyNodeCount (void) { return m_Topology.GetCount(); }
		ALERROR InitComplexArea (CXMLElement *pAreaDef, int iMinRadius, CComplexArea *retArea, STopologyCreateCtx *pCtx = NULL, CTopologyNode **iopExit = NULL); 
		void ReadFromStream (SUniverseLoadCtx &Ctx);

	private:
		enum NodeTypes
			{
			typeStandard,
			typeFragmentStart,
			typeFragment,
			};

		ALERROR AddFragment (STopologyCreateCtx &Ctx, CTopologyDesc *pFragment, CTopologyNode **retpNewNode);
		ALERROR AddNetwork (STopologyCreateCtx &Ctx, CTopologyDesc *pNetwork, CTopologyNode **retpNewNode);
		ALERROR AddNode (STopologyCreateCtx &Ctx, CTopologyDesc *pNode, CTopologyNode **retpNewNode);
		ALERROR AddNodeTable (STopologyCreateCtx &Ctx, CTopologyDesc *pTable, CTopologyNode **retpNewNode);

		ALERROR AddRandom (STopologyCreateCtx &Ctx, CTopologyDesc *pDesc, CTopologyNode **retpNewNode);
		ALERROR AddRandomParsePosition (STopologyCreateCtx *pCtx, const CString &sValue, CTopologyNode **iopExit, int *retx, int *rety);
		ALERROR AddRandomRegion (STopologyCreateCtx &Ctx, CTopologyDesc *pDesc, CXMLElement *pRegionDef, CTopologyNode *&pExitNode, CIntGraph &Graph, TArray<CTopologyNode *> &Nodes);

		ALERROR AddStargate (STopologyCreateCtx &Ctx, CTopologyNode *pNode, bool bRootNode, CXMLElement *pGateDesc);
		ALERROR AddTopologyNode (STopologyCreateCtx &Ctx,
								 const CString &sID,
								 int x,
								 int y,
								 const CString &sAttribs,
								 CXMLElement *pSystemDesc,
								 CEffectCreator *pEffect,
								 CTopologyNode **retpNode = NULL);
		ALERROR AddTopologyNode (const CString &sID, CTopologyNode *pNode);
		CString ExpandNodeID (STopologyCreateCtx &Ctx, const CString &sID);
		ALERROR FindTopologyDesc (STopologyCreateCtx &Ctx, const CString &sNodeID, CTopologyDesc **retpNode, NodeTypes *retiNodeType = NULL);
		CString GenerateUniquePrefix (const CString &sPrefix, const CString &sTestNodeID);
		void GetAbsoluteDisplayPos (STopologyCreateCtx &Ctx, int x, int y, int *retx, int *rety, int *retiRotation);
		int GetDistance (CTopologyNode *pSource, const CString &sDestID, int iBestDist = -1);
		void GetFragmentDisplayPos (STopologyCreateCtx &Ctx, CTopologyNode *pNode, int *retx, int *rety);
		ALERROR GetOrAddTopologyNode (STopologyCreateCtx &Ctx, const CString &sID, CTopologyNode *pPrevNode, CXMLElement *pGateDesc, CTopologyNode **retpNode);

		CTopologyNodeList m_Topology;
		TSortMap<CString, int> m_IDToNode;
	};

//	Events

class CTimedEncounterEvent : public CTimedEvent
	{
	public:
		CTimedEncounterEvent (void) { } //	Used only for loading
		CTimedEncounterEvent (int iTick,
							  CSpaceObject *pTarget,
							  DWORD dwEncounterTableUNID,
							  CSpaceObject *pGate,
							  Metric rDistance);

		virtual CString DebugCrashInfo (void);
		virtual void DoEvent (CSystem *pSystem);
		virtual bool OnObjDestroyed (CSpaceObject *pObj);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteClassToStream (IWriteStream *pStream);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		CSpaceObject *m_pTarget;
		DWORD m_dwEncounterTableUNID;
		CSpaceObject *m_pGate;
		Metric m_rDistance;
	};

class CTimedCustomEvent : public CTimedEvent
	{
	public:
		CTimedCustomEvent (void) { }	//	Used only for loading
		CTimedCustomEvent (int iTick,
						   CSpaceObject *pObj,
						   const CString &sEvent);

		virtual CString DebugCrashInfo (void);
		virtual void DoEvent (CSystem *pSystem);
		virtual CString GetEventHandlerName (void) { return m_sEvent; }
		virtual CSpaceObject *GetEventHandlerObj (void) { return m_pObj; }
		virtual bool OnObjChangedSystems (CSpaceObject *pObj);
		virtual bool OnObjDestroyed (CSpaceObject *pObj);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteClassToStream (IWriteStream *pStream);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		CSpaceObject *m_pObj;
		CString m_sEvent;
	};

class CTimedRecurringEvent : public CTimedEvent
	{
	public:
		CTimedRecurringEvent (void) { }	//	Used only for loading
		CTimedRecurringEvent (int iInterval,
							  CSpaceObject *pObj,
							  const CString &sEvent);

		virtual CString DebugCrashInfo (void);
		virtual void DoEvent (CSystem *pSystem);
		virtual CString GetEventHandlerName (void) { return m_sEvent; }
		virtual CSpaceObject *GetEventHandlerObj (void) { return m_pObj; }
		virtual bool OnObjChangedSystems (CSpaceObject *pObj);
		virtual bool OnObjDestroyed (CSpaceObject *pObj);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteClassToStream (IWriteStream *pStream);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		int m_iInterval;
		CSpaceObject *m_pObj;
		CString m_sEvent;
	};

class CTimedTypeEvent : public CTimedEvent
	{
	public:
		CTimedTypeEvent (void) { }	//	Used only for loading
		CTimedTypeEvent (int iTick,
						 int iInterval,
						 CDesignType *pType,
						 const CString &sEvent);

		virtual CString DebugCrashInfo (void);
		virtual void DoEvent (CSystem *pSystem);
		virtual CString GetEventHandlerName (void) { return m_sEvent; }
		virtual CDesignType *GetEventHandlerType (void) { return m_pType; }

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteClassToStream (IWriteStream *pStream);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		int m_iInterval;			//	0 = not recurring
		CDesignType *m_pType;
		CString m_sEvent;
	};

//	Docking ports implementation

class CDockingPorts
	{
	public:
		CDockingPorts (void);
		~CDockingPorts (void);

		void DockAtRandomPort (CSpaceObject *pOwner, CSpaceObject *pObj);
		int FindNearestEmptyPort (CSpaceObject *pOwner, CSpaceObject *pRequestingObj, CVector *retvDistance = NULL);
		int FindRandomEmptyPort (CSpaceObject *pOwner);
		inline int GetPortCount (CSpaceObject *pOwner) { return m_iPortCount; }
		inline CSpaceObject *GetPortObj (CSpaceObject *pOwner, int iPort) { return m_pPort[iPort].pObj; }
		CVector GetPortPos (CSpaceObject *pOwner, int iPort);
		int GetPortsInUseCount (CSpaceObject *pOwner);
		void InitPorts (CSpaceObject *pOwner, int iCount, Metric rRadius);
		void InitPorts (CSpaceObject *pOwner, int iCount, CVector *pPos);
		void InitPortsFromXML (CSpaceObject *pOwner, CXMLElement *pElement);
		inline bool IsObjDocked (CSpaceObject *pObj) { return IsDocked(pObj); }
		inline bool IsObjDockedOrDocking (CSpaceObject *pObj) { return IsDockedOrDocking(pObj); }
		void MoveAll (CSpaceObject *pOwner);
		void OnDestroyed (void);
		void OnObjDestroyed (CSpaceObject *pOwner, CSpaceObject *pObj, bool *retbDestroyed = NULL);
		void ReadFromStream (CSpaceObject *pOwner, SLoadCtx &Ctx);
		void RepairAll (CSpaceObject *pOwner, int iRepairRate);
		bool RequestDock (CSpaceObject *pOwner, CSpaceObject *pObj);
		void Undock (CSpaceObject *pOwner, CSpaceObject *pObj);
		void UpdateAll (CSpaceObject *pOwner);
		void WriteToStream (CSpaceObject *pOwner, IWriteStream *pStream);

	private:
		enum DockingPortStatus
			{
			psEmpty,
			psDocking,
			psInUse
			};

		struct DockingPort
			{
			DockingPortStatus iStatus;			//	Status of port
			CSpaceObject *pObj;					//	Object docked at this port
			CVector vPos;						//	Position of dock (relative coords)
			int iRotation;						//	Rotation of ship at dock
			};

		bool IsDocked (CSpaceObject *pObj);
		bool IsDockedOrDocking (CSpaceObject *pObj);
		bool ShipsNearPort (CSpaceObject *pOwner, CSpaceObject *pRequestingObj, const CVector &vPortPos);

		int m_iPortCount;						//	Number of docking ports
		DockingPort *m_pPort;					//	Array of docking ports
	};

//	Attack Detector

class CAttackDetector
	{
	public:
		CAttackDetector (void);

		inline void Blacklist (void) { m_iCounter = -1; }
		inline bool IsBlacklisted (void) { return m_iCounter == -1; }
		bool Hit (int iTick);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void Update (int iTick) { if ((iTick % DECAY_RATE) == 0) OnUpdate(); }
		void WriteToStream (IWriteStream *pStream);

	private:
		enum Constants
			{
			DECAY_RATE = 150,
			HIT_LIMIT = 3,
			MULTI_HIT_WINDOW = 20,
			};

		void OnUpdate (void);

		int m_iCounter;
		int m_iLastHit;
	};

//	Item Event Handler

enum EItemEventDispatchTypes
	{
	dispatchFireEvent,
	dispatchCheckEnhancementLifetime,
	};

class CItemEventDispatcher
	{
	public:
		CItemEventDispatcher (void);
		~CItemEventDispatcher (void);

		inline void FireEvent (CSpaceObject *pSource, ECodeChainEvents iEvent)	{ if (m_pFirstEntry) FireEventFull(pSource, iEvent); }
		inline void FireUpdateEvents (CSpaceObject *pSource) { if (m_pFirstEntry) FireUpdateEventsFull(pSource); }
		void Init (CSpaceObject *pSource);

	private:
		struct SEntry
			{
			EItemEventDispatchTypes iType;				//	Type of entry
			CItem theItem;								//	Item

			ECodeChainEvents iEvent;					//	Event (if dispatchFireEvent)
			SEventHandlerDesc Event;					//	Code (if dispatchFireEvent)

			DWORD dwEnhancementID;						//	ID of enhancement (if relevant)

			SEntry *pNext;
			};

		void AddEntry (const CString &sEvent, EItemEventDispatchTypes iType, const SEventHandlerDesc &Event, const CItem &Item, DWORD dwEnhancementID);
		SEntry *AddEntry (void);
		void FireEventFull (CSpaceObject *pSource, ECodeChainEvents iEvent);
		void FireUpdateEventsFull (CSpaceObject *pSource);
		void RemoveAll (void);

		SEntry *m_pFirstEntry;
	};

//	Ship classes and types

class CEnergyField
	{
	public:
		CEnergyField (void);
		~CEnergyField (void);
		static void CreateFromType (CEnergyFieldType *pType, 
									int iPosAngle,
									int iPosRadius,
									int iRotation,
									int iLifeLeft, 
									CEnergyField **retpField);

		bool AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx);
		void Destroy (CSpaceObject *pSource);
		void FireCustomEvent (CSpaceObject *pSource, const CString &sEvent, ICCItem **retpResult);
		void FireOnCreate (CSpaceObject *pSource);
		bool FireOnDamage (CSpaceObject *pSource, SDamageCtx &Ctx);
		void FireOnDestroy (CSpaceObject *pSource);
		inline const CString &GetData (const CString &sAttrib) { return m_Data.GetData(sAttrib); }
		inline int GetDevice (void) const { return m_iDevice; }
		inline DWORD GetID (void) const { return m_dwID; }
		inline CEnergyField *GetNext (void) const { return m_pNext; }
		CVector GetPos (CSpaceObject *pSource);
		inline int GetRotation (void) const { return m_iRotation; }
		inline CEnergyFieldType *GetType (void) const { return m_pType; }
		inline bool IsDestroyed (void) const { return (m_fDestroyed ? true : false); }
		void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetData (const CString &sAttrib, const CString &sData) { m_Data.SetData(sAttrib, sData); }
		inline void SetDevice (int iDev) { m_iDevice = iDev; }
		inline void SetNext (CEnergyField *pNext) { m_pNext = pNext; }
		void SetPos (CSpaceObject *pSource, const CVector &vPos);
		inline void SetRotation (int iRotation) { m_iRotation = iRotation; }
		void Update (CSpaceObject *pSource);
		void WriteToStream (IWriteStream *pStream);

	private:
		void FireOnUpdate (CSpaceObject *pSource);
		void CreateHitEffect (CSpaceObject *pSource, SDamageCtx &Ctx);

		CEnergyFieldType *m_pType;				//	Type of field
		DWORD m_dwID;							//	Universal ID
		int m_iLifeLeft;						//	Ticks left of energy field life (-1 = permanent)
		int m_iDevice;							//	Index of device that we're associated with (-1 if not a device)

		int m_iPosAngle;						//	Position relative to source (degrees)
		int m_iPosRadius;						//	Position relative to source (pixels)
		int m_iRotation;						//	Overlay orientation (degrees)

		CAttributeDataBlock m_Data;				//	data

		IEffectPainter *m_pPainter;				//	Painter

		int m_iPaintHit;						//	If >0 then we paint a hit
		int m_iPaintHitTick;					//	Tick for hit painter
		IEffectPainter *m_pHitPainter;			//	Hit painter

		DWORD m_fDestroyed:1;					//	TRUE if field should be destroyed

		CEnergyField *m_pNext;					//	Next energy field associated with this object
	};

class CEnergyFieldList
	{
	public:
		CEnergyFieldList (void);
		~CEnergyFieldList (void);

		void AddField (CSpaceObject *pSource, 
					   CEnergyFieldType *pType, 
					   int iPosAngle,
					   int iPosRadius,
					   int iRotation,
					   int iLifeLeft, 
					   DWORD *retdwID = NULL);
		bool AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx);
		bool AbsorbsWeaponFire (CInstalledDevice *pDevice);
		CEnergyField *GetOverlay (DWORD dwID) const;
		const CString &GetData (DWORD dwID, const CString &sAttrib);
		CVector GetPos (CSpaceObject *pSource, DWORD dwID);
		void GetList (TArray<CEnergyField *> &List);
		int GetRotation (DWORD dwID);
		CEnergyFieldType *GetType (DWORD dwID);
		int GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource);
		inline bool IsEmpty (void) { return (m_pFirst == NULL); }
		void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void ReadFromStream (SLoadCtx &Ctx, CSpaceObject *pSource);
		void RemoveField (CSpaceObject *pSource, DWORD dwID);
		void SetData (DWORD dwID, const CString &sAttrib, const CString &sData);
		void SetPos (CSpaceObject *pSource, DWORD dwID, const CVector &vPos);
		void SetRotation (DWORD dwID, int iRotation);
		void Update (CSpaceObject *pSource, bool *retbModified);
		void WriteToStream (IWriteStream *pStream);

	private:
		CEnergyField *m_pFirst;
	};

//	Particle Field Type

#if 0
class CParticleGroup;

class IParticleAppearance
	{
	public:
		virtual void Paint (CG16bitImage &Dest,
							const ViewportTransform &Trans,
							CParticleGroup *pGroup) = 0;
	};

class IParticleBehavior
	{
	};

class IParticleMotion
	{
	};

class CParticleGroupType : public CObject
	{
	public:
		static ALERROR CreateFromXML (CXMLElement *pDesc, CParticleGroupType **retpType);

		inline IParticleAppearance *GetAppearance (void) { return m_pAppearance; }
		inline IParticleBehavior *GetBehavior (void) { return m_pBehavior; }
		inline IParticleMotion *GetMotion (void) { return m_pMotion; }
		inline DWORD GetUNID (void) { return m_dwUNID; }

	private:
		CParticleGroupType (void);

		DWORD m_dwUNID;

		IParticleAppearance *m_pAppearance;
		IParticleBehavior *m_pBehavior;
		IParticleMotion *m_pMotion;

	friend CObjectClass<CParticleGroupType>;
	};

struct SParticle
	{
	inline bool IsAlive (void) { return iLifeLeft != -1; }

	int iDestiny;					//	Particle randomization
	int iLifeLeft;					//	Ticks left in life (-1 if particle is dead)
	CVector vPos;			//	Position of particle
	CVector vVel;			//	Velocity of particle
	};

class CParticleGroup
	{
	public:
		static ALERROR CreateFromType (CParticleGroupType *pType,
									   CSpaceObject *pAnchor,
									   CParticleGroup **retpGroup);
		~CParticleGroup (void);

		void Paint (CG16bitImage &Dest, const ViewportTransform &Trans);

	private:
		CParticleGroup (void);

		CParticleGroupType *m_pType;	//	Type of particle group

		int m_iCount;					//	Number of particles in the array
		int m_iAlive;					//	Number of particles that are alive in the group
		SParticle *m_pParticles;		//	Array of particles
	};
#endif

//	Space Objects

#define MAX_COMMS_CAPS					25

class CSpaceObject : public CObject
	{
	public:
		enum Categories
			{
			catShip =			0x00000001,
			catStation =		0x00000002,
			catBeam =			0x00000004,
			catMissile =		0x00000008,
			catFractureEffect =	0x00000010,
			catOther =			0x80000000,
			};

		enum StealthAndPerceptionLevels
			{
			stealthMin =			0,
			stealthNormal =			4,
			stealthMax =			15,			//	Cloaked

			perceptMin =			0,
			perceptNormal =			4,
			perceptMax =			15,
			};

		enum InterSystemResults
			{
			interNoAction =					0,
			interFollowPlayer =				1,
			interWaitForPlayer =			2,
			};

		enum CriteriaSortTypes
			{
			sortNone,
			sortByDistance,
			};

		struct Criteria
			{
			CSpaceObject *pSource;				//	Source

			DWORD dwCategories;					//	Only these object categories
			bool bSelectPlayer;					//	Select the player
			bool bIncludeVirtual;				//	Include virtual objects
			bool bActiveObjectsOnly;			//	Only active object (e.g., objects that can attack)
			bool bKilledObjectsOnly;			//	Only objects that cannot attack
			bool bFriendlyObjectsOnly;			//	Only friendly to source
			bool bEnemyObjectsOnly;				//	Only enemy to source
			bool bManufacturedObjectsOnly;		//	Exclude planets, stars, etc.
			bool bStructureScaleOnly;			//	Only structure-scale objects
			bool bStargatesOnly;				//	Only stargates
			bool bNearestOnly;					//	The nearest object to the source
			bool bFarthestOnly;					//	The fartest object to the source
			bool bNearerThan;					//	Only objects nearer than rMinRadius
			bool bFartherThan;					//	Only objects farther than rMaxRadius
			bool bHomeBaseIsSource;				//	Only objects whose home base is the source
			bool bDockedWithSource;				//	Only objects currently docked with source
			bool bExcludePlayer;				//	Exclude the player
			bool bTargetIsSource;				//	Only objects whose target is the source

			bool bPerceivableOnly;				//	Only objects that can be perceived by the source
			int iPerception;					//	Cached perception of pSource

			bool bSourceSovereignOnly;			//	Only objects the same sovereign as source
			DWORD dwSovereignUNID;				//	Only objects with this sovereign UNID

			CString sData;						//	Only objects with non-Nil data
			CString sStargateID;				//	Only objects with this stargate ID (if non blank)
			Metric rMinRadius;					//	Only objects at or beyond the given radius
			Metric rMaxRadius;					//	Only objects within the given radius
			int iIntersectAngle;				//	Only objects that intersect line from source
			IShipController::OrderTypes iOrder;	//	Only objects with this order

			TArray<CString> AttribsRequired;	//	Required attributes
			TArray<CString> AttribsNotAllowed;	//	Exclude objects with these attributes
			TArray<CString> SpecialRequired;	//	Special required attributes
			TArray<CString> SpecialNotAllowed;	//	Special excluding attributes

			int iEqualToLevel;					//	Objects of this level
			int iGreaterThanLevel;
			int iLessThanLevel;

			CriteriaSortTypes iSort;
			ESortOptions iSortOrder;
			};

		struct SCriteriaMatchCtx
			{
			SCriteriaMatchCtx (const Criteria &Crit);

			//	Computed from criteria
			Metric rMinRadius2;
			Metric rMaxRadius2;

			//	Nearest/farthest object
			CSpaceObject *pBestObj;
			Metric rBestDist2;

			//	Sorted results
			TSortMap<Metric, CSpaceObject *> DistSort;

			//	Temporaries
			bool bCalcPolar;
			bool bCalcDist2;
			};

		CSpaceObject (IObjectClass *pClass);
		virtual ~CSpaceObject (void);
		static void CreateFromStream (SLoadCtx &Ctx, CSpaceObject **retpObj);

		void Accelerate (const CVector &vPush, Metric rSeconds);
		void AccelerateStop (Metric rPush, Metric rSeconds);
		void AddEffect (IEffectPainter *pPainter, const CVector &vPos, int iTick = 0, int iRotation = 0);
		ALERROR AddToSystem (CSystem *pSystem);
		inline bool Blocks (CSpaceObject *pObj) { return (m_fIsBarrier && CanBlock(pObj)); }
		inline bool BlocksShips (void) { return (m_fIsBarrier && CanBlockShips()); }
		inline bool CanBeControlled (void) { return m_iControlsFrozen == 0; }
		inline bool CanBeHit (void) { return !m_fCannotBeHit; }
		inline bool CanBeHitByFriends (void) { return !m_fNoFriendlyTarget; }
		inline bool CanBounce (void) { return m_fCanBounce; }
		bool CanDetect (int Perception, CSpaceObject *pObj);
		bool CanCommunicateWith (CSpaceObject *pSender, bool bNoOnShowCheck = false);
		inline bool CanHitFriends (void) { return !m_fNoFriendlyFire; }
		inline void ClearAutoClearDestination (void) { m_fAutoClearDestination = false; }
		inline void ClearPaintNeeded (void) { m_fPaintNeeded = false; }
		inline void ClearPlayerDestination (void) { m_fPlayerDestination = false; }
		inline void ClearPlayerDocked (void) { m_fPlayerDocked = false; }
		inline void ClearPlayerTarget (void) { m_fPlayerTarget = false; }
		inline void ClearPOVLRS (void) { m_fInPOVLRS = false; }
		inline void ClearSelection (void) { m_fSelected = false; }
		inline void ClearShowDistanceAndBearing (void) { m_fShowDistanceAndBearing = false; }
		inline void ClipSpeed (Metric rMaxSpeed) { m_vVel.Clip(rMaxSpeed); }
		void CommsMessageFrom (CSpaceObject *pSender, int iIndex);
		inline DWORD Communicate (CSpaceObject *pReceiver, MessageTypes iMessage, CSpaceObject *pParam1 = NULL, DWORD dwParam2 = 0) { return pReceiver->OnCommunicate(this, iMessage, pParam1, dwParam2); }
		void CopyDataFromObj (CSpaceObject *pSource);
		ALERROR CreateRandomItems (CXMLElement *pItems, int iLevel = 1);
		ALERROR CreateRandomItems (IItemGenerator *pItems, int iLevel = 1);
		EDamageResults Damage (SDamageCtx &Ctx);
		static CString DebugDescribe (CSpaceObject *pObj);
		inline bool DebugIsValid (void) { return (DWORD)m_pSystem != 0xdddddddd; }
		inline const CVector &DeltaV (const CVector &vDelta) { m_vVel = m_vVel + vDelta; return m_vVel; }
		void Destroy (DestructionTypes iCause, const CDamageSource &Attacker, CSpaceObject **retpWreck = NULL);
		void EnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		int FindCommsMessage (const CString &sName);
		bool FindEventHandler (const CString &sEntryPoint, SEventHandlerDesc *retEvent = NULL);
		bool FindEventHandler (CDesignType::ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL);
		bool FireCanDockAsPlayer (CSpaceObject *pDockTarget, CString *retsError);
		void FireCustomEvent (const CString &sEvent, ECodeChainEvents iEvent = eventNone, ICCItem **retpResult = NULL);
		void FireCustomItemEvent (const CString &sEvent, const CItem &Item, ICCItem **retpResult = NULL);
		void FireCustomOverlayEvent (const CString &sEvent, DWORD dwID, ICCItem **retpResult = NULL);
		bool FireGetDockScreen (CString *retsScreen, int *retiPriority);
		void FireGetExplosionType (SExplosionType *retExplosion);
		void FireOnAttacked (SDamageCtx &Ctx);
		void FireOnAttackedByPlayer (void);
		void FireOnCreate (void);
		void FireOnCreate (SShipGeneratorCtx *pShipCtx);
		void FireOnCreateOrders (CSpaceObject *pBase, CSpaceObject *pTarget);
		void FireOnDamage (SDamageCtx &Ctx);
		void FireOnDestroy (const SDestroyCtx &Ctx);
		bool FireOnDockObjAdj (CSpaceObject **retpObj);
		void FireOnEnteredGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pGate);
		void FireOnEnteredSystem (CSpaceObject *pGate);
		void FireOnItemAIUpdate (void);
		void FireOnItemObjDestroyed (const SDestroyCtx &Ctx);
		void FireOnItemUpdate (void);
		void FireOnLoad (SLoadCtx &Ctx);
		void FireOnMining (const SDamageCtx &Ctx);
		void FireOnObjDestroyed (const SDestroyCtx &Ctx);
		void FireOnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget);
		void FireOnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		void FireOnObjJumped (CSpaceObject *pObj);
		bool FireOnObjJumpPosAdj (CSpaceObject *pObj, CVector *iovPos);
		void FireOnObjReconned (CSpaceObject *pObj);
		void FireOnOrderChanged (void);
		void FireOnOrdersCompleted (void);
		InterSystemResults FireOnPlayerEnteredSystem (CSpaceObject *pPlayer);
		InterSystemResults FireOnPlayerLeftSystem (CSpaceObject *pPlayer, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		void FireOnSystemExplosion (CSpaceObject *pExplosion, CSpaceObject *pSource, DWORD dwItemUNID);
		void FireOnSystemObjAttacked (SDamageCtx &Ctx);
		void FireOnSystemObjDestroyed (SDestroyCtx &Ctx);
		void FireOnSystemWeaponFire (CSpaceObject *pShot, CSpaceObject *pSource, DWORD dwItemUNID);
		bool FireOnTranslateMessage (const CString &sMessage, CString *retsMessage);
		inline void FreezeControls (void) { m_iControlsFrozen++; }
		void GetBoundingRect (CVector *retvUR, CVector *retvLL);
		inline CVector GetBoundsDiag (void) { return CVector(m_rBoundsX, m_rBoundsY); }
		inline int GetBoundsHalfAngle (Metric rDist) const { return Max((int)(180.0 * atan(GetBoundsRadius() / rDist) / g_Pi), 1); }
		inline Metric GetBoundsRadius (void) const { return Max(m_rBoundsX, m_rBoundsY); }
		CCommunicationsHandler *GetCommsHandler (void);
		int GetCommsMessageCount (void);
		inline const CString &GetData (const CString &sAttrib) const { return m_Data.GetData(sAttrib); }
		inline CString GetDataField (const CString &sField) { CString sValue; FindDataField(sField, &sValue); return sValue; }
		int GetDataInteger (const CString &sAttrib) const;
		inline int GetDestiny (void) const { return m_iDestiny; }
		inline Metric GetDetectionRange (int iPerception) { return RangeIndex2Range(GetDetectionRangeIndex(iPerception)); }
		Metric GetDetectionRange2 (int iPerception) const;
		int GetDetectionRangeIndex (int iPerception) const;
		CSovereign::Disposition GetDispositionTowards (CSpaceObject *pObj);
		Metric GetDistance (CSpaceObject *pObj) const { return (pObj->GetPos() - GetPos()).Length(); }
		Metric GetDistance2 (CSpaceObject *pObj) const { return (pObj->GetPos() - GetPos()).Length2(); }
		CDesignType *GetFirstDockScreen (CString *retsScreen);
		inline DWORD GetID (void) const { return m_dwID; }
		inline int GetIndex (void) const { return m_iIndex; }
		CSpaceObject *GetNearestEnemy (Metric rMaxRange = g_InfiniteDistance, bool bIncludeStations = false);
		CSpaceObject *GetNearestEnemyStation (Metric rMaxRange = g_InfiniteDistance);
		CSpaceObject *GetNearestStargate (bool bExcludeUncharted = false);

#define FLAG_INCLUDE_NON_AGGRESSORS		0x00000001
#define FLAG_INCLUDE_STATIONS			0x00000002
		int GetNearestVisibleEnemies (int iMaxEnemies, 
									  Metric rMaxDist, 
									  TArray<CSpaceObject *> *pretList, 
									  CSpaceObject *pExcludeObj = NULL,
									  DWORD dwFlags = 0);

		CSpaceObject *GetNearestVisibleEnemy (Metric rMaxRange = g_InfiniteDistance, bool bIncludeStations = false, CSpaceObject *pExcludeObj = NULL);
		CString GetNounPhrase (DWORD dwFlags);
		inline CSpaceObject *GetObjRefData (const CString &sAttrib) { return m_Data.GetObjRefData(sAttrib); }
		inline const CVector &GetOldPos (void) const { return m_vOldPos; }
		CSpaceObject *GetOrderGiver (DestructionTypes iCause = killedNone);
		inline CDesignType *GetOverride (void) { return m_pOverride; }
		inline CSpaceObject *GetPlayer (void) const { return (m_pSystem ? m_pSystem->GetPlayer() : NULL); }
		inline const CVector &GetPos (void) const { return m_vPos; }
		CSovereign *GetSovereignToDefend (void) const;
		const CString &GetStaticData (const CString &sAttrib);
		WORD GetSymbolColor (void);
		inline CSystem *GetSystem (void) const { return m_pSystem; }
		inline CUniverse *GetUniverse (void) const { return m_pSystem->GetUniverse(); }
		inline const CVector &GetVel (void) const { return m_vVel; }
		inline DWORD GetVersion (void) const { CDesignType *pType = GetType(); return (pType ? pType->GetVersion() : EXTENSION_VERSION); }
		inline bool HasInterSystemEvent (void) const { return (m_fHasInterSystemEvent ? true : false); }
		inline bool HasNonLinearMove (void) const { return (m_fNonLinearMove ? true : false); }
		inline bool HasOnAttackedEvent (void) const { return (m_fHasOnAttackedEvent ? true : false); }
		inline bool HasOnDamageEvent (void) const { return (m_fHasOnDamageEvent ? true : false); }
		inline bool HasOnObjDockedEvent (void) const { return (m_fHasOnObjDockedEvent ? true : false); }
		inline bool HasOnOrdersCompletedEvent (void) const { return (m_fHasOnOrdersCompletedEvent ? true : false); }
		inline bool HasRandomEncounters (void) const { int iFreq; return (GetRandomEncounterTable(&iFreq) || iFreq > 0); }
		inline void Highlight (WORD wColor) { m_wHighlightColor = wColor; m_iHighlightCountdown = 200; }
		bool InBarrier (const CVector &vPos);
		inline bool InBox (const CVector &vUR, const CVector &vLL) const 
			{ return (vUR.GetX() > m_vPos.GetX() - m_rBoundsX) 
					&& (vUR.GetY() > m_vPos.GetY() - m_rBoundsY)
					&& (vLL.GetX() < m_vPos.GetX() + m_rBoundsX)
					&& (vLL.GetY() < m_vPos.GetY() + m_rBoundsY); }
		inline bool InBoxPoint (const CVector &vUR, const CVector &vLL) const
			{ return (vUR.GetX() > m_vPos.GetX()) 
					&& (vUR.GetY() > m_vPos.GetY())
					&& (vLL.GetX() < m_vPos.GetX())
					&& (vLL.GetY() < m_vPos.GetY()); }
		bool IsAutoClearDestination (void) { return m_fAutoClearDestination; }
		bool IsAutomatedWeapon (void) const { return (m_fAutomatedWeapon ? true : false); }
		bool IsBarrier (void) const { return (m_fIsBarrier ? true : false); }
		bool IsCommsMessageValidFrom (CSpaceObject *pSender, int iIndex, CString *retsMsg = NULL, CString *retsKey = NULL);
		bool IsCovering (CSpaceObject *pObj);
		bool IsCreated (void) { return m_fOnCreateCalled; }
		bool IsDestinyTime (int iCycle, int iOffset = 0);
		bool IsDestroyed (void) { return (m_fDestroyed ? true : false); }
		static bool IsDestroyedInUpdate (void) { return m_bObjDestroyed; }
		bool IsEnemy (const CSpaceObject *pObj) const;
		bool IsEnemyInRange (Metric rMaxRange, bool bIncludeStations = false);
		bool IsEscortingFriendOf (const CSpaceObject *pObj) const;
		bool IsFriend (const CSpaceObject *pObj) const;
		bool IsInDamageCode (void) { return (m_fInDamage ? true : false); }
		bool IsLineOfFireClear (CInstalledDevice *pWeapon, CSpaceObject *pTarget, int iAngle, Metric rDistance = (30.0 * LIGHT_SECOND));
		inline bool IsMarked (void) const { return m_fMarked; }
		inline bool IsMobile (void) const { return !m_fCannotMove; }
		inline bool IsNamed (void) const { return m_fHasName; }
		inline bool IsPaintNeeded (void) { return m_fPaintNeeded; }
		inline bool IsPlayerDestination (void) { return m_fPlayerDestination; }
		inline bool IsPlayerDocked (void) { return m_fPlayerDocked; }
		inline bool IsPlayerTarget (void) { return m_fPlayerTarget; }
		inline bool IsSelected (void) { return m_fSelected; }
		inline bool IsShowingDistanceAndBearing (void) { return m_fShowDistanceAndBearing; }
		bool IsStargateInRange (Metric rMaxRange);
		inline bool IsTimeStopped (void) { return m_fTimeStop; }
		bool IsUnderAttack (void);
		void Jump (const CVector &vPos);
		inline void LoadObjReferences (CSystem *pSystem) { m_Data.LoadObjReferences(pSystem); }
		void Move (const CSpaceObjectList &Barriers, Metric rSeconds);
		inline bool NotifyOthersWhenDestroyed (void) { return (m_fNoObjectDestructionNotify ? false : true); }
		void OnObjDestroyed (const SDestroyCtx &Ctx);
		inline void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) 
			{
			PaintDebugVector(Dest, x, y, Ctx);
			OnPaint(Dest, x, y, Ctx);
			ClearPaintNeeded();
			}
		inline void Place (const CVector &vPos, const CVector &vVel = NullVector) { m_vPos = vPos; m_vOldPos = vPos; m_vVel = vVel; }
		inline bool PosInBox (const CVector &vUR, const CVector &vLL) const
			{ return (vUR.GetX() > m_vPos.GetX()) 
					&& (vUR.GetY() > m_vPos.GetY())
					&& (vLL.GetX() < m_vPos.GetX())
					&& (vLL.GetY() < m_vPos.GetY()); }
		void Reconned (void);
		void Remove (DestructionTypes iCause, const CDamageSource &Attacker);
		void ReportEventError (const CString &sEvent, ICCItem *pError);
		inline void RestartTime (void) { m_fTimeStop = false; }
		inline void SetAutoClearDestination (void) { m_fAutoClearDestination = true; }
		inline void SetAutomatedWeapon (void) { m_fAutomatedWeapon = true; }
		inline void SetData (const CString &sAttrib, const CString &sData) { m_Data.SetData(sAttrib, sData); }
		inline void SetDataFromDataBlock (const CAttributeDataBlock &Block) { m_Data = Block; }
		inline void SetDataFromXML (CXMLElement *pData) { m_Data.SetFromXML(pData); }
		void SetDataInteger (const CString &sAttrib, int iValue);
		inline void SetHasOnAttackedEvent (bool bHasEvent) { m_fHasOnAttackedEvent = bHasEvent; }
		inline void SetHasOnDamageEvent (bool bHasEvent) { m_fHasOnDamageEvent = bHasEvent; }
		inline void SetHasInterSystemEvent (bool bHasEvent) { m_fHasInterSystemEvent = bHasEvent; }
		inline void SetHasOnObjDockedEvent (bool bHasEvent) { m_fHasOnObjDockedEvent = bHasEvent; }
		inline void SetHasOnOrdersCompletedEvent (bool bHasEvent) { m_fHasOnOrdersCompletedEvent = bHasEvent; }
		inline void SetMarked (bool bMarked = true) { m_fMarked = bMarked; }
		inline void SetNamed (bool bNamed = true) { m_fHasName = bNamed; }
		inline void SetObjRefData (const CString &sAttrib, CSpaceObject *pObj) { m_Data.SetObjRefData(sAttrib, pObj); }
		void SetOverride (CDesignType *pOverride);
		inline void SetPaintNeeded (void) { m_fPaintNeeded = true; }
		inline void SetPlayerDestination (void) { m_fPlayerDestination = true; }
		inline void SetPlayerDocked (void) { m_fPlayerDocked = true; }
		inline void SetPlayerTarget (void) { m_fPlayerTarget = true; }
		inline void SetPos (const CVector &vPos) { m_vPos = vPos; }
		inline bool SetPOVLRS (void)
			{
			if (m_fInPOVLRS)
				return false;

			m_fInPOVLRS = true;
			return true;
			}
		inline void SetSelection (void) { m_fSelected = true; }
		inline void SetShowDistanceAndBearing (void) { m_fShowDistanceAndBearing = true; }
		inline void SetVel (const CVector &vVel) { m_vVel = vVel; }
		inline void StopTime (void) { m_fTimeStop = true; }
		inline bool SupportsDocking (void) { return (GetDefaultDockScreen() != NULL && GetDockingPortCount() > 0); }
		inline void UnfreezeControls (void) { m_iControlsFrozen--; }
		void Update (void);
		inline void UpdateExtended (const CTimeSpan &ExtraTime) { OnUpdateExtended(ExtraTime); }
		void WriteToStream (IWriteStream *pStream);
		inline void WriteObjRefToStream (CSpaceObject *pObj, IWriteStream *pStream) { GetSystem()->WriteObjRefToStream(pObj, pStream, this); }

		bool MatchesCriteria (SCriteriaMatchCtx &Ctx, const Criteria &Crit);
		static void ParseCriteria (CSpaceObject *pSource, const CString &sCriteria, Criteria *retCriteria);
		static void SetCriteriaSource (Criteria &Crit, CSpaceObject *pSource);

#ifdef DEBUG_VECTOR
		void PaintDebugVector (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		inline void SetDebugVector (const CVector &vVector) { m_vDebugVector = vVector; }
#else
		inline void PaintDebugVector (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { }
		inline void SetDebugVector (const CVector &vVector) { }
#endif

		//	Item functions

		EnhanceItemStatus AddItemEnhancement (const CItem &itemToEnhance, CItemType *pEnhancement, int iLifetime, DWORD *retdwID);
		EnhanceItemStatus AddItemEnhancement (CItemListManipulator &ItemList, CItemType *pEnhancement, int iLifetime, DWORD *retdwID);
		void DamageItem (CInstalledDevice *pDevice);
		void DamageItem (CItemListManipulator &ItemList);
		void DisruptItem (CItemListManipulator &ItemList, DWORD dwDuration);
		EnhanceItemStatus EnhanceItem (CItemListManipulator &ItemList, const CItemEnhancement &Mods, DWORD *retdwID = NULL);
		CItem GetItemForDevice (CInstalledDevice *pDevice);
		inline CItemList &GetItemList (void) { return m_ItemList; }
		bool HasFuelItem (void);
		void RemoveItemEnhancement (const CItem &itemToEnhance, DWORD dwID, bool bExpiredOnly = false);
		void SetCursorAtArmor (CItemListManipulator &ItemList, CInstalledArmor *pArmor);
		void SetCursorAtDevice (CItemListManipulator &ItemList, CInstalledDevice *pDevice);
		void SetCursorAtRandomItem (CItemListManipulator &ItemList, const CItemCriteria &Crit);
		bool Translate (const CString &sID, CString *retsText);
		void UseItem (CItem &Item, CString *retsError = NULL);

		inline void InvalidateItemListAddRemove (void) { m_fItemEventsValid = false; }
		inline void InvalidateItemListState (void) { m_fItemEventsValid = false; }
		void ItemsModified (void);

		//	Virtuals to be overridden

		//	...for all objects
		virtual CBoundaryMarker *AsBoundaryMarker (void) { return NULL; }
		virtual CMissile *AsMissile (void) { return NULL; }
		virtual CShip *AsShip (void) { return NULL; }
		virtual CStation *AsStation (void) { return NULL; }
		virtual bool CanAttack (void) const { return false; }
		virtual bool CanBeDestroyed (void) { return true; }
		virtual bool CanBeHitBy (const DamageDesc &Damage) { return true; }
		virtual bool CanHit (CSpaceObject *pObj) { return true; }
		virtual bool CanMove (void) { return false; }
		virtual bool ClassCanAttack (void) { return false; }
		virtual CString DebugCrashInfo (void) { return NULL_STR; }
		virtual bool FindDataField (const CString &sField, CString *retsValue) { return false; }
		virtual AbilityStatus GetAbility (Abilities iAbility) { return ablUninstalled; }
		virtual Categories GetCategory (void) const { return catOther; }
		virtual DWORD GetClassUNID (void) { return 0; }
		virtual const CString &GetGlobalData (const CString &sAttribute) { return NULL_STR; }
		virtual const CObjectImageArray &GetImage (void);
		virtual int GetInteraction (void) { return 100; }
		virtual const COrbit *GetMapOrbit (void) { return NULL; }
		virtual Metric GetMass (void) { return 0.0; }
		virtual Metric GetMaxSpeed (void) { return (IsMobile() ? LIGHT_SPEED : 0.0); }
		virtual CString GetName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return LITERAL("unknown object"); }
		virtual CString GetObjClassName (void) { return CONSTLIT("unknown"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerStations; }
		virtual Metric GetParallaxDist (void) { return 0.0; }
		virtual ICCItem *GetProperty (const CString &sName);
		virtual ScaleTypes GetScale (void) const { return scaleFlotsam; }
		virtual CSovereign *GetSovereign (void) const { return NULL; }
		virtual CDesignType *GetType (void) const { return NULL; }
		virtual CDesignType *GetWreckType (void) const { return NULL; }
		virtual bool HasAttribute (const CString &sAttribute) const { return false; }
		virtual bool HasSpecialAttribute (const CString &sAttrib) const;
		virtual bool IsBackgroundObj (void) { return false; }
		virtual bool IsKnown (void) { return true; }
		virtual bool IsMarker (void) { return false; }
		virtual bool IsVirtual (void) const { return false; }
		virtual bool IsWreck (void) const { return false; }
		virtual void LoadImages (void) { }
		virtual void MarkImages (void) { }
		virtual void OnPlayerChangedShips (CSpaceObject *pOldShip) { }
		virtual void OnSystemCreated (void) { }
		virtual void OnSystemLoaded (void) { }
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual void PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos) { return false; }
		virtual bool PointInObject (SPointInObjectCtx &Ctx, const CVector &vObjPos, const CVector &vPointPos) { return PointInObject(vObjPos, vPointPos); }
		virtual void PointInObjectInit (SPointInObjectCtx &Ctx) { }
		virtual bool SetAbility (Abilities iAbility, AbilityModifications iModification, int iDuration, DWORD dwOptions) { return false; }
		virtual void SetGlobalData (const CString &sAttribute, const CString &sData) { }
		virtual void SetKnown (bool bKnown = true) { }
		virtual void SetName (const CString &sName, DWORD dwFlags = 0) { }
		virtual bool SetProperty (const CString &sName, ICCItem *pValue, CString *retsError);
		virtual void SetSovereign (CSovereign *pSovereign) { }

		//	...for active/intelligent objects (ships, stations, etc.)
		virtual void AddOverlay (CEnergyFieldType *pType, int iPosAngle, int iPosRadius, int iRotation, int iLifetime, DWORD *retdwID = NULL) { if (retdwID) *retdwID = 0; }
		virtual CurrencyValue ChargeMoney (DWORD dwEconomyUNID, CurrencyValue iValue) { return 0; }
		virtual void CreateRandomDockedShips (IShipGenerator *pGenerator) { }
		virtual CurrencyValue CreditMoney (DWORD dwEconomyUNID, CurrencyValue iValue) { return 0; }
		virtual void DeactivateShields (void) { }
		virtual void Decontaminate (void) { }
		virtual void DepleteShields (void) { }
		virtual void DisableDevice (CInstalledDevice *pDevice) { }
		virtual CInstalledArmor *FindArmor (const CItem &Item) { return NULL; }
		virtual CInstalledDevice *FindDevice (const CItem &Item) { return NULL; }
		virtual bool FindDeviceSlotDesc (const CItem &Item, SDeviceDesc *retDesc) { return false; }
		virtual CurrencyValue GetBalance (DWORD dwEconomyUNID) { return 0; }
		virtual int GetBuyPrice (const CItem &Item, int *retiBuyPrice = NULL) { return -1; }
		virtual Metric GetCargoSpaceLeft (void) { return 1000000.0; }
		virtual int GetCombatPower (void) { return 0; }
		virtual int GetCyberDefenseLevel (void) { return 1; }
		virtual DamageTypes GetDamageType (void) { return damageGeneric; }
		virtual CEconomyType *GetDefaultEconomy (void);
		virtual DWORD GetDefaultEconomyUNID (void) { return DEFAULT_ECONOMY_UNID; }
		virtual CInstalledDevice *GetDevice (int iDev) const { return NULL; }
		virtual int GetDeviceCount (void) const { return 0; }
		virtual CSpaceObject *GetDockedObj (void) { return NULL; }
		virtual int GetDockingPortCount (void) { return 0; }
		virtual CStationType *GetEncounterInfo (void) { return NULL; }
		virtual CSpaceObject *GetEscortPrincipal (void) const { return NULL; }
		virtual int GetLastFireTime (void) const { return 0; }
		virtual int GetLevel (void) const { return 1; }
		virtual int GetMaxPower (void) const { return 0; }
		virtual int GetOpenDockingPortCount (void) { return 0; }
		virtual CEnergyField *GetOverlay (DWORD dwID) const { return NULL; }
		virtual const CString &GetOverlayData (DWORD dwID, const CString &sAttrib) { return NULL_STR; }
		virtual void GetOverlayList (TArray<CEnergyField *> &List) { List.DeleteAll(); }
		virtual CVector GetOverlayPos (DWORD dwID) { return GetPos(); }
		virtual int GetOverlayRotation (DWORD dwID) { return -1; }
		virtual CEnergyFieldType *GetOverlayType (DWORD dwID) { return NULL; }
		virtual int GetPerception (void) { return perceptNormal; }
		virtual CSpaceObject *GetTarget (bool bNoAutoTarget = false) const { return NULL; }
		virtual int GetScore (void) { return 0; }
		virtual int GetSellPrice (const CItem &Item) { return 0; }
		virtual int GetShieldLevel (void) { return -1; }
		virtual COLORREF GetSpaceColor (void) { return 0; }
		virtual CString GetStargateID (void) const { return NULL_STR; }
		virtual int GetStealth (void) const { return stealthNormal; }
		virtual int GetVisibleDamage (void) { return 0; }
		virtual bool HasMapLabel (void) { return false; }
		virtual bool IsAngryAt (CSpaceObject *pObj) { return IsEnemy(pObj); }
		virtual bool IsBlind (void) { return false; }
		virtual bool IsDisarmed (void) { return false; }
		virtual bool IsIdentified (void) { return true; }
		virtual bool IsObjDocked (CSpaceObject *pObj) { return false; }
		virtual bool IsObjDockedOrDocking (CSpaceObject *pObj) { return false; }
		virtual bool IsOutOfFuel (void) { return false; }
		virtual bool IsParalyzed (void) { return false; }
		virtual bool IsPlayer (void) const { return false; }
		virtual bool IsRadioactive (void) { return false; }
		virtual bool IsTimeStopImmune (void) { return false; }
		virtual void MakeBlind (int iTickCount = -1) { }
		virtual void MakeParalyzed (int iTickCount = -1) { }
		virtual void MakeRadioactive (void) { }
		virtual void OnComponentChanged (ObjectComponentTypes iComponent) { }
		virtual bool OnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker) { return true; }
		virtual void OnDeviceStatus (CInstalledDevice *pDev, int iEvent) { }
		virtual void OnHitByDeviceDamage (void) { }
		virtual void OnHitByDeviceDisruptDamage (DWORD dwDuration) { }
		virtual void OnHitByRadioactiveDamage (SDamageCtx &Ctx) { }
		virtual void OnNewSystem (void) { }
		virtual void OnObjLeaveGate (CSpaceObject *pObj) { }
		virtual bool OnObjJumpPosAdj (CSpaceObject *pPos, CVector *iovPos) { return false; }
		virtual void OnPlayerObj (CSpaceObject *pPlayer) { }
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx) { }
		virtual void ProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program) { }
		virtual void RegisterObjectForEvents (CSpaceObject *pObj) { }
		virtual void RemoveOverlay (DWORD dwID) { }
		virtual void SendMessage (CSpaceObject *pSender, const CString &sMsg) { }
		virtual void SetEventFlags (void) { }
		virtual void SetIdentified (bool bIdentified = true) { }
		virtual void SetMapLabelPos (int x, int y) { }
		virtual void SetOverlayData (DWORD dwID, const CString &sAttribute, const CString &sData) { }
		virtual void SetOverlayPos (DWORD dwID, const CVector &vPos) { }
		virtual void SetOverlayRotation (DWORD dwID, int iRotation) { }
		virtual void UnregisterObjectForEvents (CSpaceObject *pObj) { }
		virtual void UpdateArmorItems (void) { }

		//	...for objects that can bounce
		virtual bool ImageInObject (const CVector &vObjPos, const CObjectImageArray &Image, int iTick, int iRotation, const CVector &vImagePos) { return PointInObject(vObjPos, vImagePos); }
		virtual bool ObjectInObject (const CVector &vObj1Pos, CSpaceObject *pObj2, const CVector &vObj2Pos) { return PointInObject(vObj1Pos, vObj2Pos); }
		virtual void OnBounce (CSpaceObject *pBarrierObj, const CVector &vPos) { }
		virtual void OnObjBounce (CSpaceObject *pObj, const CVector &vPos) { }

		//	...for objects with devices
		virtual bool AbsorbWeaponFire (CInstalledDevice *pWeapon) { return false; }
		virtual CString GetInstallationPhrase (const CItem &Item) const { return NULL_STR; }
		virtual void SetFireDelay (CInstalledDevice *pWeapon, int iDelay = -1) { }

		//	...for objects with docking ports
		virtual DWORD GetDefaultBkgnd (void) { return 0; }
		virtual CDesignType *GetDefaultDockScreen (CString *retsName = NULL) { return NULL; }
		virtual CVector GetNearestDockVector (CSpaceObject *pRequestingObj) { return CVector(g_InfiniteDistance, g_InfiniteDistance); }
		virtual CXMLElement *GetScreen (const CString &sName);
		virtual void OnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget) { }
		virtual void PlaceAtRandomDockPort (CSpaceObject *pObj) { }
		virtual bool RequestDock (CSpaceObject *pObj) { return false; }
		virtual void Undock (CSpaceObject *pObj) { }

		//	...for beams, missiles, etc.
		virtual void CreateReflection (const CVector &vPos, int iDirection) { }
		virtual void DetonateNow (CSpaceObject *pHit) { }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return GetNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return killedByDamage; }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return NULL; }
		virtual CSpaceObject *GetSource (void) { return NULL; }
		virtual CSpaceObject *GetSecondarySource (void) { return NULL; }

		//	...for ships
		virtual void Behavior (void) { }
		virtual void ConsumeFuel (int iFuel) { }
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader) { return false; }
		virtual CSpaceObject *GetBase (void) const { return NULL; }
		virtual int GetRotation (void) const { return 0; }
		virtual bool IsInactive (void) const { return IsSuspended(); }
		virtual bool IsSuspended (void) const { return false; }
		virtual void OnDocked (CSpaceObject *pObj) { }
		virtual void OnDockedObjChanged (CSpaceObject *pLocation) { }
		virtual void Refuel (int iFuel) { }
		virtual void Refuel (const CItem &Fuel) { }
		virtual void RepairDamage (int iHitPoints) { }
		virtual void Resume (void) { }
		virtual void Suspend (void) { }
		virtual void UpdateDockingManeuver(const CVector &vDest, const CVector &vDestVel, int iDestFacing) { }

		//	...for stations
		virtual void AddBuyOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj) { }
		virtual void AddSellOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj) { }
		virtual void AddSubordinate (CSpaceObject *pSubordinate) { }
		virtual IShipGenerator *GetRandomEncounterTable (int *retiFrequency = NULL) const { if (retiFrequency) *retiFrequency = 0; return NULL; }
		virtual bool IsAbandoned (void) const { return false; }
		virtual bool IsActiveStargate (void) const { return false; }
		virtual bool IsStargate (void) const { return false; }
		virtual bool RemoveSubordinate (CSpaceObject *pSubordinate) { return false; }
		virtual bool RequestGate (CSpaceObject *pObj);
		virtual void SetTradeDesc (CEconomyType *pCurrency, int iMaxCurrency, int iReplenishCurrency) { }
		virtual bool SupportsGating (void) { return false; }

		//	...for particle effects
		virtual void SetAttractor (CSpaceObject *pObj) { }

		//	...miscellaneous
		virtual void OnLosePOV (void) { }

	protected:

		//	Virtuals to be overridden
		virtual bool CanBlock (CSpaceObject *pObj) { return true; }
		virtual bool CanBlockShips (void) { return true; }
		virtual bool CanFireOn (CSpaceObject *pObj) { return true; }
		virtual void GateHook (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate) { Destroy(removedFromSystem, CDamageSource()); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx) { }
		virtual void ObjectEnteredGateHook (CSpaceObject *pObjEnteredGate) { }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2) { return resNoAnswer; }
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damageNoDamage; }
		virtual void OnDestroyed (SDestroyCtx &Ctx) { }
		virtual CSpaceObject *OnGetOrderGiver (void) { return this; }
		virtual void OnItemEnhanced (CItemListManipulator &ItemList) { }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds) { }
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate) { }
		virtual void OnObjJumped (CSpaceObject *pObj) { }
		virtual void OnObjReconned (CSpaceObject *pObj) { }
		virtual void OnUpdate (Metric rSecondsPerTick) { }
		virtual void OnUpdateExtended (const CTimeSpan &ExtraTime) { }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { }
		virtual void OnReadFromStream (SLoadCtx &Ctx) { }
		virtual void OnWriteToStream (IWriteStream *pStream) { }
		virtual bool OrientationChanged (void) { return false; }
		virtual void RevertOrientationChange (void) { }

		//	Helper functions
		void AddEffect (IEffectPainter *pPainter, int xOffset, int yOffset, int iTick = 0, int iRotation = 0);
		Metric CalculateItemMass (Metric *retrCargoMass);
		bool CanFireOnObjHelper (CSpaceObject *pObj);
		inline void ClearCannotBeHit (void) { m_fCannotBeHit = false; }
		inline void ClearInDamageCode (void) { m_fInDamage = false; }
		inline void ClearInUpdateCode (void) { m_pObjInUpdate = NULL; m_bObjDestroyed = false; }
		inline void DisableObjectDestructionNotify (void) { m_fNoObjectDestructionNotify = true; }
		inline const Metric &GetBounds (void) { return m_rBoundsX; }
		CSpaceObject *HitTest (const CVector &vStart, Metric rThreshold, const DamageDesc &Damage, CVector *retvHitPos, int *retiHitDir);
		bool ImagesIntersect (const CObjectImageArray &Image1, int iTick1, int iRotation1, const CVector &vPos1,
				const CObjectImageArray &Image2, int iTick2, int iRotation2, const CVector &vPos2);
		inline bool IsHighlighted (void) { return ((m_iHighlightCountdown != 0) || m_fSelected); }
		inline bool IsObjectDestructionHooked (void) { return (m_fHookObjectDestruction ? true : false); }
		inline void ItemEnhancementModified (CItemListManipulator &ItemList) { OnItemEnhanced(ItemList); }
		bool MissileCanHitObj (CSpaceObject *pObj, CSpaceObject *pSource, bool bCanDamageSource);
		void PaintEffects (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void PaintHighlight (CG16bitImage &Dest, const RECT &rcRect);
		inline void SetObjectDestructionHook (void) { m_fHookObjectDestruction = true; }
		inline void SetCannotBeHit (void) { m_fCannotBeHit = true; }
		inline void SetCannotMove (void) { m_fCannotMove = true; }
		inline void SetCanBounce (void) { m_fCanBounce = true; }
		inline void SetBounds (Metric rBounds) { m_rBoundsX = rBounds; m_rBoundsY = rBounds; }
		inline void SetBounds (const RECT &rcRect)
			{ 
			m_rBoundsX = g_KlicksPerPixel * (RectWidth(rcRect) / 2);
			m_rBoundsY = g_KlicksPerPixel * (RectHeight(rcRect) / 2);
			}
		inline void SetBounds (IEffectPainter *pPainter)
			{
			RECT rcRect;
			pPainter->GetBounds(&rcRect);
			SetBounds(rcRect);
			}
		inline void SetIsBarrier (void) { m_fIsBarrier = true; }
		inline void SetInDamageCode (void) { m_fInDamage = true; }
		inline void SetInUpdateCode (void) { m_pObjInUpdate = this; m_bObjDestroyed = false; }
		inline void SetNoFriendlyFire (void) { m_fNoFriendlyFire = true; }
		inline void SetNoFriendlyTarget (void) { m_fNoFriendlyTarget = true; }
		inline void SetNonLinearMove (bool bValue = true) { m_fNonLinearMove = bValue; }

	private:

		struct SEffectNode
			{
			IEffectPainter *pPainter;
			int xOffset;
			int yOffset;
			int iTick;
			int iRotation;

			SEffectNode *pNext;
			};

		CSpaceObject (void);

		inline void InitItemEvents (void) { m_ItemEvents.Init(this); m_fItemEventsValid = true; }

		CSystem *m_pSystem;						//	Current system
		int m_iIndex;							//	Index in system
		DWORD m_dwID;							//	Universal ID
		int m_iDestiny;							//	Random number 0..DestinyRange-1
		CVector m_vPos;							//	Position of object in system
		CVector m_vVel;							//	Velocity of object
		CVector m_vOldPos;						//	Position last tick
		Metric m_rBoundsX;						//	Object bounds
		Metric m_rBoundsY;						//	Object bounds

		WORD m_wHighlightColor;					//	Highlight color
		int m_iHighlightCountdown:16;			//	Frames left to highlight

		CItemList m_ItemList;					//	List of items
		CAttributeDataBlock m_Data;				//	Opaque data
		SEffectNode *m_pFirstEffect;			//	List of effects
		CItemEventDispatcher m_ItemEvents;		//	Item event dispatcher
		CDesignType *m_pOverride;				//	Override event handler

		int m_iControlsFrozen:8;				//	Object will not respond to controls
		int m_iSpare:24;

		DWORD m_fHookObjectDestruction:1;		//	Call the object when another object is destroyed
		DWORD m_fNoObjectDestructionNotify:1;	//	Do not call other objects when this one is destroyed
		DWORD m_fCannotBeHit:1;					//	TRUE if it cannot be hit by other objects
		DWORD m_fSelected:1;					//	TRUE if selected
		DWORD m_fInPOVLRS:1;					//	TRUE if object appears in POV's LRS
		DWORD m_fCanBounce:1;					//	TRUE if object can bounce off others
		DWORD m_fIsBarrier:1;					//	TRUE if objects bounce off this object
		DWORD m_fCannotMove:1;					//	TRUE if object cannot move
		
		DWORD m_fNoFriendlyFire:1;				//	TRUE if object cannot hit friendly objects
		DWORD m_fTimeStop:1;					//	TRUE if time has stopped for this object
		DWORD m_fPlayerTarget:1;				//	TRUE if this is a target for the player
		DWORD m_fAutomatedWeapon:1;				//	TRUE if this is a shot from automated weapon
		DWORD m_fHasOnObjDockedEvent:1;			//	TRUE if this object has an OnObjDocked event
		DWORD m_fOnCreateCalled:1;				//	TURE if OnCreate event has already been called
		DWORD m_fNoFriendlyTarget:1;			//	TRUE if object cannot be hit by friends
		DWORD m_fItemEventsValid:1;				//	TRUE if item event dispatcher is up to date
		
		DWORD m_fHasOnDamageEvent:1;			//	TRUE if object has OnDamage event
		DWORD m_fHasOnAttackedEvent:1;			//	TRUE if object has OnAttacked event
		DWORD m_fInDamage:1;					//	TRUE if object is inside Damage call
		DWORD m_fDestroyed:1;					//	TRUE if object is destroyed (but not yet deleted)
		DWORD m_fPlayerDestination:1;			//	TRUE if object is a destination for the player
		DWORD m_fShowDistanceAndBearing:1;		//	TRUE if we should show distance and bearing when a player destination
		DWORD m_fHasInterSystemEvent:1;			//	TRUE if object has OnPlayerEnteredSystem or OnPlayerLeftSystem event
		DWORD m_fAutoClearDestination:1;		//	TRUE if m_fPlayerDestination is cleared when object in SRS

		DWORD m_fHasOnOrdersCompletedEvent:1;	//	TRUE if object has OnOrdersCompleted event
		DWORD m_fPlayerDocked:1;				//	TRUE if player is docked with this object
		DWORD m_fPaintNeeded:1;					//	TRUE if object needs to be painted
		DWORD m_fNonLinearMove:1;				//	TRUE if object updates its position inside OnMove
		DWORD m_fHasName:1;						//	TRUE if object has been named (this is an optimization--it may have false positives)
		DWORD m_fMarked:1;						//	Temporary marker for processing lists (not persistent)

		DWORD m_fSpare:2;

#ifdef DEBUG_VECTOR
		CVector m_vDebugVector;			//	Draw a vector
#endif

		//	This is a global variable that is set when we update an object.
		//	We use it to detect when an object gets destroyed inside its
		//	own Update method.
		//
		//	Note: Obviously this only works if object updates take place
		//	on the same thread and if they are not re-entrant. (i.e., can't
		//	call Update on a object from inside the Update of a different
		//	object).

		static CSpaceObject *m_pObjInUpdate;
		static bool m_bObjDestroyed;

	friend CObjectClass<CSpaceObject>;
	};

//	Implementations ------------------------------------------------------------

#ifndef INCL_TSE_SFX
#include "TSESFXImpl.h"
#endif

#ifndef INCL_TSE_DEVICE_CLASSES
#include "TSEDeviceClassesImpl.h"
#endif

#ifndef INCL_TSE_SPACE_OBJECTS
#include "TSESpaceObjectsImpl.h"
#endif

#ifndef INCL_TSE_SHIP_CONTROLLERS
#include "TSEShipControllersImpl.h"
#endif

#ifndef INCL_TSE_ORDER_MODULES
#include "TSEOrderModuleImpl.h"
#endif

#ifndef INCL_TSE_TOPOLOGY_PROCESSORS
#include "TSETopologyProcessorsImpl.h"
#endif

//	The Universe ---------------------------------------------------------------

enum EInitFlags
	{
	flagNoResources			= 0x00000001,
	flagNoVersionCheck		= 0x00000002,
	flagNewGame				= 0x00000004,
	};

enum EStorageScopes
	{
	storeUnknown				= -1,

	storeDevice					= 0,
	storeServiceExtension		= 1,
	storeServiceUser			= 2,
	};

class CUniverse : public CObject
	{
	public:
		class IHost
			{
			public:
				virtual void ConsoleOutput (const CString &sLine) { }
				virtual void DebugOutput (const CString &sLine) { }
				virtual void GameOutput (const CString &sLine) { }
				virtual const CG16bitFont *GetFont (const CString &sFont) { return NULL; }
			};

		CUniverse (void);
		virtual ~CUniverse (void);

		ALERROR Init (const CString &sFilespec, CString *retsError, DWORD dwFlags = 0);
		ALERROR Init (CResourceDb &Resources, CString *retsError, DWORD dwFlags = 0);
		ALERROR InitAdventure (DWORD dwAdventureUNID, TArray<DWORD> *pExtensionList, CString *retsError, DWORD dwFlags = 0);
		ALERROR InitGame (CString *retsError);
		void StartGame (bool bNewGame);

		inline ALERROR AddDynamicType (SExtensionDesc *pExtension, DWORD dwUNID, const CString &sSource, bool bNewGame, CString *retsError) { return m_Design.AddDynamicType(pExtension, dwUNID, sSource, bNewGame, retsError); }
		inline void AddTimeDiscontinuity (const CTimeSpan &Duration) { m_Time.AddDiscontinuity(m_iTick++, Duration); }
		ALERROR AddStarSystem (CTopologyNode *pTopology, CSystem *pSystem);
		ALERROR CreateEmptyStarSystem (CSystem **retpSystem);
		inline DWORD CreateGlobalID (void) { return m_dwNextID++; }
		ALERROR CreateRandomItem (const CItemCriteria &Crit, 
								  const CString &sLevelFrequency,
								  CItem *retItem);
		ALERROR CreateStarSystem (const CString &sNodeID, CSystem **retpSystem, CString *retsError = NULL, CSystemCreateStats *pStats = NULL);
		ALERROR CreateStarSystem (CTopologyNode *pTopology, CSystem **retpSystem, CString *retsError = NULL, CSystemCreateStats *pStats = NULL);
		void DestroySystem (CSystem *pSystem);
		CSpaceObject *FindObject (DWORD dwID);
		inline void FireOnGlobalObjDestroyed (SDestroyCtx &Ctx) { m_Design.FireOnGlobalObjDestroyed(Ctx); }
		inline void FireOnGlobalPaneInit (void *pScreen, CDesignType *pRoot, const CString &sScreen, const CString &sPane) { m_Design.FireOnGlobalPaneInit(pScreen, pRoot, sScreen, sPane); }
		inline void FireOnGlobalPlayerChangedShips (CSpaceObject *pOldShip) { m_Design.FireOnGlobalPlayerChangedShips(pOldShip); }
		inline void FireOnGlobalPlayerEnteredSystem (void) { m_Design.FireOnGlobalPlayerEnteredSystem(); }
		inline void FireOnGlobalPlayerLeftSystem (void) { m_Design.FireOnGlobalPlayerLeftSystem(); }
		inline void FireOnGlobalSystemCreated (SSystemCreateCtx &SysCreateCtx) { m_Design.FireOnGlobalSystemCreated(SysCreateCtx); }
		inline void FireOnGlobalUniverseCreated (void) { m_Design.FireOnGlobalUniverseCreated(); }
		inline void FireOnGlobalUniverseLoad (void) { m_Design.FireOnGlobalUniverseLoad(); }
		inline void FireOnGlobalUniverseSave (void) { m_Design.FireOnGlobalUniverseSave(); }
		void FlushStarSystem (CTopologyNode *pTopology);
		void GenerateGameStats (CGameStats &Stats);
		inline CAdventureDesc *GetCurrentAdventureDesc (void) { return m_pAdventure; }
		void GetCurrentAdventureExtensions (TArray<DWORD> *retList);
		CTimeSpan GetElapsedGameTime (void);
		CString GetExtensionData (EStorageScopes iScope, DWORD dwExtension, const CString &sAttrib);
		CTopologyNode *GetFirstTopologyNode (void);
		inline const CG16bitFont *GetFont (const CString &sFont) { return m_pHost->GetFont(sFont); }
		inline IHost *GetHost (void) const { return m_pHost; }
		void GetRandomLevelEncounter (int iLevel, CDesignType **retpType, IShipGenerator **retpTable, CSovereign **retpBaseSovereign);
		inline CString GetResourceDb (void) { return m_sResourceDb; }
		inline CSoundMgr *GetSoundMgr (void) { return m_pSoundMgr; }
		DWORD GetSoundUNID (int iChannel);
		inline bool InDebugMode (void) { return m_bDebugMode; }
		bool IsGlobalResurrectPending (CDesignType **retpType);
		inline bool IsRegistered (void) { return m_bRegistered; }
		bool IsStatsPostingEnabled (void);
		ALERROR LoadFromStream (IReadStream *pStream, DWORD *retdwSystemID, DWORD *retdwPlayerID, CString *retsError);
		inline bool NoImages (void) { return m_bNoImages; }
		void PlaySound (CSpaceObject *pSource, int iChannel);
		ALERROR Reinit (void);
		ALERROR SaveDeviceStorage (void);
		ALERROR SaveToStream (IWriteStream *pStream);
		void SetCurrentSystem (CSystem *pSystem);
		inline void SetDebugMode (bool bDebug = true) { m_bDebugMode = bDebug; }
		bool SetExtensionData (EStorageScopes iScope, DWORD dwExtension, const CString &sAttrib, const CString &sData);
		void SetHost (IHost *pHost);
		void SetPOV (CSpaceObject *pPOV);
		void SetPlayer (CSpaceObject *pPlayer);
		inline void SetRegistered (bool bRegistered = true) { m_bRegistered = bRegistered; }
		inline void SetSound (bool bSound = true) { m_bNoSound = !bSound; }
		inline void SetSoundMgr (CSoundMgr *pSoundMgr) { m_pSoundMgr = pSoundMgr; }
		void StartGameTime (void);
		CTimeSpan StopGameTime (void);
		static CString ValidatePlayerName (const CString &sName);

		inline CAdventureDesc *FindAdventureDesc (DWORD dwUNID) { return m_Design.FindAdventureDesc(dwUNID); }
		inline CAdventureDesc *FindAdventureForExtension (DWORD dwUNID) { return m_Design.FindAdventureForExtension(dwUNID); }
		inline CDesignType *FindDesignType (DWORD dwUNID) { return m_Design.FindEntry(dwUNID); }
		CArmorClass *FindArmor (DWORD dwUNID);
		CEffectCreator *FindDefaultHitEffect (DamageTypes iDamage);
		CDeviceClass *FindDeviceClass (DWORD dwUNID);
		inline CEffectCreator *FindEffectType (DWORD dwUNID) { return CEffectCreator::AsType(m_Design.FindEntry(dwUNID)); }
		inline CEconomyType *FindEconomyType (const CString &sName) { return m_Design.FindEconomyType(sName); }
		inline CShipTable *FindEncounterTable (DWORD dwUNID) { return CShipTable::AsType(m_Design.FindEntry(dwUNID)); }
		inline SExtensionDesc *FindExtensionDesc (DWORD dwUNID) { return m_Design.FindExtension(dwUNID); }
		inline CItemTable *FindItemTable (DWORD dwUNID) { return CItemTable::AsType(m_Design.FindEntry(dwUNID)); }
		inline CItemType *FindItemType (DWORD dwUNID) { return CItemType::AsType(m_Design.FindEntry(dwUNID)); }
		inline CPower *FindPower (DWORD dwUNID) { return CPower::AsType(m_Design.FindEntry(dwUNID)); }
		inline CDockScreenType *FindSharedDockScreen (DWORD dwUNID) { return CDockScreenType::AsType(m_Design.FindEntry(dwUNID)); }
		inline CShipClass *FindShipClass (DWORD dwUNID) { return CShipClass::AsType(m_Design.FindEntry(dwUNID)); }
		CShipClass *FindShipClassByName (const CString &sName);
		inline CEnergyFieldType *FindShipEnergyFieldType (DWORD dwUNID) { return CEnergyFieldType::AsType(m_Design.FindEntry(dwUNID)); }
		inline int FindSound (DWORD dwUNID) { return (int)FindByUNID(m_Sounds, dwUNID); }
		inline CSovereign *FindSovereign (DWORD dwUNID) const { return CSovereign::AsType(m_Design.FindEntry(dwUNID)); }
		inline CSpaceEnvironmentType *FindSpaceEnvironment (DWORD dwUNID) { return CSpaceEnvironmentType::AsType(m_Design.FindEntry(dwUNID)); }
		inline CStationType *FindStationType (DWORD dwUNID) { return CStationType::AsType(m_Design.FindEntry(dwUNID)); }
		inline CTopologyNode *FindTopologyNode (const CString &sName) { return m_Topology.FindTopologyNode(sName); }
		inline CXMLElement *FindSystemFragment (const CString &sName, CSystemTable **retpTable = NULL) { return m_Design.FindSystemFragment(sName, retpTable); }
		inline CSystemType *FindSystemType (DWORD dwUNID) { return CSystemType::AsType(m_Design.FindEntry(dwUNID)); }
		CWeaponFireDesc *FindWeaponFireDesc (const CString &sName);
		inline CCodeChain &GetCC (void) { return m_CC; }
		inline CTopologyNode *GetCurrentTopologyNode (void) { return (m_pCurrentSystem ? m_pCurrentSystem->GetTopology() : NULL); }
		inline CSystem *GetCurrentSystem (void) { return m_pCurrentSystem; }
		inline CSpaceObject *GetPOV (void) const { return m_pPOV; }
		inline CSpaceObject *GetPlayer (void) const { return m_pPlayer; }
		GenomeTypes GetPlayerGenome (void) const;
		CString GetPlayerName (void) const;
		CSovereign *GetPlayerSovereign (void) const;
		inline int GetTicks (void) { return m_iTick; }

		void GarbageCollectLibraryBitmaps (void);
		inline CObjectImage *FindLibraryImage (DWORD dwUNID) { return CObjectImage::AsType(m_Design.FindEntry(dwUNID)); }
		inline CG16bitImage *GetLibraryBitmap (DWORD dwUNID) { return m_Design.GetImage(dwUNID); }
		inline CG16bitImage *GetLibraryBitmapCopy (DWORD dwUNID) { return m_Design.GetImage(dwUNID, true); }
		void LoadLibraryBitmaps (void);
		inline void ReleaseLibraryBitmap (CG16bitImage *pBitmap) { }

		inline CAdventureDesc *GetAdventureDesc (int iIndex) { return (CAdventureDesc *)m_Design.GetEntry(designAdventureDesc, iIndex); }
		inline int GetAdventureDescCount (void) { return m_Design.GetCount(designAdventureDesc); }
		inline CDesignCollection &GetDesignCollection (void) { return m_Design; }
		inline CDesignType *GetDesignType (int iIndex) { return m_Design.GetEntry(iIndex); }
		inline int GetDesignTypeCount (void) { return m_Design.GetCount(); }
		inline const SExtensionDesc *GetExtensionDesc (int iIndex) { return m_Design.GetExtension(iIndex); }
		inline int GetExtensionDescCount (void) { return m_Design.GetExtensionCount(); }
		inline CItemType *GetItemType (int iIndex) { return (CItemType *)m_Design.GetEntry(designItemType, iIndex); }
		inline int GetItemTypeCount (void) { return m_Design.GetCount(designItemType); }
		inline CPower *GetPower (int iIndex) { return (CPower *)m_Design.GetEntry(designPower, iIndex); }
		inline int GetPowerCount (void) { return m_Design.GetCount(designPower); }
		inline CShipClass *GetShipClass (int iIndex) { return (CShipClass *)m_Design.GetEntry(designShipClass, iIndex); }
		inline int GetShipClassCount (void) { return m_Design.GetCount(designShipClass); }
		inline CSovereign *GetSovereign (int iIndex) const { return (CSovereign *)m_Design.GetEntry(designSovereign, iIndex); }
		inline int GetSovereignCount (void) { return m_Design.GetCount(designSovereign); }
		inline CStationType *GetStationType (int iIndex) { return (CStationType *)m_Design.GetEntry(designStationType, iIndex); }
		inline int GetStationTypeCount (void) { return m_Design.GetCount(designStationType); }
		inline CTopology &GetTopology (void) { return m_Topology; }
		inline CTopologyNode *GetTopologyNode (int iIndex) { return m_Topology.GetTopologyNode(iIndex); }
		inline int GetTopologyNodeCount (void) { return m_Topology.GetTopologyNodeCount(); }

		inline CG16bitFont &GetMapLabelFont (void) { return m_MapLabelFont; }
		inline CG16bitFont &GetSignFont (void) { return m_SignFont; }

		void PaintPOV (CG16bitImage &Dest, const RECT &rcView, bool bEnhanced);
		void PaintPOVLRS (CG16bitImage &Dest, const RECT &rcView, bool *retbNewEnemies);
		void PaintPOVMap (CG16bitImage &Dest, const RECT &rcView, Metric rMapScale);
		void PaintObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pObj);
		void PaintObjectMap (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pObj);
		void Update (Metric rSecondsPerTick, bool bForceEventFiring = false);
		void UpdateExtended (void);

		void DebugOutput (char *pszLine, ...);

	private:
		struct SLevelEncounter
			{
			CStationType *pType;
			int iWeight;
			CSovereign *pBaseSovereign;
			IShipGenerator *pTable;
			};

		struct STransSystemObject
			{
			CSpaceObject *pObj;
			CTopologyNode *pDest;
			int iArrivalTime;

			STransSystemObject *pNext;
			};

		ALERROR BindDesign (SDesignLoadCtx &Ctx);
		CObject *FindByUNID (CIDTable &Table, DWORD dwUNID);
		IShipController *GetPlayerController (void) const;
		ALERROR InitCodeChainPrimitives (void);
		void InitDefaultHitEffects (void);
		ALERROR InitDeviceStorage (SDesignLoadCtx &Ctx);
		ALERROR InitExtensions (SDesignLoadCtx &Ctx, const CString &sFilespec);
		ALERROR InitExtensionsFolder (SDesignLoadCtx &Ctx, const CString &sPath);
		ALERROR InitImages (SDesignLoadCtx &Ctx, CXMLElement *pImages, CResourceDb &Resources);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx,
							 CXMLElement *pElement, 
							 CResourceDb &Resources);
		ALERROR InitLevelEncounterTables (void);
		ALERROR InitSounds (SDesignLoadCtx &Ctx, CXMLElement *pSounds, CResourceDb &Resources);
		ALERROR InitStarSystemTypes (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR InitTopology (CString *retsError);
		ALERROR LoadAdventure (SDesignLoadCtx &Ctx, CAdventureDesc *pAdventure);
		ALERROR LoadDesignElement (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR LoadEncounterTable (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR LoadExtension (SDesignLoadCtx &Ctx, CXMLElement *pExtension);
		ALERROR LoadGlobals (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR LoadModule (SDesignLoadCtx &Ctx, CXMLElement *pModule);
		ALERROR LoadModules (SDesignLoadCtx &Ctx, CXMLElement *pModules);
		ALERROR LoadSound (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		inline void SetCurrentAdventureDesc (CAdventureDesc *pAdventure) { m_pAdventure = pAdventure; }

		//	Design data

		CDesignCollection m_Design;				//	Design collection
		CDeviceStorage m_DeviceStorage;			//	Local cross-game storage

		CString m_sResourceDb;					//	Resource database

		//CImageLibrary m_BitmapLibrary;			//	Cached bitmaps
		CIDTable m_Sounds;						//	Array of sound channels (int)
		CObjectArray m_LevelEncounterTables;	//	Array of SLevelEncounter arrays

		//	Game instance data

		bool m_bRegistered;						//	If TRUE, this is a registered game
		int m_iTick;							//	Ticks since beginning of time
		CGameTimeKeeper m_Time;					//	Game time tracker
		CAdventureDesc *m_pAdventure;			//	Current adventure
		CSpaceObject *m_pPOV;					//	Point of view
		CSpaceObject *m_pPlayer;				//	Player ship
		CSystem *m_pCurrentSystem;				//	Current star system (used by code)
		CIDTable m_StarSystems;					//	Array of CSystem (indexed by ID)
		CTimeDate m_StartTime;					//	Time when we started the game
		DWORD m_dwNextID;						//	Next universal ID
		CTopology m_Topology;					//	Array of CTopologyNode

		//	Support structures

		IHost *m_pHost;
		CCodeChain m_CC;
		CSoundMgr *m_pSoundMgr;
		CG16bitFont m_MapLabelFont;
		CG16bitFont m_SignFont;

		//	Debugging structures

		bool m_bNoImages;
		bool m_bDebugMode;
		bool m_bNoSound;
	};

//	String-Constant Helpers

Abilities AbilityDecode (const CString &sString);
AbilityModifications AbilityModificationDecode (const CString &sString);
AbilityModificationOptions AbilityModificationOptionDecode (const CString &sString);
AbilityStatus AbilityStatusDecode (const CString &sString);

CString AbilityEncode (Abilities iValue);
CString AbilityModificationEncode (AbilityModifications iValue);
CString AbilityModificationOptionEncode (AbilityModificationOptions iValue);
CString AbilityStatusEncode (AbilityStatus iValue);

CString GetDamageResultsName (EDamageResults iResult);

DestructionTypes GetDestructionCause (const CString &sString);
CString GetDestructionName (DestructionTypes iCause);

CString GetGenomeID (GenomeTypes iGenome);
CString GetGenomeName (GenomeTypes iGenome);
GenomeTypes ParseGenomeID (const CString &sText);

MessageTypes GetMessageFromID (const CString &sID);
CString GetMessageID (MessageTypes iMessage);

CSpaceObject::InterSystemResults GetInterSystemResult (const CString &sString);

IShipController::OrderTypes GetOrderType (const CString &sString);
CString GetOrderName (IShipController::OrderTypes iOrder);
int OrderGetDataCount (IShipController::OrderTypes iOrder);
bool OrderHasTarget (IShipController::OrderTypes iOrder, bool *retbRequired = NULL);

EStorageScopes ParseStorageScopeID (const CString &sID);

//	Miscellaneous Helpers

void AppendReferenceString (CString *iosReference, const CString &sString);
int CalcEffectiveHP (int iLevel, int iHP, int *iHPbyDamageType);
ICCItem *CreateItemFromDamageEffects (CCodeChain &CC, SDamageCtx &Ctx);
int GetDamageTypeLevel (DamageTypes iDamage);
Metric *GetDestinyToBellCurveArray (void);
void LoadDamageEffectsFromItem (ICCItem *pItem, SDamageCtx &Ctx);
IShipController::ManeuverTypes CalcTurnManeuver (int iDesired, int iCurrent, int iRotationAngle);
CString ParseCriteriaParam (char **ioPos, bool bExpectColon = true, bool *retbBinaryParam = NULL);

//	CodeChain helper functions (CCUtil.cpp)

CString CreateDataFieldFromItemList (const TArray<CItem> &List);
CString CreateDataFromItem (CCodeChain &CC, ICCItem *pItem);
ICCItem *CreateDisposition (CCodeChain &CC, CSovereign::Disposition iDisp);
ICCItem *CreateListFromImage (CCodeChain &CC, const CObjectImageArray &Image, int iRotation = 0);
ICCItem *CreateListFromItem (CCodeChain &CC, const CItem &Item);
ICCItem *CreateListFromVector (CCodeChain &CC, const CVector &vVector);
CItem CreateItemFromList (CCodeChain &CC, ICCItem *pList);
CSpaceObject *CreateObjFromItem (CCodeChain &CC, ICCItem *pItem);
ICCItem *CreateResultFromDataField (CCodeChain &CC, const CString &sValue);
CShip *CreateShipObjFromItem (CCodeChain &CC, ICCItem *pArg);
CStation *CreateStationObjFromItem (CCodeChain &CC, ICCItem *pArg);
CVector CreateVectorFromList (CCodeChain &CC, ICCItem *pList);
void DefineGlobalItem (CCodeChain &CC, const CString &sVar, const CItem &Item);
void DefineGlobalSpaceObject (CCodeChain &CC, const CString &sVar, CSpaceObject *pObj);
void DefineGlobalVector (CCodeChain &CC, const CString &sVar, const CVector &vVector);
void DefineGlobalWeaponType (CCodeChain &CC, const CString &sVar, CItemType *pWeaponType);
CInstalledArmor *GetArmorSectionArg (CCodeChain &CC, ICCItem *pArg, CSpaceObject *pObj);
CDamageSource GetDamageSourceArg (CCodeChain &CC, ICCItem *pArg);
DamageTypes GetDamageTypeFromArg (CCodeChain &CC, ICCItem *pArg);
CInstalledDevice *GetDeviceFromItem (CCodeChain &CC, CSpaceObject *pObj, ICCItem *pArg);
CEconomyType *GetEconomyTypeFromItem (CCodeChain &CC, ICCItem *pItem);
CEconomyType *GetEconomyTypeFromString (const CString &sCurrency);
ALERROR GetEconomyUNIDOrDefault (CCodeChain &CC, ICCItem *pItem, DWORD *retdwUNID);
void GetImageDescFromList (CCodeChain &CC, ICCItem *pList, CG16bitImage **retpBitmap, RECT *retrcRect);
CItem GetItemArg (CCodeChain &CC, ICCItem *pArg);
CItemType *GetItemTypeFromArg (CCodeChain &CC, ICCItem *pArg);
bool GetPosOrObject (CEvalContext *pEvalCtx, ICCItem *pArg, CVector *retvPos, CSpaceObject **retpObj = NULL, int *retiLocID = NULL);
CWeaponFireDesc *GetWeaponFireDescArg (ICCItem *pArg);
ALERROR LoadCodeBlock (const CString &sCode, ICCItem **retpCode, CString *retsError = NULL);
ICCItem *StdErrorNoSystem (CCodeChain &CC);

//	Inlines

inline CSpaceObject *CInstalledDevice::GetTarget (CSpaceObject *pSource) const { return ((m_dwTargetID && pSource) ? pSource->GetSystem()->FindObject(m_dwTargetID) : NULL); }
inline void CInstalledDevice::SetTarget (CSpaceObject *pObj) { m_dwTargetID = (pObj ? pObj->GetID() : 0); }

inline bool CItem::IsDisrupted (void) const { return (m_pExtra ? (m_pExtra->m_dwDisruptedTime >= (DWORD)g_pUniverse->GetTicks()) : false); }

inline void CSpaceObjectGrid::AddObject (CSpaceObject *pObj) { GetList(pObj->GetPos()).FastAdd(pObj); }

inline int CalcHPDamageAdj (int iHP, int iDamageAdj)
	{ return (iDamageAdj == 0 ? -1 : (int)((iHP * 100.0 / iDamageAdj) + 0.5)); }

#endif