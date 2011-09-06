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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "timestamp.h"
#include "ioterror.h"
#include "iotdebug.h"


/**
 * Produces a timestamp in the format YYYY-MM-DDTHH:MM:SS[Z|[+|-]hh:mm]
 * @param dest Destination to write the timestamp string
 * @param maxSize Maximum size of the destination, at least TIMESTAMP_STAMP_SIZE
 * @return The size of the timestamp string
 */
int getTimestamp(char *dest, int maxSize) {
  time_t epochTime = time(NULL);
  struct tm *currentTime;
  int offset = 0;

  if(maxSize < TIMESTAMP_STAMP_SIZE) {
    return FAIL;
  }

  currentTime = localtime(&epochTime);
  offset = strftime(dest, maxSize, "%Y-%m-%dT%H:%M:%S", currentTime);
  getTimezone(dest + offset, maxSize - offset);

  return strlen(dest);
}

/**
 * Produces the time zone as specified by xsd:dateTime
 * @param dest Destination buffer to write the timezone into
 * @param size Size of the buffer, at least TIMESTAMP_ZONE_SIZE large
 */
void getTimezone(char *dest, int size) {
  struct tm *currentTime;
  time_t currtime;
  currtime = time(NULL);

  currentTime = localtime(&currtime);
  strftime(dest, TIMESTAMP_ZONE_SIZE, "%z", currentTime);

  //formatting from ISO 8601:2000 to Chapter 5.4 of ISO 8601
  // [+|-]hhmm will become [+|-]hh:mm
  dest[6] = 0;
  dest[5] = dest[4];
  dest[4] = dest[3];
  dest[3] = ':';
}
