//	DirectDraw.cpp
//
//	Functions to help with DirectDraw

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

struct SDDERRText
	{
	HRESULT hr;
	char *pszText;
	};

static SDDERRText g_DDERRTable[] =
	{
		{	DDERR_GENERIC,				"DDERR_GENERIC"	},
		{	DDERR_IMPLICITLYCREATED,	"DDERR_IMPLICITLYCREATED" },
		{	DDERR_INCOMPATIBLEPRIMARY,	"DDERR_INCOMPATIBLEPRIMARY" },
		{	DDERR_INVALIDOBJECT,		"DDERR_INVALIDOBJECT" },
		{	DDERR_INVALIDPARAMS,		"DDERR_INVALIDPARAMS" },
		{	DDERR_NOEXCLUSIVEMODE,		"DDERR_NOEXCLUSIVEMODE" },
		{	DDERR_OUTOFMEMORY,			"DDERR_OUTOFMEMORY" },
		{	DDERR_UNSUPPORTED,			"DDERR_UNSUPPORTED" },
		{	DDERR_WRONGMODE,			"DDERR_WRONGMODE" },
		{	DD_OK,			"DD_OK" },
	};

void LogDDError (HRESULT hr);

void *SurfaceLock (LPDIRECTDRAWSURFACE7 pSurface, DDSURFACEDESC2 *retpDesc)

//	SurfaceLock
//
//	Locks the given surface and returns a pointer to it

	{
	HRESULT hr;
	DDSURFACEDESC2 desc;

	if (retpDesc == NULL)
		retpDesc = &desc;

	//	Lock the surface

	int iMaxCount = 10;
	while (iMaxCount-- > 0)
		{
		retpDesc->dwSize = sizeof(desc);
		hr = pSurface->Lock(NULL, retpDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);

		//	Return the pointer if successful

		if (hr == DD_OK)
			return (void *)retpDesc->lpSurface;

		//	Restore the surface if necessary

		else if (hr == DDERR_SURFACELOST)
			{
			hr = pSurface->Restore();
			::Sleep(500);

			if (FAILED(hr))
				kernelDebugLogMessage("Surface restore failed: %x", hr);
			}

		//	Fail with an error otherwise

		else
			{
			kernelDebugLogMessage("Lock failed: %x", hr);
			return NULL;
			}
		}

	//	If we've tried 10 times (5 seconds) and still nothing, then we give up

	kernelDebugLogMessage("Unable to lock surface");
	return NULL;
	}

void SurfaceUnlock (LPDIRECTDRAWSURFACE7 pSurface)

//	SurfaceUnlock
//
//	Unlocks the surface previously locked by SurfaceLock

	{
	pSurface->Unlock(NULL);
	}

void LogDDError (HRESULT hr)
	{
	SDDERRText *pEntry = g_DDERRTable;
	while (pEntry->hr != DD_OK)
		{
		if (pEntry->hr == hr)
			{
			kernelDebugLogMessage(pEntry->pszText);
			break;
			}
		pEntry++;
		}
	}
