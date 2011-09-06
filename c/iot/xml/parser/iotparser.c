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
 * This module is responsible for parsing commands from the server
 * @author David Moss
 */

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <rpc/types.h>
#include <stdio.h>

#include <libxml/parser.h>

#include "ioterror.h"
#include "iotdebug.h"
#include "iotcommandlisteners.h"
#include "iotparser.h"
#include "eui64.h"


/** True if a param tag was found in the command */
static bool paramTagFound;

/***************** Private Prototypes ****************/
static void _iotparser_xml_startElementHandler(void *ctx, const xmlChar *name, const xmlChar **atts);

static void _iotparser_xml_endElementHandler(void *ctx, const xmlChar *name);

static void _iotparser_xml_charactersHandler(void *ctx, const xmlChar *ch, int len);

/***************** Public Functions ****************/
error_t iotxml_parse(const char *xml, int len) {
  command_t command;
  memset(&command, 0x0, sizeof(command));

  xmlSAXHandler saxHandler = {
      NULL, // internalSubsetHandler,
      NULL, // isStandaloneHandler,
      NULL, // hasInternalSubsetHandler,
      NULL, // hasExternalSubsetHandler,
      NULL, // resolveEntityHandler,
      NULL, // getEntityHandler,
      NULL, // entityDeclHandler,
      NULL, // notationDeclHandler,
      NULL, // attributeDeclHandler,
      NULL, // elementDeclHandler,
      NULL, // unparsedEntityDeclHandler,
      NULL, // setDocumentLocatorHandler,
      NULL, // startDocument
      NULL, // endDocument
      _iotparser_xml_startElementHandler, // startElement
      _iotparser_xml_endElementHandler, // endElement
      NULL, // reference,
      _iotparser_xml_charactersHandler, //characters
      NULL, // ignorableWhitespace
      NULL, // processingInstructionHandler,
      NULL, // comment
      NULL, // warning
      NULL, // error
      NULL, // fatal
  };

  command.userIsWatching = (strstr(xml, "CONT") != NULL);

  SYSLOG_DEBUG("Parsing XML: %s", xml);
  if(0 != xmlSAXUserParseMemory(&saxHandler, &command, xml, len)) {
    SYSLOG_ERR("Couldn't parse XML");
    return FAIL;
  }

  return SUCCESS;
}

/***************** Private Functions ****************/
/**
 * XML start element handler
 */
static void _iotparser_xml_startElementHandler(void *ctx, const xmlChar *name, const xmlChar **atts) {
  int i;
  char *attr;
  char *value;
  command_t *command = (command_t *) ctx;


  if(strcmp((char *) name, IOTPARSER_TAG_COMMAND) == 0) {
    // New command, clear out all the residual command and argument information
    paramTagFound = false;
    bzero(command->deviceId, EUI64_STRING_SIZE);
    bzero(command->commandName, IOT_COMMAND_NAME_STRING_SIZE);
    command->commandId = -1;
    command->asciiIndex = 0;
    command->argument = NULL;
    command->argSize = 0;

  } else if(strcmp((char *) name, IOTPARSER_TAG_PARAM) == 0) {
    // New parameter, clear out the residual argument information but leave
    // everything else intact
    paramTagFound = true;
    command->asciiIndex = 0;
    command->argument = NULL;
    command->argSize = 0;
  }

  if (atts != NULL) {
    for (i = 0; (atts[i] != NULL); i++) {
      attr = (char *) atts[i++];
      value = (char *) atts[i];

      if(strcmp(attr, IOTPARSER_ATTR_COMMANDID) == 0) {
        command->commandId = atoi(value);

      } else if(strcmp(attr, IOTPARSER_ATTR_DEVICEID) == 0) {
        strncpy(command->deviceId, value, EUI64_STRING_SIZE);

      } else if(strcmp(attr, IOTPARSER_ATTR_INDEX) == 0) {
        command->asciiIndex = *((char *) value);

      } else if(strcmp(attr, IOTPARSER_ATTR_COMMANDTYPE) == 0) {
        // Command types always appear before command names.
        // If this command happens to not have a name, then the default
        // is to make the command name the same as the command type.
        strncpy(command->commandName, value, IOT_COMMAND_NAME_STRING_SIZE);

      } else if(strcmp(attr, IOTPARSER_ATTR_COMMANDNAME) == 0) {
        // If the parameter has a name, then that is the real command name
        strncpy(command->commandName, value, IOT_COMMAND_NAME_STRING_SIZE);
      }
    }
  }
}

/**
 * XML end element handler
 */
static void _iotparser_xml_endElementHandler(void *ctx, const xmlChar *name) {
  if(strcmp((char *) name, IOTPARSER_TAG_S2H) == 0) {
    command_t *command = (command_t *) ctx;

    // Send out a command to all listeners that there are no more commands
    // This is useful when we might receive several commands that we buffered
    // up because they need to execute simultaneously
    command->noMoreCommands = true;
    bzero(command->deviceId, EUI64_STRING_SIZE);
    bzero(command->commandName, IOT_COMMAND_NAME_STRING_SIZE);
    command->asciiIndex = 0;
    command->commandId = -1;
    command->argument = NULL;
    command->argSize = 0;

    iotcommandlisteners_broadcast(command);

  } else if(strcmp((char *) name, IOTPARSER_TAG_PARAM) == 0) {
    // This is the end of a param tag
    paramTagFound = true;
    iotcommandlisteners_broadcast((command_t *) ctx);

  } else if(strcmp((char *) name, IOTPARSER_TAG_COMMAND) == 0 && !paramTagFound) {
    // This is the end of a command tag where there were no param tags within it
    iotcommandlisteners_broadcast((command_t *) ctx);
  }
}


/**
 * XML character handler
 */
static void _iotparser_xml_charactersHandler(void *ctx, const xmlChar *ch, int len) {
  command_t *command = (command_t *) ctx;
  command->argument = (char *) ch;
  command->argSize = len;
}


