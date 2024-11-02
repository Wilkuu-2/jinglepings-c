
#include "jp_util.h"
#include "netinet/in.h"
#include <bits/types/sigset_t.h>
#include <stdint.h>
#include <liburing/io_uring.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ip6.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <liburing.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

#define BATCH_SIZE 17

// bc:24:11:58:37:25
void read_completions(struct io_uring * ior){
  struct io_uring_cqe *cqe; 
  
  sigset_t s = {}; 
  io_uring_enter(ior->ring_fd, BATCH_SIZE, BATCH_SIZE, 0, &s);
  int to_collect = io_uring_cq_ready(ior);
  //int to_collect = BATCH_SIZE;
  for(int i = 0; i < to_collect; i ++){
    io_uring_wait_cqe(ior,&cqe);
    if (cqe->res < 0) {
      printf("Error in sending\n");
    } 
  }
}

void submit_packet(struct io_uring * restrict ior,int socket, eth_packet * restrict packet, struct sockaddr_ll * restrict dest ){
  struct io_uring_sqe * sqe = NULL; 
  eth_packet e; 
  memcpy(&e,packet,sizeof(eth_packet));
  do {
    sqe = io_uring_get_sqe(ior); 
  } while (sqe == NULL);

  io_uring_prep_sendto(sqe, socket, packet, sizeof(eth_packet), 0,(struct sockaddr *) dest, sizeof(struct sockaddr_ll));
  io_uring_submit(ior);
}


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


  uint8_t dest_mac[ETH_ALEN] = {0x00, 0x1C, 0x73, 0x00, 0x00, 0x99};
  struct sockaddr_ll eth_addr; 
  memcpy(eth_addr.sll_addr, dest_mac, ETH_ALEN);
  eth_addr.sll_halen = ETH_ALEN; 
  eth_addr.sll_ifindex = if_idx.ifr_ifindex; 
  eth_addr.sll_family = PF_PACKET; 

  struct sockaddr_in6 src_sa = find_own_ipv6addr(argv[1]);


  in6_jpaddr jp_dest = {
    0x00A0081910060120,
    {
      htons(0xcdab), 
      htons(0x3412), 
      0xde,
      0xad,
      0xbe,
      0xaf
    },
  };

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
          // printf("\rx: %4d y: %4d", ix, iy);
          
          int s = sendto(sock_r, &pack, sizeof(eth_packet), 0,(struct sockaddr *) &eth_addr, sizeof(struct sockaddr_ll));
          if(s < 0) {
            perror("sendto");
          }

          // DumpHex(&pack, sizeof(pack));
          // DumpHex(&pack.icmp, sizeof(pack.icmp));
          // exit(0);o
        }
      }
    } 
    if (iters > 0) {
      iters --;
    }
  }
}
