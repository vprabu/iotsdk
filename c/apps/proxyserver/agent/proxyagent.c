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
 * This proxy agent manages the proxy by communicating through the proxy
 * to the server, sending periodic heartbeats, and receiving commands.
 * Because it's part of the proxyserver itself, we do not need to create
 * a socket connection to the proxyserver.
 *
 * @author David Moss
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "libconfigio.h"

#include "ioterror.h"
#include "iotdebug.h"
#include "proxyagent.h"
#include "proxy.h"
#include "proxylisteners.h"
#include "proxycli.h"
#include "eui64.h"
#include "proxyserver.h"
#include "proxyconfig.h"
#include "iotapi.h"


/** Thread termination flag */
static bool terminate;

/** Thread */
static pthread_t sThreadId;

/** Thread attributes */
static pthread_attr_t sThreadAttr;

/** Heartbeat interval in seconds */
static int heartbeatInterval_sec = PROXY_AGENT_HEARTBEAT_INTERVAL;

/** Number of times this proxy app has been rebooted */
static int reboots;

/** How much time this proxy has been alive */
static int aliveTime;

/** Device type of the proxy, set in the configuration file */
static int deviceType;

/** Device ID of this proxy, our EUI64 */
static char deviceId[EUI64_STRING_SIZE];

/***************** Private Prototypes ****************/
static void application_receive(const char *msg, int len);

static void *_proxyAgentThread(void *params);

static void _doCommand(command_t *command);

static void _sendHeartbeat();

static void _captureRebootCount();

static void _captureDeviceType();

/***************** Public Functions ****************/
/**
 * Start the proxy agent
 */
error_t proxyagent_start() {

  // Get our unique device ID of this proxy
  if(eui64_toString(deviceId, sizeof(deviceId)) != SUCCESS) {
    SYSLOG_ERR("[proxyagent] Skipping heartbeat since we don't have an EUI64");
    return FAIL;
  }

  // Add a listener directly to the inbound server messages
  if(proxylisteners_addListener(&application_receive) != SUCCESS) {
    SYSLOG_DEBUG("[proxyagent]: Proxy is out of listener slots");
    return FAIL;
  }

  // Capture and increment the reboot count
  _captureRebootCount();

  // Get the proxy's device type from the configuration file
  _captureDeviceType();

  // Make sure our thread is good to go
  terminate = false;

  // Initialize the thread
  pthread_attr_init(&sThreadAttr);

  // Will run detached from the main dispatcher thread
  pthread_attr_setdetachstate(&sThreadAttr, PTHREAD_CREATE_DETACHED);

  // Round robin schedule is fine when processing is extremely low
  pthread_attr_setschedpolicy(&sThreadAttr, SCHED_RR);

  // Create the thread
  if (pthread_create(&sThreadId, &sThreadAttr, &_proxyAgentThread, NULL)) {
    SYSLOG_ERR("[proxyagent]: Creating proxy thread failed: %s", strerror(errno));
    return FAIL;
  }

  return SUCCESS;
}

/**
 * Stop the proxy agent
 */
void proxyagent_stop() {
  terminate = true;
}


/**
 * The application layer is responsible for routing outbound messages
 * appropriately.
 *
 * Route outbound messages in this application directly to the proxy, not
 * through a socket, since we're connected directly to the proxy.
 *
 * @param msg Message to send to the server
 * @param len Length of the message to send
 * @return SUCCESS if the message was sent
 */
error_t application_send(const char *msg, int len) {
  return proxy_send(msg, len);
}

/**
 * The application layer is responsible for routing inbound mesages
 * appropriately.
 *
 * Route inbound messages in this application directly to the XML parser.
 *
 * @param msg Message received
 * @param len Length of the message received
 */
void application_receive(const char *msg, int len) {
  if(strstr(msg, "xml") != NULL) {
     iotxml_parse(msg, len);

  } else {
    SYSLOG_INFO("[proxyagent] Unknown message format: %s", msg);
  }
}


/***************** Private Functions ****************/
/**
 * Thread to periodically send updates to the server
 */
static void *_proxyAgentThread(void *params) {

  // Listen for commands
  iotxml_addCommandListener(&_doCommand);

  // Main loop
  while (!terminate) {
    _sendHeartbeat();
    sleep(heartbeatInterval_sec);
    aliveTime += heartbeatInterval_sec;
  }

  iotxml_removeCommandListener(_doCommand);

  SYSLOG_INFO("*** Exiting Proxy Agent Thread ***");
  pthread_exit(NULL);
  return NULL;
}

/**
 * Handle a command from the server
 */
static void _doCommand(command_t *cmd) {
  if(strcmp(cmd->deviceId, deviceId) == 0) {
    // This command is to me
    if(strcmp(cmd->commandName, "UploadInterval") == 0) {
      char argument[cmd->argSize + 1];
      bzero(argument, sizeof(argument));
      int uploadInterval;

      strncpy(argument, cmd->argument, cmd->argSize);
      uploadInterval = atoi(argument);
      proxyconfig_setUploadIntervalSec(uploadInterval);
      iotxml_sendResult(cmd->commandId, IOT_RESULT_EXECUTED);
    }
  }

  /**
   * Debug code for your convenience
   *
  SYSLOG_INFO(">> RECEIVED COMMAND: ");
  SYSLOG_INFO("noMoreCommands=%d", cmd->noMoreCommands);
  SYSLOG_INFO("userIsWatching=%d", cmd->userIsWatching);
  SYSLOG_INFO("deviceId=%s", cmd->deviceId);
  SYSLOG_INFO("commandId=%d", cmd->commandId);
  SYSLOG_INFO("commandName=%s", cmd->commandName);
  SYSLOG_INFO("argSize=%d", cmd->argSize);

  if(cmd->asciiIndex > 0) {
    SYSLOG_INFO("asciiIndex=%c", cmd->asciiIndex);
  }

  if(cmd->argSize > 0) {
    // We add 1 to the argument buffer to null-terminate the string
    char argument[cmd->argSize + 1];
    bzero(argument, sizeof(argument));
    strncpy(argument, cmd->argument, cmd->argSize);
    SYSLOG_INFO("argument=%s",  argument);
  }

  SYSLOG_INFO("<< RECEIVED COMMAND");
   */
}

/**
 * Send a heartbeat to the server to declare this proxy is still alive
 */
static void _sendHeartbeat() {
  char myMsg[1024];
  char firmwareVersion[8];
  int offset = 0;

  // Get the Git SHA1 firmware version, padded on the left with 0's
  snprintf(firmwareVersion, sizeof(firmwareVersion), "%.07x", GIT_FIRMWARE_VERSION);

  // 1. Create a new message
  if(iotxml_newMsg(myMsg, sizeof(myMsg)) != SUCCESS) {
    SYSLOG_INFO("[proxyagent] Couldn't start a new message!");
    return;
  }

  // 2. Fill out the message

  // Proxy firmware version is the Git repository SHA1 identifier
  offset += iotxml_addString(myMsg + offset, sizeof(myMsg) - offset,
    deviceId,
    deviceType,
    IOT_PARAM_PROFILE,
    PARAM_NAME_FIRMWARE_VERSION,
    0,
    firmwareVersion);

  // How many seconds the proxy has been alive
  offset += iotxml_addInt(myMsg + offset, sizeof(myMsg) - offset,
    deviceId,
    deviceType,
    IOT_PARAM_PROFILE,
    PARAM_NAME_ALIVE_TIME,
    0,
    aliveTime);

  // Number of times the proxy has rebooted
  offset += iotxml_addInt(myMsg + offset, sizeof(myMsg) - offset,
    deviceId,
    deviceType,
    IOT_PARAM_PROFILE,
    PARAM_NAME_REBOOTS,
    0,
    reboots);

  // Upload interval of the proxy
  offset += iotxml_addInt(myMsg + offset, sizeof(myMsg) - offset,
    deviceId,
    deviceType,
    IOT_PARAM_PROFILE,
    PARAM_NAME_UPLOAD_INTERVAL,
    0,
    (int) proxyconfig_getUploadIntervalSec());

  // 3. Send the message
  if(iotxml_send(myMsg, sizeof(myMsg)) == SUCCESS) {
    SYSLOG_INFO("[proxyagent] Heartbeat");
  }
}

/**
 * Grab the reboot count from the configuration file, increment it, and
 * store it back in the configuration file
 */
static void _captureRebootCount() {
  char buffer[8];

  bzero(buffer, sizeof(buffer));

  if(libconfigio_read(proxycli_getConfigFilename(), CONFIGIO_PROXY_REBOOTS, buffer, sizeof(buffer)) >= 0) {
    reboots = atoi(buffer);
  }

  reboots++;

  snprintf(buffer, sizeof(buffer), "%d", reboots);

  libconfigio_write(proxycli_getConfigFilename(), CONFIGIO_PROXY_REBOOTS, buffer);
}

/**
 * Capture the device type from the configuration file
 */
static void _captureDeviceType() {
  char buffer[8];
  bzero(buffer, sizeof(buffer));
  libconfigio_read(proxycli_getConfigFilename(), CONFIGIO_PROXY_DEVICE_TYPE_TOKEN_NAME, buffer, sizeof(buffer));
  deviceType = atoi(buffer);
}
