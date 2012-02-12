//	PNGUtil.cpp
//
//	PNG Utilities

#include "Alchemy.h"
#include "PNGUtil.h"

#include "png.h"

ALERROR PNGLoadFromFile (const CString &sFilename, 
						 DWORD dwFlags, 
						 HPALETTE hPalette, 
						 HBITMAP *rethBitmap,
						 HBITMAP *rethBitmask)

//	PNGLoadFromFile
//
//	Loads a PNG from a file

	{
	//	Open the file

	FILE *fp;
	if ((fp = fopen(sFilename.GetASCIIZPointer(), "rb")) == NULL)
		return ERR_FILEOPEN;

	//	Create the png_struct

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			NULL, NULL, NULL);
	if (png_ptr == NULL)
		goto Fail;

	//	Initialize memory for image information

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
		goto Fail;

	//	Set up error handler

	if (setjmp(png_jmpbuf(png_ptr)))
		goto Fail;

	//	Set up IO

	png_init_io(png_ptr, fp);

	//	If we have already read some of the signature

	png_set_sig_bytes(png_ptr, 0);

	//	Read the file

	png_read_png(png_ptr, 
			info_ptr, 
			PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING, 
			NULL);

	//	Done

	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	fclose(fp);

	return NOERROR;

Fail:

	if (png_ptr)
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);

	fclose(fp);
	return ERR_FAIL;
	}
