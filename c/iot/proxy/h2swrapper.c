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

/**
 * Every message to the server must be wrapped in an XML header.  This
 * module is responsible for generating that header, which is put in place
 * before the message exits the local device through the proxy.
 *
 * @author David Moss
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "eui64.h"
#include "ioterror.h"
#include "iotdebug.h"

static uint32_t sequenceNum = 0;

/**
 * Wraps the message to the server inside an XML header and footer
 *
 * @param msg The message to wrap
 * @param maxSize The maximum size of the message buffer
 * @param sequenceNum Sequence number of the XML message
 * @return Number of bytes written or -1 for error
 */
int h2swrapper_wrap(char *dest, char *message, int destSize) {
  int bytesWritten = 0;
  char localAddress[EUI64_STRING_SIZE];

  assert(dest);
  assert(message);

  sequenceNum++;

  eui64_toString(localAddress, sizeof(localAddress));

  bytesWritten += snprintf(dest, destSize - bytesWritten,
      "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<h2s ver=\"1\" hubId=\"%s\" seq=\"%u\">", localAddress, sequenceNum);

  strncpy(dest + bytesWritten, message, destSize - bytesWritten);

  bytesWritten += strlen(message);

  bytesWritten += snprintf(dest + bytesWritten, destSize - bytesWritten, "</h2s>");

  return bytesWritten;
}

