#include "stdafx.h"
extern "C"{
#include "jpeglib.h"
}
#include <setjmp.h>
#include <math.h>
#include "Jpeg.h"

void CJpeg::write_JPEG_file(const char* filename, unsigned char* image_buffer,
					 int image_width, int image_height, int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr		jerr;
	FILE* outfile;
	JSAMPROW row_pointer[1];
	int row_stride;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if((outfile = fopen(filename, "wb")) == NULL){
		return;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width;
	cinfo.image_height = image_height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);

	jpeg_start_compress(&cinfo, TRUE);
	
	row_stride = ((image_width * 3 + 3) >> 2) << 2; 
	while(cinfo.next_scanline < cinfo.image_height){
		row_pointer[0] = &image_buffer[(image_height - cinfo.next_scanline - 1) * row_stride];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
}