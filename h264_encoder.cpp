/**
 * 最简单的基于X264的视频编码器
 * Simplest X264 Encoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序可以YUV格式的像素数据编码为H.264码流，是最简单的
 * 基于libx264的视频编码器
 *
 * This software encode YUV data to H.264 bitstream.
 * It's the simplest encoder example based on libx264.
 */
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <string.h>

#include "h264_encoder.h"

#if defined ( __cplusplus)
extern "C"
{
#include "x264.h"
};
#else
#include "x264.h"
#endif

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 3 / 2)

//FILE* fp_src  = fopen("../cuc_ieschool_640x360_yuv444p.yuv", "rb");
//FILE* fp_src  = NULL;
FILE* fp_dst = NULL;
	
static int count;

x264_nal_t* pNals = NULL;
x264_t* pHandle   = NULL;
x264_picture_t* pPic_in = NULL;
x264_picture_t* pPic_out = NULL;
x264_param_t* pParam = NULL;
uint8_t* pic_buf = NULL;

int h264_init()
{

	int ret;
    int csp=X264_CSP_NV12;//color space

    pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
    pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
    pParam = (x264_param_t*)malloc(sizeof(x264_param_t));
    pic_buf = (uint8_t*)calloc(1,IMAGE_SIZE);
    if(pic_buf == NULL){
        printf("cannot alloc picture buffer\n");
        return -1;
    }
        

    //fp_src  = fopen("test.yuv", "rb");
    fp_dst = fopen("test.h264", "wb");
	
	//Check
	if(/*fp_src==NULL||*/fp_dst==NULL){
		printf("Error open files.\n");
		return -1;
	}

	x264_param_default(pParam);
	pParam->i_width   = IMAGE_WIDTH; 
	pParam->i_height  = IMAGE_HEIGHT;
	/*
	//Param
	pParam->i_log_level  = X264_LOG_DEBUG;
	pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
	pParam->i_frame_total = 0;
	pParam->i_keyint_max = 10;
	pParam->i_bframe  = 5;
	pParam->b_open_gop  = 0;
	pParam->i_bframe_pyramid = 0;
	pParam->rc.i_qp_constant=0;
	pParam->rc.i_qp_max=0;
	pParam->rc.i_qp_min=0;
	pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	pParam->i_fps_den  = 1; 
	pParam->i_fps_num  = 25;
	pParam->i_timebase_den = pParam->i_fps_num;
	pParam->i_timebase_num = pParam->i_fps_den;
	*/
	pParam->i_csp=csp;
	x264_param_apply_profile(pParam, x264_profile_names[5]);//high444

	x264_picture_init(pPic_out);
	x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);
}

int h264_enable(){
    count = 0;
	pHandle = x264_encoder_open(pParam);

	//ret = x264_encoder_headers(pHandle, &pNals, &iNal);
}

int h264_encode(uint8_t* yuv_buf){
    int i,iNal   = 0;
    int ret;
	int y_size = pParam->i_width * pParam->i_height;

    memcpy(pic_buf,yuv_buf,IMAGE_SIZE);    
	
    switch(pParam->i_csp){
    case X264_CSP_I444:{
        pPic_in->img.plane[0] = pic_buf;
        pPic_in->img.plane[1] = pic_buf + y_size;
        pPic_in->img.plane[2] = pic_buf + y_size *2;
//        fread(pPic_in->img.plane[0],y_size,1,fp_src);	//Y
//        fread(pPic_in->img.plane[1],y_size,1,fp_src);	//U
//        fread(pPic_in->img.plane[2],y_size,1,fp_src);	//V
        break;}
    case X264_CSP_I420:{
        pPic_in->img.plane[0] = pic_buf;
        pPic_in->img.plane[1] = pic_buf + y_size;
        pPic_in->img.plane[2] = pic_buf + y_size *5/4;
//        fread(pPic_in->img.plane[0],y_size,1,fp_src);	//Y
//        fread(pPic_in->img.plane[1],y_size/4,1,fp_src);	//U
//        fread(pPic_in->img.plane[2],y_size/4,1,fp_src);	//V
        break;}
    case X264_CSP_NV12:{
        pPic_in->img.plane[0] = pic_buf;
        pPic_in->img.plane[1] = pic_buf + y_size;
//        fread(pPic_in->img.plane[0],y_size,1,fp_src);	//Y
//        fread(pPic_in->img.plane[1],y_size/2,1,fp_src);	//UV
        break;}
    default:{
        printf("Colorspace Not Support.\n");
        return -1;}
    }
    pPic_in->i_pts = count;
    count++;

    ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
    if (ret< 0){
        printf("Error.\n");
        return -1;
    }

    printf("ffhsr Succeed encode frame: %5d\n",count);

    for ( i = 0; i < iNal; ++i){
        fwrite(pNals[i].p_payload, 1, pNals[i].i_payload, fp_dst);
	}

}

int h264_disable(){
	int i=0,j;
    int iNal = 0;
    int ret;
	//flush encoder
	while(1){
		ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);
		if(ret==0){
			break;
		}
		printf("ffhsr Flush 1 frame. count:%d\n",i);
		for (j = 0; j < iNal; ++j){
			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
		}
		i++;
	}
	x264_encoder_close(pHandle);
	pHandle = NULL;
}

int h264_deinit()
{
    free(pic_buf);
	x264_picture_clean(pPic_in);
	free(pPic_in);
	free(pPic_out);
	free(pParam);

//	fclose(fp_src);
	fclose(fp_dst);

	return 0;
}
