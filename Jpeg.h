#ifndef SAVEJPEG_H
#define SAVEJPEG_H

class CJpeg
{
public:
	void write_JPEG_file(const char* filename, unsigned char* image_buffer,
					 int image_width, int image_height, int quality);
};

#endif