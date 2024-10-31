


try3: try3.c stb_image.h jp_util.h jp_util.c
	clang try3.c -Ofast jp_util.c -lm -o try3
	sudo setcap cap_net_raw,cap_net_admin=ep try3 
