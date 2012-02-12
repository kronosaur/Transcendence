//	Debug.cpp
//
//	Debugging routines

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

void DebugOutputPixelFormat (DDPIXELFORMAT &format)

//	DebugOutputPixelFormat
//
//	Outputs the pixel format for the given surface

	{
	kernelDebugLogMessage("PIXEL FORMAT");
	if (format.dwFlags & DDPF_ALPHA) kernelDebugLogMessage("\tDDPF_ALPHA");
	if (format.dwFlags & DDPF_ALPHAPIXELS) kernelDebugLogMessage("\tDDPF_ALPHAPIXELS");
	if (format.dwFlags & DDPF_COMPRESSED) kernelDebugLogMessage("\tDDPF_COMPRESSED");
	if (format.dwFlags & DDPF_FOURCC) kernelDebugLogMessage("\tDDPF_FOURCC");
	if (format.dwFlags & DDPF_PALETTEINDEXED1) kernelDebugLogMessage("\tDDPF_PALETTEINDEXED1");
	if (format.dwFlags & DDPF_PALETTEINDEXED2) kernelDebugLogMessage("\tDDPF_PALETTEINDEXED2");
	if (format.dwFlags & DDPF_PALETTEINDEXED4) kernelDebugLogMessage("\tDDPF_PALETTEINDEXED4");
	if (format.dwFlags & DDPF_PALETTEINDEXED2) kernelDebugLogMessage("\tDDPF_PALETTEINDEXED2");
	if (format.dwFlags & DDPF_PALETTEINDEXEDTO8) kernelDebugLogMessage("\tDDPF_PALETTEINDEXEDTO8");
	if (format.dwFlags & DDPF_RGB) kernelDebugLogMessage("\tDDPF_RGB");
	if (format.dwFlags & DDPF_RGBTOYUV) kernelDebugLogMessage("\tDDPF_RGBTOYUV");
	if (format.dwFlags & DDPF_YUV) kernelDebugLogMessage("\tDDPF_YUV");
	if (format.dwFlags & DDPF_ZBUFFER) kernelDebugLogMessage("\tDDPF_ZBUFFER");
#ifdef DX6
	if (format.dwFlags & DDPF_ALPHAPREMULT) kernelDebugLogMessage("\tDDPF_ALPHAPREMULT");
	if (format.dwFlags & DDPF_BUMPLUMINANCE) kernelDebugLogMessage("\tDDPF_BUMPLUMINANCE");
	if (format.dwFlags & DDPF_BUMPDUDV) kernelDebugLogMessage("\tDDPF_BUMPDUDV");
	if (format.dwFlags & DDPF_LUMINANCE) kernelDebugLogMessage("\tDDPF_LUMINANCE");
	if (format.dwFlags & DDPF_STENCILBUFFER) kernelDebugLogMessage("\tDDPF_STENCILBUFFER");
	if (format.dwFlags & DDPF_ZPIXELS) kernelDebugLogMessage("\tDDPF_ZPIXELS");
#endif

	kernelDebugLogMessage("dwRGBBitCount: %d", format.dwRGBBitCount);
	kernelDebugLogMessage("dwRBitMask: %x", format.dwRBitMask);
	kernelDebugLogMessage("dwGBitMask: %x", format.dwGBitMask);
	kernelDebugLogMessage("dwBBitMask: %x", format.dwBBitMask);
	kernelDebugLogMessage("dwRGBAlphaBitMask: %x", format.dwRGBAlphaBitMask);
	}

void DebugOutputSurfaceDesc (DDSURFACEDESC2 &surfacedesc)

//	DebugOutputSurfaceDesc
//
//	Output surface characteristics including pixel format

	{
	kernelDebugLogMessage("SURFACE CHARACTERISTICS");
	kernelDebugLogMessage("dwHeight: %d", surfacedesc.dwHeight);
	kernelDebugLogMessage("dwWidth: %d", surfacedesc.dwWidth);
	kernelDebugLogMessage("lPitch: %d", surfacedesc.lPitch);
#ifdef DX6
	kernelDebugLogMessage("dwLinearSize: %d", surfacedesc.dwLinearSize);
#endif
	kernelDebugLogMessage("dwBackBufferCount: %d", surfacedesc.dwBackBufferCount);
	kernelDebugLogMessage("dwMipMapCount: %d", surfacedesc.dwMipMapCount);
	kernelDebugLogMessage("dwRefreshRate: %d", surfacedesc.dwRefreshRate);
	kernelDebugLogMessage("dwAlphaBitDepth: %d", surfacedesc.dwAlphaBitDepth);

	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_3DDEVICE) kernelDebugLogMessage("\tDDSCAPS_3DDEVICE");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_ALPHA) kernelDebugLogMessage("\tDDSCAPS_ALPHA");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER) kernelDebugLogMessage("\tDDSCAPS_BACKBUFFER");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_COMPLEX) kernelDebugLogMessage("\tDDSCAPS_COMPLEX");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_FLIP) kernelDebugLogMessage("\tDDSCAPS_FLIP");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER) kernelDebugLogMessage("\tDDSCAPS_FRONTBUFFER");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_MIPMAP) kernelDebugLogMessage("\tDDSCAPS_MIPMAP");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_MODEX) kernelDebugLogMessage("\tDDSCAPS_MODEX");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN) kernelDebugLogMessage("\tDDSCAPS_OFFSCREENPLAIN");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_OVERLAY) kernelDebugLogMessage("\tDDSCAPS_OVERLAY");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_OWNDC) kernelDebugLogMessage("\tDDSCAPS_OWNDC");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_PALETTE) kernelDebugLogMessage("\tDDSCAPS_PALETTE");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) kernelDebugLogMessage("\tDDSCAPS_PRIMARYSURFACE");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACELEFT) kernelDebugLogMessage("\tDDSCAPS_PRIMARYSURFACELEFT");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) kernelDebugLogMessage("\tDDSCAPS_SYSTEMMEMORY");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_TEXTURE) kernelDebugLogMessage("\tDDSCAPS_TEXTURE");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) kernelDebugLogMessage("\tDDSCAPS_VIDEOMEMORY");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_VISIBLE) kernelDebugLogMessage("\tDDSCAPS_VISIBLE");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_WRITEONLY) kernelDebugLogMessage("\tDDSCAPS_WRITEONLY");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_ZBUFFER) kernelDebugLogMessage("\tDDSCAPS_ZBUFFER");
#ifdef DX6
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM) kernelDebugLogMessage("\tDDSCAPS_LOCALVIDMEM");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) kernelDebugLogMessage("\tDDSCAPS_NONLOCALVIDMEM");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_STANDARDVGAMODE) kernelDebugLogMessage("\tDDSCAPS_STANDARDVGAMODE");
	if (surfacedesc.ddsCaps.dwCaps & DDSCAPS_VIDEOPORT) kernelDebugLogMessage("\tDDSCPAS_VIDEOPORT");
#endif

	DebugOutputPixelFormat(surfacedesc.ddpfPixelFormat);
	}
