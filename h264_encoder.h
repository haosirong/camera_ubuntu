#ifndef H264_ENCODE_H
#define H264_ENCODE_H
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
extern int h264_init();
extern int h264_deinit();
extern int h264_enable();
extern int h264_disable();
extern int h264_encode(uint8_t* yuv_buf);
#ifdef __cplusplus
}
#endif
#endif
