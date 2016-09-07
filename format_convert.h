#ifndef PIC_FORMAT_CONVERT_H
#define PIC_FORMAT_CONVERT_H

enum rgb_format{
    RGB_FORMAT_16 = 2,//size of a pixel in byte
    RGB_FORMAT_24 = 3, 
    RGB_FORMAT_32 = 4,
};

enum angular_degree{//clockwise
    ANGULAR_DEG_90 = 1,
    ANGULAR_DEG_180 = 2,
    ANGULAR_DEG_270 = 3,
};

int nv12torgb(int width, int height, char *src, void *dst,int dstbuf_w,int rgb_format);

int rotateAngular(int width,int height,int angular,int rgb_format,uint8_t* inRGB,uint8_t* outRGB);

int RGB2BMP(int width,int height,uint8_t* rgbData,uint8_t* bmpData,int rgb_format);

int packageBMP(char *rgb_buffer,int nWidth,int nHeight,FILE*fp1);
#endif
