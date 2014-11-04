//	DefPrimitives.h
//
//	Default primitives

static PRIMITIVEPROCDEF g_DefPrimitives[] =
	{
		{	"<",				fnEquality,		FN_EQUALITY_LESSER,		"",		NULL,	0,	},
		{	"ls",				fnEquality,		FN_EQUALITY_LESSER,		
			"(ls a b) -> True if a < b",
			NULL,	0,	},

		{	"<=",				fnEquality,		FN_EQUALITY_LESSER_EQ,	"",		NULL,	0,	},
		{	"leq",				fnEquality,		FN_EQUALITY_LESSER_EQ,
			"(leq a b) -> True if a <= b",
			NULL,	0,	},

		{	">",				fnEquality,		FN_EQUALITY_GREATER,	"",		NULL,	0,	},
		{	"gr",				fnEquality,		FN_EQUALITY_GREATER,
			"(gr a b) -> True if a > b",
			NULL,	0,	},

		{	">=",				fnEquality,		FN_EQUALITY_GREATER_EQ,	"",		NULL,	0,	},
		{	"geq",				fnEquality,		FN_EQUALITY_GREATER_EQ,
			"(geq a b) -> True if a >= b",		NULL,	0,	},

		{	"add",				fnMathList,		FN_MATH_ADD,
			"(add x1 x2 ... xn) -> z",
			"v*",	0,	},

		{	"abs",				fnMath,			FN_MATH_ABSOLUTE,
			"(abs x) -> z",
			"i",	0,	},

		{	"and",				fnLogical,		FN_LOGICAL_AND,
			"(and exp1 exp2 ... expn) -> True/Nil",
			NULL,	0,	},

		{	"append",			fnAppend,		0,
			"(append a b [...]) -> lists are concatenated",
			"*",	0,	},

		{	"apply",			fnApply,		0,
			"(apply exp arg1 arg2 ... argn list) -> Result",
			NULL,	0,	},

		{	"atmAddEntry",		fnAtmTable,		FN_ATMTABLE_ADDENTRY,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"atmDeleteEntry",	fnAtmTable,		FN_ATMTABLE_DELETEENTRY,"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"atmList",			fnAtmTable,		FN_ATMTABLE_LIST,		"",		NULL,	0,	},
		{	"atmLookup",		fnAtmTable,		FN_ATMTABLE_LOOKUP,		"",		NULL,	0,	},
		{	"atmAtomTable",		fnAtmCreate,	0,						"",		NULL,	0,	},
		{	"block",			fnBlock,		FN_BLOCK_BLOCK,			"",		NULL,	0,	},
		{	"cat",				fnCat,			0,
			"(cat s1 s2 ... sn) -> string",		NULL,	0,	},

		{	"count",			fnCount,		0,
			"(count list) -> number of items",
			NULL,	0,	},

		{	"divide",			fnMathOld,			FN_MATH_DIVIDE,
			"(divide x y) -> z",		NULL,	0,	},

		{	"enum",				fnEnum,			0,
			"(enum list itemVar exp)",
			NULL,	0,	},

		{	"enumwhile",		fnEnum,			FN_ENUM_WHILE,
			"(enumwhile list condition itemVar exp)",
			NULL,	0,	},

		{	"errblock",			fnBlock,		FN_BLOCK_ERRBLOCK,		"",		NULL,	0,	},

		{	"error",			fnSpecial,		FN_ERROR,
			"(error msg) -> error",
			"s",	0,	},

		{	"eq",				fnEquality,		FN_EQUALITY_EQ,			"",		NULL,	0,	},
		{	"eval",				fnEval,			0,						"",		NULL,	0,	},
		{	"filter",			fnFilter,		0,
			"(filter list var boolean-exp) -> filtered list",
			"lqu",	0,	},

		{	"find",				fnFind,			FN_FIND,
			"(find source target ['ascending|'descending] [keyIndex]) -> position of target in source (0-based)",
			"vv*",	0,	},

		{	"fncHelp",			fnItemInfo,		FN_ITEMINFO_HELP,		"",		NULL,	0,	},

		{	"for",				fnForLoop,		0,
			"(for var from to exp)",
			NULL,	0,	},

		{	"help",				fnHelp,			0,
			"(help) -> all functions\n"
			"(help partial-name) -> all functions starting with name\n"
			"(help function-name) -> help on function",
			"*",	0,	},

		{	"if",				fnIf,			0,						"",		NULL,	0,	},
		{	"int",				fnItemInfo,		FN_ITEMINFO_ASINT,		"",		NULL,	0,	},
		{	"isatom",			fnItemInfo,		FN_ITEMINFO_ISATOM,		"",		NULL,	0,	},
		{	"iserror",			fnItemInfo,		FN_ITEMINFO_ISERROR,	"",		NULL,	0,	},
		{	"isint",			fnItemInfo,		FN_ITEMINFO_ISINT,		"",		NULL,	0,	},
		{	"isfunction",		fnItemInfo,		FN_ITEMINFO_ISFUNCTION,	"",		NULL,	0,	},
		{	"isprimitive",		fnItemInfo,		FN_ITEMINFO_ISPRIMITIVE,"",		NULL,	0,	},

		{	"@",				fnItem,			FN_ITEM,
			"(@ list index)",
			"vv",	0,	},

		{	"item",				fnItem,			FN_ITEM,
			"DEPRECATED: Alias of @",
			"vv",	0,	},

		{	"lambda",			fnLambda,		0,						"",		NULL,	0,	},
		{	"lookup",			fnFind,			FN_LOOKUP,
			"(lookup source target ['ascending|'descending] [keyIndex]) -> found entry",
			"vv*",	0,	},

		{	"loop",				fnLoop,			0,
			"(loop condition exp)",
			NULL,	0,	},

		{	"link",				fnLink,			0,						"",		"s",	0,	},
		{	"list",				fnList,			0,
			"(list i1 i2 ... in) -> list",
			NULL,	0,	},

		{	"lnkAppend",		fnLinkedListAppend,	0,	
			"(lnkAppend list item) -> list",
			"uv",	PPFLAG_SIDEEFFECTS,	},

		{	"lnkRemove",		fnLinkedList,	FN_LINKEDLIST_REMOVE,
			"(lnkRemove list index) -> list",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"lnkRemoveNil",		fnLinkedList,	FN_LINKEDLIST_REMOVE_NIL,
			"(lnkRemoveNil list) -> list",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"lnkReplace",		fnLinkedList,	FN_LINKEDLIST_REPLACE,
			"(lnkReplace list index item) -> list",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"map",				fnMap,			0,
			"(map list ['excludeNil|'original|'reduceMax|'reduceMin] var exp) -> list",
			"l*qu",	0,	},

		{	"match",			fnMatch,			0,
			"(match list var boolean-exp) -> first item that matches",
			"lqu",	0,	},

		{	"max",				fnMathList,			FN_MATH_MAX,
			"(max x1 x2 ... xn) -> z",
			"v*",	0,	},

		{	"min",				fnMathList,			FN_MATH_MIN,
			"(min x1 x2 ... xn) -> z",
			"v*",	0,	},

		{	"modulo",			fnMath,			FN_MATH_MODULUS,
			"(modulo ['degrees] x y) -> z",
			"*",	0,	},

		{	"multiply",			fnMathList,			FN_MATH_MULTIPLY,
			"(multiply x1 x2 ... xn) -> z",
			"v*",	0,	},

		{	"neq",				fnEquality,		FN_EQUALITY_NEQ,			"",		NULL,	0,	},

		{	"not",				fnLogical,		FN_LOGICAL_NOT,
			"(not exp) -> True/Nil",
			NULL,	0,	},

		{	"or",				fnLogical,		FN_LOGICAL_OR,
			"(or exp1 exp2 ... expn) -> True/Nil",
			NULL,	0,	},

		{	"power",			fnMathFractions,		FN_MATH_POWER,
			"(power x y) -> z",
			"vv",	0,	},

		{	"regex",			fnRegEx,			0,
			"(regex source pattern ['offset|'subex]) -> result",
			"ss*",	0,	},

		{	"quote",			fnSpecial,			FN_QUOTE,
			"(quote exp) -> unevaluated exp",
			"u",	0,	},

		{	"random",			fnRandom,		FN_RANDOM,
			"(random from to)\n(random list)",
			"*",	0,	},

		{	"randomGaussian",		fnRandom,		FN_RANDOM_GAUSSIAN,
			"(randomGaussian low mid high) -> random number between low and high",
			"iii",	0,	},

		{	"randomTable",		fnRandomTable,		0,
			"(randomTable chance1 exp1 chance2 exp2 ... chancen expn) -> exp",
			NULL,	0,	},

		{	"seededRandom",		fnSeededRandom,		0,
			"(seededRandom seed from to)\n(seededRandom seed list)",
			"i*",	0,	},

		{	"set",				fnSet,			FN_SET_SET,				"",		NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"set@",				fnItem,			FN_SET_ITEM,
			"(set@ list-var index value) -> list\n"
			"(set@ struct-var key value) -> struct\n"
			"(set@ struct-var struct) -> merged structs",
			"uv*",	PPFLAG_SIDEEFFECTS,	},

		{	"setItem",			fnItem,			FN_SET_ITEM,
			"DEPRECATED: Alias of set@",
			"uvv",	PPFLAG_SIDEEFFECTS,	},

		{	"setq",				fnSet,			FN_SET_SETQ,			"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"shuffle",			fnShuffle,		0,
			"(shuffle list) -> shuffled list",
			"l",	0,	},

		{	"sort",				fnSort,			FN_SORT,
			"(sort list ['ascending|'descending] [keyIndex]) -> sorted list",
			"v*",	0,	},

		{	"split",			fnSplit,		0,
			"(split string [characters]) -> list",
			"s*",	0,	},

		{	"sqrt",				fnMath,			FN_MATH_SQRT,
			"(sqrt x) -> z",
			"v",	0,	},

		{	"strCapitalize",	fnStrCapitalize,0,
			"(strCapitalize string) -> string",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"strFind",			fnStrFind,		0,
			"(strFind string target) -> pos of target in string (0-based)",
			"ss",	0,	},

		{	"subset",			fnSubset,		0,
			"(subset list pos [count]) -> list",
			"vv*",	0,	},

		{	"subst",			fnSubst,		0,
			"(subst string arg1 arg2 ... argn) -> string",
			"s*",	PPFLAG_SIDEEFFECTS,	},

		{	"subtract",			fnMathOld,			FN_MATH_SUBTRACT,
			"(subtract x y) -> z",
			NULL,	0,	},

		{	"switch",			fnSwitch,		0,						"",		NULL,	0,	},

		//{	"symDeleteEntry",	fnSymTable,		FN_SYMTABLE_DELETEENTRY,"",		NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"sysGlobals",		fnSysInfo,		FN_SYSINFO_GLOBALS,		"",		NULL,	0,	},
		{	"sysPoolUsage",		fnSysInfo,		FN_SYSINFO_POOLUSAGE,	"",		NULL,	0,	},
		{	"sysTicks",			fnSysInfo,		FN_SYSINFO_TICKS,		"",		NULL,	0,	},

		{	"typeof",			fnItem,			FN_ITEM_TYPE,
			"(typeof item) -> type\n\n"

			"type\n\n"

			"   error\n"
			"   function\n"
			"   int32\n"
			"   list\n"
			"   primitive\n"
			"   string\n"
			"   struct\n",

			"v",	0,	},

		{	"vecVector",		fnVecCreate,	0,						"",		NULL,	0,	},
		{	"vecSetElement",	fnVector,		FN_VECTOR_SETELEMENT,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
	};

#define DEFPRIMITIVES_COUNT		(sizeof(g_DefPrimitives) / sizeof(g_DefPrimitives[0]))