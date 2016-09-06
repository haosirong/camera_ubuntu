#include <stdio.h>   
#include <stdlib.h>   
#include <fcntl.h>   
#include <unistd.h>   
#include <string.h>   
#include <sys/ioctl.h>   
#include <sys/mman.h>   
#include <asm/page.h>   
#include <linux/fb.h>   
#include <stdint.h>

#define TRUE        1   
#define FALSE       0   
#define MAX(x,y)        ((x)>(y)?(x):(y))   
#define MIN(x,y)        ((x)<(y)?(x):(y))   

#include <linux/fb.h>   
//a framebuffer device structure;   
typedef struct fbdev{   
       int fb;   
       unsigned long fb_mem_offset;   
       unsigned long fb_mem;   
       struct fb_fix_screeninfo fb_fix;   
       struct fb_var_screeninfo fb_var;   
       char dev[20];   
} FBDEV, *PFBDEV;   
  
  
  
  
//open & init a frame buffer   
//to use this function,   
//you must set FBDEV.dev="/dev/fb0"   
//or "/dev/fbX"   
//it's your frame buffer.   
int fb_open(PFBDEV pFbdev);   
//close a frame buffer   
int fb_close(PFBDEV pFbdev);   
//get display depth   
int get_display_depth(PFBDEV pFbdev);   
//full screen clear   
void fb_memset(void *addr, int c, size_t len); 

//open & init a frame buffer   
int fb_open(PFBDEV pFbdev)   
{   
       unsigned long screensize; 

       pFbdev->fb = open(pFbdev->dev, O_RDWR);   
       if(pFbdev->fb < 0)   
       {   
              printf("Error opening %s: %m. Check kernel config\n",   
pFbdev->dev);   
              return FALSE;   
       }   
       if (-1 == ioctl(pFbdev->fb,FBIOGET_VSCREENINFO,&(pFbdev->fb_var)))   
       {   
              printf("ioctl FBIOGET_VSCREENINFO\n");   
              return FALSE;   
       }   
       if (-1 == ioctl(pFbdev->fb,FBIOGET_FSCREENINFO,&(pFbdev->fb_fix)))   
       {   
              printf("ioctl FBIOGET_FSCREENINFO\n");   
              return FALSE;   
       }
   
       screensize = pFbdev->fb_var.xres*pFbdev->fb_var.yres*pFbdev->fb_var.bits_per_pixel/8;  

    fprintf(stdout,"<------------screen infomation------------->\n");  
    fprintf(stdout,"id=%s\n",pFbdev->fb_fix.id);  
    fprintf(stdout,"line_length=%d\n",pFbdev->fb_fix.line_length);  
    fprintf(stdout,"x=%d\n",pFbdev->fb_var.xres);  
    fprintf(stdout,"y=%d\n",pFbdev->fb_var.yres);  
    fprintf(stdout,"bpp=%d\n",pFbdev->fb_var.bits_per_pixel);  
    fprintf(stdout,"redoffset=%d,redlength=%d,msb_right=%d\n",  
            pFbdev->fb_var.red.offset,pFbdev->fb_var.red.length,pFbdev->fb_var.red.msb_right);  
    fprintf(stdout,"greenoffset=%d,greenlength=%d,msb_right=%d\n",  
            pFbdev->fb_var.green.offset,pFbdev->fb_var.green.length,pFbdev->fb_var.green.msb_right);  
    fprintf(stdout,"blueoffset=%d,bluelength=%d,msb_right=%d\n",  
            pFbdev->fb_var.blue.offset,pFbdev->fb_var.blue.length,pFbdev->fb_var.blue.msb_right);  
    fprintf(stdout,"screensize=%d\n",screensize); 
       //map physics address to virtual address   
       pFbdev->fb_mem_offset = (unsigned long)(pFbdev->fb_fix.smem_start) &   
(~PAGE_MASK);   
       pFbdev->fb_mem = (unsigned long int)mmap(NULL,   
pFbdev->fb_fix.smem_len ,              PROT_READ |   
PROT_WRITE, MAP_SHARED, pFbdev->fb, 0);   
       if (-1L == (long) pFbdev->fb_mem)   
       {   
              printf("mmap error! mem:%d offset:%d\n", pFbdev->fb_mem,   
pFbdev->fb_mem_offset);   
              return FALSE;   
       }   
       return TRUE;   
}   
  
  
  
  
//close frame buffer   
int fb_close(PFBDEV pFbdev)   
{   
       close(pFbdev->fb);   
       pFbdev->fb=-1;   
}   
  
  
  
  
//get display depth   
int get_display_depth(PFBDEV pFbdev)
{   
       if(pFbdev->fb<=0)   
       {   
              printf("fb device not open, open it first\n");   
              return FALSE;   
       }   
       return pFbdev->fb_var.bits_per_pixel;   
}   
  
  
  
  
//full screen clear   
void fb_memset (void *addr, int c, size_t len)   
{   
    memset(addr, c, len);   
}   

//void writeLine(int line_size,int row_num){
//    memcpy(fbdev.fb_mem,line_size);
//}
//use by test   
main()   
{   
       FBDEV fbdev;   
        FILE* fp = fopen("00000.rgb","rb");
        uint8_t* rgb24data = calloc(4,1);
        uint8_t* tmp = calloc(3,1);
int i,j;
int x = 1;  
if(*(char *)&x == 1)  
    printf("little-endian\n");  
else  printf("big-endian\n");
printf("11111111111\n");
//        fread(rgb24data,800*480*3,1,fp);
printf("11111111111\n");
       memset(&fbdev, 0, sizeof(FBDEV));   
       strcpy(fbdev.dev, "/dev/fb0");   
       if(fb_open(&fbdev)==FALSE)   
       {   
              printf("open frame buffer error\n");   
              return;   
       }  
	printf("fbdev.fb_mem_offset:%d,fbdev.fb_fix.smem_len:%d\n",fbdev.fb_mem_offset,fbdev.fb_fix.smem_len);
//       fb_memset(fbdev.fb_mem + fbdev.fb_mem_offset, 0x80,  fbdev.fb_fix.smem_len);   
//       fb_memset(fbdev.fb_mem + fbdev.fb_mem_offset, 0x80,  800*4);   

    for(i=0,j=0;i<800*480;i++,j++){
//        if(i%800 == 0)
//            j = i/800*1176;
        fread(tmp,3,1,fp);
        rgb24data[0] = tmp[2];//tmp[2];
        rgb24data[1] = tmp[1];//tmp[1];
        rgb24data[2] = tmp[0];//tmp[0];
        rgb24data[3] = 0;
        memcpy(fbdev.fb_mem + 4*i,rgb24data,4);
//        printf("%x %x %x %x\n",rgb24data[0],rgb24data[1],rgb24data[2],rgb24data[3]);

    }
              fb_close(&fbdev);   
        free(rgb24data);
}
