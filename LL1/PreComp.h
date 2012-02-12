//	PreComp.h
//
//	This header file should be included by all source files

#include <Windows.h>
#include <mmsystem.h>

#ifndef INCL_KERNEL
#include "Kernel.h"
#endif

#ifndef INCL_KERNELOBJID
#include "KernelObjID.h"
#endif

#ifndef INCL_GRAPHICS
#define NO_DIRECT_DRAW
#include "Graphics.h"
#endif

#ifndef INCL_CODECHAIN
#include "CodeChain.h"
#endif

#include "ALGUI.h"
#include "LL1.h"
#include "Resource.h"
