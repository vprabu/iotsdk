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

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "libpipecomm.h"

#include "iotdebug.h"
#include "ioterror.h"
#include "clientsocket.h"
#include "proxy.h"


/** Thread termination flag */
static bool gTerminate;

/** Thread */
static pthread_t sThreadId;

/** Thread attributes */
static pthread_attr_t sThreadAttr;

/** Socket file descriptor */
static int socketFd;

/**************** Prototypes ****************/
static void *_clientCommThread(void *params);

/**************** Functions ****************/
/**
 * Open a socket connection to the proxy server
 * @param serverName Name of the server, i.e. "localhost"
 * @param port Port number to connect with, i.e. DEFAULT_PROXY_PORT
 */
error_t clientsocket_open(const char *serverName, int port) {
  struct sockaddr_in serverAddress;
  struct hostent *server;

  assert(serverName);

  SYSLOG_INFO("Attempting to open socket to %s on port %d", serverName, port);

  gTerminate = false;

  socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    SYSLOG_ERR("ERROR opening socket");
    return -1;
  }

  if ((server = gethostbyname(serverName)) == NULL) {
    SYSLOG_ERR("ERROR, no such host\n");
    return -1;
  }

  bzero((char *) &serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  memcpy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
  serverAddress.sin_port = htons(port);

  SYSLOG_INFO("Connecting...");
  if (connect(socketFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    SYSLOG_ERR("ERROR connecting");
    return FAIL;
  }

  SYSLOG_INFO("Connection established on fd %d", socketFd);

  // Initialize the thread
  pthread_attr_init(&sThreadAttr);

  // Will run detached from the main dispatcher thread
  pthread_attr_setdetachstate(&sThreadAttr, PTHREAD_CREATE_DETACHED);

  // Round robin schedule is fine when processing is extremely low
  pthread_attr_setschedpolicy(&sThreadAttr, SCHED_RR);

  // Create the thread
  if (pthread_create(&sThreadId, &sThreadAttr, &_clientCommThread, NULL)) {
    SYSLOG_ERR("Creating proxy thread failed: %s", strerror(errno));
    clientsocket_close();
    return FAIL;
  }

  return SUCCESS;
}

/**
 * Close the socket
 * @return SUCCESS if the socket closed successfully, FAIL if it wasn't open
 */
error_t clientsocket_close() {
  close(socketFd);
  gTerminate = true;
  return SUCCESS;
}

/**
 * Send a message to the proxy
 * @param message Message to send
 * @param len Length of the message to send
 */
error_t clientsocket_send(const char *message, int len) {
  assert(message);

  if (write(socketFd, message, len) < 0) {
    SYSLOG_ERR("ERROR writing to socket");
    return FAIL;
  }

  return SUCCESS;
}

/**
 * Thread for socket receive communications
 */
static void *_clientCommThread(void *params) {
  char inboundMsg[CLIENTSOCKET_INBOUND_MSGSIZE];

  // Main loop
  while (!gTerminate) {
    memset(inboundMsg, 0, sizeof(inboundMsg));

    if(libpipecomm_read(socketFd, inboundMsg, CLIENTSOCKET_INBOUND_MSGSIZE) > 0) {
      SYSLOG_DEBUG("[client] Received: %s", inboundMsg);
      application_receive(inboundMsg, strlen(inboundMsg));
    }
  }

  SYSLOG_INFO("*** Exiting Client Socket Thread ***");
  pthread_exit(NULL);
  return NULL;
}

