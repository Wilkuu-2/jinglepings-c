#ifndef JPUTIL 
#define JPUTIL

#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>

typedef struct {
  uint16_t x; 
  uint16_t y; 
  uint8_t b; 
  uint8_t g; 
  uint8_t r; 
  uint8_t a; 
} jpVaL;

typedef struct {
  uint64_t a; 
  jpVaL b;
} jp6_addr;

typedef struct __attribute__((packed)) {
  struct icmp6_hdr hdr;
  //char data[8]; 
} icmp_packet;

typedef struct __attribute__((packed)) {
  struct ip6_hdr hdr;
  icmp_packet icmp; 
} ipv6_packet;

typedef struct __attribute__((packed)) {
  uint16_t up_len; 
  uint8_t rest[4];
}  icmpv6_pheader;

typedef union {
  jp6_addr jp; 
  struct in6_addr ipv6;
} in6_jpaddr;

typedef struct {
  unsigned char r; 
  unsigned char g;
  unsigned char b;
} imgColorData;

typedef struct __attribute__((packed)) {
  struct ether_header ethernet;
  struct ip6_hdr ipv6; 
  icmp_packet icmp; 
} eth_packet;

typedef struct {
  struct sockaddr_in6 ip6; 
  uint8_t eth[ETH_ALEN]; 
} eth_ip6;

void icmpv6_add_checksum(icmp_packet * restrict header, 
                                    const struct in6_addr * restrict src, 
                                    const struct in6_addr * restrict dest);
struct sockaddr_in6 find_own_ipv6addr(const char * ifname);
eth_ip6 find_own_mac_and_ip6(const char * ifname);
void DumpHex(const void* data, size_t size);

#endif 
