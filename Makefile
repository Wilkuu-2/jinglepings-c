


try3: try3.c stb_image.h jp_util.h jp_util.c
	clang try3.c -Ofast jp_util.c -lm -o try3
	sudo setcap cap_net_raw,cap_net_admin=ep try3 

try4: try4.c stb_image.h jp_util.h jp_util.c
	clang try4.c -Ofast jp_util.c -lm -L/lib/x86_64-linux-gnu/ -l:liburing.so -o try4
	sudo setcap cap_net_raw,cap_net_admin=ep try4 
