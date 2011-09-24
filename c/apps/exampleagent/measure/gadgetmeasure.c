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
 * This module takes measurements from all devices under our control
 *
 * @author Yvan Castilloux
 * @author David Moss
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cJSON.h"

#include "libhttpcomm.h"
#include "ioterror.h"
#include "iotdebug.h"
#include "proxy.h"
#include "gadgetmanager.h"
#include "gadgetmeasure.h"
#include "gadgetagent.h"
#include "iotapi.h"


/***************** Private Prototypes ****************/

/***************** Public Functions ****************/
/**
 * Capture measurements for all known gadgets
 *
 * THIS IS AN EXAMPLE ONLY! None of this code really works on any real device.
 */
void gadgetmeasure_capture() {
  int i;
  gadget_t *focusedGadget;
  char url[PATH_MAX];
  char rxBuffer[GADGET_MAX_MSG_SIZE];
  cJSON *jsonMsg = NULL;
  cJSON *jsonObject = NULL;
  http_param_t params;

  struct timeval curTime = { 0, 0 };

  params.verbose = false;
  params.timeouts.connectTimeout = 3;
  params.timeouts.transferTimeout = 15;

  /*
   * We keep a buffer of measurements until we know all measurements are good.
   * Then at the end, we copy all the measurements from the buffer to the actual
   * focusedGadget pointer.
   */
  for(i = 0; i < gadgetmanager_size(); i++) {
    if((focusedGadget = gadgetmanager_get(i)) != NULL) {
      if(focusedGadget->inUse) {

        // EXAMPLE ONLY!  This is if my device was available over an HTTP
        // RESTful interface on my network, and when I type in http://%s/get.js
        // it gives me back all the information in JSON format.
        snprintf(url, sizeof(url), "http://%s/get.xml", focusedGadget->ip);

        if (libhttpcomm_getMsg(NULL, url, NULL, NULL, rxBuffer, sizeof(rxBuffer), params, NULL) == 0) {
          if ((jsonMsg = cJSON_Parse(rxBuffer)) != NULL) {

            // State of the example gadget's outlet, 1 or 0
            if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "state")) != NULL) {
              focusedGadget->isOn = jsonObject->valueint;
            }

            // Current
            if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "amps")) != NULL) {
              focusedGadget->current_amps = jsonObject->valuedouble;
            }

            // Power
            if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "watts")) != NULL) {
              focusedGadget->power_watts = jsonObject->valuedouble;
            }

            // Volts
            if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "volts")) != NULL) {
              focusedGadget->voltage = jsonObject->valuedouble;
            }

            // Power Factor
            if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "pf")) != NULL) {
              focusedGadget->powerFactor = jsonObject->valueint;
            }

            // Energy
            if ((jsonObject = cJSON_GetObjectItem(jsonMsg, "energy")) != NULL) {
              focusedGadget->energy_wh = jsonObject->valuedouble;
            }

            // Log that we updated the measurements and last contact time
            focusedGadget->measurementsUpdated = true;
            gettimeofday(&curTime, NULL);
            focusedGadget->lastTouchTime.tv_sec = curTime.tv_sec;
          }
        }
      }
    }
  }
}

/**
 * Here we construct an IOT API XML message.
 *
 * We don't need to know the details of the XML API. We just need to do
 * 3 things for each device we want to send measurement updates for:
 *
 * 1. Create a new message with iotxml_newMsg(char *msg, int len)
 * 2. Add Strings and Ints to that message using iotxml_addString(..) and
 *    iotxml_addInt(..)
 * 3. Send the message with iotxml_send(char *msg, int len);
 *
 * THIS IS AN EXAMPLE ONLY!
 */
void gadgetmeasure_send() {
  char myMsg[PROXY_MAX_MSG_LEN];
  char buffer[10];
  int i;
  int offset = 0;
  gadget_t *focusedGadget;


  for(i = 0; i < gadgetmanager_size(); i++) {
    if((focusedGadget = gadgetmanager_get(i)) != NULL) {
      if(focusedGadget->inUse && focusedGadget->measurementsUpdated) {
        focusedGadget->measurementsUpdated = false;

        // I didn't want to fill up our buffer here, so each gadget gets
        // its own message and we toss it like a hot potato
        iotxml_newMsg(myMsg, sizeof(myMsg));

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
        snprintf(buffer, sizeof(buffer), "%4.2lf", focusedGadget->current_amps);
        offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
            focusedGadget->uuid,
            GADGET_DEVICE_TYPE,
            IOT_PARAM_MEASURE,
            "current",
            "m",
            0,
            buffer);

        // Power
        snprintf(buffer, sizeof(buffer), "%4.2lf", focusedGadget->power_watts);
        offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
            focusedGadget->uuid,
            GADGET_DEVICE_TYPE,
            IOT_PARAM_MEASURE,
            "power",
            "1",
            0,
            buffer);

        // Voltage
        snprintf(buffer, sizeof(buffer), "%3.1lf", focusedGadget->voltage);
        offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
            focusedGadget->uuid,
            GADGET_DEVICE_TYPE,
            IOT_PARAM_MEASURE,
            "volts",
            "1",
            0,
            buffer);

        // Energy
        snprintf(buffer, sizeof(buffer), "%3.1lf", focusedGadget->energy_wh);
        offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
            focusedGadget->uuid,
            GADGET_DEVICE_TYPE,
            IOT_PARAM_MEASURE,
            "energy",
            "k",
            0,
            buffer);

        // Power Factor
        offset += iotxml_addInt(myMsg + offset, sizeof(myMsg) - offset,
            focusedGadget->uuid,
            GADGET_DEVICE_TYPE,
            IOT_PARAM_MEASURE,
            "powerFactor",
            NULL,
            0,
            focusedGadget->powerFactor);

        // Outlet status
        offset += iotxml_addInt(myMsg + offset, sizeof(myMsg) - offset,
            focusedGadget->uuid,
            GADGET_DEVICE_TYPE,
            IOT_PARAM_MEASURE,
            "outletStatus",
            NULL,
            0,
            focusedGadget->isOn);

        // Send the measurement
        iotxml_send(myMsg, sizeof(myMsg));
      }
    }
  }
}


/***************** Private Functions *****************/


