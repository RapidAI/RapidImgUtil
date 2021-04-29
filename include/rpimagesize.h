#pragma once
namespace RPIMG
{

	typedef enum
	{
		IMAGE_FILETYPE_UNKNOWN = 0,
		IMAGE_FILETYPE_GIF = 1,
		IMAGE_FILETYPE_JPEG,
		IMAGE_FILETYPE_PNG,
		IMAGE_FILETYPE_PSD,
		IMAGE_FILETYPE_BMP,
		IMAGE_FILETYPE_TIFF_II, /* intel */
		IMAGE_FILETYPE_TIFF_MM, /* motorola */
		IMAGE_FILETYPE_JPC,
		IMAGE_FILETYPE_JP2,
		IMAGE_FILETYPE_JPX,
		IMAGE_FILETYPE_JB2,
		IMAGE_FILETYPE_IFF,
		IMAGE_FILETYPE_WBMP,
		/* IMAGE_FILETYPE_JPEG2000 is a userland alias for IMAGE_FILETYPE_JPC */
		IMAGE_FILETYPE_ICO,
		IMAGE_FILETYPE_WEBP,
		/* WHEN EXTENDING: PLEASE ALSO REGISTER IN image.c:PHP_MINIT_FUNCTION(imagetypes) */
		IMAGE_FILETYPE_COUNT
	} image_filetype;
	/* }}} */


	typedef		struct gfxinfo {
		unsigned int width;
		unsigned int height;
		unsigned int bits;
		unsigned int channels;
	}GFXINFO;


	typedef struct
	{

		const char* data;
		long nOffset;
		long nLen;

	}RPMIMG_STREAM;


	unsigned int rpimg_stream_seek(RPMIMG_STREAM* stream,long nOffset,int From);
	unsigned int rpimg_stream_read(RPMIMG_STREAM* stream, void * buf, int nRead);
	unsigned char rpimg_stream_getc(RPMIMG_STREAM*  stream);
	int rpimg_stream_rewind(RPMIMG_STREAM* stream); // 0 success, -1 failed.
	int rpimg_stream_eof(RPMIMG_STREAM* stream); // 1 eof, 0 not eof


	image_filetype  getimagetype(const char* stream, int nLen, char* filetype);

	char* image_type_to_mime_type(int image_type);

	
	GFXINFO* rpimg_getimagesize_from_stream(RPMIMG_STREAM* stream);
	void rpimg_free_gfxinfo(GFXINFO* info);

};
