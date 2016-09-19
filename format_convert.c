#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "include/universal.h"
#include "format_convert.h"
#define clip(var) ((var>=255)?255:(var<=0)?0:var)

extern bool pic_gray;

typedef unsigned char  BYTE;
typedef unsigned short WORD;

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
}rgb888;

// BMP图像各部分说明如下
/***********
    第一部分    位图文件头
该结构的长度是固定的，为14个字节，各个域的依次如下：
    2byte   ：文件类型，必须是0x4d42，即字符串"BM"。
    4byte   ：整个文件大小
    4byte   ：保留字，为0
    4byte   ：从文件头到实际的位图图像数据的偏移字节数。
*************/
typedef struct
{    int imageSize;
    int blank;
    int startPosition;
}BmpHead;
/*********************
第二部分    位图信息头
该结构的长度也是固定的，为40个字节，各个域的依次说明如下：
    4byte   ：本结构的长度，值为40
    4byte   ：图像的宽度是多少象素。
    4byte   ：图像的高度是多少象素。
    2Byte   ：必须是1。
    2Byte   ：表示颜色时用到的位数，常用的值为1(黑白二色图)、4(16色图)、8(256色图)、24(真彩色图)。
    4byte   ：指定位图是否压缩，有效值为BI_RGB，BI_RLE8，BI_RLE4，BI_BITFIELDS。Windows位图可采用RLE4和RLE8的压缩格式，BI_RGB表示不压缩。
    4byte   ：指定实际的位图图像数据占用的字节数，可用以下的公式计算出来：
     图像数据 = Width' * Height * 表示每个象素颜色占用的byte数(即颜色位数/8,24bit图为3，256色为1）
     要注意的是：上述公式中的biWidth'必须是4的整数倍(不是biWidth，而是大于或等于biWidth的最小4的整数倍)。
     如果biCompression为BI_RGB，则该项可能为0。
    4byte   ：目标设备的水平分辨率。
    4byte   ：目标设备的垂直分辨率。
    4byte   ：本图像实际用到的颜色数，如果该值为0，则用到的颜色数为2的(颜色位数)次幂,如颜色位数为8，2^8=256,即256色的位图
    4byte   ：指定本图像中重要的颜色数，如果该值为0，则认为所有的颜色都是重要的。
***********************************/
typedef struct
  
{
    int    Length;
    int    width;
    int    height;
    WORD    colorPlane;
    WORD    bitColor;
    int    zipFormat;
    int    realSize;
    int    xPels;
    int    yPels;
    int    colorUse;
    int    colorImportant;
  /*  void show()
  
    {    
        printf("infoHead Length:%dn",Length);
        printf("width&height:%d*%dn",width,height);
        printf("colorPlane:%dn",colorPlane);
        printf("bitColor:%dn",bitColor);
        printf("Compression Format:%dn",zipFormat);
        printf("Image Real Size:%dn",realSize);
        printf("Pels(X,Y):(%d,%d)n",xPels,yPels);
        printf("colorUse:%dn",colorUse);    
        printf("Important Color:%dn",colorImportant);
    }*/
}InfoHead;
/***************************
    第三部分    调色盘结构  颜色表
    对于256色BMP位图，颜色位数为8，需要2^8 = 256个调色盘；
    对于24bitBMP位图，各象素RGB值直接保存在图像数据区，不需要调色盘，不存在调色盘区
    rgbBlue：   该颜色的蓝色分量。
    rgbGreen：  该颜色的绿色分量。
    rgbRed：    该颜色的红色分量。
    rgbReserved：保留值。
************************/
typedef struct
{         BYTE   rgbBlue;
         BYTE   rgbGreen;
         BYTE   rgbRed;
         BYTE   rgbReserved;
      /*   void show(void)
         {
            printf("Mix Plate B,G,R:%d %d %dn",rgbBlue,rgbGreen,rgbRed);
         }*/
}RGBMixPlate;

int packageBMP(char *rgb_buffer,int nWidth,int nHeight,FILE*fp1)
{
     BmpHead m_BMPHeader;       
     char bfType[2]={'B','M'};
     m_BMPHeader.imageSize=4*nWidth*nHeight+54;
     m_BMPHeader.blank=0;
     m_BMPHeader.startPosition=54;
     printf("size of header %ld,long:%ld,word:%ld\n",sizeof(m_BMPHeader),sizeof(long),sizeof(WORD));
  
     fwrite(bfType,1,sizeof(bfType),fp1);
     fwrite(&m_BMPHeader.imageSize,1,sizeof(m_BMPHeader.imageSize),fp1);
     fwrite(&m_BMPHeader.blank,1,sizeof(m_BMPHeader.blank),fp1);
     fwrite(&m_BMPHeader.startPosition,1,sizeof(m_BMPHeader.startPosition),fp1);
         
     InfoHead  m_BMPInfoHeader;
     m_BMPInfoHeader.Length=40;
     m_BMPInfoHeader.width=nWidth;
     m_BMPInfoHeader.height=nHeight;
     m_BMPInfoHeader.colorPlane=1;
     m_BMPInfoHeader.bitColor=32;
     m_BMPInfoHeader.zipFormat=0;
     m_BMPInfoHeader.realSize=4*nWidth*nHeight;
     m_BMPInfoHeader.xPels=0;
     m_BMPInfoHeader.yPels=0;
     m_BMPInfoHeader.colorUse=0;
     m_BMPInfoHeader.colorImportant=0;
     printf("size of info %ld\n",sizeof(m_BMPInfoHeader));
  
     fwrite(&m_BMPInfoHeader.Length,1,sizeof(m_BMPInfoHeader.Length),fp1);
     fwrite(&m_BMPInfoHeader.width,1,sizeof(m_BMPInfoHeader.width),fp1);
     fwrite(&m_BMPInfoHeader.height,1,sizeof(m_BMPInfoHeader.height),fp1);
     fwrite(&m_BMPInfoHeader.colorPlane,1,sizeof(m_BMPInfoHeader.colorPlane),fp1);
     fwrite(&m_BMPInfoHeader.bitColor,1,sizeof(m_BMPInfoHeader.bitColor),fp1);
     fwrite(&m_BMPInfoHeader.zipFormat,1,sizeof(m_BMPInfoHeader.zipFormat),fp1);
     fwrite(&m_BMPInfoHeader.realSize,1,sizeof(m_BMPInfoHeader.realSize),fp1);
     fwrite(&m_BMPInfoHeader.xPels,1,sizeof(m_BMPInfoHeader.xPels),fp1);
     fwrite(&m_BMPInfoHeader.yPels,1,sizeof(m_BMPInfoHeader.yPels),fp1);
     fwrite(&m_BMPInfoHeader.colorUse,1,sizeof(m_BMPInfoHeader.colorUse),fp1);
     fwrite(&m_BMPInfoHeader.colorImportant,1,sizeof(m_BMPInfoHeader.colorImportant),fp1);
     fwrite(rgb_buffer,4*nWidth*nHeight,1,fp1);
     return 0;
}

#if 1
int nv12torgb(int width, int height, char *src, void *dst,int dstbuf_w,int rgb_format)
{
    uint8_t *y = src;//y(luminace) address
//    unsigned char *line = (char*)dst +(height -1) * 4 *width;//ffhsr we write the last line cos we convert rgb to bitmap
    uint8_t *misc = src + width * height;//u(Cb) v(Cr) start address
    uint8_t *cur = dst;
    int c, d, e;
    uint8_t  r, g, b;
    uint16_t rgb16;
    uint16_t *pRGB16;
    
    
    int i,j;
    printf("convert nv12 to rgb%d\n",8*rgb_format);
//    unsigned char *r,*g,*b,temp;
//    FILE * log = fopen("log.txt","wb");
//    char log_buffer[256];
    for( j = 0 ; j < height ; j++ ){
//        cur = line;
        for( i = 0 ; i < width ; i++ ){
            c = y[j*width+i] - 16;
            if(pic_gray == true){//trigger by key press
                d=0;e=0;
            }
            else{
                d = misc[(j>>1)*width+(i>>1<<1)  ] - 128;
                e = misc[(j>>1)*width+(i>>1<<1)+1] - 128;
            }
            switch(rgb_format){
                case RGB_FORMAT_16:
                    r = clip(( 298 * c           + 409 * e + 128) >> 8);
                    g = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);
                    b = clip(( 298 * c + 516 * d           + 128) >> 8);
                    rgb16 = (((short int)r>>3)<<11) | (((short int)g>>2)<<5) | (((short int)b>>3)<<0);
                    *((uint16_t*)cur) = rgb16;
                    cur = (uint8_t*)((uint16_t*)cur + 1);
                break;
                case RGB_FORMAT_24:
                    (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                    (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                    (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur++;
                break;
                case RGB_FORMAT_32:
 
        //            r = cur;
                    (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
        //            g = cur;
                    (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
        //            b = cur;
                    (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;
        //            temp = *r;
        //            *r = *b;
        //            *b = temp;
        //            sprintf(log_buffer,"%d*%d:c:%d,d:%d,e:%d,r:%x,g:%x,b:%x\n",j,i,c,d,e,*b,*g,*r);
        //            fwrite(log_buffer,strlen(log_buffer),1,log);
                break;
                default:
                    ASSERT(0);
                break;
            }
        }
        if(dstbuf_w > width)
            cur += (dstbuf_w - width)*rgb_format;
//        line -= 4 * width;
    }
//    fclose(log);
	

}
#else
int nv12torgb(int width, int height, char *src, void *dst,int dstbuf_w,int rgb_format)
{
    int line, col;
    int y, u, v, yy, vr, ug, vg, ub;
    int r, g, b;
    char *py, *pu, *pv;

    py = src;
    pu = py + (width * height);
    pv = pu + 1;
    y = *py++;
    yy = y << 8;
    u = *pu - 128;
    ug = 88 * u;
    ub = 454 * u;
    v = *pv - 128;
    vg = 183 * v;
    vr = 359 * v;
    
    for (line = 0; line < height; line++) {
        for (col = 0; col < width; col++) {
            r = (yy +      vr) >> 8;
            g = (yy - ug - vg) >> 8;
            b = (yy + ub     ) >> 8;
            if (r < 0)   r = 0;
            if (r > 255) r = 255;
            if (g < 0)   g = 0;
            if (g > 255) g = 255;
            if (b < 0)   b = 0;
            if (b > 255) b = 255;
            
            if(rgb_format == RGB_FORMAT_565)
            *(short int*)dst++ = (((short int)r>>3)<<11) | (((short int)g>>2)<<5) | (((short int)b>>3)<<0);
            else{
                ((rgb888*)dst)->r = b;
                ((rgb888*)dst)->g = g;
                ((rgb888*)dst)->b = r;
                dst = ((rgb888*)dst) + 1;

            }
        
            y = *py++;
            yy = y << 8;
            if (col & 1) {
                pu += 2;
                pv = pu+1;
                u = *pu - 128;
                ug =   88 * u;
                ub = 454 * u;
                v = *pv - 128;
                vg = 183 * v;
                vr = 359 * v;
            }
        }
        dst += dstbuf_w - width;
        if ((line & 1) == 0) { 
            //even line: rewind
            pu -= width;
            pv = pu+1;
        }
    }
    return 0;
}
#endif
/*
int rgb2nv12(int width, int height, char *src, void *dst,int dstbuf_w,int rgb_format){
float Y,U,V;
    int line, col;
    int y, u, v, yy, vr, ug, vg, ub;
    int r, g, b;
    char* rgb = src;
    for (line = 0; line < height; line++) {
        for (col = 0; col < width; col++) {
		R = *rgb;
		rgb++;
		G = *rgb;
		rgb++;
		B = *rgb;
		rgb++;

		Y = 0.299 R + 0.587 G + 0.114 B

		U = - 0.1687 R - 0.3313 G + 0.5 B + 128

		V = 0.5 R - 0.4187 G - 0.0813 B + 128
}*/

//only support rgb32
int yuyvtorgb(int width, int height, char *src, void *dst,int dstbuf_w,int rgb_format)
{
    uint8_t *misc = src;
    uint8_t *line = dst;
    uint8_t *cur;
    int c, d, e;
    uint8_t  r, g, b;
    uint16_t rgb16;
    uint16_t *pRGB16;
    int i,j;

    unsigned char* t = misc;
    for( j = 0 ; j < height ; j++ ){
        cur = line;
        for( i = 0 ; i < width ; i+=2 ){
            c = *(t+0) - 16;    // Y1
            d = *(t+1) - 128;   // U
            e = *(t+3) - 128;   // V

            (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
            (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
            (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;

            c = *(t+2) - 16;    // Y2
            (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
            (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
            (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;

            t += 4;
        }
        line += dstbuf_w<<2;
    }
}

//TODO now we only implement the 90 degree rotate 
//in rgb32 mode
int rotateAngular(int width,int height,int angular,int rgb_format,uint8_t* inRGB,uint8_t* outRGB)
{
    int i,j;
    for( j = 0 ; j < height ; j++ ){
//        cur = line;
        for( i = 0 ; i < width ; i++ ){
            //below is a travesal matrix
            //outRGB[i*height+j] = inRGB[j*width+i];//R
            //outRGB[i*height+j+1] = inRGB[j*width+i+1];//G
            //outRGB[i*height+j+2] = inRGB[j*width+i+2];//B
    //        switch(rgb_format){
      //          case RGB_FORMAT_16:
        //    }

            outRGB[(i*height+height-1-j)*rgb_format]   = inRGB[(j*width+i)*rgb_format];
            outRGB[(i*height+height-1-j)*rgb_format+1] = inRGB[(j*width+i)*rgb_format+1];
            outRGB[(i*height+height-1-j)*rgb_format+2] = inRGB[(j*width+i)*rgb_format+2];
            
        }
    }
    return 0;
    
}
/**********************
//TODO
1.write the last line first cos bmp start at down left corner
2.storage rgb as bgr
****************************************************************/
int RGB2BMP(int width,int height,uint8_t* rgbData,uint8_t* bmpData,int rgb_format){
    int i,j;
    int size = width * height;
    uint8_t R,G,B;
    uint8_t *line = bmpData +(height -1) * 4 *width;//write the last line
    uint8_t *cur;
    uint8_t *pRGB = rgbData;

    for( j = 0 ; j < height ; j++ ){
        cur = line;
        for( i = 0 ; i < width ; i++ ){

            switch(rgb_format){
                case RGB_FORMAT_16:
                    R = (*((short int*)pRGB)>>11)&0x1f;
                    G = (*((short int*)pRGB)>>5)&0x3f;
                    B = *((short int*)pRGB)&0x1f;
                    B = B<<3;
                    G = G<<2;
                    R = R<<3;
                    *cur=B;cur++;
                    *cur=G;cur++;
                    *cur=R;cur+=2;
                    pRGB = (uint8_t*)((short int*)pRGB + 1);
                break;
                case RGB_FORMAT_24:
                    R = *pRGB;pRGB++;
                    G = *pRGB;pRGB++;
                    B = *pRGB;pRGB++;
                    *cur=B;cur++;
                    *cur=G;cur++;
                    *cur=R;cur+=2;
                break;
                case RGB_FORMAT_32:
                    R = *pRGB;pRGB++;
                    G = *pRGB;pRGB++;
                    B = *pRGB;pRGB+=2;
                    *cur=B;cur++;
                    *cur=G;cur++;
                    *cur=R;cur+=2;
                break;
                default:
                    ASSERT(0);
                break;
            }
        } 
        line -= 4 * width;
    }
    return 0;
}

//test main
#if 0
void main(){
    int rgb_format = RGB_FORMAT_24;
    int w = 640,h = 480;
    int dst_w = 800;
    
    uint8_t in_buf[w*h*3/2];//yuv420
    uint8_t out_buf[dst_w*h*4];
    uint8_t out_buf_90[dst_w*h*4];
    memset(in_buf,0,sizeof(in_buf));
    memset(out_buf,0,sizeof(out_buf));
    memset(out_buf_90,0,sizeof(out_buf_90));

    FILE* fd_in = fopen("00000.yuv","rb");
    FILE* fd_out = fopen("00000.rgb","wb+");
    FILE* fd_out_90 = fopen("00000_90.rgb","wb+");
    FILE* fd_bmp = fopen("00000.bmp","wb+");//FIXME when open file
    printf("open file success\n");

    fread(in_buf,sizeof(in_buf),1,fd_in);
    nv12torgb(w,h,in_buf,out_buf,dst_w,rgb_format);
    fwrite(out_buf,dst_w*h*rgb_format,1,fd_out);
    printf("convert to rgb over\n");

    rotateAngular(dst_w,h,0,rgb_format,out_buf,out_buf_90);
    fwrite(out_buf_90,dst_w*h*rgb_format,1,fd_out_90);
    printf("roate 90 degree over\n");

    unsigned char* pBMPData =  malloc(dst_w * h * 4 *sizeof(unsigned char));
    
    RGB2BMP(dst_w,h,out_buf,pBMPData,rgb_format);
//    RGB2BMP(h,dst_w,out_buf_90,pBMPData,rgb_format);

/*****************************************************/
/***********  write file *******************/
    packageBMP(pBMPData,dst_w,h,fd_bmp);
//    packageBMP(pBMPData,h,dst_w,fd_bmp);
    printf("package bmp over\n");

    fclose(fd_in);
    fclose(fd_out);
    fclose(fd_bmp);

    free(pBMPData);
}
#endif
