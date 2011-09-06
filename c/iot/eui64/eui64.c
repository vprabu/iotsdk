/*
 * Copyright (c) 2011 People Power Company
 * All rights reserved.
 *
 * This open source code was developed with funding from People Power Company
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the People Power Corporation nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * PEOPLE POWER CO. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE
 */


#include <assert.h>
#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <rpc/types.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "ioterror.h"
#include "iotdebug.h"
#include "eui64.h"

/**
 * Obtain the 48-bit MAC dest and convert to an EUI-64 value from the
 * hardware NIC
 *
 * @param dest Buffer of at least 8 bytes
 * @param destLen Length of the buffer
 * @return SUCCESS if we are able to capture the EUI64
 */
error_t eui64_toBytes(uint8_t *dest, int destLen) {
  struct ifreq *ifr;
  struct ifconf ifc;
  char buf[1024];
  int sock, i;
  int ok = 0;

  assert(dest);

  if(destLen < EUI64_BYTES_SIZE) {
    return FAIL;
  }

  memset(dest, 0x0, destLen);

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == -1) {
    return -1;
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  ioctl(sock, SIOCGIFCONF, &ifc);

  ifr = ifc.ifc_req;
  for (i = 0; i < ifc.ifc_len / sizeof(struct ifreq); ifr++) {
    if (strcmp(ifr->ifr_name, "eth0") == 0 || strcmp(ifr->ifr_name, "eth1")
        == 0 || strcmp(ifr->ifr_name, "wlan0") == 0 || strcmp(ifr->ifr_name,
        "br0") == 0) {
      if (ioctl(sock, SIOCGIFFLAGS, ifr) == 0) {
        if (!(ifr->ifr_flags & IFF_LOOPBACK)) {
          if (ioctl(sock, SIOCGIFHWADDR, ifr) == 0) {
            ok = 1;
            break;
          }
        }
      }
    }
  }

  close(sock);
  if (ok) {
    /* Convert 48 bit MAC dest to EUI-64 */
    memcpy(dest, ifr->ifr_hwaddr.sa_data, 3);
    /* Insert the converting bits in the middle */
    dest[3] = 0xFF;
    dest[4] = 0xFE;
    memcpy(&dest[5], &(ifr->ifr_hwaddr.sa_data[3]), 3);

  } else {
    SYSLOG_ERR("Couldn't read MAC dest to seed EUI64");
    return FAIL;
  }

  return SUCCESS;
}

/**
 * Copy the EUI64 into a string
 * @return SUCCESS if we are able to capture the EUI64
 */
error_t eui64_toString(char *dest, int destLen) {
  uint8_t byteAddress[EUI64_BYTES_SIZE];

  assert(dest);

  if(destLen < EUI64_STRING_SIZE) {
    return FAIL;
  }

  if (eui64_toBytes(byteAddress, sizeof(byteAddress)) == SUCCESS) {
    snprintf(dest, destLen, "%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X",
        byteAddress[0], byteAddress[1], byteAddress[2], byteAddress[3],
        byteAddress[4], byteAddress[5], byteAddress[6], byteAddress[7]);

    return SUCCESS;
  }

  return FAIL;
}
