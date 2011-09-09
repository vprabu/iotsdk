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

#ifndef GADGETMANAGER_H
#define GADGETMANAGER_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "eui64.h"
#include "ioterror.h"

/**
 * Maximum number of gadgets can be set at compile time
 */
#ifndef GADGET_MAX_DEVICES
#define GADGET_MAX_DEVICES 8
#endif

/** Amount of time after we haven't heard from an gadget that we think it's dead */
#define GADGET_DEATH_PERIOD_SEC 600

/**
 * Information about each gadget
 */
typedef struct gadget_t {

  /** True if this slot is in use */
  bool inUse;

  /** True if this gadget's measurements have been updated recently */
  bool measurementsUpdated;

  /** Time of the last successful measurement */
  struct timeval lastTouchTime;

  /** IP address of the gadget */
  char ip[INET6_ADDRSTRLEN];

  /** Unique device ID */
  char uuid[EUI64_STRING_SIZE];

  /** Model description of the gadget */
  char model[32];

  /** Firmware version */
  char firmwareVersion[8];

  /** True if the outlet is on */
  bool isOn;

  /** Current in amps */
  double current_amps;

  /** Power in watts */
  double power_watts;

  /** Voltage */
  double voltage;

  /** Power factor */
  int powerFactor;

  /** Accumulated energy in watt-hours */
  double energy_wh;

} gadget_t;


/***************** Public Prototypes ****************/
error_t gadgetmanager_add(gadget_t *gadget);

gadget_t *gadgetmanager_getByIp(const char *ip);

gadget_t *gadgetmanager_getByUuid(const char *uuid);

int gadgetmanager_size();

gadget_t *gadgetmanager_get(int index);

void gadgetmanager_garbageCollection();

#endif
