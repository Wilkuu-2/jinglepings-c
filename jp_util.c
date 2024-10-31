
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "jp_util.h"
#include "ifaddrs.h"
#include "netdb.h"

uint32_t intermediate_checksum(uint16_t * data, uint16_t len) {
  uint32_t checksum = 0; 
  for(int i = 0; i < len/2; i++) {
    checksum += data[i];
    if (checksum > 0xFFFF) {       // Check for overflow
        checksum = (checksum  & 0xFFFF) + 1; // Wrap around the carry bit
    }
  } 

  return checksum; 
  
}

void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

void icmpv6_add_checksum(icmp_packet * restrict header, 
                                    const struct in6_addr * restrict src, 
                                    const struct in6_addr * restrict dest) {

  uint32_t checksum = 0; 

  icmpv6_pheader p;
  p.up_len = sizeof(icmp_packet); 
  p.next = 58; // ICMPv6
  memset(&p.padding, 0, 3);

  // TODO: Try using SIMD here 
  checksum += intermediate_checksum((uint16_t *) src, sizeof(struct in6_addr));
  checksum += intermediate_checksum((uint16_t *) dest, sizeof(struct in6_addr));
  checksum += intermediate_checksum((uint16_t *) &p, sizeof(icmpv6_pheader));
  checksum += intermediate_checksum((uint16_t *) header, sizeof(icmp_packet));
  header->hdr.icmp6_cksum = htons((uint16_t) ~checksum);
}

struct sockaddr_in6 find_own_ipv6addr(const char * ifname) {
  struct ifaddrs *ifap, *ifa;
  struct sockaddr_in6 *sa;
  char addr[INET6_ADDRSTRLEN];

  if (getifaddrs(&ifap) == -1) {
      perror("getifaddrs");
      exit(1);
  }

  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET6) {
      sa = (struct sockaddr_in6 *) ifa->ifa_addr;
      getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), addr,
                  sizeof(addr), NULL, 0, NI_NUMERICHOST);
      printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
      if(!strcmp(ifa->ifa_name, ifname)) {
        printf("Found interface\n"); 
        return *sa;
      }
    }
  }

  freeifaddrs(ifap);
  
  errno = ENONET;
  perror("find interface");
  exit(1); 

  struct sockaddr_in6 r = {};
  return r; 
}
