#ifndef INCL_NPPROTOCOL
#define INCL_NPPROTOCOL

const int NPRPC_VERSION						= 1;

#define	GAME_ACCEPT_NEW_PLAYERS		0x00000001
#define GAME_ALLOW_GUESTS			0x00000002

//	STRUCTURES
//
//	ASSET LIST STRUCT
//
//	[int] no of resources
//	[list] resources & units
//
//	UNIT CLASS STRUCT
//
//	[int] UNID
//	[str] name
//	[str] class name
//	[stats-struct]
//		[int] hull
//		[int] firepower
//		[int] firerate
//		[int] armor
//		[int] stealth
//		[int] scanners
//		[int] hyperdrive
//	[build-struct]
//		[int] build tech
//		[int] build time
//		[struct] build res
//	[list] list of traits

//	Client commands

const int CMD_REQUEST_CONNECT		= 0x00000001;	// RequestConnect
													// [int] RPC Version
													// -> RES_CONNECT

const int CMD_CREATE_UNIVERSE		= 0x00000002;	// CreateUniverse
													// [str] Overlord password
													// -> RES_CMD_SUCCESS

const int CMD_NEW_PLAYER			= 0x00000003;	// NewPlayer
													// [str] Emperor name
													// [str] Password
													// [str] Empire name
													// -> RES_CMD_SUCCESS

const int CMD_LOGIN					= 0x00000004;	// Login
													// [str] Emperor name (or "overlord" or "guest")
													// [str] Password
													// -> RES_LOGIN

const int CMD_VIEW					= 0x00000005;	// View
													// [int] UNID
													// -> RES_VIEW_SOVEREIGN
													// -> RES_VIEW_NULLPOINT
													// -> RES_VIEW_WORLD
													// -> RES_VIEW_FLEET

const int CMD_UNVIEW				= 0x00000006;	// Unview
													// [int] UNID
													// -> RES_CMD_SUCCESS

const int CMD_LIST_SOVEREIGNS		= 0x00000007;	// ListSovereigns
													// [int] Sovereign UNID
													// -> RES_UNID_LIST

const int CMD_LIST_NULLPOINTS		= 0x00000008;	// ListNullPoints
													// [int] Sovereign UNID
													// -> RES_UNID_LIST

const int CMD_LIST_FLEETS			= 0x00000009;	// ListFleets
													// [int] Sovereign UNID
													// -> RES_UNID_LIST

const int CMD_FLEET_DEST			= 0x0000000A;	// Set fleet destination
													// [int] Fleet UNID
													// [int] New destination
													// -> RES_CMD_SUCCESS

//	Server responses

const int RES_CMD_SUCCESS			= 0x00000000;	// Command succeeded
													// [int] CMD_???

const int RES_CMD_ERROR				= 0x00010000;	// Error in command structure

const int RES_CMD_FAIL				= 0x00020000;	// Command failed
													// [int] CMD_???
													// [str] message

const int RES_CONNECT				= 0x00030000;	// Connection accepted
													// [int] Universe Version
													// [int] Game UNID
													// [str] Name of game
													// [int] # Turns so far
													// [int] # Null points
													// [int] # players
													// [int] Flags
													//		GAME_???

const int RES_DISCONNECT			= 0x00040000;	// Disconnect

const int RES_LOGIN					= 0x00050000;	// Login accepted
													// [int] Sovereign UNID
													// [int] Stardate (hundreths)
													// [str] Empire name

const int RES_VIEW_SOVEREIGN		= 0x00060000;	// Sovereign data
													// [int] UNID
													// [str] Name
													// [int] Capital UNID (may be unknown)

const int RES_VIEW_NULLPOINT		= 0x00070000;	// NullPoint data
													// [int] UNID
													// [int] X
													// [int] Y
													// [int] World UNID (may be none)
													// [list] Links
													// [list] list of fleets here

const int RES_VIEW_WORLD			= 0x00080000;	// World data
													// [int] UNID
													// [int] Sovereign UNID
													// [int] Location (NP UNID)
													// [int] population
													// [int] tech level
													// [int] efficiency
													// [list] list of traits

const int RES_VIEW_FLEET			= 0x00090000;	// Fleet data
													// [int] UNID
													// [int] Sovereign UNID
													// [int] Location (NP or Link UNID)
													// [int] Dest UNID
													// [int] Turns left in transit
													// [struct] asset list

const int RES_UNID_LIST				= 0x000A0000;	// UNID list
													// [int] CMD_???
													// [list] list of UNIDs

#endif