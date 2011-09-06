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
 * This module allows the proxy to broadcast to all interested listeners
 * who want to receive commands from the server
 *
 * @author David Moss
 */

#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include "proxylisteners.h"
#include "iotdebug.h"
#include "ioterror.h"

/** Array of listeners */
static struct {

  proxylistener l;

  bool inUse;

} proxyListeners[TOTAL_PROXY_LISTENERS];

/** Mutex to protect proxyListeners */
static pthread_mutex_t sProxyListenersMutex;


/***************** Proxylisteners Public ****************/
/**
 * Start the proxy listener by initialize the mutex
 */
void proxylisteners_start() {
  pthread_mutex_init(&sProxyListenersMutex, NULL);
}

/**
 * Stop the proxy listener by destroying the mutex
 */
void proxylisteners_stop() {
  pthread_mutex_destroy(&sProxyListenersMutex);
}


/**
 * Add a listener to the messages sent by the server.  This simply
 * forwards to the correct listener module.
 *
 * @param proxylistener Function pointer to a function(const char *msg, int len)
 * @return SUCCESS if the listener was added
 */
error_t proxylisteners_addListener(proxylistener l) {
  int i;

  pthread_mutex_lock(&sProxyListenersMutex);
  for(i = 0; i < TOTAL_PROXY_LISTENERS; i++) {
    if(proxyListeners[i].inUse && proxyListeners[i].l == l) {
      // Already in the log
      SYSLOG_DEBUG("Listener already exists");
      pthread_mutex_unlock(&sProxyListenersMutex);
      return SUCCESS;
    }
  }

  for(i = 0; i < TOTAL_PROXY_LISTENERS; i++) {
    if(!proxyListeners[i].inUse) {
      SYSLOG_DEBUG("Adding proxy listener to element %d", i);
      proxyListeners[i].inUse = true;
      proxyListeners[i].l = l;
      pthread_mutex_unlock(&sProxyListenersMutex);
      return SUCCESS;
    }
  }
  pthread_mutex_unlock(&sProxyListenersMutex);

  return FAIL;
}

/**
 * Remove a listener from the proxy
 * @param proxylistener Function pointer to remove
 * @return SUCCESS if the listener was found and removed
 */
error_t proxylisteners_removeListener(proxylistener l) {
  int i;

  pthread_mutex_lock(&sProxyListenersMutex);
  for(i = 0; i < TOTAL_PROXY_LISTENERS; i++) {
    if(proxyListeners[i].inUse && proxyListeners[i].l == l) {
      SYSLOG_DEBUG("Removing proxy listener at element %d", i);
      proxyListeners[i].inUse = false;
      pthread_mutex_unlock(&sProxyListenersMutex);
      return SUCCESS;
    }
  }
  pthread_mutex_unlock(&sProxyListenersMutex);

  return FAIL;
}

/**
 * Broadcast a message to all proxy listeners
 * @param msg Message to broadcast
 * @param len Length of the message
 */
error_t proxylisteners_broadcast(const char *msg, int len) {
  int i;

  if(*msg && len > 0) {
    SYSLOG_INFO("[broadcast]: %s", msg);

    pthread_mutex_lock(&sProxyListenersMutex);
    for(i = 0; i < TOTAL_PROXY_LISTENERS; i++) {
      if(proxyListeners[i].inUse) {
        SYSLOG_INFO("[broadcast]: Broadcasting to known client");
        proxyListeners[i].l(msg, len);
      }
    }
    pthread_mutex_unlock(&sProxyListenersMutex);

  } else {
    SYSLOG_INFO("[broadcast]: Nobody to broadcast to :(");
    return FAIL;
  }

  return SUCCESS;
}

/**
 * @return the total number of registered listeners
 */
int proxylisteners_totalListeners() {
  int i;
  int total = 0;

  pthread_mutex_lock(&sProxyListenersMutex);
  for(i = 0; i < TOTAL_PROXY_LISTENERS; i++) {
    if(proxyListeners[i].inUse) {
      total++;
    }
  }
  pthread_mutex_unlock(&sProxyListenersMutex);

  return total;
}
