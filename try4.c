
#include "jp_util.h"
#include "netinet/in.h"
#include <bits/types/sigset_t.h>
#include <stdint.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ip6.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <stdlib.h>
 #include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

#define BATCH_SIZE 17

// bc:24:11:58:37:25

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

  int iters = -1;
  if(argc >= 6) {
    iters = atoi(argv[5]); 
  }

  int x, y, n; 
  unsigned char * imgData = stbi_load(argv[2], &x, &y, &n, 4);

  if (n < 3) {
    printf("Image has an invalid amount of channels (%d), quitting", n);
    return 1;
  }


  printf("Image: (x: %d ,y: %d, n: %d)\n", x ,y, n);


  int sock_r; 
  sock_r = socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
  if (sock_r < 0) {
    perror("socket");
    return -1;
  } 

  struct ifreq if_idx;
  struct ifreq if_mac;

  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, argv[1], IFNAMSIZ-1);
  if (ioctl(sock_r, SIOCGIFINDEX, &if_idx) < 0){
    perror("Find index ioctl");
    return 1;
  }
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, argv[1], IFNAMSIZ-1);
  if (ioctl(sock_r, SIOCGIFHWADDR, &if_mac) < 0){
    perror("Find mac ioctl");
    return 1;
  }


  // uint8_t dest_mac[ETH_ALEN] = {0x00, 0x1C, 0x73, 0x00, 0x00, 0x99};
  uint8_t dest_mac[ETH_ALEN] = {0x04,0xf4,0x1c,0x39, 0xc2, 0xe1};
  struct sockaddr_ll eth_addr; 
  memcpy(eth_addr.sll_addr, dest_mac, ETH_ALEN);
  eth_addr.sll_halen = ETH_ALEN; 
  eth_addr.sll_ifindex = if_idx.ifr_ifindex; 
  eth_addr.sll_family = PF_PACKET; 

  struct sockaddr_in6 src_sa = find_own_ipv6addr(argv[1]);

  unsigned char pref[8] = {0x20, 0x01, 0x06, 0x10, 0x05, 0xea, 0x22, 0x1e};
  in6_jpaddr jp_dest = {0};
  memcpy(&(jp_dest.jp.a), pref, 8);

  

  struct sockaddr_in6 out_sa; 
  out_sa.sin6_family = src_sa.sin6_family;

  icmp_packet icmp = {
    {128, 
    0, 
    0,
    },
  };

// #define DEFAULT_GATEWAY 0x0000990000751C00 
// #define ETHERNET_ADDR   0x00003725581124BC 
  

  struct ether_header eth;
  memcpy(eth.ether_dhost, dest_mac, ETH_ALEN);
  memcpy(eth.ether_shost, if_mac.ifr_hwaddr.sa_data, ETH_ALEN);
  DumpHex(eth.ether_shost, 14);
  eth.ether_type = htons(ETH_P_IPV6);

  uint32_t top;
  uint8_t t_bytes[4] = {0x60,0x00, 0x00, 0x00};
  memcpy(&top, t_bytes, 4);
  struct ip6_hdr ip6 = {
    {top,
     htons(sizeof(icmp_packet)),
     IPPROTO_ICMPV6,
     64
    },
    src_sa.sin6_addr,
    jp_dest.ipv6,
  };

  eth_packet pack = {
    eth, 
    ip6, 
    icmp
  };

  jp_dest.jp.b.a = 255;

  printf("test %d---\n", iters);
  while(iters == -1 || iters > 0) {
    for(short nx = 0; nx < BATCH_SIZE; nx++){
      for (short iy = 0; iy < y; iy++){
        jp_dest.jp.b.y = htons(iy + offY); 
        for(short ix = 0; ix < x; ix += BATCH_SIZE){
          int index = x * iy + ix + nx;
          jp_dest.jp.b.x = htons(ix + nx + offX); 
          uint8_t * d = &imgData[index * n];
          // imgColorData d = imgData[0];
          if(n == 4) { 
            if(d[3] < 20) continue;
            jp_dest.jp.b.a = d[3];
          } else {
            jp_dest.jp.b.a = 255;
          }
          jp_dest.jp.b.r = d[0];
          jp_dest.jp.b.g = d[1];
          jp_dest.jp.b.b = d[2];
          pack.ipv6.ip6_dst = jp_dest.ipv6;
          // Do the checksum in the separate icmp packet, because of allignment 
          icmp.hdr.icmp6_cksum = 0;
          icmpv6_add_checksum(&icmp, &src_sa.sin6_addr, &jp_dest.ipv6); 
          pack.icmp.hdr.icmp6_cksum = icmp.hdr.icmp6_cksum;

           printf("\rx: %4d y: %4d", ix, iy);
          
           int s = sendto(sock_r, &pack, sizeof(eth_packet), 0,(struct sockaddr *) &eth_addr, sizeof(struct sockaddr_ll));
          if(s < 0) {
             perror("sendto");
           }
          //usleep(20);

          // DumpHex(&pack, sizeof(pack));
          // DumpHex(&pack.icmp, sizeof(pack.icmp));
          // printf("Size of packet: %lu\n", sizeof(pack));
          // exit(0);
        }
      }
    } 
    if (iters > 0) {
      iters --;
    }
  }
}
