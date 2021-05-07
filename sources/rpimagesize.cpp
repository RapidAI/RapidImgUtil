#include "precomp.h"
#include <stdio.h>


namespace RPIMG
{

// support 18 kinds of images.

/* file type markers */
 const char rpimg_sig_gif[3] = {'G', 'I', 'F'};
 const char rpimg_sig_psd[4] = {'8', 'B', 'P', 'S'};
 const char rpimg_sig_bmp[2] = {'B', 'M'};
 const char rpimg_sig_swf[3] = {'F', 'W', 'S'};
 const char rpimg_sig_swc[3] = {'C', 'W', 'S'};
 const char rpimg_sig_jpg[3] = {(char) 0xff, (char) 0xd8, (char) 0xff};
 const char rpimg_sig_png[8] = {(char) 0x89, (char) 0x50, (char) 0x4e, (char) 0x47,
                                    (char) 0x0d, (char) 0x0a, (char) 0x1a, (char) 0x0a};
 const char rpimg_sig_tif_ii[4] = {'I','I', (char)0x2A, (char)0x00};
 const char rpimg_sig_tif_mm[4] = {'M','M', (char)0x00, (char)0x2A};
 const char rpimg_sig_jpc[3]  = {(char)0xff, (char)0x4f, (char)0xff};
 const char rpimg_sig_jp2[12] = {(char)0x00, (char)0x00, (char)0x00, (char)0x0c,
                                     (char)0x6a, (char)0x50, (char)0x20, (char)0x20,
                                     (char)0x0d, (char)0x0a, (char)0x87, (char)0x0a};
 const char rpimg_sig_iff[4] = {'F','O','R','M'};
 const char rpimg_sig_ico[4] = {(char)0x00, (char)0x00, (char)0x01, (char)0x00};
 const char rpimg_sig_riff[4] = {'R', 'I', 'F', 'F'};
 const char rpimg_sig_webp[4] = {'W', 'E', 'B', 'P'};

/* REMEMBER TO ADD MIME-TYPE TO FUNCTION rpimg_image_type_to_mime_type */
/* PCX must check first 64bytes and byte 0=0x0a and byte2 < 0x06 */

/* return info as a struct, to make expansion easier */


GFXINFO * rpimg_handle_gif (RPMIMG_STREAM * stream)
{
	struct gfxinfo *result = NULL;
	char  dim[5];  // 5 bytes

	if (rpimg_stream_seek(stream, 3, SEEK_CUR))
		return NULL;

	if (rpimg_stream_read(stream, (char*)dim, sizeof(dim)) != sizeof(dim))
		return NULL;

	result = (GFXINFO*) malloc(sizeof(struct gfxinfo));
	result->width    = (unsigned short)dim[0]&0x00FF | (((unsigned short)dim[1]&0x00ff)<<8);
	result->height   = (unsigned short)dim[2]&0x00FF | (((unsigned short)dim[3]&0x00ff)<<8);
	result->bits     = dim[4]&0x80 ? ((((unsigned int)dim[4])&0x07) + 1) : 0;
	result->channels = 3; /* always */

	return result;
}
/* }}} */

/* {{{ rpimg_handle_psd */
GFXINFO* rpimg_handle_psd (RPMIMG_STREAM* stream)
{
	struct gfxinfo *result = NULL;
	char dim[8] = { 0 };

	if (rpimg_stream_seek(stream, 11, SEEK_CUR))
		return NULL;

	if (rpimg_stream_read(stream, (char*)dim, sizeof(dim)) != sizeof(dim))
		return NULL;

	result = (struct gfxinfo *)malloc( sizeof(struct gfxinfo));
	result->height   =  (((unsigned int)dim[0] & 0xff) << 24) + (((unsigned int)dim[1] & 0xff) << 16) + (((unsigned int)dim[2] & 0xff)  << 8) + ((unsigned int)dim[3] & 0xff);
	result->width    =  (((unsigned int)dim[4] & 0xff) << 24) + (((unsigned int)dim[5] & 0xff) << 16) + (((unsigned int)dim[6] & 0xff) << 8) + ((unsigned int)dim[7] & 0xff);

	return result;
}
/* }}} */

/* {{{ rpimg_handle_bmp */
GFXINFO* rpimg_handle_bmp (RPMIMG_STREAM* stream)
{
	struct gfxinfo *result = NULL;
	char  dim[16] = { 0 };
	int size;

	if (rpimg_stream_seek(stream, 11, SEEK_CUR))
		return NULL;

	if (rpimg_stream_read(stream, (char*)dim, sizeof(dim)) != sizeof(dim))
		return NULL;
	size   = (((unsigned int)dim[ 3] & 0xff) << 24) + (((unsigned int)dim[ 2] & 0xff) << 16) + (((unsigned int)dim[ 1] & 0xff) << 8) + ((unsigned int) dim[ 0] & 0xff);
	if (size == 12) {
		result = (struct gfxinfo *) malloc (sizeof(struct gfxinfo));
		result->width    =  (((unsigned int)dim[ 5] & 0xff) << 8) + ((unsigned int) dim[ 4] & 0xff);
		result->height   =  (((unsigned int)dim[ 7] & 0xff) << 8) + ((unsigned int) dim[ 6] & 0xff);
		result->bits     =  ((unsigned int)dim[11]);
	} else if (size > 12 && (size <= 64 || size == 108 || size == 124)) {
		result = (struct gfxinfo *) malloc (sizeof(struct gfxinfo));
		result->width    =  (((unsigned int)dim[ 7]&0xff) << 24) + (((unsigned int)dim[ 6] & 0xff) << 16) + (((unsigned int)dim[ 5] & 0xff) << 8) + ((unsigned int) dim[ 4] & 0xff);
		result->height   =  (((unsigned int)dim[11] & 0xff) << 24) + (((unsigned int)dim[10] & 0xff) << 16) + (((unsigned int)dim[ 9] & 0xff) << 8) + ((unsigned int) dim[ 8] & 0xff);
		result->height   =  abs((int32_t)result->height);
		result->bits     =  (((unsigned int)dim[15]) <<  8) +  ((unsigned int)dim[14]);
	} else {
		return NULL;
	}

	return result;
}

/* {{{ rpimg_handle_png
 * routine to handle PNG files */
GFXINFO* rpimg_handle_png (RPMIMG_STREAM* stream)
{
	struct gfxinfo *result = NULL;
	unsigned char dim[9] = { 0 };
/* Width:              4 bytes
 * Height:             4 bytes
 * Bit depth:          1 byte
 * Color type:         1 byte
 * Compression method: 1 byte
 * Filter method:      1 byte
 * Interlace method:   1 byte
 */

	if (rpimg_stream_seek(stream, 8, SEEK_CUR))
		return NULL;

	if((rpimg_stream_read(stream, (char*)dim, sizeof(dim))) < sizeof(dim))
		return NULL;

	result = (struct gfxinfo *) malloc(sizeof(struct gfxinfo));
	result->width  = (((unsigned int)dim[0]) << 24) + (((unsigned int)dim[1]) << 16) + (((unsigned int)dim[2]) << 8) + ((unsigned int)dim[3]);
	result->height = (((unsigned int)dim[4]) << 24) + (((unsigned int)dim[5]) << 16) + (((unsigned int)dim[6]) << 8) + ((unsigned int)dim[7]);
	result->bits   = (unsigned int)dim[8];
	return result;
}
/* }}} */

/* routines to handle JPEG data */

/* some defines for the different JPEG block types */
#define M_SOF0  0xC0			/* Start Of Frame N */
#define M_SOF1  0xC1			/* N indicates which compression process */
#define M_SOF2  0xC2			/* Only SOF0-SOF2 are now in common use */
#define M_SOF3  0xC3
#define M_SOF5  0xC5			/* NB: codes C4 and CC are NOT SOF markers */
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8
#define M_EOI   0xD9			/* End Of Image (end of datastream) */
#define M_SOS   0xDA			/* Start Of Scan (begins compressed data) */
#define M_APP0  0xe0
#define M_APP1  0xe1
#define M_APP2  0xe2
#define M_APP3  0xe3
#define M_APP4  0xe4
#define M_APP5  0xe5
#define M_APP6  0xe6
#define M_APP7  0xe7
#define M_APP8  0xe8
#define M_APP9  0xe9
#define M_APP10 0xea
#define M_APP11 0xeb
#define M_APP12 0xec
#define M_APP13 0xed
#define M_APP14 0xee
#define M_APP15 0xef
#define M_COM   0xFE            /* COMment                                  */

#define M_PSEUDO 0xFFD8			/* pseudo marker for start of image(byte 0) */

/* {{{ rpimg_read2 */
static unsigned short rpimg_read2(RPMIMG_STREAM * stream)
{
	char a[2] = { 0 };

	///* return 0 if we couldn't read enough data */
	if(rpimg_stream_read(stream, (char *) a, sizeof(a)) < sizeof(a)) return 0;

	return (((unsigned short)a[0]&0xff) << 8) + ((unsigned short)a[1] & 0xff);
}
/* }}} */

/* {{{ rpimg_next_marker
 * get next marker byte from file */
static unsigned int rpimg_next_marker(RPMIMG_STREAM* stream, int last_marker, int ff_read)
{
	int a=0, marker;

	/* get marker byte, swallowing possible padding                           */
	if (!ff_read) {
		size_t extraneous = 0;

		while ((marker = rpimg_stream_getc(stream)) != 0xff) {
			if (marker == EOF) {
				return M_EOI;/* we hit EOF */
	}
			extraneous++;
	}
		if (extraneous) {
			printf("Corrupt JPEG data: %zu extraneous bytes before marker", extraneous);
		}
	}
	a = 1;
	do {
		if ((marker = rpimg_stream_getc(stream)) == EOF)
		{
			return M_EOI;/* we hit EOF */
		}
		a++;
	} while (marker == 0xff);
	if (a < 2)
	{
		return M_EOI; /* at least one 0xff is needed before marker code */
	}
	return (unsigned int)marker;
}
/* }}} */

/* {{{ rpimg_skip_variable
 * skip over a variable-length block; assumes proper length marker */
static int rpimg_skip_variable(RPMIMG_STREAM* stream)
{
	int  length = ((unsigned int)rpimg_read2(stream));

	if (length < 2)	{
		return 0;
	}
	length = length - 2;
	rpimg_stream_seek(stream, (long )length, SEEK_CUR);
	return 1;
}
/* }}} */


/* {{{ rpimg_handle_jpeg
   main loop to parse JPEG structure */


unsigned short byteswap_ushort(unsigned short i)
{
	unsigned short j;
	j = (i << 8);
	j += (i >> 8);
	return j;
}
static struct gfxinfo *rpimg_handle_jpeg (RPMIMG_STREAM* stream)
{
	struct gfxinfo *result = NULL;
	
	char sigBuf[4] = { 0 };
	rpimg_stream_seek(stream, 0, SEEK_SET);
	char b = 0;
	rpimg_stream_seek(stream,2,SEEK_CUR);
	b = rpimg_stream_getc(stream);
	int w = -1;
	int h = -1;
	while (b && ((unsigned char)b & 0xff) != 0xDA) {
		while (((unsigned char)b & 0xff) != 0xFF)
		{
			b = rpimg_stream_getc(stream);
		}
		while (((unsigned char)b & 0xff) == 0xFF) {
			b = rpimg_stream_getc(stream);
		}
		if (((unsigned char)b & 0xff) >= 0xC0 && ((unsigned char)b & 0xff) <= 0xC3)
		{
			rpimg_stream_read(stream ,sigBuf, 3);
			rpimg_stream_read(stream, sigBuf, 4);
			unsigned short* size_info = (unsigned short*)(sigBuf);
			h = byteswap_ushort(size_info[0]);
			w = byteswap_ushort(size_info[1]);
			break;
		}
		else
		{
			unsigned short chunk_size = 0;
			rpimg_stream_read(stream, &chunk_size, 2);
			if (rpimg_stream_seek(stream, byteswap_ushort(chunk_size) - 2, SEEK_CUR) != 0)
				break;
		}
		
		rpimg_stream_read(stream, &b, 1);
	}
	if (w != -1 && h != -1)
	{

		result = (struct gfxinfo*)malloc(sizeof(struct gfxinfo));
		result->width = w;
		result->height = h;
	}

	if(result)
		result->channels = 0;

	
	return result; /* perhaps image broken -> no info but size */
}
/* }}} */

/* {{{ rpimg_read4 */
static unsigned int rpimg_read4(RPMIMG_STREAM* stream)
{
	unsigned char a[4];

	/* just return 0 if we hit the end-of-file */
	if ((rpimg_stream_read(stream, (char*)a, sizeof(a))) != sizeof(a)) return 0;

	return (((unsigned int)a[0]&0xff) << 24)
	     + (((unsigned int)a[1]&0xff) << 16)
	     + (((unsigned int)a[2]&0xff) <<  8)
	     + (((unsigned int)a[3]&0xff));
}
/* }}} */

/* {{{ JPEG 2000 Marker Codes */
#define JPEG2000_MARKER_PREFIX 0xFF /* All marker codes start with this */
#define JPEG2000_MARKER_SOC 0x4F /* Start of Codestream */
#define JPEG2000_MARKER_SOT 0x90 /* Start of Tile part */
#define JPEG2000_MARKER_SOD 0x93 /* Start of Data */
#define JPEG2000_MARKER_EOC 0xD9 /* End of Codestream */
#define JPEG2000_MARKER_SIZ 0x51 /* Image and tile size */
#define JPEG2000_MARKER_COD 0x52 /* Coding style default */
#define JPEG2000_MARKER_COC 0x53 /* Coding style component */
#define JPEG2000_MARKER_RGN 0x5E /* Region of interest */
#define JPEG2000_MARKER_QCD 0x5C /* Quantization default */
#define JPEG2000_MARKER_QCC 0x5D /* Quantization component */
#define JPEG2000_MARKER_POC 0x5F /* Progression order change */
#define JPEG2000_MARKER_TLM 0x55 /* Tile-part lengths */
#define JPEG2000_MARKER_PLM 0x57 /* Packet length, main header */
#define JPEG2000_MARKER_PLT 0x58 /* Packet length, tile-part header */
#define JPEG2000_MARKER_PPM 0x60 /* Packed packet headers, main header */
#define JPEG2000_MARKER_PPT 0x61 /* Packed packet headers, tile part header */
#define JPEG2000_MARKER_SOP 0x91 /* Start of packet */
#define JPEG2000_MARKER_EPH 0x92 /* End of packet header */
#define JPEG2000_MARKER_CRG 0x63 /* Component registration */
#define JPEG2000_MARKER_COM 0x64 /* Comment */
/* }}} */

/* {{{ rpimg_handle_jpc
   Main loop to parse JPEG2000 raw codestream structure */
static struct gfxinfo *rpimg_handle_jpc(RPMIMG_STREAM* stream)
{
	struct gfxinfo *result = NULL;
	int highest_bit_depth, bit_depth;
	unsigned char first_marker_id;
	unsigned int i;

	/* JPEG 2000 components can be vastly different from one another.
	   Each component can be sampled at a different resolution, use
	   a different colour space, have a separate colour depth, and
	   be compressed totally differently! This makes giving a single
	   "bit depth" answer somewhat problematic. For this implementation
	   we'll use the highest depth encountered. */

	/* Get the single byte that remains after the file type identification */
	first_marker_id = rpimg_stream_getc(stream);

	/* Ensure that this marker is SIZ (as is mandated by the standard) */
	if (first_marker_id != JPEG2000_MARKER_SIZ) {
		printf("JPEG2000 codestream corrupt(Expected SIZ marker not found after SOC)");
		return NULL;
	}
	
	result = (struct gfxinfo *)malloc(sizeof(struct gfxinfo));

	rpimg_read2(stream); /* Lsiz */
	rpimg_read2(stream); /* Rsiz */
	result->width = rpimg_read4(stream); /* Xsiz */
	result->height = rpimg_read4(stream); /* Ysiz */

#if MBO_0
	rpimg_read4(stream); /* XOsiz */
	rpimg_read4(stream); /* YOsiz */
	rpimg_read4(stream); /* XTsiz */
	rpimg_read4(stream); /* YTsiz */
	rpimg_read4(stream); /* XTOsiz */
	rpimg_read4(stream); /* YTOsiz */
#else
	if (rpimg_stream_seek(stream, 24, SEEK_CUR)) {
		free(result);
		return NULL;
	}
#endif

	result->channels = rpimg_read2(stream); /* Csiz */
	if ((result->channels == 0 && rpimg_stream_eof(stream)) || result->channels > 256) {
		free(result);
		return NULL;
	}

	/* Collect bit depth info */
	highest_bit_depth = 0;
	for (i = 0; i < result->channels; i++) {
		bit_depth = rpimg_stream_getc(stream); /* Ssiz[i] */
		bit_depth++;
		if (bit_depth > highest_bit_depth) {
			highest_bit_depth = bit_depth;
		}

		rpimg_stream_getc(stream); /* XRsiz[i] */
		rpimg_stream_getc(stream); /* YRsiz[i] */
	}

	result->bits = highest_bit_depth;

	return result;
}
/* }}} */

/* {{{ rpimg_handle_jp2
   main loop to parse JPEG 2000 JP2 wrapper format structure */
struct gfxinfo *rpimg_handle_jp2(RPMIMG_STREAM*  stream)
{
	struct gfxinfo *result = NULL;
	unsigned int box_length;
	unsigned int box_type;
	char jp2c_box_id[] = {(char)0x6a, (char)0x70, (char)0x32, (char)0x63};

	/* JP2 is a wrapper format for JPEG 2000. Data is contained within "boxes".
	   Boxes themselves can be contained within "super-boxes". Super-Boxes can
	   contain super-boxes which provides us with a hierarchical storage system.
	   It is valid for a JP2 file to contain multiple individual codestreams.
	   We'll just look for the first codestream at the root of the box structure
	   and handle that.
	*/

	for (;;)
	{
		box_length = rpimg_read4(stream); /* LBox */
		/* TBox */
		if (rpimg_stream_read(stream, (void *)&box_type, sizeof(box_type)) != sizeof(box_type)) {
			/* Use this as a general "out of stream" error */
			break;
		}

		if (box_length == 1) {
			/* We won't handle XLBoxes */
			return NULL;
		}

		if (!memcmp(&box_type, jp2c_box_id, 4))
		{
			/* Skip the first 3 bytes to emulate the file type examination */
			rpimg_stream_seek(stream, 3, SEEK_CUR);

			result = rpimg_handle_jpc(stream);
			break;
		}

		/* Stop if this was the last box */
		if ((int)box_length <= 0) {
			break;
		}

		/* Skip over LBox (Which includes both TBox and LBox itself */
		if (rpimg_stream_seek(stream, box_length - 8, SEEK_CUR)) {
			break;
		}
	}

	if (result == NULL) {
		printf("JP2 file has no codestreams at root level");
	}

	return result;
}
/* }}} */

/* {{{ tiff constants */
 const int rpimg_tiff_bytes_per_format[] = {0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

/* uncompressed only */
#define TAG_IMAGEWIDTH              0x0100
#define TAG_IMAGEHEIGHT             0x0101
/* compressed images only */
#define TAG_COMP_IMAGEWIDTH         0xA002
#define TAG_COMP_IMAGEHEIGHT        0xA003

#define TAG_FMT_BYTE       1
#define TAG_FMT_STRING     2
#define TAG_FMT_USHORT     3
#define TAG_FMT_ULONG      4
#define TAG_FMT_URATIONAL  5
#define TAG_FMT_SBYTE      6
#define TAG_FMT_UNDEFINED  7
#define TAG_FMT_SSHORT     8
#define TAG_FMT_SLONG      9
#define TAG_FMT_SRATIONAL 10
#define TAG_FMT_SINGLE    11
#define TAG_FMT_DOUBLE    12
/* }}} */

/* {{{ rpimg_ifd_get16u
 * Convert a 16 bit unsigned value from file's native byte order */
static int rpimg_ifd_get16u(void *Short, int motorola_intel)
{
	if (motorola_intel) {
		return (((unsigned char *)Short)[0] << 8) | ((unsigned char *)Short)[1];
	} else {
		return (((unsigned char *)Short)[1] << 8) | ((unsigned char *)Short)[0];
	}
}
/* }}} */

/* {{{ rpimg_ifd_get16s
 * Convert a 16 bit signed value from file's native byte order */
static signed short rpimg_ifd_get16s(void *Short, int motorola_intel)
{
	return (signed short)rpimg_ifd_get16u(Short, motorola_intel);
}
/* }}} */

/* {{{ rpimg_ifd_get32s
 * Convert a 32 bit signed value from file's native byte order */
static int rpimg_ifd_get32s(void *Long, int motorola_intel)
{
	if (motorola_intel) {
		return  ((( char *)Long)[0] << 24) | (((unsigned char *)Long)[1] << 16)
		      | (((unsigned char *)Long)[2] << 8 ) | (((unsigned char *)Long)[3] << 0 );
	} else {
		return  ((( char *)Long)[3] << 24) | (((unsigned char *)Long)[2] << 16)
		      | (((unsigned char *)Long)[1] << 8 ) | (((unsigned char *)Long)[0] << 0 );
	}
}
/* }}} */

/* {{{ rpimg_ifd_get32u
 * Convert a 32 bit unsigned value from file's native byte order */
static unsigned int rpimg_ifd_get32u(void *Long, int motorola_intel)
{
	return (unsigned)rpimg_ifd_get32s(Long, motorola_intel) & 0xffffffff;
}
/* }}} */

/* {{{ rpimg_handle_tiff
   main loop to parse TIFF structure */
static struct gfxinfo *rpimg_handle_tiff (RPMIMG_STREAM* stream, int motorola_intel)
{
	struct gfxinfo *result = NULL;
	int i, num_entries;
	unsigned char *dir_entry;
	size_t  ifd_size, dir_size, entry_value, width=0, height=0, ifd_addr;
	int entry_tag , entry_type;
	char *ifd_data, ifd_ptr[4],ifd_tmp[2];

	if (rpimg_stream_read(stream, ifd_ptr, 4) != 4)
		return NULL;
	ifd_addr = rpimg_ifd_get32u(ifd_ptr, motorola_intel);
	if (rpimg_stream_seek(stream, ifd_addr-8, SEEK_CUR))
		return NULL;
	ifd_size = 2;

	ifd_data = ifd_tmp;

	if (rpimg_stream_read(stream, ifd_tmp, 2) != 2) {
		return NULL;
	}


	num_entries = rpimg_ifd_get16u(ifd_data, motorola_intel);
	dir_size = 2/*num dir entries*/ +12/*length of entry*/*num_entries +4/* offset to next ifd (points to thumbnail or NULL)*/;
	ifd_size = dir_size;
	ifd_data =(char *)malloc(ifd_size); // need to malloc & copy data 
	memcpy(ifd_data, ifd_tmp, 2);


	if (rpimg_stream_read(stream, ifd_data+2, dir_size-2) != dir_size-2) {
		free(ifd_data);
		return NULL;
	}
	/* now we have the directory we can look how long it should be */
	ifd_size = dir_size;
	for(i=0;i<num_entries;i++) {
		dir_entry 	 = (unsigned char *) ifd_data+2+i*12;
		entry_tag    = rpimg_ifd_get16u(dir_entry+0, motorola_intel);
		entry_type   = rpimg_ifd_get16u(dir_entry+2, motorola_intel);
		switch(entry_type) {
			case TAG_FMT_BYTE:
			case TAG_FMT_SBYTE:
				entry_value  = (size_t)(dir_entry[8]);
				break;
			case TAG_FMT_USHORT:
				entry_value  = rpimg_ifd_get16u(dir_entry+8, motorola_intel);
				break;
			case TAG_FMT_SSHORT:
				entry_value  = rpimg_ifd_get16s(dir_entry+8, motorola_intel);
				break;
			case TAG_FMT_ULONG:
				entry_value  = rpimg_ifd_get32u(dir_entry+8, motorola_intel);
				break;
			case TAG_FMT_SLONG:
				entry_value  = rpimg_ifd_get32s(dir_entry+8, motorola_intel);
				break;
			default:
				continue;
		}
		switch(entry_tag) {
			case TAG_IMAGEWIDTH:
			case TAG_COMP_IMAGEWIDTH:
				width  = entry_value;
				break;
			case TAG_IMAGEHEIGHT:
			case TAG_COMP_IMAGEHEIGHT:
				height = entry_value;
				break;
		}
	}
	free(ifd_data);
	if ( width && height) {
		/* not the same when in for-loop */
		result = (struct gfxinfo *) malloc(sizeof(struct gfxinfo));
		result->height   = height;
		result->width    = width;
		result->bits     = 0;
		result->channels = 0;
		return result;
	}
	return NULL;
}
/* }}} */

/* {{{ rpimg_handle_psd */
static struct gfxinfo *rpimg_handle_iff(RPMIMG_STREAM * stream)
{
	struct gfxinfo * result;
	unsigned char a[10];
	int chunkId;
	int size;
	short width, height, bits;

	if (rpimg_stream_read(stream, (char *) a, 8) != 8) {
		return NULL;
	}
	if (strncmp((char *) a+4, "ILBM", 4) && strncmp((char *) a+4, "PBM ", 4)) {
		return NULL;
	}

	/* loop chunks to find BMHD chunk */
	do {
		if (rpimg_stream_read(stream, (char*)a, 8) != 8) {
			return NULL;
		}
		chunkId = rpimg_ifd_get32s(a+0, 1);
		size    = rpimg_ifd_get32s(a+4, 1);
		if (size < 0) {
			return NULL;
		}
		if ((size & 1) == 1) {
			size++;
		}
		if (chunkId == 0x424d4844) { /* BMHD chunk */
			if (size < 9 || rpimg_stream_read(stream, (char*)a, 9) != 9) {
				return NULL;
			}
			width  = rpimg_ifd_get16s(a+0, 1);
			height = rpimg_ifd_get16s(a+2, 1);
			bits   = a[8] & 0xff;
			if (width > 0 && height > 0 && bits > 0 && bits < 33) {
				result = (struct gfxinfo *) malloc(sizeof(struct gfxinfo));
				result->width    = width;
				result->height   = height;
				result->bits     = bits;
				result->channels = 0;
				return result;
			}
		} else {
			if (rpimg_stream_seek(stream, size, SEEK_CUR)) {
				return NULL;
			}
		}
	} while (1);
}
/* }}} */

/* {{{ rpimg_get_wbmp
 * int WBMP file format type
 * byte Header Type
 *	byte Extended Header
 *		byte Header Data (type 00 = multibyte)
 *		byte Header Data (type 11 = name/pairs)
 * int Number of columns
 * int Number of rows
 */
static int rpimg_get_wbmp(RPMIMG_STREAM *stream, struct gfxinfo **result, int check)
{
	int i, width = 0, height = 0;

	if (rpimg_stream_rewind(stream)) {
		return 0;
	}

	/* get type */
	if (rpimg_stream_getc(stream) != 0) {
		return 0;
	}

	/* skip header */
	do {
		i = rpimg_stream_getc(stream);
		if (i < 0) {
			return 0;
		}
	} while (i & 0x80);

	/* get width */
	do {
		i = rpimg_stream_getc(stream);
		if (i < 0) {
			return 0;
		}
		width = (width << 7) | (i & 0x7f);
        /* maximum valid width for wbmp (although 127 may be a more accurate one) */
        if (width > 2048) {
            return 0;
        }
	} while (i & 0x80);

	/* get height */
	do {
		i = rpimg_stream_getc(stream);
		if (i < 0) {
			return 0;
		}
		height = (height << 7) | (i & 0x7f);
        /* maximum valid height for wbmp (although 127 may be a more accurate one) */
        if (height > 2048) {
            return 0;
        }
	} while (i & 0x80);

	if (!height || !width) {
		return 0;
	}

	if (!check) {
		(*result)->width = width;
		(*result)->height = height;
	}

	return IMAGE_FILETYPE_WBMP;
}
/* }}} */

/* {{{ rpimg_handle_wbmp */
static struct gfxinfo *rpimg_handle_wbmp(RPMIMG_STREAM * stream)
{
	struct gfxinfo *result = (struct gfxinfo *) malloc(sizeof(struct gfxinfo));

	if (!rpimg_get_wbmp(stream, &result, 0)) {
		free(result);
		return NULL;
	}

	return result;
}
/* }}} */

/* {{{ rpimg_get_xbm */

/* {{{ rpimg_handle_ico */
static struct gfxinfo *rpimg_handle_ico(RPMIMG_STREAM* stream)
{
	struct gfxinfo *result = NULL;
	unsigned char dim[16];
	int num_icons = 0;

	if (rpimg_stream_read(stream, (char *) dim, 2) != 2)
		return NULL;

	num_icons = (((unsigned int)dim[1] & 0xff) << 8) + ((unsigned int) dim[0] & 0xff);

	if (num_icons < 1 || num_icons > 255)
		return NULL;

	result = (struct gfxinfo *) malloc( sizeof(struct gfxinfo));
	result->bits = 0;

	while (num_icons > 0)
	{
		if (rpimg_stream_read(stream, (char *) dim, sizeof(dim)) != sizeof(dim))
			break;

		if ((((unsigned int)dim[7]&0xff) <<  8) +  ((unsigned int)dim[6] & 0xff) >= result->bits)
		{
			result->width    =  (unsigned int)dim[0] & 0xff;
			result->height   =  (unsigned int)dim[1] & 0xff;
			if (result->height == 0 && result->width > 0)
				result->height = 256;

			result->bits     =  (((unsigned int)dim[7] & 0xff) <<  8) +  ((unsigned int)dim[6] & 0xff);
		}
		num_icons--;
	}

	return result;
}
/* }}} */

/* {{{ rpimg_handle_webp */
static struct gfxinfo *rpimg_handle_webp(RPMIMG_STREAM* stream)
{
	struct gfxinfo *result = NULL;
	const char sig[3] = {'V', 'P', '8'};
	unsigned char buf[18];
	char format;

	if (rpimg_stream_read(stream, (char *) buf, 18) != 18)
		return NULL;

	if (memcmp(buf, sig, 3)) {
		return NULL;
	}
	switch (buf[3]) {
		case ' ':
		case 'L':
		case 'X':
			format = buf[3];
			break;
		default:
			return NULL;
	}

	result = (struct gfxinfo *) malloc( sizeof(struct gfxinfo));

	switch (format) {
		case ' ':
			result->width = buf[14] + ((buf[15] & 0x3F) << 8);
			result->height = buf[16] + ((buf[17] & 0x3F) << 8);
			break;
		case 'L':
			result->width = buf[9] + ((buf[10] & 0x3F) << 8) + 1;
			result->height = (buf[10] >> 6) + (buf[11] << 2) + ((buf[12] & 0xF) << 10) + 1;
			break;
		case 'X':
			result->width = buf[12] + (buf[13] << 8) + (buf[14] << 16) + 1;
			result->height = buf[15] + (buf[16] << 8) + (buf[17] << 16) + 1;
			break;
	}
	result->bits = 8; /* always 1 byte */

	return result;
}
/* }}} */

/* {{{ rpimg_image_type_to_mime_type
 * Convert internal image_type to mime type */
 char * rpimg_image_type_to_mime_type(int image_type)
{
	switch( image_type) {
		case IMAGE_FILETYPE_GIF:
			return "image/gif";
		case IMAGE_FILETYPE_JPEG:
			return "image/jpeg";
		case IMAGE_FILETYPE_PNG:
			return "image/png";
		case IMAGE_FILETYPE_PSD:
			return "image/psd";
		case IMAGE_FILETYPE_BMP:
			return "image/bmp";
		case IMAGE_FILETYPE_TIFF_II:
		case IMAGE_FILETYPE_TIFF_MM:
			return "image/tiff";
		case IMAGE_FILETYPE_IFF:
			return "image/iff";
		case IMAGE_FILETYPE_WBMP:
			return "image/vnd.wap.wbmp";
		case IMAGE_FILETYPE_JPC:
			return "application/octet-stream";
		case IMAGE_FILETYPE_JP2:
			return "image/jp2";
		case IMAGE_FILETYPE_ICO:
			return "image/vnd.microsoft.icon";
		case IMAGE_FILETYPE_WEBP:
			return "image/webp";
		default:
		case IMAGE_FILETYPE_UNKNOWN:
			return "application/octet-stream"; /* suppose binary format */
	}
}
/* }}} */



/* {{{ Get file extension for image-type returned by getimagesize, exif_read_data, exif_thumbnail, exif_imagetype */

 // 0 success, otherwise failed.
int image_type_to_extension(image_filetype image_type,char * szExt,int nLen)
{
	char* imgext = NULL;
	switch (image_type) {
		case IMAGE_FILETYPE_GIF:
			imgext = "gif";
			break;
		case IMAGE_FILETYPE_JPEG:
			imgext = "jpeg";
			break;
		case IMAGE_FILETYPE_PNG:
			imgext = "png";
			break;
		case IMAGE_FILETYPE_PSD:
			imgext = "psd";
			break;
		case IMAGE_FILETYPE_BMP:
		case IMAGE_FILETYPE_WBMP:
			imgext = "bmp";
			break;
		case IMAGE_FILETYPE_TIFF_II:
		case IMAGE_FILETYPE_TIFF_MM:
			imgext = ".tiff";
			break;
		case IMAGE_FILETYPE_IFF:
			imgext = ".iff";
			break;
		case IMAGE_FILETYPE_JPC:
			imgext = ".jpc";
			break;
		case IMAGE_FILETYPE_JP2:
			imgext = ".jp2";
			break;
		case IMAGE_FILETYPE_JPX:
			imgext = ".jpx";
			break;
		case IMAGE_FILETYPE_JB2:
			imgext = ".jb2";
			break;
		case IMAGE_FILETYPE_ICO:
			imgext = ".ico";
			break;
		case IMAGE_FILETYPE_WEBP:
			imgext = ".webp";
			break;
	}

	if (imgext && strlen(imgext) < nLen)
	{
		strncpy(szExt, imgext, strlen(imgext));
		szExt[strlen(imgext)] = 0;
		return 0;
	}

	return -1;
}
/* }}} */

/* {{{ rpimg_imagetype
   detect filetype from first bytes */
image_filetype getimagetype(RPMIMG_STREAM * stream, char *filetype)
{
	char tmp[12];
    int twelve_bytes_read;

	if ( !filetype) 
		filetype = tmp;
	if((rpimg_stream_read(stream, filetype, 3)) != 3) {
		printf("Error reading from the file!");
		return IMAGE_FILETYPE_UNKNOWN;
	}

/* BYTES READ: 3 */
	if (!memcmp(filetype, rpimg_sig_gif, 3)) {
		return IMAGE_FILETYPE_GIF;
	} else if (!memcmp(filetype, rpimg_sig_jpg, 3)) {
		return IMAGE_FILETYPE_JPEG;
	} else if (!memcmp(filetype, rpimg_sig_png, 3)) {
		if (rpimg_stream_read(stream, filetype+3, 5) != 5) {
			printf("Error reading from file!");
			return IMAGE_FILETYPE_UNKNOWN;
		}
		if (!memcmp(filetype, rpimg_sig_png, 8)) {
			return IMAGE_FILETYPE_PNG;
		} else {
			printf("PNG file corrupted by ASCII conversion");
			return IMAGE_FILETYPE_UNKNOWN;
		}

	} else if (!memcmp(filetype, rpimg_sig_psd, 3)) {
		return IMAGE_FILETYPE_PSD;
	} else if (!memcmp(filetype, rpimg_sig_bmp, 2)) {
		return IMAGE_FILETYPE_BMP;
	} else if (!memcmp(filetype, rpimg_sig_jpc, 3)) {
		return IMAGE_FILETYPE_JPC;
	} else if (!memcmp(filetype, rpimg_sig_riff, 3)) {
		if (rpimg_stream_read(stream, filetype+3, 9) != 9) {
			printf("Error reading from file!");
			return IMAGE_FILETYPE_UNKNOWN;
		}
		if (!memcmp(filetype+8, rpimg_sig_webp, 4)) {
			return IMAGE_FILETYPE_WEBP;
		} else {
			return IMAGE_FILETYPE_UNKNOWN;
		}
	}

	if (rpimg_stream_read(stream, filetype+3, 1) != 1) {
		printf("Error reading !");
		return IMAGE_FILETYPE_UNKNOWN;
	}
/* BYTES READ: 4 */
	if (!memcmp(filetype, rpimg_sig_tif_ii, 4)) {
		return IMAGE_FILETYPE_TIFF_II;
	} else if (!memcmp(filetype, rpimg_sig_tif_mm, 4)) {
		return IMAGE_FILETYPE_TIFF_MM;
	} else if (!memcmp(filetype, rpimg_sig_iff, 4)) {
		return IMAGE_FILETYPE_IFF;
	} else if (!memcmp(filetype, rpimg_sig_ico, 4)) {
		return IMAGE_FILETYPE_ICO;
	}

    /* WBMP may be smaller than 12 bytes, so delay error */
	twelve_bytes_read = (rpimg_stream_read(stream, filetype+4, 8) == 8);

/* BYTES READ: 12 */
   	if (twelve_bytes_read && !memcmp(filetype, rpimg_sig_jp2, 12)) {
		return IMAGE_FILETYPE_JP2;
	}

/* AFTER ALL ABOVE FAILED */
	if (rpimg_get_wbmp(stream, NULL, 1)) {
		return IMAGE_FILETYPE_WBMP;
	}
    if (!twelve_bytes_read) {
		printf("Error reading from file!");
		return IMAGE_FILETYPE_UNKNOWN;
    }

	return IMAGE_FILETYPE_UNKNOWN;
}

void rpimg_free_gfxinfo(GFXINFO* info)
{

	free(info);
}
GFXINFO * rpimg_getimagesize_from_stream(RPMIMG_STREAM * stream)
{
	image_filetype itype = IMAGE_FILETYPE_UNKNOWN;
	struct gfxinfo *result = NULL;

	if (!stream) {
		return NULL;
	}

	itype = getimagetype(stream,NULL);
	switch( itype) {
		case IMAGE_FILETYPE_GIF:
			result = rpimg_handle_gif(stream);  //ok
			break;
		case IMAGE_FILETYPE_JPEG:
			{
				result = rpimg_handle_jpeg(stream); //ok
			}
			break;
		case IMAGE_FILETYPE_PNG: 
			result = rpimg_handle_png(stream); //ok
			break;

		case IMAGE_FILETYPE_PSD:
			result = rpimg_handle_psd(stream); //ok
			break;
		case IMAGE_FILETYPE_BMP:
			result = rpimg_handle_bmp(stream); //ok
			break;
		case IMAGE_FILETYPE_TIFF_II:
			result = rpimg_handle_tiff(stream, 0); //ok
			break;
		case IMAGE_FILETYPE_TIFF_MM:
			result = rpimg_handle_tiff(stream, 1); //ok
			break;
		case IMAGE_FILETYPE_JPC:
			result = rpimg_handle_jpc(stream);
			break;
		case IMAGE_FILETYPE_JP2: // ok
			result = rpimg_handle_jp2(stream);  
			break;
		case IMAGE_FILETYPE_IFF:
			result = rpimg_handle_iff(stream); //ok
			break;
		case IMAGE_FILETYPE_WBMP:   
			result = rpimg_handle_wbmp(stream);  //ok
			break;
		case IMAGE_FILETYPE_ICO:
			result = rpimg_handle_ico(stream); //ok
			break;
		case IMAGE_FILETYPE_WEBP:
			result = rpimg_handle_webp(stream); //ok
			break;
		default:
		case IMAGE_FILETYPE_UNKNOWN:
			break;
	}

	if (result) {
		return result;
	} else 
	{
		return NULL;
	}
}
/* }}} */



unsigned int rpimg_stream_seek(RPMIMG_STREAM* stream, long nOffset, int From)
{
	if (From == SEEK_CUR)
	{
		stream->nOffset += nOffset; //
	}
	else
	if (From == SEEK_SET)
		stream->nOffset = nOffset;
	else
		if (From == SEEK_END)
			stream->nOffset = stream->nLen - nOffset;
		else return -1;

	return 0;
}
unsigned int rpimg_stream_read(RPMIMG_STREAM* stream, void* buf, int nRead)
{
	if (stream->nOffset + nRead < stream->nLen)
	{
		memcpy(buf, stream->data + stream->nOffset, nRead);
		stream->nOffset += nRead;
		return nRead;
	}
	else
		return 0;
}
unsigned char rpimg_stream_getc(RPMIMG_STREAM* stream)
{
	unsigned char ch = *(stream->data+ stream->nOffset);
	stream->nOffset++;

	return ch;

}
int rpimg_stream_rewind(RPMIMG_STREAM* stream) // 0 success, -1 failed.
{
	if (!stream->data)
		return -1;
	stream->nOffset = 0;
	return 0;
}
int rpimg_stream_eof(RPMIMG_STREAM* stream) // 1 eof, 0 not eof
{


	if (stream->nOffset < stream->nLen)
		return 1;
	else
		return 0;
}

};
