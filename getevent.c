#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>  
int keypad_fd = -1;
bool video_enable = false;

int open_device(const char *device)
{
    int fd;
    struct pollfd *new_ufds;
    char **new_device_names;
    char name[80];
    char location[80];
    char idstr[80];
    struct input_id id;

    fd = open(device, O_RDWR);
    if(fd < 0) {
        fprintf(stderr, "could not open %s, %s\n", device, strerror(errno));
        return -1;
    }
    
    name[sizeof(name) - 1] = '\0';
    if(ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
        //fprintf(stderr, "could not get device name for %s, %s\n", device, strerror(errno));
        name[0] = '\0';
        return -1;
    }
    if(!strncmp(name,"rk29-keypad",11)){
//    if(!strncmp(name,"AT Translated",13)){
        keypad_fd = fd;
        return 0;
    }
    else{
        close(fd);
        return -1;
    }
    
}
static int scan_dir(const char *dirname)
{
    char devname[256];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        if(open_device(devname) == 0){
            printf("find keypad path:%s\n",devname);
            break;
        }    
    }
    closedir(dir);
    return 0;
}

void readEvent(){
//FIXME need we capture SIGINT to exit this thread?
    struct input_event event;
//    keypad_fd = open("/dev/input/event2",O_RDWR);
    while(1){
        if(keypad_fd == -1)
            pthread_exit(0);
        else{
            read(keypad_fd,&event,sizeof(event));
            if(event.type == EV_KEY){
                if(event.value == 1)//key pressed down
                    video_enable = !video_enable;
                    printf("read key event\n");
            }
        }
    }
}

int key_init(){
    char* input_path = "/dev/input";
    return scan_dir(input_path);
}
/*int main(){
    char* input_path = "/dev/input";
    pthread_t keypad_th;
 
    scan_dir(input_path);

    pthread_create(&keypad_th,NULL,(void  *) readEvent,NULL); 

    pthread_join(keypad_th,NULL);
    return 0;

}*/


