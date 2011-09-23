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
 * This module manages the state of all gadgets in our network
 * @author David Moss
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "ioterror.h"
#include "iotdebug.h"

#include "gadgetmanager.h"
#include "gadgetagent.h"

/** Gadgets we are currently tracking */
static gadget_t devices[GADGET_MAX_DEVICES];

/***************** Public Functions ****************/
error_t gadgetmanager_add(gadget_t *gadget) {
  int i;

  for(i = 0; i < GADGET_MAX_DEVICES; i++) {
    if(devices[i].inUse && strcmp(gadget->ip, devices[i].ip) == 0) {
      SYSLOG_INFO("[gadget] Device at IP %s is already being tracked", gadget->ip);
      return SUCCESS;
    }
  }

  for(i = 0; i < GADGET_MAX_DEVICES; i++) {
    if(!devices[i].inUse) {
      SYSLOG_INFO("[gadget] Adding new device at ip %s!", gadget->ip);

      // Always add the device to the ESP
      iotxml_addDevice(gadget->uuid, GADGET_DEVICE_TYPE);

      memcpy(&devices[i], gadget, sizeof(gadget_t));
      devices[i].inUse = true;
      gadgetagent_refreshDevices();
      return SUCCESS;
    }
  }

  return FAIL;
}


/**
 * Obtain a gadget by IP address
 * @param ip IP address of a device
 * @return gadget_t if the device is being tracked
 */
gadget_t *gadgetmanager_getByIp(const char *ip) {
  int i;
  for(i = 0; i < GADGET_MAX_DEVICES; i++) {
    if(strncmp(ip, devices[i].ip, EUI64_STRING_SIZE) == 0) {
      return &devices[i];
    }
  }

  return NULL;
}


/**
 * Obtain a device by UUID
 * @param uuid UUID of a device
 * @return gadget_t if we are tracking this device
 */
gadget_t *gadgetmanager_getByUuid(const char *uuid) {
  int i;
  for(i = 0; i < GADGET_MAX_DEVICES; i++) {
    if(strncmp(uuid, devices[i].uuid, EUI64_STRING_SIZE) == 0) {
      return &devices[i];
    }
  }

  return NULL;
}


/**
 * @return the number of elements in our device array
 */
int gadgetmanager_size() {
  return GADGET_MAX_DEVICES;
}

/**
 * @param index Index into the array of devices
 * @return the device at the given index
 */
gadget_t *gadgetmanager_get(int index) {
  if(index < GADGET_MAX_DEVICES) {
    return &devices[index];
  } else {
    return NULL;
  }
}

/**
 * Kill off the devices we haven't heard from in a long time
 */
void gadgetmanager_garbageCollection() {
  int i;
  struct timeval curTime = { 0, 0 };

  for(i = 0; i < GADGET_MAX_DEVICES; i++) {
    if(devices[i].inUse) {
      if ((curTime.tv_sec - devices[i].lastTouchTime.tv_sec) >= GADGET_DEATH_PERIOD_SEC) {
        SYSLOG_INFO("[gadget] Killing device at IP %s", devices[i].ip);

        // Alert that the device is gone
        iotxml_alertDeviceIsGone(devices[i].uuid);

        bzero(&devices[i], sizeof(gadget_t));
      }
    }
  }
}
