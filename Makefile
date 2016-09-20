#CFLAGS := -Wdeprecated-declarations -Wunused-result
LDFLAGS := -lavformat -lavcodec -lavutil -lpthread
cset := open_camera.c ionalloc.c format_convert.c getevent.c
open_camera:h264_encoder.cpp open_camera.c ionalloc.c format_convert.c getevent.c 
	gcc -o $@ $^ $(LDFLAGS)
#	g++ -c h264_encoder.cpp
#	gcc -o $@ $(cset) h264_encoder.o $(LDFLAGS)
