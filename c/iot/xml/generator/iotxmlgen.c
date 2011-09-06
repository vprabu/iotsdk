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
 * SERVICES { } LOSS OF USE, DATA, OR PROFITS { } OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>

#include "ioterror.h"
#include "iotdebug.h"
#include "iotapi.h"
#include "iotxmlgen.h"
#include "timestamp.h"


/** Copy of the last Device ID we generated XML for */
static char lastDeviceId[IOTGEN_DEVICE_ID_SIZE];

/** Copy of the last param type we generated XML for */
static int lastParamType = -1;

/** True if there is a message currently being constructed */
static bool inProgress = false;

/**
 * param_type_e's map to these key words recognized by the server in XML
 */
static const char *paramTypeMap[] = {
    "profile",
    "measure",
    "alert",
};

/***************** Prototypes ****************/

/***************** Public IOT XML Generator ****************/
/**
 * Begin a new message. This locks out other callers to iotxml_newMsg(..)
 * until the current message has been sent, because the message is constructed
 * based on the context of the parameters that are being passed in across
 * mutiple function calls.
 *
 * @param dest the destination buffer for the message
 * @param maxSize the maximum size of the message
 * @return SUCCESS if we started the new message, FAIL if another message is
 *     already being constructed and needs to be sent.
 */
error_t iotxml_newMsg(char *destMsg, int maxSize) {
  if(inProgress) {
    return FAIL;
  }

  inProgress = true;

  bzero(lastDeviceId, sizeof(lastDeviceId));
  bzero(destMsg, maxSize);
  lastParamType = -1;
  return SUCCESS;
}

/**
 * Add a string to the message. This function operates similar to snprintf,
 * for example:
 *
 * <code>
 *   offset += iotxml_addString(destMsg + offset, sizeof(destMsg) - offset, ...)
 * </code>
 *
 * @param dest Starting point in a destination buffer to write data
 * @param maxSize maximum size remaining past the start point
 * @param deviceId Device ID string
 * @param deviceType Device type that is registered with the cloud service
 * @param paramType Type of parameter, see param_type_e enum in iotapi.h
 * @param asciiParamIndex Index of this parameter, for instance if you are
 *     measuring watts from a powerstrip with multiple sockets, where each
 *     socket has its own index number.  Use NULL or 0 if your param doesn't
 *     need an index.
 * @param paramValue Param value string
 * @return the size of the string written to the destination pointer
 */
int iotxml_addString(char *dest, int maxSize, const char *deviceId, int deviceType, param_type_e paramType, const char *paramName, char asciiParamIndex, const char *paramValue) {
  int offset = 0;

  // First check if we need a new param block
  if(strcmp(deviceId, lastDeviceId) != 0 || lastParamType != paramType) {
    // We need to start a new tag
    if(lastParamType >= 0) {
      // But first we need to close off the last tag
      offset += snprintf(dest + offset, maxSize - offset,
          "</%s>",
          paramTypeMap[lastParamType]);
    }

    lastParamType = paramType;
    strncpy(lastDeviceId, deviceId, sizeof(lastDeviceId));

    offset += snprintf(dest + offset, maxSize - offset,
        "<%s deviceId=\"%s\" deviceType=\"%d\" timestamp=\"",
        paramTypeMap[lastParamType],
        lastDeviceId,
        deviceType);

    offset += getTimestamp(dest + offset, maxSize - offset);

    offset += snprintf(dest + offset, maxSize - offset, "\">");
  }

  // Next add in the new param
  offset += snprintf(dest + offset, maxSize - offset,
      "<param name=\"%s\"", paramName);

  // Give it an index # if a valid ASCII paramIndex was given
  if(asciiParamIndex >= '0') {
    offset += snprintf(dest + offset, maxSize - offset,
        " index=\"%c\"", asciiParamIndex);
  }

  // Add the value
  offset += snprintf(dest + offset, maxSize - offset,
      ">%s</param>", paramValue);

  return offset;
}

/**
 * Add an int to the message. This function operates similar to snprintf,
 * for example:
 *
 * <code>
 *   offset += iotxml_addString(destMsg + offset, sizeof(destMsg) - offset, ...)
 * </code>
 *
 * @param dest Starting point in a destination buffer to write data
 * @param maxSize maximum size remaining past the start point
 * @param deviceId Device ID string
 * @param deviceType Device type that is registered with the cloud service
 * @param paramType Type of parameter, see param_type_e enum in iotapi.h
 * @param asciiParamIndex Index of this parameter, for instance if you are
 *     measuring watts from a powerstrip with multiple sockets, where each
 *     socket has its own index number.  Use NULL or 0 if your param doesn't
 *     need an index.
 * @param paramValue Param value integer
 * @return the size of the string written to the destination pointer
 */
int iotxml_addInt(char *dest, int maxSize, const char *deviceId, int deviceType, param_type_e paramType, const char *paramName, char asciiParamIndex, int paramValue) {
  char value[IOTGEN_NUMERIC_STRING_SIZE];
  snprintf(value, IOTGEN_NUMERIC_STRING_SIZE, "%d", paramValue);
  return iotxml_addString(dest, maxSize, deviceId, deviceType, paramType, paramName, asciiParamIndex, value);
}

/**
 * Close off the last tag and send the message. This will allow other message
 * creating functions to create a new message using iotxml_newMsg(...)
 *
 * @param destMsg Pointer to the start of the destination message
 * @param maxSize Maximum size of the message buffer
 */
error_t iotxml_send(char *destMsg, int maxSize) {
  int totalSize = strlen(destMsg);

  if(lastParamType >= 0) {
    // Close off the last tag
    snprintf(destMsg + strlen(destMsg), maxSize - totalSize,
        "</%s>",
        paramTypeMap[lastParamType]);
  }

  inProgress = false;
  return application_send(destMsg, strlen(destMsg));
}

/**
 * Abort the current message.  This is useful when we start a message to send
 * data, but then find out after iterating through our devices that we
 * have no data to send.
 */
void iotxml_abortMsg() {
  inProgress = false;
}

/**
 * Create XML to represent and convey the result of a command back to the server
 * @param commandId The command ID as the server presented to us
 * @param result The result code for the command
 * @return SUCCESS if we will send the response back to the server
 */
error_t iotxml_sendResult(int commandId, result_code_e result) {
  char xmlResult[IOTGEN_RESULT_XML_SIZE];
  bzero(xmlResult, IOTGEN_RESULT_XML_SIZE);
  snprintf(xmlResult, IOTGEN_RESULT_XML_SIZE, "<response cmdId=\"%d\" result=\"%d\"/>", commandId, result);
  SYSLOG_INFO("Sending result: %s", xmlResult);
  return application_send(xmlResult, strlen(xmlResult));
}

