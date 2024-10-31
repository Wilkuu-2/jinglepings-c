
#include "netinet/in.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#include "jp_util.h"


int main (void) {
  int sock_r; 
  sock_r = socket(AF_INET6,SOCK_RAW,IPPROTO_RAW);
  if (sock_r < 0) {
    perror("socket");
    return -1;
  } 
  
  int opt = 1; 
  setsockopt(sock_r,IPPROTO_IPV6, IPV6_HDRINCL, (void *)&opt, 4);

  struct sockaddr_in6 src_sa = find_own_ipv6addr("wlp0s20f3");
  in6_jpaddr jp_dest = {
    0x00000000000008fe,// d7a1 ce01 dbb2 b44a
    {
      htons(0xd7a1), 
      htons(0xce01), 
      0xdb,
      0xb2,
      0xb4,
      0x4a
    },
  };

  icmp_packet icmp = {
    {
      128, 
      0, 
      0,
    },
    "DEEZNUTS"
  };

  icmpv6_add_checksum(&icmp, &src_sa.sin6_addr, (struct in6_addr *) &jp_dest);
 
  struct ip6_hdr head = {
    {
      htonl(0x69600000), 
      htons(sizeof(icmp_packet)), 
      58, 
      64,
    },
    src_sa.sin6_addr, 
    jp_dest.ipv6,
  };

  ipv6_packet pack = {
    head,
    icmp,
  };


  while(1) {
    int n_sent = send(sock_r, (void *) &pack, sizeof(ipv6_packet), 0);

    if(n_sent < 0) {
      perror("send");
    } 
    
    sleep(1);
  }
} 

