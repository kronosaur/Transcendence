//	MemDataSource.c
//
//	This file contains decompression data source routines for
//	the case of reading JPEG data from a memory block
//
//	The code is based on jdatasrc.c which is included in the IJG
//	source code.

/*	this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

/*	Expanded data source object for stdio input */

typedef struct
	{
	struct jpeg_source_mgr pub;					/*	public fields */
	} my_source_mgr, *my_src_ptr;

METHODDEF(void) init_source (j_decompress_ptr cinfo)

/*	init_source

	Initialize data source
*/
	{
	}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF(boolean) fill_input_buffer (j_decompress_ptr cinfo)

/*	fill_input_buffer

	Since our buffer is the entire block, we should never get here.
*/
	{
	ERREXIT(cinfo, JERR_INPUT_EMPTY);
	return TRUE;
	}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF(void) skip_input_data (j_decompress_ptr cinfo, long num_bytes)
	{
	my_src_ptr src = (my_src_ptr)cinfo->src;

	/*	We better not need to skip more than we've got */

	if (num_bytes > (long)src->pub.bytes_in_buffer)
		{
		ERREXIT(cinfo, JERR_INPUT_EMPTY);
		return;
		}

    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}

/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void) term_source (j_decompress_ptr cinfo)
	{
	}


GLOBAL(void) jpeg_memory_src (j_decompress_ptr cinfo, char *pData, int iDataLen)

/*	jpeg_memory_src

	Prepare for input from a memory block.
*/

	{
	my_src_ptr src;

	/*	Allocate a block of memory to hold our manager data */

	if (cinfo->src == NULL)
		cinfo->src = (struct jpeg_source_mgr *) (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo,
				JPOOL_PERMANENT,
				SIZEOF(my_source_mgr));
	
	/*	Set up our methods */

	src = (my_src_ptr)cinfo->src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart;	/*	user default method */
	src->pub.term_source = term_source;

	/*	Set up the buffer */

	src->pub.bytes_in_buffer = iDataLen;
	src->pub.next_input_byte = pData;
	}
