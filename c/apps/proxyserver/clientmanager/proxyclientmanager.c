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
 * This module is responsible for tracking open file descriptors so we may
 * use them to broadcast to lister sockets
 * @author David Moss
 */

#include <stdbool.h>
#include <stdlib.h>

#include "proxyclientmanager.h"
#include "ioterror.h"
#include "iotdebug.h"

/** Array of clients, 1 element per open client socket */
proxy_client_t clients[PROXYCLIENTMANAGER_CLIENTS];


/**
 * Add a client socket
 * @param fd File descriptor to add
 * @return SUCCESS if the client was added
 */
error_t proxyclientmanager_add(int fd) {
  int i;
  SYSLOG_DEBUG("Add %d", fd);
  for(i = 0; i < PROXYCLIENTMANAGER_CLIENTS; i++) {
    if(!clients[i].inUse) {
      clients[i].inUse = true;
      clients[i].fd = fd;
      return SUCCESS;
    }
  }

  return FAIL;
}

/**
 * Remove the file descriptor from the list we're tracking
 * @param fd File descriptor
 */
void proxyclientmanager_remove(int fd) {
  int i;
  SYSLOG_DEBUG("Remove %d", fd);
  for(i = 0; i < PROXYCLIENTMANAGER_CLIENTS; i++) {
    if(clients[i].fd == fd) {
      clients[i].inUse = false;
    }
  }
}

/**
 * @return the number of elements in our proxy_client_t array
 */
int proxyclientmanager_size() {
  return PROXYCLIENTMANAGER_CLIENTS;
}

/**
 * @param index Index into the array of clients
 * @return the array of proxy_client_t's
 */
proxy_client_t *proxyclientmanager_get(int index) {
  if(index < PROXYCLIENTMANAGER_CLIENTS) {
    return &clients[index];
  } else {
    return NULL;
  }
}

