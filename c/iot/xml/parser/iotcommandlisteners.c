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
 * This module tracks listener functions that want to receive commands from
 * the server.
 * @author David Moss
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "iotapi.h"
#include "iotdebug.h"
#include "ioterror.h"
#include "iotcommandlisteners.h"

/** Array of listeners */
static struct {

  commandlistener_f l;

  char type[TYPE_ATTRIBUTE_CHARS_TO_MATCH];

  bool inUse;

} listeners[TOTAL_COMMAND_LISTENERS];


/***************** Public Functions ****************/
/**
 * Add a listener to receive parsed commands
 *
 * @param commandlistener_f Function pointer to a function(command_t cmd)
 * @param type Type attribute to listen for, "set", "delete", "discover", etc.
 * @return SUCCESS if the listener was added
 */
error_t iotxml_addCommandListener(commandlistener_f l, char *type) {
  int i;

  for(i = 0; i < TOTAL_COMMAND_LISTENERS; i++) {
    if(listeners[i].inUse && listeners[i].l == l) {
      // Already in the log
      return SUCCESS;
    }
  }

  for(i = 0; i < TOTAL_COMMAND_LISTENERS; i++) {
    if(!listeners[i].inUse) {
      listeners[i].inUse = true;
      strncpy(listeners[i].type, type, TYPE_ATTRIBUTE_CHARS_TO_MATCH);
      listeners[i].l = l;
      return SUCCESS;
    }
  }

  return FAIL;
}

/**
 * Remove a listener from the command
 * @param commandlistener_f Function pointer to remove
 * @return SUCCESS if the listener was found and removed
 */
error_t iotxml_removeCommandListener(commandlistener_f l) {
  int i;

  for(i = 0; i < TOTAL_COMMAND_LISTENERS; i++) {
    if(listeners[i].inUse && listeners[i].l == l) {
      listeners[i].inUse = false;
      return SUCCESS;
    }
  }

  return FAIL;
}

/**
 * Broadcast a command to all listeners
 * @param msg Message to broadcast
 * @param len Length of the message
 * @param type Type attribute of the command describing the action
 */
error_t iotcommandlisteners_broadcast(command_t *cmd) {
  int i;

  for(i = 0; i < TOTAL_COMMAND_LISTENERS; i++) {
    if(listeners[i].inUse) {
      if(strncmp(listeners[i].type, cmd->commandType, TYPE_ATTRIBUTE_CHARS_TO_MATCH) == 0) {
        listeners[i].l(cmd);
      }
    }
  }

  return SUCCESS;
}

/**
 * @return the total number of registered listeners
 */
int iotcommandlisteners_totalListeners() {
  int i;
  int total = 0;

  for(i = 0; i < TOTAL_COMMAND_LISTENERS; i++) {
    if(listeners[i].inUse) {
      total++;
    }
  }

  return total;
}

