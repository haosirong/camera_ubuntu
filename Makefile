open_camera:open_camera.c ionalloc.c format_convert.c getevent.c
	gcc -o $@ $^ -lpthread
