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
 * Internet of Things API connecting physical devices to cloud services
 *
 * @author David Moss
 * @author Andrey Malashenko
 */

#ifndef IOTAPI_H
#define IOTAPI_H

#include <stdbool.h>
#include "ioterror.h"
#include "eui64.h"

enum {
  IOT_COMMAND_NAME_STRING_SIZE = 16,
  IOT_COMMAND_VALUE_STRING_SIZE = 256,
  IOT_COMMAND_TYPE_STRING_SIZE = 10,
};

/**
 * Param types enumerator
 */
typedef enum param_type_e {
  IOT_PARAM_PROFILE,
  IOT_PARAM_MEASURE,
  IOT_PARAM_ALERT,
} param_type_e;

/**
 * Result codes enumerator
 */
typedef enum result_code_e {
  IOT_RESULT_RECEIVED = 0,
  IOT_RESULT_EXECUTED = 1,
  IOT_RESULT_HUBERROR = 2,
  IOT_RESULT_HUBNOTSUPPORTED = 3,
  IOT_RESULT_DEVICENOTIDENTIFIED = 4,
  IOT_RESULT_DEVICENOTSUPPORTED = 5,
  IOT_RESULT_DEVICECONNECTIONERROR = 6,
  IOT_RESULT_DEVICEEXECUTIONERROR = 7,
  IOT_RESULT_WRONGFORMAT = 8,
} result_code_e;


/**
 * Structure to store an individual command
 */
typedef struct command_t {

  /** True when a user is actively watching in real time, send data quickly */
  bool userIsWatching;

  /**
   * True when this command is not valid except for the userIsWatching flag
   * and there are no more commands in this message from the server,
   * so execute all the commands you might have been buffering now.
   */
  bool noMoreCommands;

  /** Command ID to use in a response to the server */
  int commandId;

  /** Destination device ID */
  char deviceId[EUI64_STRING_SIZE];

  /** Type attribute, i.e. "set", "delete", "discover", etc. */
  char commandType[IOT_COMMAND_TYPE_STRING_SIZE];

  /** Index # in ascii */
  char asciiIndex;

  /** Command name */
  char commandName[IOT_COMMAND_NAME_STRING_SIZE];

  /** Command arguments */
  const char *argument;

  /** Arguments size */
  int argSize;

} command_t;

/**
 * Command listener function definitions take on the form:
 *
 *   void doCommand(command_t *cmd)
 *
 * Command listeners are responsible for matching their device ID and command
 * name, etc.
 */
#ifndef __type_commandlistener
#define __type_commandlistener
typedef void (*commandlistener_f)(command_t *);
#endif

/**
 * This function prototype MUST be implemented by the application layer in order
 * to route the outbound message to the appropriate handler
 * @param msg Final message to send
 * @param len Length of the message to send
 */
error_t application_send(const char *msg, int len);

/***************** API ****************/
int iotxml_newMsg(char *destMsg, int maxSize);

int iotxml_addString(char *dest, int maxSize, const char *deviceId, int deviceType, param_type_e paramType, const char *paramName, char asciiParamIndex, const char *paramValue);

int iotxml_addInt(char *dest, int maxSize, const char *deviceId, int deviceType, param_type_e paramType, const char *paramName, char asciiParamIndex, int paramValue);

error_t iotxml_send(char *destMsg, int maxSize);

void iotxml_abortMsg();

error_t iotxml_sendResult(int commandId, result_code_e result);

error_t iotxml_parse(const char *xml, int len);

error_t iotxml_addCommandListener(commandlistener_f l, char *type);

error_t iotxml_removeCommandListener(commandlistener_f l);


#endif
