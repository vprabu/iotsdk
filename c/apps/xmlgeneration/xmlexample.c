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
 * Example XML message generation
 * @author David Moss
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ioterror.h"
#include "iotdebug.h"
#include "iotapi.h"

static int TOTAL_DEVICES = 2;

static int DEVICE_TYPE = 0;

/***************** Private Prototypes ****************/
void _xmlexample_msg();

/***************** Public Functions ****************/
int main() {
  _xmlexample_msg();
  return 0;
}

/*
 * After the XML generator is finished generating a message, it attempts to
 * send a message to this function.  The IOT XML generator does not have this
 * function implemented because the application layer is responsible for
 * routing the outgoing message to whatever communication mechanism we're using.
 *
 * In this case, we really don't care about sending the message, we just
 * print out the XML to the terminal.
 *
 * @param msg The constructed message to send
 * @param len The size of the message
 * @return SUCCESS if the message is sent
 */
error_t application_send(const char *msg, int len) {
  printf("\n\n%s", msg);
  return SUCCESS;
}

/***************** Private Functions *****************/
void _xmlexample_msg() {
  char myMsg[4096];
  char buffer[10];
  char deviceId[10];
  int i;
  int offset = 0;

  for (i = 0; i < TOTAL_DEVICES; i++) {

    // I didn't want to fill up our buffer here, so each emulated device
    // gets its own message.
    iotxml_newMsg(myMsg, sizeof(myMsg));
    offset = 0;

    // Create an emulated unique device ID for each device
    snprintf(deviceId, sizeof(deviceId), "%d", i);

    // We have to print the floats / doubles to a string in order
    // to format it as we want to see it at the server. That's why
    // we do an snprintf(..) to a buffer, and then add that buffer
    // to our message.
    //
    // Also, since my example gadget is a 1-socket smart outlet,
    // I use 0 in place of the index because there are not multiple
    // outlets on each example device.  If we had a multiple outlets,
    // I would have used the character '0', '1', '2', .. as the index
    // for each individual outlet.

    // Current
    snprintf(buffer, sizeof(buffer), "%4.2lf", 1.0);
    offset += iotxml_addString(
        myMsg + offset, // Offset into the existing message to start writing XML
        sizeof(myMsg) - offset,  // Amount of space we have left to write
        deviceId, // Emulated unique device ID. Each for-loop has a unique ID
        DEVICE_TYPE, // Device ID, given to us when we registered our device with ESP
        IOT_PARAM_MEASURE, // We are sending a measurement message
        "current", // The parameter name of this measurement
        "m", // The multiplier of the units of measurement
        0, // ASCII character to be used as an index in case of multiple outlets
        buffer); // Actual measurement to add, ours is a pre-formatted string

    // Power
    snprintf(buffer, sizeof(buffer), "%4.2lf", 2.0);
    offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
        deviceId, DEVICE_TYPE, IOT_PARAM_MEASURE, "power",
        "1", 0, buffer);

    // Voltage
    snprintf(buffer, sizeof(buffer), "%3.1lf", 3.0);
    offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
        deviceId, DEVICE_TYPE, IOT_PARAM_MEASURE, "volts",
        "1", 0, buffer);

    // Energy
    snprintf(buffer, sizeof(buffer), "%3.1lf", 4.0);
    offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
        deviceId, DEVICE_TYPE, IOT_PARAM_MEASURE, "energy",
        "k", 0, buffer);

    // Power Factor
    offset += iotxml_addInt(myMsg + offset, sizeof(myMsg) - offset,
        deviceId, DEVICE_TYPE, IOT_PARAM_MEASURE,
        "powerFactor", NULL, 0, 5);

    // Outlet status
    offset += iotxml_addInt(myMsg + offset, sizeof(myMsg) - offset,
        deviceId, DEVICE_TYPE, IOT_PARAM_MEASURE,
        "outletStatus", NULL, 0, 6);

    // Send the measurement
    iotxml_send(myMsg, sizeof(myMsg));
  }
}

