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
 * This module implements commands from the server
 *
 * The function here, gadgetconrol_execute(command_t *cmd), is called because
 * our main gadgetagent.c file added the function as a listener to the XML
 * parser.  When the XML parser is done parsing a new command, this function
 * is called with a pointer to the command struct.
 *
 * @author Yvan Castilloux
 * @author David Moss
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#include <cJSON.h>

#include "libhttpcomm.h"

#include "ioterror.h"
#include "iotdebug.h"
#include "proxy.h"
#include "gadgetagent.h"
#include "gadgetmanager.h"
#include "gadgetmeasure.h"
#include "iotapi.h"


/***************** Public Functions ****************/
/**
 * All commands and acks get broadcast to all agents. Therefore, we must
 * filter out all the commands that don't belong to us, and only execute
 * the commands that we have a device for and we know how to execute.
 *
 * EXAMPLE ONLY!
 */
void gadgetcontrol_execute(command_t *cmd) {
  gadget_t *focusedGadget;

  if(cmd->userIsWatching) {
    // The user is watching! Send out measurements asap!
    gadgetagent_refreshDevices();
  }

  // 1. Filter the command by device ID.
  if((focusedGadget = gadgetmanager_getByUuid(cmd->deviceId)) != NULL) {

    // 2. I have this device. Filter the command by name
    if(strcmp(cmd->commandName, "outletStatus") == 0) {
      char url[PATH_MAX];
      char rxBuffer[GADGET_MAX_MSG_SIZE];
      http_param_t params;

      params.verbose = false;
      params.timeouts.connectTimeout = 3;
      params.timeouts.transferTimeout = 15;

      // 3. This is a command I know how to execute!
      snprintf(url, sizeof(url), "%s/set.xml/%d", focusedGadget->ip, atoi(cmd->argument));
      if(0 != libhttpcomm_getMsg(NULL, url, NULL, NULL, rxBuffer, sizeof(rxBuffer), params, NULL)) {
        iotxml_sendResult(cmd->commandId, IOT_RESULT_DEVICECONNECTIONERROR);
        return;
      }

      if(strstr(rxBuffer, "success") == NULL) {
        iotxml_sendResult(cmd->commandId, IOT_RESULT_DEVICEEXECUTIONERROR);
        return;
      }

      iotxml_sendResult(cmd->commandId, IOT_RESULT_EXECUTED);

      // We just executed the command, refresh the measurements quickly
      gadgetagent_refreshDevices();
    }
  }
}
