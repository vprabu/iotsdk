/*
 * Copyright (c) 2010 People Power Company
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


/**
 * Discovery gadgets on the local subnet
 *
 * This module is responsible for discovering devices and gathering static
 * "heartbeat" related profile data about those devices.
 *
 * All of this code is EXAMPLE ONLY
 *
 * @author Yvan Castilloux
 * @author David Moss
 */

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>
#include <rpc/types.h>

#include "cJSON.h"
#include "libhttpcomm.h"
#include "ioterror.h"
#include "iotdebug.h"
#include "gadgetdiscovery.h"
#include "gadgetmanager.h"
#include "gadgetagent.h"


/***************** Private Prototypes ***************/
static void _gadgetdiscovery_ssdpHandler(char *url);

static error_t _gadgetdiscovery_captureGadgetDetails(gadget_t *gadget);

void _gadgetdiscovery_poll(const char *ip);

/***************** Public Functions ***************/
/**
 * Discover new GADGET devices
 *
 * This is an EXAMPLE ONLY! This code won't actually do anything.
 * It is up to the developer to implement the appropriate discovery mechanism
 * required to find what devices can be under our control, and then
 * gather information about those devices so we can pass the device on to
 * the gadgetmanager to manage its lifetime.
 *
 * This will run periodically and update the existing devices on each pass.
 */
error_t gadgetdiscovery_runOnce() {
  unsigned int len;
  struct sockaddr_in cliaddr;
  struct sockaddr_in destaddr;
  struct timeval tv;
  struct timeval curTime;
  char buffer[GADGET_MAX_MSG_SIZE] = "TYPE: WM-DISCOVER\r\nVERSION:2.5\r\n\r\nservices: com.peoplepower.wm.system*\r\n\r\n";
  char *token;
  int count = 0;
  struct ip_mreq mc_req;  int sock;
  int ret;
  int one = 1;
  int ttl = 3;

  // Create socket
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    return FAIL;
  }

  // Allow socket reuse
  ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one));

  if (ret < 0) {
    return FAIL;
  }

  if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
    return FAIL;
  }

  ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &ttl,
      sizeof(ttl));

  if (ret < 0) {
    return FAIL;
  }

  // construct a socket bind address structure
  cliaddr.sin_family = AF_INET;
  cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliaddr.sin_port = htons(0);

  ret = bind(sock, (struct sockaddr *) &cliaddr, sizeof(cliaddr));

  if (ret < 0) {
    return FAIL;
  }

  // construct an IGMP join request structure
  mc_req.imr_multiaddr.s_addr = inet_addr(GADGET_SSDP_ADDR);
  mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

  // send an ADD MEMBERSHIP message via setsockopt
  if ((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*) &mc_req,
      sizeof(mc_req))) < 0) {
    return FAIL;
  }

  // Set destination for multicast address
  destaddr.sin_family = AF_INET;
  destaddr.sin_addr.s_addr = inet_addr(GADGET_SSDP_ADDR);
  destaddr.sin_port = htons(GADGET_SSDP_PORT);

  // Send the multicast packet
  len = strlen(buffer);
  ret = sendto(sock, buffer, len, 0, (struct sockaddr *) &destaddr,
      sizeof(destaddr));

  if (ret < 0) {
    return FAIL;
  }

  SYSLOG_DEBUG("[gadget] SSDP multicast sent, waiting...");
  gettimeofday(&curTime, NULL);
  tv.tv_sec = curTime.tv_sec;

  while ((curTime.tv_sec - tv.tv_sec) < 3) {
    gettimeofday(&curTime, NULL);

    len = sizeof(destaddr);
    ret = recvfrom(sock, buffer, GADGET_MAX_MSG_SIZE, 0, (struct sockaddr *) &destaddr, &len);

    if (ret > 0) {
      count++;

      token = strtok(buffer, "\r\n");

      while (token != NULL) {
        if (!strncasecmp(token, GADGET_LOCATION_HDR, strlen(GADGET_LOCATION_HDR))) {
          SYSLOG_DEBUG("Found a wireless microcontroller, base URI: %s", token + strlen(GADGET_LOCATION_HDR));
          _gadgetdiscovery_ssdpHandler(token + strlen(GADGET_LOCATION_HDR));
          break;
        }

        token = strtok(NULL, "\r\n");
      }

    } else {
      usleep(500000);
    }
  }

  /* send a DROP MEMBERSHIP message via setsockopt */
  if ((setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*) &mc_req, sizeof(mc_req))) < 0) {
    return FAIL;
  }

  return SUCCESS;
}



/***************** Private Functions ****************/
/**
 * Here I take the URL of a newly discovered device and extract its IP address
 * before passing it on to _gadgetdiscovery_poll(..) to update the device
 * details
 */
static void _gadgetdiscovery_ssdpHandler(char *url) {
  char ip[INET6_ADDRSTRLEN];
  char *ptr;
  char *ptr2;

  ptr = strstr(url, "://");
  if (ptr != NULL) {
    ptr += 3; // advance pointer after "://"
    ptr2 = strchr(ptr, '/');
    if (ptr2 == NULL) {
      strncpy(ip, ptr, sizeof(ip));
    } else {
      strncpy(ip, ptr, ptr2 - ptr);
      ip[ptr2 - ptr] = 0;
    }

    _gadgetdiscovery_poll(ip);
  }
}


/**
 * Once we found a new device at some IP address, we attempt to either grab
 * the gadget_t pointer from our gadgetmanager and update it... or if it
 * doesn't exist, we create a new gadget.  In either case, we query the
 * gadget for metadata details.
 *
 * The gadgetmanager_add(...) will happily return SUCCESS without making
 * multiple copies of the gadget if it is already tracking the gadget at that
 * IP address.
 */
void _gadgetdiscovery_poll(const char *ip) {
  gadget_t gadget;
  gadget_t *gadgetPtr;

  assert(ip);

  if ((gadgetPtr = gadgetmanager_getByIp(ip)) == NULL) {
    gadgetPtr = &gadget;
    bzero(gadgetPtr, sizeof(gadget_t));
    strcpy(gadgetPtr->ip, ip);
    SYSLOG_INFO("[gadget] Creating new device with IP %s", gadgetPtr->ip);

  } else {
    SYSLOG_INFO("[gadget] Refreshing device IP %s", gadgetPtr->ip);
  }

  if(_gadgetdiscovery_captureGadgetDetails(gadgetPtr) == SUCCESS) {
    gadgetmanager_add(gadgetPtr);
  }
}

/**
 * Capture the details of a gadget at a particular address
 *
 * EXAMPLE ONLY!  We capture the static metadata from1 a device by querying it
 * and pretend the returned data is JSON
 */
static error_t _gadgetdiscovery_captureGadgetDetails(gadget_t *gadget) {
  char url[PATH_MAX];
  char rxBuffer[GADGET_MAX_MSG_SIZE];
  http_param_t params;
  cJSON *jsonMsg = NULL;
  cJSON *jsonObject = NULL;

  assert(gadget);

  params.verbose = false;
  params.timeouts.connectTimeout = 3;
  params.timeouts.transferTimeout = 15;

  snprintf(url, sizeof(url), "http://%s/get.xml", gadget->ip);
  if (libhttpcomm_getMsg(NULL, url, NULL, NULL, rxBuffer, sizeof(rxBuffer), params, NULL) == 0) {
    if ((jsonMsg = cJSON_Parse(rxBuffer)) != NULL) {

      if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "uuid")) != NULL) {
        strcpy(gadget->uuid, jsonObject->valuestring);
      }

      if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "model")) != NULL) {
        strcpy(gadget->model, jsonObject->valuestring);
      }

      if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "version")) != NULL) {
        strcpy(gadget->firmwareVersion, jsonObject->valuestring);
      }
    }
  }

  return SUCCESS;
}

