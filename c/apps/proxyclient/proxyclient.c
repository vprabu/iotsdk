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
 * This is just an example client application to show how to interact with
 * the proxy server through a socket
 * @author David Moss
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "ioterror.h"
#include "iotdebug.h"
#include "proxyserver.h"
#include "clientsocket.h"
#include "iotapi.h"


/** On / off status of our virtual sockets */
static bool isOn[5];

/** My virtual device ID */
#define MY_DEVICE_ID "B0C8AD000001"

/***************** Prototypes ****************/
static void _sendXmlMessage(const char *deviceId, int deviceType, int aliveTime);

static void _doCommand(command_t *cmd);


/***************** Functions ****************/
/**
 * Main function
 */
int main(int argc, char *argv[]) {
  int i;

  SYSLOG_INFO("[client] Opening socket...");

  // Open a socket to the proxy server
  if(clientsocket_open("localhost", DEFAULT_PROXY_PORT) != SUCCESS) {
    SYSLOG_DEBUG("[client] Couldn't open client socket");
    exit(1);
  }

  // Listen for commands
  iotxml_addCommandListener(&_doCommand);

  // Turn on all of our virtual plugs
  for(i = 0; i < sizeof(isOn); i++) {
    isOn[i] = true;
  }

  // Now we use i to keep track of time in each loop
  i = 0;
  while(1) {
    sleep(5);
    i++;

    // Device type 6 is a generic powerstrip registered with the cloud
    _sendXmlMessage(MY_DEVICE_ID, 6, i*5);
  }

  return 0;
}

/**
 * The application layer is responsible for routing raw messages received
 * from the server.
 *
 * Because we use a clientsocket module to communicate with the proxy server,
 * the clientsocket module will execute this function upon receiving a message.
 * Here in our application layer, we route the message to the appropriate
 * parser. Currently this parser is XML, but in the future it could be JSON.
 * @param msg The received message
 * @param len The length of the message
 */
void application_receive(const char *msg, int len) {
  if(strstr(msg, "xml") != NULL) {
       iotxml_parse(msg, len);

    } else {
      SYSLOG_INFO("[client] Unknown message format: %s", msg);
    }
}

/**
 * The application layer is responsible for routing raw send messages to the
 * appropriate destination.  Because this client uses a clientsocket
 * module to send a message to the proxy server, we simply forward the
 * message on to the clientsocket module.
 * @param msg The constructed message to send
 * @param len The size of the message
 * @return SUCCESS if the message is sent
 */
error_t application_send(const char *msg, int len) {
  return clientsocket_send(msg, len);
}

/***************** Private Functions *****************/
/**
 * Execute a command from the server
 */
static void _doCommand(command_t *cmd) {
  if(strcmp(cmd->deviceId, MY_DEVICE_ID) == 0) {
    // This command is to me
    if(strcmp(cmd->commandName, "OutletStatus") == 0) {
      // We add 1 to the argument buffer to null-terminate the string
      char argument[cmd->argSize + 1];
      bzero(argument, sizeof(argument));
      strncpy(argument, cmd->argument, cmd->argSize);
      SYSLOG_INFO("Argument size = %d; sizeof(argument) = %d", cmd->argSize, sizeof(argument));

      if(cmd->asciiIndex >= 'A') {
        if(isOn[cmd->asciiIndex - 'A'] != (strcmp(argument, "ON") == 0)) {
          isOn[cmd->asciiIndex - 'A'] = (strcmp(argument, "ON") == 0);
          printf("Plug %c is now set to %s\n", cmd->asciiIndex, argument);
          SYSLOG_INFO("Plug %c is now set to %s\n", cmd->asciiIndex, argument);
        }
        iotxml_sendResult(cmd->commandId, IOT_RESULT_EXECUTED);

      } else {
        iotxml_sendResult(cmd->commandId, IOT_RESULT_WRONGFORMAT);
      }
    }
  }
}


/**
 * This is my example function to send a message. I included
 * some arguments to the function so we can play around with the device ID,
 * device type, and the alive time to generate something a little more
 * dynamic.
 */
static void _sendXmlMessage(const char *deviceId, int deviceType, int aliveTime) {
  char myMsg[4096];
  int offset = 0;
  char index;

  // 1. Create a new message
  if(iotxml_newMsg(myMsg, sizeof(myMsg)) != SUCCESS) {
    SYSLOG_INFO("[client] Couldn't start a new message!");
    return;
  }

  // 2. Start filling it out. Keep the paramType's together for optimization
  offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
    deviceId,
    deviceType,
    IOT_PARAM_PROFILE,
    "FW",
    0,
    "1.0");

  offset += iotxml_addInt(myMsg + offset, sizeof(myMsg) - offset,
    deviceId,
    deviceType,
    IOT_PARAM_PROFILE,
    "AliveTime",
    0,
    aliveTime);

  for(index = 'A'; index < 'F'; index++) {
    char buffer[10];

    if(isOn[index - 'A']) {
      snprintf(buffer, sizeof(buffer), "ON");
    } else {
      snprintf(buffer, sizeof(buffer), "OFF");
    }

    offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
      deviceId,
      deviceType,
      IOT_PARAM_PROFILE,
      "OutletStatus",
      index,
      buffer);
  }

  // Switch to a measurement param type
  for(index = 'A'; index < 'F'; index++) {
    char buffer[10];
    float milliAmps = ((1000 * (index - 'A')) + 1000) * isOn[index - 'A'];
    float milliVolts = 120000.0;
    float watts = (milliAmps * milliVolts) / 1000000.0;

    snprintf(buffer, sizeof(buffer), "%4.3f", milliAmps);
    offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
      deviceId,
      deviceType,
      IOT_PARAM_MEASURE,
      "amps",
      index,
      buffer);

    snprintf(buffer, sizeof(buffer), "%4.3f", watts);
    offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
      deviceId,
      deviceType,
      IOT_PARAM_MEASURE,
      "watts",
      index,
      buffer);

    snprintf(buffer, sizeof(buffer), "%4.3f", milliVolts);
    offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
      deviceId,
      deviceType,
      IOT_PARAM_MEASURE,
      "voltage",
      index,
      buffer);
  }

  // Send the message
  if(iotxml_send(myMsg, sizeof(myMsg)) == SUCCESS) {
    SYSLOG_INFO("[client] Sent a message!");
    SYSLOG_INFO("%s", myMsg);
  }

}


