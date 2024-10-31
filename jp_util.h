#ifndef JPUTIL 
#define JPUTIL

#include <stdint.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

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

typedef struct {
  struct icmp6_hdr hdr;
  char data[8]; 
} icmp_packet;

typedef struct{
  struct ip6_hdr hdr;
  icmp_packet icmp; 
} ipv6_packet;

typedef struct {
  uint32_t up_len; 
  uint8_t next;
  uint8_t padding[3];
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

uint32_t intermediate_checksum(uint16_t * data, uint16_t len);
void icmpv6_add_checksum(icmp_packet * restrict header, 
                                    const struct in6_addr * restrict src, 
                                    const struct in6_addr * restrict dest);
struct sockaddr_in6 find_own_ipv6addr(const char * ifname);
void DumpHex(const void* data, size_t size);

#endif 
