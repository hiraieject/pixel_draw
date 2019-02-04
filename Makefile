
CFLAGS +=

all: 
	@make compile
test:
	@make server
	@sleep 1
	@make client


compile: test01.cc test01.h connection.cc connection.h Makefile wb.h wb.cc wb_platform_xwin.cc wb_platform.h
	g++ test01.cc connection.cc wb.cc wb_platform_xwin.cc -DUSE_XWIN -o test01 -lX11 -lxml2 -I/usr/include/libxml2
# -I/usr/X11/include -L/usr/X11/lib

PORTNO=15032

server:
	./test01 -s -p $(PORTNO) &
client:
	./test01 -c -i 127.0.0.1 -p $(PORTNO) &

#	@sleep 1
#	@ ./test01 -s -i 127.0.0.1 -p $(PORTNO) &

#	sudo ./test01 -m -p $(PORTNO)
#	sudo ./test01 -s -i 127.0.0.1 -p $(PORTNO) &
