
#include "jp_util.h"
#include "netinet/in.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

int main(int argc, char ** argv){
  if(argc < 2) {
    printf("No interface defined \n");
    return 1; 
  }

  if(argc < 3) {
    printf("No image defined");
    return 1; 
  }
  
  short offX = 100;
  short offY = 300; 

  if(argc >= 5) {
	  offX = atoi(argv[3]);
	  offY = atoi(argv[4]);
  }

  int x, y, n; 
  imgColorData * imgData = stbi_load(argv[2], &x, &y, &n, 3); 

  if (n != 3) {
    printf("Image has an invalid amount of channels (%d), quitting", n);
    return 1;
  }

  printf("Image: (x: %d ,y: %d)", x ,y);


  int sock_r; 
  sock_r = socket(AF_INET6,SOCK_RAW,IPPROTO_ICMPV6);
  if (sock_r < 0) {
    perror("socket");
    return -1;
  } 

  struct sockaddr_in6 src_sa = find_own_ipv6addr(argv[1]);


  in6_jpaddr jp_dest = {
    0x00A0081910060120,
    {
      htons(0xd7a1), 
      htons(0xce01), 
      0xdb,
      0xb2,
      0xb4,
      0x4a
    },
  };

  struct sockaddr_in6 out_sa; 
  out_sa.sin6_family = src_sa.sin6_family;

  icmp_packet icmp = {
    {
      128, 
      0, 
      0,
    },
    "DEEZNUTS"
  };
 
  jp_dest.jp.b.a = 255;


  while(1) {
    
    short ix = 0; 
    short Nh = 17; 
    for (short ni = 0; ni < Nh; ni++){
      for (short iy = 0; iy < y; iy++){
        jp_dest.jp.b.y = htons(iy + offY); 
        for(int ix = ni; ix < x; ix += Nh){
          int index = x * iy + ix;
          jp_dest.jp.b.x = htons(ix + offX); 
          

          // memcpy((void *) &jp_dest.jp.b.r, (void *) imgData, 3); 
          imgColorData d = imgData[index];
          jp_dest.jp.b.r = d.r;
          jp_dest.jp.b.g = d.g;
          jp_dest.jp.b.b = d.b;
          out_sa.sin6_addr = jp_dest.ipv6;
          icmp.hdr.icmp6_cksum = 0;
          icmpv6_add_checksum(&icmp, &src_sa.sin6_addr, &jp_dest.ipv6); 
          int nsent = 0;
          // DumpHex(&jp_dest, 16);
          // printf("\rx: %4d y: %4d", ix, iy);
          sendto(sock_r, (void *) &icmp, sizeof(icmp_packet), 0, (struct sockaddr * )&out_sa, sizeof(struct sockaddr_in6));
         
          if(nsent < 0) {
              perror("sendto");
          } 
        }
      } 
    }
  }
} 
