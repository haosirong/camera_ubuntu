#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/aufs_type.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ion/ionalloc.h"

#define PAGE_SIZE 4096

#define  LOGD(...)  do {printf(__VA_ARGS__);printf("\n");} while (0)
#define DBG(fmt, args...) LOGD("%s:%d, " fmt, __FUNCTION__, __LINE__, ##args);
#define ASSERT(b) \
do \
{ \
    if (!(b)) \
    { \
        LOGD("error on %s:%d", __FUNCTION__, __LINE__); \
        return 0; \
    } \
} while (0)

#define VIDEO_DEVICE "/dev/video0"
#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 3/2)  //yuv420
#define BUFFER_COUNT 4

extern int ion_alloc(struct ion_device_t *ion, unsigned long size, enum _ion_heap_type type,ion_buffer_t * *data);

int cam_fd = -1;
struct v4l2_buffer video_buffer[BUFFER_COUNT];
uint8_t* video_buffer_ptr[BUFFER_COUNT];
uint8_t yuv_buf[IMAGE_SIZE];

int cam_open()
{
    cam_fd = open(VIDEO_DEVICE, O_RDWR);

    if (cam_fd >= 0) return 0;
    else return -1;
}

int cam_close()
{
    close(cam_fd);

    return 0;
}

int cam_select(int index)
{
    int ret;

    int input = index;
    ret = ioctl(cam_fd, VIDIOC_S_INPUT, &input);
    return ret;
}

int cam_init()
{
    int i;
    int ret;
    struct v4l2_format format;
    ion_device_t *ion_dev;
    ion_buffer_t *ion_data;

    ion_open(PAGE_SIZE,ION_MODULE_CAM,&ion_dev);

    ion_alloc(ion_dev,ALIGN(IMAGE_SIZE,PAGE_SIZE)*BUFFER_COUNT,_ION_HEAP_RESERVE,&ion_data);

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    format.fmt.pix.width = IMAGE_WIDTH;
    format.fmt.pix.height = IMAGE_HEIGHT;
    ret = ioctl(cam_fd, VIDIOC_TRY_FMT, &format);
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_TRY_FMT) failed %d(%s)", errno, strerror(errno));
        return ret;
    }

    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd, VIDIOC_S_FMT, &format);
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_S_FMT) failed %d(%s)", errno, strerror(errno));
        return ret;
    }

    struct v4l2_requestbuffers req;
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_OVERLAY;
    ret = ioctl(cam_fd, VIDIOC_REQBUFS, &req);
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_REQBUFS) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
    DBG("req.count: %d", req.count);
    if (req.count < BUFFER_COUNT)
    {
        DBG("request buffer failed");
        return ret;
    }

    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.type = req.type;
    buffer.memory = V4L2_MEMORY_OVERLAY;
    for (i=0; i<req.count; i++)
    {
        buffer.index = i;
        ret = ioctl (cam_fd, VIDIOC_QUERYBUF, &buffer);
        if (ret != 0)
        {
            DBG("ioctl(VIDIOC_QUERYBUF) failed %d(%s)", errno, strerror(errno));
            return ret;
        }
        DBG("buffer.length: %d", buffer.length);
        DBG("buffer.m.offset: %d", buffer.m.offset);
/*        video_buffer_ptr[i] = (uint8_t*) mmap(NULL, buffer.length, PROT_READ, MAP_SHARED, cam_fd, buffer.m.offset);
        if (video_buffer_ptr[i] == MAP_FAILED)
        {
            DBG("mmap() failed %d(%s)", errno, strerror(errno));
            return -1;
        }*/

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_OVERLAY;
        buffer.index = i;
        buffer.m.offset = ion_data->phys+i*ALIGN(IMAGE_SIZE,PAGE_SIZE);
        video_buffer_ptr[i] = (uint8_t*)(ion_data->virt+i*ALIGN(IMAGE_SIZE,PAGE_SIZE));
        ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);
        if (ret != 0)
        {
            DBG("ioctl(VIDIOC_QBUF) failed %d(%s)", errno, strerror(errno));
            return ret;
        }
    }

    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd, VIDIOC_STREAMON, &buffer_type);
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_STREAMON) failed %d(%s)", errno, strerror(errno));
        return ret;
    }

    DBG("cam init done.");

    return 0;
}

int cam_get_image(uint8_t* out_buffer, int out_buffer_size)
{
    int ret;
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_OVERLAY;
    buffer.index = BUFFER_COUNT;
    ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_DQBUF) failed %d(%s)", errno, strerror(errno));
        return ret;
    }

    if (buffer.index < 0 || buffer.index >= BUFFER_COUNT)
    {
        DBG("invalid buffer index: %d", buffer.index);
        return ret;
    }

    DBG("dequeue done, index: %d", buffer.index);
    memcpy(out_buffer, video_buffer_ptr[buffer.index], IMAGE_SIZE);
    DBG("copy done.");

    ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_QBUF) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
    DBG("enqueue done.");

    return 0;
}

int main()
{
    int i;
    int ret;
    int rgb_format = RGB_FORMAT_24;
    int w = 640,h = 480;
    int dst_w = 800;


    ret = cam_open();
    ASSERT(ret==0);

    ret = cam_select(0);
    ASSERT(ret==0);

    ret = cam_init();
    ASSERT(ret==0);

    int count = 0;
    while (1)
    {
        ret = cam_get_image(yuv_buf, IMAGE_SIZE);
        ASSERT(ret==0);

        char tmp[64] = {"---\n"};
        for (i=0; i<16; i++)
            sprintf(&tmp[strlen(tmp)], "%02x ", yuv_buf[i]);
        LOGD("%s", tmp);

/*        char filename[32];
        sprintf(filename, "%05d.yuv", count++);
        int fd = open(filename, O_RDWR | O_CREAT);
        if (fd >= 0)
        {
            write(fd, yuv_buf, IMAGE_SIZE);
            close(fd);
        }
        else
        {
            LOGD("open() failed: %d(%s)", errno, strerror(errno));
        }*/
    }

    ret = cam_close();
    ASSERT(ret==0);

    return 0;
}

